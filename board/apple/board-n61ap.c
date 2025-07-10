/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

int n61ap_init(void)
{
	return 0;
}

int n61ap_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 752,
		.height = 1334,
		.stride = 4,
		.address = (void *)0x83e900000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

int n61ap_late_init(void)
{
	return 0;
}

struct board_data board_ops = {
	.name = "apple-iphone6",
	.ops = {
		.early_init = n61ap_init,
		.drivers_init = n61ap_drv,
		.late_init = n61ap_late_init,
	},
	.quirks = 0
};
