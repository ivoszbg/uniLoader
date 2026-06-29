// SPDX-License-Identifier: GPL-2.0
/*
 * CPUXGPT driver for early aarch64 Mediatek platforms
 * Copyright (c) 2026, Alexandru Chimac <alex@chimac.ro>
 *
 * Based on https://patchwork.kernel.org/project/linux-mediatek/patch/20220509210741.12020-3-angelogioacchino.delregno@collabora.com/
 * Copyright (c) 2022, AngeloGioacchino Del Regno <angelogioacchino.delregno@collabora.com>
 */

#include <lib/debug.h>
#include <drivers/framework.h>
#include <drivers/mediatek/mcusys.h>

/* for MCUSYS wrapper */
#define CPUX_CON_REG 0x0
#define CPUX_IDX_REG 0x4

/* CPUXGPT */
#define CPUX_IDX_GLOBAL_CTRL 0x0
#define CPUX_ENABLE          (1 << 0)
#define CPUX_CLK_DIV1        (1 << 8)
#define CPUX_CLK_DIV2        (1 << 9)
#define CPUX_CLK_DIV4        (1 << 10)
#define CPUX_IDX_GLOBAL_IRQ  0x30

static int cpuxgpt_init(void *data)
{
	uint32_t val;

	struct mcusys_params cpuxgpt_mcusys = {
		.address = (phys_addr_t) data,
		.con_reg = CPUX_CON_REG,
		.idx_reg = CPUX_IDX_REG,
	};

	/* Initialize CPUX timers */
	/* Set DIV2 to achieve 13MHz clock */
	val = mcusys_readl(cpuxgpt_mcusys, CPUX_IDX_GLOBAL_CTRL);
	val &= ~(CPUX_CLK_DIV1 | CPUX_CLK_DIV2 | CPUX_CLK_DIV4);
	val |= CPUX_CLK_DIV2;
	mcusys_writel(val, cpuxgpt_mcusys, CPUX_IDX_GLOBAL_CTRL);

	/* Enable all CPUX timers */
	val = mcusys_readl(cpuxgpt_mcusys, CPUX_IDX_GLOBAL_CTRL);
	mcusys_writel(val | CPUX_ENABLE, cpuxgpt_mcusys, CPUX_IDX_GLOBAL_CTRL);

	printk(KERN_INFO, "cpuxgpt: ready\n");

	return 0;
}

DRIVER_REGISTER("mediatek,mt6795-systimer", cpuxgpt_init);
