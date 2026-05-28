// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <lib/console.h>
#include <lib/debug.h>

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

/* earlycon (usually uart) + fb; bump if a third sink appears. */
#define MAX_CONSOLES 2

static const struct console *consoles[MAX_CONSOLES];
static unsigned int n_consoles;

void console_register(const struct console *con)
{
	if (con && n_consoles < MAX_CONSOLES)
		consoles[n_consoles++] = con;
}

static const char *log_prefix(int level)
{
	static const char *const prefix[] = {
		[KERN_EMERG]	= "[EMERG] ",
		[KERN_ALERT]	= "[ALERT] ",
		[KERN_CRIT]	= "[CRIT] ",
		[KERN_ERR]	= "[ERROR] ",
		[KERN_WARNING]	= "[WARN] ",
		[KERN_NOTICE]	= "[NOTICE] ",
		[KERN_INFO]	= "[INFO] ",
		[KERN_DEBUG]	= "[DEBUG] ",
	};

	return (level >= 0 && level < (int)(sizeof(prefix) / sizeof(prefix[0]))) ?
		prefix[level] : "[UNKNOWN] ";
}

#ifdef CONFIG_UART_DEBUG
extern void uart_puts(const char *s);

static void earlycon_write(int level, const char *prefix, const char *msg)
{
	(void)level; /* unused */
	uart_puts(prefix);
	uart_puts(msg);
	uart_puts("\r");
}

static const struct console earlycon = {
	.write = earlycon_write,
};

void earlycon_register(void)
{
	console_register(&earlycon);
}
#else
void earlycon_register(void) {}
#endif

/* no set-up console for early debugging w/ uart; overridable in board files */
__attribute__((weak)) void early_console_init(void)
{
	earlycon_register();
}

void printk(int log_level, const char *fmt, ...)
{
	if (log_level > CONFIG_LOGLEVEL || !n_consoles)
		return;

	va_list args;
	static char print_buffer[PRINTK_BUFFER_SIZE];

	va_start(args, fmt);
	npf_vsnprintf(print_buffer, sizeof(print_buffer), fmt, args);
	va_end(args);

	const char *prefix = log_prefix(log_level);

	for (unsigned int i = 0; i < n_consoles; i++)
		consoles[i]->write(log_level, prefix, print_buffer);
}
