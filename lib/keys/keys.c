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
    int isPressed = 0;

    while(1) {
        if(isPressed == 0) {
	         if (!(readKey(0x15850000, 0x40) & (1 << 0x4))) {
                    printk("POWER 0 pressed");
	                isPressed = 1;
       	     }

    	      if (!(readKey(0x15850000, 0x0) & (1 << 0x4))) {
                    printk("VOL- 1 pressed");
                    isPressed = 1;
              }

              if (!(readKey(0x15850000, 0x00) & (1 << 0x3))) {
                      printk("VOL+ 2 pressed");
                      isPressed = 1;
              }
        }
        else
        {
        // Poll all keys and check if non are pressed
        if((readKey(0x15850000, 0x40) & (1 << 0x4)) && (readKey(0x15850000, 0x0) & (1 << 0x4)) && (readKey(0x15850000, 0x00) & (1 << 0x3)))
            {
              isPressed = 0;
            }
    }
}
}
