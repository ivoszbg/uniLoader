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

#define WDT_BASE        0xE8A06000U

static struct video_info agassi2_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1200,
	.height = 1920,
	.stride = 4,
	.address = (void *)0x5d000000,
	.scale = 2
};

static const struct device agassi2_devices[] = {
	{ "arm,sp805-wdt", (void *)WDT_BASE, "wdt0" },
	{ "simplefb", &agassi2_fb, "fb" },
};

struct board_data board_ops = {
	.name = "huawei-agassi2",
	.devices = agassi2_devices,
	.num_devices = ARRAY_SIZE(agassi2_devices),
	.quirks = 0
};
