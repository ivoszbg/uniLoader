/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, cgik <idkzzz50@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info on7xelte_fb = {
	.format = FB_FORMAT_ABGR8888,
	.width = 1080,
	.height = 1920,
	.stride = 4,
	.address = (void *)0x67000000
};

static const struct device on7xelte_devices[] = {
	{ "simplefb", &on7xelte_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-on7xelte",
	.devices = on7xelte_devices,
	.num_devices = ARRAY_SIZE(on7xelte_devices),
	.quirks = 0
};
