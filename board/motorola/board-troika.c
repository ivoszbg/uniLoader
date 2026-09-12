// SPDX-License-Identifier: GPL-2.0
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info troika_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2520,
	.stride = 4,
	.address = (void *)0xec000000,
};

static const struct device troika_devices[] = {
	{ "simplefb", &troika_fb, "fb" },
};

struct board_data board_ops = {
	.name = "motorola-troika",
	.ops = {
	},
	.devices = troika_devices,
	.num_devices = ARRAY_SIZE(troika_devices),
	.quirks = 0
};
