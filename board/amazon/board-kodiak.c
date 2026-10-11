/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, R0rt1z2 <roger@r0rt1z2.com>
 */

#include <board.h>
#include <string.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define MDP_BASE			0xfd900000
#define MDP_CTL_0_START			(MDP_BASE + 0x0061c)
#define MDP_PP_0_SYNC_CONFIG_VSYNC	(MDP_BASE + 0x12d04)
#define MDP_PP_0_AUTOREFRESH_CONFIG	(MDP_BASE + 0x12d30)

#define PANEL_WIDTH			720
#define PANEL_HEIGHT			1280

/* The panel runs in DSI command mode, let the MDP keep pushing frames to it */
int kodiak_init(void)
{
	writel((19200000 / (PANEL_HEIGHT * 60)) | (1 << 19),
	       (void *)MDP_PP_0_SYNC_CONFIG_VSYNC);
	writel((1 << 31) | 1, (void *)MDP_PP_0_AUTOREFRESH_CONFIG);
	writel(1, (void *)MDP_CTL_0_START);

	return 0;
}

static struct video_info kodiak_fb = {
	.format = FB_FORMAT_RGB888,
	.width = PANEL_WIDTH,
	.height = PANEL_HEIGHT,
	.stride = 3,
	.address = (void *)0x0d200000,
};

static const struct device kodiak_devices[] = {
	{ "simplefb", &kodiak_fb, "fb" },
};

struct board_data board_ops = {
	.name = "amazon-kodiak",
	.ops = {
		.early_init = kodiak_init,
	},
	.devices = kodiak_devices,
	.num_devices = ARRAY_SIZE(kodiak_devices),
	.quirks = 0,
};
