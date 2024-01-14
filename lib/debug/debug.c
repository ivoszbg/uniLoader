/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <lib/debug.h>
#include <stddef.h>

/* TODO: Import libc */
void writel(unsigned int value, void* address) {
    // Cast the address pointer to a 32-bit unsigned integer pointer
    volatile unsigned int* ptr = (volatile unsigned int*)address;

    // Write the value to the memory location
    *ptr = value;
}

int readl(void* address) {
    // Cast the address pointer to a 32-bit unsigned integer pointer
    volatile unsigned int* ptr = (volatile unsigned int*)address;

    // return the value of the memory location
    return *ptr;
}


void printk(char *text) {
#ifdef CONFIG_SIMPLE_FB
	/* IMPORTANT: Limit the linecount */
	if(debug_linecount > 100 || debug_linecount < 0)
		debug_linecount = 0;

	draw_text((char*)CONFIG_FRAMEBUFFER_BASE, "[uniLoader] ", 0, (20 + (debug_linecount * 30)), CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
	draw_text((char*)CONFIG_FRAMEBUFFER_BASE, text, 0 + (12 * 8), (20 + (debug_linecount * 30)), CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);

	debug_linecount++;
#endif
}
