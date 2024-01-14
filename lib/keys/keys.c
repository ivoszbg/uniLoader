// Keys! - a key driver that does some mmio magic
// 2024 - BotchedRPR <ibelwon@protonmail.com>

#include <main.h>

#define POWER_BUTTON 0
#define VOLUMEDOWN_BUTTON 1
#define VOLUMEUP_BUTTON 2

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
		printk("Resetting system.");
		rebootToMode();
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
    /* This code looks (and is) very hacky but it works. */
    if(checkKey(POWER_BUTTON) ||
        checkKey(VOLUMEDOWN_BUTTON) ||
        checkKey(VOLUMEUP_BUTTON))
    {
        return 1;
    }
    return 0;
}

void testreadkeys() {
    int isPressed = 0;

    while(1)
    {
        if(isPressed == 0)
        {
            if(anyKeyPressed())
            {
                printk("You have pressed a key. Good job.");
                isPressed = 1;
            }
        }
        else if(!anyKeyPressed())
        {
            isPressed = 0;
        }
    }
}
