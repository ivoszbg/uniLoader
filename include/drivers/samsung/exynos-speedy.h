/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (c) 2024 Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2024 Markuss Broks <markuss.broks@gmail.com>
 *
 * Bindings for exynos-speedy
 */

#ifndef EXYNOS_SPEEDY_H_
#define EXYNOS_SPEEDY_H_

struct speedy_transaction {
	unsigned long base;
	unsigned int slave;
	unsigned int offset;
	unsigned int val;
};

extern int speedy_read(struct speedy_transaction *tr);
extern int speedy_write(struct speedy_transaction *tr);

#endif /* EXYNOS_SPEEDY_H_ */
