/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

int algiz_init(void)
{
	/* TODO: deal with framebuffer refreshing here */
	return 0;
}

int algiz_drv(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1080,
		.height = 2400,
		.stride = 4,
		.address = (void *)0x7d010000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}

int algiz_late_init(void)
{
	return 0;
}

struct board_data board_ops = {
	.name = "volla-quintus",
	.ops = {
		.early_init = algiz_init,
		.drivers_init = algiz_drv,
		.late_init = algiz_late_init,
	},
	.quirks = 0
};
