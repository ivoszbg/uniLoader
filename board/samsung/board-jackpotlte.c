/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
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

// Late initialization
int jackpotlte_late_init(void)
{
	return 0;
}

int jackpotlte_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1080,
		.height = 2200,
		.stride = 4,
		.address = (void *)0xec000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-jackpotlte",
	.ops = {
		.early_init = jackpotlte_init,
		.drivers_init = jackpotlte_drv,
		.late_init = jackpotlte_late_init,
	},
	.quirks = 0
};
