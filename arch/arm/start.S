/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
.global _start

_start:
    ldr r0, =dtb
    ldr r1, =kernel
    b main

.global load_kernel
load_kernel:
    bx r4
