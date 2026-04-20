/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Aiden Isik <aidenisik+git@member.fsf.org>
 */
#include <board.h>
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

#define DECON_F_BASE 0x138C0000
#define HW_SW_TRIG_CONTROL 0x70
#define HW_TRIG_EN 0x1

int lucky7_init(void)
{
	/* Enable writing to the framebuffer */
	writel(HW_TRIG_EN, (void *)(DECON_F_BASE + HW_SW_TRIG_CONTROL));
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info lucky7_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 396,
	.height = 396,
	.stride = 4,
	.address = (void *)0x9e0cf000
};
#endif

int lucky7_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &lucky7_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-lucky7",
	.ops = {
		.early_init = lucky7_init,
		.drivers_init = lucky7_drv,
	},
	.quirks = 0
};
