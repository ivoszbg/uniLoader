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

	#ifdef __aarch64__
		/* Define our own 128 bit memcpy */
		void memcpy(void *dest, void *src, size_t size)
		{
			unsigned __int128 *src2 = src;
			unsigned __int128 *dest2 = dest;

			for (size_t i = 0; i < size / 16; i++)
				dest2[i] = src2[i];
		}
	#endif

	#ifdef __arm__
		void memcpy (char* src, char* dest, size_t len) {
			for (size_t i = 0; i < len; i++)
				dest[i] = src[i];
		}
	#endif
