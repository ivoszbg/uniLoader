// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * ARM32 QEMU virt board for J1 XNU bring-up.  QEMU "virt" machine,
 * -cpu cortex-a7, 1GB RAM @0x40000000.
 *
 * No framebuffer: all diagnostics go out the emulated PL011.
 */

#include <board.h>
#include <drivers/framework.h>
#include <stdint.h>

#define PL011_BASE	0x09000000
#define PL011_DR	(PL011_BASE + 0x000)
#define PL011_FR	(PL011_BASE + 0x018)
#define PL011_FR_TXFF	(1 << 5)

#ifdef CONFIG_EARLYCON
void uart_putc(char ch)
{
	volatile unsigned int *fr = (volatile unsigned int *)PL011_FR;

	while (*fr & PL011_FR_TXFF)
		;
	*(volatile unsigned int *)PL011_DR = (unsigned int)(ch & 0xff);
}

void uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
}
#endif

static void enable_fpu(void)
{
	uint32_t cpacr;

	__asm__ volatile("mrc p15, 0, %0, c1, c0, 2" : "=r"(cpacr));
	cpacr |= (3u << 20);
	__asm__ volatile("mcr p15, 0, %0, c1, c0, 2" :: "r"(cpacr));
}

int virt_arm_init(void)
{
	enable_fpu();
	return 0;
}

struct board_data board_ops = {
	.name = "qemu-virt-arm",
	.ops = {
		.early_init = virt_arm_init,
	},
	.quirks = 0
};