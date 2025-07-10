/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023, predefine <predefine@yandex.ru>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x14830000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
int j4lte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

// Late initialization
int j4lte_late_init(void)
{
	return 0;
}

int j4lte_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 720,
		.height = 1280,
		.stride = 4,
		.address = (void *)0x67000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-j4lte",
	.ops = {
		.early_init = j4lte_init,
		.drivers_init = j4lte_drv,
		.late_init = j4lte_late_init,
	},
	.quirks = 0
};
