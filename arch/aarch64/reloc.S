/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include "asmdefs.h"

.extern _reloc_size
.extern _start

ENTRY_ALIAS(_head)
	adr	x1, _relocated_uni
	b	 _reloc_entry

ENTRY_ALIAS(_rel_stack_base)
	.quad _start

ENTRY_ALIAS(_rel_stack_size)
	.quad _reloc_size
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.ascii "ARM\x64"
	.long 0

ENTRY_ALIAS( _reloc_entry)
	adr	x4, _start
	ldr	x5, _rel_stack_base
	cmp	x4, x5
	beq	_entry
	ldr	x6,  _rel_stack_size

_copy:
	ldp	x2, x3, [x4], #16
	stp	x2, x3, [x5], #16
	subs	x6, x6, #16
	b.ne	_copy
	ldr	x5, _rel_stack_base

_entry:
	br	x5

_dead: /* we should never get here */
	b	_dead

ENTRY_ALIAS(_relocated_uni)
