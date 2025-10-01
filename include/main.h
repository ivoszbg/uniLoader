// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>

#ifndef MAIN_H_   /* Include guard */
#define MAIN_H_

extern unsigned long kernel_size;
extern unsigned long ramdisk_size;

#define HANG() while(1) {}

/*
 * Some initcalls (for example late_init) are not used
 * on any devices (other than a stub return function).
 * As such, all initcalls are optional, and only ran if they exist.
 * However, if they do exist, getting a positive return value is needed.
 */
#define INITCALL(_call) \
	do { \
		if (_call && _call()) { \
			printk(KERN_EMERG, "initcall %s() failed\n", #_call); \
			HANG(); \
		} \
	} while (0)

#ifdef __aarch64__
extern void load_kernel_and_jump(void* dtb, void* x1, void* x2, void* x3,
				 void* kernel);
#elif __arm__
extern void load_kernel_and_jump(unsigned int r0, unsigned int r1,
                                 void* dtb_addr, void* kernel_entry);
#endif

extern void soc_init(void);

#endif // MAIN_H_
