// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <string.h>
#include <lib/debug.h>
#include <drivers/ramdisk-handler.h>

extern struct board_data board_ops;

#ifdef CONFIG_LIBFDT
static char fdt_buf[CONFIG_FDT_BUF_SIZE];
#endif

void main(void* dt, void* kernel, void* ramdisk)
{
	int ret;

	INITCALL(board_ops.ops.early_init);
	INITCALL(board_ops.ops.drivers_init);

	printk(KERN_INFO, "             .__.____                     .___\n");
	printk(KERN_INFO, " __ __  ____ |__|    |    _________     __| _/___________\n");
	printk(KERN_INFO, "|  |  \\/    \\|  |    |   /  _ \\__  \\   / __ |/ __ \\_  __\\\n");
	printk(KERN_INFO, "|  |  /   |  \\  |    |__(  <_> ) __ \\_/ /_/ \\  ___/|  |\\/\n");
	printk(KERN_INFO, "|____/|___|  /__|_______ \\____(____  /\\____ |\\___  >__|\n");
	printk(KERN_INFO, "           \\/           \\/         \\/      \\/    \\/\n");

	printk(KERN_INFO, "passed board initialization\n");
	printk(KERN_INFO, "welcome to uniLoader %s on %s\n", VER_TAG, board_ops.name);

	INITCALL(board_ops.ops.late_init);

#ifdef CONFIG_LIBFDT
	ret = ramdisk_handler_patch_dtb(dt, &fdt_buf, sizeof(fdt_buf));
	if (ret == 0)
		dt = &fdt_buf;
	else
		printk(KERN_ERR, "failed to patch dtb\n");
#endif

	/* copy kernel to memory and boot  */
	printk(KERN_INFO, "booting linux...\n");
#ifdef __aarch64__
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
#ifndef CONFIG_RAMDISK_NO_COPY
	__optimized_memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) &ramdisk_size);
#endif
	load_kernel_and_jump(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);
#elif __arm__
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) kernel_size);
#ifndef CONFIG_RAMDISK_NO_COPY
	memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) ramdisk_size);
#endif
	load_kernel_and_jump(0, 0, dt, (void*)CONFIG_PAYLOAD_ENTRY);
#endif

	/* we shouldn't get there */
	/* TODO: make this a per-board reset */
	printk(KERN_EMERG, "Something wrong happened, we shouldn't be here. Hanging....\n");
	HANG();

}
