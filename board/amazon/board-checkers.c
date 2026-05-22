/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026, Akari Tsuyukusa <akkun11.open@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info checkers_fb = {
	.format = FB_FORMAT_ABGR8888,
	.width = 480,
	.height = 960,
	.stride = 4,
	.address = (void *)0x5f900000,
	.scale = 1,
};


static const struct device checkers_devices[] = {
	{ "simplefb", &checkers_fb, "fb" },
};

struct board_data board_ops = {
	.name = "amazon-checkers",
	.ops = {},
	.devices = checkers_devices,
	.num_devices = ARRAY_SIZE(checkers_devices),
	.quirks = 0,
};
