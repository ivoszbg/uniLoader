/* SPDX-License-Identifier: GPL-2.0 */
/* 
 * Copyright (c) xewvvi
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info li6_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2436,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xfdfff000
};

static const struct device li6_devices[] = {
	{ "simplefb", &li6_fb, "fb" },
};

struct board_data board_ops = {
	.name = "tecno-li6",
	.ops = {
	},
	.devices = li6_devices,
	.num_devices = ARRAY_SIZE(li6_devices),
	.quirks = 0
};
