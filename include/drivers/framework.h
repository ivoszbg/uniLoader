/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Drivers registration framework
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#ifndef DRIVERS_H_	/* Include guard */
#define DRIVERS_H_

#include <stddef.h>
#include <string.h>

// Define the type for the probe function pointer
typedef void (*probe_func_t)(void *data);

extern probe_func_t find_driver_probe(const char *name);

// Function registry entry
struct driver_registry_entry {
	const char *name;        // Driver name
	probe_func_t probe;      // Function pointer to the probe function
};

#define REGISTER_DRIVER(name, probe_func, data)		\
	do {						\
		(probe_func)(data);			\
	} while (0)

#endif // DRIVERS_H_
