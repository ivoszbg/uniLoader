/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) BotchedRPR <thenxguy0@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x19050000
#define HW_SW_TRIG_CONTROL	0x70

int c1s_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

int c1s_late_init(void)
{
	return 0;
}

int c1s_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1080,
		.height = 2400,
		.stride = 4,
		.address = (void *)0xf1000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-c1s",
	.ops = {
		.early_init = c1s_init,
		.drivers_init = c1s_drv,
		.late_init = c1s_late_init,
	},
	.quirks = 0
};
