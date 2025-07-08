/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef INTTYPES_H	/* Include guard */
#define INTTYPES_H

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#if __SIZEOF_POINTER__ == 8
typedef unsigned long uintptr_t;
#else
typedef unsigned int uintptr_t;
#endif

#ifdef __x86_64__
typedef unsigned long uint64_t;
typedef long int64_t;
#else
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif

#define CHAR_BIT 8

#endif // INTTYPES_H
