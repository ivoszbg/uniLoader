// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026 Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 */

#include <board.h>
#include <util.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/debug.h>

#define UART_BASE 0x11002000
#define UART_LSR_BASE	0x14
#define UART_LSR_DR 	0x01 	/* Data ready */
#define UART_LSR_THRE 	0x20 	/* TX holding register empty */

#ifdef CONFIG_EARLYCON
void uart_putc(char ch)
{
	while (!(readl((void *)(UART_BASE + UART_LSR_BASE)) & UART_LSR_THRE))
		;

	writel(ch, (void *)UART_BASE);
}

void uart_puts(const char *s)
{
	while (*s != '\0') {
		uart_putc(*s);
		s++;
	}
}
#endif

struct board_data board_ops = {
	.name = "xiaomi-moon",
	.devices = NULL,
	.num_devices = 0,
	.quirks = 0
};
