/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <main.h>

void main(void* dt, void* kernel) {
	/* C entry */

	/* Initialize SoC and Board specific peripherals/quirks */
	soc_init();
	#ifdef CONFIG_SIMPLE_FB
	debug_printfb((char*)CONFIG_FRAMEBUFFER_BASE, "soc_init() passed!", 0, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
	#endif

	board_init();
	#ifdef CONFIG_SIMPLE_FB
	debug_printfb((char*)CONFIG_FRAMEBUFFER_BASE, "board_init() passed!", 0, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
	#endif

	/* Copy kernel to memory and boot  */
	#ifdef CONFIG_SIMPLE_FB
	debug_printfb((char*)CONFIG_FRAMEBUFFER_BASE, "Booting linux...", 0, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
	#endif
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
	load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);	
}
