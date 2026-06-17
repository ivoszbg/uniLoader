/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Ivan <ivan@int4.cc>
 *
 * Parts of this code are adapted from board/samsung/board-r0q.c
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info pong_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2412,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xb8000000
};

static const struct device pong_devices[] = {
	{ "simplefb", &pong_fb, "fb" },
};

struct board_data board_ops = {
	.name = "nothing-pong",
	.ops = {
	},
	.devices = pong_devices,
	.num_devices = ARRAY_SIZE(pong_devices),
	.quirks = 0
};
