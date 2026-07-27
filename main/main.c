// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/console.h>
#include <main/boot.h>
#include <main/main.h>
#include <lib/debug.h>

extern struct board_data board_ops;

static void print_splash(void)
{
	printk(KERN_INFO, "             .__.____                     .___\n");
	printk(KERN_INFO, " __ __  ____ |__|    |    _________     __| _/___________\n");
	printk(KERN_INFO, "|  |  \\/    \\|  |    |   /  _ \\__  \\   / __ |/ __ \\_  __\\\n");
	printk(KERN_INFO, "|  |  /   |  \\  |    |__(  <_> ) __ \\_/ /_/ \\  ___/|  |\\/\n");
	printk(KERN_INFO, "|____/|___|  /__|_______ \\____(____  /\\____ |\\___  >__|\n");
	printk(KERN_INFO, "           \\/           \\/         \\/      \\/    \\/\n");

	printk(KERN_INFO, "passed board initialization\n");
	printk(KERN_INFO, "welcome to uniLoader %s on %s\n", VER_TAG, board_ops.name);
}

static void hw_init(void)
{
	early_console_init();
	INITCALL(board_ops.ops.early_init);

	driver_probe_all(board_ops.devices, board_ops.num_devices);

	print_splash();

	INITCALL(board_ops.ops.late_init);
}

#ifdef CONFIG_FIT_BOOT

void boot_kernel_fit(void* fit);

void fit_main(void* fit)
{
	hw_init();

	printk(KERN_INFO, "Booting using FIT image at 0x%p\n", fit);

	boot_kernel_fit(fit);

	// todo: reset the board?
	printk(KERN_EMERG, "Something wrong happened, we shouldn't be here. Hanging....\n");
	HANG();
}
#else
void main(void* dt, void* kernel, void* ramdisk)
{
	hw_init();

	printk(KERN_INFO, "Booting using legacy image method\n");

	boot_kernel(dt, kernel, ramdisk);

	// todo: reset the board?
	printk(KERN_EMERG, "Something wrong happened, we shouldn't be here. Hanging....\n");
	HANG();
}
#endif
