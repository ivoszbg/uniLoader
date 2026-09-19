/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, faveoled <faveoled@yandex.com>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info lua_l21_fb = {
	.format = FB_FORMAT_ABGR8888,
	.width = 480,
	.height = 854,
	.stride = 4,
	.address = (void *)0x7fa30000
};

static const struct device lua_l21_devices[] = {
	{ "simplefb", &lua_l21_fb, "fb" },
};

struct board_data board_ops = {
	.name = "huawei-lua-l21",
	.ops = {
	},
	.devices = lua_l21_devices,
	.num_devices = ARRAY_SIZE(lua_l21_devices),
	.quirks = 0
};
