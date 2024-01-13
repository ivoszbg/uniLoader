/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#include <lib/simplefb.h>
#include <lib/font.h>
#include <string.h>

void clean_fb(volatile char *fb, int width, int height, int stride) {
	for (volatile char *addr = fb; addr < fb + (width * height * stride); addr += stride)
		*(int*)(addr) = 0x0;
}

/* RGB888 format */
/* Unlike ARGB8888, we explicitly use 3 bytes to represent each pixel, making sure no extra padding byte is added. */
void draw_pixel(volatile char *fb, int x, int y, int width, int stride, color c) {
	long int location = (x * stride) + (y * width * stride);

	*(fb + location) = c.b;
	*(fb + location + 1) = c.g;
	*(fb + location + 2) = c.r;
#if CONFIG_FRAMEBUFFER_STRIDE == 4
	*(fb + location + 3) = c.a;
#endif
}

void draw_horizontal_line(volatile char *fb, int x1, int x2, int y, color c, int width, int stride) {
	for (int i = x1; i < x2; i++)
		draw_pixel(fb, i, y, width, stride, c);
}

void draw_vertical_line(volatile char *fb, int x, int y1, int y2, color c, int width, int stride) {
	for (int i = y1; i < y2; i++)
		draw_pixel(fb, x, i, width, stride, c);
}

void draw_filled_rectangle(volatile char *fb, int x1, int y1, int w, int h, color c, int width, int stride) {
	for (int i = y1; i < (y1 + h); i++)
		draw_horizontal_line(fb, x1, (x1 + w), i, c, width, stride);
}

void draw_text(volatile char *fb, char *text, int textX, int textY, int width, int stride) {
	// loop through all characters in the text string
	int l = strlen(text);

	for (int i = 0; i < l; i++) {
		if (text[i] < 32)
			continue;

		int ix = font_index(text[i]);
		unsigned char *img = letters[ix];

		for (int y = 0; y < FONTH; y++) {
			unsigned char b = img[y];

			for (int x = 0; x < FONTW; x++) {
				if (((b << x) & 0b10000000) > 0)
					draw_pixel(fb, textX + i * FONTW + x, textY + y, width, stride, (color){255, 255, 255});
			}
		}
	}
}
