/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, famfo <famfo@famfo.xyz>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos3830.h>
#include <stdint.h>

int a12s_init(void)
{
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info a12s_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 720,
	.height = 1600,
	.stride = 4,
	.address = (void *)0xfa000000
};
#endif

int a12s_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &a12s_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-a12s",
	.ops = {
		.early_init = a12s_init,
		.drivers_init = a12s_drv,
	},
	.quirks = 0
};
