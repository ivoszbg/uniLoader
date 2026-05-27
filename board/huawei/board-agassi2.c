/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Board driver for Huawei Mediapad T5
 *
 * Copyright (c) 2026, Tildeguy <tildeguy@mainlining.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info agassi2_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1200,
	.height = 1920,
	.stride = 4,
	.address = (void *)0x5d000000,
	.scale = 2
};

static const struct device agassi2_devices[] = {
	{ "simplefb", &agassi2_fb, "fb" },
};

int agassi2_early_init(void)
{
	unsigned long cpacr;
	__asm__ volatile ("mrs %0, cpacr_el1" : "=r" (cpacr));
	cpacr |= (3UL << 20);
	__asm__ volatile ("msr cpacr_el1, %0" :: "r" (cpacr));
	return 0;
}

struct board_data board_ops = {
	.name = "huawei-agassi2",
	.ops = {
		.early_init = agassi2_early_init,
	},
	.devices = agassi2_devices,
	.num_devices = ARRAY_SIZE(agassi2_devices),
	.quirks = 0
};
