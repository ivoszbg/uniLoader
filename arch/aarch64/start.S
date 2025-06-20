/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include "asmdefs.h"

_head:
        mov x1,_start
        b      _start

_stack_base:
        .quad 0x0

_stack_size:
         .quad _stack_end
         .quad 0
         .quad 0
         .quad 0
         .quad 0
         .ascii "ARM\x64"
         .long 0

.align 4
.section .text
ENTRY(_start)
	/* Set up the base address for the stack */
	adrp    x0, _stack_end      /* Load page of the _stack_end symbol */
	add     x0, x0, #:lo12:_stack_end  /* Add the offset within the page */

	/* Set up the stack pointer (SP) */
	mov sp, x0

	/* Fall through */
	adrp    x0, dtb             /* Load page of dtb */
	add     x0, x0, #:lo12:dtb  /* Add offset within the page */
	adrp    x1, kernel          /* Load page of kernel */
	add     x1, x1, #:lo12:kernel /* Add offset within the page */
	adrp    x2, ramdisk          /* Load page of kernel */
	add     x2, x2, #:lo12:ramdisk /* Add offset within the page */

	bl main
END(_start)

ENTRY(load_kernel)
	br	x4
END(load_kernel)
