/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info x6710_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2400,
	.stride = 4,
	.address = (void *)0x76a10000
};

int x6710_drv(void)
{
	REGISTER_DRIVER("simplefb", simplefb_probe, &x6710_fb);
	return 0;
}

struct board_data board_ops = {
	.name = "infinix-x6710",
	.ops = {
		.drivers_init = x6710_drv,
	},
	.quirks = 0
};
