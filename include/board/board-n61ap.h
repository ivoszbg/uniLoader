/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef BOARD_N61AP_H_	/* Include guard */
#define BOARD_N61AP_H_

/* TODO: Adjust PAYLOAD_ENTRY */
#define PAYLOAD_ENTRY		0x830000000
#define PAYLOAD_SIZE		0x2000000

#define FRAMEBUFFER_BASE       0x83e900000
/* Width is offsetted by 2 */
#define FRAMEBUFFER_WIDTH      752
#define FRAMEBUFFER_HEIGHT     1334
#define FRAMEBUFFER_STRIDE     4

#endif // BOARD_N61AP_H_
