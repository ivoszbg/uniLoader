/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/debug.h>

#define UART_BASE		0x11001000

#define UART_LSR_BASE		0x14
#define UART_LSR_DR 		0x01 /* Data ready */
#define UART_LSR_THRE 		0x20 /* TX holding register empty */

#define WDT_BASE		0x1c00a000

#ifdef CONFIG_EARLYCON
void uart_putc(char ch)
{
	/* wait until TX is ready */
	while (!(readl((void *)(UART_BASE + UART_LSR_BASE)) & UART_LSR_THRE))
		;

	writel(ch, (void *)UART_BASE); // thr is at offset 0x0, which is just uart_base
}

void uart_puts(const char *s)
{
	while (*s != '\0') {
		uart_putc(*s);
		s++;
	}
}
#endif

static const struct device tetris_devices[] = {
	{ "mediatek,wdt", (void *)WDT_BASE, "wdt0" },
};

struct board_data board_ops = {
	.name = "cmf-phone-1",
	.devices = tetris_devices,
	.quirks = 0
};
