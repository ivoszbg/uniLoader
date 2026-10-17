/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Exynos 3475 support for uniLoader
 */

#ifndef EXYNOS3475_H_	/* Include guard */
#define EXYNOS3475_H_

/* Debug UART: ttySAC2, S3C-style 32-bit registers */
#define EXYNOS3475_UART_BASE		0x13820000
#define EXYNOS3475_ULCON		0x00
#define EXYNOS3475_UCON			0x04
#define EXYNOS3475_UFCON		0x08
#define EXYNOS3475_UTRSTAT		0x10
#define EXYNOS3475_UTXH			0x20
#define EXYNOS3475_URXH			0x24

#define EXYNOS3475_UTRSTAT_TX_EMPTY	(1 << 1)
#define EXYNOS3475_UTRSTAT_RX_READY	(1 << 0)

/* DECON (display controller) */
#define EXYNOS3475_DECON_F_BASE		0x14830000
#define EXYNOS3475_DECON_TRIG_CON	0x6b0

#endif // EXYNOS3475_H_
