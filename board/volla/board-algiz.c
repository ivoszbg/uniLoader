/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info algiz_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.address = (void *)0x7d010000
};

static const struct device algiz_devices[] = {
	{ "simplefb", &algiz_fb, "fb" },
};

struct board_data board_ops = {
	.name = "volla-quintus",
	.ops = {
	},
	.devices = algiz_devices,
	.num_devices = ARRAY_SIZE(algiz_devices),
	.quirks = 0
};
