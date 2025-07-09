/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

ENTRY(_start);

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)

INPUT(KERNEL_PATH)
INPUT(DTB_PATH)
INPUT(RAMDISK_PATH)

SECTIONS
{
	.text : {
		KEEP(*(.text.start))
		*(.text)
		*(.text.*)
	}

	.rodata ALIGN(4) : {
		*(.rodata)
		*(.rodata.*)
	}

	.data ALIGN(4) : {
		*(.data)
		*(.data.*)
	}

	.bss ALIGN(4) : {
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		__bss_end = .;
	}

	.stack ALIGN(4096) : {
		. = . + 4096;
		_stack_end = .;
	}

	.dtb ALIGN(4096) : {
		dtb = .;
		DTB_PATH
	}

	.kernel ALIGN(4096) : {
		kernel = .;
		KERNEL_PATH
	}

	.ramdisk ALIGN(4096) : {
		ramdisk = .;
		RAMDISK_PATH
	}

	kernel_size = SIZEOF(.kernel);
	ramdisk_size = SIZEOF(.ramdisk);
}
