/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, v1-727 <marokhinmark@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info pine_fb = {
	.format = FB_FORMAT_RGB565,
	.width = 1080,
	.height = 2400,
	.stride = 2,
	.scale = 2,
	.address = (void *)0xb0000000
};

static const struct device pine_devices[] = {
	{ "simplefb", &pine_fb, "fb" },
};

struct board_data board_ops = {
	.name = "zte-pine",
	.ops = {
	},
	.devices = pine_devices,
	.num_devices = ARRAY_SIZE(pine_devices),
	.quirks = 0
};
