/**
 * @file uart.c
 *
 * @brief Interrupt-driven UART primitives
 *
 * @date Feb 9, 2025
 *
 * @author Kevin Zhu (krzhu@andrew.cmu.edu)
 */

#ifndef _UART_H_
#define _UART_H_

void uart_init(int baud);

int uart_put_byte(char c);

int uart_get_byte(char *c);

int uart_write( int file, char *ptr, int len );

int uart_read(int file, char *ptr, int len );

void uart_flush();

#endif /* _UART_H_ */
