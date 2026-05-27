/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, predefine <predefine@yandex.ru>
 */

#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

// Used on miui 12.5.8.0 firmware
static struct video_info begonia_r_fb = {
    .format = FB_FORMAT_ARGB8888,
    .width = 1088,
    .height = 2340,
    .stride = 4,
    .address = (void *)0x7d9b0000
};

static const struct device begonia_r_devices[] = {
	{ "simplefb", &begonia_r_fb, "fb" },
};

struct board_data board_ops = {
    .name = "xiaomi-begonia",
    .ops = {
    },
	.devices = begonia_r_devices,
	.num_devices = ARRAY_SIZE(begonia_r_devices),
    .quirks = 0
};
