/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Denzeel Oliva <wachiturroxd150@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
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

static struct video_info r8s_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2400,
	.stride = 4,
	.address = (void *)0xf1000000
};

static const struct device r8s_devices[] = {
	{ "simplefb", &r8s_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-r8s",
	.ops = {
		.early_init = r8s_init,
	},
	.devices = r8s_devices,
	.num_devices = ARRAY_SIZE(r8s_devices),
	.quirks = 0
};
