/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Henri Ommer <henrio6@users.noreply.github.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos7885.h>

// Early initialization
int a20e_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info a20e_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1560,
	.stride = 4,
	.address = (void *)0xec000000
};

static const struct device a20e_devices[] = {
	{ "simplefb", &a20e_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-a20e",
	.ops = {
		.early_init = a20e_init,
	},
	.devices = a20e_devices,
	.num_devices = ARRAY_SIZE(a20e_devices),
	.quirks = 0
};