/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define PIPE_SSPP_SRC_FORMAT                    0x30
#define PIPE_SSPP_SRC_UNPACK_PATTERN            0x34
#define PIPE_BASE				0x1A15000
#define PIPE_SSPP_SRC_YSTRIDE			0x24

#define MDP_CTL_0_BASE                          0x1A02000
#define MDP_CTL_FLUSH				0x18

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

	board_restruct->name = "J5LTE";
}

int board_init(void)
{
	/* TODO: Doesn't really work :P */
	writel(0x000236FF, PIPE_BASE + PIPE_SSPP_SRC_FORMAT);
	writel(0x03020001, PIPE_BASE + PIPE_SSPP_SRC_UNPACK_PATTERN);
	writel((720 * 4), MDP_CTL_0_BASE + MDP_CTL_FLUSH);
	writel((1 << (0)), PIPE_BASE + PIPE_SSPP_SRC_YSTRIDE);

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
		.width = 720,
		.height = 1280,
		.stride = 3,
		.address = (void *)0x8e000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}
