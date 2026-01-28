// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#ifndef SIMPLEFB_H_	/* Include guard */
#define SIMPLEFB_H_

#include <stdint.h>

typedef struct _color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} color;

typedef enum {
	FB_FORMAT_RGB888,
	FB_FORMAT_ARGB8888,
	FB_FORMAT_BGRA8888,
} video_format;

struct video_info {
	video_format format;
	int width;
	int height;
	int stride;
	int scale;
	int scale_f;
	void *address;
};

extern void __simplefb_raw_print(const char *text, int text_x, int text_y,
				 color text_color);
extern void simplefb_probe(void *data);

int get_font_scale_factor(void);

typedef struct _font_params {
	int width;
	int height;
} font_params;

#endif // SIMPLEFB_H_
