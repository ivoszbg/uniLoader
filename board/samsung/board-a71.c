/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info a71_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.scale = 2,
	.address = (void *)0x9c000000
};

static const struct device a71_devices[] = {
	{ "simplefb", &a71_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a71",
	.ops = {
	},
	.devices = a71_devices,
	.num_devices = ARRAY_SIZE(a71_devices),
	.quirks = 0
};
