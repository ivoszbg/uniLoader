/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Drivers registration framework
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#ifndef DRIVERS_FRAMEWORK_H_
#define DRIVERS_FRAMEWORK_H_

#include <stddef.h>

struct driver {
	const char *name;
	int (*probe)(void *plat_data);
};

struct device {
	const char *driver_name;
	void *plat_data;
	const char *label;
};

#define DRIVER_REGISTER(_name, _probe_fn)				\
	static const struct driver __uniloader_drv_##_probe_fn		\
		__attribute__((used, section(".uniloader_drivers"),	\
			       aligned(__alignof__(struct driver)))) = {\
			.name = (_name),				\
			.probe = (_probe_fn),				\
		}

const struct driver *driver_find(const char *name);
void driver_probe_all(const struct device *devices, size_t count);

#endif /* DRIVERS_FRAMEWORK_H_ */
