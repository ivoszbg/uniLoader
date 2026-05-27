/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info rmx3511_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2408,
	.stride = 4,
	.address = (void *)0x9e000000
};

static const struct device rmx3511_devices[] = {
	{ "simplefb", &rmx3511_fb, "fb" },
};

struct board_data board_ops = {
	.name = "realme-rmx3511",
	.ops = {
	},
	.devices = rmx3511_devices,
	.num_devices = ARRAY_SIZE(rmx3511_devices),
	.quirks = 0
};
