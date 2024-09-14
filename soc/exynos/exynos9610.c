// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Alexandru Chimac <alexchimac@protonmail.com>
 */

#include <soc/exynos9610.h>

void soc_init(void) {
	*(int *) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
