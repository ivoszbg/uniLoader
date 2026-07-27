/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, David Wronek <w.david0@protonmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE		0x19030000

static struct video_info beyond1lte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 3040,
	.stride = 4,
	.address = (void *)0xca000000
};

static const struct device beyond1lte_devices[] = {
	{ "samsung,exynos9-decon", (void *)DECON_F_BASE, "decon" },
	{ "simplefb", &beyond1lte_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-beyond1lte",
	.devices = beyond1lte_devices,
	.num_devices = ARRAY_SIZE(beyond1lte_devices),
	.quirks = 0
};
