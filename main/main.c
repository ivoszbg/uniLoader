/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <main.h>

void main(void* dt, void* kernel) {
	/* C entry */

	/* Initialize SoC and Board specific peripherals/quirks */
	soc_init();
	printk("soc_init() passed!");

	board_init();
	printk("board_init() passed!");

	/* Copy kernel to memory and boot  */
	printk("Booting linux...");
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
	load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);	
}
