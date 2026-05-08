#include "hc_sr04.h"
#include "stm32f411xx.h"
#include "system.h"

/*
 * PA5 = TRIG output
 * PA0 = ECHO input  (5V-tolerant pin on STM32F411)
 *
 * Trigger: 10 µs pulse generated using TIM2 counter (bare-metal, no HAL).
 * Echo:    TIM2 CNT sampled at rising and falling edges of ECHO (polling).
 *
 * Max valid echo = ~38 000 µs (HC-SR04 timeout when no object detected).
 * We use 40 000 µs as our software timeout.
 */

#define TRIG_PIN        5U
#define ECHO_PIN        0U
#define ECHO_TIMEOUT_US 40000U

void hc_sr04_init(void) {
    /* PA5: output push-pull, low speed */
    GPIOA->MODER  &= ~(3U << (TRIG_PIN * 2));
    GPIOA->MODER  |=  (1U << (TRIG_PIN * 2));   /* Output */
    GPIOA->OTYPER &= ~(1U << TRIG_PIN);          /* Push-pull */
    GPIOA->BSRR    =  (1U << (TRIG_PIN + 16U)); /* Start LOW */

    /* PA0: input, no pull (external signal drives it) */
    GPIOA->MODER &= ~(3U << (ECHO_PIN * 2));     /* Input (00) */
    GPIOA->PUPDR &= ~(3U << (ECHO_PIN * 2));     /* No pull */
}

uint32_t hc_sr04_measure_us(void) {
    uint32_t t_start, t_end, t_wait;

    /* --- Generate 10 µs TRIG pulse (bare-metal timer, no HAL) --- */
    GPIOA->BSRR = (1U << TRIG_PIN);              /* TRIG = HIGH */
    delay_us(10U);
    GPIOA->BSRR = (1U << (TRIG_PIN + 16U));      /* TRIG = LOW  */

    /* --- Wait for ECHO to go HIGH (rising edge) --- */
    t_wait = TIM2->CNT;
    while (!(GPIOA->IDR & (1U << ECHO_PIN))) {
        if ((TIM2->CNT - t_wait) > ECHO_TIMEOUT_US) return 0U; /* No echo */
    }
    t_start = TIM2->CNT;

    /* --- Wait for ECHO to go LOW (falling edge) --- */
    while (GPIOA->IDR & (1U << ECHO_PIN)) {
        if ((TIM2->CNT - t_start) > ECHO_TIMEOUT_US) return 0U; /* Timeout */
    }
    t_end = TIM2->CNT;

    return t_end - t_start; /* Microseconds */
}

uint32_t hc_sr04_measure_cm_x100(void) {
    uint32_t us = hc_sr04_measure_us();
    if (us == 0U) return 0U;

    /*
     * Distance (cm) = echo_us * speed_of_sound / 2
     *               = echo_us * 34300 cm/s / 2 / 1 000 000 µs/s
     *               = echo_us * 343 / 20000
     *
     * Distance * 100 (cm_x100) = echo_us * 343 / 200
     *
     * Overflow check: max echo_us ~38000, 38000*343 = 13 034 000 < 2^32. OK.
     */
    return (us * 343U) / 200U;
}
