/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <string.h>

void main(void* dt, void* kernel) {
	/* Initialize SoC and Board specific peripherals/quirks */
	struct board_data *board = get_current_board();

	/* TODO: Move into the simpleFB driver once the drivers framework is done */
#ifdef CONFIG_SIMPLE_FB
	clean_fb((char*)CONFIG_FRAMEBUFFER_BASE, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_HEIGHT, CONFIG_FRAMEBUFFER_STRIDE);
#endif
	board->init();
	printk("board init() passed!");

	board->driver_setup();

	board->late_init();
	printk("board late_init() passed!");

	/* Copy kernel to memory and boot  */
	printk("Booting linux...");

	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
	load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);

	/* We shouldn't get there */
	while(1) {}
}
