/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, RadGoodNow (jekurbo@proton.me)
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define ANA_W_TIMEOUT_NUM	(10000)
#define W_UNLOCK_KEY		0xE551

/* Watchdog Addresses */
#define W_BASE		0x64220040
#define W_CTRL		(W_BASE + 0x08)
#define W_INT_CLR	(W_BASE + 0x0C)
#define W_INT_RAW	(W_BASE + 0x10)
#define W_LOCK		(W_BASE + 0x20)

/* Watchdog Bits */
#define W_INT_CLEAR_BIT		0x00000001
#define W_INT_RST_BIT		0x00000008
#define W_INT_EN_BIT		0x00000001
#define W_CNT_EN_BIT		0x00000002

#define B_FIFO_FULL		0x00000800

#define B_RD_CMD_BUSY		0x80000000

/* ADI Regs */
#define R_ADI_RD_CMD		0x64200028
#define R_ADI_FIFO_STS		0x64200030
#define R_ADI_RD_DATA		0x6420002C

int adi_write(uint32_t *reg, uint16_t val, uint32_t sync)
{
	while (!(readl((void *)R_ADI_FIFO_STS)) & B_FIFO_FULL) {
		writel(val, reg);
	}

	if (sync == 1) {
		while ((readl((void *)R_ADI_FIFO_STS)) & B_FIFO_FULL)
			;

		writel(val, reg);
	}

	return 0;
}

int adi_read(uint32_t reg)
{
	uint16_t val;
	int cnt = 2000;

	writel(reg, (void *)R_ADI_RD_CMD);
	do {
		val = readl((void *)R_ADI_RD_DATA);
		/* TODO: Implement a proper delay func */
		for (volatile int i = 0; i < 1000; i++)
			;
	} while ((val & B_RD_CMD_BUSY) && cnt--);

	return (val & 0xFFFF);
}

int p682lpn_init(void)
{
	uint32_t cnt = 0;
	uint16_t val;

	adi_write((void *)W_LOCK, W_UNLOCK_KEY, 1);

	adi_write((void *)W_INT_CLR, (W_INT_CLEAR_BIT | W_INT_RST_BIT), 1);
	while((adi_read(W_INT_RAW) & (W_INT_CLEAR_BIT | W_INT_RST_BIT)) && (cnt < ANA_W_TIMEOUT_NUM))
		cnt++;

	val = (adi_read(W_CTRL) & ~W_INT_EN_BIT) | 0;
	adi_write((void *)W_CTRL, val, 1);    /* cfgmode change */

	val = (adi_read(W_CTRL) & ~W_CNT_EN_BIT) | 0; 
	adi_write((void *)W_CTRL, val, 1);    /* cfgstate change */

	adi_write((void *)W_LOCK, ~W_UNLOCK_KEY, 1);
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info p682lpn_fb = {
	.format = FB_FORMAT_BGRA8888,
	.width = 720,
	.height = 1640,
	.stride = 4,
	.address = (void *)0x9e000000
};
#endif

int p682lpn_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &p682lpn_fb);
#endif
	return 0;
}

struct board_data board_ops = {
	.name = "itel-p682lpn",
	.ops = {
		.early_init = p682lpn_init,
		.drivers_init = p682lpn_drv,
	},
	.quirks = 0
};
