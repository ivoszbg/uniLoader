// Keys! - a key driver that does some mmio magic
// 2024 - BotchedRPR <ibelwon@protonmail.com>

#include <main.h>
#include <string.h>


int readKey(int base, int bank)
{
    unsigned int address = ((base + bank) + 0x4);
    return ((char*)address)[0];
}

void testreadkeys()
{
    int powerHeld = 0;
    int vol1Held = 0;
    int vol2Held = 0;

    if (!(readKey(0x15850000, 0x40) & (1 << 0x4))) {
        powerHeld = 1;
    }
    else
    {
        if(powerHeld == 1)
        {
            printk("Power released.");
        }
    }

    if (!(readKey(0x15850000, 0x0) & (1 << 0x4))) {
        vol1Held = 1;
    }
    else
    {
        if(vol1Held == 1)
        {
            printk("Vol- released.");
        }
    }

    if (!(readKey(0x15850000, 0x00) & (1 << 0x3))) {
        vol2Held = 1;
    }
    else
    {
        if(vol2Held == 1)
        {
            printk("Vol+ released.");
        }
    }
}