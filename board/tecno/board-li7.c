/* SPDX-License-Identifier: GPL-2.0 */
/* 
 * Copyright (c) Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info li7_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1088,
	.height = 2436,
	.stride = 4,
	.address = (void *)0xfdfff000
};

int li7_drv(void)
{
	REGISTER_DRIVER("simplefb", simplefb_probe, &li7_fb);
	return 0;
}

struct board_data board_ops = {
	.name = "tecno-li7",
	.ops = {
		.drivers_init = li7_drv,
	},
	.quirks = 0
};
