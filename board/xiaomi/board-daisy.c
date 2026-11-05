/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

void enable_fpu(void)
{
	unsigned long cpacr;
	__asm__ volatile ("mrs %0, cpacr_el1" : "=r" (cpacr));
	cpacr |= (3UL << 20);
	__asm__ volatile ("msr cpacr_el1, %0" :: "r" (cpacr));
}

// Early initialization
int daisy_init(void)
{
	enable_fpu();
	return 0;
}

static struct video_info daisy_fb = {
	.format = FB_FORMAT_RGB888,
	.width = 1080,
	.height = 2280,
	.scale = 2,
	.stride = 3,
	.address = (void *)0x90001000,
};

static const struct device daisy_devices[] = {
	{ "simplefb", &daisy_fb, "fb" },
};

struct board_data board_ops = {
	.name = "xiaomi-daisy",
	.ops = {
		.early_init = daisy_init,
	},
	.devices = daisy_devices,
	.num_devices = ARRAY_SIZE(daisy_devices),
	.quirks = 0
};
