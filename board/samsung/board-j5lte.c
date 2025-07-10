/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <board.h>
#include <main.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define PIPE_SSPP_SRC_FORMAT                    0x30
#define PIPE_SSPP_SRC_UNPACK_PATTERN            0x34
#define PIPE_BASE				0x1A15000
#define PIPE_SSPP_SRC_YSTRIDE			0x24

#define MDP_CTL_0_BASE                          0x1A02000
#define MDP_CTL_FLUSH				0x18

int j5lte_init(void)
{
	/* TODO: Doesn't really work :P */
	writel(0x000236FF, PIPE_BASE + PIPE_SSPP_SRC_FORMAT);
	writel(0x03020001, PIPE_BASE + PIPE_SSPP_SRC_UNPACK_PATTERN);
	writel((720 * 4), MDP_CTL_0_BASE + MDP_CTL_FLUSH);
	writel((1 << (0)), PIPE_BASE + PIPE_SSPP_SRC_YSTRIDE);

	return 0;
}

int j5lte_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 720,
		.height = 1280,
		.stride = 3,
		.address = (void *)0x8e000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

int j5lte_late_init(void)
{
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-j5lte",
	.ops = {
		.early_init = j5lte_init,
		.drivers_init = j5lte_drv,
		.late_init = j5lte_late_init,
	},
	.quirks = 0
};
