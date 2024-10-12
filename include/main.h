// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>

#ifndef MAIN_H_   /* Include guard */
#define MAIN_H_

extern unsigned long kernel_size;
extern unsigned long ramdisk_size;
extern void load_kernel(void* dtb, void* x1, void* x2, void* x3, void* kernel);
extern void soc_init(void);

extern void writel(unsigned int value, void* address);

#endif // MAIN_H_
