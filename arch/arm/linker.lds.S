/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 *
 * __image_start..__image_end covers everything that must be relocated as a
 * unit when S-Boot loads us away from TEXT_BASE.  The trailing .payload
 * section (embedded kernel/dtb/ramdisk) stays put; its runtime address is
 * just adjusted by the relocation delta.
 */

ENTRY(_start);

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

SECTIONS
{
	. = TEXT_BASE;

	__image_start = .;

	.text : {
		KEEP(*(.text.header))
		KEEP(*(.text.start))
		*(.text)
		*(.text.*)
	}

	.rodata ALIGN(4) : {
		*(.rodata)
		*(.rodata.*)
	}

	.uniloader_drivers ALIGN(4) : {
		__drivers_start = .;
		KEEP(*(.uniloader_drivers))
		__drivers_end = .;
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
		. += 0x10000;
		__stack_bottom = .;
	}

	__image_end = .;

	.payload ALIGN(0x1000) : {
		__payload_start = .;
		KEEP(*(.payload))
		__payload_end = .;
	}

	_end = .;

	/* size optimization for flat binaries. */
	/DISCARD/ : {
		*(.eh_frame)
		*(.eh_frame.*)
		*(.note.gnu.property)
		*(.note.gnu.build-id)
		*(.comment)
		*(.ARM.attributes)
	}
}
