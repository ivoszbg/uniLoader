/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Faiz Faadhillah <faiz.faadhillah@gmail.com>
 */

#include <soc/exynos7420.h>

void soc_init(void) {
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x2058;
}
