/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Alexandru Chimac <alex@chimac.ro>
 */
#include <board.h>
#include <util.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define TOPRGU_BASE 0x10212000
#define WDT_MODE_KEY 0x22000000
#define WDT_MODE_EN (1 << 0)

// Early initialization
int essential_init(void)
{
	unsigned long wdg_reg;

	// Disable watchdog
	wdg_reg = readl((volatile unsigned int *) TOPRGU_BASE);
	wdg_reg &= ~WDT_MODE_EN;
	wdg_reg |= WDT_MODE_KEY;
	writel(wdg_reg, (void *) TOPRGU_BASE);

	return 0;
}

static struct video_info essential_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 736,
	.height = 1280,
	.stride = 4,
	.address = (void *)0xbf370000
};

static const struct device essential_devices[] = {
	{ "simplefb", &essential_fb, "fb" },
	{ "mediatek,mt6795-systimer", (void *)0x10200670, "timer0" },
};

struct board_data board_ops = {
	.name = "nokia-essential",
	.ops = {
		.early_init = essential_init,
	},
	.devices = essential_devices,
	.num_devices = ARRAY_SIZE(essential_devices),
	.quirks = 0
};
