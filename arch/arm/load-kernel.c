// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <main/boot.h>
#include <string.h>

#ifdef CONFIG_EMBED_XNU

#include <stdint.h>

/* Mirrors pexpert/pexpert/arm/boot.h of the darwin-on-arm XNU tree. */
#define XNU_BOOT_LINE_LENGTH	2048

struct xnu_boot_video {
	uint32_t v_baseAddr;
	uint32_t v_display;
	uint32_t v_rowBytes;
	uint32_t v_width;
	uint32_t v_height;
	uint32_t v_depth;
};

struct xnu_boot_args {
	uint16_t Revision;
	uint16_t Version;
	uint32_t virtBase;
	uint32_t physBase;
	uint32_t memSize;
	uint32_t topOfKernelData;
	struct xnu_boot_video Video;
	uint32_t machineType;
	void *deviceTreeP;	/* virtual address */
	uint32_t deviceTreeLength;
	char CommandLine[XNU_BOOT_LINE_LENGTH];
	uint32_t RamdiskPA;	/* physical address of ramdisk image */
	uint32_t RamdiskSize;	/* size in bytes */
};

extern unsigned long dtb_size;
extern void jump_to_xnu(unsigned int entry, unsigned int boot_args);
extern int sd_arch_load_kernel_xnu(unsigned int phys_base,
				   unsigned int virt_base,
				   unsigned int load_pa);
int inflate_raw(const unsigned char *in, unsigned int inlen,
    unsigned char *out, unsigned int outlen);
void inflate_init_tables(void *area);
extern void (*inflate_progress_hook)(unsigned int outpos);
extern unsigned int inflate_in_pos;


/* Relocation delta published by the engine in start.S. The .payload blobs
 * stay at their load address, so every payload symbol must be adjusted. */
extern uint32_t delta_value;
#define PAYLOAD_SYM(sym) (*(unsigned long *)((char *)&(sym) + delta_value))

static void fb_mark(unsigned int byte_off, unsigned int color)
{
	volatile unsigned int *fb = (volatile unsigned int *)CONFIG_XNU_FB_BASE;
	unsigned int i;

	for (i = 0; i < CONFIG_XNU_FB_WIDTH * 24; i++)
		fb[(byte_off >> 2) + i] = color;
}



/* Ladder: A green @0x0 entry, B blue/magenta @0x1B000 delta, C white
 * @0x36000 reloc, D gray @0x51000 bss, E navy @0x6C000 main, F purple
 * @0x87000 earlycon, G olive @0xA2000 boot_kernel, H yellow @0xBD000
 * board init, I blue @0xD8000 decoded, J blue @0xF0000 entering XNU. */

#include <lib/debug.h>

static void
inflate_progress(unsigned int outpos)
{
	static unsigned int blocks = 0;

	blocks++;
	printk(KERN_INFO, "xnu: blk %u out=%u KB in=%u KB\n", blocks,
	    outpos >> 10, inflate_in_pos >> 10);
}


#include <stddef.h>
size_t lz4_decompress(const uint8_t *in, size_t inlen, uint8_t *out,
		      size_t outcap);
void uart_puts(const char *s);

#ifndef CONFIG_XNU_PHYS_BASE
#define CONFIG_XNU_PHYS_BASE	0x22000000
#endif
#ifndef CONFIG_XNU_VIRT_BASE
#define CONFIG_XNU_VIRT_BASE	0x80000000
#endif
#ifndef CONFIG_XNU_MEMSIZE
#define CONFIG_XNU_MEMSIZE	0x35E00000
#endif
#ifndef CONFIG_XNU_ENTRY_OFFSET
#define CONFIG_XNU_ENTRY_OFFSET	0xdf8e0
#endif
#ifndef CONFIG_XNU_FB_BASE
#define CONFIG_XNU_FB_BASE	0x57E05000
#endif
#ifndef CONFIG_XNU_FB_WIDTH
#define CONFIG_XNU_FB_WIDTH	480
#endif
#ifndef CONFIG_XNU_FB_HEIGHT
#define CONFIG_XNU_FB_HEIGHT	800
#endif
#ifndef CONFIG_XNU_CMDLINE
#define CONFIG_XNU_CMDLINE	"-v keepsyms=1 kprintf=1 debug=0x14e"
#endif
#ifndef CONFIG_XNU_CACHESLICE_TOP
#define CONFIG_XNU_CACHESLICE_TOP	0x55E00000
#endif

static unsigned int align_up(unsigned int v, unsigned int a)
{
	return (v + a - 1) & ~(a - 1);
}

/*
 * Derive the entry offset from the unpacked Image's own Mach-O header so
 * a kernel edit that relinks the entry does not require re-flashing the
 * uniLoader.  Matches sd-boot.c:sd_xnu_entry_offset().
 */
#define J1_MH_MAGIC		0xfeedface
#define J1_LC_SEGMENT		0x1
#define J1_LC_UNIXTHREAD	0x5
#define J1_ARM_THREAD_STATE	1

static unsigned int j1_xnu_entry_offset(unsigned int load_pa,
					unsigned int def)
{
	unsigned char *p = (unsigned char *)load_pa;
	unsigned int base = 0, entry = 0, off;
	unsigned int magic, ncmds;
	int i;

	magic = p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned)p[3] << 24);
	if (magic != J1_MH_MAGIC)
		return def;
	ncmds = p[16] | (p[17] << 8) | (p[18] << 16) | ((unsigned)p[19] << 24);
	off = 28;
	for (i = 0; i < ncmds; i++) {
		unsigned int cmd, csize = 0;
		cmd = p[off] | (p[off + 1] << 8) | (p[off + 2] << 16) |
		      ((unsigned)p[off + 3] << 24);
		csize = p[off + 4] | (p[off + 5] << 8) | (p[off + 6] << 16) |
			((unsigned)p[off + 7] << 24);
		if (cmd == J1_LC_SEGMENT) {
			unsigned int vmaddr;
			vmaddr = p[off + 24] | (p[off + 25] << 8) |
				 (p[off + 26] << 16) | ((unsigned)p[off + 27] << 24);
			if (base == 0 || vmaddr < base)
				base = vmaddr;
		} else if (cmd == J1_LC_UNIXTHREAD) {
			unsigned int flavor;
			flavor = p[off + 8] | (p[off + 9] << 8) |
				 (p[off + 10] << 16) | ((unsigned)p[off + 11] << 24);
			if (flavor == J1_ARM_THREAD_STATE)
				entry = p[off + 76] | (p[off + 77] << 8) |
					(p[off + 78] << 16) | ((unsigned)p[off + 79] << 24);
		}
		if (csize == 0)
			break;
		off += csize;
	}
	if (base && entry > base)
		return entry - base;
	return def;
}

void arch_load_kernel_xnu(void *kernel, void *dt, void *ramdisk)
{
	unsigned int phys_base = CONFIG_XNU_PHYS_BASE;
	unsigned int virt_base = CONFIG_XNU_VIRT_BASE;
	unsigned int load_pa = CONFIG_PAYLOAD_ENTRY;
	unsigned int ksize;
	unsigned int dtsize;
	unsigned int dt_pa, ba_pa, top;
	unsigned int rd_pa = 0;
	/* 2048 bytes on the 4KB bootstrap stack squeezed the caller frame
	 * past the stack bottom during the DSC loop's printk calls
	 * (observed: cacheslice lz4 fails at stride/C-cadence, kernel and
	 * ramdisk blobs decode fine).  Keep the cmdline builder static. */
	static char dsc_arg[2048];
	struct xnu_boot_args *ba;

	dsc_arg[0] = 0;

#ifdef CONFIG_EXYNOS_SD_FAT
	/* BRING-UP (J1): FIRST try the FAT32 SD card - if Image/dtb/ramdisk
	 * are present and readable we load plain (uncompressed) binaries
	 * straight into their final addresses and never touch the embedded
	 * .payload copies.  Any failure falls through to the embedded path
	 * below so the device still boots without a card. */
	if (sd_arch_load_kernel_xnu(phys_base, virt_base, load_pa) == 0)
		return;
	printk(KERN_INFO, "xnu: SD boot not available, using payload\n");
#endif


	/*
	 * BRING-UP (J1): dyld shared cache slices.  The build embeds
	 * LZ4 slices of the real dyld_shared_cache_armv7 (file "cacheslice"
	 * in .payload).  Decode each into reserved low RAM below the root
	 * ramdisk and pass the (pa,fileoff,len) triples to the kernel via
	 * bootarg "dsc", so it can feed the shared-cache pages the same
	 * way it feeds exec segments.
	 * Blob: 'CAC1' + u32 count + {u32 fileoff,u32 len,u32 clen}*count
	 * followed by the count LZ4 blocks.
	 */
	{
		extern unsigned char cacheslice_start, cacheslice_end;
		extern unsigned int cacheslice_size;
		const unsigned char *cs = (const unsigned char *)
		    (unsigned long)&cacheslice_start + delta_value;
		unsigned int cs_size = PAYLOAD_SYM(cacheslice_size);
		/* BRING-UP (J1): enlarged DSC slice window.  dst was glued to
		 * 0x55A00000; the grown core-framework set (~14MB raw)
		 * overflowed CONFIG_XNU_MEMSIZE (0x55E00000) there, so the
		 * kernel could not map/feed the tail pages.  Slide it below
		 * the ramdisk decode (top: 0x54300000) but still inside xnu
		 * RAM so every fed page stays mappable. */
		unsigned int dst = 0x54500000;
		unsigned int dsc_pos = 0;
		unsigned int i;

		dsc_arg[0] = 0;
		if (cs_size > 8 && cs[0] == 'C' && cs[1] == 'A' &&
		    cs[2] == 'C' && cs[3] == '1') {
			unsigned int count = cs[4] | (cs[5] << 8) |
			    (cs[6] << 16) | (cs[7] << 24);
			const unsigned char *p = cs + 8;

			printk(KERN_INFO, "xnu: cacheslice %u bytes, %u slices\n",
			    cs_size, count);
			for (i = 0; i < count; i++) {
				unsigned int off, len, clen;

				if ((unsigned int)(p - cs) + 12 > cs_size)
					break;
				off = p[0] | (p[1] << 8) | (p[2] << 16) |
				    (p[3] << 24);
				len = p[4] | (p[5] << 8) | (p[6] << 16) |
				    (p[7] << 24);
				clen = p[8] | (p[9] << 8) | (p[10] << 16) |
				    (p[11] << 24);
				p += 12;
if ((unsigned int)(p - cs) + clen > cs_size)
						break;
					if (dst + len > CONFIG_XNU_CACHESLICE_TOP) {
						printk(KERN_EMERG,
						    "cacheslice: dst overflow\n");
						fb_mark(0x155000, 0xFFFF00FF);
						while (1)
							;
					}
					{
						unsigned int j, cs32 = 0;
						size_t r;

						/* BRING-UP (J1): checksummed ALL
						 * slices so a "success" that actually
						 * decoded corrupted bytes (cs32 vs the
						 * host-verified blob) is visible. */
						for (j = 0; j + 4 <= clen; j += 4)
							cs32 += (p[j] << 24) |
							    (p[j + 1] << 16) |
							    (p[j + 2] << 8) | p[j + 3];
						printk(KERN_INFO,
						    "cacheslice: %u/%u off=0x%08x len=%u clen=%u dst=%08x blk@%08x cs32=%08x\n",
						    i + 1, count, off, len, clen, dst,
						    (unsigned)p, cs32);
						/* BRING-UP (QEMU): a slice whose clen
						 * equals len is carried RAW (no LZ4);
						 * just memcpy it.  The mklz4
						 * greedy compressor is too slow
						 * for a full cache in the emulator
						 * bring-up loop. */
						if (clen == len) {
							memcpy((void *)dst, p,
							    len);
							r = len;
						} else {
							r = lz4_decompress(p, clen,
							    (void *)dst, len);
						}

					if (r == 59 || r == 85 || r == 101 ||
					    r != len) {
						/* BRING-UP diagnostic + retry.
						 * The blob is host-verified, so
						 * a decode failure here means
						 * the C-stream the loader read
						 * differs from the file.  Dump
						 * a checksum + head bytes (to
						 * compare against the host
						 * blob) and retry from a
						 * pristine copy at a scratch
						 * address in case the direct
						 * read is aliased/stale. */
						static const unsigned int
						    scratch = 0x3A000000;
						const unsigned char *bp = p;
						size_t r2 = 0;

						printk(KERN_EMERG,
						    "cacheslice: %u/%u FAILED off=0x%08x len=%u clen=%u dst=%08x blk@%08x cs32=%08x head=%02x%02x%02x%02x %02x%02x%02x%02x\n",
						    i + 1, count, off, len, clen,
						    dst, (unsigned)p, cs32,
						    p[0], p[1], p[2], p[3],
						    p[4], p[5], p[6], p[7]);
						if (clen <= 0x400000) {
							/* 4MB scratch above the
							 * loader, below the
							 * ramdisk decode.  Every
							 * failing slice retries
							 * (scratch is re-filled
							 * per-slice). */
							for (j = 0;
							    j + 4 <= clen;
							    j += 4) {
								volatile
								    unsigned
								    int *sc = (volatile
								    unsigned
								    int *)(scratch +
								    j);
								*sc = (bp[j] <<
								    24) |
								    (bp[j + 1] <<
								    16) |
								    (bp[j + 2] <<
								    8) |
								    bp[j + 3];
							}
							{
								unsigned int cs2 = 0;

								for (j = 0;
								    j + 4 <= clen;
								    j += 4)
									cs2 +=
									    *(volatile
									    unsigned int
									    *)(scratch + j);
								printk(KERN_EMERG,
								    "cacheslice: %u/%u scratch cs32=%08x\n",
								    i + 1,
								    count, cs2);
							}
							/* The first attempt may
							 * have scribbled garbage
							 * into dst already (LZ4
							 * matches read prior
							 * output): zero it so
							 * the retry starts from
							 * clean state. */
							memset((void *)dst, 0,
							    len);
							r2 = lz4_decompress(
							    (const uint8_t *)
							    scratch, clen,
							    (void *)dst, len);
							if (r2 == 59 ||
							    r2 == 85 ||
							    r2 == 101 ||
							    r2 != len) {
								printk(KERN_EMERG,
								    "cacheslice: %u/%u copy-retry ALSO failed r2=%lu\n",
								    i + 1, count,
								    (unsigned long)
								    r2);
								r = r2;
							} else {
								printk(KERN_EMERG,
								    "cacheslice: %u/%u copy-retry OK!\n",
								    i + 1, count);
								r = len;
							}
						}
						if (r == 59 || r == 85 ||
						    r == 101 || r != len) {
							/* skip: keep the rest */
							p += clen;
							dst += (len + 0xfff) &
							    ~0xfff;
							continue;
						}
					}
				}
				printk(KERN_INFO,
				    "cacheslice: %u/%u fileoff=%u len=%u cl=%u -> %08x\n",
				    i + 1, count, off, len, clen, dst);
				{
					static const char hexd[] =
					    "0123456789abcdef";
					unsigned int part[3] = { dst, off,
					    len };
					unsigned int w, b2;

					if (dsc_pos &&
					    dsc_pos < sizeof(dsc_arg) - 1)
						dsc_arg[dsc_pos++] = ',';
					for (w = 0; w < 3; w++) {
						unsigned int vv = part[w];
						int started = 0;

						for (b2 = 0; b2 < 8; b2++) {
							int nb = 7 - b2;
							unsigned int nib =
							    (vv >> (nb *
							    4)) & 0xf;

							if (nib || started ||
							    nb == 0) {
								if (dsc_pos >=
								    sizeof(dsc_arg) - 1)
									break;
								started = 1;
								dsc_arg[
								dsc_pos++] =
								    hexd[nib];
							}
						}
						if (w != 2 &&
						    dsc_pos <
						    sizeof(dsc_arg) - 1)
							dsc_arg[dsc_pos++] = ',';
					}
					if (dsc_pos < sizeof(dsc_arg) - 1)
						dsc_arg[dsc_pos] = 0;
				}
				p += clen;
				dst += (len + 0xfff) & ~0xfff;
			}
			if (dsc_pos)
				printk(KERN_INFO, "xnu: cacheslice dsc='%s'\n",
				    dsc_arg);
		}
	}

	ksize = PAYLOAD_SYM(kernel_size);
	dtsize = PAYLOAD_SYM(dtb_size);

	printk(KERN_INFO, "xnu: blob %u bytes, dt %u bytes\n",
	       ksize, dtsize);

#ifdef CONFIG_XNU_BLOB_LZ4
	{
		unsigned int compressed = ksize;

		printk(KERN_INFO, "xnu: decoding %u bytes...\n", compressed);
		/* The embedded blob is a raw LZ4 block; decode straight to
		 * the load address (src lives in .payload, far from dst). */
		ksize = (unsigned int) lz4_decompress(kernel, compressed,
						      (void *)load_pa,
						      CONFIG_XNU_MEMSIZE);
		if (ksize == 59 || ksize == 85 || ksize == 101) {
			printk(KERN_EMERG, "lz4: kernel FAILED line %u\n",
			       ksize);
			fb_mark(0x140000, 0xFFFF0000); /* red = lz4 died */
			while (1)
				;
		}
		if (!ksize) {
			printk(KERN_EMERG, "lz4: decompression FAILED\n");
			fb_mark(0x140000, 0xFFFF0000); /* red = lz4 died */
			while (1)
				;
		}
		printk(KERN_INFO, "xnu: decoded %u bytes to 0x%08x\n",
		       ksize, load_pa);
	}
#else
	memcpy((void *)load_pa, kernel, ksize);
	{
		volatile unsigned int *p = (volatile unsigned int *)load_pa;
		unsigned int sum = 0, i;

		for (i = 0; i < ksize; i += 4)
			sum += p[i >> 2];
		printk(KERN_INFO, "xnu: copied %u bytes to 0x%08x sum=%08x\n",
		       ksize, load_pa, sum);
	}
#endif

	/* Apple-format device tree right behind the kernel image.
	 * MUST skip past the kernel's BSS: __DATA vmsize > filesize, and
	 * kernel statics written at runtime physically alias the DT/boot
	 * args region if placed at file-end (observed: args fields turned
	 * into pixel-color garbage like 0xFF000000). */
	dt_pa = align_up(load_pa + ksize + 0x1000000, 4);
	memcpy((void *)dt_pa, dt, dtsize);

	/* boot_args behind the device tree. */
	ba_pa = align_up(dt_pa + dtsize, 4);
	ba = (struct xnu_boot_args *) ba_pa;
	memset(ba, 0, sizeof(*ba));

	/* HFS+ root ramdisk right behind the boot args, still below
	 * topOfKernelData so the VM page allocator never touches it.
	 * The embedded blob is gzip-compressed (build.sh packs it). */
	{
		unsigned int rd_size = PAYLOAD_SYM(ramdisk_size);
		unsigned int rd_expect = 0;
		const unsigned char *rd = (const unsigned char *)ramdisk;

		if (rd && rd_size > 4 &&
		    rd[0] == 'R' && rd[1] == 'D' && rd[2] == 'S' &&
		    rd[3] == 'C') {
			/* 'RDSC' + raw LZ4 block (same codec as the
			 * kernel blob).  Decode at the TOP of RAM:
			 * the sequential write used to run straight
			 * over the loader's active page tables at
			 * ~9 MB into this image. */
#ifdef CONFIG_XNU_RAMDISK_PA
			/* BRING-UP (QEMU): a ramdisk parked at top-of-RAM
			 * pushes topOfKernelData (hence first_avail) up,
			 * collapsing the kernel's free-page pool below the
			 * exec threshold (vm_page_free_count hits 0 -> the
			 * first exec UPL VM_PAGE_WAITs forever).  Decode it
			 * at a fixed MID-RAM address instead so the VM gets
			 * hundreds of MB of free pages. */
			rd_pa = CONFIG_XNU_RAMDISK_PA;
#else
			rd_pa = (unsigned)phys_base +
			    CONFIG_XNU_MEMSIZE - 0x2100000;
#endif
			{
				unsigned int i3, csum = 0;

				for (i3 = 0; i3 + 4 <= rd_size - 4;
				    i3 += 4)
					csum += (rd[4 + i3] << 24) |
					    (rd[5 + i3] << 16) |
					    (rd[6 + i3] << 8) | rd[7 + i3];
				printk(KERN_INFO,
				    "xnu: ramdisk lz4 %u bytes -> %08x cs=%08x\n",
				    rd_size - 4, rd_pa, csum);
			}
			{
				size_t rr = lz4_decompress(rd + 4,
				    rd_size - 4, (void *)rd_pa,
				    0x2000000);

				rd_size = (unsigned int)rr;
				/* lz4_decompress returns the decoded byte
				 * count (or error line numbers 59/85/101). */
				if (rr == 59 || rr == 85 || rr == 101)
					printk(KERN_EMERG,
					    "lz4: ramdisk FAILED line %lu\n",
					    (unsigned long)rr);
			}
			if (!rd_size || rd_size < 0x1000) {
				fb_mark(0x150000, 0xFFFF00FF);
				while (1)
					;
			}
			{
				volatile unsigned char *v =
				    (volatile unsigned char *)rd_pa;
				unsigned int i2, sum = 0;
				unsigned int vh = (v[1024] << 24) |
				    (v[1025] << 16) | (v[1026] << 8) |
				    v[1027];

				for (i2 = 0; i2 < 65536; i2 += 4)
					sum += (v[i2] << 24) |
					    (v[i2 + 1] << 16) |
					    (v[i2 + 2] << 8) | v[i2 + 3];
				printk(KERN_INFO,
		    "xnu: vh=%08x sum=%08x (want 482b0004 cbcc81a2)\n",
				    vh, sum);
			}
			ba->RamdiskPA = rd_pa;
			ba->RamdiskSize = rd_size;
			printk(KERN_INFO,
			    "xnu: ramdisk lz4 -> %08x (%u)\n", rd_pa,
			    rd_size);
		} else if (rd && rd_size > 2 &&
		    rd[0] == 0x1F && rd[1] == 0x8B) {
			unsigned char method = rd[2];

			rd_pa = align_up(ba_pa + sizeof(*ba), 4);
			if (method == 8) {
				/* Huffman tables go into the unused gap
				 * between the kernel image and the DT
				 * (below topOfKernelData, above BSS). */
				{
					unsigned int tp = align_up(
					    load_pa + ksize + 0x800000, 4);
					volatile unsigned int *tv =
					    (volatile unsigned int *)tp;

					tv[0] = 0xC0DE0001;
					tv[(320 * 1024 >> 2) - 1] = 0xC0DE0002;
					if (tv[0] != 0xC0DE0001 ||
					    tv[(320 * 1024 >> 2) - 1] !=
					    0xC0DE0002) {
						printk(KERN_EMERG,
						    "xnu: tbl probe FAILED\n");
						while (1)
							;
					}
					inflate_init_tables((void *)tp);
				}

				/* Walk the gzip header properly: fixed
				 * 10 bytes, then optional FNAME /
				 * FCOMMENT strings and FHCRC. */
				const unsigned char *p = rd + 10;
				unsigned char flg = rd[3];
				unsigned int hdr = rd_size;

				if ((flg & 0x07) != flg) {
					printk(KERN_EMERG,
					    "xnu: gzip reserved flags\n");
					while (1)
						;
				}
				if (flg & 0x04) {	/* FEXTRA */
					p += 2 + (p[0] | (p[1] << 8));
				}
				if (flg & 0x08) {	/* FNAME */
					while (*p++)
						;
				}
				if (flg & 0x10) {	/* FCOMMENT */
					while (*p++)
						;
				}
				if (flg & 0x02) {	/* FHCRC */
					p += 2;
				}
				hdr = (unsigned int)(p - rd);
				printk(KERN_INFO,
				    "xnu: gzip hdr %u bytes\n", hdr);
				/* Probe destination DRAM before committing:
				 * a bus wedge here means memory beyond this
				 * point is not actually usable. */
				volatile unsigned int *pp =
				    (volatile unsigned int *)rd_pa;

				inflate_progress_hook = inflate_progress;
				printk(KERN_INFO,
				    "xnu: rd dst %08x, probing...\n", rd_pa);
				pp[0] = 0xA55A5AA5;
				if (pp[0] != 0xA55A5AA5) {
					printk(KERN_EMERG,
					    "xnu: probe low FAILED\n");
					while (1)
						;
				}
				pp[(0x1800000 >> 2) - 1] = 0x5AA5A55A;
				if (pp[(0x1800000 >> 2) - 1] !=
				    0x5AA5A55A) {
					printk(KERN_EMERG,
					    "xnu: probe hi FAILED\n");
					while (1)
						;
				}
				printk(KERN_INFO, "xnu: probe ok\n");
				printk(KERN_INFO,
				    "xnu: inflating %u bytes...\n", rd_size);
				/* ISIZE trailer = exact uncompressed size */
				{
					const unsigned char *t =
					    rd + rd_size - 4;

					rd_expect = t[0] | (t[1] << 8) |
					    (t[2] << 16) | ((unsigned)t[3] <<
					    24);
				}
				printk(KERN_INFO,
				    "xnu: expect %u bytes out\n", rd_expect);
				rd_size = (unsigned int)inflate_raw(
				    rd + hdr, rd_size - hdr,
				    (void *)rd_pa, rd_expect);
				if (!rd_size) {
					printk(KERN_EMERG,
					    "inflate: FAILED\n");
					fb_mark(0x150000, 0xFFFF00FF);
					while (1)
						;
				}
				{
					/* Verify the decompressed image:
					 * VH signature at +1024 and a
					 * word-sum of the first 64 KiB
					 * against offline-computed gold. */
					volatile unsigned char *v =
					    (volatile unsigned char *)rd_pa;
					unsigned int i2, sum = 0;
					unsigned int vh =
					    (v[1024] << 24) | (v[1025] << 16) |
					    (v[1026] << 8) | v[1027];

					for (i2 = 0; i2 < 65536; i2 += 4)
						sum += (v[i2] << 24) |
						    (v[i2 + 1] << 16) |
						    (v[i2 + 2] << 8) | v[i2 + 3];
					printk(KERN_INFO,
		    "xnu: vh=%08x sum=%08x (want 482b0004 cbcc81a2)\n",
					    vh, sum);
				}
				ba->RamdiskPA = rd_pa;
				ba->RamdiskSize = rd_size;
				printk(KERN_INFO,
				    "xnu: ramdisk inflated -> %08x (%u)\n",
				    rd_pa, rd_size);
			} else {
				printk(KERN_EMERG, "xnu: bad gzip method\n");
			}
		} else if (ramdisk && rd_size) {
			/* Uncompressed fallback */
			rd_pa = align_up(ba_pa + sizeof(*ba), 4);
			memcpy((void *)rd_pa, ramdisk, rd_size);
			ba->RamdiskPA = rd_pa;
			ba->RamdiskSize = rd_size;
			printk(KERN_INFO,
			    "xnu: ramdisk %u bytes -> %08x vh=%02x%02x\n",
			       rd_size, rd_pa,
			       ((volatile unsigned char *)rd_pa)[1024],
			       ((volatile unsigned char *)rd_pa)[1025]);
		}
	}


	{
		unsigned int t1 = ba_pa + sizeof(*ba);
		unsigned int t2 = rd_pa + ba->RamdiskSize;

		top = align_up(t1 > t2 ? t1 : t2, 1 << 20);
	}

	ba->Revision = 1;
	ba->Version = 2;
	ba->virtBase = virt_base;
	ba->physBase = phys_base;
	ba->memSize = CONFIG_XNU_MEMSIZE;
	ba->topOfKernelData = top;

	ba->Video.v_baseAddr = CONFIG_XNU_FB_BASE;
	ba->Video.v_display = 0;
	ba->Video.v_rowBytes = CONFIG_XNU_FB_WIDTH * 4;
	ba->Video.v_width = CONFIG_XNU_FB_WIDTH;
	ba->Video.v_height = CONFIG_XNU_FB_HEIGHT;
	ba->Video.v_depth = 32;

	ba->machineType = 0;
	ba->deviceTreeP = (void *)(dt_pa - phys_base + virt_base);
	ba->deviceTreeLength = dtsize;

	memcpy(ba->CommandLine, CONFIG_XNU_CMDLINE,
	       sizeof(CONFIG_XNU_CMDLINE) - 1);
	if (dsc_arg[0]) {
		unsigned int q = 0, dp = 0;

		while (ba->CommandLine[q])
			q++;
		while (dsc_arg[dp])
			dp++;
		if (q + 6 + dp < sizeof(ba->CommandLine)) {
			ba->CommandLine[q++] = ' ';
			ba->CommandLine[q++] = 'd';
			ba->CommandLine[q++] = 's';
			ba->CommandLine[q++] = 'c';
			ba->CommandLine[q++] = '=';
			{
				unsigned int k;

				for (k = 0; dsc_arg[k]; k++)
					ba->CommandLine[q++] = dsc_arg[k];
			}
			ba->CommandLine[q] = 0;
		}
		printk(KERN_INFO, "xnu: cmdline='%s'\n", ba->CommandLine);
	}

	{
		volatile unsigned int *p = (volatile unsigned int *)ba;
		unsigned int i, s = 0;

		for (i = 0; i < sizeof(*ba) / 4; i++)
			s += p[i];
		printk(KERN_INFO,
		       "xnu: layout pa=%08x ks=%u dt=%08x(%u) ba=%08x top=%08x\n",
		       load_pa, ksize, dt_pa, dtsize, ba_pa, top);
		printk(KERN_INFO,
		       "xnu: ba#1 vb=%08x pb=%08x ms=%08x tk=%08x sum=%08x\n",
		       ba->virtBase, ba->physBase, ba->memSize,
		       ba->topOfKernelData, s);
	}

	{
		unsigned int entry_off =
			j1_xnu_entry_offset(load_pa, CONFIG_XNU_ENTRY_OFFSET);
		printk(KERN_INFO, "xnu: entry offset %x (%s)\n", entry_off,
		       entry_off == CONFIG_XNU_ENTRY_OFFSET ?
		       "fallback" : "parsed");
		jump_to_xnu(load_pa + entry_off, ba_pa);
	}

	/* Not reached. */
	while (1)
		;
}
#endif /* CONFIG_EMBED_XNU */

void arch_load_kernel(void* kernel, void* dt, void* ramdisk)
{
#ifdef CONFIG_EMBED_XNU
	arch_load_kernel_xnu(kernel, dt, ramdisk);
#else
	memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) kernel_size);
#ifndef CONFIG_RAMDISK_NO_COPY
	memcpy((void*)CONFIG_RAMDISK_ENTRY, ramdisk, (unsigned long) ramdisk_size);
#endif
	load_kernel_and_jump(0, 0, dt, (void*)CONFIG_PAYLOAD_ENTRY);
#endif
}
