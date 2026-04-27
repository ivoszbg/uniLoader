/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info rmx3511_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2408,
	.stride = 4,
	.address = (void *)0x9e000000
};
#endif

int rmx3511_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &rmx3511_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "realme-rmx3511",
	.ops = {
		.drivers_init = rmx3511_drv
	},
	.quirks = 0
};
