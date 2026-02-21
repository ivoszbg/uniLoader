// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#ifndef BOOT_H_
#define BOOT_H_

extern unsigned long kernel_size;
extern unsigned long ramdisk_size;

#ifdef __aarch64__
extern void load_kernel_and_jump(void* dtb, void* x1, void* x2, void* x3,
                                 void* kernel);
#elif __arm__
extern void load_kernel_and_jump(unsigned int r0, unsigned int r1,
                                 void* dtb_addr, void* kernel_entry);
#endif

void boot_kernel(void* dt, void* kernel, void* ramdisk);

#endif // BOOT_H_
