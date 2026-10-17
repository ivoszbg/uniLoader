/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Exynos 3475 USB host (EHCI) register definitions for uniLoader.
 *
 * Ground truth: kernel-j120f (LineageOS/Samsung 3.10) - drivers/usb/phy/
 * phy-samsung-usb2-cal.c, phy-samsung-dwc-usb2.{c,h} and drivers/usb/host/
 * ehci-s5p.c + drivers/clk/samsung/clk-exynos3475.c.
 *
 * Do NOT reference the mainline-j1 (6.x) port as a source of truth: it is
 * a half-working reference and its register assumptions may differ.
 */

#ifndef EXYNOS3475_USB_H_
#define EXYNOS3475_USB_H_

/* ---- Blocks ---- */
#define EXYNOS3475_EHCI_BASE		0x13510000
#define EXYNOS3475_USB2PHY_BASE		0x135c0000
#define EXYNOS3475_USBPHY_PMU_BASE	0x10460738	/* PHY isolation ctrl */
#define EXYNOS3475_USB20_SYSREG_BASE	0x13721020	/* HOST/DEV cfg */
#define EXYNOS3475_CMU_FSYS_BASE	0x13730000
#define EXYNOS3475_CMU_TOP_BASE		0x10010000

/* ---- CMU FSYS gates (clk-exynos3475.c) ---- */
#define EXYNOS3475_CLK_ENABLE_ACLK_FSYS_200		(EXYNOS3475_CMU_FSYS_BASE + 0x800)
#define  ACLK_FSYS200_USBHOST20		(1 << 2)
#define  ACLK_FSYS200_USBOTG20		(1 << 7)
#define  ACLK_FSYS200_XIU_D_FSYS1	(1 << 11)
#define  ACLK_FSYS200_AHB_USBHS		(1 << 14)
#define  ACLK_FSYS200_AHB2AXI_USBHS	(1 << 15)
#define  ACLK_FSYS200_UPSIZER_FSYS1	(1 << 19)
#define  ACLK_FSYS200_UPSIZER_AHB_USBHS (1 << 22)
#define EXYNOS3475_CLK_ENABLE_SCLK_USB20_PHY_CLKCORE	(EXYNOS3475_CMU_FSYS_BASE + 0x810)
#define EXYNOS3475_CLK_ENABLE_PHYCLK_USBHOST20_USB20_FREECLK	(EXYNOS3475_CMU_FSYS_BASE + 0x814)
#define EXYNOS3475_CLK_ENABLE_PHYCLK_USBHOST20_USB20_PHYCLOCK	(EXYNOS3475_CMU_FSYS_BASE + 0x818)
#define EXYNOS3475_CLK_ENABLE_PHYCLK_USBHOST20_USB20_CLK48MOHCI	(EXYNOS3475_CMU_FSYS_BASE + 0x81c)
#define EXYNOS3475_CLK_ENABLE_ACLK_FSYS_200_TOP		(EXYNOS3475_CMU_TOP_BASE + 0x800)

#define EXYNOS3475_USB_PLL_CON0		(EXYNOS3475_CMU_FSYS_BASE + 0x100)
#define EXYNOS3475_USB_PLL_LOCK		(EXYNOS3475_CMU_FSYS_BASE + 0x000)
#define EXYNOS3475_CLK_CON_MUX_USB_PLL	(EXYNOS3475_CMU_FSYS_BASE + 0x200)

/* ---- PHY (usb2phy@135c0000, phy-samsung-dwc-usb2.h) ---- */
#define EXYNOS3475_PHYUTMI		(0x08)
#define  PHYUTMI_OTGDISABLE		(1 << 6)
#define  PHYUTMI_IDPULLUP		(1 << 5)
#define  PHYUTMI_DRVVBUS		(1 << 4)
#define  PHYUTMI_DPPULLDOWN		(1 << 3)
#define  PHYUTMI_DMPULLDOWN		(1 << 2)
#define  PHYUTMI_FORCESUSPEND		(1 << 1)
#define  PHYUTMI_FORCESLEEP		(1 << 0)

#define EXYNOS3475_PHYCLKRST		(0x10)
#define  PHYCLKRST_FSEL_MASK		(0x7 << 5)
#define  PHYCLKRST_FSEL(x)		((x) << 5)
#define  PHYCLKRST_COMMONONN		(1 << 0)

#define EXYNOS3475_PHYPARAM0		(0x1c)
#define  PHYPARAM0_TXVREFTUNE(x)	((x) << 22)
#define  PHYPARAM0_TXVREFTUNE_MASK	(0xf << 22)
#define  PHYPARAM0_TXPREEMPAMPTUNE(x)	((x) << 15)
#define  PHYPARAM0_TXPREEMPAMPTUNE_MASK	(0x3 << 15)
#define  PHYPARAM0_SQRXTUNE(x)		((x) << 6)
#define  PHYPARAM0_SQRXTUNE_MASK	(0x7 << 6)
#define  PHYPARAM0_COMPDISTUNE(x)	((x) << 0)
#define  PHYPARAM0_COMPDISTUNE_MASK	(0x7 << 0)

#define EXYNOS3475_RESUME		(0x34)
#define  USB_BYPASSSEL			(1 << 4)
#define  USB_BYPASSDMEN			(1 << 3)
#define  USB_BYPASSDPEN			(1 << 2)

#define EXYNOS3475_PHY_STDPHYCTRL	(0x54)
#define  STDPHYCTRL_SIDDQ		(1 << 6)
#define  STDPHYCTRL_SWRST_PHY		(1 << 0)
#define  STDPHYCTRL_SWRST_ALL		(1 << 31)

#define EXYNOS3475_PHY_HOSTPLLTUNE	(0x70)
#define  HOSTPLLTUNE_PLLBTUNE		(1 << 6)

#define EXYNOS3475_LINK_HOSTLINKCTL	(0x88)
#define  HOSTLINKCTL_SW_RESET_PORT2	(1 << 3)
#define  HOSTLINKCTL_SW_RESET_PORT1	(1 << 2)
#define  HOSTLINKCTL_SW_RESET_PORT0	(1 << 1)
#define  HOSTLINKCTL_LINKSWRST		(1 << 0)

#define EXYNOS3475_LINK_OTGLINKCTL	(0x8c)

/* PHY control in the PMU reg block: enable (de-isolate) bit */
#define EXYNOS3475_USBPHY_ENABLE	(1 << 0)
/* sysreg: 1 = host link (EHCI), 0 = device link (UDC) */
#define EXYNOS3475_USB20PHY_CFG_HOST_LINK (1 << 0)

/* ---- EHCI MMIO ---- */
#define EXYNOS3475_EHCI_CAPLENGTH	0x00
#define EXYNOS3475_EHCI_HCSPARAMS	0x04
#define EXYNOS3475_EHCI_HCCPARAMS	0x08

#define EXYNOS3475_EHCI_USBCMD		0x00
#define  CMD_RUN			(1 << 0)
#define  CMD_HCRESET			(1 << 1)
#define  CMD_ASSE			(1 << 2)
#define  CMD_PSE			(1 << 4)
#define EXYNOS3475_EHCI_USBSTS		0x04
#define  STS_HCHALTED			(1 << 12)
#define  STS_ASSS			(1 << 15)
#define  STS_PSS			(1 << 14)
#define EXYNOS3475_EHCI_USBINTR		0x08
#define EXYNOS3475_EHCI_ASYNCLISTADDR	0x18
#define EXYNOS3475_EHCI_CONFIGFLAG	0x40
#define EXYNOS3475_EHCI_PORTSC		0x44
#define  PORTSC_CCS			(1 << 0)	/* device connected */
#define  PORTSC_CSC			(1 << 1)	/* connect status change */
#define  PORTSC_PE			(1 << 2)	/* port enabled */
#define  PORTSC_PEC			(1 << 3)	/* enable change */
#define  PORTSC_OC			(1 << 4)	/* overcurrent active */
#define  PORTSC_OCC			(1 << 5)	/* overcurrent change */
#define  PORTSC_PR			(1 << 8)	/* port reset */
#define  PORTSC_SPEED			(3 << 10)	/* port speed field */
#define  PORTSC_SPD_HS			(0 << 10)	/* high speed */
#define  PORTSC_SPD_FS			(1 << 10)	/* full speed */
#define  PORTSC_SPD_LS			(2 << 10)	/* low speed */
#define  PORTSC_PP			(1 << 12)	/* port power */
#define  PORTSC_OWNER			(1 << 13)	/* companion owns */

#define EXYNOS3475_EHCI_PORTSHIFT	1

/* ---- USB descriptor / class constants (Stage B: FAT) ---- */
#define USB_DEVICE_DESCRIPTOR_TYPE	0x01
#define USB_CONFIG_DESCRIPTOR_TYPE	0x02
#define USB_STRING_DESCRIPTOR_TYPE	0x03

#define USB_CLASS_MASS_STORAGE		0x08
#define USB_SUBCLASS_SCSI		0x06
#define USB_PROTOCOL_BULK_ONLY		0x50

#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_DIR_IN			0x80
#define USB_DIR_OUT			0x00
#define USB_TYPE_CLASS			0x20

#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_CONFIGURATION	0x09

#endif /* EXYNOS3475_USB_H_ */