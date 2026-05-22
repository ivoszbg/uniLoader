/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026, Akari Tsuyukusa <akkun11.open@gmail.com>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info checkers_fb = {
	.format = FB_FORMAT_BGRA8888,
	.width = 480,
	.height = 960,
	.stride = 4,
	.address = (void *)0x5f900000,
	.scale = 1,
};
#endif

int checkers_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &checkers_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "amazon-checkers",
	.ops = {
		.drivers_init = checkers_drv
	},
	.quirks = 0,
};
