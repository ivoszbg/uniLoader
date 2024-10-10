// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef DEBUG_H_	/* Include guard */
#define DEBUG_H_

// Sorted by importance
#define KERN_EMERG	0
#define KERN_ALERT	1
#define KERN_CRIT	2
#define KERN_ERR	3
#define KERN_WARNING	4
#define KERN_NOTICE	5
#define KERN_INFO	6
#define KERN_DEBUG	7

extern void printk(int loglevel, char *text);

#endif // DEBUG_H_
