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
		asm/Start.o
	}

	.dtb ALIGN(0x1000) : {
		dtb = .;
		DTB_PATH
	}

	.kernel ALIGN(0x1000) : {
		kernel = .;
		KERNEL_PATH
	}
	end = .;

	kernel_size = SIZEOF(.kernel);
}
