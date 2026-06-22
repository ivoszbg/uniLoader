/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>

static struct video_info nezha_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1200,
	.height = 2608,
	.stride = 4,
	.address = (void *)0xfc800000,

	// The scale that is set by default is extremely large.
	.scale = 2
};

static const struct device nezha_devices[] = {
	{ "simplefb", &nezha_fb, "fb" },
};

struct board_data board_ops = {
	.name = "xiaomi-nezha",
	.ops = {
	},
	.devices = nezha_devices,
	.num_devices = ARRAY_SIZE(nezha_devices),
	.quirks = 0
};
