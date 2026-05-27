/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info tundra_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xe1000000
};

static const struct device tundra_devices[] = {
	{ "simplefb", &tundra_fb, "fb" },
};

struct board_data board_ops = {
	.name = "motorola-tundra",
	.ops = {
	},
	.devices = tundra_devices,
	.num_devices = ARRAY_SIZE(tundra_devices),
	.quirks = 0
};
