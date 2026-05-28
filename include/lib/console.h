// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

struct console {
	void (*write)(int level, const char *prefix, const char *msg);
};

void console_register(const struct console *con);

void earlycon_register(void);

void early_console_init(void);

#endif // CONSOLE_H_
