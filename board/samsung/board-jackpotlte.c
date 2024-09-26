/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>

#define DECON_F_BASE		0x14860000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
static void jackpotlte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}

// Late initialization
static void jackpotlte_late_init(void)
{

}

static void jackpotlte_driver_setup(void)
{
    // Register drivers for S5PV210-specific peripherals
    // register_driver(&simplefb_driver);
    // Add more drivers here as needed
}

// Create the board_data structure for S5PV210
static struct board_data jackpotlte_board = {
	.name = "JACKPOTLTE",
	.init = jackpotlte_init,
	.late_init = jackpotlte_late_init,
	.driver_setup = jackpotlte_driver_setup,
};

// Function to retrieve current board data
struct board_data *get_current_board(void) {
	return &jackpotlte_board; // Return the correct board based on runtime config
}
