/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <main.h>
/* Device specific board config, copied in runtime */
#include <board-config.h>

void main(void* dt, void* kernel) {
	/* Initialize SoC and Board specific peripherals/quirks */
	soc_init();
	board_init();

	/* Copy kernel to memory and boot  */
	memcpy((void*)PAYLOAD_ENTRY, kernel, PAYLOAD_SIZE);
	load_kernel(dt, 0, 0, 0, (void*)PAYLOAD_ENTRY);	
}
