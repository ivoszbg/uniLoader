// SPDX-License-Identifier: GPL-2.0
/*
 * DECON driver for Samsung Exynos SoCs
 * Copyright (c) 2026, Alexandru Chimac <alex@chimac.ro>
 */

#include <limits.h>
#include <string.h>
#include <lib/debug.h>
#include <drivers/framework.h>

/* for exynos8890 - exynos9830 */
#define HW_SW_TRIG_CONTROL 0x70
/* for exynos2100 -  */
#define TRIG_CON 0x30

#define HW_TRIG_EN (1 << 0)
#define HW_TRIG_MASK_DECON (1 << 4)

static int exynos_decon_init(void *address, uint32_t trig_con)
{
	uint32_t val;

	val = readl(address + trig_con);
	val |= HW_TRIG_EN;
	val &= ~HW_TRIG_MASK_DECON;
	writel(val, address + trig_con);

	printk(KERN_INFO, "decon: ready\n");
	return 0;
}

static int exynos9_decon_init(void *data)
{
	return exynos_decon_init(data, HW_SW_TRIG_CONTROL);
}

DRIVER_REGISTER("samsung,exynos9-decon", exynos9_decon_init);
