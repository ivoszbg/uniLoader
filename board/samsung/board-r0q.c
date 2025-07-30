/* SPDX-License-Identifier: GPL-2.0 */

#include <board.h>
#include <main.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

int r0q_init(void)
{
	return 0;
}

int r0q_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1080,
		.height = 2340,
		.stride = 4,
		.address = (void *)0xb8000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

int r0q_late_init(void)
{
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-r0q",
	.ops = {
		.early_init = r0q_init,
		.drivers_init = r0q_drv,
		.late_init = r0q_late_init,
	},
	.quirks = 0
};
