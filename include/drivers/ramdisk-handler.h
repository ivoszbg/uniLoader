// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025, Victor Paul <vipollmail@gmail.com>
 */

#ifndef RAMDISK_HANDLER_H_   /* Include guard */
#define RAMDISK_HANDLER_H_

extern unsigned long ramdisk_size;

int ramdisk_handler_patch_dtb(const void *fdt, void *buf, int bufsize);

#endif
