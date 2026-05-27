/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <util.h>
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

static struct video_info tb373fu_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 2944,
	.height = 1840,
	.stride = 4,
	.address = (void *)0xfc16f000
};

static const struct device tb373fu_devices[] = {
	{ "simplefb", &tb373fu_fb, "fb" },
};

int tb373fu_late_init(void)
{
	tb373fu_disable_wdt();

	return 0;
}

struct board_data board_ops = {
	.name = "lenovo-tb373fu",
	.ops = {
		.late_init = tb373fu_late_init,
	},
	.devices = tb373fu_devices,
	.num_devices = ARRAY_SIZE(tb373fu_devices),
	.quirks = 0
};
