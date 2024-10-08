/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)

INPUT(KERNEL_PATH)
INPUT(DTB_PATH)

SECTIONS
{
	.boot : {
		arch/aarch64/start.o
	}

	.text ALIGN(4096) : {
		*(.text)
	}

	.bss ALIGN(4096) : {
		_bss_start = .;
		. = . + 4096;
		_bss_end = .;
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

	kernel_size = SIZEOF(.kernel);
}
