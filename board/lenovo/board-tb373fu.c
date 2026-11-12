/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define WDT_BASE		0x1c00a000

static struct video_info tb373fu_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 2944,
	.height = 1840,
	.stride = 4,
	.address = (void *)0xfc16f000
};

static const struct device tb373fu_devices[] = {
	{ "mediatek,wdt", (void *)WDT_BASE, "wdt0" },
	{ "simplefb", &tb373fu_fb, "fb" },
};

struct board_data board_ops = {
	.name = "lenovo-tb373fu",
	.devices = tb373fu_devices,
	.num_devices = ARRAY_SIZE(tb373fu_devices),
	.quirks = 0
};
