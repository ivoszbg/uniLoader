// SPDX-License-Identifier: GPL-2.0-only
/*
 * Bare-metal DWMMC (DesignWare Mobile Storage Host) for Exynos 3475.
 * Targets ONLY dwmmc2 @0x13560000 (SD slot on SM-J120F).
 *
 * Stage 1: SD init + PIO block read (CMD17/CMD18).
 */

#include <stdint.h>
#include <string.h>
#include <lib/debug.h>

#if defined(CONFIG_EXYNOS_SD_FAT)
void sd_fat_probe(void);
#endif

#define MMC2_BASE	0x13560000

/* GPIO: gpc0 -> vmmc, gpa1 -> CD, gpr4 -> SD2 pins */
#define PIN_GPR4_CON	0x13750080
#define PIN_GPR4_DAT	0x13750084
#define PIN_GPR4_PUD	0x13750088
#define PIN_GPC0_CON	0x139C0040
#define PIN_GPC0_DAT	0x139C0044
#define PIN_GPC0_PUD	0x139C0048
#define PIN_GPA1_DAT	0x139B0024

/* DW MMC registers */
static volatile uint32_t *const MMC = (volatile uint32_t *)MMC2_BASE;
#define MMC_CTRL	0x000
#define MMC_PWREN	0x004
#define MMC_CLKDIV	0x008
#define MMC_CLKSRC	0x00c
#define MMC_CLKENA	0x010
#define MMC_TMOUT	0x014
#define MMC_CTYPE	0x018
#define MMC_BLKSIZ	0x01c
#define MMC_BYTCNT	0x020
#define MMC_INTMASK	0x024
#define MMC_CMDARG	0x028
#define MMC_CMD	0x02c
#define MMC_RESP0	0x030
#define MMC_RESP1	0x034
#define MMC_RESP2	0x038
#define MMC_RESP3	0x03c
#define MMC_MINTSTS	0x040
#define MMC_RINTSTS	0x044
#define MMC_STATUS	0x048
#define MMC_FIFOTH	0x04c
#define MMC_CDETECT	0x050
#define MMC_WRTPRT	0x054
#define MMC_GPIO	0x058
#define MMC_MCGBLK	0x05c
#define MMC_VERID	0x06c
#define MMC_HCON	0x070
#define MMC_UHS_REG	0x074
#define MMC_BMOD	0x080
#define MMC_PLDMND	0x084
#define MMC_DBADDRL	0x088
#define MMC_DBADDRU	0x08c
#define MMC_IDSTS	0x090
#define MMC_IDINTEN	0x094
#define MMC_DSCADDRL	0x098
#define MMC_DSCADDRU	0x09c
#define MMC_BUFADDRL	0x0a0
/* Data port: DW 2.40a+ registers are at 0x200, FIFO is 64-bit wide on
 * this SoC (HCON data_shift==3).  Kernel reads it with readq from 0x200
 * (dw_mmc_pull_data64, DATA_240A_OFFSET). */
#define MMC_DATA	0x200
#define MMC_DATA_U	0x204
/* Exynos-specific CLKSEL (sampling/drive/div), 3475 DT values:
 * sdr-timing <3 0 2 0>, sdr-hs-timing <3 0 2 1> (gtes_common.dtsi). */
#define MMC_CLKSEL	0x0a4
#define SD_LEGACY_CLKSEL	0x03020000
#define SD_HS_CLKSEL		0x03020001

#define SDMMC_CTRL_RESET	(1 << 0)
#define SDMMC_CTRL_FIFO_RESET	(1 << 1)
#define SDMMC_CTRL_DMA_RESET	(1 << 2)
#define SDMMC_CTRL_INT_ENABLE	(1 << 4)
#define SDMMC_CTRL_DMA_ENABLE	(1 << 5)
#define SDMMC_CTRL_USE_IDMAC	(1 << 25)
#define SDMMC_CLKEN_ENABLE	(1 << 0)
#define SDMMC_CLKEN_LOW_PWR	(1 << 16)

/* IDMAC BMOD bits */
#define IDMAC_SWRESET	(1 << 0)
#define IDMAC_FB	(1 << 1)
#define IDMAC_ENABLE	(1 << 7)
/* IDMAC descriptor control bits */
#define DES0_OWN	(1u << 31)
#define DES0_CES	(1u << 30)
#define DES0_ER		(1u << 5)
/* FMP sector key: 0x80000000 = bypass (kernel DW_MMC_BYPASS_SECTOR_BEGIN) */
#define DESC_SECTOR_KEY_BYPASS	0x80000000
#define DES0_CH		(1u << 4)
#define DES0_FD		(1u << 3)
#define DES0_LD		(1u << 2)
#define DES0_DIC	(1u << 1)
/* IDMAC interrupt bits */
#define IDMAC_TI	(1 << 0)
#define IDMAC_RI	(1 << 1)
#define IDMAC_NI	(1 << 8)

#define CMD_START	(1 << 31)
#define CMD_UPD_CLK	(1 << 21)
#define CMD_DAT_EXP	(1 << 9)
#define CMD_DAT_WR	(1 << 10)
#define CMD_INIT	(1 << 15)
#define CMD_PRV_DAT_WAIT	(1 << 13)
#define CMD_RESP_EXP	(1 << 6)
#define CMD_RESP_LONG	(1 << 7)
#define CMD_RESP_CRC	(1 << 8)
#define CMD_STOP	(1 << 14)
#define CMD_INDX(n)	((n) & 0x3f)

#define INT_CD		(1 << 0)
#define INT_RESP_ERR	(1 << 1)
#define INT_CMD_DONE	(1 << 2)
#define INT_DATA_OVER	(1 << 3)
#define INT_RXDR	(1 << 5)
#define INT_TXDR	(1 << 4)
#define INT_RCRC	(1 << 6)
#define INT_RTO		(1 << 8)
#define INT_DTO		(1 << 9)
#define INT_EBE		(1 << 15)
#define INT_ERROR	(INT_RTO | INT_RCRC | INT_RESP_ERR | INT_EBE)

#define STATUS_FCNT(x)	(((x) >> 17) & 0x1fff)

#define MMC_GO_IDLE_STATE	0
#define MMC_ALL_SEND_CID	2
#define MMC_SET_RELATIVE_ADDR	3
#define MMC_SELECT_CARD		7
#define MMC_SEND_CSD		9
#define MMC_STOP_TRANSMISSION	12
#define MMC_READ_SINGLE_BLOCK	17
#define MMC_READ_MULTIPLE_BLOCK	18
#define MMC_APP_CMD		55
#define SD_APP_OP_COND		41
#define SD_SEND_IF_COND		8
#define SD_SET_BUS_WIDTH	6	/* via CMD55 (ACMD6) */
#define SD_SWITCH_FUNC		6	/* CMD6 (function switch) */

#define R1_APP_CMD		(1 << 5)
#define MMC_CARD_BUSY		(1 << 31)
#define SD_ACMD41_ARG		0x40300000
#define SD_IF_COND_ARG		0x000001AA
#define SD_ACMD6_ARG_4BIT	0x2
#define SD_CMD6_ARG_SWITCH_HS	0x80FFFFF1
#define CTYPE_4BIT		0x1

#define SD_BLOCK_SIZE		512
#define SD_FIFO_DEPTH		64

/* ---- IDMAC descriptor (64-bit, CONFIG_MMC_DW_64_IDMAC layout) ---- */
struct idmac_desc {
	uint32_t des0;	/* OWN(31) CES(30) ER(5) CH(4) FD(3) LD(2) DIC(1) */
	uint32_t des1;
	uint32_t des2;	/* buffer1 size (low 13 bits) via SET_BUFFER1_SIZE */
	uint32_t des3;
	uint32_t des4;	/* buffer1 physical address */
	uint32_t des5;
	uint32_t des6;	/* next descriptor (IDMAC_NEXT_DESC_ADDR) */
	uint32_t des7;
	uint32_t des8;	/* sector key / bypass (0) */
};

#define DES0_BUFFER1_SIZE(d, s)	((d)->des2 = ((d)->des2 & 0x03ffe000) | ((s) & 0x1fff))

static struct idmac_desc idmac_desc_tab[2] __attribute__((aligned(64)));
/* DMA destination buffer, kept far from the descriptor table (the
   earlier test buffer ended up overlapping des8 of the table) */
static uint8_t sd_dma_buf[4096] __attribute__((aligned(64)));


/* ---- card state (set by probe, used by read functions) ---- */
static uint32_t sd_rca;
static uint64_t sd_sectors;
static int sd_ready;

/*
 * Per-chunk read chatter ("sd:pio18 ok ...").  A 154 MB cacheslice read
 * emits hundreds of these and each one costs ~1.5 console lines, so the
 * simplefb console wipes the whole screen many times over and the useful
 * milestones scroll away.  The SD boot path clears this to 0; the error
 * prints (no-dover / cmd error) are NOT gated.
 */
int sd_chatter = 1;

static inline uint32_t r32(unsigned off) { return MMC[off / 4]; }
static inline void w32(unsigned off, uint32_t v) { MMC[off / 4] = v; }
static inline void dmb(void) { __asm__ __volatile__("dmb" ::: "memory"); }

static void mdelay(unsigned us)
{
	volatile unsigned t = us * 400;
	while (t--)
		;
}

static uint32_t mbit(const uint32_t *r, int s, int sz)
{
	uint32_t m = (sz < 32 ? (1U << sz) : 0U) - 1U;
	int o = 3 - (s / 32), sh = s & 31;
	uint32_t v = r[o] >> sh;
	if (sz + sh > 32)
		v |= r[o - 1] << ((32 - sh) % 32);
	return v & m;
}

static const char *off_name[] = {
	"CTRL","PWREN","CLKDIV","CLKSRC","CLKENA","TMOUT",
	"CTYPE","BLKSIZ","BYTCNT","INTMASK","CMDARG","CMD",
	"RESP0","RESP1","RESP2","RESP3","MINTSTS","RINTSTS",
	"STATUS","FIFOTH","CDETECT","WRTPRT","GPIO","MCGBLK",
	"---","---","---","---","---","---","---","---",
	"VERID","HCON","UHS_REG"
};

static void dump_regs(const char *tag)
{
	static const unsigned offs[] = {
		0x000,0x004,0x008,0x00c,0x010,0x014,
		0x018,0x01c,0x020,0x024,0x028,0x02c,
		0x030,0x034,0x038,0x03c,0x040,0x044,
		0x048,0x04c,0x050,0x054,0x058,0x05c,
		0x068,0x06c,0x070,0x074,0x078,0x07c,
		0x080,0x084,0x088,0x08c
	};
	unsigned i;
	printk(KERN_INFO, "sd:%s ---\n", tag);
	for (i = 0; i < sizeof(offs)/sizeof(offs[0]); i++) {
		printk(KERN_INFO, "sd:%s %03x %-8s = %08x\n",
		       tag, offs[i], off_name[i], r32(offs[i]));
	}
}

static int wait_reset(uint32_t bit, unsigned max)
{
	unsigned t;
	w32(MMC_CTRL, bit);
	dmb();
	for (t = 0; t < max && (r32(MMC_CTRL) & bit); t++)
		;
	return (r32(MMC_CTRL) & bit) ? -1 : 0;
}

static int upd_clk(void)
{
	unsigned t, try;

	for (try = 0; try < 50; try++) {
		w32(MMC_CMDARG, 0);
		dmb();
		w32(MMC_CMD, CMD_START | CMD_UPD_CLK | CMD_PRV_DAT_WAIT);
		dmb();
		for (t = 0; t < 1000000 && (r32(MMC_CMD) & CMD_START); t++)
			;
		if (!(r32(MMC_CMD) & CMD_START))
			return 0;
		wait_reset(SDMMC_CTRL_RESET, 100000);
	}
	printk(KERN_INFO, "sd:upd-clk timeout cmd=%08x ctrl=%08x\n",
	       r32(MMC_CMD), r32(MMC_CTRL));
	return -1;
}

static void set_slow_clock(void)
{
	w32(MMC_CLKENA, 0);
	w32(MMC_CLKSRC, 0);
	upd_clk();
	w32(MMC_CLKDIV, 0xff);
	upd_clk();
	w32(MMC_CLKENA, SDMMC_CLKEN_ENABLE);
	upd_clk();
	printk(KERN_INFO, "sd:clk-div=%08x clkena=%08x cmd=%08x\n",
	       r32(MMC_CLKDIV), r32(MMC_CLKENA), r32(MMC_CMD));
}

/* ---- command-only (no data transfer) ---- */
static int mm_cmd(unsigned idx, uint32_t arg, uint32_t flags,
		  uint32_t *resp, int init)
{
	uint32_t cmd_val, st;
	unsigned t;

	for (t = 0; t < 1000000 && (r32(MMC_CMD) & CMD_START); t++)
		;

	w32(MMC_RINTSTS, 0xffffffff);
	w32(MMC_CMDARG, arg);
	cmd_val = CMD_START | CMD_INDX(idx) | flags;
	if (init)
		cmd_val |= CMD_INIT;
	w32(MMC_CMD, cmd_val);
	dmb();

	for (t = 0; t < 5000000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_CMD_DONE | INT_RTO | INT_RESP_ERR | INT_RCRC))
			break;
		mdelay(2);
	}
	if (!(st & (INT_CMD_DONE | INT_RTO | INT_RESP_ERR | INT_RCRC))) {
		printk(KERN_INFO, "sd:cmd%u timeout st=%08x status=%08x\n",
		       idx, st, r32(MMC_STATUS));
		return -1;
	}

	if (resp) {
		if (flags & CMD_RESP_LONG) {
			resp[3] = r32(MMC_RESP0);
			resp[2] = r32(MMC_RESP1);
			resp[1] = r32(MMC_RESP2);
			resp[0] = r32(MMC_RESP3);
		} else {
			resp[0] = r32(MMC_RESP0);
			resp[1] = 0;
			resp[2] = 0;
			resp[3] = 0;
		}
	}
	w32(MMC_RINTSTS, st);

	/* Success-path chatter is per-CMD, and sd_read_sectors() issues one
	 * CMD12 per 1024-block chunk, so leaving this unconditional fills the
	 * whole screen with hundreds of identical lines during the 154 MB
	 * cacheslice read - which reads exactly like an infinite loop.  Only
	 * the failures above are always printed. */
	if (sd_chatter)
		printk(KERN_INFO, "sd:cmd%u done st=%08x status=%08x cmd=%08x"
		       " r0=%08x\n",
		       idx, st, r32(MMC_STATUS), r32(MMC_CMD),
		       resp ? resp[0] : 0);

	if (st & INT_RTO)
		return -1;
	if (st & (INT_RESP_ERR | INT_RCRC))
		return -2;
	return 0;
}


/* ---- IDMAC (internal DMA) helpers ----
   Kernel reads this SD slot ONLY via IDMAC; the PIO CPU data port
   returns zeros on this Exynos.  Layout from dw_mmc.c (j1xlte):
   - DBADDRL 0x88 / DBADDRU 0x8c / IDSTS 0x90 (SFR_OFFSET=4 variant)
   - 64-bit descriptor: des0 OWN/FD/LD, des4 = buf phys addr,
     des6 = next desc, des8 = sector key (0 = bypass) */
static void sd_idmac_init(void)
{
	/* Build the descriptor ring once (dw_mci_idmac_init) and point the
	 * IDMAC at it.  ring_size=2 is fine for our single-block use. */
	memset(idmac_desc_tab, 0, sizeof(idmac_desc_tab));
	idmac_desc_tab[0].des6 = (uint32_t)&idmac_desc_tab[1];
	idmac_desc_tab[1].des6 = (uint32_t)&idmac_desc_tab[0]; /* ring */
	idmac_desc_tab[1].des0 = DES0_ER;		      /* end-of-ring */
	dmb();

	w32(MMC_BMOD, IDMAC_SWRESET);
	mdelay(2);
	w32(MMC_IDINTEN, IDMAC_TI | IDMAC_RI | IDMAC_NI);
	w32(MMC_DBADDRU, 0);
	w32(MMC_DBADDRL, (uint32_t)&idmac_desc_tab[0]);
}

/* dw_mci_idmac_stop_dma: CTRL &= ~USE_IDMAC, then wait DMA reset, then
 * BMOD &= ~(ENABLE|FB).  The real SoC needs the CTRL-DMA reset, not just
 * the BMOD SWRESET (kernel does exactly this). */
static void sd_idmac_stop(void)
{
	uint32_t v;
	unsigned t;

	v = r32(MMC_CTRL);
	v &= ~SDMMC_CTRL_USE_IDMAC;
	w32(MMC_CTRL, v);

	/* dw_mci_wait_reset(CTRL_DMA_RESET): set bit, wait self-clear */
	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_DMA_RESET;
	w32(MMC_CTRL, v);
	for (t = 0; t < 100000; t++) {
		if (!(r32(MMC_CTRL) & SDMMC_CTRL_DMA_RESET))
			break;
	}

	v = r32(MMC_BMOD);
	v &= ~(IDMAC_ENABLE | IDMAC_FB);
	w32(MMC_BMOD, v);
}

/* dw_mci_idma_reset_dma: BMOD |= SWRESET (self-clear) */
static void sd_idma_reset(void)
{
	w32(MMC_BMOD, r32(MMC_BMOD) | IDMAC_SWRESET);
}

static int sd_idmac_read(uint32_t lba, void *buf)
{
	uint32_t v, st;
	unsigned t;
	struct idmac_desc *d = &idmac_desc_tab[0];

	sd_idmac_stop();	/* clean state (kernel idmac stop_dma) */
	mdelay(1);

	/* kernel order per-transfer:
	 *   init: BMOD=SWRESET, IDINTEN, DBADDRL
	 *   reset: BMOD |= SWRESET
	 *   translate (DMA_ENABLE already set) -> write the descriptor
	 */
	/* per-transfer init (dw_mci_idmac_init is also called every time) */
	w32(MMC_BMOD, IDMAC_SWRESET);
	w32(MMC_IDINTEN, IDMAC_TI | IDMAC_RI | IDMAC_NI);
	w32(MMC_DBADDRU, 0);
	w32(MMC_DBADDRL, (uint32_t)d);
	mdelay(1);
	sd_idma_reset();
	mdelay(1);

	/* Enable the DMA interface (dw_mci_submit_data_dma step).  The
	 * descriptor must be built AFTER this, exactly like kernel
	 * dw_mci_idmac_start_dma -> translate_sglist does. */
	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_DMA_ENABLE;
	w32(MMC_CTRL, v);

	/* kernel: disable RX/TX IRQs, let DMA handle it */
	w32(MMC_RINTSTS, INT_RXDR | INT_TXDR);
	v = r32(MMC_INTMASK);
	v &= ~(INT_RXDR | INT_TXDR);
	w32(MMC_INTMASK, v);

	/* build the ring + one descriptor AFTER the DMA_ENABLE write,
	 * exactly like dw_mci_translate_sglist leaves it */
	memset(idmac_desc_tab, 0, sizeof(idmac_desc_tab));
	idmac_desc_tab[0].des6 = (uint32_t)&idmac_desc_tab[1];
	idmac_desc_tab[1].des6 = (uint32_t)&idmac_desc_tab[0];
	idmac_desc_tab[1].des0 = DES0_ER;
	d->des0 = DES0_OWN | DES0_FD | DES0_LD;
	DES0_BUFFER1_SIZE(d, SD_BLOCK_SIZE);
	d->des2 |= SD_BLOCK_SIZE;	/* 64-bit: kernel writes length into des2 */
	d->des4 = (uint32_t)buf;
	d->des8 = DESC_SECTOR_KEY_BYPASS;  /* 0x80000000, not 0 */
	dmb();

	/* transfer size */
	w32(MMC_BLKSIZ, SD_BLOCK_SIZE);
	w32(MMC_BYTCNT, SD_BLOCK_SIZE);

	/* Select IDMAC interface + enable it (dw_mci_idmac_start_dma) */
	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_USE_IDMAC;
	w32(MMC_CTRL, v);
	dmb();

	v = r32(MMC_BMOD);
	v |= IDMAC_ENABLE | IDMAC_FB;
	w32(MMC_BMOD, v);

	/* clear stale ints, then tell IDMAC there are pending descriptors */
	w32(MMC_RINTSTS, 0xffffffff);
	w32(MMC_IDSTS, 0xffffffff);
	w32(MMC_PLDMND, 1);
	dmb();

	/* issue CMD17 */
	w32(MMC_CMDARG, lba);
	dmb();
	w32(MMC_CMD, CMD_START | CMD_INDX(MMC_READ_SINGLE_BLOCK) |
	     CMD_RESP_EXP | CMD_RESP_CRC | CMD_DAT_EXP |
	     CMD_PRV_DAT_WAIT);
	dmb();

	/* wait CMD_DONE */
	for (t = 0; t < 1000000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_CMD_DONE | INT_RTO))
			break;
	}
	if (st & INT_RTO) {
		printk(KERN_INFO, "sd:idmac rto st=%08x idsts=%08x\n",
		       st, r32(MMC_IDSTS));
		sd_idmac_stop();
		return -1;
	}
	w32(MMC_RINTSTS, INT_CMD_DONE);

	/* wait DMA transfer-interrupt (TI) = 512B delivered to buf */
	v = 0;
	for (t = 0; t < 5000000; t++) {
		v = r32(MMC_IDSTS);
		if (v & (IDMAC_TI | IDMAC_RI | IDMAC_NI))
			break;
		st = r32(MMC_RINTSTS);
		if (st & INT_ERROR) {
			printk(KERN_INFO, "sd:idmac err st=%08x idsts=%08x\n",
			       st, r32(MMC_IDSTS));
			sd_idmac_stop();
			return -1;
		}
	}
	if (!(v & (IDMAC_TI | IDMAC_RI | IDMAC_NI))) {
		printk(KERN_INFO, "sd:idmac timeout st=%08x idsts=%08x "
		       "ctrl=%08x bmod=%08x dbaddr=%08x dbaddu=%08x pld=%08x "
		       "idinten=%08x imask=%08x st=%08x fcnt=%u "
		       "dsc=%08x buf=%08x\n",
		       r32(MMC_RINTSTS), r32(MMC_IDSTS),
		       r32(MMC_CTRL), r32(MMC_BMOD), r32(MMC_DBADDRL),
		       r32(MMC_DBADDRU), r32(MMC_PLDMND), r32(MMC_IDINTEN),
		       r32(MMC_INTMASK), r32(MMC_STATUS), STATUS_FCNT(r32(MMC_STATUS)),
		       r32(MMC_DSCADDRL), r32(MMC_BUFADDRL));
		printk(KERN_INFO, "sd:idmac desc0=%08x desc2=%08x "
		       "desc4=%08x desc6=%08x desc8=%08x own=%u buf[0]=%08x\n",
		       d->des0, d->des2, d->des4, d->des6, d->des8,
		       !!(d->des0 & DES0_OWN), *(uint32_t *)buf);
		sd_idmac_stop();
		return -1;
	}

	/* clear DMA ints, stop */
	w32(MMC_IDSTS, 0xffffffff);
	w32(MMC_RINTSTS, 0xffffffff);
	sd_idmac_stop();

	if (sd_chatter)
		printk(KERN_INFO, "sd:idmac ok lba=%u 512B\n", lba);
	return 0;
}

/* ---- single-block read: CMD17 via PIO, 64-bit FIFO ----
 * Kernel reference (dw_mmc.c): for data_shift==3 the FIFO is read as
 * 64-bit words from DATA_OFFSET=0x200 (mci_readq + dw_mci_pull_data64).
 * Number of FIFO words free/ready: STATUS bits[31:17] (SDMMC_GET_FCNT).
 * We do NOT touch IDMAC at all here - the data port is the fallback the
 * kernel uses when !using_dma.
 */
static int sd_read_fifo_pio(uint8_t *buf, unsigned words)
{
	/* Kernel reference (dw_mmc_read_data_pio): data arrives via the FIFO
	 * into the CPU data port (DATA_OFFSET=0x200) once RXDR is raised; on
	 * data_shift==3 each 8-byte entry is read as a 64-bit word (readq of
	 * DATA).  FCNT counts entries (SDMMC_GET_FCNT >>17). */
	unsigned t;
	uint32_t st;

	/* wait for the first RXDR (data ready) */
	for (t = 0; t < 2000000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_RXDR | INT_DTO | INT_ERROR))
			break;
		mdelay(1);
	}
	if (!(st & (INT_RXDR | INT_DTO))) {
printk(KERN_INFO, "sd:pio no-rxdr st=%08x stt=%08x verid=%08x\n",
	       st, r32(MMC_STATUS), r32(MMC_VERID));
	return -1;
	}

	while (words > 0) {
		unsigned fcnt = STATUS_FCNT(r32(MMC_STATUS));
		unsigned take;

		if (fcnt == 0) {
			/* FIFO drained momentarily: poll until fresh data
			 * lands, bounded so a dead data line can't hang. */
			for (t = 0; t < 1000000; t++) {
				fcnt = STATUS_FCNT(r32(MMC_STATUS));
				if (fcnt)
					break;
			}
			if (!fcnt) {
				printk(KERN_INFO, "sd:pio stall words=%u st=%08x\n",
				       words, r32(MMC_STATUS));
				return -1;
			}
		}
		take = (fcnt < words) ? fcnt : words;
		/* Drain the whole counted batch back-to-back: FCNT only grows
		 * while the card streams, so no STATUS re-poll per word. */
		while (take--) {
			uint64_t lo = (uint64_t)r32(MMC_DATA);
			uint64_t hi = (uint64_t)r32(MMC_DATA_U);
			*(uint64_t *)buf = lo | (hi << 32);
			buf += 8;
			words--;
		}
	}
	return 0;
}

static int sd_cmd17(uint32_t lba, void *buf)
{
	uint32_t cmd_val, st, v;
	unsigned t;

	/* reset data path cleanly: clear FIFO and any pending DMA state,
	 * make sure IDMAC is OFF for this transfer */
	sd_idmac_stop();
	w32(MMC_RINTSTS, 0xffffffff);

	/* FIFO/DMA reset via RMW to preserve CTRL_INT_ENABLE (kernel keeps
	 * INT_ENABLE set in init_host all the time) */
	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_DMA_RESET | SDMMC_CTRL_FIFO_RESET;
	w32(MMC_CTRL, v);
	for (t = 0; t < 100000; t++) {
		if (!(r32(MMC_CTRL) &
		      (SDMMC_CTRL_DMA_RESET | SDMMC_CTRL_FIFO_RESET)))
			break;
	}
	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_INT_ENABLE;
	w32(MMC_CTRL, v);

	/* no CTRL DMA_ENABLE, no USE_IDMAC, no BMOD: pure PIO */

	/* programming the transfer size is what makes FIFO rev up on RX */
	w32(MMC_BLKSIZ, SD_BLOCK_SIZE);
	w32(MMC_BYTCNT, SD_BLOCK_SIZE);
	w32(MMC_RINTSTS, 0xffffffff);

	w32(MMC_CMDARG, lba);
	dmb();
	cmd_val = CMD_START | CMD_INDX(MMC_READ_SINGLE_BLOCK) |
		CMD_RESP_EXP | CMD_RESP_CRC | CMD_DAT_EXP |
		CMD_PRV_DAT_WAIT;
	w32(MMC_CMD, cmd_val);
	dmb();

	/* wait CMD_DONE (or error) first */
	for (t = 0; t < 1000000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_CMD_DONE | INT_RTO | INT_RESP_ERR))
			break;
	}
	if (st & INT_RTO) {
		printk(KERN_INFO, "sd:pio rto st=%08x stt=%08x\n",
		       st, r32(MMC_STATUS));
		return -1;
	}
	if (st & (INT_RESP_ERR | INT_RCRC)) {
		printk(KERN_INFO, "sd:pio rerr st=%08x stt=%08x\n",
		       st, r32(MMC_STATUS));
		return -1;
	}

	/* drain FIFO: 512 bytes = 64 words of 8 bytes */
	if (sd_read_fifo_pio(buf, SD_BLOCK_SIZE / 8)) {
		return -1;
	}

	/* expect DATA_OVER to confirm the full block arrived */
	for (t = 0; t < 500000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_DATA_OVER | INT_ERROR))
			break;
	}
	if (!(st & INT_DATA_OVER)) {
		printk(KERN_INFO, "sd:pio no-dover st=%08x\n", st);
		return -1;
	}
	w32(MMC_RINTSTS, 0xffffffff);

	if (sd_chatter)
		printk(KERN_INFO, "sd:pio ok lba=%u st=%08x fcnt=%u\n",
		       lba, st, STATUS_FCNT(r32(MMC_STATUS)));
	return 0;
}

/* ---- multi-block read: CMD18 via PIO.  blocks*512 bytes into buf.
 * Kernel reference (dw_mmc.c): CMD18 is handled exactly like CMD17 data
 * transfer, BLKSIZ stays 512 and BYTCNT = blocks*512; FIFO is drained by
 * the same 64-bit pull as single-block.
 * Multi-block reads MUST be terminated with CMD12 STOP (kernel sends stop
 * with SDMMC_CMD_STOP after the data complete) or the card stays in
 * data-transfer state and every later command times out (RTO). */
static void sd_stop_transmission(void)
{
	uint32_t resp[4];

	if (mm_cmd(MMC_STOP_TRANSMISSION, 0,
		   CMD_STOP | CMD_RESP_EXP | CMD_RESP_CRC, resp, 0) < 0)
		printk(KERN_INFO, "sd:pio18 stop-failed\n");
}

static int sd_cmd18(uint32_t lba, void *buf, unsigned blocks)
{
	uint32_t cmd_val, st, v;
	unsigned t;

	/* reset data path cleanly, IDMAC OFF (same as CMD17) */
	sd_idmac_stop();
	w32(MMC_RINTSTS, 0xffffffff);

	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_DMA_RESET | SDMMC_CTRL_FIFO_RESET;
	w32(MMC_CTRL, v);
	for (t = 0; t < 100000; t++) {
		if (!(r32(MMC_CTRL) &
		      (SDMMC_CTRL_DMA_RESET | SDMMC_CTRL_FIFO_RESET)))
			break;
	}
	v = r32(MMC_CTRL);
	v |= SDMMC_CTRL_INT_ENABLE;
	w32(MMC_CTRL, v);

	w32(MMC_BLKSIZ, SD_BLOCK_SIZE);
	w32(MMC_BYTCNT, blocks * SD_BLOCK_SIZE);
	w32(MMC_RINTSTS, 0xffffffff);

	w32(MMC_CMDARG, lba);
	dmb();
	cmd_val = CMD_START | CMD_INDX(MMC_READ_MULTIPLE_BLOCK) |
		CMD_RESP_EXP | CMD_RESP_CRC | CMD_DAT_EXP |
		CMD_PRV_DAT_WAIT;
	w32(MMC_CMD, cmd_val);
	dmb();

	for (t = 0; t < 1000000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_CMD_DONE | INT_RTO | INT_RESP_ERR))
			break;
	}
	if (st & (INT_RTO | INT_RESP_ERR | INT_RCRC)) {
		printk(KERN_INFO, "sd:pio18 err st=%08x stt=%08x\n",
		       st, r32(MMC_STATUS));
		sd_stop_transmission();
		return -1;
	}

	if (sd_read_fifo_pio(buf, blocks * (SD_BLOCK_SIZE / 8))) {
		sd_stop_transmission();
		return -1;
	}

	for (t = 0; t < 500000; t++) {
		st = r32(MMC_RINTSTS);
		if (st & (INT_DATA_OVER | INT_ERROR))
			break;
	}
	if (!(st & INT_DATA_OVER)) {
		printk(KERN_INFO, "sd:pio18 no-dover st=%08x\n", st);
		sd_stop_transmission();
		return -1;
	}
	w32(MMC_RINTSTS, 0xffffffff);

	sd_stop_transmission();

	if (sd_chatter)
		printk(KERN_INFO, "sd:pio18 ok lba=%u blocks=%u st=%08x fcnt=%u\n",
		       lba, blocks, st, STATUS_FCNT(r32(MMC_STATUS)));
	return 0;
}


/*
 * sd_tune_clock - raise CLKDIV until CMD18 stops verifying, then rewind
 * to the last good divider.  CLKSEL must already be set for high speed.
 * MBR signature (0x55 0xaa at offsets 510/511) is our read-back check.
 */
static void sd_tune_clock(void)
{
	static const uint32_t divs[] = {
		0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00,
	};
	uint8_t *b = sd_dma_buf;
	unsigned i;
	uint32_t good = 0xff;

	for (i = 0; i < sizeof(divs) / sizeof(divs[0]); i++) {
		w32(MMC_CLKENA, 0);
		upd_clk();
		w32(MMC_CLKDIV, divs[i]);
		upd_clk();
		w32(MMC_CLKENA, SDMMC_CLKEN_ENABLE);
		upd_clk();
		printk(KERN_INFO, "sd:tune div=%02x\n", divs[i]);

		if (sd_cmd18(0, b, 4)) {
			printk(KERN_INFO, "sd:tune div=%02x FAIL, keep %02x\n",
			       divs[i], good);
			break;
		}
		if (b[510] != 0x55 || b[511] != 0xaa) {
			printk(KERN_INFO, "sd:tune div=%02x badsig=%02x%02x,"
			       " keep %02x\n", divs[i], b[510], b[511], good);
			break;
		}
		good = divs[i];
	}

	w32(MMC_CLKENA, 0);
	upd_clk();
	w32(MMC_CLKDIV, good);
	upd_clk();
	w32(MMC_CLKENA, SDMMC_CLKEN_ENABLE);
	upd_clk();
	printk(KERN_INFO, "sd:clk-final div=%02x\n", good);
}

/*
 * sd_set_bus_speed - after CMD7 selection:
 *  1) 4-bit bus width via ACMD6, then CTYPE=1;
 *  2) switch to high speed via CMD6 (0x80FFFFF1), then CLKSEL=sdr-hs;
 *  3) raise CLKDIV until reads stop verifying.
 */
static void sd_set_bus_speed(void)
{
	uint32_t resp[4];

	/* 4-bit bus width.  Kernel does this right after card select:
	 * mmc_select_bus_width -> SD_SET_BUS_WIDTH via CMD55+ACMD6. */
	if (mm_cmd(MMC_APP_CMD, sd_rca << 16,
		   CMD_RESP_EXP | CMD_RESP_CRC, resp, 0) < 0)
		return;
	if (mm_cmd(SD_SET_BUS_WIDTH, SD_ACMD6_ARG_4BIT,
		   CMD_RESP_EXP | CMD_RESP_CRC, resp, 0) < 0)
		return;
	w32(MMC_CTYPE, CTYPE_4BIT);
	printk(KERN_INFO, "sd:bus-4bit ok\n");

	/* high-speed switch: CMD6 arg 0x80FFFFF1 (mode-check + switch
	 * function 1 group 1).  Kernel sd_init_card does this after bus. */
	if (mm_cmd(SD_SWITCH_FUNC, SD_CMD6_ARG_SWITCH_HS,
		   CMD_RESP_EXP | CMD_RESP_CRC, resp, 0) < 0) {
		printk(KERN_INFO, "sd:hs CMD6 fail, legacy timing\n");
		w32(MMC_CLKSEL, SD_LEGACY_CLKSEL);
	} else {
		printk(KERN_INFO, "sd:hs ok\n");
		w32(MMC_CLKSEL, SD_HS_CLKSEL);
	}

	sd_tune_clock();
}


/*
 * sd_read_sectors - read arbitrary sector count from SD card
 * @lba:    start sector (byte-addressed for SDSC, sector-addressed for SDHC)
 * @count:  number of 512-byte sectors
 * @buf:    destination buffer
 *
 * Returns 0 on success, negative on error.
 * Single-block CMD17 via PIO draining the 64-bit FIFO (DATA at 0x200,
 * readq-style, like dw_mci_pull_data64).  No IDMAC involved.
 */
int sd_read_sectors(uint32_t lba, unsigned count, void *buf)
{
	uint8_t *p = (uint8_t *)buf;
	int rc;

	if (!sd_ready)
		return -1;

	/* Multi-block CMD18, 512KB per call: BLKSIZ=512, BYTCNT fine, and
	 * PIO downshifts to the 64-bit FIFO as data arrives. */
	while (count > 0) {
		unsigned blocks = count < 1024 ? count : 1024;

		rc = sd_cmd18(lba, p, blocks);
		if (rc)
			return rc;
		lba += blocks;
		p += blocks * SD_BLOCK_SIZE;
		count -= blocks;
	}
	return 0;
}

int sd_get_sectors(void) { return sd_ready ? (int)sd_sectors : 0; }
int sd_get_rca(void) { return sd_ready ? (int)sd_rca : -1; }
int sd_is_ready(void) { return sd_ready; }

/* ==================================================================
 * SD init: called once from board late_init.
 * After successful init, sd_rca / sd_sectors / sd_ready are set and
 * subsequent sd_read_sectors() calls work.
 * ================================================================== */
void sd_host_probe_3475(void)
{
	uint32_t resp[4];
	int i, rc, t2;
	uint32_t csd_struct;

	printk(KERN_INFO, "sd:probe start\n");

	/* vmmc on */
	w32(PIN_GPC0_CON, (r32(PIN_GPC0_CON) & ~0xf) | 0x1);
	w32(PIN_GPC0_DAT, r32(PIN_GPC0_DAT) | 0x1);
	mdelay(20000);

	/* sd2 pins fn=2 */
	w32(PIN_GPR4_CON, 0x22222222);
	w32(PIN_GPR4_PUD, 0);

	/* CD */
	printk(KERN_INFO, "sd:cd=%u\n", (r32(PIN_GPA1_DAT) >> 3) & 1);

	/* Reset */
	printk(KERN_INFO, "sd:reset\n");
	wait_reset(SDMMC_CTRL_RESET, 200000);
	wait_reset(SDMMC_CTRL_FIFO_RESET, 200000);
	w32(MMC_CTRL, SDMMC_CTRL_INT_ENABLE);

	/* FIFO threshold for 64-bit FIFO (HCON=2): kernel computes
	 * msize=3 rx_wmark=15 tx_wmark=48 -> 0x300F0030.  BRING-UP (J1):
	 * raise rx_wmark to 63 (fill the 64-entry FIFO before RXDR) so the
	 * PIO drain moves data in deep 63-entry batches instead of 15-entry
	 * dribbles - ~3x fewer CPU stalls on the cacheslice bulk read. */
	w32(MMC_FIFOTH, 0x303f0030);
	printk(KERN_INFO, "sd:fifoth=%08x\n", r32(MMC_FIFOTH));

	/* IDMAC one-time setup */
	sd_idmac_init();

	set_slow_clock();

	/* 74 clocks */
	mdelay(10);

	/* ---- SD init sequence ---- */

	rc = mm_cmd(MMC_GO_IDLE_STATE, 0, 0, NULL, 1);
	printk(KERN_INFO, "sd:CMD0 rc=%d\n", rc);
	if (rc < 0)
		return;

	/* CMD8 - check SDv2 */
	rc = mm_cmd(SD_SEND_IF_COND, SD_IF_COND_ARG,
		    CMD_RESP_EXP | CMD_RESP_CRC, resp, 0);
	printk(KERN_INFO, "sd:CMD8 rc=%d r0=%08x\n", rc, resp[0]);
	if (rc < 0)
		return;
	if ((resp[0] & 0xff) != 0xaa || ((resp[0] >> 8) & 0xf) != 1) {
		printk(KERN_INFO, "sd:CMD8 bad-echo\n");
		return;
	}

	/* ACMD41 loop */
	for (i = 0; i < 400; i++) {
		rc = -1;
		for (t2 = 0; t2 < 6 && rc < 0; t2++) {
			rc = mm_cmd(MMC_APP_CMD, 0,
				    CMD_RESP_EXP | CMD_RESP_CRC, resp, 0);
			if (rc < 0)
				mdelay(2000);
		}
		if (rc < 0) {
			printk(KERN_INFO, "sd:ACMD41 CMD55 failed %dx\n", t2);
			return;
		}
		if (!(resp[0] & R1_APP_CMD))
			return;
		if (mm_cmd(SD_APP_OP_COND, SD_ACMD41_ARG,
			   CMD_RESP_EXP, resp, 0) < 0)
			return;
		if (resp[0] & MMC_CARD_BUSY)
			break;
		mdelay(10000);
	}
	if (i >= 400)
		return;
	printk(KERN_INFO, "sd:ACMD41 ok ocr=%08x\n", resp[0]);

	/* ALL_SEND_CID */
	if (mm_cmd(MMC_ALL_SEND_CID, 0,
		   CMD_RESP_EXP | CMD_RESP_CRC | CMD_RESP_LONG,
		   resp, 0) < 0)
		return;

	/* SET_RELATIVE_ADDR */
	if (mm_cmd(MMC_SET_RELATIVE_ADDR, 0,
		   CMD_RESP_EXP | CMD_RESP_CRC, resp, 0) < 0)
		return;
	sd_rca = (resp[0] >> 16) & 0xffff;
	printk(KERN_INFO, "sd:RCA=%04x\n", sd_rca);

	/* SEND_CSD - CSD is read in stand-by state, BEFORE CMD7 select.
	 * Kernel order: CMD2 -> CMD3 -> CMD9, THEN CMD7 select. */
	if (mm_cmd(MMC_SEND_CSD, sd_rca << 16,
		   CMD_RESP_EXP | CMD_RESP_CRC | CMD_RESP_LONG,
		   resp, 0) < 0)
		return;

	csd_struct = mbit(resp, 126, 2);
	printk(KERN_INFO, "sd:CSD=%08x %08x %08x %08x struct=%u\n",
	       resp[0], resp[1], resp[2], resp[3], csd_struct);

	if (csd_struct == 1)
		sd_sectors = (uint64_t)(1 + mbit(resp, 48, 22)) << 10;
	else if (csd_struct == 0)
		sd_sectors = (uint64_t)(1 + mbit(resp, 62, 12))
			  << (mbit(resp, 47, 3) + 2);

	printk(KERN_INFO, "sd:SIZE=%uMB (%u sectors)\n",
	       (unsigned)(sd_sectors >> 11),
	       (unsigned)(sd_sectors));

	/* CMD7 SELECT_CARD: move from stand-by to transfer state.
	 * CMD17/CMD18 without selection get RTO. */
	rc = mm_cmd(MMC_SELECT_CARD, sd_rca << 16,
		    CMD_RESP_EXP | CMD_RESP_CRC, resp, 0);
	printk(KERN_INFO, "sd:CMD7 sel rc=%d\n", rc);
	if (rc < 0)
		return;

	sd_set_bus_speed();

	/* ---- test read: CMD17 sector 0 via IDMAC to verify DMA path ---- */
	{
		uint8_t *b = sd_dma_buf;
		unsigned chk = 0, j;

		printk(KERN_INFO, "sd:test-read lba=0 ...\n");
		rc = sd_cmd17(0, b);
		if (rc == 0) {
			for (j = 0; j < 128; j++)
				chk += ((uint32_t *)b)[j];
			printk(KERN_INFO, "sd:test-read ok chk=%08x "
			       "sig=%02x%02x mbr=%02x%02x%02x%02x "
			       "type=%02x lba=%u\n",
			       chk, b[510], b[511], b[0], b[1], b[2], b[3],
			       b[450],
			       (b[454] | (b[455]<<8) | (b[456]<<16) |
				((unsigned)b[457]<<24)));
		} else {
			printk(KERN_INFO, "sd:test-read FAILED rc=%d\n", rc);
			dump_regs("post-read");
		}
	}

	sd_ready = 1;
	printk(KERN_INFO, "sd:init done (%u sectors)\n",
	       (unsigned)sd_sectors);

#if defined(CONFIG_EXYNOS_SD_FAT)
	sd_fat_probe();
#endif
}

#if defined(CONFIG_EXYNOS_SD_FAT)
int sd_fat_mount(void);
int sd_fat_read(const char *name, void *dst, unsigned maxlen);

/* After SD init: mount the FAT32 partition and probe the four boot blobs
 * by reading their first few KB.  Prints size + stored header magic so a
 * broken LBAs/cluster chain is caught before it reaches load-kernel. */
void sd_fat_probe(void)
{
	static const char *files[] = {
		"Image", "dtb", "ramdisk", "cacheslice",
	};
	unsigned i;

	if (sd_fat_mount()) {
		printk(KERN_INFO, "sd:fat mount failed\n");
		return;
	}

	for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
		int n = sd_fat_read(files[i], sd_dma_buf, 512);

		if (n < 0) {
			printk(KERN_INFO, "sd:fat %s not-found\n", files[i]);
			continue;
		}
		/* Report the first 16 bytes of each blob: lz4 magic
		 * 04 22 4d 18, FDT 00 d0 0d fe, RDSC payload .. */
		printk(KERN_INFO, "sd:fat %s bytes=%d hdr=%02x %02x %02x "
		       "%02x | %02x %02x %02x %02x\n",
		       files[i], n,
		       sd_dma_buf[0], sd_dma_buf[1], sd_dma_buf[2], sd_dma_buf[3],
		       sd_dma_buf[4], sd_dma_buf[5], sd_dma_buf[6], sd_dma_buf[7]);
	}
}
#endif
