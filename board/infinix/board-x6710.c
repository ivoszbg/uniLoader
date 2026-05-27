/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info x6710_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2400,
	.stride = 4,
	.address = (void *)0x76a10000
};

static const struct device x6710_devices[] = {
	{ "simplefb", &x6710_fb, "fb" },
};

struct board_data board_ops = {
	.name = "infinix-x6710",
	.devices = x6710_devices,
	.num_devices = ARRAY_SIZE(x6710_devices),
	.quirks = 0
};
