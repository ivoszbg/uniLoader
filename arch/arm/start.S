/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include "asmdefs.h"

.section .text
ENTRY(_start)
	/* disable interrupts */
	mrs r0, cpsr
	orr r0, r0, #(0x1 << 7)
	orr r0, r0, #(0x1 << 6)
	msr cpsr_c, r0

	mrc     p15, 0, r12, c1, c0, 0
	/* i/d cache disable, mmu disabled */
	bic     r12, #(1<<12)
	bic     r12, #(1<<2 | 1<<0)
	mcr     p15, 0, r12, c1, c0, 0

	ldr sp, =__stack_top

	/* Zero BSS section */
	ldr r0, =__bss_start
	ldr r1, =__bss_end
	mov r2, #0
1:
	cmp r0, r1
	bge 2f
	str r2, [r0], #4
	b 1b
2:

	/* main(void* dt, void* kernel, void* ramdisk) */
	ldr r0, =_dt_start		@ Load address of dtb into r0
	ldr r1, =_kernel_start		@ Load address of kernel into r1
	ldr r2, =_ramdisk_start		@ Load address of ramdisk into r2
	b main
END(_start)

ENTRY(load_kernel_and_jump)
	mov r1, #0      @ machine type = 0 (device tree)
	mov r0, #0      @ reserved
	bx r3
END(load_kernel_and_jump)

/* embedding kernel and dt */
.section .rodata
ENTRY(_kernel_start)
	.incbin "blob/Image"
END(_kernel_start)

ENTRY_ALIAS(_kernel_end)

ENTRY_ALIAS(kernel_size)
	.word _kernel_end - _kernel_start

ENTRY(_dt_start)
	.incbin "blob/dtb"
END(_dt_start)

ENTRY(_ramdisk_start)
	.incbin "blob/ramdisk"
END(_ramdisk_start)

ENTRY_ALIAS(_ramdisk_end)

ENTRY_ALIAS(ramdisk_size)
	.word _ramdisk_end - _ramdisk_start

