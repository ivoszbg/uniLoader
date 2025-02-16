/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, famfo <famfo@famfo.xyz>
 */

#include <soc/exynos3830.h>

void soc_init(void) {
        *(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
