/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>

#ifndef MAIN_H_   /* Include guard */
#define MAIN_H_

extern unsigned long kernel_size;
extern void load_kernel(void* dtb, void* x1, void* x2, void* x3, void* kernel);
extern void soc_init(void);
extern void board_init(void);

extern void printk(char *text);

#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_memcpy)
	void memcpy(void *dest, void *src, size_t size) {
		__builtin_memcpy(dest, src, size);
	}
#else
	#ifdef __aarch64__
		/* Define our own 128 bit memcpy */
		void memcpy(void *dest, void *src, size_t size)
		{
			unsigned __int128 *src2 = src;
			unsigned __int128 *dest2 = dest;

			for (size_t i = 0; i < size / 16; i++)
				dest2[i] = src2[i];
		}
	#endif

	#ifdef __arm__
		void memcpy (char* src, char* dest, size_t len) {
			for (size_t i = 0; i < len; i++)
				dest[i] = src[i];
		}
	#endif
#endif



#endif // MAIN_H_
