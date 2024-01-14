// Keys! - a key driver that does some mmio magic
// 2024 - BotchedRPR <ibelwon@protonmail.com>

#include <keys.h>

int rebootToMode();

/* Don't call this function. */
int readKey(int base, int bank) {
    unsigned int address = ((base + bank) + 0x4);
    return ((unsigned char *)address)[0];
}

/* Call this one instead! :)*/
int checkKey(int button)
{
    switch(button)
    {
        case POWER_BUTTON:
            if (!(readKey(0x15850000, 0x40) & (1 << 0x4))) {
                return 1;
            }
        
        case VOLUMEDOWN_BUTTON:
            if (!(readKey(0x15850000, 0x0) & (1 << 0x4))) {
                return 1;
            }

        case VOLUMEUP_BUTTON:
            if (!(readKey(0x15850000, 0x00) & (1 << 0x3))) {
                return 1;
            }

        default:
            return 0;
    }
    return 0;
}

int anyKeyPressed()
{
    /* Probably useless right now. */
    if(checkKey(POWER_BUTTON) ||
        checkKey(VOLUMEDOWN_BUTTON) ||
        checkKey(VOLUMEUP_BUTTON))
    {
        return 1;
    }
    return 0;
}

int readKeyStatus(int key) 
{
    return checkKey(key);
}
