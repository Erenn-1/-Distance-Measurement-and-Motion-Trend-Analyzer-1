#include "system.h"
#include "stm32f411xx.h"

/* Incremented by SysTick_Handler every 1 ms */
static volatile uint32_t sys_tick_ms = 0;

void system_init(void) {
    /* Enable GPIOA clock (AHB1) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    /* Enable TIM2 and USART2 clocks (APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_USART2EN;

    /*
     * SysTick: 1 ms period.
     * HSI = 16 MHz (default after reset, no PLL).
     * RELOAD = 16 000 000 / 1000 - 1 = 15 999.
     */
    SysTick->LOAD = 15999U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE;

    /*
     * TIM2: 1 MHz free-running 32-bit counter.
     * PSC = 15  ->  16 MHz / 16 = 1 MHz  ->  1 tick = 1 µs.
     * ARR = 0xFFFFFFFF (full 32-bit range, ~71 minutes before wrap).
     */
    TIM2->PSC = 15U;
    TIM2->ARR = 0xFFFFFFFFU;
    TIM2->CNT = 0U;
    TIM2->CR1 = TIM_CR1_CEN;
}

void SysTick_Handler(void) {
    sys_tick_ms++;
}

uint32_t get_tick_ms(void) {
    return sys_tick_ms;
}

void delay_ms(uint32_t ms) {
    uint32_t start = get_tick_ms();
    while ((get_tick_ms() - start) < ms);
}

/* Uses TIM2 1 MHz counter for microsecond-accurate delay. */
void delay_us(uint32_t us) {
    uint32_t start = TIM2->CNT;
    while ((TIM2->CNT - start) < us);
}
