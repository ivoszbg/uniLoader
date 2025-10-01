/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x16030000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int starlte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info starlte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 2960,
	.height = 1440,
	.stride = 4,
	.address = (void *)0xcc000000
};
#endif

int starlte_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &starlte_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-starlte",
	.ops = {
		.early_init = starlte_init,
		.drivers_init = starlte_drv,
	},
	.quirks = 0
};
