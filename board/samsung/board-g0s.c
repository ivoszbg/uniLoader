/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <drivers/samsung/exynos-acpm.h>
#include "s2mps25-26-pmic.h"

/* DECON Register MAP */
#define HW_SW_TRIG_CONTROL	0x30

/* MMIO MAP */
#define EXYNOS_MAILBOX_AP2APM	0x15900000
#define DECON_F_BASE		0x19f00000

void init_board_funcs(void *board)
{
	/*
	 * Parsing the struct directly without restructing is
	 * broken as of Sep 29 2024
	 */
	struct {
		const char *name;
		int ops[BOARD_OP_EXIT];
	} *board_restruct = board;

	board_restruct->name = "G0S";
}

/* WDT Register MAP */
#define S5E9925_WDT_BASE				0x10050000
#define S5E9925_WDT_WTCON				(S5E9925_WDT_BASE + 0x0000)
#define S5E9925_WDT_WTCLRINT				(S5E9925_WDT_BASE + 0x000C)
#define S5E9925_WDT_PRESCALE(x)				((x) << 8)
#define S5E9925_WDT_PRESCALE_MASK			(0xff << 8)

#define S5E9925_WDT_WTCON_RSTEN				(1 << 0)
#define S5E9925_WDT_WTCON_ENABLE			(1 << 5)

#define S5E9925_CLUSTER0_NONCPU_INT_EN			(0x15860000 + 0x1244)
#define S5E9925_nWDTRESET_EN				(1 << 2)

void kill_dog(void)
{
	unsigned long wtcon, pmu_reg;

	pmu_reg = readl((volatile unsigned int *) S5E9925_CLUSTER0_NONCPU_INT_EN);
	pmu_reg &= ~S5E9925_nWDTRESET_EN;
	writel(pmu_reg, (void *) S5E9925_CLUSTER0_NONCPU_INT_EN);

	wtcon = readl((volatile unsigned int *) S5E9925_WDT_WTCON);
	wtcon &= ~(S5E9925_WDT_WTCON_ENABLE | S5E9925_WDT_WTCON_RSTEN);
	writel(wtcon, (void *) S5E9925_WDT_WTCON);

	wtcon = readl((volatile unsigned int *) S5E9925_WDT_WTCON);
	writel(1, (void *) S5E9925_WDT_WTCLRINT);
}

#define S2MPS25_REG_ENABLE		(0x3 << 6)

/* (LV) LDOs 1M/2M/4M/5M/6M/7M/8M/9M/10M/11M/14M/19M */
#define S2MPS25_LDO_MIN1	300000
#define S2MPS25_LDO_STEP1	6250
/* (IV) LDOs 3M/12M/13M/15M/16M/20M/21M/22M/23M/24M/25M/
	     26M/27M/28M/29M/30M */
#define S2MPS25_LDO_MIN2	1500000
#define S2MPS25_LDO_STEP2	12500
/* (MV) LDOs 14S ~ 20S */
#define S2MPS26_LDO_MIN2	1800000
#define S2MPS26_LDO_STEP2	25000

void pmic_init(void)
{
	unsigned char reg;
	void *mailbox_base = (void *)EXYNOS_MAILBOX_AP2APM;

	spmi_read(mailbox_base, SUB_PMIC_ID, S2MPS26_PM1_ADDR, S2MPS26_REG_LDO17S_CTRL, &reg);
	reg = S2MPS25_REG_ENABLE + (3000000 - S2MPS26_LDO_MIN2) / S2MPS26_LDO_STEP2;
	spmi_write(mailbox_base, SUB_PMIC_ID, S2MPS26_PM1_ADDR, S2MPS26_REG_LDO17S_CTRL, reg);

	spmi_read(mailbox_base, 0, S2MPS25_PM1_ADDR, S2MPS25_REG_LDO28M_CTRL, &reg);
	reg = S2MPS25_REG_ENABLE + (1800000 - S2MPS25_LDO_MIN2) / S2MPS25_LDO_STEP2;
	spmi_write(mailbox_base, 0, S2MPS25_PM1_ADDR, S2MPS25_REG_LDO28M_CTRL, reg);
}

// Early initialization
int board_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;

	kill_dog();
	pmic_init();

	return 0;
}

// Late initialization
int board_late_init(void)
{
	return 0;
}

int board_driver_setup(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1080,
		.height = 2340,
		.stride = 4,
		.address = (void *)0xf6200000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}
