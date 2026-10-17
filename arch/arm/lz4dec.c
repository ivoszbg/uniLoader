// SPDX-License-Identifier: GPL-2.0-only
/*
 * Minimal LZ4 block-format decompressor for the embedded XNU blob.
 * Consumes exactly what build/mklz4.py produces.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* On-screen progress: one green/blue slot per 512 KiB of output.
 * If the decoder dies mid-stream, the bars freeze at the culprit. */
extern void fb_mark_asm(unsigned int byte_off, unsigned int color);
static unsigned int lz_prog_slot;
static unsigned int big_cnt;
static unsigned char big_tok;
static unsigned int big_op;
extern void printk(int log_level, const char *fmt, ...);

size_t lz4_decompress(const uint8_t *in, size_t inlen, uint8_t *out,
		      size_t outcap)
{
	const uint8_t *ip = in, *ie = in + inlen;
	uint8_t *op = out, *oe = out + outcap;

	while (ip < ie) {
#ifdef LZPROG
		/* BRING-UP: no printing inside the decode loop - the
		 * loader stack is tiny and printk's frame was smashing
		 * our locals (observed: bogus-offset failures exactly
		 * at print cadence). */
		lz_prog_slot = (size_t)(op - out) >> 22;
		big_cnt = 0;
#endif
		uint8_t tok = *ip++;
#ifdef LZPROG
		if ((tok >> 4) == 15 || (tok & 0xF) == 15) {
			big_cnt++;
			big_tok = tok;
			big_op = (unsigned)(op - out);
		}
#endif
		size_t ll = tok >> 4;

		if (ll == 15) {
			uint8_t b;
			do {
				b = *ip++;
				ll += b;
			} while (b == 255 && ip < ie);
		}
		if (ll > (size_t)(ie - ip) || ll > (size_t)(oe - op)) {
			fb_mark_asm(0x110000, 0xFFFF3333); /* red */
			printk(6,
		    "lz4bad L: ip=%u ll=%u ie=%u op=%u oe=%u\n",
			    (unsigned)(ip - in), (unsigned)ll,
			    (unsigned)(ie - ip), (unsigned)(op - out),
			    (unsigned)(oe - op));
			return (__LINE__);
		}
		/* Byte copy ONLY: __memcpy_arm issues word loads at
		 * arbitrary alignments, which aborts under SCTLR.A=1
		 * (the S-Boot handover state).  volatile defeats any
		 * compiler merging of the loop into word ops. */
		while (ll--) {
			uint8_t d = *ip++;

			*(volatile uint8_t *)op = d;
			op++;
		}
		if (ip == ie)
			break; /* last sequence carries no match */

		/* Byte loads ONLY: gcc merges these into LDRH, which faults
		 * on odd addresses when SCTLR.A=1 (S-Boot handover state). */
		volatile const uint8_t *vip = ip;
		size_t off = (size_t)vip[0] | ((size_t)vip[1] << 8);
		ip += 2;
		if (off == 0 || off > (size_t)(op - out)) {
			fb_mark_asm(0x110000, 0xFFFFFF33); /* yellow */
			printk(6,
		    "lz4bad O: tok=%02x off=%u op=%u ip=%u\n",
			    tok, (unsigned)off, (unsigned)(op - out),
			    (unsigned)(ip - in));
			return (__LINE__);
		}

		size_t ml = tok & 0xF;
		if (ml == 15) {
			uint8_t b;
			do {
				b = *ip++;
				ml += b;
			} while (b == 255 && ip < ie);
		}
		ml += 4;
		if (ml > (size_t)(oe - op)) {
			fb_mark_asm(0x110000, 0xFF33FFFF); /* cyan */
			printk(6, "lz4bad M: ml=%u op=%u\n",
			    (unsigned)ml, (unsigned)(op - out));
			return (__LINE__);
		}

		volatile const uint8_t *mp = op - off;
		for (size_t k = 0; k < ml; k++) {
			uint8_t d = *mp++;

			*(volatile uint8_t *)op = d;
			op++;
		}
	}

	fb_mark_asm(0x100000, 0xFF33FF33); /* green = decode ok */
	return (size_t)(op - out);
}
