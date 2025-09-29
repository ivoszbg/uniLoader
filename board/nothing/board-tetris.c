/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/debug.h>

#define UART_BASE		0x11001000

#define UART_LSR_BASE	0x14
#define UART_LSR_DR 	0x01 	/* Data ready */
#define UART_LSR_THRE 	0x20 	/* TX holding register empty */

#define WDT_BASE		0x1c00a000
#define WDT_MODE_KEY	0x22000000
#define WDT_MODE_EN		(1 << 0)

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

void tetris_disable_wdt(void)
{
	uint32_t reg;

	reg = readl((void*)WDT_BASE);
	if (reg & WDT_MODE_EN)
	{
		reg &= ~WDT_MODE_EN;
		reg |= WDT_MODE_KEY;
		writel(reg, (void*)WDT_BASE);
		printk(KERN_INFO, "%s: disabled watchdog timer.\n", __func__);
	}
	else
		printk(KERN_ERR, "%s: watchdog is already disabled.\n", __func__);

	return;
}

int tetris_init(void)
{
	return 0;
}

int tetris_drv(void)
{
	return 0;
}

int tetris_late_init(void)
{
#ifdef CONFIG_NOTHING_TETRIS_DISABLE_WDT
	tetris_disable_wdt();
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "cmf-phone-1",
	.ops = {
		.early_init = tetris_init,
		.drivers_init = tetris_drv,
		.late_init = tetris_late_init,
	},
	.quirks = 0
};
