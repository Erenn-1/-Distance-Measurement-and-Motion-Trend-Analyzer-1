#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

/*
 * system_init: Enable clocks, configure SysTick (1 ms), start TIM2 (1 MHz).
 * Must be called before any other module.
 */
void system_init(void);
uint32_t get_tick_ms(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);  /* Uses TIM2 CNT — call after system_init */

#endif /* SYSTEM_H */
