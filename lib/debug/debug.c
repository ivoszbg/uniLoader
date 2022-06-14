/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <lib/debug.h>

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
