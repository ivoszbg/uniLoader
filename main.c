/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <main.h>
/* Device specific board config, copied in runtime */
#include <board-config.h>

extern unsigned long kernel_size;

void main(void* dt, void* kernel, void* end) {
	void* kernel_load;
	/* Initialize SoC and Board specific peripherals/quirks */
	soc_init();
	board_init();

#ifndef PAYLOAD_ENTRY
	/* Find closest 2MB-aligned address to our load location */
	kernel_load = (void *) ((((unsigned long long)end - 1) | (SZ_2M - 1)) + 1);
#else
	kernel_load = (void *) PAYLOAD_ENTRY;
#endif

	/* Copy kernel to memory and boot  */
	memcpy((void*)kernel_load, kernel, (unsigned long) &kernel_size);
	load_kernel(dt, 0, 0, 0, (void*)kernel_load);
}
