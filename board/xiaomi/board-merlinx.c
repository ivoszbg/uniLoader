// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026 Ryo "evilMyQueen" Yamada <evilMyQueen@mainlining.org>
 * Copyright (c) 2026 knukima <knukima@proton.me>
 */

/*
 * This is a uniLoader board for xiaomi-merlinx (Redmi Note 9)
 * Tested on engineering fw (Android 10) and on V13.0.2.0.SJOMIXM (Android 12)
 * 
 * Based on the xiaomi-blossom board
 */

#include <board.h>
#include <util.h>
#include <string.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

#define UART_BASE 0x11002000
#define UART_LSR_BASE	0x14
#define UART_LSR_DR 	0x01 	/* Data ready */
#define UART_LSR_THRE 	0x20 	/* TX holding register empty */

#ifdef CONFIG_EARLYCON
void uart_putc(char ch)
{
    while (!(readl((void *)(UART_BASE + UART_LSR_BASE)) & UART_LSR_THRE))
        ;

    writel((unsigned int)(unsigned char)ch, (void *)UART_BASE);
}

void uart_puts(const char *s)
{
    while (*s != '\0') {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s);
        s++;
    }
}
#endif

static struct video_info merlinx_fb = {
    .format = FB_FORMAT_ARGB8888,
    .width = 1088, // the screen width is 1080px. Use 1088 for lines so they align to 4352 bytes
    .height = 2340,
    .stride = 4,
    .address = (void *)0x7dcb0000
};

static const struct device merlinx_devices[] = {
    { "simplefb", &merlinx_fb, "fb" },
};

struct board_data board_ops = {
    .name = "xiaomi-merlinx",
    .devices = merlinx_devices,
    .num_devices = ARRAY_SIZE(merlinx_devices),
    .quirks = 0
};
