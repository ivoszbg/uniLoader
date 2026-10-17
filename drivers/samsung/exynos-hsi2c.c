// SPDX-License-Identifier: GPL-2.0-only
/*
 * Minimal polling host driver for Exynos 3475 HSI2C ch0 (0x13870000)
 * and SM5703 MUIC 5V-boost enable (sources VBUS for USB OTG).
 *
 * Reference: kernel-j120f drivers/i2c/busses/i2c-exynos5.c (auto mode,
 * check-transdone-int) and drivers/battery/sm5703_charger.c
 * (sm5703_charger_otg_control).
 *
 * Only what the USB stage needs: SMBus-style single register read/write
 * on hsi2c_0 (the bus the SM5703 MUIC sits on, addr 0x25).  A read is
 * two auto-mode messages: write { reg }, then read { data }.
 */

#include <stdint.h>
#include <stddef.h>
#include <lib/debug.h>

/* SoC bases (PA = VA, MMU off) */
#define EXYNOS3475_HSI2C0_BASE		0x13870000
#define EXYNOS3475_CMU_PERI_BASE	0x139F0000
#define EXYNOS3475_CMU_TOP_BASE		0x10010000
#define EXYNOS3475_CLK_ENABLE_ACLK_PERI_66_1 \
				(EXYNOS3475_CMU_PERI_BASE + 0x800)
#define ACLK_PERI66_MUX_USER_BASE	(EXYNOS3475_CMU_PERI_BASE + 0x200)
#define ACLK_PERI66_DIV_BASE		(EXYNOS3475_CMU_TOP_BASE + 0x414)

/* HSI2C register map (i2c-exynos5.h) */
#define HSI2C_CTL		0x00
#define HSI2C_FIFO_CTL		0x04
#define HSI2C_TRAILIG_CTL	0x08
#define HSI2C_INT_ENABLE	0x20
#define HSI2C_INT_STATUS	0x24
#define HSI2C_ERR_STATUS	0x2C
#define HSI2C_FIFO_STATUS	0x30
#define HSI2C_TX_DATA		0x34
#define HSI2C_RX_DATA		0x38
#define HSI2C_CONF		0x40
#define HSI2C_AUTO_CONF		0x44
#define HSI2C_TIMEOUT		0x48
#define HSI2C_TRANS_STATUS	0x50
#define HSI2C_TIMING_FS1	0x60
#define HSI2C_TIMING_FS2	0x64
#define HSI2C_TIMING_FS3	0x68
#define HSI2C_TIMING_SLA	0x6C
#define HSI2C_ADDR		0x70

/* CTL */
#define HSI2C_FUNC_MODE_I2C	(1u << 0)
#define HSI2C_MASTER		(1u << 3)
#define HSI2C_RXCHON		(1u << 6)
#define HSI2C_TXCHON		(1u << 7)
#define HSI2C_SW_RST		(1u << 31)
/* FIFO_CTL */
#define HSI2C_RXFIFO_EN		(1u << 0)
#define HSI2C_TXFIFO_EN		(1u << 1)
#define HSI2C_RXFIFO_TRIGGER	(0x8u << 4)
#define HSI2C_TXFIFO_TRIGGER	(0x8u << 16)
#define HSI2C_FIFO_MAX		(0x40)
/* CONF */
#define HSI2C_AUTO_MODE		(1u << 31)
/* AUTO_CONF */
#define HSI2C_READ_WRITE	(1u << 16)
#define HSI2C_STOP_AFTER_TRANS	(1u << 17)
#define HSI2C_MASTER_RUN	(1u << 31)
/* INT */
#define HSI2C_INT_TX_ALMOSTEMPTY_EN	(1u << 0)
#define HSI2C_INT_RX_ALMOSTFULL_EN	(1u << 1)
#define HSI2C_INT_TRAILING_EN		(1u << 6)
#define HSI2C_INT_TRANSFER_DONE		(1u << 7)
#define HSI2C_INT_CHK_TRANS_STATE	(0xfu << 8)

/* TRANS_STATUS */
#define HSI2C_NO_DEV		(1u << 3)
#define HSI2C_NO_DEV_ACK	(1u << 2)
#define HSI2C_TRANS_ABORT	(1u << 1)
#define HSI2C_TRANS_DONE	(1u << 0)

#define HSI2C_RX_FIFO_LVL(x)	(((x) >> 16) & 0x7f)
#define HSI2C_TX_FIFO_LVL(x)	((x) & 0x7f)

#define HSI2C_TIMEOUT_LOOPS	50000

/* S2MU005 (PMIC+MUIC+charger) on hsi2c_0, verified by REV_ID read */
#define S2MU005_ADDR		0x3D
#define S2MU005_REV_ID		0x73
#define S2MU005_CHG_CTRL0	0x0E	/* REG_MODE[2:0] = 0b100 = OTG */
#define S2MU005_CHG_CTRL4	0x12	/* OCP[3:2]=0b11, SW_OFF[4]=1 */
#define S2MU005_CHG_CTRL5	0x13	/* VF_VMID_BST[4:0]=0x16 (5.1V) */
#define S2MU005_CHG_CTRL11	0x19	/* OSC_BST[6:5]=0b10 (1MHz) */
#define S2MU005_MUIC_CTRL1	0xB2	/* MANUAL_SW bit */
#define S2MU005_MUIC_SW_CTRL	0xB5	/* DM[7:5] DP[4:2] JIG[0] */
#define CTRL_MANUAL_SW_SHIFT	2
#define MANUAL_SW_USB		(0x1u << 5 | 0x1u << 2)	/* D+/D- to AP */
#define MANUAL_SW_OTGEN		(0x1u)

#define I2C	(cbus)
#define CMU_PERI (volatile uint32_t *)EXYNOS3475_CMU_PERI_BASE
#define CMU_TOP  (volatile uint32_t *)EXYNOS3475_CMU_TOP_BASE

static unsigned int hsi2c_silent;	/* suppress timeout spam during scan */
static unsigned int hsi2c_tries = HSI2C_TIMEOUT_LOOPS;
static unsigned int hsi2c_op_clk;	/* Hz, current SCL timing target */
static volatile uint32_t *cbus;		/* active controller base */

struct hsi2c_bus;	/* defined below */

static void hsi2c0_init_op(unsigned int op_clk);
static void hsi2c_bus_prepare(const struct hsi2c_bus *b, unsigned int op_clk);
static void hsi2c0_scan_at(const char *tag);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#endif

/* candidate HSI2C controllers: base, gate bit, pin group config.
 * gpc1-0/1 -> hsi2c0, gpc1-2/3 -> hsi2c1, gpf0-0/1 -> hsi2c7,
 * gpf0-2/3 -> hsi2c8.  All on aclk_peri_66. */
struct hsi2c_bus {
	uint32_t base;
	uint32_t gatebit;
	uint32_t con;		/* GPCON address */
	uint32_t pud;		/* PUD address */
	uint32_t con_mask;	/* pin fields we touch */
	uint32_t fn;		/* fn2 on the i2c pins */
	uint32_t pud_mask;	/* pull-up on the i2c pins */
};
static const struct hsi2c_bus hsi2c_buses[] = {
	{ 0x13870000, 15, 0x139C0080, 0x139C0088, 0x0F, 0x0A, 0x0F },
	{ 0x13880000, 16, 0x139C0080, 0x139C0088, 0xF0, 0xA0, 0xF0 },
	{ 0x138A0000, 17, 0x139C0120, 0x139C0128, 0x0F, 0x0A, 0x0F },
	{ 0x138B0000, 18, 0x139C0120, 0x139C0128, 0xF0, 0xA0, 0xF0 },
};

static inline uint32_t hsi2c_rd(unsigned int o)
{
	return *(I2C + (o >> 2));
}

static inline void hsi2c_wr(unsigned int o, uint32_t v)
{
	*(I2C + (o >> 2)) = v;
}

static inline void hsi2c_dmb(void)
{
	__asm__ __volatile__("dmb" ::: "memory");
}

static void hsi2c_delay_us(unsigned int us)
{
	volatile unsigned int t = us * 400;

	while (t--)
		;
}

/* enable gate clock: aclk_peri_66_1 ipclk of the controller */
static void hsi2c_gate_enable(uint32_t bit)
{
	uint32_t v;

	v = *(CMU_PERI + ((EXYNOS3475_CLK_ENABLE_ACLK_PERI_66_1 -
			   EXYNOS3475_CMU_PERI_BASE) >> 2)) | (1u << bit);
	*(CMU_PERI + ((EXYNOS3475_CLK_ENABLE_ACLK_PERI_66_1 -
		       EXYNOS3475_CMU_PERI_BASE) >> 2)) = v;
	hsi2c_dmb();
	hsi2c_delay_us(200);
}

/* switch the muxed pins to function 2 (I2C) + internal pull-up */
static void hsi2c_pinmux_setup(const struct hsi2c_bus *b)
{
	volatile uint32_t *con = (volatile uint32_t *)b->con;
	volatile uint32_t *pud = (volatile uint32_t *)b->pud;

	*con = (*con & ~b->con_mask) | b->fn;
	*pud = (*pud & ~b->pud_mask) | b->pud_mask;
	hsi2c_dmb();
	hsi2c_delay_us(50);
}

static void hsi2c_bus_prepare(const struct hsi2c_bus *b, unsigned int op_clk)
{
	cbus = (volatile uint32_t *)b->base;
	hsi2c_gate_enable(b->gatebit);
	hsi2c0_init_op(op_clk);
	hsi2c_pinmux_setup(b);
}

/* master init, mirrors exynos5_i2c_init() + exynos5_i2c_set_timing()
 * for fast (non-HS, 7-bit) mode.  clkin = aclk_peri_66 = 66 MHz (clk 307).
 * `op_clk` is the target SCL (400 kHz fast, 50 kHz for the slow probe). */
static void hsi2c0_init_op(unsigned int op_clk)
{
	uint32_t conf, t_ftl_cycle, utemp0, clk_cycle = 0;
	uint32_t s1, s2, s3, sla;
	unsigned int div, utemp1;

	hsi2c_op_clk = op_clk;

	if (hsi2c_rd(HSI2C_TRANS_STATUS) != 0) {
		uint32_t ctl = hsi2c_rd(HSI2C_CTL);

		hsi2c_wr(HSI2C_CTL, ctl | HSI2C_SW_RST);
		hsi2c_wr(HSI2C_CTL, ctl & ~HSI2C_SW_RST);
	}

	hsi2c_wr(HSI2C_CTL, HSI2C_FUNC_MODE_I2C | HSI2C_MASTER);
	hsi2c_wr(HSI2C_TRAILIG_CTL, 0xf);

	/* disable h/w transfer timeout (kernel does the same) */
	hsi2c_wr(HSI2C_TIMEOUT, hsi2c_rd(HSI2C_TIMEOUT) & ~(1u << 31));

	conf = hsi2c_rd(HSI2C_CONF);
	hsi2c_wr(HSI2C_CONF, conf | HSI2C_AUTO_MODE);

	t_ftl_cycle = (conf >> 16) & 0x7;
	utemp0 = (66000000u / op_clk) - 8 - t_ftl_cycle;
	for (div = 0; div < 256; div++) {
		utemp1 = utemp0 / (div + 1);
		if (utemp1 < 512 && utemp1 > 4) {
			clk_cycle = utemp1 - 2;
			break;
		}
	}
	s1 = (clk_cycle / 2) << 24 | (clk_cycle / 2) << 16 |
	     (clk_cycle / 2) << 8;
	s2 = ((clk_cycle / 2) / 2) << 24 | (clk_cycle / 2) << 8 |
	     (clk_cycle / 2);
	s3 = div << 16 | clk_cycle;
	sla = (clk_cycle / 2) / 2;
	hsi2c_wr(HSI2C_TIMING_FS1, s1);
	hsi2c_wr(HSI2C_TIMING_FS2, s2);
	hsi2c_wr(HSI2C_TIMING_FS3, s3);
	hsi2c_wr(HSI2C_TIMING_SLA, sla);

	/* RX+TX fifos for both directions, default trigger levels */
	hsi2c_wr(HSI2C_FIFO_CTL,
		 HSI2C_RXFIFO_EN | HSI2C_TXFIFO_EN |
		 HSI2C_TXFIFO_TRIGGER | HSI2C_RXFIFO_TRIGGER);
	hsi2c_dmb();
	hsi2c_delay_us(100);
}

static void hsi2c0_init(void)
{
	hsi2c0_init_op(400000);
}

static void hsi2c0_sysreset(void)
{
	uint32_t ctl = hsi2c_rd(HSI2C_CTL);

	hsi2c_wr(HSI2C_CTL, ctl | HSI2C_SW_RST);
	hsi2c_wr(HSI2C_CTL, ctl & ~HSI2C_SW_RST);
	hsi2c0_init();
}

/* one auto-mode transaction; rd!=0 reads `len` bytes into out */
static int hsi2c0_msg(uint8_t client, const uint8_t *tx, unsigned int txlen,
		      uint8_t *out, unsigned int rdlen)
{
	uint32_t ctl, auto_conf, addr, int_st;
	unsigned int i, got = 0;

	/* NACK leaves the data byte in the TX fifo; a full fifo wedges the
	 * next transaction, so start from a clean slate. */
	if ((hsi2c_rd(HSI2C_FIFO_STATUS) & 0x7f) ||
	    HSI2C_RX_FIFO_LVL(hsi2c_rd(HSI2C_FIFO_STATUS)))
		hsi2c0_sysreset();

	ctl = hsi2c_rd(HSI2C_CTL);
	ctl |= HSI2C_MASTER;
	if (rdlen) {
		ctl &= ~HSI2C_TXCHON;
		ctl |= HSI2C_RXCHON;
	} else {
		ctl &= ~HSI2C_RXCHON;
		ctl |= HSI2C_TXCHON;
	}
	hsi2c_wr(HSI2C_CTL, ctl);

	addr = hsi2c_rd(HSI2C_ADDR);
	addr &= ~(0x3ffu << 10);
	addr |= ((client & 0x7f) << 10);
	hsi2c_wr(HSI2C_ADDR, addr);

	auto_conf = hsi2c_rd(HSI2C_AUTO_CONF);
	auto_conf &= ~0xffff;
	auto_conf &= ~HSI2C_READ_WRITE;
	if (rdlen) {
		auto_conf |= rdlen | HSI2C_READ_WRITE |
			     HSI2C_STOP_AFTER_TRANS;
	} else {
		auto_conf |= txlen | HSI2C_STOP_AFTER_TRANS;
	}
	hsi2c_wr(HSI2C_AUTO_CONF, auto_conf);

	/* clear pending ints, arm transfer-done */
	hsi2c_wr(HSI2C_INT_STATUS, hsi2c_rd(HSI2C_INT_STATUS));
	hsi2c_wr(HSI2C_INT_ENABLE,
		 HSI2C_INT_TX_ALMOSTEMPTY_EN | HSI2C_INT_RX_ALMOSTFULL_EN |
		 HSI2C_INT_TRAILING_EN | HSI2C_INT_CHK_TRANS_STATE |
		 HSI2C_INT_TRANSFER_DONE);

	auto_conf |= HSI2C_MASTER_RUN;
	hsi2c_wr(HSI2C_AUTO_CONF, auto_conf);

	/* push tx bytes into the fifo after run (auto mode pulls from it) */
	for (i = 0; i < txlen; ) {
		if (HSI2C_TX_FIFO_LVL(hsi2c_rd(HSI2C_FIFO_STATUS)) <
		    HSI2C_FIFO_MAX) {
			hsi2c_wr(HSI2C_TX_DATA, tx[i]);
			i++;
		}
	}

	for (i = 0; i < hsi2c_tries; i++) {
		int_st = hsi2c_rd(HSI2C_INT_STATUS);
		if (int_st & HSI2C_INT_TRANSFER_DONE)
			break;
		hsi2c_delay_us(1);
	}
	if (i >= hsi2c_tries) {
		if (!hsi2c_silent)
			printk(KERN_ERR, "usb:hsi2c timeout rdlen=%u txlen=%u st=%08x "
			       "int=%08x auto=%08x ctl=%08x fs=%08x\n",
			       rdlen, txlen, hsi2c_rd(HSI2C_TRANS_STATUS), int_st,
			       hsi2c_rd(HSI2C_AUTO_CONF), hsi2c_rd(HSI2C_CTL),
			       hsi2c_rd(HSI2C_FIFO_STATUS));
		return -1;
	}

	hsi2c_wr(HSI2C_INT_STATUS, int_st);
	hsi2c_wr(HSI2C_INT_ENABLE, 0);

	if (hsi2c_rd(HSI2C_TRANS_STATUS) & (HSI2C_NO_DEV |
					   HSI2C_NO_DEV_ACK | HSI2C_TRANS_ABORT)) {
		printk(KERN_ERR, "usb:hsi2c nack st=%08x fs=%08x ctl=%08x "
		       "fs1=%08x fs3=%08x\n",
		       hsi2c_rd(HSI2C_TRANS_STATUS),
		       hsi2c_rd(HSI2C_FIFO_STATUS),
		       hsi2c_rd(HSI2C_CTL),
		       hsi2c_rd(HSI2C_TIMING_FS1),
		       hsi2c_rd(HSI2C_TIMING_FS3));
		return -2;
	}

	if (rdlen) {
		while (got < rdlen) {
			uint32_t fs = hsi2c_rd(HSI2C_FIFO_STATUS);

			if (HSI2C_RX_FIFO_LVL(fs) == 0)
				break;
			out[got++] = (uint8_t)hsi2c_rd(HSI2C_RX_DATA);
		}
		if (got != rdlen)
			return -3;
	}

	return 0;
}

/* SMBus read byte: write {reg} then read 1 byte */
static int hsi2c0_read_reg(uint8_t client, uint8_t reg, uint8_t *val)
{
	uint8_t tx = reg;
	int r;

	r = hsi2c0_msg(client, &tx, 1, NULL, 0);
	if (r)
		return r;
	return hsi2c0_msg(client, NULL, 0, val, 1);
}

/* SMBus write byte to reg: write {reg, val} */
static int hsi2c0_write_reg(uint8_t client, uint8_t reg, uint8_t val)
{
	uint8_t tx[2] = { reg, val };

	return hsi2c0_msg(client, tx, 2, NULL, 0);
}

/* probe every 7-bit address, print those that ACK */
static void hsi2c0_scan_at(const char *tag)
{
	uint8_t one = 0x00;
	unsigned int a;

	hsi2c_silent = 1;
	hsi2c_tries = 2000;
	printk(KERN_INFO, "usb:i2c[%x] scan %s...\n",
	       (unsigned int)((uint32_t)cbus >> 16), tag);
	for (a = 0x01; a < 0x7f; a++) {
		if (hsi2c0_msg(a, &one, 1, NULL, 0) == 0)
			printk(KERN_INFO, "usb:i2c[%x] ack %02x (%s)\n",
			       (unsigned int)((uint32_t)cbus >> 16), a, tag);
	}
	hsi2c_tries = HSI2C_TIMEOUT_LOOPS;
	hsi2c_silent = 0;
	printk(KERN_INFO, "usb:i2c[%x] scan %s done\n",
	       (unsigned int)((uint32_t)cbus >> 16), tag);
}

/* scan every candidate HSI2C controller at fast speed */
static void hsi2c_scan_all(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hsi2c_buses); i++) {
		hsi2c_bus_prepare(&hsi2c_buses[i], 400000);
		hsi2c0_scan_at("fast");
	}
}

/* dump untouched state, as S-Boot left it, to diff against what works */
static void hsi2c0_dump_raw(void)
{
	static const struct reg { uint32_t off; const char *n; } regs[] = {
		{ 0x00, "ctl" }, { 0x04, "fifo_ctl" }, { 0x08, "trail" },
		{ 0x20, "int_en" }, { 0x24, "int_st" }, { 0x2c, "err" },
		{ 0x30, "fifo_st" }, { 0x40, "conf" }, { 0x44, "auto" },
		{ 0x48, "timeout" }, { 0x50, "trans" }, { 0x60, "fs1" },
		{ 0x64, "fs2" }, { 0x68, "fs3" }, { 0x6c, "sla" }, { 0x70, "addr" },
	};
	unsigned int i;

	cbus = (volatile uint32_t *)0x13870000;
	printk(KERN_INFO, "usb:sboot gpc1 con=%08x pud=%08x dat=%08x\n",
	       *(volatile uint32_t *)0x139C0080,
	       *(volatile uint32_t *)0x139C0088,
	       *(volatile uint32_t *)0x139C0084);
	for (i = 0; i < ARRAY_SIZE(regs); i++)
		printk(KERN_INFO, "usb:sboot %-8s %08x\n", regs[i].n,
		       hsi2c_rd(regs[i].off));
}

/* ------- bit-banged I2C over gpc1-0(SDA)/gpc1-1(SCL) --------
 * Probes whether the s2mu005 is electrically responsive AT THIS
 * BOOT STAGE, independent of the HSI2C IP block.  Pins are taken
 * out of fn2 into GPIO, pull-ups on, and a slow 7-bit write is
 * done per candidate address; SDA is sampled in the ACK slot.
 * A hit means "wire + chip fine, HSI2C init is the problem"; a
 * clean miss means "chip not reachable/powered yet".  Restored to
 * fn2 afterwards by hsi2c_bus_prepare().                        */
#define GPB_CON	0x139C0080
#define GPB_DAT	0x139C0084
#define GPB_PUD	0x139C0088

static void gpb_cfg(volatile uint32_t *C, unsigned int scl_out,
		    unsigned int sda_out)
{
	uint32_t v = *C & ~0xFFu;

	if (scl_out) v |= 0x10u;	/* pin1 fn1 = output */
	if (sda_out) v |= 0x01u;	/* pin0 fn1 = output */
	*C = v;
	hsi2c_dmb();
}

static void gpb_scl(volatile uint32_t *D, int v)
{
	if (v) *D |= 0x2u; else *D &= ~0x2u;
}

static void gpb_sda(volatile uint32_t *D, int v)
{
	if (v) *D |= 0x1u; else *D &= ~0x1u;
}

/* both pins GPIO input with pull-up; also used after any HSI2C use */
static void gpb_bus_release(volatile uint32_t *C, volatile uint32_t *P)
{
	*C &= ~0xFFu;
	*P = (*P & ~0x0Fu) | 0x0Fu;
	hsi2c_dmb();
	hsi2c_delay_us(50);
}

static void gpb_start(volatile uint32_t *C, volatile uint32_t *D)
{
	gpb_cfg(C, 1, 1);
	gpb_sda(D, 1); gpb_scl(D, 1);
	hsi2c_delay_us(3);
	gpb_sda(D, 0);
	hsi2c_delay_us(3);
	gpb_scl(D, 0);
	hsi2c_delay_us(3);
}

static void gpb_stop(volatile uint32_t *C, volatile uint32_t *D)
{
	gpb_cfg(C, 1, 1);
	gpb_sda(D, 0);
	hsi2c_delay_us(3);
	gpb_scl(D, 1);
	hsi2c_delay_us(3);
	gpb_sda(D, 1);
	hsi2c_delay_us(3);
	gpb_cfg(C, 1, 0);		/* bus released */
	hsi2c_delay_us(3);
}

/* tx one byte; returns 0 on ACK (SDA pulled low), -1 on NACK */
static int gpb_byte(volatile uint32_t *C, volatile uint32_t *D, unsigned int val)
{
	unsigned int i;
	int ack;

	for (i = 0; i < 8; i++) {
		gpb_cfg(C, 1, 1);
		gpb_sda(D, (val >> (7 - i)) & 1);
		hsi2c_delay_us(3);
		gpb_scl(D, 1);
		hsi2c_delay_us(3);
		gpb_scl(D, 0);
		hsi2c_delay_us(3);
	}

	gpb_cfg(C, 1, 0);		/* SDA released for ACK slot */
	hsi2c_delay_us(3);
	gpb_scl(D, 1);
	hsi2c_delay_us(3);
	ack = (*D) & 1;
	gpb_scl(D, 0);
	hsi2c_delay_us(3);
	return ack ? -1 : 0;
}

/* rx one byte; master ACKs (mack=0) or NACKs (mack=1) afterwards */
static unsigned int gpb_rbyte(volatile uint32_t *C, volatile uint32_t *D,
			      int mack)
{
	unsigned int i;
	unsigned int v = 0;

	for (i = 0; i < 8; i++) {
		gpb_cfg(C, 1, 0);
		hsi2c_delay_us(3);
		gpb_scl(D, 1);
		hsi2c_delay_us(3);
		v = (v << 1) | ((*D) & 1);
		gpb_scl(D, 0);
		hsi2c_delay_us(3);
	}

	gpb_cfg(C, 1, 1);
	gpb_sda(D, mack ? 1 : 0);
	hsi2c_delay_us(3);
	gpb_scl(D, 1);
	hsi2c_delay_us(3);
	gpb_scl(D, 0);
	hsi2c_delay_us(3);
	return v;
}

static int gpb_read_reg(unsigned int addr, unsigned int reg, unsigned char *v)
{
	volatile uint32_t *C = (volatile uint32_t *)GPB_CON;
	volatile uint32_t *D = (volatile uint32_t *)GPB_DAT;

	gpb_start(C, D);
	if (gpb_byte(C, D, (addr << 1) | 0)) { gpb_stop(C, D); return -1; }
	if (gpb_byte(C, D, reg))             { gpb_stop(C, D); return -1; }
	gpb_start(C, D);
	if (gpb_byte(C, D, (addr << 1) | 1)) { gpb_stop(C, D); return -1; }
	*v = gpb_rbyte(C, D, 1);
	gpb_stop(C, D);
	return 0;
}

static int gpb_write_reg(unsigned int addr, unsigned int reg, unsigned int val)
{
	volatile uint32_t *C = (volatile uint32_t *)GPB_CON;
	volatile uint32_t *D = (volatile uint32_t *)GPB_DAT;

	gpb_start(C, D);
	if (gpb_byte(C, D, (addr << 1) | 0)) { gpb_stop(C, D); return -1; }
	if (gpb_byte(C, D, reg))             { gpb_stop(C, D); return -1; }
	if (gpb_byte(C, D, val))             { gpb_stop(C, D); return -1; }
	gpb_stop(C, D);
	return 0;
}

/* read-modify-write: (v | set) & ~clr */
static int gpb_update_reg(unsigned int addr, unsigned int reg,
			  unsigned int set, unsigned int clr)
{
	unsigned char v;
	int r;

	r = gpb_read_reg(addr, reg, &v);
	if (r)
		return r;
	v = (v & ~clr) | set;
	return gpb_write_reg(addr, reg, v);
}

/* sends addr<<1|0, returns 1 if ACKed, 0 if NACKed */
static int gpb_probe_addr(volatile uint32_t *C, volatile uint32_t *D,
			  unsigned int addr)
{
	int ack;

	gpb_start(C, D);
	ack = gpb_byte(C, D, addr << 1) == 0;
	gpb_stop(C, D);
	return ack;
}

static void hsi2c_gpb_probe(void)
{
	static const unsigned int adrs[] = { 0x20, 0x25, 0x3b, 0x3d, 0x4a, 0x66 };
	volatile uint32_t *C = (volatile uint32_t *)GPB_CON;
	volatile uint32_t *D = (volatile uint32_t *)GPB_DAT;
	volatile uint32_t *P = (volatile uint32_t *)GPB_PUD;
	unsigned int i;

	gpb_bus_release(C, P);
	printk(KERN_INFO, "usb:gbp dat=%08x start\n", *D);

	for (i = 0; i < ARRAY_SIZE(adrs); i++) {
		int hit = gpb_probe_addr(C, D, adrs[i]);
		printk(KERN_INFO, "usb:gbp %02x %s\n", adrs[i],
		       hit ? "ACK" : "no_ack");
	}
	printk(KERN_INFO, "usb:gbp dat=%08x done\n", *D);
}

/* ------- end bit-bang probe -------- */

/*
 * Enable S2MU005 5V-boost + D+/D- to AP for USB host.
 * Transport is GPIO bit-bang over gpc1-0/1: the HSI2C IP block
 * refuses to ACK the chip at this boot stage (SCL timing), but the
 * slow bit-banged transfer provably reaches it.  REV_ID 0x73 is
 * verified before any write.
 */
static void s2mu005_pin_release(void)
{
	volatile uint32_t *C = (volatile uint32_t *)GPB_CON;
	volatile uint32_t *P = (volatile uint32_t *)GPB_PUD;

	/* back to fn2 with pull-up, as S-Boot/kernel expect */
	*C = (*C & ~0xFFu) | 0x20u | 0x02u;
	*P = (*P & ~0x0Fu) | 0x0Fu;
	hsi2c_dmb();
}

int muic_sm5703_otg_enable(void)
{
	uint8_t id = 0;
	int r;

	printk(KERN_INFO, "usb:muic s2mu005 bitbang\n");
	hsi2c0_dump_raw();

	r = gpb_read_reg(S2MU005_ADDR, S2MU005_REV_ID, &id);
	if (r < 0) {
		printk(KERN_ERR, "usb:muic gbp rev fail (%d)\n", r);
		hsi2c_gpb_probe();
		return -2;
	}
	printk(KERN_INFO, "usb:muic rev_id=%02x\n", id);

	r = gpb_update_reg(S2MU005_ADDR, S2MU005_CHG_CTRL0, 0x04, 0x07);
	if (r < 0) goto wfail;
	r = gpb_update_reg(S2MU005_ADDR, S2MU005_CHG_CTRL11, 0x40, 0x60);
	if (r < 0) goto wfail;
	r = gpb_update_reg(S2MU005_ADDR, S2MU005_CHG_CTRL4, 0x1C, 0x1C);
	if (r < 0) goto wfail;
	r = gpb_update_reg(S2MU005_ADDR, S2MU005_CHG_CTRL5, 0x16, 0x1F);
	if (r < 0) goto wfail;

	/* manual sw mode: CTRL1 bit2 set, CTRL1 bit3 clear (raw data) */
	r = gpb_update_reg(S2MU005_ADDR, S2MU005_MUIC_CTRL1, 0x04, 0x08);
	if (r < 0) goto wfail;
	r = gpb_write_reg(S2MU005_ADDR, S2MU005_MUIC_SW_CTRL,
			  MANUAL_SW_USB | MANUAL_SW_OTGEN);
	if (r < 0) goto wfail;

	hsi2c_delay_us(300);	/* let boost ramp */

	/* readback to prove the writes actually landed */
	{
		unsigned char c0 = 0, c5 = 0, sw = 0, c1 = 0;

		gpb_read_reg(S2MU005_ADDR, S2MU005_CHG_CTRL0, &c0);
		gpb_read_reg(S2MU005_ADDR, S2MU005_CHG_CTRL5, &c5);
		gpb_read_reg(S2MU005_ADDR, S2MU005_MUIC_SW_CTRL, &sw);
		gpb_read_reg(S2MU005_ADDR, S2MU005_MUIC_CTRL1, &c1);
		printk(KERN_INFO,
		       "usb:muic rd chg0=%02x chg5=%02x sw=%02x ctrl1=%02x\n",
		       c0, c5, sw, c1);
	}
	s2mu005_pin_release();
	printk(KERN_INFO, "usb:muic otg boost on\n");
	return 0;

wfail:
	printk(KERN_ERR, "usb:muic write fail (%d)\n", r);
	return -3;
}