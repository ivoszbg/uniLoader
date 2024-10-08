/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <string.h>

/*
 * Provide an empty board config that has
 * to be filled in the board files
 * TODO: figure out why having it in
 * the board files makes devices reboot.
 */
struct board_data board = {
	.name = "default",
	.ops = {
		[BOARD_OP_INIT] = BOARD_OP_INIT,
		[BOARD_OP_LATE_INIT] = BOARD_OP_LATE_INIT,
		[BOARD_OP_DRIVER_SETUP] = BOARD_OP_DRIVER_SETUP,
	}
};

void main(void* dt, void* kernel)
{
	/* Initialize SoC and Board specific peripherals/quirks */
	init_board_funcs(&board);

	EXECUTE_BOARD_OP(board.ops[BOARD_OP_INIT]);
	EXECUTE_BOARD_OP(board.ops[BOARD_OP_DRIVER_SETUP]);
	EXECUTE_BOARD_OP(board.ops[BOARD_OP_LATE_INIT]);

	printk("Passed board initialization!");
	printk("Welcome to uniLoader!");

	/* Copy kernel to memory and boot  */
	printk("Booting linux...");

	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
	load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);

	/* We shouldn't get there */
	while(1) {}
}
