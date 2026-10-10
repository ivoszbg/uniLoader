// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026 Dimitris Katsanos <deekatsanos@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info a14m_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2408,
	.stride = 4,
	.scale = 2,
	.address = (void *)0x782c0000
};

static const struct device a14m_devices[] = {
	{ "simplefb", &a14m_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a14m",
	.devices = a14m_devices,
	.num_devices = ARRAY_SIZE(a14m_devices),
	.quirks = 0
};
