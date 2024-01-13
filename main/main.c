/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <main.h>
#include <string.h>

void testreadkeys();

void main(void* dt, void* kernel) {
	/* Initialize SoC and Board specific peripherals/quirks */

	/* TODO: Find a better way to make this more universal (since devices like arm64 Samsung Galaxies enable FB after soc_init) */
#ifdef CONFIG_SIMPLE_FB
	clean_fb((char*)CONFIG_FRAMEBUFFER_BASE, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_HEIGHT, CONFIG_FRAMEBUFFER_STRIDE);
#endif
	soc_init();
	printk("soc_init() passed!");

	board_init();
	printk("board_init() passed!");

	/* Copy kernel to memory and boot  */
	printk("Booting linux...");

//	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
//	load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);

	/* We shouldn't get there */
	while(1) {testreadkeys();}
}
