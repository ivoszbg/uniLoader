/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Denzeel Oliva <wachiturroxd150@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x19050000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int r8s_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

// Late initialization
int r8s_late_init(void)
{
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info r8s_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = CONFIG_FRAMEBUFFER_WIDTH,
	.height = CONFIG_FRAMEBUFFER_HEIGHT,
	.stride = CONFIG_FRAMEBUFFER_STRIDE,
	.address = (void *)CONFIG_FRAMEBUFFER_BASE
};
#endif

int r8s_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &r8s_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-r8s",
	.ops = {
		.early_init = r8s_init,
		.drivers_init = r8s_drv,
		.late_init = r8s_late_init,
	},
	.quirks = 0
};
