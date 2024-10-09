/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef STRING_H_	/* Include guard */
#define STRING_H_

#include "stddef.h"

#define LBLOCKSIZE (sizeof(long))
#define UNALIGNED(X)   ((long)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < LBLOCKSIZE)

// C-driven unoptimized functions
int memcmp (const void *s1, const void *s2, size_t n);
void *memchr (const void *s, int c, size_t n);
char *strcpy (char *s1, const char *s2);
char *strncpy (char *s1, const char *s2, size_t n);
int strcmp (const char *s1, const char *s2);
int strncmp (const char *s1, const char *s2, size_t n);
size_t strlen (const char *s);
size_t strnlen (const char *s, size_t n);
char *strchr (const char *s, int c);
char *strrchr (const char *s, int c);
long atol (const char *s);
void writel (unsigned int value, void* address);

// C-driven optimized functions
void *memset (void *m, int c, size_t n);

// Assembly-driven functions
#ifdef __aarch64__
void *memcpy (void *__restrict, const void *__restrict, size_t);
void *memmove (void *, const void *, size_t);
#endif

#ifdef __arm__
void *memcpy (void *s1, const void *s2, size_t n)
{
	char *dest = (char *)s1;
	const char *src = (const char *)s2;

	while (n--) {
		*dest++ = *src++;
	}

	return s1;
}

void *memmove (void *s1, const void *s2, size_t n)
{
	char *dest = (char *)s1;
	const char *src = (const char *)s2;

	if (dest <= src) {
		while (n--) {
			*dest++ = *src++;
		}
	} else {
		src += n;
		dest += n;

		while (n--) {
			*--dest = *--src;
		}
	}

	return s1;
}
#endif

static inline int tolower (int c)
{
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 'a';
	else
		return c;
}

static inline int toupper (int c)
{
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 'A';
	else
		return c;
}

#endif // STRING_H_
