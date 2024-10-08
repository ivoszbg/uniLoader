/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Drivers registration framework
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
#include <drivers/framework.h>
#include <stddef.h>

// Max number of drivers that can be registered
#define MAX_DRIVERS 256
#define DRIVER_NAME_LEN 20

// TODO
