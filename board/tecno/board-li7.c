/* SPDX-License-Identifier: GPL-2.0 */
/* 
 * Copyright (c) Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info li7_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2436,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xfdfff000
};

static const struct device li7_devices[] = {
	{ "simplefb", &li7_fb, "fb" },
};

struct board_data board_ops = {
	.name = "tecno-li7",
	.ops = {
	},
	.devices = li7_devices,
	.num_devices = ARRAY_SIZE(li7_devices),
	.quirks = 0
};
