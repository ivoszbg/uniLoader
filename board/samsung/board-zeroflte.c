/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x13930000
#define HW_SW_TRIG_CONTROL	0x6b0

// Early initialization
int zeroflte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x2058;
	return 0;
}

// Late initialization
int zeroflte_late_init(void)
{
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info zeroflte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = CONFIG_FRAMEBUFFER_WIDTH,
	.height = CONFIG_FRAMEBUFFER_HEIGHT,
	.stride = CONFIG_FRAMEBUFFER_STRIDE,
	.address = (void *)CONFIG_FRAMEBUFFER_BASE
};
#endif

int zeroflte_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &zeroflte_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-zeroflte",
	.ops = {
		.early_init = zeroflte_init,
		.drivers_init = zeroflte_drv,
		.late_init = zeroflte_late_init,
	},
	.quirks = 0
};
