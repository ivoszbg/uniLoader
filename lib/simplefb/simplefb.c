// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#include <string.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/console.h>
#include <lib/debug.h>
#include <lib/video/font.h>
#include <lib/simplefb.h>

struct video_info *fb_info;

/*
 * Boot progress bar painted into the framebuffer, outside the console's
 * text area.  simplefb draws text starting at y = FB_TEXT_TOP_PADDING (5)
 * downwards, so pixel rows 0..4 are never touched by text.
 *
 * Every stage repaints the WHOLE bar as a run of solid 40x5 blocks rather
 * than adding a single pixel, because (a) one pixel per stage is impossible
 * to count in a phone photo and (b) clean_fbmem() wipes the entire
 * framebuffer on overflow, so a partial bar would be destroyed by the
 * hundreds of clears during the 154 MB cacheslice read.  The last repaint
 * therefore always shows the last stage actually reached.
 *
 * Number of lit blocks = last stage reached:
 *   1 tag  2 Image  3 dtb  4 "sdlog tail"  5 append entered
 *   6 append done  7 "args ok"  8 "cmd=[..]"  9 ramdisk  10 cacheslice
 *   11 about to jump into XNU
 *
 * This lives next to the console because it needs fb_info->address; a
 * caller outside this file would have to materialise the panel address
 * itself.
 */
#define SIMPLEFB_MARK_BLOCK	40	/* pixels per stage block */
#define SIMPLEFB_MARK_MAX	11

void simplefb_mark(unsigned int step)
{
	volatile char *fb;
	int stride, width, s, i, y;

	if (!fb_info)
		return;

	fb = (volatile char *)fb_info->address;
	width = fb_info->width;
	stride = fb_info->stride;

	if (step > SIMPLEFB_MARK_MAX)
		step = SIMPLEFB_MARK_MAX;

	for (s = 0; s <= (int)step; s++) {
		for (i = 0; i < SIMPLEFB_MARK_BLOCK; i++) {
			long int x = s * (SIMPLEFB_MARK_BLOCK + 1) + i;

			if (x >= width)
				break;
			for (y = 0; y < 5; y++) {
				long int loc = (x * stride) +
					       (y * width * stride);

				fb[loc + 0] = 0xf0;	/* b */
				fb[loc + 1] = 0xf0;	/* g */
				fb[loc + 2] = 0xf0;	/* r */
				fb[loc + 3] = 0xff;	/* a */
			}
		}
	}
}

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
	case FB_FORMAT_ABGR8888:
		*(fb + location) = c.r;
		*(fb + location + 1) = c.g;
		*(fb + location + 2) = c.b;
		*(fb + location + 3) = c.a;
		break;
	case FB_FORMAT_BGRA8888:
		*(fb + location) = c.a;
		*(fb + location + 1) = c.r;
		*(fb + location + 2) = c.g;
		*(fb + location + 3) = c.b;
		break;
	case FB_FORMAT_RGB888:
		*(fb + location) = c.b;
		*(fb + location + 1) = c.g;
		*(fb + location + 2) = c.r;
		break;
	case FB_FORMAT_RGB565:
		*(volatile unsigned short *)(fb + location) = ((c.r >> 3) << 11) |
							      ((c.g >> 2) << 5)  |
							       (c.b >> 3);
		break;
	}
}

#define SCALED_FONTW (FONTW * fb_info->scale_f)
#define SCALED_FONTH (FONTH * fb_info->scale_f)

int get_font_scale_factor()
{
	if (!fb_info)
		return 1;

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

	/* determine scale factor from pseudo-diagonal */
	int scale = x / 750;

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
			current_y = FB_TEXT_TOP_PADDING;
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

static const color level_color[] = {
	[KERN_EMERG]	= {255,   0,   0, 255},
	[KERN_ALERT]	= {255, 165,   0, 255},
	[KERN_CRIT]	= {255, 255,   0, 255},
	[KERN_ERR]	= {255,   0,   0, 255},
	[KERN_WARNING]	= {255, 255,   0, 255},
	[KERN_NOTICE]	= {  0, 255,   0, 255},
	[KERN_INFO]	= {255, 192,   0, 255},
	[KERN_DEBUG]	= {128, 128, 128, 255},
};
static const color body_color = {128, 128, 128, 255};

static void simplefb_console_write(int level, const char *prefix, const char *msg)
{
	color pc = (level >= 0 && level < (int)ARRAY_SIZE(level_color)) ?
		level_color[level] : (color){255, 255, 255, 255};
	int prefix_width = strlen(prefix) * FONTW * get_font_scale_factor();

	__simplefb_raw_print(prefix, 0, FB_TEXT_TOP_PADDING, pc);
	__simplefb_raw_print(msg, prefix_width, FB_TEXT_TOP_PADDING, body_color);
}

static const struct console simplefb_console = {
	.write = simplefb_console_write,
};

static int simplefb_probe(void *data)
{
	fb_info = data;

	clean_fbmem((char*)fb_info->address, fb_info->width, fb_info->height,
		 fb_info->stride);

	fb_info->scale_f = get_font_scale_factor();

	console_register(&simplefb_console);
	printk(KERN_INFO, "simplefb: ready (%dx%d)\n",
	       fb_info->width, fb_info->height);

	return 0;
}

DRIVER_REGISTER("simplefb", simplefb_probe);
