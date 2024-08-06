/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
.include "arch/aarch64/macros.h"

.section .text
ENTRY _start
	/* Set up the base address for the stack */
	adr x0, _stack_end

	/* Set up the stack pointer (SP) */
	mov sp, x0

	/* Fall through */
	adr	x0, dtb
	adr	x1, kernel
	b main

ENTRY load_kernel
	br	x4
ENDPROC load_kernel
