// SPDX-License-Identifier: GPL-2.0-only
/*
 * Read-only FAT32 filesystem on the Exynos 3475 SD card.
 *
 * Relies on sd_read_sectors() from sdmmc3475.c for block access.  This
 * module only parses MBR -> BPB -> cluster chains; it never writes and
 * never touches the eMMC.  Supports long file names (LFN) because the
 * boot blobs (Image/dtb/ramdisk/cacheslice) use mixed-case / >8.3 names.
 */

#include <stdint.h>
#include <string.h>
#include <lib/debug.h>

int sd_read_sectors(uint32_t lba, unsigned count, void *buf);
int sd_is_ready(void);

/* ---- FAT32 BPB-derived geometry (filled by sd_fat_mount) ---- */
static uint32_t fat_part_lba;	/* first LBA of the FAT32 partition */
static uint32_t fat_bytes_sec;
static uint32_t fat_secs_clu;
static uint32_t fat_resv;	/* reserved sectors (boot sectors) */
static uint32_t fat_nfats;
static uint32_t fat_fat_secs;	/* sectors per FAT (BPB_FATSz32) */
static uint32_t fat_root_clu;	/* first cluster of root directory */
static uint32_t fat_data_lba;	/* first data-sector LBA */
static int fat_mounted;

static uint8_t fat_sect[512] __attribute__((aligned(8)));
static uint32_t fat_cache_lba = 0xFFFFFFFF;	/* FAT sector cached */
static uint8_t fat_cache[512] __attribute__((aligned(8)));

static inline uint16_t le16(const uint8_t *p)
{
	return (uint16_t)(p[0] | (p[1] << 8));
}

static inline uint32_t le32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
	       ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Case-insensitive ASCII compare, used for file names. */
static int fat_ci_cmp(const char *a, const char *b)
{
	while (*a && *b) {
		char ca = *a, cb = *b;

		if (ca >= 'A' && ca <= 'Z')
			ca += 'a' - 'A';
		if (cb >= 'A' && cb <= 'Z')
			cb += 'a' - 'A';
		if (ca != cb)
			return (int)(unsigned char)ca - (int)(unsigned char)cb;
		a++;
		b++;
	}
	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

/* Compare an 11-byte 8.3 directory name (space-padded, no NUL) against
 * "name.ext"; a dot matches when the extension is empty. */
static int fat_ci_cmp_83(const uint8_t *e, const char *name)
{
	const char *dot = NULL;
	const char *p = name;
	unsigned k;

	while (*p) {
		if (*p == '.') {
			dot = p;
			break;
		}
		p++;
	}
	/* base of the name vs e[0..7] (space = end) */
	for (k = 0; k < 8; k++) {
		char c = e[k];
		char d = *name;

		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		if (d >= 'A' && d <= 'Z')
			d += 'a' - 'A';
		if (c != ' ' && c != d)
			return -1;
		if (c == ' ') {
			/* base ends where 8.3 ends */
			if (dot && *name)
				return -1;
			break;
		}
		if (!dot && !*name && !e[k + 1])
			return 0;
		name++;
	}
	if (dot) {
		/* extension */
		for (k = 8; k < 11; k++) {
			char c = e[k];
			char d = *name;

			if (c >= 'A' && c <= 'Z')
				c += 'a' - 'A';
			if (d >= 'A' && d <= 'Z')
				d += 'a' - 'A';
			if (c != ' ' && c != d)
				return -1;
			if (c == ' ') {
				if (*name)
					return -1;
				break;
			}
			name++;
		}
	} else if (e[8] != ' ') {
		return -1;	/* has extension, lookup has none */
	}
	return *name ? -1 : 0;
}

static uint32_t fat_cluster_lba(uint32_t clu)
{
	return fat_data_lba + (clu - 2) * fat_secs_clu;
}

/* Read FAT entry for a cluster; returns next cluster or 0x0FFFFFF8 EOC. */
static uint32_t fat_next_cluster(uint32_t clu)
{
	uint32_t pos = clu * 4;
	uint32_t secn = pos / fat_bytes_sec;
	uint32_t off = pos % fat_bytes_sec;
	uint32_t v;

	if (secn != fat_cache_lba) {
		if (sd_read_sectors(fat_part_lba + fat_resv + secn, 1,
				    fat_cache))
			return 0x0FFFFFF8;
		fat_cache_lba = secn;
	}
	v = le32(fat_cache + off);
	if (v >= 0x0FFFFFF8)
		return 0x0FFFFFF8;
	return v & 0x0FFFFFFF;
}

/*
 * Directory entries: 8.3 short names are uppercase-only; mixed-case or
 * >8.3 names carry LFN records (attr 0x0F) that precede the real entry.
 * LFN records appear highest-order-first; each holds up to 13 UTF-16LE
 * chars.  We only care about ASCII host-side names.
 */
static int fat_dir_lookup(uint32_t clu, const char *name,
			  uint32_t *out_clu, uint32_t *out_size)
{
	char lfn[256];
	unsigned lfn_max;	/* chars decoded into lfn[] so far */

	for (;;) {
		uint32_t secn;

		if (clu == 0x0FFFFFF8 || clu < 2)
			return -1;
		for (secn = 0; secn < fat_secs_clu; secn++) {
			uint32_t lba = fat_cluster_lba(clu) + secn;
			unsigned o;

			if (sd_read_sectors(lba, 1, fat_sect))
				return -1;
			lfn_max = 0;
			for (o = 0; o + 32 <= 512; o += 32) {
				uint8_t *e = fat_sect + o;
				unsigned attr = e[11];
				unsigned i;

				if (e[0] == 0x00)
					return -1;	/* end of dir */
				if (e[0] == 0xE5) {
					lfn_max = 0;	/* orphaned LFN */
					continue;
				}

				if (attr == 0x0F) {
					/* LFN record: order in low 5 bits
					 * (1 = first part, decoded last),
					 * 0x40 marks the LAST record we see
					 * (contains the name's first chars). */
					unsigned ord = e[0] & 0x1f;
					unsigned base = (ord - 1) * 13;

					if (ord == 0 || base + 13 > sizeof(lfn))
						continue;
					/* First record of a group: the buffer
					 * must be zeroed so unterminated UTF-16
					 * padding (0x0000) reads as NUL. */
					if (lfn_max == 0)
						memset(lfn, 0, sizeof(lfn));
					/* chars 1-5 in bytes 1,3,5,7,9 */
					for (i = 0; i < 5; i++) {
						uint8_t c = e[1 + i * 2];

						if (c)
							lfn[base + i] = (char)c;
					}
					/* chars 6-11 in bytes 14,16,18,20,22,24 */
					for (i = 0; i < 6; i++) {
						uint8_t c = e[14 + i * 2];

						if (c)
							lfn[base + 5 + i] = (char)c;
					}
					/* chars 12-13 in bytes 28,30 */
					for (i = 0; i < 2; i++) {
						uint8_t c = e[28 + i * 2];

						if (c)
							lfn[base + 11 + i] = (char)c;
					}
					if (base + 13 > lfn_max)
						lfn_max = base + 13;
					/* the 8.3 entry follows immediately */
					continue;
				}
				if (attr & 0x10) {
					lfn_max = 0;	/* subdir: skip */
					continue;
				}

				/* Compare the (long) name, or fall back to the
				 * 8.3 name (spaces pad the 11 chars). */
				if (lfn_max) {
					unsigned n = lfn_max;

					while (n && !lfn[n - 1])
						n--;
					lfn[n] = 0;
					if (fat_ci_cmp(lfn, name) == 0) {
						*out_clu = le16(e + 20) << 16 |
							   le16(e + 26);
						*out_size = le32(e + 28);
						return 0;
					}
				} else if (fat_ci_cmp_83(e, name) == 0) {
					*out_clu = le16(e + 20) << 16 |
						   le16(e + 26);
					*out_size = le32(e + 28);
					return 0;
				}
				lfn_max = 0;	/* LFN belongs to THIS entry only */
			}
		}
		clu = fat_next_cluster(clu);
	}
}

int sd_fat_mount(void)
{
	uint32_t i;
	uint8_t type = 0;
	uint32_t part_lba = 0;

	if (!sd_is_ready())
		return -1;
	if (fat_mounted)
		return 0;

	/* Sector 0: MBR, walk the 4 partition entries for FAT32 (0x0b/0x0c). */
	if (sd_read_sectors(0, 1, fat_sect))
		return -1;
	if (le16(fat_sect + 510) != 0xAA55)
		return -1;
	for (i = 0; i < 4; i++) {
		const uint8_t *pe = fat_sect + 446 + i * 16;

		type = pe[4];
		if (type == 0x0b || type == 0x0c) {
			part_lba = le32(pe + 8);
			break;
		}
	}
	if (!part_lba) {
		printk(KERN_INFO, "sd:fat no FAT32 partition\n");
		return -1;
	}

	/* Partition boot sector / BPB. */
	if (sd_read_sectors(part_lba, 1, fat_sect))
		return -1;
	if (le16(fat_sect + 510) != 0xAA55)
		return -1;

	fat_part_lba  = part_lba;
	fat_bytes_sec = le16(fat_sect + 11);
	fat_secs_clu  = fat_sect[13];
	fat_resv      = le16(fat_sect + 14);
	fat_nfats     = fat_sect[16];
	fat_fat_secs  = le32(fat_sect + 36);
	fat_root_clu  = le32(fat_sect + 44);
	fat_data_lba  = part_lba + fat_resv + fat_nfats * fat_fat_secs;

	if (fat_bytes_sec != 512 || fat_secs_clu == 0)
		return -1;

	fat_mounted = 1;
	fat_cache_lba = 0xFFFFFFFF;
	printk(KERN_INFO, "sd:fat part@%u bps=%u spc=%u resv=%u nfat=%u "
		"fatsec=%u root=%u\n",
	       part_lba, fat_bytes_sec, fat_secs_clu, fat_resv,
	       fat_nfats, fat_fat_secs, fat_root_clu);
	return 0;
}

/* Progress reporting for very large reads.  The 154 MiB cacheslice takes a
 * long time over CMD18 and used to look exactly like a hang: one bracketing
 * printk, then minutes of silence.  Off by default so ordinary file loads
 * stay quiet; sd-boot.c turns it on around the big reads only. */
int sd_progress_on;
static unsigned int sd_progress_mark;

void sd_progress_enable(int on)
{
	sd_progress_on = on;
	sd_progress_mark = 0;
}

static void sd_progress_report(unsigned int done)
{
	if (!sd_progress_on)
		return;
	sd_progress_mark++;
	/* Every 8 MiB, and never more than a couple of lines per MiB so the
	 * console itself cannot become the thing that is slow.  This is now
	 * the ONLY per-chunk output during the big reads (all the per-CMD
	 * success chatter is gated behind sd_chatter), so it is what shows
	 * that the read is advancing. */
	if (sd_progress_mark >= 8u || (done & 0xFFFFF) < 0x10000) {
		sd_progress_mark = 0;
		printk(KERN_INFO, "  rd %u MiB\n", done >> 20);
	}
}

/*
 * sd_fat_read - load a whole file from the FAT32 root directory.
 * @name: file name (case-insensitive)
 * @dst:  destination buffer
 * @maxlen: capacity of dst
 * Returns bytes read, or negative on error.
 */
int sd_fat_read(const char *name, void *dst, unsigned maxlen)
{
	uint32_t clu, size, done = 0;
	uint64_t total;
	uint8_t *p = (uint8_t *)dst;
	int rc;

	if (!fat_mounted) {
		rc = sd_fat_mount();
		if (rc)
			return rc;
	}
	if (fat_dir_lookup(fat_root_clu, name, &clu, &size))
		return -1;

	if (size > maxlen)
		size = maxlen;
	total = (uint64_t)size;

	while (done < total) {
		uint64_t left = total - done;
		uint32_t clu_bytes = fat_secs_clu * fat_bytes_sec;
		uint32_t clus_left = (uint32_t)((left + clu_bytes - 1) /
		    clu_bytes);
		uint32_t run = 1;
		uint32_t c = clu;
		unsigned i;

		if (clu < 2 || clu == 0x0FFFFFF8)
			return -1;
		/* Collect the physically-contiguous prefix of the cluster
		 * chain: freshly written files are sequential on disk, so
		 * this turns one CMD18 per cluster (blocks=64, 32KB) into one
		 * big sd_read_sectors() (up to 512KB per call). */
		while (run < clus_left) {
			uint32_t n = fat_next_cluster(c);

			if (n < 2 || n == 0x0FFFFFF8 || n != c + 1)
				break;
			run++;
			c = n;
		}
		{
			uint64_t run_bytes = (uint64_t)run * clu_bytes;
			uint32_t take = run_bytes < left ? (uint32_t)run_bytes :
			    (uint32_t)left;
			uint32_t blocks = (take + 511) / 512;

			if (sd_read_sectors(fat_cluster_lba(clu), blocks, p))
				return -1;
			done += take;
			p += take;
			sd_progress_report((unsigned int)done);
		}
		for (i = 0; i < run; i++)
			clu = fat_next_cluster(clu);
	}
	return (int)size;
}
/*
 * sd_fat_lba - resolve a file to the physical sector range it occupies.
 *
 * The kernel log writer needs the raw LBA of a pre-allocated file so it
 * can push sectors straight at the card without ever touching the FAT or
 * the directory entry.  Only the first cluster's LBA plus the sector
 * count is returned, which is what a sequential pre-allocated file gives
 * us; a fragmented file would need the whole chain walked.
 */
int sd_fat_lba(const char *name, uint32_t *lba, uint32_t *nsect)
{
	uint32_t clu, size;

	if (!fat_mounted && sd_fat_mount())
		return -1;
	if (fat_dir_lookup(fat_root_clu, name, &clu, &size))
		return -1;
	if (clu < 2)
		return -1;

	*lba = fat_cluster_lba(clu);
	*nsect = (size + 511u) / 512u;
	return 0;
}
