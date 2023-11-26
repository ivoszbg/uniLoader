/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023, BotchedRPR <igor@botchedrpr.com>
*/

#include <soc/exynosw920.h>

void soc_init(void) {
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
}
