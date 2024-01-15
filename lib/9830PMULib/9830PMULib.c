// 9830PMULib - A Driver to control the PMU on devices using the Exynos 9830 (990) platform, may also work on platforms above 9810
// 2024 - halal-beef

// Original sources: https://github.com/sonic011gamer/Mu-Samsung/blob/main/Silicon/Samsung/E9820Pkg/Library/ResetSystemLib/ResetSystemLib.c

#include <main.h>

// SWRESET Registers
#define SWRESET         0x3A00
#define SWRESET_TRIGGER (1 << 1)

// PMU Registers
#define PMU_BASE_ADDR   0x15860000
#define PMU_SYSIP_DAT0  0x810

int pmuWrite(int PmuBaseAddr, int Offset, int Value)
{
    writel(Value, PmuBaseAddr + Offset);
    return 0;
}

int warmReset()
{
    pmuWrite(PMU_BASE_ADDR, SWRESET, SWRESET_TRIGGER);
}
