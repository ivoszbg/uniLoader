/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info vita12k_fb = {
	.format = FB_FORMAT_BGRA8888,
	.width = 960,
	.height = 544,
	.stride = 4,
	.address = (void *)0x20000000
};
#endif

int vita_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	// for now only vita1k/vita2k is supported, PSTV would need more config :/
	REGISTER_DRIVER("simplefb", simplefb_probe, &vita12k_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "sony-psvita",
	.ops = {
		.drivers_init = vita_drv,
	},
	.quirks = 0
};
