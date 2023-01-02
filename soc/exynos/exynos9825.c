/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023, Leandro Friedrich <email@leandrofriedrich.de>
 */

#include <soc/exynos9825.h>

void soc_init(void) {
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
