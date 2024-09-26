/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>

#define DECON_F_BASE		0x13930000
#define HW_SW_TRIG_CONTROL	0x6b0

// Early initialization
static void zeroflte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x2058;
}

// Late initialization
static void zeroflte_late_init(void)
{

}

static void zeroflte_driver_setup(void)
{
    // Register drivers for S5PV210-specific peripherals
    // register_driver(&simplefb_driver);
    // Add more drivers here as needed
}

// Create the board_data structure for S5PV210
static struct board_data zeroflte_board = {
	.name = "ZEROFLTE",
	.init = zeroflte_init,
	.late_init = zeroflte_late_init,
	.driver_setup = zeroflte_driver_setup,
};

// Function to retrieve current board data
struct board_data *get_current_board(void) {
	return &zeroflte_board; // Return the correct board based on runtime config
}
