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

extern void fb_mark_asm(unsigned int byte_off, unsigned int color);

void main(void* dt, void* kernel, void* ramdisk)
{
	early_console_init();

	INITCALL(board_ops.ops.early_init);

	driver_probe_all(board_ops.devices, board_ops.num_devices);

	print_splash();

	INITCALL(board_ops.ops.late_init);

	/* Framebuffer console is live now - dump handover environment. */
	{
		extern unsigned int entered_hyp;
		unsigned int sctlr, cpacr, cpsr_v;
		static const unsigned int probes[] = {
			0x20630000, 0x20800000, 0x21000000, 0x22001000,
			0x24000000, 0x28000000, 0x30000000, 0x38000000,
			0x40000000, 0x48000000, 0x50000000, 0x55000000
		};

		__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0"
				     : "=r"(sctlr));
		__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 2"
				     : "=r"(cpacr));
		__asm__ __volatile__("mrs %0, cpsr" : "=r"(cpsr_v));
		printk(KERN_INFO, "cpu: CPSR=%08x SCTLR=%08x CPACR=%08x\n",
		       cpsr_v, sctlr, cpacr);
		printk(KERN_INFO, "entered_hyp=%u\n", entered_hyp);

#ifndef CONFIG_QEMU_BOARD_VIRT_ARM
		/* BRING-UP (J1): probe physical DRAM windows left free by
		 * S-Boot.  Not valid on QEMU virt - those addresses are
		 * outside the 0x40000000 RAM and would abort. */
		{
			unsigned int i;

			for (i = 0; i < sizeof(probes) / sizeof(probes[0]); i++) {
				volatile unsigned int *p =
					(volatile unsigned int *)probes[i];
				unsigned int v;

				p[0] = 0xA5A55A5A;
				p[1] = probes[i];
				v = p[0];
				printk(KERN_INFO, "memtest %08x: %s\n", probes[i],
				       (v == 0xA5A55A5A && p[1] == probes[i]) ?
						"OK" : "FAIL");
			}
		}
#endif
	}

	boot_kernel(dt, kernel, ramdisk);

	// todo: reset the board?
	printk(KERN_EMERG, "Something wrong happened, we shouldn't be here. Hanging....\n");
	HANG();
}
