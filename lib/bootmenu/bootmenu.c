// Bootmenu!
// Copyright (c) 2024 - BotchedRPR

#include <lib/simplefb.h> // We write to framebuffer via simplefb lib to omit printk
#include <keys.h>
#include <main.h> // debug shit
#include <lib/minlib.h> // minlib

#define ENTRY_OFFSET 20

#define MAX_ENTRIES 2
#define ENTRIES (centerH + 3*20)

int rebootToMode();

void drawText()
{
    int centerH = CONFIG_FRAMEBUFFER_WIDTH / 2;

    draw_text(CONFIG_FRAMEBUFFER_BASE, "Entry 1 - It Works!", 5, centerH + (28 / 2), CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
    draw_text(CONFIG_FRAMEBUFFER_BASE, "Reset", 5, centerH + (68 / 2), CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
}

int lastSel = 0;

void highlightSelection(int selected)
{
    int centerH = CONFIG_FRAMEBUFFER_WIDTH / 2;

    int entriesY = centerH + 10;

    int entriesOffset = 20;

    draw_filled_rectangle(CONFIG_FRAMEBUFFER_BASE, 5, entriesY + ((lastSel-1) * entriesOffset - 1), CONFIG_FRAMEBUFFER_WIDTH, entriesOffset - 2, (color){0,0,0}, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
    draw_filled_rectangle(CONFIG_FRAMEBUFFER_BASE, 5, entriesY + ((selected-1) * entriesOffset - 1), CONFIG_FRAMEBUFFER_WIDTH, entriesOffset - 2, (color){255,50,50}, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
    drawText();

    lastSel = selected;
}

int startListener()
{
    int selected = 0;

    int selection = 1;

    while(!selected)
    {
        switch (checkKeys())
        {
            case POWER_BUTTON:
                selected = 1;
                break;

            case VOLUMEUP_BUTTON:
                if(selection-1 < 1)
                {
                    break;
                }
                selection--;
                printk("selection up");
                highlightSelection(selection);
                break;

            case VOLUMEDOWN_BUTTON:
                if(selection+1 > MAX_ENTRIES)
                {
                    break;
                }
                selection++;
                printk("selection down");
                highlightSelection(selection);
                break;
        }
    }

    return selection;
}

int bootmenu_show()
{
    int selection;

    int centerH = CONFIG_FRAMEBUFFER_WIDTH / 2;

    int bootspace = CONFIG_FRAMEBUFFER_HEIGHT - CONFIG_FRAMEBUFFER_HEIGHT / 2; // uhhhh change if too much

    draw_text(CONFIG_FRAMEBUFFER_BASE, "--uniLoader boot menu--", 20, centerH - 18, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);

    color white = {255,255,255};
    draw_filled_rectangle(CONFIG_FRAMEBUFFER_BASE, 0, centerH, CONFIG_FRAMEBUFFER_WIDTH, 10, white, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE); // i hate this
    draw_filled_rectangle(CONFIG_FRAMEBUFFER_BASE, 0, centerH + bootspace, CONFIG_FRAMEBUFFER_WIDTH, 10, white, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);

    /* from now on we treat centerH + 10 as our y base value and bootspace as y max value relative to centerH + 10 */
    /* calculate maximum number of entries with fontH + padding = 20 */
    // for(int i = centerH + 10; i < bootspace + (centerH + 10); i=i+20)
    for(int i = centerH + 10; i < ENTRIES; i=i+20)
    {
        draw_horizontal_line(CONFIG_FRAMEBUFFER_BASE, 0, CONFIG_FRAMEBUFFER_WIDTH, i, white, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
    }

    highlightSelection(1);

    selection = startListener();

    if(selection == 2)
    {
        rebootToMode();
    }

    return selection;
}
