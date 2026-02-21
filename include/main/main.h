// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef MAIN_H_   /* Include guard */
#define MAIN_H_

#define HANG() while(1) {}

/*
 * Some initcalls (for example late_init) are not used
 * on any devices (other than a stub return function).
 * As such, all initcalls are optional, and only ran if they exist.
 * However, if they do exist, getting a positive return value is needed.
 */
#define INITCALL(_call) \
	do { \
		if (_call && _call()) { \
			printk(KERN_EMERG, "initcall %s() failed\n", #_call); \
			HANG(); \
		} \
	} while (0)

extern void soc_init(void);

#endif // MAIN_H_
