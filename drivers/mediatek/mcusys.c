// SPDX-License-Identifier: GPL-2.0
/*
 * Helper functions for Mediatek MCUSYS
 * Copyright (c) 2026, Alexandru Chimac <alex@chimac.ro>
 */

#include <string.h>
#include <drivers/mediatek/mcusys.h>

uint32_t mcusys_readl(struct mcusys_params params, uint32_t offset)
{
	writel(offset, (void *)(params.address + params.idx_reg));
	return readl((void *)(params.address + params.con_reg));
}

void mcusys_writel(uint32_t value, struct mcusys_params params, uint32_t offset)
{
	writel(offset, (void *)(params.address + params.idx_reg));
	writel(value, (void *)(params.address + params.con_reg));
}
