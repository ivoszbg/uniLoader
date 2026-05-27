/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Muhammad <thevancedgamer@mentallysanemainliners.org>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos1280.h>

// Early initialization
int a33x_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;

	return 0;
}

static struct video_info a33x_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.address = (void *)0xfa200000
};

static const struct device a33x_devices[] = {
	{ "simplefb", &a33x_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a33x",
	.ops = {
		.early_init = a33x_init,
	},
	.devices = a33x_devices,
	.num_devices = ARRAY_SIZE(a33x_devices),
	.quirks = 0
};
