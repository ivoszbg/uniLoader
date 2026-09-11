/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026, Vasiliy Borisov <vasy@vasy.ru>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info t40pro_fb = {
	.format = FB_FORMAT_RGB565,
	.width = 1200,
	.height = 2000,
	.stride = 2,
	.address = (void *)0x9e000000,
	.scale = 2,
};

static const struct device t40pro_devices[] = {
       { "simplefb", &t40pro_fb, "fb" },
};

struct board_data board_ops = {
	.name = "teclast-t40pro",
	.ops = {
	},
	.devices = t40pro_devices,
	.num_devices = ARRAY_SIZE(t40pro_devices),
	.quirks = 0
};
