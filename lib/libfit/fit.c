// SPDX-License-Identifier: GPL-2.0-only
/*
 * Base FIT image handling logic
 *
 * FIT specification: https://fitspec.osfw.foundation
 *  Copyright 2008 Marian Balakowicz <m8@semihalf.com>
 *  Copyright 2008 Bartlomiej Sieka <tur@semihalf.com>
 *  Copyright 2022 Simon Glass <sjg@chromium.com>
 *
 * uniLoader implementation:
 *  Copyright (c) 2026, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <lib/debug.h>
#include <lib/libfdt/libfdt.h>
#include <string.h>

#define DEFAULT_CONFIG_PATH_MAX_LEN	255

char* configuration;

const void* kernel_base = 0x0;
const void* fdt_base = 0x0;
const void* ramdisk_base = 0x0;

int kernel_size, fdt_size, ramdisk_size;

#if defined(CONFIG_ARCH_AARCH64)
#define FIT_ARCH "arm64"
#elif defined(CONFIG_ARCH_ARM)
#define FIT_ARCH "arm"
#endif

static bool fit_check_arch(const void* fit, int offset)
{
	const char* arch;

	arch = fdt_getprop(fit, offset, "arch", NULL);
	if (!arch)
	{
		printk(KERN_ERR, "Missing architecture property\n");
		return false;
	}

	if (strcmp(arch, FIT_ARCH) == 0)
		return true;

	return false;
}

static int fit_load_kernel(const void *fit, char *kernel_path)
{
	int kernel_offset = fdt_path_offset(fit, kernel_path);
	if (kernel_offset < 0)
	{
		printk(KERN_ERR, "Invalid FIT (couldn't find %s)!: %s\n",
		       kernel_path, fdt_strerror(kernel_offset));
		return kernel_offset;
	}

	if (fit_check_arch(fit, kernel_offset) != true)
	{
		printk(KERN_ERR, "Invalid FIT (Wrong kernel architecture)!\n");
		return -1;
	}

	kernel_base = fdt_getprop(fit, kernel_offset, "data", &kernel_size);
	if (kernel_base == NULL)
	{
		printk(KERN_ERR, "Invalid FIT (Invalid kernel data)!\n");
		return -2;
	}

	return 0;
}

static int fit_load_fdt(const void *fit, char *fdt_path)
{
	int fdt_offset = fdt_path_offset(fit, fdt_path);
	if (fdt_offset < 0)
	{
		printk(KERN_ERR, "Invalid FIT (couldn't find %s)!: %s\n",
		       fdt_path, fdt_strerror(fdt_offset));
		return fdt_offset;
	}

	if (fit_check_arch(fit, fdt_offset) != true)
	{
		printk(KERN_ERR, "Invalid FIT (Wrong FDT architecture)!\n");
		return -3;
	}

	fdt_base = fdt_getprop(fit, fdt_offset, "data", &fdt_size);
	if (fdt_base == NULL)
	{
		printk(KERN_ERR, "Invalid FIT (Invalid FDT data)!\n");
		return -4;
	}

	return 0;
}

static void fit_load_ramdisk(const void *fit, char *ramdisk_path)
{
	int ramdisk_offset = fdt_path_offset(fit, ramdisk_path);
	if (ramdisk_offset < 0)
	{
		printk(KERN_ERR, "Invalid FIT (couldn't find %s)!: %s\n",
		       ramdisk_path, fdt_strerror(ramdisk_offset));
		return;
	}

	ramdisk_base = fdt_getprop(fit, ramdisk_offset, "data", &ramdisk_size);
	if (fdt_base == NULL)
	{
		printk(KERN_WARNING, "Invalid FIT (Invalid ramdisk data)!\n");
		// Once again, ramdisk is optional.
	}
}

const void* fit_get_kernel_base(void)
{
	return kernel_base;
}

const void* fit_get_fdt_base(void)
{
	return fdt_base;
}

const void* fit_get_ramdisk_base(void)
{
	return ramdisk_base;
}

int fit_get_kernel_size(void)
{
	return kernel_size;
}

int fit_get_fdt_size(void)
{
	return fdt_size;
}

int fit_get_ramdisk_size(void)
{
	return ramdisk_size;
}

int fit_load_images(const void *fit, const char* kernel_name,
		    const char* fdt_name, const char* ramdisk_name,
		    int kernel_len, int fdt_len, int ramdisk_len)
{
	char kernel_path[DEFAULT_CONFIG_PATH_MAX_LEN];
	char fdt_path[DEFAULT_CONFIG_PATH_MAX_LEN];
	char ramdisk_path[DEFAULT_CONFIG_PATH_MAX_LEN];
	int ret;

	/* Prepare paths for kernel/ramdisk/dtb */
	// TODO: For some reason, initializing these strings with any value causes an exception
	strcpy(kernel_path, "/images/");
	strcat(kernel_path, (char*)kernel_name);

	strcpy(fdt_path, "/images/");
	strcat(fdt_path, (char*)fdt_name);

	if (ramdisk_len > 0)
	{
		strcpy(ramdisk_path, "/images/");
		strcat(ramdisk_path, (char*)ramdisk_name);
	}

	/* Load kernel */
	ret = fit_load_kernel(fit, kernel_path);
	if (ret != 0)
	{
		printk(KERN_ERR, "Failed to load kernel image (%i).\n", ret);
		return ret;
	}

	/* Load fdt */
	ret = fit_load_fdt(fit, fdt_path);
	if (ret != 0)
	{
		printk(KERN_ERR, "Failed to load FDT image (%i).\n", ret);
		return ret;
	}

	/* Try to load ramdisk */
	if (ramdisk_len <= 0)
		return 0;

	fit_load_ramdisk(fit, ramdisk_path);
	// Ramdisk is optional. It may fail, it may not.

	return 0;
}

int fit_boot_configuration(const void *fit)
{
	int configuration_offset;
	const char *description, *kernel_name, *fdt_name, *ramdisk_name;
	int kernel_len, fdt_len, ramdisk_len;

	configuration_offset = fdt_path_offset(fit, configuration);
	if (configuration_offset < 0)
	{
		printk(KERN_ERR, "Invalid configuration %s!: %s\n",
		       configuration, fdt_strerror(configuration_offset));
		return configuration_offset;
	}

	kernel_name = fdt_getprop(fit, configuration_offset,
				     "kernel", &kernel_len);
	if (!kernel_name)
	{
		printk(KERN_ERR, "No kernel specified in configuration %s!\n",
		       configuration);
		return -1;
	}

	fdt_name = fdt_getprop(fit, configuration_offset,
				  "fdt", &fdt_len);
	if (!fdt_name)
	{
		printk(KERN_ERR, "No FDT specified in configuration %s!\n",
		       configuration);
		return -1;
	}

	if (kernel_len <= 0 || fdt_len <= 0)
	{
		printk(KERN_ERR, "Invalid Kernel/FDT name in FIT!\n");
		return -2;
	}

	ramdisk_name = fdt_getprop(fit, configuration_offset,
				  "ramdisk", &ramdisk_len);
	if (!ramdisk_name)
	{
		printk(KERN_WARNING, "No ramdisk specified in configuration %s!\n",
		       configuration);

		// We can get around no ramdisk. It may be embedded in the kernel.
		ramdisk_len = -1;
	}

	description = fdt_getprop(fit, configuration_offset,
				  "description", NULL);
	if (!description)
	{
		printk(KERN_WARNING, "Running an unknown action.\n");
		goto boot;
	}
	printk(KERN_INFO, "Running action: %s\n", description);

boot:
	fit_load_images(fit, kernel_name, fdt_name, ramdisk_name,
			kernel_len, fdt_len, ramdisk_len);
	return 0;
}

int fit_get_configuration(const void *fit)
{
	int configurations_offset, def_len;
	const char* default_config;
	static char default_config_path[DEFAULT_CONFIG_PATH_MAX_LEN];
	const char configs_path[] = "/configurations";
	size_t len = strlen(configs_path);

	configurations_offset = fdt_path_offset(fit, configs_path);
	if (configurations_offset < 0)
	{
		printk(KERN_ERR, "FIT image has no configurations?: %s\n",
		       fdt_strerror(configurations_offset));
		return configurations_offset;
	}

	// TODO: Multiple configurations
	default_config = fdt_getprop(fit, configurations_offset,
				     "default", &def_len);
	if (!default_config)
	{
		printk(KERN_ERR, "Default boot configuration unspecified.\n");
		return -1;
	}

	/*
	 * +2 is:
	 *  - 1 '/' character for the path
	 *  - 1 null terminator
	 */
	if (len + def_len + 2 > DEFAULT_CONFIG_PATH_MAX_LEN)
	{
		printk(KERN_ERR, "Configuration path overflow!\n");
		return -2;
	}

	strcpy(default_config_path, configs_path);

	default_config_path[len] = '/';
	default_config_path[len + 1] = '\0';

	strcat(default_config_path, (char*)default_config);

	configuration = &default_config_path[0];

	printk(KERN_INFO, "Using configuration: %s\n", configuration);

	return 0;
}

int fit_open(const void *fit)
{
	int ret, len;
	const char* desc;

	ret = fdt_check_header(fit);
	if (ret != 0)
	{
		printk(KERN_ERR, "Couldn't open appended FIT image: %s\n",
		       fdt_strerror(ret));
	}

	desc = fdt_getprop(fit, 0, "description", &len);
	if (!desc)
	{
		printk(KERN_ERR, "Missing description property\n");
		return -1;
	}

	printk(KERN_INFO, "Loaded FIT image: %s\n", desc);

	return ret;
}
