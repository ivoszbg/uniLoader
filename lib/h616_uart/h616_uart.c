/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024, halal-beef <78730004+halal-beef@users.noreply.github.com>
 */

#include <lib/h616_uart.h>

#define H6_UART0_BASE		0x05000000
#define UART0_LSR 		(H6_UART0_BASE + 0x14)   /* line status register */
#define UART0_THR		(H6_UART0_BASE + 0x0)    /* transmit holding register */

void putc(char c)
{
	while (!(readl((const volatile void *)UART0_LSR) & (1 << 6))) {}
	*(int*) (UART0_THR) = c;
}

void puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			putc('\r');
		putc(*s++);
	}
}
