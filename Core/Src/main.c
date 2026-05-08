#include "main.h"

/*
 * Project 2 – Distance Measurement and Motion Trend Analyzer
 * Platform  : STM32 NUCLEO-F411RE, bare-metal (no HAL)
 * Sensor    : HC-SR04 ultrasonic distance sensor
 * Peripherals used: GPIO, TIM2 (timer-based pulse generation & timing), USART2
 *
 * Hardware wiring:
 *   HC-SR04 TRIG  -->  PA5  (GPIO output, 10 µs pulse)
 *   HC-SR04 ECHO  -->  PA0  (GPIO input, echo width measured via TIM2)
 *   USART2  TX    -->  PA2  (AF7, connected to ST-Link VCP)
 *   USART2  RX    -->  PA3  (AF7, connected to ST-Link VCP)
 *   VCC: 5 V, GND: GND
 *
 * UART settings: 115200 baud, 8N1. Open any serial terminal.
 *
 * Modes (select by sending a character):
 *   R - Reference Mode : 10 samples at each of 5 fixed target positions
 *   D - Dynamic Mode   : continuous measurement with motion trend + event logging
 *   S - Single shot    : one distance reading
 */

/* ====== Configuration ====== */
#define REF_POSITIONS        5U
#define REF_SAMPLES_PER_POS  10U
#define SAMPLE_INTERVAL_MS   100U   /* 10 Hz */
#define NEAR_THRESHOLD_CM    20U    /* cm — near event zone */
#define FAR_THRESHOLD_CM     100U   /* cm — far  event zone */
#define DYNAMIC_SAMPLES      200U   /* run 200 samples then return to menu */

/* ====== Forward declarations ====== */
static void run_reference_mode(void);
static void run_dynamic_mode(void);
static void print_banner(void);
static void print_stats(uint32_t *buf, uint32_t n);

/* ====== Main ====== */
int main(void) {
    system_init();
    uart_init();
    hc_sr04_init();

    print_banner();

    while (1) {
        uart_print("\r\n> Send  R  D  or  S  : ");
        char cmd = uart_getchar_wait(0);

        switch (cmd) {
            case 'R': case 'r': run_reference_mode(); break;
            case 'D': case 'd': run_dynamic_mode();   break;
            case 'S': case 's': {
                uint32_t d = hc_sr04_measure_cm_x100();
                uart_print("\r\nDistance: ");
                if (d == 0U) { uart_print("OUT OF RANGE\r\n"); }
                else         { uart_print_cm(d); uart_print(" cm\r\n"); }
                break;
            }
            default: break;
        }
    }
}

/* ====== Reference Mode ====== */
/*
 * Places target at fixed known distances.
 * Records REF_SAMPLES_PER_POS readings per position, prints min/max/avg.
 * Fulfils: "at least 10 reference measurements for each position."
 */
static void run_reference_mode(void) {
    uint32_t samples[REF_SAMPLES_PER_POS];

    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("        REFERENCE MODE\r\n");
    uart_print("========================================\r\n");
    uart_print("Place target at a fixed distance.\r\n");
    uart_print("Press any key to record 10 samples,\r\n");
    uart_print("then move to the next position.\r\n\r\n");

    for (uint32_t pos = 1U; pos <= REF_POSITIONS; pos++) {
        uart_print("Position ");
        uart_print_uint(pos);
        uart_print(" -> Press any key when ready... ");
        uart_getchar_wait(0);
        uart_print("\r\n");

        uint32_t valid = 0U;
        for (uint32_t i = 0U; i < REF_SAMPLES_PER_POS; i++) {
            delay_ms(200U);
            uint32_t d = hc_sr04_measure_cm_x100();
            uart_print("  [");
            uart_print_uint(i + 1U);
            uart_print("] ");
            if (d == 0U) {
                uart_print("TIMEOUT\r\n");
            } else {
                uart_print_cm(d);
                uart_print(" cm\r\n");
                samples[valid++] = d;
            }
        }

        if (valid > 0U) {
            uart_print("  --- Stats ---\r\n");
            print_stats(samples, valid);
        }
        uart_print("\r\n");
    }

    uart_print("Reference mode complete. Returning to menu.\r\n");
}

/* Prints min, max, average for an array of cm_x100 values. */
static void print_stats(uint32_t *buf, uint32_t n) {
    uint32_t sum = 0U, mn = 0xFFFFFFFFU, mx = 0U;
    for (uint32_t i = 0U; i < n; i++) {
        sum += buf[i];
        if (buf[i] < mn) mn = buf[i];
        if (buf[i] > mx) mx = buf[i];
    }
    uint32_t avg = sum / n;

    uart_print("  AVG: "); uart_print_cm(avg); uart_print(" cm\r\n");
    uart_print("  MIN: "); uart_print_cm(mn);  uart_print(" cm\r\n");
    uart_print("  MAX: "); uart_print_cm(mx);  uart_print(" cm\r\n");
    uart_print("  NOISE (max-min): "); uart_print_cm(mx - mn); uart_print(" cm\r\n");
}

/* ====== Dynamic Mode ====== */
/*
 * Continuously measures distance and:
 *   - Logs timestamped samples over UART (CSV-friendly format).
 *   - Classifies each sample as APPROACHING / RETREATING / STATIONARY.
 *   - Counts near events (enter < 20 cm) and far events (enter > 100 cm).
 *   - Computes instantaneous speed and 5-sample sliding-window average speed.
 *   - Runs for DYNAMIC_SAMPLES samples or until 'Q' is received.
 */
static void run_dynamic_mode(void) {
    uint32_t prev_cm_x100 = 0U;
    uint32_t near_events  = 0U;
    uint32_t far_events   = 0U;
    uint8_t  in_near      = 0U;
    uint8_t  in_far       = 0U;

    motion_init();

    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("        DYNAMIC MODE\r\n");
    uart_print("========================================\r\n");
    uart_print("Format: [ts_ms], dist_cm, TREND, spd_cm/s, avg_spd_cm/s, near_ev, far_ev\r\n");
    uart_print("Send 'Q' to stop early.\r\n\r\n");

    for (uint32_t sample = 0U; sample < DYNAMIC_SAMPLES; sample++) {
        /* Non-blocking quit check */
        int8_t ch = uart_getchar_nb();
        if (ch == 'Q' || ch == 'q') break;

        uint32_t ts       = get_tick_ms();
        uint32_t cm_x100  = hc_sr04_measure_cm_x100();

        if (cm_x100 == 0U) {
            uart_print("[");
            uart_print_uint(ts);
            uart_print("] OUT_OF_RANGE\r\n");
            delay_ms(SAMPLE_INTERVAL_MS);
            continue;
        }

        /* Trend and speed (skip first sample — no previous reading) */
        motion_trend_t trend     = STATIONARY;
        uint32_t       spd       = 0U;
        uint32_t       avg_spd   = 0U;

        if (sample > 0U && prev_cm_x100 != 0U) {
            trend   = motion_classify(prev_cm_x100, cm_x100);
            spd     = motion_speed_x100(prev_cm_x100, cm_x100, SAMPLE_INTERVAL_MS);
        }
        motion_update_window(cm_x100, ts);
        avg_spd = motion_avg_speed_x100();

        /* Near / far event detection (edge-triggered) */
        uint8_t is_near = (cm_x100 < (NEAR_THRESHOLD_CM * 100U)) ? 1U : 0U;
        uint8_t is_far  = (cm_x100 > (FAR_THRESHOLD_CM  * 100U)) ? 1U : 0U;
        if (is_near && !in_near) near_events++;
        if (is_far  && !in_far)  far_events++;
        in_near = is_near;
        in_far  = is_far;

        /* Log CSV row */
        uart_putchar('[');
        uart_print_uint(ts);
        uart_print("], ");
        uart_print_cm(cm_x100);
        uart_print(", ");
        uart_print(motion_trend_str(trend));
        uart_print(", ");
        uart_print_cm(spd);
        uart_print(", ");
        uart_print_cm(avg_spd);
        uart_print(", near=");
        uart_print_uint(near_events);
        uart_print(", far=");
        uart_print_uint(far_events);
        uart_print("\r\n");

        prev_cm_x100 = cm_x100;
        delay_ms(SAMPLE_INTERVAL_MS);
    }

    uart_print("\r\n--- Session Summary ---\r\n");
    uart_print("Near events (entered <");
    uart_print_uint(NEAR_THRESHOLD_CM);
    uart_print(" cm zone): ");
    uart_print_uint(near_events);
    uart_print("\r\nFar  events (entered >");
    uart_print_uint(FAR_THRESHOLD_CM);
    uart_print(" cm zone): ");
    uart_print_uint(far_events);
    uart_print("\r\nReturning to menu.\r\n");
}

/* ====== Startup banner ====== */
static void print_banner(void) {
    uart_print("\r\n");
    uart_print("========================================\r\n");
    uart_print("  Distance Measurement & Motion Analyzer\r\n");
    uart_print("  STM32 NUCLEO-F411RE | Bare-Metal\r\n");
    uart_print("  HC-SR04 | GPIO + TIM2 + USART2\r\n");
    uart_print("========================================\r\n");
    uart_print("Wiring:\r\n");
    uart_print("  HC-SR04 TRIG -> PA5\r\n");
    uart_print("  HC-SR04 ECHO -> PA0\r\n");
    uart_print("  UART TX/RX   -> PA2/PA3 (ST-Link VCP)\r\n\r\n");
    uart_print("Commands:\r\n");
    uart_print("  R - Reference Mode (10 samples x 5 positions)\r\n");
    uart_print("  D - Dynamic Mode   (motion trend + events)\r\n");
    uart_print("  S - Single shot measurement\r\n");
}
