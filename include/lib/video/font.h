// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on https://github.com/torvalds/linux/blob/master/lib/fonts/font_8x16.c
 */

#ifndef FONT_H_	/* Include guard */
#define FONT_H_

#define FONTW 8
#define FONTH 16

// Define scaled font dimensions --> 8x16 becomes 16x32
#define SCALE_FACTOR 2
#define SCALED_FONTW (FONTW * SCALE_FACTOR)
#define SCALED_FONTH (FONTH * SCALE_FACTOR)

extern const unsigned char letters[256][16];
extern int font_index(char a);

#endif // FONT_H_
