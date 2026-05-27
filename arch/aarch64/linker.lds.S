/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */
ENTRY(
#ifdef CONFIG_POSITION_INDEPENDENT
      _head
#else
      _start
#endif
     );


OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)

INPUT(KERNEL_PATH)
INPUT(DTB_PATH)
#ifndef CONFIG_RAMDISK_NO_COPY
INPUT(RAMDISK_PATH)
#endif

SECTIONS
{
	. = TEXT_BASE;

	.text : {
#ifdef CONFIG_POSITION_INDEPENDENT
		_reloc_start = .;
#endif
		*(.text)
		*(.text.*)
	}

	.kernel ALIGN(4096) : {
		kernel = .;
		KERNEL_PATH
	}

	.dtb ALIGN(4096) : {
		dtb = .;
		DTB_PATH
	}

#ifndef CONFIG_RAMDISK_NO_COPY
	.ramdisk ALIGN(4096) : {
		ramdisk = .;
		RAMDISK_PATH
	}
#endif

	.rodata ALIGN(8) : {
		*(.rodata)
		*(.rodata.*)
	}

	.uniloader_drivers ALIGN(8) : {
		__drivers_start = .;
		KEEP(*(.uniloader_drivers))
		__drivers_end = .;
	}

	.data ALIGN(8) : {
		*(.data)
		*(.data.*)
	}

	.bss ALIGN(8) : {
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(8);
		__bss_end = .;
	}

	.stack ALIGN(4096) : {
		. = . + 4096;
		_stack_end = .;
#ifdef CONFIG_POSITION_INDEPENDENT
		_reloc_end = .;
#endif
	}

	kernel_size = SIZEOF(.kernel);
#ifdef CONFIG_RAMDISK_NO_COPY
	ramdisk_size = RAMDISK_SIZE;
#else
	ramdisk_size = SIZEOF(.ramdisk);
#endif
#ifdef CONFIG_POSITION_INDEPENDENT
	_reloc_size = _reloc_end - _reloc_start;
#endif
#ifdef CONFIG_LINUX_KRNL_HEADER_IMG
	_kernel_size = _stack_end - _start;
#endif

	/* size optimization for flat binaries */
	/DISCARD/ : {
		*(.eh_frame)
		*(.eh_frame.*)
		*(.note.gnu.property)
		*(.note.gnu.build-id)
		*(.comment)
		*(.ARM.attributes)
	}
}
