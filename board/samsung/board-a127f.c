/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, famfo <famfo@famfo.xyz>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <soc/exynos3830.h>
#include <stdint.h>

void init_board_funcs(void *board)
{
	struct {
		const char *name;
		int ops[BOARD_OP_EXIT];
	} *board_restruct = board;

        board_restruct->name = "A127F";
}

int board_init(void)
{
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

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
		.width = 720,
		.height = 1600,
		.stride = 4,
		.address = (void *)0xfa000000,
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}
