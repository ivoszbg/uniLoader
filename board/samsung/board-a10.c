/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos7885.h>

// Early initialization
int a10_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info a10_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1520,
	.stride = 4,
	.address = (void *)0xec000000
};
#endif

int a10_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &a10_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-a10",
	.ops = {
		.early_init = a10_init,
		.drivers_init = a10_drv,
	},
	.quirks = 0
};
