/* SPDX-License-Identifier: GPL-2.0 */

#include <board.h>
#include <main.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

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

	board_restruct->name = "R0Q";
}

int board_init(void)
{
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
		.height = 2340,
		.stride = 4,
		.address = (void *)0xb8000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}
