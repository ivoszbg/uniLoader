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

struct video_info *fb_info;

static void clean_fbmem(void *fb, int width, int height, int stride)
{
	memset(fb, 0x0, (width * height * stride));
}

static void draw_pixel(volatile char *fb, int x, int y, int width, int stride,
		       color c)
{
	// Check bounds to prevent drawing outside the framebuffer
	if (x < 0 || x >= width || y < 0 || y >= fb_info->height)
		return;

	long int location = (x * stride) + (y * width * stride);
	switch (fb_info->format) {
	case FB_FORMAT_ARGB8888:
		*(fb + location) = c.b;
		*(fb + location + 1) = c.g;
		*(fb + location + 2) = c.r;
		*(fb + location + 3) = c.a;
		break;
	case FB_FORMAT_BGRA8888:
		*(fb + location) = c.r;
		*(fb + location + 1) = c.g;
		*(fb + location + 2) = c.b;
		*(fb + location + 3) = c.a;
		break;
	case FB_FORMAT_RGB888:
		*(fb + location) = c.b;
		*(fb + location + 1) = c.g;
		*(fb + location + 2) = c.r;
		break;
	}
}

#define SCALED_FONTW (FONTW * fb_info->scale_f)
#define SCALED_FONTH (FONTH * fb_info->scale_f)

int get_font_scale_factor()
{
	if (fb_info->scale) {
		return fb_info->scale;
	}

	/* integer sqrt inline */
	int n = fb_info->width * fb_info->width + fb_info->height * fb_info->height;
	int x = n;
	int y = (x + 1) / 2;

	while (y < x) {
		x = y;
		y = (x + n / x) / 2;
	}
	int diag = x;

	/* determine scale factor from pseudo-diagonal */
	int scale = diag / 750;

	/* clamp the result */
	if (scale < 1)
		scale = 1;

	return scale;
}

void __simplefb_raw_print(const char *text, int text_x, int text_y,
			  color text_color)
{
	if (!fb_info) return;

	int l = strlen(text);
	int current_x = text_x;
	int current_y = text_y;

	static int last_y = 0;
	if (last_y == 0) {
		last_y = text_y;
	} else {
		current_y = last_y;
	}

	int max_x = fb_info->width - SCALED_FONTW;

	for (int i = 0; i < l; i++) {
		// Special characters handling
		switch (text[i]) {
		case '\n':
			current_x = text_x;
			current_y += SCALED_FONTH;
			break;
		case '\r':
			current_x = text_x;
			break;
		}

		// Non-printable chars
		if (text[i] < 32)
			continue;

		if (current_x > max_x) {
			current_x = text_x;
			current_y += SCALED_FONTH;
		}

		if (current_y >= fb_info->height - SCALED_FONTH) {
			clean_fbmem((char*)fb_info->address, fb_info->width, fb_info->height, fb_info->stride);
			current_x = text_x;
			current_y = 5;
		}

		int ix = font_index(text[i]);
		unsigned char *img = letters[ix];

		// Draw the character as a scaled bitmap
		for (int y = 0; y < FONTH; y++) {
			unsigned char b = img[y];

			for (int x = 0; x < FONTW; x++) {
				if (((b << x) & 0b10000000) > 0) {
					for (int dy = 0; dy < fb_info->scale_f; dy++) {
						for (int dx = 0; dx < fb_info->scale_f; dx++) {
							draw_pixel(fb_info->address, current_x + x * fb_info->scale_f + dx,
								   current_y + y * fb_info->scale_f + dy,
								   fb_info->width, fb_info->stride, text_color);
						}
					}
				}
			}
		}
		current_x += SCALED_FONTW;
	}
	last_y = current_y;
}

void simplefb_probe(void *data)
{
	fb_info = data;

	clean_fbmem((char*)fb_info->address, fb_info->width, fb_info->height,
		 fb_info->stride);

	fb_info->scale_f = get_font_scale_factor();
}
