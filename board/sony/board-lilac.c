/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info lilac_fb = {
	.format = FB_FORMAT_RGB888,
	.width = 720,
	.height = 1280,
	.stride = 3,
	.address = (void *)0x9d400000
};
#endif

int lilac_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &lilac_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "sony-lilac",
	.ops = {
		.drivers_init = lilac_drv,
	},
	.quirks = 0
};
