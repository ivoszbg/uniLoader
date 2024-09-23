/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef STDDEF_H_	/* Include guard */
#define STDDEF_H_

#define NULL			((void *) 0)
#define offsetof(type, field)	((int) (&((type *) 0)->field))

typedef unsigned long size_t;

#endif // STDDEF_H_
