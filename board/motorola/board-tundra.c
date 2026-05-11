/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info tundra_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xe1000000
};
#endif

int tundra_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &tundra_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "motorola-tundra",
	.ops = {
		.drivers_init = tundra_drv
	},
	.quirks = 0
};
