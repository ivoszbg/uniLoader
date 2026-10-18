/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, R0rt1z2 <roger@r0rt1z2.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info sunstone_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 2000,
	.height = 1216,
	.stride = 4,
	.scale = 2,
	.rotate = 1,
	.address = (void *)0x796c0000,
};

static const struct device sunstone_devices[] = {
	{ "simplefb", &sunstone_fb, "fb" },
};

struct board_data board_ops = {
	.name = "amazon-sunstone",
	.devices = sunstone_devices,
	.num_devices = ARRAY_SIZE(sunstone_devices),
	.quirks = 0,
};
