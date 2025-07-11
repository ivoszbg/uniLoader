// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <lib/debug.h>
#include <lib/simplefb.h>
#include <lib/video/font.h>

#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS	1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS	1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS		0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS		1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS		0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS		1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS	1
#define NANOPRINTF_USE_ALT_FORM_FLAG			1
#include <lib/nanoprintf.h>

#define PRINTK_BUFFER_SIZE 256

long int debug_linecount = 0;
extern void uart_puts(const char *s);

typedef struct {
	const char *prefix;
	color text_color;
} log_style_t;

static const color red = {255, 0, 0, 255};
static const color orange = {255, 165, 0, 255};
static const color yellow = {255, 255, 0, 255};
static const color cyan = {0, 255, 255, 255};
static const color green = {0, 255, 0, 255};
static const color gray = {128, 128, 128, 255};
static const color amber = {255, 192, 0, 255};

static inline log_style_t get_log_style(int level)
{
	static const log_style_t styles[] = {
		[KERN_EMERG]	= {"[EMERG] ", red},
		[KERN_ALERT]	= {"[ALERT] ", orange},
		[KERN_CRIT]	= {"[CRIT] ", yellow},
		[KERN_ERR]	= {"[ERROR] ", red},
		[KERN_WARNING]	= {"[WARN] ", yellow},
		[KERN_NOTICE]	= {"[NOTICE] ", green},
		[KERN_INFO]	= {"[INFO] ", amber},
		[KERN_DEBUG]	= {"[DEBUG] ", gray}
	};

	return (level >= 0 && level < sizeof(styles)/sizeof(styles[0])) ?
		styles[level] : (log_style_t){"[UNKNOWN] ", {255, 255, 255, 255}};
}

static inline void uart_output(const char *prefix, const char *message)
{
#ifdef CONFIG_UART_DEBUG
	uart_puts(prefix);
	uart_puts(message);
	uart_puts("\r");
#endif
}

static inline void fb_output(const char *prefix, const char *message, color text_color)
{
#ifdef CONFIG_SIMPLE_FB
	const int y_pos = 5;
	const int prefix_width = strlen(prefix) * SCALED_FONTW;

	__simplefb_raw_print((char*)CONFIG_FRAMEBUFFER_BASE, prefix, 0, y_pos, text_color);
	__simplefb_raw_print((char*)CONFIG_FRAMEBUFFER_BASE, message, prefix_width, y_pos, gray);
#endif
}

void printk(int log_level, const char *fmt, ...)
{
#if !defined(CONFIG_UART_DEBUG) && !defined(CONFIG_SIMPLE_FB)
	return;
#endif

	if (log_level > CONFIG_LOGLEVEL) return;

	va_list args;
	static char print_buffer[PRINTK_BUFFER_SIZE];

	const log_style_t style = get_log_style(log_level);

	va_start(args, fmt);
	npf_vsnprintf(print_buffer, sizeof(print_buffer), fmt, args);
	va_end(args);

	uart_output(style.prefix, print_buffer);
	fb_output(style.prefix, print_buffer, style.text_color);
}
