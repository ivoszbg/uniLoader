/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Kaloyan Donev <donevkaloyan@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define TOPRGU_BASE 0x10212000

static struct video_info p1ma40_fb = {
	.format = FB_FORMAT_ABGR8888,
	.width = 736,
	.height = 1280,
	.stride = 4,
	.address = (void *)0xbf330000
};

static const struct device p1ma40_devices[] = {
	{ "mediatek,wdt", (void *)TOPRGU_BASE, "wdt0" },
	{ "simplefb", &p1ma40_fb, "fb" },
};

struct board_data board_ops = {
	.name = "lenovo-p1ma40",
	.devices = p1ma40_devices,
	.num_devices = ARRAY_SIZE(p1ma40_devices),
	.quirks = 0
};
