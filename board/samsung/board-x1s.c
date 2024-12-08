/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, halal-beef <78730004+halal-beef@users.noreply.github.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>
#include <soc/exynos990.h>
#include <drivers/samsung/exynos-speedy.h>

void init_board_funcs(void *board)
{
	/*
	 * Parsing the struct directly without restructing is
	 * broken as of Sep 29 2024
	 */
	struct {
		const char *name;
		int ops[BOARD_OP_EXIT];
	} *board_restruct = board;

	board_restruct->name = "X1S";
}

struct s2mps19_data {
	uint32_t ldo_address;
};

static void s2mps19_setup(void)
{
	int ret;
	struct speedy_transaction s2mps19;

	/* S2MPS19 configuration */
	s2mps19.base = SPEEDY_0_BASE;
	s2mps19.slave = S2MPS19_PM_ADDR;

	/*
	 * Define LDO control register values
	 * Values have been borrowed from Linaro's LK
	 */
	const struct s2mps19_data data[] = {
		{
			S2MPS19_PM_LDO2M_CTRL
		}, {
			S2MPS19_PM_LDO15M_CTRL
		}
	};

	/* Configure LDOs */
	for (int i = 0; i < 3; i++) {
		s2mps19.offset = data[i].ldo_address;

		ret = speedy_read(&s2mps19);
		if (ret)
			goto handle_err;

		s2mps19.val |= S2MPS_OUTPUT_ON_NORMAL;

		ret = speedy_write(&s2mps19);
		if (ret)
			goto handle_err;
	}

	return;

handle_err:
	printk(KERN_ERR, "s2mps19: err\n");
	return;
}

// Early initialization
int board_init(void)
{
	/* Allow framebuffer to be written to */
	*(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
	return 0;
}

// Late initialization
int board_late_init(void)
{
	s2mps19_setup();

	return 0;
}

int board_driver_setup(void)
{
	struct {
		int width;
		int height;
		int stride;
		void *address;
	} simplefb_data = {
		.width = 1440,
		.height = 3200,
		.stride = 4,
		.address = (void *)0xf1000000
	};

	REGISTER_DRIVER("simplefb", simplefb_probe, &simplefb_data);
	return 0;
}
