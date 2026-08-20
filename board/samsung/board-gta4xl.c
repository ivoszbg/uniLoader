// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025, Alexandru Chimac <alex@chimac.ro>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info gta4xl_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1200,
	.height = 2000,
	.stride = 4,
	.address = (void *)0xca000000
};

static const struct device gta4xl_devices[] = {
	{ "simplefb", &gta4xl_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-gta4xl",
	.devices = gta4xl_devices,
	.num_devices = ARRAY_SIZE(gta4xl_devices),
	.quirks = 0
};
