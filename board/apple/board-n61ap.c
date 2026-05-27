/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info n61ap_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 752,
	.height = 1334,
	.stride = 4,
	.address = (void *)0x83e900000
};

static const struct device n61ap_devices[] = {
	{ "simplefb", &n61ap_fb, "fb" },
};

struct board_data board_ops = {
	.name = "apple-iphone6",
	.ops = {
	},
	.devices = n61ap_devices,
	.num_devices = ARRAY_SIZE(n61ap_devices),
	.quirks = 0
};
