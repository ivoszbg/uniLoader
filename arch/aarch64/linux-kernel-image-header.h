/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Derived info from arch/arm64/kernel/head.S in the Linux kernel.
 */
#ifndef _AARCH64_LINUX_KERNEL_IMAGE_HEADER_H
#define _AARCH64_LINUX_KERNEL_IMAGE_HEADER_H

/*
 * Image header flags (Documentation/arm64/booting.rst):
 * Bit 0 Kernel endianness.
	1 if BE, 0 if LE.
 * Bit 1-2 Kernel Page size.
	0 - Unspecified.
	1 - 4K
	2 - 16K
	3 - 64K
 * Bit 3 Kernel physical placement
	0
	  2MB aligned base should be as close as possible
	  to the base of DRAM, since memory below it is not
	  accessible via the linear mapping
	1
	  2MB aligned base such that all image_size bytes
	  counted from the start of the image are within
	  the 48-bit addressable range of physical memory
 */
#define LINUX_IMAGE_FLAGS	((1 << 1) | (1 << 3))

	.macro linux_image_header entry
	/*
	 * DO NOT MODIFY. Image header expected by Linux boot-loaders.
	 */
	b	\entry
	.long	0				/* reserved */
	.quad	0				/* text_offset */
	.quad	_kernel_size			/* image_size, see linker script */
	.quad	LINUX_IMAGE_FLAGS		/* flags */
	.quad	0				/* reserved */
	.quad	0				/* reserved */
	.quad	0				/* reserved */
	.ascii	"ARM\x64"			/* magic number */
	.long	0				/* reserved */
	.endm

#endif /* _AARCH64_LINUX_KERNEL_IMAGE_HEADER_H */
