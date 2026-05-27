/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x14860000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int jackpotlte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

static struct video_info jackpotlte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2200,
	.stride = 4,
	.address = (void *)0xec000000
};

static const struct device jackpotlte_devices[] = {
	{ "simplefb", &jackpotlte_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-jackpotlte",
	.ops = {
		.early_init = jackpotlte_init,
	},
	.devices = jackpotlte_devices,
	.num_devices = ARRAY_SIZE(jackpotlte_devices),
	.quirks = 0
};
