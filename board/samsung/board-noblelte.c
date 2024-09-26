/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Faiz Faadhillah <faiz.faadhillah@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>

#define DECON_F_BASE		0x13930000
#define HW_SW_TRIG_CONTROL	0x6b0

// Early initialization
static void noblelte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x2058;
}

// Late initialization
static void noblelte_late_init(void)
{

}

static void noblelte_driver_setup(void)
{
    // Register drivers for S5PV210-specific peripherals
    // register_driver(&simplefb_driver);
    // Add more drivers here as needed
}

// Create the board_data structure for S5PV210
static struct board_data noblelte_board = {
	.name = "NOBLELTE",
	.init = noblelte_init,
	.late_init = noblelte_late_init,
	.driver_setup = noblelte_driver_setup,
};

// Function to retrieve current board data
struct board_data *get_current_board(void) {
	return &noblelte_board; // Return the correct board based on runtime config
}
