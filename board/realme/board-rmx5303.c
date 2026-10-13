/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026, Vasiliy Borisov <vasy@vasy.ru>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info rmx5303_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1604,
	.stride = 4,
	.address = (void *)0x9caa8000,
};

static const struct device rmx5303_devices[] = {
       { "simplefb", &rmx5303_fb, "fb" },
};

struct board_data board_ops = {
	.name = "realme-rmx5303",
	.devices = rmx5303_devices,
	.num_devices = ARRAY_SIZE(rmx5303_devices),
	.quirks = 0
};
