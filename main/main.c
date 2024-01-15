/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2024, BotchedRPR <ibelwon@protonmail.com>
 */

#include <main.h>
#include <bootmenu/bootmenu.h>
#include <keys.h>

void memcpy(void *dest, void *src, int size);

int rebootToMode();

// Function to write two strings using printk (HACK)
void write_two_strings(char* str1, char* str2) {
    // Concatenate the two strings
    char combined_str[100];  // Adjust the size based on your needs
    int i, j;

    // Copy characters from str1 to combined_str
    for (i = 0; str1[i] != '\0'; ++i) {
        combined_str[i] = str1[i];
    }

    // Copy characters from str2 to combined_str
    for (j = 0; str2[j] != '\0'; ++j) {
        combined_str[i + j] = str2[j];
    }

    // Null-terminate the combined string
    combined_str[i + j] = '\0';

    // Call printk with the combined string
    printk(combined_str);
}


void main(void* dt, void* kernel) {
	/* Initialize SoC and Board specific peripherals/quirks */

	/* TODO: Find a better way to make this more universal (since devices like arm64 Samsung Galaxies enable FB after soc_init) */
#ifdef CONFIG_SIMPLE_FB
	clean_fb((char*)CONFIG_FRAMEBUFFER_BASE, CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_HEIGHT, CONFIG_FRAMEBUFFER_STRIDE);
#endif

	write_two_strings("uniLoader ", VERSION);

	soc_init();
	printk("soc_init() passed!");

	board_init();
	printk("board_init() passed!");

	/* Copy kernel to memory and boot  */
	printk("Booting linux...");

    int sel = bootmenu_show();

    if(sel == 1)
    {
        memcpy((void*)CONFIG_PAYLOAD_ENTRY, kernel, (unsigned long) &kernel_size);
        load_kernel(dt, 0, 0, 0, (void*)CONFIG_PAYLOAD_ENTRY);
    }
    else if(sel == 2)
    {
        rebootToMode();
    }

	/* We shouldn't get there */
	while(1) {}
}
