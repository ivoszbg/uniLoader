/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <main.h>
/* Device specific board config, copied in runtime */
#include <board-config.h>

void main(void* dt, void* kernel) {
	/* C entry */

	/* Initialize SoC and Board specific peripherals/quirks */
	soc_init();
	debug_printfb((char*)FRAMEBUFFER_BASE, "soc_init() passed!", 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_STRIDE);

	board_init();
	debug_printfb((char*)FRAMEBUFFER_BASE, "board_init() passed!", 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_STRIDE);

	/* Copy kernel to memory and boot  */
	debug_printfb((char*)FRAMEBUFFER_BASE, "Booting linux...", 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_STRIDE);
	memcpy((void*)PAYLOAD_ENTRY, kernel, PAYLOAD_SIZE);
	load_kernel(dt, 0, 0, 0, (void*)PAYLOAD_ENTRY);	
}
