/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2026, Samsung Galaxy J1 (2016, SM-J120F, j1xlte) support
 * for uniLoader. Exynos 3475, 480x800 AMOLED.
 *
 * S-Boot leaves the DECON display engine scanning out a boot logo
 * framebuffer at 0x57E05000 (see bootloader_fb in the vendor kernel DTS),
 * so we simply reuse it as a simplefb console.
 */

#include <board.h>
#include <util.h>
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>
#include <soc/exynos3475.h>

/* Boot logo framebuffer left behind by S-Boot (ARGB8888).
 * Live scanout is panel-native 480x800, stride 480*4 (verified on hw:
 * rendering at the vendor DTS's 540-wide logo geometry scrambles text). */
#define FB_ADDRESS	0x57E05000
#define FB_WIDTH	480
#define FB_HEIGHT	800

int j1xlte_init(void)
{

	/* Do NOT touch DECON here: S-Boot left it scanning out the logo
	 * framebuffer at 0x57E05000; CPU writes to that RAM show up on the
	 * panel automatically. Poking TRIG_CON disturbed the scanout on
	 * other Samsung boards (see upstream gta4xl commit). */

	/* I: YELLOW @fb+0xF0000 = board early_init done. */
	{
		volatile unsigned int *fb =
			(volatile unsigned int *) FB_ADDRESS;
		unsigned int i;

		for (i = 0; i < FB_WIDTH * 24; i++)
			fb[(0x128000 >> 2) + i] = 0xFFFFFF00;
	}

	return 0;
}

void uart_puts(const char *s)
{
	while (*s) {
		/* Bounded wait: if S-Boot handed over with UART2 clock-gated,
		 * UTRSTAT reads stick and an unbounded poll would hang the
		 * whole boot with zero diagnostics. Bail instead. */
		unsigned int t = 50000;

		while (!(readl((void *)(EXYNOS3475_UART_BASE + EXYNOS3475_UTRSTAT))
			 & EXYNOS3475_UTRSTAT_TX_EMPTY)) {
			if (!--t)
				return;
		}
		writel((unsigned int)(*s++ & 0xff),
		       (void *)(EXYNOS3475_UART_BASE + EXYNOS3475_UTXH));
	}
}

#if defined(CONFIG_EXYNOS_SDMMC3475)
extern void sd_host_probe_3475(void);

/* Runs after driver_probe_all(): the simplefb console is live, so
 * printk() output lands on the panel. */
static int j1xlte_late_init(void)
{
	sd_host_probe_3475();
	return 0;
}
#elif defined(CONFIG_EXYNOS_USBHOST3475)
extern void usb_host_probe_3475(void);

/* Runs after driver_probe_all(): the simplefb console is live, so
 * printk() output lands on the panel. */
static int j1xlte_late_init(void)
{
	usb_host_probe_3475();
	return 0;
}
#endif

static struct video_info j1xlte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = FB_WIDTH,
	.height = FB_HEIGHT,
	.stride = 4,
	.address = (void *)FB_ADDRESS
};

static const struct device j1xlte_devices[] = {
	{ "simplefb", &j1xlte_fb, "fb" },
};

struct board_data board_ops = {
	.name = "samsung-j1xlte",
	.ops = {
		.early_init = j1xlte_init,
#if defined(CONFIG_EXYNOS_SDMMC3475) || defined(CONFIG_EXYNOS_USBHOST3475)
		.late_init = j1xlte_late_init,
#endif
	},
	.devices = j1xlte_devices,
	.num_devices = ARRAY_SIZE(j1xlte_devices),
	.quirks = 0
};
