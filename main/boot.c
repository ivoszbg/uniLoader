// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <lib/debug.h>
#include <main/boot.h>
#include <main/boot-fdt.h>
#include <string.h>

void boot_kernel(void* dt, void* kernel, void* ramdisk)
{
#ifdef CONFIG_LIBFDT
	patch_dtb(dt);
#endif

	printk(KERN_INFO, "Booting kernel...\n");
	arch_load_kernel(kernel, dt, ramdisk);
}
