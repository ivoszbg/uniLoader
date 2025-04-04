/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Muhammad <thevancedgamer@mentallysanemainliners.org>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos1280.h>

void init_board_funcs(void *board)
{
	/*
	 * Parsing the struct directly without restructing is
	 * broken as of Sep 29 2024
	 */
	struct {
		const char *name;
		int ops[BOARD_OP_EXIT];
	} *board_restruct = board;

	board_restruct->name = "A33x";
}

// Early initialization
int board_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;

	return 0;
}

// Late initialization
int board_late_init(void)
{
	return 0;
}

int board_driver_setup(void)
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
