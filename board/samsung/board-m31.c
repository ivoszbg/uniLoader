// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026, FelcoX <parbindarsingh@outlook.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info m31_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xca000000
};

static const struct device m31_devices[] = {
	{ "simplefb", &m31_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-m31",
	.devices = m31_devices,
	.num_devices = ARRAY_SIZE(m31_devices),
	.quirks = 0
};
