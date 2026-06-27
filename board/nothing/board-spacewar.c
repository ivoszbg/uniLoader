// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026 Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info spacewar_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xe1000000
};

static const struct device spacewar_devices[] = {
	{ "simplefb", &spacewar_fb, "fb" },
};

struct board_data board_ops = {
	.name = "nothing-spacewar",
	.ops = {
	},
	.devices = spacewar_devices,
	.num_devices = ARRAY_SIZE(spacewar_devices),
	.quirks = 0
};
