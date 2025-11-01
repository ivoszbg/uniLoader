// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025, Victor Paul <vipollmail@gmail.com>
 */

#include <drivers/ramdisk-handler.h>
#include <lib/debug.h>
#include <lib/libfdt/libfdt.h>

int ramdisk_handler_patch_dtb(const void *fdt, void *buf, int bufsize)
{
	int ret, dtb_chosen_offset;
	void *ramdisk_end = (void*)CONFIG_RAMDISK_ENTRY + (unsigned long)&ramdisk_size;

	printk(KERN_INFO, "trying to open fdt...\n");
	ret = fdt_open_into(fdt, buf, bufsize);
	if (ret < 0) {
		printk(KERN_ERR, "fdt_open_into() failed: %s\n", fdt_strerror(ret));
		return ret;
	}

	dtb_chosen_offset = fdt_path_offset(buf, "/chosen");
	if (dtb_chosen_offset < 0) {
		printk(KERN_ERR, "Could not find /chosen node: %s\n", fdt_strerror(dtb_chosen_offset));
		return dtb_chosen_offset;
	}

	printk(KERN_INFO, "adding linux,initrd-start...\n");
#ifdef __aarch64__
	ret = fdt_setprop_u64(buf, dtb_chosen_offset, "linux,initrd-start", (uint64_t)CONFIG_RAMDISK_ENTRY);
#elif __arm__
	ret = fdt_setprop_u32(buf, dtb_chosen_offset, "linux,initrd-start", (uint32_t)CONFIG_RAMDISK_ENTRY);
#endif
	if (ret < 0) {
		printk(KERN_ERR, "failed to add linux,initrd-start: %s\n", fdt_strerror(ret));
		return ret;
	}

	printk(KERN_INFO, "adding linux,initrd-end...\n");
#ifdef __aarch64__
	ret = fdt_setprop_u64(buf, dtb_chosen_offset, "linux,initrd-end", (uint64_t)ramdisk_end);
#elif __arm__
	ret = fdt_setprop_u32(buf, dtb_chosen_offset, "linux,initrd-end", (uint32_t)ramdisk_end);
#endif
	if (ret < 0) {
		printk(KERN_ERR, "failed to add linux,initrd-end: %s\n", fdt_strerror(ret));
		return ret;
	}

	return 0;
}
