/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_EARLYCON
volatile unsigned int* uart = (unsigned int*)0x9000000;

void uart_putc(char ch)
{
	*uart = ch;
}

void uart_puts(const char *s)
{
	while (*s != '\0') 
	{
		uart_putc(*s);
		s++;
	}
}
#endif

struct board_data board_ops = {
	.name = "qemu-virt",
	.quirks = 0
};
