/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>

static struct video_info vita12k_fb = {
	.format = FB_FORMAT_ABGR8888,
	.width = 960,
	.height = 544,
	.stride = 4,
	.address = (void *)0x20000000
};

/* for now only vita1k/vita2k is supported, PSTV would need more config :/ */
static const struct device vita_devices[] = {
	{ "simplefb", &vita12k_fb, "fb" },
};

struct board_data board_ops = {
	.name = "sony-psvita",
	.devices = vita_devices,
	.num_devices = ARRAY_SIZE(vita_devices),
	.quirks = 0
};
