// SPDX-License-Identifier: GPL-2.0
/*
 * Watchdog "driver" for ARM SP805 Watchdog block
 *
 * Copied from https://github.com/ivoszbg/uniLoader/blob/master/board/huawei/board-agassi2.c
 * Copyright (c) 2026, Tildeguy <tildeguy@mainlining.org>
 */

#include <drivers/framework.h>
#include <lib/debug.h>

#define SP805_WDTCONTROL      0x008U
#define SP805_WDTLOCK         0xC00U

#define SP805_WDT_UNLOCK_KEY  0x1ACCE551U
#define SP805_WDT_LOCK_KEY    0x00000001U

static int sp805_wdt_init(void *data)
{
	volatile unsigned int* base;

	if (!data)
		return -1;

#ifndef CONFIG_WDT_DISABLE_ON_BOOT
	printk(KERN_INFO, "%s: probed, not disabling watchdog.\n", __func__);
	return 0;
#endif

	base = (volatile unsigned int *)data;

	volatile unsigned int *wdt_control =
		(volatile unsigned int *)(base + SP805_WDTCONTROL);
	volatile unsigned int *wdt_lock =
		(volatile unsigned int *)(base + SP805_WDTLOCK);

	*wdt_lock = SP805_WDT_UNLOCK_KEY;
	*wdt_control = 0;
	*wdt_lock = SP805_WDT_LOCK_KEY;
	(void)*wdt_lock;

	return 0;
}

DRIVER_REGISTER("arm,sp805-wdt", sp805_wdt_init);
