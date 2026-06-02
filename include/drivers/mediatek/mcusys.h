// SPDX-License-Identifier: GPL-2.0
/*
 * Helper functions for Mediatek MCUSYS
 * Copyright (c) 2026, Alexandru Chimac <alex@chimac.ro>
 */

#include <limits.h>

struct mcusys_params {
	phys_addr_t address;
	uint32_t idx_reg;
	uint32_t con_reg;
};

extern uint32_t mcusys_readl(struct mcusys_params params, uint32_t offset);
extern void mcusys_writel(uint32_t value, struct mcusys_params params, uint32_t offset);
