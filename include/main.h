// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>

#ifndef MAIN_H_   /* Include guard */
#define MAIN_H_

extern unsigned long kernel_size;
extern unsigned long ramdisk_size;

#ifdef __aarch64__
extern void load_kernel_and_jump(void* dtb, void* x1, void* x2, void* x3,
				 void* kernel);
#elif __arm__
extern void load_kernel_and_jump(unsigned int r0, unsigned int r1,
                                 void* dtb_addr, void* kernel_entry);
#endif

extern void soc_init(void);

#endif // MAIN_H_
