/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef STRING_H_	/* Include guard */
#define STRING_H_

#include "stddef.h"
#include "stdint.h"

#define LBLOCKSIZE (sizeof(long))
#define UNALIGNED(X) ((long)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < LBLOCKSIZE)

// C-driven unoptimized functions
int memcmp (const void *s1, const void *s2, size_t n);
void *memchr (const void *s, int c, size_t n);
char *strcpy (char *s1, const char *s2);
char *strncpy (char *d, char *s, long n);
char *strcat (char *d, char *s);
int strcmp (const char *s1, const char *s2);
int strncmp (const char *s1, const char *s2, size_t n);
size_t strlen (const char *s);
size_t strnlen (const char *s, size_t n);
char *strchr (const char *s, int c);
char *strrchr (const char *s, int c);
long atol (const char *s);
void writel (unsigned int value, void* address);
uint32_t readl(volatile uint32_t *addr);

// C-driven optimized functions
void *memset (void *m, int c, size_t n);

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED_CPY(X, Y) (((long)X & (sizeof (long) - 1)) | \
			    ((long)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE (sizeof(long) << 2)

static void *__optimized_memcpy (void *dst0, const void *src0, size_t len0) __attribute__((unused));
static void *__optimized_memcpy (void *dst0, const void *src0, size_t len0)
{
	char *dst = dst0;
	const char *src = src0;
	long *aligned_dst;
	const long *aligned_src;

	/* If the size is small, or either SRC or DST is unaligned,
	then punt into the byte copy loop.  This should be rare.  */
	if (!(((len0) < BIGBLOCKSIZE)) && !UNALIGNED_CPY (src, dst)) {
		aligned_dst = (long*)dst;
		aligned_src = (long*)src;
		/* Copy 4X long words at a time if possible.  */
		while (len0 >= BIGBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			len0 -= BIGBLOCKSIZE;
		}
		/* Copy one long word at a time if possible.  */
		while (len0 >= LBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			len0 -= LBLOCKSIZE;
		}

		/* Pick up any residual with a byte copier.  */
		dst = (char*)aligned_dst;
		src = (char*)aligned_src;
	}

	while (len0--)
		*dst++ = *src++;
	return dst0;
}

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
