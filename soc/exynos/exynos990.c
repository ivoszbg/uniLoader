/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, halal-beef <78730004+halal-beef@users.noreply.github.com>
 */

#include <soc/exynos990.h>

void soc_init(void) {
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
