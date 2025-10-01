/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>
#include <soc/exynos8895.h>
#include <drivers/samsung/exynos-speedy.h>

/* DECON Register MAP */
#define HW_SW_TRIG_CONTROL		0x70

/* MMIO MAP */
#define DECON_F_BASE			0x12860000
#define SPEEDY_BASE			0x15b50000

enum s2mps17_ldos {
	S2MPS17_LDO2,
	S2MPS17_LDO34,
	S2MPS17_LDO35,
	S2MPS17_END
};

struct s2mps17_data {
	uint8_t	ldo_ctrl_value;
	uint32_t ldo_voltage_value;
};

/* Slave addr = 0xCC */
/* S2MPS17 Register MAP */
#define S2MPS17_PMIC_REG_L2CTRL		0x3e
#define S2MPS17_PMIC_REG_L34CTRL	0x5f
#define S2MPS17_PMIC_REG_L35CTRL	0x60

#define S2MPS17_BUCK_MIN1	300000
#define S2MPS17_BUCK_MIN2	600000
#define S2MPS17_LDO_MIN1	700000
#define S2MPS17_LDO_MIN2	400000
#define S2MPS17_LDO_MIN3	1800000
#define S2MPS17_LDO_MIN4	300000

#define S2MPS17_LDO_STEP1	12500
#define S2MPS17_LDO_STEP2	25000

#define S2MPS17_LDO_VSEL_MASK	0x3f
#define S2MPS17_BUCK_VSEL_MASK	0xff

#define S2MPS17_ENABLE_MASK	(3 << 6)

#define S2MPS17_LDO2_VOLTAGE	2800000
#define S2MPS17_LDO34_VOLTAGE	1850000
#define S2MPS17_LDO35_VOLTAGE	3000000

#define _LDO(macro)	S2MPS17_LDO##macro
#define _REG(ctrl)	S2MPS17_PMIC_REG##ctrl
#define _ldo_ops(num)	s2mps17_ldo_ops##num
#define _TIME(macro)	S2MPS17_ENABLE_TIME##macro

static void s2mps17_setup(void)
{
	int ret;
	struct speedy_transaction s2mps17;

	/* S2MPS17 configuration */
	s2mps17.base = SPEEDY_BASE;
	s2mps17.slave = 1;

	/*
	 * Define LDO control register values
	 * Calculation formula: enable mask + (voltage to set - minimal voltage) / step value
	 */
	const struct s2mps17_data data[] = {
		{
			S2MPS17_PMIC_REG_L2CTRL, // vqmmc
			S2MPS17_ENABLE_MASK + (S2MPS17_LDO2_VOLTAGE -
					       S2MPS17_LDO_MIN3) / S2MPS17_LDO_STEP2
		}, {
			S2MPS17_PMIC_REG_L34CTRL, // tsp_io
			S2MPS17_ENABLE_MASK + (S2MPS17_LDO34_VOLTAGE -
					       S2MPS17_LDO_MIN1) / S2MPS17_LDO_STEP2
		}, {
			S2MPS17_PMIC_REG_L35CTRL, // tsp_avdd
			S2MPS17_ENABLE_MASK + (S2MPS17_LDO35_VOLTAGE -
					       S2MPS17_LDO_MIN3) / S2MPS17_LDO_STEP2
		}
	};

	/* Go ahead and enable the LDOs */
	for (int i = 0; i < S2MPS17_END; i++) {
		s2mps17.offset = data[i].ldo_ctrl_value;
		s2mps17.val = data[i].ldo_voltage_value;

		ret = speedy_write(&s2mps17);
		if (ret)
			goto handle_err;
	}

	return;

 handle_err:
	printk(KERN_ERR, "s2mps17: err\n");
	return;
}

int dreamlte_init(void)
{
	/* allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info dreamlte_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 2960,
	.stride = 4,
	.address = (void *)0xcc000000
};
#endif

int dreamlte_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
	REGISTER_DRIVER("simplefb", simplefb_probe, &dreamlte_fb);
#endif
	return 0;
}

int dreamlte_late_init(void)
{
	s2mps17_setup();

	return 0;
}

struct board_data board_ops = {
	.name = "samsung-dreamlte",
	.ops = {
		.early_init = dreamlte_init,
		.drivers_init = dreamlte_drv,
		.late_init = dreamlte_late_init,
	},
	.quirks = 0
};
