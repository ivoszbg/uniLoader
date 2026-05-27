/* SPDX-License-Identifier: GPL-2.0 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info r0q_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xb8000000
};

static const struct device r0q_devices[] = {
	{ "simplefb", &r0q_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-r0q",
	.ops = {
	},
	.devices = r0q_devices,
	.num_devices = ARRAY_SIZE(r0q_devices),
	.quirks = 0
};
