/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Drivers registration framework
 * Copyright (c) 2026, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stddef.h>
#include <string.h>
#include <drivers/framework.h>

extern const struct driver __drivers_start[];
extern const struct driver __drivers_end[];

const struct driver *driver_find(const char *name)
{
	const struct driver *d;

	if (!name)
		return NULL;

	for (d = __drivers_start; d < __drivers_end; d++) {
		if (d->name && !strcmp(d->name, name))
			return d;
	}

	return NULL;
}

void driver_probe_all(const struct device *devices, size_t count)
{
	size_t i;

	if (!devices)
		return;

	for (i = 0; i < count; i++) {
		const struct driver *drv = driver_find(devices[i].driver_name);

		if (drv)
			drv->probe(devices[i].plat_data);
	}
}
