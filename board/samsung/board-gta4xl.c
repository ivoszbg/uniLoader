// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Alexandru Chimac <alexchimac@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x148b0000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int gta4xl_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

// Late initialization
int gta4xl_late_init(void)
{
	return 0;
}

int gta4xl_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1200,
		.height = 2000,
		.stride = 4,
		.address = (void *)0xca000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-gta4xl",
	.ops = {
		.early_init = gta4xl_init,
		.drivers_init = gta4xl_drv,
		.late_init = gta4xl_late_init,
	},
	.quirks = 0
};
