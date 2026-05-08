#include "uart.h"
#include "stm32f411xx.h"
#include "system.h"

void uart_init(void) {
    /*
     * PA2 = USART2_TX (AF7)
     * PA3 = USART2_RX (AF7)
     */

    /* Set PA2 and PA3 to Alternate Function mode (MODER = 10b) */
    GPIOA->MODER &= ~((3U << (2*2)) | (3U << (3*2)));
    GPIOA->MODER |=  ((2U << (2*2)) | (2U << (3*2)));

    /* Set AF7 for PA2 and PA3 in AFR[0] (pins 0-7) */
    GPIOA->AFR[0] &= ~((0xFU << (2*4)) | (0xFU << (3*4)));
    GPIOA->AFR[0] |=  ((7U   << (2*4)) | (7U   << (3*4)));

    /* Medium speed for UART pins */
    GPIOA->OSPEEDR |= ((2U << (2*2)) | (2U << (3*2)));

    /*
     * BRR for 115200 baud @ fPCLK1 = 16 MHz (HSI, no PLL):
     *   USARTDIV = 16 000 000 / (16 * 115200) = 8.6805...
     *   Mantissa = 8, Fraction = round(0.6805 * 16) = 11
     *   BRR = (8 << 4) | 11 = 0x008B
     */
    USART2->BRR = 0x008BU;

    /* Enable TX, RX, and USART */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_putchar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint32_t)(uint8_t)c;
}

void uart_print(const char *s) {
    while (*s) uart_putchar(*s++);
}

static void print_digits(uint32_t n) {
    if (n >= 10U) print_digits(n / 10U);
    uart_putchar((char)('0' + (n % 10U)));
}

void uart_print_uint(uint32_t n) {
    print_digits(n);
}

void uart_print_int(int32_t n) {
    if (n < 0) { uart_putchar('-'); n = -n; }
    print_digits((uint32_t)n);
}

void uart_print_cm(uint32_t cm_x100) {
    uint32_t whole = cm_x100 / 100U;
    uint32_t frac  = cm_x100 % 100U;
    print_digits(whole);
    uart_putchar('.');
    if (frac < 10U) uart_putchar('0');
    print_digits(frac);
}

int8_t uart_getchar_nb(void) {
    if (USART2->SR & USART_SR_RXNE)
        return (int8_t)(USART2->DR & 0xFFU);
    return -1;
}

char uart_getchar_wait(uint32_t timeout_ms) {
    if (timeout_ms == 0U) {
        while (!(USART2->SR & USART_SR_RXNE));
        return (char)(USART2->DR & 0xFFU);
    }
    uint32_t start = get_tick_ms();
    while (!(USART2->SR & USART_SR_RXNE)) {
        if ((get_tick_ms() - start) >= timeout_ms) return '\0';
    }
    return (char)(USART2->DR & 0xFFU);
}
