/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, predefine <predefine@yandex.ru>
 */

#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

#ifdef CONFIG_SIMPLE_FB
// Used on miui 12.5.8.0 firmware
static struct video_info begonia_r_fb = {
    .format = FB_FORMAT_ARGB8888,
    .width = 1088,
    .height = 2340,
    .stride = 4,
    .address = (void *)0x7d9b0000
};
#endif

int begonia_drv(void)
{
#ifdef CONFIG_SIMPLE_FB
    REGISTER_DRIVER("simplefb", simplefb_probe, &begonia_r_fb);
#endif
    return 0;
}

struct board_data board_ops = {
    .name = "xiaomi-begonia",
    .ops = {
        .drivers_init = begonia_drv,
    },
    .quirks = 0
};
