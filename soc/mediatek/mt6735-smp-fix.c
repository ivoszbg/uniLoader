#include <../../include/lib/libfdt/libfdt.h>
#include <stdint.h>
#include <lib/debug.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long uintptr_t;
typedef unsigned long long uint64_t;
typedef uintptr_t size_t;
extern char cpu_entry[];
extern char cpu_entry_end[];
extern uint64_t cpu_spin_table[3];

#define SPM_POWERON_CONFIG_SET (*(volatile uint32_t*)0x10006000)
#define SPM_CA7_CPUTOP_PWR_CON (*(volatile uint32_t*)0x10006208)
#define SPM_CA7_CPU1_PWR_CON ((volatile uint32_t*)0x10006218)
#define SPM_CA7_CPUTOP_L2_PDN (*(volatile uint32_t*)0x10006244)
#define SPM_CA7_CPU1_L1_PDN ((volatile uint32_t*)0x10006264)
#define SPM_CA15_CPUTOP_PWR_CON (*(volatile uint32_t*)0x100062b0)
#define SPM_CA15_L1_PWR_CON (*(volatile uint32_t*)0x100062b4)
#define SPM_PWR_STATUS (*(volatile uint32_t*)0x1000660c)
#define SPM_PWR_STATUS_2ND (*(volatile uint32_t*)0x10006610)

enum
{
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
    "mrs x1, mpidr_el1\n"
    "tst x1, #1\n"
    "b 1f\n"
    "ldr x1, =0x10211010\n"
    "mov w2, #2048\n"
    "str w2, [x1]\n"
    "str w2, [x1, #256]\n"
    "1:\n"
    "str xzr, [x0]\n"
    "1:\n"
    "wfe\n"
    "ldr x30, [x0]\n"
    "cbnz x30, 1f\n"
    "b 1b\n"
    "1:\n"
    "mov x0, #0\n"
    "mov x1, #0\n"
    "mov x2, #0\n"
    "mov x3, #0\n"
    "ret\n"

    ".section .text.smc\n"
    ".global smc\n"

    "smc:\n"
    "smc #0\n"
    "dsb sy\n"
    "isb\n"
    "ret\n"

    ".p2align 3\n"
    "cpu_spin_table:\n"
    ".8byte 1\n"
    ".8byte 1\n"
    ".8byte 1\n"

    "cpu_entry_end:\n"

    ".text\n"
);

static void power_domain_on(volatile uint32_t* ctl, uint32_t sta, volatile uint32_t* l1_pdn)
{
    SPM_POWERON_CONFIG_SET = 0xb160001;
    *ctl |= PWR_ON;
    *ctl |= PWR_ON_2ND;
    while(!(SPM_PWR_STATUS & sta) || !(SPM_PWR_STATUS_2ND & sta));
    *ctl &= ~PWR_ISO;
    if(l1_pdn)
    {
        *l1_pdn &= ~L1_PDN;
        while((*l1_pdn & L1_PDN_ACK));
    }
    *ctl |= SRAM_ISOINT_B;
    *ctl &= ~SRAM_CKISO;
    *ctl &= ~PWR_CLK_DIS;
    *ctl |= PWR_RST_B;
}

static void power_on_cpu(int nr)
{
    power_domain_on(&SPM_CA7_CPU1_PWR_CON[nr-1], CA7_CPU0 >> nr, &SPM_CA7_CPU1_L1_PDN[2*(nr-1)]);
}

uint64_t smc(...);

static int set_cpu_entry(int nr, uint64_t entry, uint64_t opaque)
{
    return smc(0x84000003, nr, entry, opaque);
    /*register uint64_t r0 __asm__("r0") = 0x84000003;
     *   register uint64_t r1 __asm__("r1") = nr;
     *   register uint64_t r2 __asm__("r2") = entry;
     *   register uint64_t r3 __asm__("r3") = opaque;
     *   asm volatile("smc #0":"=r"(r0):"r"(r0),"r"(r1),"r"(r2),"r"(r3));
     *   return r0;*/
}

extern char cpu_entry[];

void cpu_on(int nr, uint64_t* spin)
{
    set_cpu_entry(nr, (uintptr_t)cpu_entry, (uintptr_t)spin);
    power_on_cpu(nr);
    while(*spin)
        asm volatile("":"=m"(*spin));
}
void mt6735_start_secondary_cpus(void *fdt)

{
    if (fdt_check_header(fdt) < 0)
        return;

    char name[] = "/cpus/cpu@?";

    for (size_t i = 1; i < 4; i++)
    {
        name[10] = (char)('0' + i);

        int node = fdt_path_offset(fdt, name);

        if (node == -FDT_ERR_NOTFOUND)
            continue;

        if (node < 0)
            return;

        int len;
        const void *prop =
        fdt_getprop(fdt, node, "cpu-release-addr", &len);

        if (prop == NULL)
        {
            if (len == -FDT_ERR_NOTFOUND)
                continue;

            return;
        }

        if (len != sizeof(fdt64_t))
            continue;

        cpu_on((int)i, &cpu_spin_table[i - 1]);

        int rc = fdt_setprop_inplace_u64(
            fdt,
            node,
            "cpu-release-addr",
            (uint64_t)(uintptr_t)&cpu_spin_table[i - 1]
        );

        if (rc < 0)
            return;
    }

    int node = fdt_path_offset(
        fdt, "/reserved-memory/spin-table@0");

    if (node == -FDT_ERR_NOTFOUND)
        return;

    if (node < 0)
        return;

    int len;
    const void *prop = fdt_getprop(fdt, node, "reg", &len);

    if (prop == NULL || len < (int)sizeof(fdt32_t))
        return;

    int parent = fdt_path_offset(fdt, "/reserved-memory");
    if (parent < 0)
        return;

    int address_cells = 2; /* Default #address-cells */
    int size_cells = 1;   /* Default #size-cells */
    int cells_len;
    const fdt32_t *cells_prop;

    /* Read #address-cells directly. */
    cells_prop = fdt_getprop(
        fdt, parent, "#address-cells", &cells_len);

    if (cells_prop != NULL)
    {
        if (cells_len != sizeof(*cells_prop))
            return;

        address_cells = (int)fdt32_to_cpu(*cells_prop);
    }
    else if (cells_len != -FDT_ERR_NOTFOUND)
    {
        return;
    }

    /* Read #size-cells directly. */
    cells_prop = fdt_getprop(
        fdt, parent, "#size-cells", &cells_len);

    if (cells_prop != NULL)
    {
        if (cells_len != sizeof(*cells_prop))
            return;

        size_cells = (int)fdt32_to_cpu(*cells_prop);
    }
    else if (cells_len != -FDT_ERR_NOTFOUND)
    {
        return;
    }

    if (address_cells < 1 || address_cells > 2 ||
        size_cells < 1 || size_cells > 2)
        return;

    int reg_len =
    (address_cells + size_cells) * (int)sizeof(fdt32_t);

    if (len != reg_len)
        return;

    uintptr_t start = (uintptr_t)cpu_entry;
    uintptr_t end = (uintptr_t)cpu_entry_end;

    if (end <= start)
        return;

    uint64_t size = (uint64_t)(end - start);
    fdt32_t reg[4];

    if (address_cells == 1)
    {
        if (start > 0xffffffffULL)
            return;

        reg[0] = cpu_to_fdt32((uint32_t)start);
    }
    else
    {
        reg[0] = cpu_to_fdt32((uint32_t)((uint64_t)start >> 32));
        reg[1] = cpu_to_fdt32((uint32_t)start);
    }

    if (size_cells == 1)
    {
        if (size > 0xffffffffULL)
            return;

        reg[address_cells] = cpu_to_fdt32((uint32_t)size);
    }
    else
    {
        reg[address_cells] =
        cpu_to_fdt32((uint32_t)(size >> 32));
        reg[address_cells + 1] =
        cpu_to_fdt32((uint32_t)size);
    }

    fdt_setprop_inplace(fdt, node, "reg", reg, reg_len);

    printk(KERN_INFO, "MT6735 SMP Fix: Bringing up secondary cpus...\n");
}
