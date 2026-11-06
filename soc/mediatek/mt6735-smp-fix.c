// SPDX-License-Identifier: GPL-2.0-only
/*
 * MT6735 Secondary CPU bringup SMP fix
 * Boots up CPUs before Linux starts, pass them via a spin table in the device DTS.
 * Based on https://gitlab.com/mt6735-mainline/mt6735-smp-fix
 *
 * Copyright (c) 2026, Sonya Sireneva <sleirsgoevy@gmail.com>
 * Copyright (c) 2026, Kaloyan Donev <donevkaloyan@gmail.com>
 */

#include <../../include/lib/libfdt/libfdt.h>
#include <stdint.h>
#include <lib/debug.h>
#include <limits.h>

extern char cpu_entry[];
extern char cpu_entry_end[];
extern uint64_t cpu_spin_table[3];

#define SPM_BASE		0x10000000UL

#define SPM_REG(off)		((volatile uint32_t *)(SPM_BASE + (off)))

#define SPM_POWERON_CONFIG_SET	SPM_REG(0x6000)
#define SPM_CA7_CPUTOP_PWR_CON	SPM_REG(0x6208)
#define SPM_CA7_CPU1_PWR_CON	SPM_REG(0x6218)
#define SPM_CA7_CPUTOP_L2_PDN	SPM_REG(0x6244)
#define SPM_CA7_CPU1_L1_PDN	SPM_REG(0x6264)
#define SPM_CA15_CPUTOP_PWR_CON	SPM_REG(0x62b0)
#define SPM_CA15_L1_PWR_CON	SPM_REG(0x62b4)
#define SPM_PWR_STATUS		SPM_REG(0x660c)
#define SPM_PWR_STATUS_2ND	SPM_REG(0x6610)

// Standard PSCI AAARCH32 command for starting a CPU, as defined in psci.h in ATF
#define PSCI_CPU_ON_AARCH32 0x84000003
// All 3 values here where derived from the Lenovo Vibe P1ma40 downstream kernel (https://github.com/Lenovo-vibe-p1m/android_kernel_lenovo_p1m_oreo) in drivers/misc/mediatek/base/power/mt6735/mt_spm.h
#define SPM_PROJECT_CODE	0x0b16
#define SPM_REGWR_CFG_KEY	(SPM_PROJECT_CODE << 16)
#define SPM_REGWR_EN		0x1

enum {
	PWR_RST_B = 1,
	PWR_ISO = 2,
	PWR_ON = 4,
	PWR_ON_2ND = 8,
	PWR_CLK_DIS = 16,
	SRAM_CKISO = 32,
	SRAM_ISOINT_B = 64,
	L1_PDN = 1,
	L1_PDN_ACK = 256,
	CA7_CPUTOP = 512,
	CA7_CPU0 = 8192,
	CA15_CPUTOP = 65536,
};

__asm__(
	".section .text.cpu_entry\n"
	".global cpu_entry\n"
	".global cpu_spin_table\n"
	".global cpu_entry_end\n"

	"cpu_entry:\n"
	"	mrs	x1, mpidr_el1\n"
	"	tst	x1, #1\n"
	"	b	1f\n"
	"	ldr	x1, =0x10211010\n"
	"	mov	w2, #2048\n"
	"	str	w2, [x1]\n"
	"	str	w2, [x1, #256]\n"
	"1:\n"
	"	str	xzr, [x0]\n"
	"1:\n"
	"	wfe\n"
	"	ldr	x30, [x0]\n"
	"	cbnz	x30, 1f\n"
	"	b	1b\n"
	"1:\n"
	"	mov	x0, #0\n"
	"	mov	x1, #0\n"
	"	mov	x2, #0\n"
	"	mov	x3, #0\n"
	"	ret\n"

	".section .text.smc\n"
	".global smc\n"

	"smc:\n"
	"	smc	#0\n"
	"	dsb	sy\n"
	"	isb\n"
	"	ret\n"

	".p2align 3\n"
	"cpu_spin_table:\n"
	"	.8byte	1\n"
	"	.8byte	1\n"
	"	.8byte	1\n"

	"cpu_entry_end:\n"

	".text\n"
);

static void power_domain_on(volatile uint32_t *ctl, uint32_t sta,
							volatile uint32_t *l1_pdn)
{
	writel(SPM_REGWR_CFG_KEY | SPM_REGWR_EN, SPM_POWERON_CONFIG_SET);
	writel(readl(ctl) | PWR_ON, ctl);
	writel(readl(ctl) | PWR_ON_2ND, ctl);

	while (!(readl(SPM_PWR_STATUS) & sta) ||
		!(readl(SPM_PWR_STATUS_2ND) & sta))
		;

	writel(readl(ctl) & ~PWR_ISO, ctl);

	if (l1_pdn) {
		writel(readl(l1_pdn) & ~L1_PDN, l1_pdn);

		while (readl(l1_pdn) & L1_PDN_ACK)
			;
	}

	writel(readl(ctl) | SRAM_ISOINT_B, ctl);
	writel(readl(ctl) & ~SRAM_CKISO, ctl);
	writel(readl(ctl) & ~PWR_CLK_DIS, ctl);
	writel(readl(ctl) | PWR_RST_B, ctl);
}

static void power_on_cpu(int nr)
{
	power_domain_on(&SPM_CA7_CPU1_PWR_CON[nr - 1], CA7_CPU0 >> nr,
					&SPM_CA7_CPU1_L1_PDN[2 * (nr - 1)]);
}

uint64_t smc(uint64_t fn, uint64_t a1, uint64_t a2, uint64_t a3);

static int set_cpu_entry(int nr, uint64_t entry, uint64_t opaque)
{
	return smc(PSCI_CPU_ON_AARCH32, nr, entry, opaque);
}

void cpu_on(int nr, uint64_t *spin)
{
	set_cpu_entry(nr, (uintptr_t)cpu_entry, (uintptr_t)spin);
	power_on_cpu(nr);

	while (*spin)
		asm volatile("" : "=m"(*spin));
}

int mt6735_start_secondary_cpus(void *fdt)
{
	char name[] = "/cpus/cpu@?";
	int address_cells = 2;
	int size_cells = 1;
	const fdt32_t *cells_prop;
	const void *prop;
	fdt32_t reg[4];
	uint64_t size, addr;
	uintptr_t start, end;
	int node, parent, len, cells_len, reg_len, rc;
	size_t i;

	rc = fdt_check_header(fdt);
	if (rc < 0) {
		printk(KERN_ERR, "MT6735 SMP Fix: invalid FDT header: %s\n",
			   fdt_strerror(rc));
		return rc;
	}

	for (i = 1; i < 4; i++) {
		name[10] = (char)('0' + i);

		node = fdt_path_offset(fdt, name);
		if (node == -FDT_ERR_NOTFOUND) {
			printk(KERN_INFO, "MT6735 SMP Fix: %s not found, skipping\n",
				   name);
			continue;
		}

		if (node < 0) {
			printk(KERN_ERR, "MT6735 SMP Fix: failed to look up %s: %s\n",
				   name, fdt_strerror(node));
			return node;
		}

		prop = fdt_getprop(fdt, node, "cpu-release-addr", &len);
		if (prop == NULL) {
			if (len == -FDT_ERR_NOTFOUND) {
				printk(KERN_INFO, "MT6735 SMP Fix: %s has no cpu-release-addr, skipping\n",
					   name);
				continue;
			}

			printk(KERN_ERR, "MT6735 SMP Fix: failed to read cpu-release-addr of %s: %s\n",
				   name, fdt_strerror(len));
			return len;
		}

		if (len != sizeof(fdt64_t)) {
			printk(KERN_WARNING, "MT6735 SMP Fix: %s: cpu-release-addr is %d bytes, expected %d, skipping\n",
				   name, len, (int)sizeof(fdt64_t));
			continue;
		}

		cpu_on((int)i, &cpu_spin_table[i - 1]);

		addr = (uint64_t)(uintptr_t)&cpu_spin_table[i - 1];
		rc = fdt_setprop_inplace_u64(fdt, node, "cpu-release-addr",
									 addr);
		if (rc < 0) {
			printk(KERN_ERR, "MT6735 SMP Fix: failed to set cpu-release-addr of %s: %s\n",
				   name, fdt_strerror(rc));
			return rc;
		}
	}

	node = fdt_path_offset(fdt, "/reserved-memory/spin-table@0");
	if (node < 0) {
		printk(KERN_ERR, "MT6735 SMP Fix: failed to find /reserved-memory/spin-table@0: %s\n",
			   fdt_strerror(node));
		return node;
	}

	prop = fdt_getprop(fdt, node, "reg", &len);
	if (prop == NULL) {
		printk(KERN_ERR, "MT6735 SMP Fix: failed to read spin-table reg: %s\n",
			   fdt_strerror(len));
		return len;
	}

	if (len < (int)sizeof(fdt32_t)) {
		printk(KERN_ERR, "MT6735 SMP Fix: spin-table reg is too short (%d bytes)\n",
			   len);
		return -FDT_ERR_BADVALUE;
	}

	parent = fdt_path_offset(fdt, "/reserved-memory");
	if (parent < 0) {
		printk(KERN_ERR, "MT6735 SMP Fix: failed to find /reserved-memory: %s\n",
			   fdt_strerror(parent));
		return parent;
	}

	/* Read #address-cells directly. */
	cells_prop = fdt_getprop(fdt, parent, "#address-cells", &cells_len);
	if (cells_prop != NULL) {
		if (cells_len != sizeof(*cells_prop)) {
			printk(KERN_ERR, "MT6735 SMP Fix: #address-cells has bad length %d\n", cells_len);
			return -FDT_ERR_BADVALUE;
		}

		address_cells = (int)fdt32_to_cpu(*cells_prop);
	} else if (cells_len != -FDT_ERR_NOTFOUND) {
		printk(KERN_ERR, "MT6735 SMP Fix: failed to read #address-cells: %s\n",
			   fdt_strerror(cells_len));
		return cells_len;
	}

	/* Read #size-cells directly. */
	cells_prop = fdt_getprop(fdt, parent, "#size-cells", &cells_len);
	if (cells_prop != NULL) {
		if (cells_len != sizeof(*cells_prop)) {
			printk(KERN_ERR, "MT6735 SMP Fix: #size-cells has bad length %d\n", cells_len);
			return -FDT_ERR_BADVALUE;
		}

		size_cells = (int)fdt32_to_cpu(*cells_prop);
	} else if (cells_len != -FDT_ERR_NOTFOUND) {
		printk(KERN_ERR, "MT6735 SMP Fix: failed to read #size-cells: %s\n",
			   fdt_strerror(cells_len));
		return cells_len;
	}

	if (address_cells < 1 || address_cells > 2 ||
		size_cells < 1 || size_cells > 2) {
		printk(KERN_ERR, "MT6735 SMP Fix: unsupported #address-cells=%d / #size-cells=%d\n",
			   address_cells, size_cells);
		return -FDT_ERR_BADVALUE;
		}

		reg_len = (address_cells + size_cells) * (int)sizeof(fdt32_t);
	if (len != reg_len) {
		printk(KERN_ERR, "MT6735 SMP Fix: spin-table reg is %d bytes, expected %d\n",
			   len, reg_len);
		return -FDT_ERR_BADVALUE;
	}

	start = (uintptr_t)cpu_entry;
	end = (uintptr_t)cpu_entry_end;
	if (end <= start) {
		printk(KERN_ERR, "MT6735 SMP Fix: invalid cpu_entry range\n");
		return -FDT_ERR_BADVALUE;
	}

	size = (uint64_t)(end - start);

	if (address_cells == 1) {
		if (start > 0xffffffffULL) {
			printk(KERN_ERR, "MT6735 SMP Fix: cpu_entry does not fit in 1 address cell\n");
			return -FDT_ERR_BADVALUE;
		}

		reg[0] = cpu_to_fdt32((uint32_t)start);
	} else {
		reg[0] = cpu_to_fdt32((uint32_t)((uint64_t)start >> 32));
		reg[1] = cpu_to_fdt32((uint32_t)start);
	}

	if (size_cells == 1) {
		if (size > 0xffffffffULL) {
			printk(KERN_ERR, "MT6735 SMP Fix: spin-table size does not fit in 1 size cell\n");
			return -FDT_ERR_BADVALUE;
		}

		reg[address_cells] = cpu_to_fdt32((uint32_t)size);
	} else {
		reg[address_cells] = cpu_to_fdt32((uint32_t)(size >> 32));
		reg[address_cells + 1] = cpu_to_fdt32((uint32_t)size);
	}

	rc = fdt_setprop_inplace(fdt, node, "reg", reg, reg_len);
	if (rc < 0) {
		printk(KERN_ERR, "MT6735 SMP Fix: failed to set spin-table reg: %s\n",
			   fdt_strerror(rc));
		return rc;
	}

	printk(KERN_INFO, "MT6735 SMP Fix: secondary cpus brought up\n");

	return 0;
}
