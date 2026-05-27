/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>

static struct video_info pdx245_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xd5100000,

	// The scale that is set by default is extremely large.
	.scale = 2
};

static const struct device pdx245_devices[] = {
	{ "simplefb", &pdx245_fb, "fb" },
};

struct board_data board_ops = {
	.name = "sony-pdx245",
	.ops = {
	},
	.devices = pdx245_devices,
	.num_devices = ARRAY_SIZE(pdx245_devices),
	.quirks = 0
};
