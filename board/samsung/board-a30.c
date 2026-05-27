/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, faveoled <faveoled@yandex.com> 
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos7885.h>

// Early initialization
int a30_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info a30_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xec000000
};

static const struct device a30_devices[] = {
	{ "simplefb", &a30_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a30",
	.ops = {
		.early_init = a30_init,
	},
	.devices = a30_devices,
	.num_devices = ARRAY_SIZE(a30_devices),
	.quirks = 0
};
