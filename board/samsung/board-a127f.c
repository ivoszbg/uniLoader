/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, famfo <famfo@famfo.xyz>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos3830.h>
#include <stdint.h>

int a127f_init(void)
{
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info a127f_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1600,
	.stride = 4,
	.address = (void *)0xfa000000
};
#endif

int a127f_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &a127f_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-a127f",
	.ops = {
		.early_init = a127f_init,
		.drivers_init = a127f_drv,
	},
	.quirks = 0
};
