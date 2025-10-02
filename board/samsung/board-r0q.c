/* SPDX-License-Identifier: GPL-2.0 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
static struct video_info r0q_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)0xb8000000
};
#endif

int r0q_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &r0q_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-r0q",
	.ops = {
		.drivers_init = r0q_drv
	},
	.quirks = 0
};
