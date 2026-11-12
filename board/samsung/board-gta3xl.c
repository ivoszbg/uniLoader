/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Umer Uddin <umer.uddin@mentallysanemainliners.org>
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos7885.h>

// Early initialization
int gta3xl_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info gta3xl_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1200,
	.height = 1920,
	.scale = 2,
	.stride = 4,
	.address = (void *)0xec000000
};

static const struct device gta3xl_devices[] = {
	{ "simplefb", &gta3xl_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-gta3xl",
	.ops = {
		.early_init = gta3xl_init,
	},
	.devices = gta3xl_devices,
	.num_devices = ARRAY_SIZE(gta3xl_devices),
	.quirks = 0
};
