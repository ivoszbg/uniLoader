/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

#define DECON_F_BASE				0x13960000
#define HW_SW_TRIG_CONTROL			0x70
#define HW_SW_TRIG_CONTROL_TRIG_AUTO_MASK_TRIG	(1 << 12)
#define HW_SW_TRIG_CONTROL_SW_TRIG		(1 << 8)
#define HW_SW_TRIG_CONTROL_UNKN_BIT		(1 << 7)
#define HW_SW_TRIG_CONTROL_HW_TRIG_MASK		(1 << 5)
#define HW_SW_TRIG_CONTROL_HW_TRIG_EN		(1 << 4)
#define HW_TRIG_EN				(HW_SW_TRIG_CONTROL_TRIG_AUTO_MASK_TRIG | \
						 HW_SW_TRIG_CONTROL_HW_TRIG_EN)

int herolte_init(void)
{
	/* allow framebuffer to be written to */
	writel(HW_TRIG_EN, (void *)(DECON_F_BASE + HW_SW_TRIG_CONTROL));

	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info herolte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 2960,
	.stride = 4,
	.address = (void *)0xe2a00000
};
#endif

int herolte_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &herolte_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-herolte",
	.ops = {
		.early_init = herolte_init,
		.drivers_init = herolte_drv,
	},
	.quirks = 0
};
