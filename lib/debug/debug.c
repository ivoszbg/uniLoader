// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>
#include <string.h>
#include <lib/debug.h>
#include <lib/simplefb.h>
#include <lib/video/font.h>

long int debug_linecount = 0;

void uart_puts(const char *s);

// Global log level that controls the verbosity
static int global_loglevel = CONFIG_LOGLEVEL;

void printk(int log_level, char *text)
{
	if (log_level > global_loglevel)
		return;

	char *prefix;
	switch (log_level) {
	case KERN_EMERG:
		prefix = "[EMERG] ";
		break;
	case KERN_ALERT:
		prefix = "[ALERT] ";
		break;
	case KERN_CRIT:
		prefix = "[CRIT] ";
		break;
	case KERN_ERR:
		prefix = "[ERROR] ";
		break;
	case KERN_WARNING:
		prefix = "[WARN] ";
		break;
	case KERN_NOTICE:
		prefix = "[NOTICE] ";
		break;
	case KERN_INFO:
		prefix = "[INFO] ";
		break;
	case KERN_DEBUG:
		prefix = "[DEBUG] ";
		break;
	default:
		prefix = "[LOG] ";
		break;
	}

#ifdef CONFIG_UART_DEBUG
	uart_puts(prefix);
	uart_puts(text);
	uart_puts("\r");
#endif

#ifdef CONFIG_SIMPLE_FB
	// Actually the first line's Y pos. Scales in __simplefb_raw_print
	int y_pos = 5;
	int prefix_width = strlen(prefix) * SCALED_FONTW;

	__simplefb_raw_print((char*)CONFIG_FRAMEBUFFER_BASE, prefix, 0, y_pos,
		  CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
	__simplefb_raw_print((char*)CONFIG_FRAMEBUFFER_BASE, text, prefix_width, y_pos,
		  CONFIG_FRAMEBUFFER_WIDTH, CONFIG_FRAMEBUFFER_STRIDE);
#endif
}
