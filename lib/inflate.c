/* Fast DEFLATE (RFC 1951) decoder for the embedded XNU ramdisk.
 * Table-driven Huffman (15-bit direct lookup) + 32-bit bit accumulator.
 * Public domain. */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "main/inflate.h"
#ifdef HOSTTEST
#include <stdio.h>
#define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

void (*inflate_progress_hook)(unsigned int outpos);
unsigned int inflate_in_pos;	/* input byte position at last block end */

#define HTBITS	15
#define HTSIZE	(1u << HTBITS)

struct htab {
	unsigned char len[HTSIZE];	/* code length, 0 = invalid */
	unsigned short sym[HTSIZE];	/* symbol, up to 285 */
};

/* Bit accumulator (LSB-first, as DEFLATE packs values). */
static const unsigned char *inb;
static unsigned int inlen, inpos;
static uint32_t bitacc;
static int bitcnt;

static int
refill(int need)
{
	while (bitcnt < need) {
		if (inpos >= inlen)
			return (-1);
		bitacc |= (uint32_t)inb[inpos++] << bitcnt;
		bitcnt += 8;
	}
	return (0);
}

static inline unsigned int
peekbits(int n)
{
	return (bitacc & ((1u << n) - 1));
}

static inline void
dropbits(int n)
{
	bitacc >>= n;
	bitcnt -= n;
}

static inline unsigned int
getbits(int n)
{
	unsigned int v;

	/* Top up before peeking: callers may request more bits than are
	 * currently buffered (bitcnt must never go negative). */
	if (refill(n))
		return (0);
	v = peekbits(n);
	dropbits(n);
	return (v);
}

/* Build a direct 15-bit lookup table from canonical code lengths. */
static int
htab_build(struct htab *ht, const unsigned char *lengths, int n)
{
	unsigned short count[16];
	unsigned short next[16];
	int i, l;
	unsigned int code, index, step, last;

	memset(ht->len, 0, sizeof(ht->len));
	memset(count, 0, sizeof(count));

	for (i = 0; i < n; i++)
		count[lengths[i]]++;
	count[0] = 0;

	/* Canonical first codes per length. */
	code = 0;
	next[0] = 0;
	for (l = 1; l <= 15; l++) {
		code = (code + count[l - 1]) << 1;
		next[l] = (unsigned short)code;
	}

	/* Reject oversubscribed codes. */
	last = 0;
	for (l = 1; l <= 15; l++)
		last += (unsigned int)count[l] << (15 - l);
	if (last > HTSIZE)
		return (-1);

	for (i = 0; i < n; i++) {
		l = lengths[i];
		if (l == 0)
			continue;
		code = next[l]++;
		step = 1u << l;
		for (index = code; index < HTSIZE; index += step) {
			ht->len[index] = (unsigned char)l;
			ht->sym[index] = (unsigned short)i;
		}
	}
	return (0);
}

/* Huffman tables live in caller-provided scratch RAM (see
 * inflate_init_tables): three 96 KiB tables would blow the boot
 * partition budget if kept in the loader image. */
static struct htab *ltable;
static struct htab *dtable;
static struct htab *ctable;	/* code-length alphabet */

void
inflate_init_tables(void *area)
{
	ltable = (struct htab *)area;
	dtable = (struct htab *)((char *)area + sizeof(struct htab));
	ctable = (struct htab *)((char *)area + 2 * sizeof(struct htab));
}

static inline int
decode_sym(const struct htab *ht)
{
	unsigned int idx;

	if (refill(HTBITS))
		return (-1);
	idx = peekbits(HTBITS);
	if (!ht->len[idx])
		return (-1);
	dropbits(ht->len[idx]);
	return (ht->sym[idx]);
}

static const unsigned short lens_tab[29] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static const unsigned char lext_tab[29] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
static const unsigned short dists_tab[30] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577
};
static const unsigned char dext_tab[30] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

static int
codes_block(unsigned char *out, unsigned int outlen, unsigned int *outpos)
{
	for (;;) {
		int sym = decode_sym(ltable);
		int len, dist, dsym;
		unsigned int i;

		if (sym < 0)
			{ DBG("cb: lit sym<0 out=%u in=%u\n", *outpos, inpos); return (-1); }
		if (sym < 256) {
			if (*outpos >= outlen)
				return (-1);
			out[(*outpos)++] = (unsigned char)sym;
			continue;
		}
		if (sym == 256) {
			inflate_in_pos = inpos;
			if (inflate_progress_hook)
				inflate_progress_hook(*outpos);
			return (0);
		}

		sym -= 257;
		if (sym >= 29)
			return (-1);
		len = lens_tab[sym];
		if (lext_tab[sym]) {
			if (refill(lext_tab[sym]))
				{ DBG("cb: refill lext out=%u in=%u\n", *outpos, inpos); return (-1); }
			len += (int)getbits(lext_tab[sym]);
		}

		dsym = decode_sym(dtable);
		if (dsym < 0 || dsym >= 30)
			{ DBG("cb: dsym=%d out=%u\n", dsym, *outpos); return (-1); }
		dist = dists_tab[dsym];
		if (dext_tab[dsym]) {
			if (refill(dext_tab[dsym]))
				{ DBG("cb: refill dext out=%u in=%u\n", *outpos, inpos); return (-1); }
			dist += (int)getbits(dext_tab[dsym]);
		}

		if ((unsigned int)dist > *outpos ||
		    *outpos + (unsigned int)len > outlen)
			{ DBG("cb: bounds dist=%d len=%d out=%u\n", dist, len, *outpos); return (-1); }
		for (i = 0; i < (unsigned int)len; i++) {
			out[*outpos] = out[*outpos - dist];
			(*outpos)++;
		}
	}
}

static const unsigned short order_tab[19] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2,
	14, 1, 15
};

static int
dynamic_tables(void)
{
	static unsigned char lengths[320];
	unsigned char clens[19];
	int nlen, ndist, ncode, index, err;

	nlen = (int)getbits(5) + 257;
	ndist = (int)getbits(5) + 1;
	ncode = (int)getbits(4) + 4;
	if (nlen > 286 || ndist > 30)
		{ DBG("dt@%d\n", __LINE__); return (-1); }

	memset(clens, 0, sizeof(clens));
	for (index = 0; index < ncode; index++)
		clens[order_tab[index]] = (unsigned char)getbits(3);
	if (htab_build(ctable, clens, 19))
		{ DBG("dt@%d\n", __LINE__); return (-1); }

	index = 0;
	while (index < nlen + ndist) {
		int sym = decode_sym(ctable);

		if (sym < 0)
			{ DBG("dt@%d\n", __LINE__); return (-1); }
		if (sym < 16) {
			lengths[index++] = (unsigned char)sym;
			continue;
		}
		{
			int len = 0, rep;

			if (sym == 16) {
				if (index == 0)
					{ DBG("dt@%d\n", __LINE__); return (-1); }
				len = lengths[index - 1];
				rep = (int)getbits(2) + 3;
			} else if (sym == 17) {
				rep = (int)getbits(3) + 3;
			} else {
				rep = (int)getbits(7) + 11;
			}
			if (index + rep > nlen + ndist)
				{ DBG("dt@%d\n", __LINE__); return (-1); }
			while (rep--)
				lengths[index++] = (unsigned char)len;
		}
	}

	err = htab_build(ltable, lengths, nlen);
	if (err)
		return (err);
	err = htab_build(dtable, lengths + nlen, ndist);
	return (err);
}

int
inflate_raw(const unsigned char *in, unsigned int ilen,
    unsigned char *out, unsigned int outlen)
{
	unsigned int outpos = 0;
	int last;

	inb = in;
	inlen = ilen;
	inpos = 0;
	bitacc = 0;
	bitcnt = 0;

	do {
		int type;

		if (refill(3))
			return (-1);
		last = (int)(bitacc & 1);
		type = (int)((bitacc >> 1) & 3);
		dropbits(3);

		switch (type) {
		case 0:
			/* Stored block: discard partial bits, LEN/NLEN. */
			dropbits(bitcnt & 7);
			if (refill(32))
				return (-1);
			{
				unsigned int l = getbits(16);

				(void)getbits(16);	/* NLEN */
				if (outpos + l > outlen)
					return (-1);
				while (l--) {
					if (refill(8))
						return (-1);
					out[outpos++] =
					    (unsigned char)getbits(8);
				}
			}
			break;
		case 1:
			{
				static int init = 0;
				static unsigned char fl[288];
				int i;

				if (!init) {
					for (i = 0; i < 144; i++)
						fl[i] = 8;
					for (; i < 256; i++)
						fl[i] = 9;
					for (; i < 280; i++)
						fl[i] = 7;
					for (; i < 288; i++)
						fl[i] = 8;
					init = 1;
				}
				if (htab_build(ltable, fl, 288))
					return (-1);
				for (i = 0; i < 30; i++)
					fl[i] = 5;
				if (htab_build(dtable, fl, 30))
					return (-1);
			}
			if (codes_block(out, outlen, &outpos))
				return (-1);
			break;
		case 2:
			if (dynamic_tables())
				return (-1);
			if (codes_block(out, outlen, &outpos))
				return (-1);
			break;
		default:
			return (-1);
		}
	} while (!last);

	inflate_in_pos = inpos;
	return ((int)outpos);
}
