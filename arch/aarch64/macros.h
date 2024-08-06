/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

.macro ENTRY name
	.globl \name
	.type \name, %function
	.align 2
	\name:
.endm

.macro ENDPROC name
	.size \name, . - \name
.endm
