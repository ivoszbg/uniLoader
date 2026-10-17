/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 *
 * Samsung S-Boot integration (j1xlte):
 *  - Stock-zImage-style head: 8x nop sled, branch over the magic at 0x24,
 *    real entry at 0x2C - byte-compatible with what S-Boot expects.
 *  - Earliest diagnostics: solid bars painted into candidate bootloader
 *    framebuffers before anything else can fail (see _start).
 *  - S-Boot loads the payload at its own hardcoded address which we do not
 *    know a priori.  _start migrates the loader image to TEXT_BASE with a
 *    two-phase relocation that can never overwrite executing code:
 *
 *      phase 0: copy the relocation engine ([_start, +ENGINE_SZ)) to its
 *               linked home and continue executing FROM the linked home;
 *      phase 1: from there, copy the rest of the image in two ranges that
 *               exclude the engine home, using the memmove-safe direction
 *               for the global delta.
 *
 *    The embedded kernel/dtb/ramdisk blobs (.payload section) are NOT
 *    copied; their runtime addresses are linked + delta.  The delta is
 *    published in the global `delta_value` so C code can adjust .payload
 *    symbol accesses, and is also handed to main() in r9 for pointers.
 */

#include "asmdefs.h"

#define ENGINE_SZ	0x300

.section .text.header
.globl zimg_header
zimg_header:
	mov	r0, r0			@ 8x nop sled, mirrors stock zImage head
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	b	1f			@ branch over the magic (@0x20 -> 0x40)
	.word	0x016f2818		@ ARM linux zImage magic @0x24
	.word	0			@ preferred load address (unused)
	.space	0x14			@ pad; entry lands on the 64B line @0x40
1:

.section .text.start
.globl _start
.align	2
_start:
	/* Only CPU0 continues; park any secondary cores S-Boot released.
	 * Concurrent entry corrupts everything downstream silently. */
	mrc	p15, 0, r12, c0, c0, 5	/* MPIDR */
	ands	r12, r12, #0x3
	bne	95f

	.arch_extension	virt

	/* If S-Boot dropped us in Hyp mode, ERET down into SVC.
	 * PL1-only CP15 registers (DACR, TTBCR, ...) UNDEF from Hyp,
	 * which would kill the kernel's first MMU setup instruction. */
	mrs	r1, cpsr
	and	r1, r1, #0x1f
	cmp	r1, #0x1a		/* 0b11010 = Hyp? */
	bne	96f			/* SVC: skip the stub, continue */
	mov	r1, #0
	mcr	p15, 4, r1, c1, c1, 0	/* HCR = 0: stop trapping PL1 */
	mvn	r1, #0
	mcr	p15, 4, r1, c1, c1, 2	/* HCPTR: full coprocessor access */
	mov	r1, #0
	mcr	p15, 4, r1, c2, c0, 2	/* HTTBCR = 0 */
	movw	r2, #0x5000		/* keep painting - now in magenta slot */
	movt	r2, #0x57E6
	movw	r3, #0x00FF
	movt	r3, #0xFFFF
	ldr	r1, =entered_hyp
	mov	r3, #1
	str	r3, [r1]
	movw	r2, #0x5000
	movt	r2, #0x57E6
	movw	r3, #0x00FF
	movt	r3, #0xFFFF
	mov	r1, #2400
91:	str	r3, [r2], #4
	subs	r1, r1, #1
	bne	91b
	mov	r1, #0x13		/* SVC | irq+fiq stay masked via SPSR */
	orr	r1, r1, #0xc0
	msr	spsr_cxsf, r1
	adr	r2, 96f			/* SVC continuation */
	msr	elr_hyp, r2
	dsb
	isb
	eret
96:					/* running in SVC now */

	/* ---- earliest possible diagnostics ---------------------------- *
	 * Paint solid bars into candidate bootloader framebuffers BEFORE
	 * any relocation, stack or UART work.  Pure register immediates.
	 *   green @0x57E05000 (vendor DTS bootloader_fb, j1xlte)
	 *   blue  @0x57000000 (universal3475 sibling logo address)
	 *   red   @0x40200000 (paranoid alternate DRAM window)
	 * A visible bar proves execution and identifies the live scanout.
	 * No bar at all => our code never executed.                    */
	movw	r0, #0x8000
	movt	r0, #0x57E1
	movw	r1, #0xFF00
	movt	r1, #0xFF00		/* green 0xFF00FF00 */
	bl	paint
	/* Where did S-Boot actually put us? */
	adr	r7, _start
	movw	r8, #:lower16:_start
	movt	r8, #:upper16:_start
	subs	r9, r7, r8		/* r9 = load_addr - TEXT_BASE */

	/* B: BLUE @fb+0x1B000 = delta==0 (loaded at TEXT_BASE),
	 * MAGENTA = delta!=0 (S-Boot used another address). */
	movw	r0, #0xA000
	movt	r0, #0x57E1
	cmp	r9, #0
	bne	41f
	movw	r1, #0x00FF
	movt	r1, #0xFF00		/* blue 0xFF0000FF */
	b	42f
41:	movw	r1, #0x00FF
	movt	r1, #0xFFFF		/* magenta 0xFFFF00FF */
42:	bl	paint

	/* ---- sanitize CPU state ---- */
	/* C0: TEAL @fb+0x36000 = entering sanitize. */
	movw	r0, #0xC000
	movt	r0, #0x57E1
	movw	r1, #0x8080
	movt	r1, #0xFF00		/* teal 0xFF008080 */
	bl	paint

	cpsid	if
	mrc	p15, 0, r12, c1, c0, 0
	bic	r12, #(1 << 12)
	bic	r12, #(1 << 2 | 1 << 1 | 1 << 0)   @ I|C|M + A(alignment check)
	mcr	p15, 0, r12, c1, c0, 0
	/* NOTE: no c7,c7 / c8,c7 here - reserved on ARMv7, may trap. */

	/* C: CYAN @fb+0x51000 = sanitize survived. */
	movw	r0, #0x6000
	movt	r0, #0x57E5
	movw	r1, #0xFFFF
	movt	r1, #0xFF00		/* cyan 0xFF00FFFF */
	bl	paint

	/* Fresh flag test - paint() above destroyed the old ones. */
	cmp	r9, #0
	beq	5f

	/* I-cache on: keeps the engine alive during its own overlap move. */
	mrc	p15, 0, r12, c1, c0, 0
	orr	r12, r12, #(1 << 12)
	mcr	p15, 0, r12, c1, c0, 0

	/* ---- phase 0: migrate the engine to its linked home ---- */
	add	r1, r7, #ENGINE_SZ	/* src end (actual) */
	add	r0, r8, #ENGINE_SZ	/* dst end (linked home) */
	cmp	r9, #0
	blt	21f
11:	ldr	r3, [r1, #-4]!
	str	r3, [r0, #-4]!
	cmp	r0, r8
	bhi	11b
	b	22f
21:	mov	r0, r8			/* dst start (home) */
	mov	r1, r7			/* src start (actual) */
	add	r2, r0, #ENGINE_SZ
31:	ldr	r3, [r1], #4
	str	r3, [r0], #4
	cmp	r0, r2
	blo	31b
22:
	/* continue inside the migrated engine, at linked addresses */
	adr	r6, eng2
	sub	r6, r6, r9
	bx	r6

/*
 * Engine continuation - placed near the END of the engine so its bytes
 * land at the linked home early in the descending copy order.  Running at
 * the linked home from here on; r8 = linked _start, r9 = delta.
 */
.align	4
eng2:
	movw	r0, #:lower16:__image_start
	movt	r0, #:upper16:__image_start
	movw	r2, #:lower16:__image_end
	movt	r2, #:upper16:__image_end
	add	r4, r8, #ENGINE_SZ	/* engine home end */

	/* publish the delta for C code (.data is at its linked home now) */
	movw	r1, #:lower16:delta_value
	movt	r1, #:upper16:delta_value
	str	r9, [r1]

	cmp	r9, #0
	blt	51f

	/* delta > 0: dst = src - delta => descending per range */
	/* range B: dst [engine_home_end, E) */
	mov	r1, r2
	add	r3, r1, r9
41:	ldr	r5, [r3, #-4]!
	str	r5, [r1, #-4]!
	cmp	r1, r4
	bhi	41b
	/* range A: dst [S, engine_home_start) */
	mov	r1, r8
	add	r3, r1, r9
42:	ldr	r5, [r3, #-4]!
	str	r5, [r1, #-4]!
	cmp	r1, r0
	bhi	42b
	b	60f

	/* delta < 0: dst = src - delta > src => ascending per range */
51:	/* range A: dst [S, engine_home_start) */
	mov	r1, r0
	add	r3, r1, r9
52:	ldr	r5, [r3], #4
	str	r5, [r1], #4
	cmp	r1, r8
	blo	52b
	/* range B: dst [engine_home_end, E) */
	mov	r1, r4
	add	r3, r1, r9
53:	ldr	r5, [r3], #4
	str	r5, [r1], #4
	cmp	r1, r2
	blo	53b

60:	/* image is whole at its linked home; refetch cleanly */
	mov	r12, #0
	mcr	p15, 0, r12, c7, c5, 0	/* invalidate I-cache */
	mcr	p15, 0, r12, c7, c5, 6	/* flush BTB */

	/* ---- common path ---- */
5:	/* D: WHITE @fb+0x11E000 = relocation done (both paths). */
	movw	r0, #0xE000
	movt	r0, #0x57E1
	movw	r1, #0xFFFF
	movt	r1, #0xFFFF
	bl	paint

	mrc	p15, 0, r12, c1, c0, 0
	bic	r12, #(1 << 12)
	bic	r12, #(1 << 2 | 1 << 0)
	mcr	p15, 0, r12, c1, c0, 0

	ldr	sp, =__stack_bottom

	/* Zero BSS */
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0
1:	cmp	r0, r1
	strlo	r2, [r0], #4
	blo	1b

	/* E: GRAY @fb+0x120000 = bss cleared. */
	movw	r0, #0x2000
	movt	r0, #0x57E2
	movw	r1, #0x8080
	movt	r1, #0xFF80
	bl	paint

	/* main(dt, kernel, ramdisk) - .payload addresses adjusted by delta */
	ldr	r0, =_dt_start
	ldr	r1, =_kernel_start
	ldr	r2, =_ramdisk_start
	cmp	r9, #0
	beq	1f
	add	r0, r0, r9
	add	r1, r1, r9
	add	r2, r2, r9
1:	b	main

	b	97f
95:					/* parked secondary */
	wfi
	b	95b
97:

/*
 * paint: fill 24 rows x 640 px of argb pixels.
 * r0 = framebuffer base, r1 = color.  Clobbers r0-r2 only.
 */
paint:
	mov	r2, #1600
71:	str	r1, [r0], #4
	subs	r2, r2, #1
	bne	71b
	bx	lr

/*
 * fb_mark_asm(byte_off, color): paint a bar at FB_BASE + byte_off.
 * C-callable milestone marker (r0/r1 args, r2/r3 clobbered).
 */
ENTRY(fb_mark_asm)
	movw	r2, #0x5000
	movt	r2, #0x57E0
	add	r0, r0, r2		/* r0 is already a FB byte offset */
	b	paint
END(fb_mark_asm)

ENTRY(load_kernel_and_jump)
	mov r1, #0      @ machine type = 0 (device tree)
	mov r0, #0      @ reserved
	bx r3
END(load_kernel_and_jump)

/*
 * jump_to_xnu(u32 entry_pa, u32 boot_args_pa)
 * Enter the XNU locore: r0 = physical boot_args, MMU off, SVC mode.
 */
ENTRY(jump_to_xnu)
	/* r0 = entry, r1 = boot_args -> XNU wants r0 = boot_args */
	mov r3, r0              /* stash entry */
	mov r0, r1              /* r0 = boot_args (physical) */

	/* DIAG: WHITE @fb+0xE4000 = about to bx into the kernel. */
	movw	r1, #0xD000
	movt	r1, #0x57EE
	movw	r2, #0xFFFF
	movt	r2, #0xFFFF
	mov	ip, #1600
91:	str	r2, [r1], #4
	subs	ip, ip, #1
	bne	91b

	mov r1, #0
	mov r2, #0
	mov ip, #0
	mov lr, #0
	bx r3                   /* jump to the kernel entry */
END(jump_to_xnu)

/* Published relocation delta (written by the engine, read by C code). */
.data
.globl delta_value
delta_value:
	.word 0

.globl entered_hyp
entered_hyp:
	.word 0

/* Embedded payloads - NOT part of the relocated loader image. */
.section .payload, "ax", %progbits
.globl _kernel_start
_kernel_start:
	.incbin "blob/Image"
.globl _kernel_end
_kernel_end:

.align	4
.globl _dt_start
_dt_start:
	.incbin "blob/dtb"
.globl _dt_end
_dt_end:

.align	4
.globl _ramdisk_start
_ramdisk_start:
	.incbin "blob/ramdisk"
.globl _ramdisk_end
_ramdisk_end:

	.align	4
	.globl cacheslice_start
 cacheslice_start:
#if defined(CONFIG_XNU_EMBED_CACHESLICE) && CONFIG_XNU_EMBED_CACHESLICE
	.incbin "blob/cacheslice"
#endif
	.globl cacheslice_end
 cacheslice_end:


.align	2
.globl kernel_size
kernel_size:
	.word _kernel_end - _kernel_start

.align	2
.globl dtb_size
dtb_size:
	.word _dt_end - _dt_start

.align	2
.globl ramdisk_size
ramdisk_size:
	.word _ramdisk_end - _ramdisk_start

.align	2
.globl cacheslice_size
cacheslice_size:
	.word cacheslice_end - cacheslice_start
