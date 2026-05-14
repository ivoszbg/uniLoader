/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info pdx245_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xd5100000,

	// The scale that is set by default is extremely large.
	.scale = 2
};
#endif

int pdx245_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &pdx245_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "sony-pdx245",
	.ops = {
		.drivers_init = pdx245_drv,
	},
	.quirks = 0
};
