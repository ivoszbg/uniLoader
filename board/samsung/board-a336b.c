/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Muhammad <thevancedgamer@mentallysanemainliners.org>
 */
#include <board.h>
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

#ifdef CONFIG_SIMPLE_FB
static struct video_info a33x_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.address = (void *)0xfa200000
};
#endif

int a33x_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &a33x_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-a33x",
	.ops = {
		.early_init = a33x_init,
		.drivers_init = a33x_drv,
	},
	.quirks = 0
};
