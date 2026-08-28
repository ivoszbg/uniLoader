/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, kmiit <kmiit@foxmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info shennong_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 3200,
	.stride = 4,
	.address = (void *)0xfc400000,

	// The scale that is set by default is extremely large.
	.scale = 2
};

static const struct device shennong_devices[] = {
	{ "simplefb", &shennong_fb, "fb" },
};

struct board_data board_ops = {
	.name = "xiaomi-shennong",
	.devices = shennong_devices,
	.num_devices = ARRAY_SIZE(shennong_devices),
	.quirks = 0
};
