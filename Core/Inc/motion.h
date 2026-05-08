#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>

typedef enum {
    STATIONARY = 0,
    APPROACHING,
    RETREATING
} motion_trend_t;

/*
 * Classify motion based on consecutive distance readings.
 * Threshold: 0.5 cm (50 in cm*100 units) to suppress sensor noise.
 */
void          motion_init(void);
motion_trend_t motion_classify(uint32_t prev_cm_x100, uint32_t curr_cm_x100);
const char   *motion_trend_str(motion_trend_t t);

/* Instantaneous speed in cm/s * 100 between two consecutive samples. */
uint32_t motion_speed_x100(uint32_t prev_cm_x100, uint32_t curr_cm_x100, uint32_t dt_ms);

/* Bonus: sliding-window average speed over last SPEED_WINDOW_SIZE samples. */
void     motion_update_window(uint32_t dist_cm_x100, uint32_t ts_ms);
uint32_t motion_avg_speed_x100(void);

#endif /* MOTION_H */
