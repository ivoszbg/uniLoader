/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023, predefine <predefine@yandex.ru>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>

#define DECON_F_BASE		0x14830000
#define HW_SW_TRIG_CONTROL	0x70

// Early initialization
static void j4lte_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}

// Late initialization
static void j4lte_late_init(void)
{

}

static void j4lte_driver_setup(void)
{
    // Register drivers for S5PV210-specific peripherals
    // register_driver(&simplefb_driver);
    // Add more drivers here as needed
}

// Create the board_data structure for S5PV210
static struct board_data j4lte_board = {
	.name = "J4LTE",
	.init = j4lte_init,
	.late_init = j4lte_late_init,
	.driver_setup = j4lte_driver_setup,
};

// Function to retrieve current board data
struct board_data *get_current_board(void) {
	return &j4lte_board; // Return the correct board based on runtime config
}
