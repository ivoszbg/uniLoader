/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

int n61ap_init(void)
{
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info n61ap_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = CONFIG_FRAMEBUFFER_WIDTH,
	.height = CONFIG_FRAMEBUFFER_HEIGHT,
	.stride = CONFIG_FRAMEBUFFER_STRIDE,
	.address = (void *)CONFIG_FRAMEBUFFER_BASE
};
#endif

int n61ap_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &n61ap_fb);
#endif
	return 0;
}

int n61ap_late_init(void)
{
	return 0;
}

struct board_data board_ops = {
	.name = "apple-iphone6",
	.ops = {
		.early_init = n61ap_init,
		.drivers_init = n61ap_drv,
		.late_init = n61ap_late_init,
	},
	.quirks = 0
};
