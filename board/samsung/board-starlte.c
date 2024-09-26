/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>

#define DECON_F_BASE		0x16030000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
static void starlte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}

// Late initialization
static void starlte_late_init(void)
{

}

static void starlte_driver_setup(void)
{
    // Register drivers for S5PV210-specific peripherals
    // register_driver(&simplefb_driver);
    // Add more drivers here as needed
}

// Create the board_data structure for S5PV210
static struct board_data starlte_board = {
	.name = "STARLTE",
	.init = starlte_init,
	.late_init = starlte_late_init,
	.driver_setup = starlte_driver_setup,
};

// Function to retrieve current board data
struct board_data *get_current_board(void) {
	return &starlte_board; // Return the correct board based on runtime config
}
