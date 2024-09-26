/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>

#define PIPE_SSPP_SRC_FORMAT                    0x30
#define PIPE_SSPP_SRC_UNPACK_PATTERN            0x34
#define PIPE_BASE				0x1A15000
#define PIPE_SSPP_SRC_YSTRIDE			0x24

#define MDP_CTL_0_BASE                          0x1A02000
#define MDP_CTL_FLUSH				0x18

static void j5lte_init(void)
{
	/* TODO: Doesn't really work :P */
	writel(0x000236FF, PIPE_BASE + PIPE_SSPP_SRC_FORMAT);
	writel(0x03020001, PIPE_BASE + PIPE_SSPP_SRC_UNPACK_PATTERN);
	writel((720 * 4), MDP_CTL_0_BASE + MDP_CTL_FLUSH);
	writel((1 << (0)), PIPE_BASE + PIPE_SSPP_SRC_YSTRIDE);
}

// Late initialization
static void j5lte_late_init(void)
{

}

static void j5lte_driver_setup(void)
{
    // Register drivers for S5PV210-specific peripherals
    // register_driver(&simplefb_driver);
    // Add more drivers here as needed
}

// Create the board_data structure for S5PV210
static struct board_data j5lte_board = {
	.name = "J5LTE",
	.init = j5lte_init,
	.late_init = j5lte_late_init,
	.driver_setup = j5lte_driver_setup,
};

// Function to retrieve current board data
struct board_data *get_current_board(void) {
	return &j5lte_board; // Return the correct board based on runtime config
}
