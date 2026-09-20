// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026, Oleksii Onchul <oleksiionchul@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info froggerpro_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1260,
	.height = 2800,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xfc800000
};

static const struct device froggerpro_devices[] = {
	{ "simplefb", &froggerpro_fb, "fb" },
};

struct board_data board_ops = {
	.name = "nothing-froggerpro",
	.devices = froggerpro_devices,
	.num_devices = ARRAY_SIZE(froggerpro_devices),
	.quirks = 0
};
