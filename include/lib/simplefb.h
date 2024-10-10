// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#ifndef SIMPLEFB_H_	/* Include guard */
#define SIMPLEFB_H_

#include <stdint.h>

extern void __simplefb_raw_print(volatile char *fb, char *text, int text_x,
				 int text_y, int width, int stride);
extern void simplefb_probe(void *data);
extern struct video *video_info;

struct video {
	int width;
	int height;
	int stride;
	void *address;
};

typedef struct _color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} color;

typedef struct _font_params {
	int width;
	int height;
} font_params;

font_params get_font_params(void);

#endif // SIMPLEFB_H_
