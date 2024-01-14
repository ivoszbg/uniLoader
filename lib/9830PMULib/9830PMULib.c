// 9830PMULib - A Driver to control the PMU on devices using the Exynos 9830 (990) platform, may also work on platforms above 9810
// 2024 - halal-beef

// Original sources: https://github.com/sonic011gamer/Mu-Samsung/blob/main/Silicon/Samsung/E9820Pkg/Library/ResetSystemLib/ResetSystemLib.c

#include <main.h>

// SWRESET Registers
#define SWRESET         0x3A00
#define SWRESET_TRIGGER (1 << 1)

// PS_HOLD Registers
#define PS_HOLD_CONTROL 0x30C
#define PS_HOLD_EN      (1 << 8)

// PMU Registers
#define PMU_BASE_ADDR   0x15860000
#define PMU_SYSIP_DAT0  0x810

int pmuUpdate(int PmuBaseAddr, int Offset, int Mask, int Value)
{
  int i;

  if (Offset > 0x3FFF) {
    // TODO: Add Stuff
  } else {
    for (i = 0; i < 32; i++) {
      if (Mask & (1 << i)) {
        if (Value & (1 << i)) {
          writel(i, PmuBaseAddr + (Offset | 0xC000));
        } else {
          writel(i, PmuBaseAddr + (Offset | 0x8000));
        }
      }
    }
  }

  return 0;
}

int coldReset()
{
    // TODO
return 1;
}

int powerOff()
{
    pmuUpdate(PMU_BASE_ADDR, PS_HOLD_CONTROL, PS_HOLD_EN, 0);
    return 0;
}

int resetToMode (char * resetData)
{
    // TODO
return 1;
}
