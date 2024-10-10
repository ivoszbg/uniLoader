// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <string.h>
#include <lib/debug.h>

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

	printk(-1, "             .__.____                     .___\n");
	printk(-1, " __ __  ____ |__|    |    _________     __| _/___________\n");
	printk(-1, "|  |  \\/    \\|  |    |   /  _ \\__  \\   / __ |/ __ \\_  __\\\n");
	printk(-1, "|  |  /   |  \\  |    |__(  <_> ) __ \\_/ /_/ \\  ___/|  |\\/\n");
	printk(-1, "|____/|___|  /__|_______ \\____(____  /\\____ |\\___  >__|\n");
	printk(-1, "           \\/           \\/         \\/      \\/    \\/\n");

	printk(-1, "Passed board initialization!\n");
	printk(-1, "Welcome to uniLoader!\n");

	/* Copy kernel to memory and boot  */
	printk(-1, "Booting linux...\n");

	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
	load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);

	/* We shouldn't get there */
	printk(KERN_WARNING, "Something wrong happened, we shouldn't be here. Hanging....\n");
	while(1) {}
}
