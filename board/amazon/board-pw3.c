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

void init_board_funcs(void *board)
{
	/*
	 * Parsing the struct directly without restructing is
	 * broken as of Sep 29 2024
	 */
	struct {
		const char *name;
		int ops[BOARD_OP_EXIT];
	} *board_restruct = board;

	board_restruct->name = "PW3";
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}

int board_driver_setup(void)
{	
	return 0;
}
