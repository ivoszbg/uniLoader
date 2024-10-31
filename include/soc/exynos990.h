/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, halal-beef <78730004+halal-beef@users.noreply.github.com>
 */

#ifndef EXYNOS990_H_	/* Include guard */
#define EXYNOS990_H_

#define DECON_F_BASE		0x19050000
#define HW_SW_TRIG_CONTROL	0x70

#define SPEEDY_0_BASE		0x15940000

/* All exynos990 devices use S2MPS_19_22 */

/* S2MPS19 slave address */
#define S2MPS19_PM_ADDR		0x1

/* S2MPS19 Register Address */
#define S2MPS19_PM_LDO2M_CTRL	0x047
#define S2MPS19_PM_LDO15M_CTRL	0x054

/* LDOx_CTRL */
#define S2MPS_OUTPUT_ON_NORMAL	(0x3 << 6)

#endif // EXYNOS990_H_
