/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

ENTRY(_start);

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)


SECTIONS
{
	. = TEXT_BASE;

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

	.stack ALIGN(0x1000) : {
		__stack_top = .;
		. += 0x1000;
		__stack_bottom = .;
	}

	_end = .;
}
