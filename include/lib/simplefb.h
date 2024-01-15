/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#ifndef SIMPLEFB_H_	/* Include guard */
#define SIMPLEFB_H_

typedef struct _color {
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
} color;

typedef struct _font_params {
	int width;
	int height;
} font_params;

font_params get_font_params(void);

void draw_pixel(volatile char *fb, int x, int y, int width, int stride, color c);

void draw_text(volatile char *fb, char *text, int textX, int textY, int width, int stride);

void draw_horizontal_line(volatile char *fb, int x1, int x2, int y, color c, int width, int stride);

void draw_vertical_line(volatile char *fb, int x, int y1, int y2, color c, int width, int stride);

void draw_filled_rectangle(volatile char *fb, int x1, int y1, int w, int h, color c, int width, int stride);

#endif
