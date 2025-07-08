/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef _LIMITS_H	/* Include guard */
#define _LIMITS_H

#include <stdint.h>
#include <stddef.h>

typedef uintptr_t phys_addr_t;
typedef uintptr_t resource_size_t;

#define SIZE_MAX        (~(size_t)0)
#define SSIZE_MAX       ((ssize_t)(SIZE_MAX >> 1))
#define PHYS_ADDR_MAX   (~(phys_addr_t)0)

#define RESOURCE_SIZE_MAX ((resource_size_t)~0)

#define U8_MAX      ((uint8_t)~0U)
#define S8_MAX      ((int8_t)(U8_MAX >> 1))
#define S8_MIN      ((int8_t)(-S8_MAX - 1))

#define U16_MAX     ((uint16_t)~0U)
#define S16_MAX     ((int16_t)(U16_MAX >> 1))
#define S16_MIN     ((int16_t)(-S16_MAX - 1))

#define U32_MAX     ((uint32_t)~0U)
#define U32_MIN     ((uint32_t)0)
#define S32_MAX     ((int32_t)(U32_MAX >> 1))
#define S32_MIN     ((int32_t)(-S32_MAX - 1))

#define U64_MAX     ((uint64_t)~0ULL)
#define S64_MAX     ((int64_t)(U64_MAX >> 1))
#define S64_MIN     ((int64_t)(-S64_MAX - 1))

typedef signed long long     intmax_t;
typedef unsigned long long   uintmax_t;

typedef signed long          ptrdiff_t;
typedef unsigned long        size_t;
typedef signed long          ssize_t;

typedef char        int_fast8_t;
typedef unsigned char        uint_fast8_t;

#endif /* _LIMITS_H */
