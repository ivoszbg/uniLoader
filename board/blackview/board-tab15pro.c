/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026, Akari Tsuyukusa <akkun11.open@gmail.com>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info tab15pro_fb = {
	.format = FB_FORMAT_RGB565,
	.width = 1200,
	.height = 1920,
	.stride = 2,
	.address = (void *)0x9e000000,
	.scale = 2,
};
#endif

int tab15pro_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &tab15pro_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "blackview-tab15pro",
	.ops = {
		.drivers_init = tab15pro_drv
	},
	.quirks = 0
};
