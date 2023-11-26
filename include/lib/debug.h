/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef DEBUG_H_	/* Include guard */
#define DEBUG_H_

extern void draw_text(volatile char *fb, char *text, int textX, int textY, int width, int stride);
long int debug_linecount = CONFIG_FB_FIRST_LINE_OFFSET;

#endif
