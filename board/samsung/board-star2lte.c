/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2025, Leah <leah@realfx.rocks>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x16030000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int star2lte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info star2lte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 2960,
	.stride = 4,
	.address = (void *)0xcc000000
};
#endif

int star2lte_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &star2lte_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-star2lte",
	.ops = {
		.early_init = star2lte_init,
		.drivers_init = star2lte_drv,
	},
	.quirks = 0
};
