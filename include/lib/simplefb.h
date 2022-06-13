/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2022, Markuss Broks <markuss.broks@gmail.com>
 * Copyright (c) 2022, Michael Srba <Michael.Srba@seznam.cz>
 */

#ifndef SIMPLEFB_H_	/* Include guard */
#define SIMPLEFB_H_

typedef struct _color {
	int r;
	int g;
	int b;
} color;

typedef struct _font_params {
	int width;
	int height;
} font_params;

font_params get_font_params(void);

long unsigned int strlen(const char *p) {
	unsigned int i = 0;

	while(*p != '\0') {
		i++;
		p++;
	}

    return i;
}

#endif
