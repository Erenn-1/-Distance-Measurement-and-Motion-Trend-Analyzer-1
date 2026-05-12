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
    TIM2->PSC = 15U;    /*PSC + 1 = 15 + 1 = 16*/
    TIM2->ARR = 0xFFFFFFFFU;      /*0 → 1 → 2 → ... → 4.294.967.295 → 0 (71 dk sonra)*/
    TIM2->EGR = TIM_EGR_UG;     /* UG yazıldıktan sonra → Donanım PSC=15 (1 MHz) ile çalışır */
    TIM2->SR  = 0U;     /* UG biti yazılınca donanım otomatik olarak UIF (Update Interrupt Flag) bayrağını set eder. */
    TIM2->CNT = 0U;     /*Sayacı sıfırdan başlatır. Yazma anında CNT = 0 olur, TIM2 temiz başlar.*/
    TIM2->CR1 = TIM_CR1_CEN;    /*CEN (Counter Enable) biti. Bu satırla sayaç fiilen çalışmaya başlar.*/
}
/*Her 1 ms'de bir donanım bu fonksiyonu otomatik çağırır. 
Başka bir şey yapmaz, sadece global sayacı 1 artırır.
volatile olarak tanımlandığı için derleyici bu değişkeni optimize edip bellekten kaldıramaz.*/
void SysTick_Handler(void) {
    sys_tick_ms++;
}

/*sys_tick_ms değerini okur ve döndürür. 
Doğrudan global değişkene erişmek yerine fonksiyon üzerinden okunması, 
gelecekte atomik okuma gibi korumalar eklemek için kapı açar.*/
uint32_t get_tick_ms(void) {
    return sys_tick_ms;
}

/*Meşgul bekleme (busy-wait) yöntemiyle milisaniye gecikme.*/
void delay_ms(uint32_t ms) {
    uint32_t start = get_tick_ms();
    while ((get_tick_ms() - start) < ms);
}

/* TIM2 1 MHz'de çalıştığı için her CNT artışı tam olarak 1 µs'e karşılık gelir. */
void delay_us(uint32_t us) {
    uint32_t start = TIM2->CNT;
    while ((TIM2->CNT - start) < us);
}
