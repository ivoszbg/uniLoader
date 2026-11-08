/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Umer Uddin <umer.uddin@mentallysanemainliners.org>
 */

#include <soc/exynos990.h>

void soc_init(void) {
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
