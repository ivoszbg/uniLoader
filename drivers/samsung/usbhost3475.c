// SPDX-License-Identifier: GPL-2.0-only
/*
 * Bare-metal USB 2.0 host driver for Exynos 3475 (EHCI @0x13510000).
 * Copyright (c) 2026, uniLoader j1xlte bring-up.
 *
 * Reference: kernel-j120f (Samsung/Lineage 3.10) - the CAL PHY init from
 * phy-samsung-usb2-cal.c, register map from phy-samsung-dwc-usb2.h, QH/qTD
 * layouts from drivers/usb/host/ehci.h + ehci-q.c.  Do NOT use the
 * mainline-j1 6.x port as reference (half-working).
 *
 * Stage A scope: clocks -> PMU de-isolate -> sysreg HOST -> PHY host init ->
 * EHCI enumeration -> MASS STORAGE (BOT) -> INQUIRY/READ CAPACITY -> sector 0
 * MBR dump.  FAT32 readdir is Stage B.
 *
 * Every step prints a marker so a failure on real hw is pinpointable.
 */

#include <stdint.h>
#include <string.h>
#include <lib/debug.h>
#include <soc/exynos3475-usb.h>

/* exynos-hsi2c.c: SM5703 MUIC OTG 5V-boost over hsi2c_0 */
int muic_sm5703_otg_enable(void);

#define EHCI		(volatile uint32_t *)EXYNOS3475_EHCI_BASE
#define PHY		(volatile uint32_t *)EXYNOS3475_USB2PHY_BASE
#define PMU		(volatile uint32_t *)EXYNOS3475_USBPHY_PMU_BASE
#define SYSREG		(volatile uint32_t *)EXYNOS3475_USB20_SYSREG_BASE
#define CMU_FSYS	(volatile uint32_t *)EXYNOS3475_CMU_FSYS_BASE
#define CMU_TOP		(volatile uint32_t *)EXYNOS3475_CMU_TOP_BASE

/* generic 32-bit reg access; uniloader runs with MMU off, memory is
 * coherent (no caches), so a DMB + volatile access is enough */
static inline uint32_t rd32(volatile uint32_t *p)
{
	return *p;
}

static inline void wr32(volatile uint32_t *p, uint32_t v)
{
	*p = v;
}

static inline void dmb(void)
{
	__asm__ __volatile__("dmb" ::: "memory");
}

static void usb_busy_us(unsigned int us)
{
	volatile unsigned int t = us * 400;	/* ~1.2x overshoot @ ~1GHz */

	while (t--)
		;
}

/* ------------------------------------------------------------------ */
/* CMU / PMU / sysreg / PHY                                           */
/* ------------------------------------------------------------------ */

static void usb_clk_host_enable(void)
{
	uint32_t v;

	/* aclk_fsys_200: the EHCI-facing AHB/AXI cells (clk-exynos3475.c)
	 * NOTE: rd32() on ENABLE_ACLK_FSYS_200 (0x800) hangs the bus when
	 * the FSYS domain is gated, so use flat writes of known bits. */
	v = ACLK_FSYS200_USBHOST20 | ACLK_FSYS200_USBOTG20 |
	     ACLK_FSYS200_AHB_USBHS | ACLK_FSYS200_AHB2AXI_USBHS |
	     ACLK_FSYS200_UPSIZER_AHB_USBHS | ACLK_FSYS200_XIU_D_FSYS1 |
	     ACLK_FSYS200_UPSIZER_FSYS1;
	printk(KERN_INFO, "usb:c1\n");
	wr32(CMU_FSYS + (EXYNOS3475_CLK_ENABLE_ACLK_FSYS_200 -
			 EXYNOS3475_CMU_FSYS_BASE), v);
	printk(KERN_INFO, "usb:c2\n");

	/* PHY clocks: usb_pll to PHY core + PHY-generated 60/48MHz for host */
	wr32(CMU_FSYS + (EXYNOS3475_CLK_ENABLE_SCLK_USB20_PHY_CLKCORE -
			 EXYNOS3475_CMU_FSYS_BASE), 1);
	printk(KERN_INFO, "usb:c3\n");
	wr32(CMU_FSYS + (EXYNOS3475_CLK_ENABLE_PHYCLK_USBHOST20_USB20_FREECLK -
			 EXYNOS3475_CMU_FSYS_BASE), 1);
	printk(KERN_INFO, "usb:c4\n");
	wr32(CMU_FSYS + (EXYNOS3475_CLK_ENABLE_PHYCLK_USBHOST20_USB20_PHYCLOCK -
			 EXYNOS3475_CMU_FSYS_BASE), 1);
	printk(KERN_INFO, "usb:c5\n");
	wr32(CMU_FSYS + (EXYNOS3475_CLK_ENABLE_PHYCLK_USBHOST20_USB20_CLK48MOHCI -
			 EXYNOS3475_CMU_FSYS_BASE), 1);
	printk(KERN_INFO, "usb:c6\n");
	dmb();

	/* usb_pll (pll_2551x) feeds sclk_usb20_phy_clkcore = PHY register/clk
	 * core bus. S-Boot may have left it switched off after its USB
	 * download finished -> PHY block then appears dead (regs read 0).
	 * Ensure it is running at 24MHz: 26MHz fin, MDIV=384, PDIV=13, SDIV=5.
	 * CON0: [31]=ENABLE [29]=LOCKED [23:12]=MDIV [9:4]=PDIV [2:0]=SDIV */
	{
		volatile uint32_t *fsys =
			(volatile uint32_t *)EXYNOS3475_CMU_FSYS_BASE;
		uint32_t con0 = fsys[EXYNOS3475_USB_PLL_CON0 / 4];
		uint32_t i;

		printk(KERN_INFO, "usb:pll before con0=%08x lock=%08x\n", con0,
		       fsys[EXYNOS3475_USB_PLL_LOCK / 4]);
		if (!(con0 & (1U << 31))) {
			uint32_t pms = (384U << 12) | (13U << 4) | (5U << 0);

			fsys[EXYNOS3475_USB_PLL_LOCK / 4] = 13U * 200U;
			fsys[EXYNOS3475_USB_PLL_CON0 / 4] = pms;
			fsys[EXYNOS3475_USB_PLL_CON0 / 4] = pms | (1U << 31);
			for (i = 0;
			     i < 200000 &&
			     !(fsys[EXYNOS3475_USB_PLL_CON0 / 4] & (1U << 29));
			     i++)
				;
		}
		printk(KERN_INFO, "usb:pll after  con0=%08x lock=%08x\n",
		       fsys[EXYNOS3475_USB_PLL_CON0 / 4],
		       fsys[EXYNOS3475_USB_PLL_LOCK / 4]);
	}
	dmb();
	usb_busy_us(100);
}

static void usb_deisolate_host(void)
{
	wr32(PMU, rd32(PMU) | EXYNOS3475_USBPHY_ENABLE);
	dmb();
	usb_busy_us(100);
}

static void usb_sysreg_host(void)
{
	wr32(SYSREG, rd32(SYSREG) | EXYNOS3475_USB20PHY_CFG_HOST_LINK);
	dmb();
	usb_busy_us(100);
}

/*
 * samsung_exynos_cal_dwc_usb2phy_enable() + reset() + tune(),
 * ported 1:1 from phy-samsung-usb2-cal.c (TYPE_EXYNOS7580 / HOST).
 */
static void usb_phy_enable(void)
{
	uint32_t ctrl, resume, utmi, clkrst, plltune, param;

	printk(KERN_INFO, "usb:phy pmu_raw=%08x sysreg_raw=%08x\n",
	       rd32((volatile uint32_t *)EXYNOS3475_USBPHY_PMU_BASE),
	       rd32((volatile uint32_t *)EXYNOS3475_USB20_SYSREG_BASE));

	ctrl = rd32(PHY + EXYNOS3475_PHY_STDPHYCTRL);
	ctrl &= ~STDPHYCTRL_SIDDQ;
	wr32(PHY + EXYNOS3475_PHY_STDPHYCTRL, ctrl);
	usb_busy_us(500);

	resume = rd32(PHY + EXYNOS3475_RESUME);
	resume &= ~(USB_BYPASSSEL | USB_BYPASSDPEN | USB_BYPASSDMEN);
	wr32(PHY + EXYNOS3475_RESUME, resume);

	utmi = rd32(PHY + EXYNOS3475_PHYUTMI);
	utmi &= ~(PHYUTMI_FORCESLEEP | PHYUTMI_FORCESUSPEND |
		  PHYUTMI_DMPULLDOWN | PHYUTMI_DPPULLDOWN | PHYUTMI_OTGDISABLE);
	utmi |= PHYUTMI_DRVVBUS;
	wr32(PHY + EXYNOS3475_PHYUTMI, utmi);

	clkrst = rd32(PHY + EXYNOS3475_PHYCLKRST);
	clkrst &= ~PHYCLKRST_COMMONONN;
	/* 24MHz ref: FSEL=0x2, same values CAL forces (phy-samsung-usb2-cal.c).
	 * Previously we trusted whatever S-Boot left in the field. */
	clkrst &= ~PHYCLKRST_FSEL_MASK;
	clkrst |= PHYCLKRST_FSEL(0x2);
	wr32(PHY + EXYNOS3475_PHYCLKRST, clkrst);

	plltune = rd32(PHY + EXYNOS3475_PHY_HOSTPLLTUNE);
	plltune |= HOSTPLLTUNE_PLLBTUNE;	/* 24MHz case */
	wr32(PHY + EXYNOS3475_PHY_HOSTPLLTUNE, plltune);

	param = rd32(PHY + EXYNOS3475_PHYPARAM0);
	param &= ~(PHYPARAM0_TXVREFTUNE_MASK | PHYPARAM0_TXPREEMPAMPTUNE_MASK |
		   PHYPARAM0_COMPDISTUNE_MASK);
	param |= PHYPARAM0_TXVREFTUNE(0x5) | PHYPARAM0_TXPREEMPAMPTUNE(0x2) |
		 PHYPARAM0_COMPDISTUNE(0x7);
	wr32(PHY + EXYNOS3475_PHYPARAM0, param);
	dmb();

	/* diagnostic: PHY state after init */
	printk(KERN_INFO, "usb:phy init clkrst=%08x resume=%08x utmi=%08x "
	       "stdctrl=%08x r0=%08x r4=%08x pmu=%08x sysreg=%08x\n",
	       rd32(PHY + EXYNOS3475_PHYCLKRST),
	       rd32(PHY + EXYNOS3475_RESUME),
	       rd32(PHY + EXYNOS3475_PHYUTMI),
	       rd32(PHY + EXYNOS3475_PHY_STDPHYCTRL),
	       rd32(PHY + 0x00 / 4), rd32(PHY + 0x04 / 4),
	       rd32((volatile uint32_t *)EXYNOS3475_USBPHY_PMU_BASE),
	       rd32((volatile uint32_t *)EXYNOS3475_USB20_SYSREG_BASE));
	printk(KERN_INFO, "usb:diag r8=%08x rc=%08x\n",
	       rd32(PHY + 0x08 / 4), rd32(PHY + 0x0c / 4));
	/* write-latch test: reserved word at PHY+0x00 */
	{
		uint32_t back, pllt, lnk;

		wr32(PHY + 0x00 / 4, 0xdeadbeef);
		back = rd32(PHY + 0x00 / 4);
		wr32(PHY + 0x00 / 4, 0);
		pllt = rd32(PHY + EXYNOS3475_PHY_HOSTPLLTUNE);
		lnk = rd32(PHY + EXYNOS3475_LINK_HOSTLINKCTL);
		printk(KERN_INFO, "usb:wl r0=%08x plltn=%08x link=%08x\n",
		       back, pllt, lnk);
	}
}

static void usb_phy_reset_host(void)
{
	uint32_t ctrl, link;

	ctrl = rd32(PHY + EXYNOS3475_PHY_STDPHYCTRL);
	ctrl |= STDPHYCTRL_SWRST_PHY | STDPHYCTRL_SWRST_ALL;
	wr32(PHY + EXYNOS3475_PHY_STDPHYCTRL, ctrl);

	link = rd32(PHY + EXYNOS3475_LINK_HOSTLINKCTL);
	link |= HOSTLINKCTL_LINKSWRST | HOSTLINKCTL_SW_RESET_PORT0 |
		HOSTLINKCTL_SW_RESET_PORT1 | HOSTLINKCTL_SW_RESET_PORT2;
	wr32(PHY + EXYNOS3475_LINK_HOSTLINKCTL, link);

	usb_busy_us(10);

	ctrl &= ~(STDPHYCTRL_SWRST_ALL | STDPHYCTRL_SWRST_PHY);
	wr32(PHY + EXYNOS3475_PHY_STDPHYCTRL, ctrl);

	link &= ~(HOSTLINKCTL_LINKSWRST | HOSTLINKCTL_SW_RESET_PORT0 |
		  HOSTLINKCTL_SW_RESET_PORT1 | HOSTLINKCTL_SW_RESET_PORT2);
	wr32(PHY + EXYNOS3475_LINK_HOSTLINKCTL, link);

	usb_busy_us(50);
	dmb();
}

/* ------------------------------------------------------------------ */
/* EHCI descriptors (layouts = drivers/usb/host/ehci.h / ehci-q.c)    */
/* ------------------------------------------------------------------ */

#define Q_TYPE_QH			(1 << 1)
#define QH_LINK_TERMINATE		0x1

#define QH_HEAD				(1 << 15)
#define QH_TOGGLE_CTL			(1 << 14)
#define QH_HIGH_SPEED			(2 << 12)
#define QH_RL_HS			(4 << 28)
#define QH_MULT_HS			(1 << 30)

#define QTD_ACTIVE			(1 << 7)	/* HC may execute */
#define QTD_HALT			(1 << 6)
#define QTD_DBE				(1 << 5)
#define QTD_BABBLE			(1 << 4)
#define QTD_XACT			(1 << 3)
#define QTD_MMF				(1 << 2)
#define QTD_TOGGLE			(1 << 31)
#define QTD_IOC				(1 << 15)
#define QTD_LEN(n)			(((n) & 0x7fff) << 16)
#define QTD_PID_SETUP			(2 << 8)
#define QTD_PID_IN			(1 << 8)
#define QTD_PID_OUT			(0 << 8)
#define QTD_CERR			(3 << 10)
#define QTD_NEXT_TERMINATE		0x1

struct ehci_qtd {
	uint32_t next;
	uint32_t alt_next;
	uint32_t token;
	uint32_t buf[5];
} __attribute__((aligned(32)));

struct ehci_qh {
	uint32_t next;
	uint32_t info1;
	uint32_t info2;
	uint32_t current;
	uint32_t overlay_next;	/* qtd overlay lives inline */
	uint32_t overlay_alt;
	uint32_t overlay_token;
	uint32_t overlay_buf[5];
} __attribute__((aligned(32)));

/* ---------------- globals (DMA, MMU off => physical == virtual) ---- */

static struct ehci_qh asynq_head __attribute__((aligned(32)));
static struct ehci_qh xfer_qh  __attribute__((aligned(32)));
static struct ehci_qtd qtd_pool[4] __attribute__((aligned(32)));

/* transfer / sector buffers - keep 4K aligned for qTD page ptrs */
static uint8_t setup_pkt[8]   __attribute__((aligned(4096)));
static uint8_t data_buf[512]  __attribute__((aligned(4096)));
static uint8_t mbr_sector[512] __attribute__((aligned(4096)));

enum { EP0_MPS_INITIAL = 8 };

/* what we learned about the device/bulk endpoint */
static unsigned int dev_addr;
static unsigned int dev_mps0;
static unsigned int ms_iface, ms_cfg;
static unsigned int bulk_in_ep, bulk_out_ep, bulk_mps;
static unsigned int bulk_toggle_out;	/* sw-managed DT for bulk */
static unsigned int bulk_toggle_in;

static volatile uint32_t *op_regs;

/* ------------------------------------------------------------------ */
/* EHCI low level                                                     */
/* ------------------------------------------------------------------ */

static uint32_t ehci_op(unsigned int off)
{
	return *(op_regs + (off >> 2));
}

static void ehci_opw(unsigned int off, uint32_t v)
{
	*(op_regs + (off >> 2)) = v;
}

static void ehci_hc_reset(void)
{
	unsigned int t = 100000;

	printk(KERN_INFO, "usb:hw\n");
	ehci_opw(EXYNOS3475_EHCI_USBCMD, CMD_HCRESET);
	printk(KERN_INFO, "usb:hx\n");
	while (ehci_op(EXYNOS3475_EHCI_USBCMD) & CMD_HCRESET) {
		if (!--t) {
			printk(KERN_ERR, "usb: EHCI HCRESET timeout\n");
			return;
		}
		usb_busy_us(1);
	}
	printk(KERN_INFO, "usb:hy\n");
	usb_busy_us(1000);
}

static void ehci_run(void)
{
	unsigned int t = 100000;

	printk(KERN_INFO, "usb:rw1\n");
	ehci_opw(EXYNOS3475_EHCI_USBINTR, 0);	/* polling, no IRQs */
	printk(KERN_INFO, "usb:rw2\n");
	ehci_opw(EXYNOS3475_EHCI_USBCMD, CMD_RUN);
	printk(KERN_INFO, "usb:rw3\n");
	while (ehci_op(EXYNOS3475_EHCI_USBSTS) & STS_HCHALTED) {
		if (!--t) {
			printk(KERN_ERR, "usb: EHCI run timeout\n");
			return;
		}
		usb_busy_us(1);
	}
	printk(KERN_INFO, "usb:rw4\n");
}

static void ehci_async_enable(void)
{
	unsigned int t = 100000;

	ehci_opw(EXYNOS3475_EHCI_ASYNCLISTADDR, (uint32_t)&asynq_head);
	ehci_opw(EXYNOS3475_EHCI_USBCMD, ehci_op(EXYNOS3475_EHCI_USBCMD) |
					  CMD_ASSE);
	while (!(ehci_op(EXYNOS3475_EHCI_USBSTS) & STS_ASSS)) {
		if (!--t) {
			printk(KERN_ERR, "usb: async enable timeout\n");
			return;
		}
		usb_busy_us(1);
	}
}

static void ehci_port_reset(unsigned int port)
{
	volatile uint32_t *psc = op_regs + ((EXYNOS3475_EHCI_PORTSC +
					     (port << EXYNOS3475_EHCI_PORTSHIFT)) >> 2);
	unsigned int t;

	printk(KERN_INFO, "usb:pp1\n");
	/* power the port */
	*psc |= PORTSC_PP;
	printk(KERN_INFO, "usb:pp2 portsc=%08x\n", *psc);
	usb_busy_us(200);

	/* Wait for a connect for as long as it takes: the J120F port has no
	 * 5V VBUS unless a Y-cable/MUIC boost feeds it, so the stick may be
	 * plugged in at any moment.  Print a dot ~every 0.4 s so the loop
	 * never looks dead. */
	t = 0;
	printk(KERN_INFO, "usb: waiting dev port%u (plug powered HS stick)...\n",
	       port);
	while (!(*psc & PORTSC_CCS)) {
		usb_busy_us(200);
		if ((++t & 0x3ff) == 0) {
			uint32_t ls = (*psc >> 12) & 3;
			printk(KERN_INFO, "usb:wait[%u] portsc=%08x LS=%s\n",
			       t >> 10, *psc,
			       ls == 0 ? "SE0" : ls == 1 ? "K" :
			       ls == 2 ? "J" : "undef");
		}
	}
	printk(KERN_INFO, "usb: port%u connected (portsc=%08x)\n",
	       port, *psc);

	/* reset */
	*psc |= PORTSC_PR;
	printk(KERN_INFO, "usb: PR set portsc=%08x\n", *psc);
	t = 50000;	/* ~5s window */
	while (*psc & PORTSC_PR) {
		if (!--t) {
			printk(KERN_ERR, "usb: port%u reset timeout\n", port);
			break;
		}
		if ((t & 0x2710) == 0)
			printk(KERN_INFO,
			       "usb: PR wait portsc=%08x clkrst=%08x utmi=%08x "
			       "toctrl=%08x r0=%08x pmu=%08x usbcmd=%08x\n",
			       *psc, rd32(PHY + EXYNOS3475_PHYCLKRST),
			       rd32(PHY + EXYNOS3475_PHYUTMI),
			       rd32(PHY + EXYNOS3475_PHY_STDPHYCTRL),
			       rd32(PHY + 0x00 / 4),
			       rd32((volatile uint32_t *)
				    EXYNOS3475_USBPHY_PMU_BASE),
			       psc[0 - (EXYNOS3475_EHCI_PORTSC >> 2)]);
		usb_busy_us(100);
	}
	/* post-reset settling */
	usb_busy_us(20000);
	printk(KERN_INFO, "usb: port%u %s,%s\n", port,
	       ((*psc & PORTSC_SPEED) == PORTSC_SPD_HS) ?
		"high-speed" : "full/low-speed",
	       (*psc & PORTSC_PE) ? "enabled" : "NOT enabled");
}

/* ------------------------------------------------------------------ */
/* Transfer engine (u-boot-style: one endpoint QH linked off the head) */
/* ------------------------------------------------------------------ */

struct xfer_stage {
	unsigned int pid;	/* QTD_PID_* */
	unsigned int len;
	unsigned int tog;	/* data toggle bit for this stage */
	const void *src;	/* DMA buffer (must be 4K aligned) */
};

static void qtd_init(struct ehci_qtd *q, unsigned int pid_shift,
		     unsigned int len, const void *buf, unsigned int tog)
{
	memset(q, 0, sizeof(*q));
	q->next = QTD_NEXT_TERMINATE;
	q->alt_next = QTD_NEXT_TERMINATE;
	q->token = QTD_ACTIVE | QTD_CERR | QTD_IOC | pid_shift |
		   QTD_LEN(len) | ((tog & 1) ? QTD_TOGGLE : 0);
	if (buf && len)
		q->buf[0] = (uint32_t)buf;	/* must be 4K aligned */
	dmb();
}

/*
 * Submit a 1..3-stage control transfer on EP0 of `addr` (mps `mps0`)
 * and wait.  Uses setup_pkt + data_buf/qtd_pool.  `data` may point at
 * setup-independent pages; IN results land in `data`.
 */
static int usb_control(unsigned int addr, unsigned int mps0,
		       uint8_t bmReqType, uint8_t bReq, uint16_t wVal,
		       uint16_t wIdx, uint16_t wLen, void *data)
{
	struct xfer_stage st[3];
	unsigned int n = 0, i, t;
	unsigned int pid_data, pid_status;
	int ret = -1;

	/* build the 8-byte setup packet */
	memset(setup_pkt, 0, sizeof(setup_pkt));
	setup_pkt[0] = bmReqType;
	setup_pkt[1] = bReq;
	setup_pkt[2] = wVal & 0xff;
	setup_pkt[3] = wVal >> 8;
	setup_pkt[4] = wIdx & 0xff;
	setup_pkt[5] = wIdx >> 8;
	setup_pkt[6] = wLen & 0xff;
	setup_pkt[7] = wLen >> 8;

	/* SETUP: toggle 0 */
	st[n].pid = QTD_PID_SETUP;
	st[n].len = 8;
	st[n].tog = 0;
	st[n].src = setup_pkt;
	n++;

	/* DATA (optional): toggle 1 */
	pid_data = (bmReqType & USB_DIR_IN) ? QTD_PID_IN : QTD_PID_OUT;
	pid_status = (bmReqType & USB_DIR_IN) ? QTD_PID_OUT : QTD_PID_IN;
	if (wLen) {
		st[n].pid = pid_data;
		st[n].len = wLen;
		st[n].tog = 1;
		st[n].src = data;
		n++;
	}

	/* STATUS: PID is the opposite data phase; toggle continues
	 * the parity (data==present ? 0 : 1) */
	st[n].pid = pid_status;
	st[n].len = 0;
	st[n].tog = wLen ? 0 : 1;
	st[n].src = NULL;
	n++;

	/* prepare qtd chain */
	for (i = 0; i < n; i++) {
		qtd_init(&qtd_pool[i], st[i].pid, st[i].len, st[i].src,
			 st[i].tog);
		if (i > 0)
			qtd_pool[i - 1].next = (uint32_t)&qtd_pool[i];
	}
	qtd_pool[n - 1].next = QTD_NEXT_TERMINATE;

	/* endpoint QH: EP0, address, high speed, control, toggle from qtd */
	memset(&xfer_qh, 0, sizeof(xfer_qh));
	xfer_qh.info1 = QH_RL_HS | (mps0 << 16) | QH_TOGGLE_CTL |
			QH_HIGH_SPEED | (0 << 8) | addr;
	xfer_qh.info2 = QH_MULT_HS;
	xfer_qh.overlay_next = (uint32_t)&qtd_pool[0];
	xfer_qh.current = (uint32_t)&qtd_pool[0];
	dmb();

	/* link ring: head -> xfer -> head */
	asynq_head.next = (uint32_t)&xfer_qh | Q_TYPE_QH;
	xfer_qh.next = (uint32_t)&asynq_head | Q_TYPE_QH;
	dmb();

	/* enable async (idempotent) */
	ehci_async_enable();

	/* wait for the LAST stage's qtd to leave Active */
	t = 4000000;
	while (qtd_pool[n - 1].token & QTD_ACTIVE) {
		if (qtd_pool[0].token & QTD_HALT)
			break;
		if (!--t) {
			printk(KERN_ERR, "usb: control timeout\n");
			goto out;
		}
		usb_busy_us(10);
	}

	/* error scan over all stages */
	for (i = 0; i < n; i++) {
		uint32_t tok = qtd_pool[i].token;

		if (tok & (QTD_HALT | QTD_DBE | QTD_BABBLE | QTD_XACT |
			   QTD_MMF)) {
			printk(KERN_ERR,
			       "usb: ctrl stg%u err tok=%08x\n", i, tok);
			goto out;
		}
	}
	/* IN payload already went straight into `data` (the data-stage qtd
	 * DMA'd into it directly) */
	ret = 0;
out:
	/* take the transfer QH back out of the schedule */
	asynq_head.next = QH_LINK_TERMINATE;
	dmb();
	return ret;
}

static int usb_bulk_io(unsigned int addr, unsigned int ep, int dir_in,
		       unsigned int len, void *buf)
{
	unsigned int pid = dir_in ? QTD_PID_IN : QTD_PID_OUT;
	unsigned int *tog = dir_in ? &bulk_toggle_in : &bulk_toggle_out;
	unsigned int t;
	int ret = -1;

	if (len > 512) {
		printk(KERN_ERR, "usb: bulk too big (%u)\n", len);
		return ret;
	}

	memset(&xfer_qh, 0, sizeof(xfer_qh));
	xfer_qh.info1 = QH_RL_HS | (bulk_mps << 16) | QH_TOGGLE_CTL |
			QH_HIGH_SPEED | (ep << 8) | addr;
	xfer_qh.info2 = QH_MULT_HS;

	/* DMA into the caller buffer for IN?  We route via data_buf below. */
	{
		unsigned char *dst;

		if (dir_in) {
			dst = data_buf;
			memset(data_buf, 0, 512);
		} else {
			dst = buf;
		}

		memset(&qtd_pool[0], 0, sizeof(qtd_pool[0]));
		qtd_pool[0].next = QTD_NEXT_TERMINATE;
		qtd_pool[0].alt_next = QTD_NEXT_TERMINATE;
		qtd_pool[0].token = QTD_ACTIVE | QTD_CERR | QTD_IOC | pid |
				    QTD_LEN(len) | ((*tog & 1) ? QTD_TOGGLE : 0);
		qtd_pool[0].buf[0] = (uint32_t)dst;
		xfer_qh.overlay_next = (uint32_t)&qtd_pool[0];
		xfer_qh.current = (uint32_t)&qtd_pool[0];
		dmb();

		asynq_head.next = (uint32_t)&xfer_qh | Q_TYPE_QH;
		xfer_qh.next = (uint32_t)&asynq_head | Q_TYPE_QH;
		dmb();

		ehci_async_enable();

		t = 4000000;
		while (qtd_pool[0].token & QTD_ACTIVE) {
			if (!--t) {
				printk(KERN_ERR, "usb: bulk timeout tok=%08x\n",
				       qtd_pool[0].token);
				asynq_head.next = QH_LINK_TERMINATE;
				dmb();
				return ret;
			}
			usb_busy_us(10);
		}

		if (qtd_pool[0].token &
		    (QTD_HALT | QTD_DBE | QTD_BABBLE | QTD_XACT | QTD_MMF)) {
			printk(KERN_ERR, "usb: bulk err tok=%08x\n",
			       qtd_pool[0].token);
			asynq_head.next = QH_LINK_TERMINATE;
			dmb();
			return ret;
		}

		if (dir_in) {
			unsigned int rem = (qtd_pool[0].token >> 16) & 0x7fff;

			memcpy(buf, dst, len - rem);
		}

		/* advance sw toggle: toggles per transferred packet */
		if ((len % bulk_mps) == 0)
			*tog ^= ((len / bulk_mps) & 1);
		else
			*tog ^= 1;
		ret = 0;
	}

	asynq_head.next = QH_LINK_TERMINATE;
	dmb();
	return ret;
}

/* ------------------------------------------------------------------ */
/* Enumeration + MSC                                                  */
/* ------------------------------------------------------------------ */

static int usb_get_device_descriptor(unsigned int addr, unsigned int mps,
				     uint8_t *dd)
{
	return usb_control(addr, mps, USB_DIR_IN, USB_REQ_GET_DESCRIPTOR,
			   (USB_DEVICE_DESCRIPTOR_TYPE << 8) | 0, 0, 8, dd);
}

static int usb_set_address(unsigned int addr)
{
	return usb_control(0, EP0_MPS_INITIAL, 0, USB_REQ_SET_ADDRESS,
			   addr, 0, 0, NULL);
}

static int usb_get_config_descriptor(uint16_t wLen, void *buf)
{
	return usb_control(dev_addr, dev_mps0, USB_DIR_IN,
			   USB_REQ_GET_DESCRIPTOR,
			   USB_CONFIG_DESCRIPTOR_TYPE << 8, 0, wLen, buf);
}

static int usb_set_configuration(unsigned int cfg)
{
	return usb_control(dev_addr, dev_mps0, 0, USB_REQ_SET_CONFIGURATION,
			   cfg, 0, 0, NULL);
}

static int usb_msc_bulk_reset(unsigned int iface)
{
	return usb_control(dev_addr, dev_mps0,
			   USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			   0xff, 0, iface, 0, NULL);
}

struct cbw {
	uint32_t sig;
	uint32_t tag;
	uint32_t dtlen;
	uint8_t flags;
	uint8_t lun;
	uint8_t cb_len;
	uint8_t cb[16];
	uint8_t rsvd[10];
} __attribute__((packed));

struct csw {
	uint32_t sig;
	uint32_t tag;
	uint32_t residue;
	uint8_t status;
} __attribute__((packed));

static uint8_t csw_stag;

static struct cbw cbw __attribute__((aligned(4096)));
static struct csw csw __attribute__((aligned(4096)));

static int msc_transport(uint8_t *cb, unsigned int cb_len,
			 unsigned int dir_in, void *data, unsigned int dlen)
{
	/* one CBW + optional data phase + CSW */
	if (usb_bulk_io(dev_addr, bulk_out_ep, 0, 31, &cbw))
		return -1;
	if (dlen) {
		if (usb_bulk_io(dev_addr,
				dir_in ? bulk_in_ep : bulk_out_ep,
				dir_in, dlen, data))
			return -1;
	}
	if (usb_bulk_io(dev_addr, bulk_in_ep, 1, 13, &csw))
		return -1;
	if (csw.sig != 0x53555342UL || csw.status != 0) {
		printk(KERN_WARNING, "msc: csw sig=%08x st=%u\n",
		       csw.sig, csw.status);
		return -1;
	}
	return 0;
}

static void msc_build_cbw(uint8_t *cb, unsigned int cb_len)
{
	csw_stag++;
	memset(&cbw, 0, sizeof(cbw));
	cbw.sig = 0x43425355UL;
	cbw.tag = csw_stag;
	cbw.lun = 0;
	cbw.cb_len = cb_len;
	memcpy(cbw.cb, cb, cb_len);
}

static int msc_inquiry(void)
{
	uint8_t cb[12];
	uint8_t inq[36] __attribute__((aligned(4096)));

	memset(cb, 0, sizeof(cb));
	cb[0] = 0x12;	/* INQUIRY */
	cb[4] = 36;	/* alloc len */
	msc_build_cbw(cb, 12);
	if (msc_transport(cb, 12, 1, inq, 36))
		return -1;
	inq[35] = 0;
	printk(KERN_INFO, "msc: %.*s %.*s rev %c%c%c%c\n",
	       8, inq + 8, 16, inq + 16, inq[32], inq[33], inq[34], 0);
	return 0;
}

static int msc_read_capacity(unsigned long *lba, unsigned long *blk)
{
	uint8_t cb[10];
	uint8_t rc[8] __attribute__((aligned(4096)));

	memset(cb, 0, sizeof(cb));
	cb[0] = 0x25;	/* READ CAPACITY(10) */
	msc_build_cbw(cb, 10);
	if (msc_transport(cb, 10, 1, rc, 8))
		return -1;
	*lba = ((unsigned long)rc[0] << 24) | ((unsigned long)rc[1] << 16) |
	       ((unsigned long)rc[2] << 8) | rc[3];
	*blk = ((unsigned long)rc[4] << 24) | ((unsigned long)rc[5] << 16) |
	       ((unsigned long)rc[6] << 8) | rc[7];
	return 0;
}

static int msc_read_sector(unsigned long lba, uint8_t *data)
{
	uint8_t cb[10];

	memset(cb, 0, sizeof(cb));
	cb[0] = 0x28;	/* READ(10) */
	cb[2] = lba >> 24;
	cb[3] = lba >> 16;
	cb[4] = lba >> 8;
	cb[5] = lba;
	cb[8] = 1;	/* 1 block */
	msc_build_cbw(cb, 10);
	return msc_transport(cb, 10, 1, data, 512);
}

/* ------------------------------------------------------------------ */
/* Public entry                                                        */
/* ------------------------------------------------------------------ */

/* Probe runs from late_init (framebuffer console already live).
 * After ALL USB work the loader halts here forever so XNU never boots
 * and the last line on the panel is always genuine USB output. */
static void usb_host_probe_3475_main(void)
{
	unsigned int caplen;
	unsigned int nports;
	uint8_t dd[18] __attribute__((aligned(4096)));
	uint8_t cd[512] __attribute__((aligned(4096)));
	unsigned int i = 0, np;
	unsigned long last_lba, blk;

	printk(KERN_INFO, "usb: Exynos3475 EHCI host probe\n");

	/* 1. clocks */
	printk(KERN_INFO, "usb: clocks\n");
	usb_clk_host_enable();

	/* 2. PMU de-isolate + sysreg host-link */
	printk(KERN_INFO, "usb: pmu+sysreg\n");
	usb_deisolate_host();
	usb_sysreg_host();

	/* 3. PHY host init: reset FIRST, then configure.  The earlier
	 * enable->reset order was wrong: STDPHYCTRL.SWRST_ALL in the reset
	 * wiped every register (FSEL, DRVVBUS, tunes) just after enable,
	 * leaving a default, never-locked PHY - port reset then hung. */
	printk(KERN_INFO, "usb: phy\n");
	usb_phy_reset_host();
	usb_phy_enable();

	/* 4. SM5703 MUIC: power 5V onto VBUS so a stick is detected */
	{
		int r = muic_sm5703_otg_enable();

		if (r)
			printk(KERN_ERR, "usb: MUIC boost failed (%d) "
			       "- no VBUS 5V\n", r);
	}

	/*
	 * 5. EHCI core enable.
	 * NOTE: VBUS 5V !!! On SM-J120F the 5V boost is driven by the
	 * SM5703 MUIC; without it a flash drive will never power up.
	 */
	caplen = rd32((volatile uint32_t *)EXYNOS3475_EHCI_BASE) & 0xff;
	op_regs = (volatile uint32_t *)(EXYNOS3475_EHCI_BASE + caplen);
	nports = (rd32((volatile uint32_t *)(EXYNOS3475_EHCI_BASE +
					    EXYNOS3475_EHCI_HCSPARAMS)) >> 0)
		 & 0xf;
	printk(KERN_INFO, "usb: ehci caplen=%u ports=%u\n", caplen, nports);

	if (caplen < 0x10) {
		printk(KERN_ERR, "usb: bogus caplen 0x%02x\n", caplen);
		return;
	}

	ehci_hc_reset();
	ehci_run();
	printk(KERN_INFO, "usb:cf1\n");
	*(op_regs + (EXYNOS3475_EHCI_CONFIGFLAG >> 2)) = 1;	/* CF */
	printk(KERN_INFO, "usb:cf2\n");
	dmb();
	usb_busy_us(100);

	/* PHY/link state just before the connect wait: does the PHY see the
	 * 5V fed into VBUS (external bank/Y-cable) and are D+/D- live? */
	{
		uint32_t lnk = *(PHY + 0x8c / 4);	/* OTGLINKCTRL */
		uint32_t res = *(PHY + 0x34 / 4);	/* RESUME */
		printk(KERN_INFO, "usb:phy resume=%08x otg_lnk=%08x (SPEED=%d "
		       "VBUSDET=%d BVALID=%d IDDIG=%d)\n",
		       res, lnk,
		       (lnk >> 15) & 1, (lnk >> 11) & 1,
		       (lnk >> 13) & 1, (lnk >> 12) & 1);
	}

	/* 5. reset the port(s) + wait for a device */
	printk(KERN_INFO, "usb: usbcmd=%08x hcsp=%08x\n",
	       op_regs[EXYNOS3475_EHCI_USBCMD >> 2],
	       rd32((volatile uint32_t *)EXYNOS3475_EHCI_BASE +
		    (EXYNOS3475_EHCI_HCSPARAMS >> 2)));
	for (i = 0; i < nports && i < 4; i++) {
		ehci_port_reset(i);
		if (*(op_regs + ((EXYNOS3475_EHCI_PORTSC +
				  (i << EXYNOS3475_EHCI_PORTSHIFT)) >> 2)) &
		    PORTSC_CCS)
			break;
	}
	if (i >= nports) {
		printk(KERN_INFO, "usb: nothing connected\n");
		return;
	}
	{
		unsigned int psc = *(op_regs + ((EXYNOS3475_EHCI_PORTSC +
						(i << EXYNOS3475_EHCI_PORTSHIFT)) >> 2));

		if ((psc & PORTSC_SPEED) != PORTSC_SPD_HS) {
			printk(KERN_WARNING,
			       "usb: device is not high-speed (portsc=%08x) - out of scope\n",
			       psc);
			return;
		}
	}

	/* 6. enumerate device 0 (EP0, mps=8) */
	printk(KERN_INFO, "usb: enum dev 0\n");
	if (usb_get_device_descriptor(0, EP0_MPS_INITIAL, dd)) {
		printk(KERN_ERR, "usb: device desc @mps8 failed\n");
		return;
	}
	dev_mps0 = dd[7];
	printk(KERN_INFO, "usb: vid=%04x pid=%04x class=%02x sub=%02x mps0=%u\n",
	       dd[8] | (dd[9] << 8), dd[10] | (dd[11] << 8), dd[4], dd[5],
	       dev_mps0);

	if (usb_set_address(1)) {
		printk(KERN_ERR, "usb: set address failed\n");
		return;
	}
	dev_addr = 1;
	usb_busy_us(2000);

	/* 7. full descriptor + config */
	printk(KERN_INFO, "usb: full device + config desc\n");
	if (usb_get_device_descriptor(dev_addr, dev_mps0, dd)) {
		printk(KERN_ERR, "usb: re-get device desc failed\n");
		return;
	}
	if (usb_get_config_descriptor(9, cd)) {
		printk(KERN_ERR, "usb: config head failed\n");
		return;
	}
	np = cd[2] | (cd[3] << 8);
	if (np > sizeof(cd))
		np = sizeof(cd);
	if (usb_get_config_descriptor(np, cd)) {
		printk(KERN_ERR, "usb: config full failed\n");
		return;
	}
	if (usb_set_configuration(cd[5])) {
		printk(KERN_ERR, "usb: set configuration failed\n");
		return;
	}

	/* 8. walk interfaces, find mass-storage BOT bulk pair */
	ms_iface = 0xff;
	{
		unsigned int off = 9;

		while (off + 1 < np &&
		       !(cd[off] == 0 && cd[off + 1] == 0)) {
			unsigned int len = cd[off];
			unsigned int type = cd[off + 1];

			if (type == 0x04 && len >= 9) {	/* interface */
				unsigned int bClass = cd[off + 5];

				if (bClass == USB_CLASS_MASS_STORAGE) {
					ms_iface = cd[off + 2];
					ms_cfg = cd[off + 3];
					printk(KERN_INFO,
					       "msc: iface %u (sub %02x proto %02x)\n",
					       ms_iface, cd[off + 6], cd[off + 7]);
				}
			} else if (type == 0x05 && len >= 7 && ms_iface != 0xff) {
				unsigned int ep = cd[off + 2];

				if (cd[off + 3] & 0x80)
					bulk_in_ep = ep & 0xf;
				else
					bulk_out_ep = ep & 0xf;
				bulk_mps = cd[off + 4] | (cd[off + 5] << 8);
			}
			off += len;
		}
	}
	if (ms_iface == 0xff || !bulk_in_ep || !bulk_out_ep) {
		printk(KERN_WARNING,
		       "usb: no mass-storage endpooints (in=%u out=%u)\n",
		       bulk_in_ep, bulk_out_ep);
		return;
	}
	printk(KERN_INFO, "usb: msc bulk in=%u out=%u mps=%u\n",
	       bulk_in_ep, bulk_out_ep, bulk_mps);

	/* 9. BOT: reset, max-LUN, INQUIRY, READ CAPACITY, MBR */
	if (usb_msc_bulk_reset(ms_iface))
		printk(KERN_WARNING, "usb: msc reset rc\n");
	usb_busy_us(50000);

	if (msc_inquiry()) {
		printk(KERN_ERR, "usb: msc inquiry failed\n");
		return;
	}
	if (msc_read_capacity(&last_lba, &blk)) {
		printk(KERN_ERR, "usb: read capacity failed\n");
		return;
	}
	printk(KERN_INFO, "usb: capacity %lu MB (%lu x %lu B)\n",
	       (last_lba + 1) * blk / (1024 * 1024), last_lba + 1, blk);

	if (msc_read_sector(0, mbr_sector)) {
		printk(KERN_ERR, "usb: read sector0 failed\n");
		return;
	}
	if (mbr_sector[510] == 0x55 && mbr_sector[511] == 0xaa) {
		unsigned int p, ptype = 0, pstart = 0;

		for (p = 0; p < 4; p++) {
			unsigned int off = 446 + p * 16;

			ptype = mbr_sector[off + 4];
			pstart = mbr_sector[off + 8] |
				 (mbr_sector[off + 9] << 8) |
				 (mbr_sector[off + 10] << 16) |
				 (mbr_sector[off + 11] << 24);
			printk(KERN_INFO, "mbr: p%u type=%02x start=%08x\n",
			       p, ptype, pstart);
		}
	} else {
		printk(KERN_INFO, "msc: no MBR (superfloppy?)\n");
	}

	printk(KERN_INFO,
	       "usb: Stage A OK - FAT32 readdir is Stage B\n");
}

/* Entry from board late_init: probe, then stop here without booting XNU. */
void usb_host_probe_3475(void)
{
	usb_host_probe_3475_main();
	printk(KERN_INFO, "usb: probe done - halting (reset to re-run)\n");
	for (;;)
		;
}