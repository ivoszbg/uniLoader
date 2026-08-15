/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, tuxmanxd <tuxkun09@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info v36bml_fb = {
	.format = FB_FORMAT_ABGR8888,
	.width = 736,
	.height = 1280,
	.stride = 4,
	.address = (void *)0xfddc0000
};

static const struct device v36bml_devices[] = {
	{ "simplefb", &v36bml_fb, "fb" },
};

struct board_data board_ops = {
	.name = "htc-v36bml_dugl",
	.ops = {
	},
	.devices = v36bml_devices,
	.num_devices = ARRAY_SIZE(v36bml_devices),
	.quirks = 0
};
