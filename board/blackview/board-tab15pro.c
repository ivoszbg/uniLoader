/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026, Akari Tsuyukusa <akkun11.open@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info tab15pro_fb = {
	.format = FB_FORMAT_RGB565,
	.width = 1200,
	.height = 1920,
	.stride = 2,
	.address = (void *)0x9e000000,
	.scale = 2,
};

static const struct device tab15pro_devices[] = {
	{ "simplefb", &tab15pro_fb, "fb" },
};

struct board_data board_ops = {
	.name = "blackview-tab15pro",
	.ops = {
	},
	.devices = tab15pro_devices,
	.num_devices = ARRAY_SIZE(tab15pro_devices),
	.quirks = 0
};
