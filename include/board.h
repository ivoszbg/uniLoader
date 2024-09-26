/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Board files registration framework
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#ifndef BOARD_H_	/* Include guard */
#define BOARD_H_

#include <stddef.h>

/*
 * Well, well, well. We come back to this style again.
 * TODO: Implement libfdt once we have C libs all sorted out.
 *
 * @name -> board name
 * @init -> initialization as soon as we hit C
 * @late_init -> late initialization
 * @driver_setup -> driver initialization
 */
struct board_data {
	const char *name;
	void (*init)(void);
	void (*late_init)(void);
	void (*driver_setup)(void);
};

extern struct board_data *get_current_board(void);

// Declare a weak reference to the board data structure
extern struct board_data __attribute__((weak)) default_board;

struct board_data default_board = {
	.name = "DEFAULT",
	.init = NULL,  // No init function for the default
	.late_init = NULL,
	.driver_setup = NULL,
};

#endif // BOARD_H_
