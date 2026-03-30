/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, schoosh212 <superaviation001@gmail.com>
 */

#include <soc/exynos3830.h>

void soc_init(void) {
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
