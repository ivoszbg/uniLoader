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
 * BOARD_OP_INIT -> initialization as soon as we hit C
 * BOARD_OP_LATE_INIT -> late initialization
 * BOARD_OP_DRIVER_SETUP -> drivers setup
 * BOARD_OP_EXIT -> the last op, currently unused
 */
enum board_ops {
	BOARD_OP_INIT,
	BOARD_OP_LATE_INIT,
	BOARD_OP_DRIVER_SETUP,
	BOARD_OP_EXIT
};

// Hold board data WITHOUT POINTERS
struct board_data {
	const char *name;
	int ops[BOARD_OP_EXIT];
};

extern void init_board_funcs(void *board);

extern int board_driver_setup(void);
extern int board_init(void);
extern int board_late_init(void);

// Macro definitions for board operations
#define EXECUTE_BOARD_OP(op_id)			\
	do {					\
		switch (op_id) {		\
		case BOARD_OP_INIT:		\
			board_init();		\
			break;			\
		case BOARD_OP_LATE_INIT:	\
			board_late_init();	\
			break;			\
		case BOARD_OP_DRIVER_SETUP:	\
			board_driver_setup();	\
			break;			\
		default:			\
			break;			\
		}				\
	} while (0)

#endif // BOARD_H_
