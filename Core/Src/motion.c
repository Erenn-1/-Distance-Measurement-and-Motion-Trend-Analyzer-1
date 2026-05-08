#include "motion.h"

#define MOTION_THRESHOLD_CM_X100  50U   /* 0.5 cm — filters sensor noise */
#define SPEED_WINDOW_SIZE         5U    /* Bonus: sliding-window depth */

typedef struct {
    uint32_t dist_cm_x100;
    uint32_t ts_ms;
} speed_sample_t;

static speed_sample_t speed_buf[SPEED_WINDOW_SIZE];
static uint8_t speed_idx;
static uint8_t speed_full;

void motion_init(void) {
    speed_idx  = 0;
    speed_full = 0;
}

motion_trend_t motion_classify(uint32_t prev_cm_x100, uint32_t curr_cm_x100) {
    int32_t delta = (int32_t)curr_cm_x100 - (int32_t)prev_cm_x100;
    if (delta < -(int32_t)MOTION_THRESHOLD_CM_X100) return APPROACHING;
    if (delta >  (int32_t)MOTION_THRESHOLD_CM_X100) return RETREATING;
    return STATIONARY;
}

const char *motion_trend_str(motion_trend_t t) {
    switch (t) {
        case APPROACHING: return "APPROACHING";
        case RETREATING:  return "RETREATING ";
        default:          return "STATIONARY ";
    }
}

uint32_t motion_speed_x100(uint32_t prev_cm_x100, uint32_t curr_cm_x100, uint32_t dt_ms) {
    if (dt_ms == 0U) return 0U;
    int32_t delta = (int32_t)curr_cm_x100 - (int32_t)prev_cm_x100;
    uint32_t abs_delta = (delta < 0) ? (uint32_t)(-delta) : (uint32_t)delta;
    /* speed_x100 (cm/s*100) = abs_delta_cm_x100 * 1000 / dt_ms */
    return (abs_delta * 1000U) / dt_ms;
}

/* --- Bonus: Sliding-window average speed --- */

void motion_update_window(uint32_t dist_cm_x100, uint32_t ts_ms) {
    speed_buf[speed_idx].dist_cm_x100 = dist_cm_x100;
    speed_buf[speed_idx].ts_ms        = ts_ms;
    speed_idx = (uint8_t)((speed_idx + 1U) % SPEED_WINDOW_SIZE);
    if (speed_idx == 0U) speed_full = 1;
}

uint32_t motion_avg_speed_x100(void) {
    uint8_t n = speed_full ? SPEED_WINDOW_SIZE : speed_idx;
    if (n < 2U) return 0U;

    uint8_t oldest = speed_full ? speed_idx : 0U;

    uint32_t total_dist = 0U;
    for (uint8_t i = 0U; i < (n - 1U); i++) {
        uint8_t a = (uint8_t)((oldest + i)       % SPEED_WINDOW_SIZE);
        uint8_t b = (uint8_t)((oldest + i + 1U)  % SPEED_WINDOW_SIZE);
        int32_t d = (int32_t)speed_buf[b].dist_cm_x100 - (int32_t)speed_buf[a].dist_cm_x100;
        total_dist += (d < 0) ? (uint32_t)(-d) : (uint32_t)d;
    }

    uint8_t newest = (speed_idx == 0U) ? (SPEED_WINDOW_SIZE - 1U) : (speed_idx - 1U);
    uint32_t dt_ms = speed_buf[newest].ts_ms - speed_buf[oldest].ts_ms;
    if (dt_ms == 0U) return 0U;

    /* avg_speed_x100 = total_dist_cm_x100 * 1000 / dt_ms */
    return (total_dist * 1000U) / dt_ms;
}
