// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#include <string.h>
#include <drivers/framework.h>
#include <lib/video/font.h>
#include <lib/simplefb.h>

static void clean_fbmem(void *fb, int width, int height, int stride)
{
	memset(fb, 0x0, (width * height * stride));
}

// RGB888 format
/*
 * Unlike ARGB8888, we explicitly use 3 bytes to represent each pixel, making sure
 * no extra padding byte is added.
 */
static void draw_pixel(volatile char *fb, int x, int y, int width, int stride,
		       color c)
{
	long int location = (x * stride) + (y * width * stride);
#ifdef CONFIG_FRAMEBUFFER_BGRA
	*(fb + location) = c.b;
	*(fb + location + 2) = c.r;
#else
	*(fb + location) = c.r;
	*(fb + location + 2) = c.b;
#endif
	*(fb + location + 1) = c.g;
#if CONFIG_FRAMEBUFFER_STRIDE == 4
	*(fb + location + 3) = c.a;
#endif
}

static int current_y = 0;
void __simplefb_raw_print(volatile char *fb, char *text, int text_x, int text_y,
			  int width, int stride)
{
	int l = strlen(text);
	int current_x = text_x;
	current_y = current_y == 0 ? text_y : current_y;

	int max_x = width - SCALED_FONTW;

	for (int i = 0; i < l; i++) {
		// Special characters handling
		switch (text[i]) {
		case '\n':
			current_x = text_x;
			current_y += SCALED_FONTH;
			break;
		}

		// Non-printable chars
		if (text[i] < 32)
			continue;

		if (current_x > max_x) {
			current_x = text_x;
			current_y += SCALED_FONTH;
		}

		int ix = font_index(text[i]);
		unsigned char *img = letters[ix];

		// Draw the character as a scaled bitmap
		for (int y = 0; y < FONTH; y++) {
			unsigned char b = img[y];

			for (int x = 0; x < FONTW; x++) {
				if (((b << x) & 0b10000000) > 0) {
					for (int dy = 0; dy < SCALE_FACTOR; dy++) {
						for (int dx = 0; dx < SCALE_FACTOR; dx++) {
							draw_pixel(fb, current_x + x * SCALE_FACTOR + dx,
								   current_y + y * SCALE_FACTOR + dy,
								   width, stride, (color){255, 255, 255});
						}
					}
				}
			}
		}
		current_x += SCALED_FONTW;
	}
}

void simplefb_probe(void *data)
{
	struct video *fb_info = data;

	clean_fbmem((char*)fb_info->address, fb_info->width, fb_info->height,
		 fb_info->stride);

	/* TODO: Introduce a full drivers framework that allows proper exiting */
}
