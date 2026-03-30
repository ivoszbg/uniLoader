/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) schoosh212 <superaviation001@gmail.com>
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#define DECON_F_BASE            0x13060000
#define HW_SW_TRIG_CONTROL      0x70

// Early initialization
int a135f_init(void)
{
        /* Allow framebuffer to be written to */
        *(int*) (DECON_F_BASE + HW_SW_TRIG_CONTROL) = 0x1281;
        return 0;
}

#ifdef CONFIG_SIMPLE_FB
static struct video_info a135f_fb = {
        .format = FB_FORMAT_ARGB8888,
        .width = 1080,
        .height = 2408,
        .stride = 4,
        .address = (void *)0xfa000000
};
#endif

int a135f_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
        REGISTER_DRIVER("simplefb", simplefb_probe, &a135f_fb);
#endif
        return 0;
}

struct board_data board_ops = {
        .name = "samsung-a135f",
        .ops = {
                .early_init = a135f_init,
                .drivers_init = a135f_drv,
        },
        .quirks = 0
};
