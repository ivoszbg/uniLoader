/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Oleksii Onchul <oleksiionchul@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info ginkgo_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.scale = 2,
	.address = (void *)0x5c000000
};

static const struct device ginkgo_devices[] = {
	{ "simplefb", &ginkgo_fb, "fb" },
};

struct board_data board_ops = {
	.name = "xiaomi-ginkgo",
	.ops = {
	},
	.devices = ginkgo_devices,
	.num_devices = ARRAY_SIZE(ginkgo_devices),
	.quirks = 0
};
