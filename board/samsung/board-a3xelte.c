/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

#define DECON_F_BASE			0x14830000
#define HW_SW_TRIG_CONTROL		0x6b0

int a3xelte_init(void)
{
	/* allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x2058;
	return 0;
}

static struct video_info a3xelte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1280,
	.stride = 4,
	.address = (void *)0x67000000
};

static const struct device a3xelte_devices[] = {
	{ "simplefb", &a3xelte_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a3xelte",
	.ops = {
		.early_init = a3xelte_init,
	},
	.devices = a3xelte_devices,
	.num_devices = ARRAY_SIZE(a3xelte_devices),
	.quirks = 0
};
