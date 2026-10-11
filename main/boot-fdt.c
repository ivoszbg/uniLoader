// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <drivers/ramdisk-handler.h>
#include <lib/debug.h>

#if CONFIG_FDT_BUF_ADDR
#define fdt_buf ((char *)CONFIG_FDT_BUF_ADDR)
#else
static char fdt_buf[CONFIG_FDT_BUF_SIZE] __attribute__((aligned(8)));
#endif

void patch_dtb(void** dt)
{
	int ret;

	ret = ramdisk_handler_patch_dtb(*dt, fdt_buf, CONFIG_FDT_BUF_SIZE);
	if (ret != 0)
		printk(KERN_ERR, "failed to patch dtb\n");
	else
		*dt = fdt_buf;
}
