/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Quentin Bouteiller <contact@quent1.fr>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info gts8wifi_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 2560,
	.height = 1600,
	.stride = 4,
	.rotate = 1,
	.address = (void *)0xb8000000
};

static const struct device gts8wifi_devices[] = {
	{ "simplefb", &gts8wifi_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-gts8wifi",
	.ops = {
	},
	.devices = gts8wifi_devices,
	.num_devices = ARRAY_SIZE(gts8wifi_devices),
	.quirks = 0
};
