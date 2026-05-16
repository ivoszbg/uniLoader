/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Board driver for Huawei Mediapad T5
 *
 * Copyright (c) 2026, Tildeguy <tildeguy@mainlining.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define SP805_WDT_BASE        0xE8A06000U

#define SP805_WDTCONTROL      0x008U
#define SP805_WDTLOCK         0xC00U

#define SP805_WDT_UNLOCK_KEY  0x1ACCE551U
#define SP805_WDT_LOCK_KEY    0x00000001U

void agassi2_disable_wdt(void)
{
	volatile unsigned int *wdt_control =
		(volatile unsigned int *)(SP805_WDT_BASE + SP805_WDTCONTROL);
	volatile unsigned int *wdt_lock =
		(volatile unsigned int *)(SP805_WDT_BASE + SP805_WDTLOCK);

	*wdt_lock = SP805_WDT_UNLOCK_KEY;
	*wdt_control = 0;
	*wdt_lock = SP805_WDT_LOCK_KEY;
	(void)*wdt_lock;
}

int agassi2_early_init(void)
{
	unsigned long cpacr;
	__asm__ volatile ("mrs %0, cpacr_el1" : "=r" (cpacr));
	cpacr |= (3UL << 20);
	__asm__ volatile ("msr cpacr_el1, %0" :: "r" (cpacr));
	return 0;
}

int agassi2_late_init(void)
{
#ifdef CONFIG_HUAWEI_AGASSI2_DISABLE_WDT
	agassi2_disable_wdt();
#endif
	return 0;
}

static struct video_info agassi2_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1200,
	.height = 1920,
	.stride = 4,
	.address = (void *)0x5d000000,
	.scale = 2
};

static const struct device agassi2_devices[] = {
	{ "simplefb", &agassi2_fb, "fb" },
};

struct board_data board_ops = {
	.name = "huawei-agassi2",
	.ops = {
		.early_init = agassi2_early_init,
		.late_init = agassi2_late_init,
	},
	.devices = agassi2_devices,
	.num_devices = ARRAY_SIZE(agassi2_devices),
	.quirks = 0
};
