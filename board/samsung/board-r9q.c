/* SPDX-License-Identifier: GPL-2.0 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info r9q_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xe4000000,
	.scale = 2
};

static const struct device r9q_devices[] = {
	{ "simplefb", &r9q_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-r9q",
	.ops = {
	},
	.devices = r9q_devices,
	.num_devices = ARRAY_SIZE(r9q_devices),
	.quirks = 0
};
