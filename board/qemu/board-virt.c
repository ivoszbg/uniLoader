/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

volatile unsigned int* uart = (unsigned int*)0x9000000;

void uart_putc(char ch)
{
	*uart = ch;
}

void uart_puts(const char *s)
{
	while (*s != '\0') 
	{
		uart_putc(*s);
		s++;
	}
}

void enable_fpu(void)
{
	unsigned long cpacr;
	__asm__ volatile ("mrs %0, cpacr_el1" : "=r" (cpacr));
	cpacr |= (3UL << 20);
	__asm__ volatile ("msr cpacr_el1, %0" :: "r" (cpacr));
}

int virt_init(void)
{
	// TODO: We can implement a framebuffer with QEMU. This would however
	// require more setup.
	enable_fpu();
	return 0;
}

int virt_late_init(void)
{
	return 0;
}

int virt_drv(void)
{
	return 0;
}

struct board_data board_ops = {
	.name = "qemu-virt",
	.ops = {
		.early_init = virt_init,
		.drivers_init = virt_drv,
		.late_init = virt_late_init,
	},
	.quirks = 0
};
