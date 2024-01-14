// Bootmenu!
// Copyright (c) 2024 - BotchedRPR

#include <lib/simplefb.h> // We write to framebuffer via simplefb lib to omit printk
#include <main.h> // debug shit
#include <lib/minlib.h> // minlib

#define ENTRY_OFFSET 20

int bootmenu_show()
{
    int centerH = CONFIG_FRAMEBUFFER_WIDTH / 2;

    int bootspace = CONFIG_FRAMEBUFFER_HEIGHT - CONFIG_FRAMEBUFFER_HEIGHT / 2; // uhhhh change if too much

    color white = {255,255,255};
    draw_filled_rectangle(CONFIG_FRAMEBUFFER_BASE, 0, centerH, CONFIG_FRAMEBUFFER_WIDTH, 10, white, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE); // i hate this
    draw_filled_rectangle(CONFIG_FRAMEBUFFER_BASE, 0, centerH + bootspace, CONFIG_FRAMEBUFFER_WIDTH, 10, white, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);

    /* from now on we treat centerH + 10 as our y base value and bootspace as y max value relative to centerH + 10 */
    for(int i = centerH + 10; i < bootspace + (centerH + 10); i=i+20)
    {
        draw_horizontal_line(CONFIG_FRAMEBUFFER_BASE, 0, CONFIG_FRAMEBUFFER_WIDTH, i, white, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
    }
}
