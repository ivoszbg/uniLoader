/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Faiz Faadhillah <faiz.faadhillah@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x13930000
#define HW_SW_TRIG_CONTROL	0x6b0

// Early initialization
int noblelte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x2058;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info noblelte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 2560,
	.stride = 4,
	.address = (void *)0xe2a00000
};
#endif

int noblelte_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &noblelte_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-noblelte",
	.ops = {
		.early_init = noblelte_init,
		.drivers_init = noblelte_drv,
	},
	.quirks = 0
};
