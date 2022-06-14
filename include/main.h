/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef MAIN_H_   /* Include guard */
#define MAIN_H_

extern unsigned long kernel_size;
extern void load_kernel(void* dtb, void* x1, void* x2, void* x3, void* kernel);
extern void soc_init(void);
extern void board_init(void);

extern void printk(char *text);

/* Define our own 128 bit memcpy */
void memcpy(void *dest, void *src, int size)
{
	unsigned __int128 *src2 = src;
	unsigned __int128 *dest2 = dest;

	for (int i=0; i<size/16; i++)
		dest2[i] = src2[i];
}

#endif // MAIN_H_
