// SPDX-License-Identifier: GPL-2.0
/*
 * Watchdog "driver" for most aarch64 MediaTek Platforms
 * Copyright (c) 2026, Igor Belwon <botchedrpr@lineageos.org>
 *
 * Based on https://github.com/torvalds/linux/blob/master/drivers/watchdog/mtk_wdt.c
 * Copyright (C) 2014 Matthias Brugger
 */

#include <drivers/framework.h>
#include <lib/debug.h>
#include <string.h>

#define WDT_MODE_KEY		0x22000000
#define WDT_MODE_EN		(1 << 0)

static int mtk_wdt_init(void *data)
{
	volatile unsigned int* base;
	uint32_t reg;

	if (!data)
		return -1;

#ifndef CONFIG_WDT_DISABLE_ON_BOOT
	printk(KERN_INFO, "%s: probed, not disabling watchdog.\n", __func__);
	return 0;
#endif

	base = (volatile unsigned int *)data;

	reg = readl((void*)base);
	if (reg & WDT_MODE_EN)
	{
		reg &= ~WDT_MODE_EN;
		reg |= WDT_MODE_KEY;
		writel(reg, (void*)base);
		printk(KERN_INFO, "%s: disabled watchdog timer.\n", __func__);
	}
	else
		printk(KERN_ERR, "%s: watchdog is already disabled.\n", __func__);

	return 0;
}

DRIVER_REGISTER("mediatek,wdt", mtk_wdt_init);
