/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Use the fw_camellian_miui_CAMELLIANGlobal_V14_0_6_0_TKSMIXM_65527e11f3_13
 *
 * Copyright (c) Ryo "evilMyQueen" Yamada <vernamjap@gmail.com>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

#define UART_BASE 0x11002000
#define UART_LSR_BASE	0x14
#define UART_LSR_DR	0x01	/* Data ready */
#define UART_LSR_THRE	0x20	/* TX holding register empty */

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

static struct video_info camellia_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2400,
	.stride = 4,
	.address = (void *)0x7ba10000
};

int camellia_drv(void)
{
	REGISTER_DRIVER("simplefb", simplefb_probe, &camellia_fb);
	return 0;
}

struct board_data board_ops = {
	.name = "xiaomi-camellia",
	.ops = {
		.drivers_init = camellia_drv,
	},
	.quirks = 0
};
