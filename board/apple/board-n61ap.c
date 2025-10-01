/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info n61ap_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 752,
	.height = 1334,
	.stride = 4,
	.address = (void *)0x83e900000
};
#endif

int n61ap_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &n61ap_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "apple-iphone6",
	.ops = {
		.drivers_init = n61ap_drv,
	},
	.quirks = 0
};
