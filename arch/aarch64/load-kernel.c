// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <main/boot.h>
#include <string.h>

#include <lib/debug.h>

void arch_load_kernel(void* kernel, void* dt, void* ramdisk)
{
	unsigned long _kernel_size = GET_KERNEL_SIZE();
	unsigned long _ramdisk_size = GET_RAMDISK_SIZE();

#ifndef CONFIG_FIT_BOOT
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &_kernel_size);
#ifndef CONFIG_RAMDISK_NO_COPY
	__optimized_memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) &_ramdisk_size);
#endif // CONFIG_RAMDISK_NO_COPY
#else
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) _kernel_size);
	__optimized_memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) _ramdisk_size);
#endif
	load_kernel_and_jump(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);
}
