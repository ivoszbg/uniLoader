/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2002, Shane Nay <shane@minirl.com>
 * Copyright (c) 2003-2008, Simtec Electronics <linux@simtec.co.uk>
 * Copyright (c) 2021, Samsung Electronics Co., Ltd.
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2026, Aiden Isik <aidenisik+git@member.fsf.org>
 *
 * Parts of this code are adapted from the Linux kernel:
 * - drivers/misc/uart_sel/uart_sel.c (Linux 5.15 SM-R861 downstream - Samsung)
 * - drivers/tty/serial/samsung_tty.c (Linux 7.0 mainline - Simtec)
 * - include/linux/serial_s3c.h (Linux 7.0 mainline - Shane Nay)
 */
#include <board.h>
#include <util.h>
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

/* Framebuffer constants */
#define DECON_F_BASE 0x138C0000
#define HW_SW_TRIG_CONTROL 0x70
#define HW_TRIG_EN 0x1

/* MUIC constants */
#define SYSREG_APM_BASE 0x12820000
#define JTAG_OVER_USB 0x600
#define SEL_TXD_USB_PHY 0x634
#define SEL_RXD_CP_UART 0x650
#define SEL_RXD_AP_UART_DEBUG 0x658

#define APBIF_PMU_ALIVE_BASE 0x12860000
#define USB20_PHY_CONFIGURATION 0x72C

#define USB20DRD_CTRL0_BASE 0x10520000
#define HSP_CTRL 0x54

/* UART constants */
#define UART_BASE 0x10080000
#define S3C2410_UTXH 0x20
#define S3C2410_UFCON 0x8
#define S3C2410_UFCON_FIFOMODE 0x1
#define S3C2410_UTRSTAT 0x10
#define S3C2410_UTRSTAT_TXFE 0x2
#define S3C2410_UFSTAT 0x18
#define S5PV210_UFSTAT_TXFULL 0x1000000

int lucky7_init(void)
{
	/* Enable writing to the framebuffer */
	writel(HW_TRIG_EN, (void *)(DECON_F_BASE + HW_SW_TRIG_CONTROL));

	/* Switch MUIC UART from Cellular Processor (CP) to Application Processor (AP) */
	writel(0x1, (void *)(APBIF_PMU_ALIVE_BASE + USB20_PHY_CONFIGURATION));
	writel(0x4, (void *)(SYSREG_APM_BASE + JTAG_OVER_USB));

	uint32_t hsp_ctrl_reg = readl((void *)(USB20DRD_CTRL0_BASE + HSP_CTRL));
	writel(hsp_ctrl_reg | (0x3 << 24), (void *)(USB20DRD_CTRL0_BASE + HSP_CTRL));

	writel(0x0, (void *)(SYSREG_APM_BASE + SEL_TXD_USB_PHY));
	writel(0x3, (void *)(SYSREG_APM_BASE + SEL_RXD_AP_UART_DEBUG));
	writel(0x0, (void *)(SYSREG_APM_BASE + SEL_RXD_CP_UART));

	return 0;
}

#ifdef CONFIG_UART_DEBUG
void uart_putc(const char ch)
{
	/* Wait until TX is ready */
	if (readl((void *)(UART_BASE + S3C2410_UFCON)) & S3C2410_UFCON_FIFOMODE)
	{
		/* FIFO wait */
		while (readl((void *)(UART_BASE + S3C2410_UFSTAT)) & S5PV210_UFSTAT_TXFULL);
	}
	else
	{
		/* Non-FIFO wait */
		while (!(readl((void *)(UART_BASE + S3C2410_UTRSTAT)) & S3C2410_UTRSTAT_TXFE));
	}

	/* Write the character to TX */
	writel(ch, (void *)(UART_BASE + S3C2410_UTXH));
}

void uart_puts(const char *s)
{
	while (*s != '\0')
	{
		uart_putc(*s);
		s++;
	}
}
#endif

static struct video_info lucky7_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 396,
	.height = 396,
	.stride = 4,
	.address = (void *)0x9e0cf000
};

static const struct device lucky7_devices[] = {
	{ "simplefb", &lucky7_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-lucky7",
	.ops = {
		.early_init = lucky7_init,
	},
	.devices = lucky7_devices,
	.num_devices = ARRAY_SIZE(lucky7_devices),
	.quirks = 0
};
