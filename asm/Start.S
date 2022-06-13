/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

    adr	x0, dtb
    adr	x1, kernel
    adrp x2, end
    b main

.global load_kernel
load_kernel:
    br	x4
