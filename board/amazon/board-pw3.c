/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <drivers/framework.h>

#define IMX6SL_UART1_BASE_ADDR	0x02020000
#define URTX0_OFFSET		0x40
#define IMX_UTS_OFFSET		0xB4
#define UTS_TXFULL		(1 << 4)

void uart_putc(char ch)
{
	volatile uint32_t *uts_reg = (volatile
				      uint32_t *)(IMX6SL_UART1_BASE_ADDR +
						  IMX_UTS_OFFSET);
	volatile uint32_t *urtx0_reg = (volatile
					uint32_t *)(IMX6SL_UART1_BASE_ADDR +
						    URTX0_OFFSET);

	while ((*uts_reg) & UTS_TXFULL) { }

	*urtx0_reg = (uint32_t)ch;
}

void uart_puts(const char *s)
{
	while (*s != '\0') {
		uart_putc(*s);
		s++;
	}
}

struct board_data board_ops = {
	.name = "amazon-paperwhite3",
	.quirks = 0
};
