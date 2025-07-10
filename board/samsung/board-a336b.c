/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Muhammad <thevancedgamer@mentallysanemainliners.org>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos1280.h>

// Early initialization
int a33x_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;

	return 0;
}

// Late initialization
int a33x_late_init(void)
{
	return 0;
}

int a33x_drv(void)
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
		.address = (void *)0x0fa200000,
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-a33x",
	.ops = {
		.early_init = a33x_init,
		.drivers_init = a33x_drv,
		.late_init = a33x_late_init,
	},
	.quirks = 0
};
