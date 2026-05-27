/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>

#define WDT_BASE		0x1c00a000
#define WDT_MODE_KEY	0x22000000
#define WDT_MODE_EN		(1 << 0)

void tb373fu_disable_wdt(void)
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

#ifdef CONFIG_SIMPLE_FB
static struct video_info tb373fu_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 2944,
	.height = 1840,
	.stride = 4,
	.address = (void *)0xfc16f000
};
#endif

int tb373fu_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &tb373fu_fb);
#endif
	return 0;
}

int tb373fu_late_init(void)
{
	tb373fu_disable_wdt();

	return 0;
}

struct board_data board_ops = {
	.name = "lenovo-tb373fu",
	.ops = {
		.drivers_init = tb373fu_drv,
		.late_init = tb373fu_late_init,
	},
	.quirks = 0
};
