// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <string.h>
#include <lib/debug.h>

extern struct board_data board_ops;

void main(void* dt, void* kernel, void* ramdisk)
{
	if (board_ops.ops.early_init())
		goto err;

	if (board_ops.ops.drivers_init())
		goto err;

	printk(-1, "             .__.____                     .___\n");
	printk(-1, " __ __  ____ |__|    |    _________     __| _/___________\n");
	printk(-1, "|  |  \\/    \\|  |    |   /  _ \\__  \\   / __ |/ __ \\_  __\\\n");
	printk(-1, "|  |  /   |  \\  |    |__(  <_> ) __ \\_/ /_/ \\  ___/|  |\\/\n");
	printk(-1, "|____/|___|  /__|_______ \\____(____  /\\____ |\\___  >__|\n");
	printk(-1, "           \\/           \\/         \\/      \\/    \\/\n");

	printk(-1, "passed board initialization\n");
	printk(-1, "welcome to uniLoader %s\n", VER_TAG);

	if (board_ops.ops.late_init())
		goto err;

	/* copy kernel to memory and boot  */
	printk(-1, "booting linux...\n");
#ifdef __aarch64__
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
	__optimized_memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) &ramdisk_size);
	load_kernel_and_jump(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);
#elif __arm__
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) kernel_size);
	memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) ramdisk_size);
	load_kernel_and_jump(0, 0, dt, (void*)CONFIG_PAYLOAD_ENTRY);
#endif

 err:
	/* we shouldn't get there */
	/* TODO: make this a per-board reset */
	printk(KERN_WARNING, "Something wrong happened, we shouldn't be here. Hanging....\n");
	while(1) {}
}
