/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, schoosh212 <superaviation001@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE            0x13060000
#define HW_SW_TRIG_CONTROL      0x70

int a13_init(void)
{
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info a13_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2408,
	.stride = 4,
	.address = (void *)0xfa000000
};

static const struct device a13_devices[] = {
	{ "simplefb", &a13_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a13",
	.ops = {
		.early_init = a13_init,
	},
	.devices = a13_devices,
	.num_devices = ARRAY_SIZE(a13_devices),
	.quirks = 0
};
