/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info daisy_fb = {
	.format = FB_FORMAT_RGB888,
	.width = 1080,
	.height = 2280,
	.scale = 2,
	.stride = 3,
	.address = (void *)0x90001000,
};

static const struct device daisy_devices[] = {
	{ "simplefb", &daisy_fb, "fb" },
};

struct board_data board_ops = {
	.name = "xiaomi-daisy",
	.devices = daisy_devices,
	.num_devices = ARRAY_SIZE(daisy_devices),
	.quirks = 0
};
