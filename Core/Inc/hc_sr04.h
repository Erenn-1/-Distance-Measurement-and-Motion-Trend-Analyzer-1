#ifndef HC_SR04_H
#define HC_SR04_H

#include <stdint.h>

/*
 * Hardware connections (bare-metal, no HAL):
 *   TRIG  ->  PA5  (GPIO output)
 *   ECHO  ->  PA0  (GPIO input, 5V-tolerant on STM32F411)
 *
 * Trigger pulse: 10 µs HIGH generated via TIM2 counter (no HAL delay).
 * Echo measurement: TIM2 CNT captured at echo rising and falling edges.
 */

void     hc_sr04_init(void);

/* Returns echo pulse width in microseconds. Returns 0 on timeout (no object). */
uint32_t hc_sr04_measure_us(void);

/* Returns distance in cm * 100 (e.g. 2347 = 23.47 cm). Returns 0 on timeout. */
uint32_t hc_sr04_measure_cm_x100(void);

#endif /* HC_SR04_H */
