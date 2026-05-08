#ifndef UART_H
#define UART_H

#include <stdint.h>

/*
 * USART2 @ 115200 8N1, bare-metal (no HAL).
 *   TX -> PA2 (AF7)  connected to ST-Link VCP
 *   RX -> PA3 (AF7)  connected to ST-Link VCP
 */

void  uart_init(void);
void  uart_putchar(char c);
void  uart_print(const char *s);
void  uart_print_uint(uint32_t n);
void  uart_print_int(int32_t n);

/* Prints value as "XX.XX" (e.g. 2347 -> "23.47") */
void  uart_print_cm(uint32_t cm_x100);

/* Returns received char, -1 if RX buffer empty. */
int8_t uart_getchar_nb(void);

/* Blocks until a character arrives. Pass timeout_ms=0 to wait forever. */
char  uart_getchar_wait(uint32_t timeout_ms);

#endif /* UART_H */
