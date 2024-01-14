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

// Reboot Modes
#define REBOOT_MODE_NORMAL		0x00
#define REBOOT_MODE_CHARGE		0x0A
#define REBOOT_MODE_FASTBOOT		0xFC
#define REBOOT_MODE_RECOVERY		0xFF

int pmuWrite(int PmuBaseAddr, int Offset, int Value)
{
    writel(Value, PmuBaseAddr + Offset);
    return 0;
}

int pmuUpdate(int PmuBaseAddr, int Offset, int Mask, int Value)
{
  int i;
  int read;

  if (Offset > 0x3FFF) {
    read = readl(PmuBaseAddr + Offset);
    read &= ~Mask;
    read |= (Value & Mask);
    writel(read, PmuBaseAddr + Offset);
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

int warmReset()
{
    pmuWrite(PMU_BASE_ADDR, SWRESET, SWRESET_TRIGGER);
}

int powerOff()
{
    pmuUpdate(PMU_BASE_ADDR, PS_HOLD_CONTROL, PS_HOLD_EN, 0);
    return 0;
}

int rebootToMode()
{
    // TODO
writel(REBOOT_MODE_RECOVERY, PMU_BASE_ADDR + PMU_SYSIP_DAT0);
warmReset();
return 0;
}
