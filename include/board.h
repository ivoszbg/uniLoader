/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Board files registration framework
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#ifndef BOARD_H_	/* Include guard */
#define BOARD_H_

#include <stddef.h>

struct board_ops {
	int (*early_init)(void);
	int (*drivers_init)(void);
	int (*late_init)(void);
};

struct board_data {
	const char *name;
	struct board_ops ops;
	unsigned int quirks;
};

extern void init_board_funcs(void *board);

extern int board_driver_setup(void);
extern int board_init(void);
extern int board_late_init(void);

#endif // BOARD_H_
