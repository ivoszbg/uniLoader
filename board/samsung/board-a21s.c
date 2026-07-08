/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, DaemonMCR <tturner@lineageos.org>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos3830.h>

int a21s_init(void)
{
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info a21s_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1600,
	.stride = 4,
	.address = (void *)0xfa000000
};

static const struct device a21s_devices[] = {
	{ "simplefb", &a21s_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a21s",
	.ops = {
		.early_init = a21s_init,
	},
	.devices = a21s_devices,
	.num_devices = ARRAY_SIZE(a21s_devices),
	.quirks = 0
};
