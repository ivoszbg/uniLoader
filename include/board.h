/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Board files registration framework
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#ifndef BOARD_H_
#define BOARD_H_

#include <stddef.h>
#include <drivers/framework.h>

struct board_ops {
	int (*early_init)(void);
	int (*late_init)(void);
};

struct board_data {
	const char *name;
	struct board_ops ops;
	const struct device *devices;
	size_t num_devices;
	unsigned int quirks;
};

#endif /* BOARD_H_ */
