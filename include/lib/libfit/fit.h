// SPDX-License-Identifier: GPL-2.0-only
/*
 * FIT image handling logic
 *
 * FIT specification: https://fitspec.osfw.foundation
 *  Copyright 2008 Marian Balakowicz <m8@semihalf.com>
 *  Copyright 2008 Bartlomiej Sieka <tur@semihalf.com>
 *  Copyright 2022 Simon Glass <sjg@chromium.com>
 *
 * uniLoader implementation:
 *  Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#ifndef LIBFIT_FIT_H
#define LIBFIT_FIT_H

int fit_open(const void *fit);
int fit_get_configuration(const void *fit);
int fit_boot_configuration(const void *fit);

const void* fit_get_kernel_base(void);
const void* fit_get_fdt_base(void);
const void* fit_get_ramdisk_base(void);

int fit_get_kernel_size(void);
int fit_get_fdt_size(void);
int fit_get_ramdisk_size(void);

#endif // LIBFIT_FIT_H
