// SPDX-License-Identifier: GPL-2.0-only
/*
 * Samsung Galaxy A50 (SM-A505F) bootloader-framebuffer handoff.
 *
 * Copyright (c) 2026, Realgamer7067 <khantjaimin37@gmail.com>
 *
 * Based on board/samsung/board-gta4xl.c:
 * Copyright (c) 2025, Alexandru Chimac <alex@chimac.ro>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <stdint.h>
#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/debug.h>
#include <lib/simplefb.h>
#include <soc/exynos9610.h>

#define A50_FB_BASE			0xca000000UL
#define A50_PD_DISPAUD_STATUS		0x11864004UL
#define A50_LOCAL_PWR_CFG		0x0f
#define A50_HW_TRIG_EN			(1U << 0)
#define A50_HW_TRIG_MASK_DECON		(1U << 4)

static struct video_info a50_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1080,
	.height = 2340,
	.stride = 4,
	.address = (void *)A50_FB_BASE,
};

static const struct device a50_devices[] = {
	{ "simplefb", &a50_fb, "fb" },
};

static int a50_display_handoff(void)
{
	volatile uint32_t *pd_status = (void *)A50_PD_DISPAUD_STATUS;
	volatile uint32_t *trig_control =
		(void *)(DECON_F_BASE + HW_SW_TRIG_CONTROL);
	uint32_t status = *pd_status;
	uint32_t before;
	uint32_t after;

	/*
	 * DECON MMIO is unsafe while pd-dispaud is off. The Exynos power-domain
	 * status field is at base + 4 and reads 0xf when the domain is on.
	 */
	if ((status & A50_LOCAL_PWR_CFG) != A50_LOCAL_PWR_CFG) {
		printk(KERN_WARNING,
		       "a50: pd-dispaud is off (status=%x); trigger untouched\n",
		       status);
		return 0;
	}

	/*
	 * The A50 panel is MIPI command mode with a hardware TE trigger. Preserve
	 * the bootloader's polarity/selection/timer fields; only enable and
	 * unmask DECON as the vendor driver's trigger helper does.
	 */
	before = *trig_control;
	after = (before | A50_HW_TRIG_EN) & ~A50_HW_TRIG_MASK_DECON;
	if (after != before) {
		*trig_control = after;
		__asm__ volatile("dsb sy" ::: "memory");
	}

	printk(KERN_INFO, "a50: pd=%x decon-trig=%x->%x\n",
	       status, before, after);
	return 0;
}

struct board_data board_ops = {
	.name = "samsung-a50",
	.ops = {
		.late_init = a50_display_handoff,
	},
	.devices = a50_devices,
	.num_devices = ARRAY_SIZE(a50_devices),
	.quirks = 0,
};
