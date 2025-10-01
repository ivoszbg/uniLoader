/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Alexandru Chimac <alex@chimac.ro>
 */
#include <board.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define TOPRGU_BASE 0x10212000
#define WDT_MODE_KEY 0x22000000
#define WDT_MODE_EN (1 << 0)

// Early initialization
int essential_init(void)
{
	unsigned long wdg_reg;

	// Disable watchdog
	wdg_reg = readl((volatile unsigned int *) TOPRGU_BASE);
	wdg_reg &= ~WDT_MODE_EN;
	wdg_reg |= WDT_MODE_KEY;
	writel(wdg_reg, (void *) TOPRGU_BASE);

	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info essential_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 736,
	.height = 1280,
	.stride = 4,
	.address = (void *)0xbf370000
};
#endif

int essential_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &essential_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "nokia-essential",
	.ops = {
		.early_init = essential_init,
		.drivers_init = essential_drv,
	},
	.quirks = 0
};
