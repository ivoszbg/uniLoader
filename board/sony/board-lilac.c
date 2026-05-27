/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info lilac_fb = {
	.format = FB_FORMAT_RGB888,
	.width = 720,
	.height = 1280,
	.stride = 3,
	.address = (void *)0x9d400000
};

static const struct device lilac_devices[] = {
	{ "simplefb", &lilac_fb, "fb" },
};

struct board_data board_ops = {
	.name = "sony-lilac",
	.ops = {
	},
	.devices = lilac_devices,
	.num_devices = ARRAY_SIZE(lilac_devices),
	.quirks = 0
};
