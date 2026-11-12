/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Alexandru Chimac <alex@chimac.ro>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

// Timer
#define SYSTIMER_BASE 0x10200670

// Watchdog
#define TOPRGU_BASE 0x10212000

static struct video_info essential_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 736,
	.height = 1280,
	.stride = 4,
	.address = (void *)0xbf370000
};

static const struct device essential_devices[] = {
	{ "simplefb", &essential_fb, "fb" },
	{ "mediatek,mt6795-systimer", (void *)SYSTIMER_BASE, "timer0" },
	{ "mediatek,wdt", (void *)TOPRGU_BASE, "wdt0" },
};

struct board_data board_ops = {
	.name = "nokia-essential",
	.devices = essential_devices,
	.num_devices = ARRAY_SIZE(essential_devices),
	.quirks = 0
};
