// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <lib/debug.h>
#include <main/boot.h>
#include <main/boot-fdt.h>
#include <string.h>

#include <lib/libfit/fit.h>

void boot_kernel_fit(void* fit)
{
	void *kernel, *dt, *ramdisk;

	// FIT: error handling is done inside, so we don't need more here
	if (fit_open(fit) != 0)
		return;

	if (fit_get_configuration(fit) != 0)
		return;

	if (fit_boot_configuration(fit) != 0)
		return;

	// Now we are ready to get images
	kernel = (void*)fit_get_kernel_base();
	dt = (void*)fit_get_fdt_base();
	ramdisk = (void*)fit_get_ramdisk_base();

#ifdef CONFIG_LIBFDT
	patch_dtb(&dt);
#endif

	printk(KERN_INFO, "Booting kernel...\n");
	printk(KERN_INFO, "Kernel @ 0x%x...\n", kernel);
	printk(KERN_INFO, "DT @ 0x%x...\n", dt);
	arch_load_kernel(kernel, dt, ramdisk);
}
