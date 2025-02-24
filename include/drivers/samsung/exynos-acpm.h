/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (c) 2025 Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 *
 * Bindings for exynos-acpm
 */

#ifndef EXYNOS_ACPM_H_
#define EXYNOS_ACPM_H_

#define INTGR0						(0x0008)
#define INTCR1						(0x0020)
#define INTSR1						(0x0028)
#define SR0						(0x100)
#define SR1						(0x104)
#define SR2						(0x108)
#define SR3						(0x10C)
#define SR4						(0x110)
#define SR5						(0x114)

#define IPC_AP_SPMI					(8)
#define INTR_FLAG_OFFSET				(16)

void spmi_read(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, unsigned char *dest);
void spmi_write(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, unsigned char value);
void spmi_bulk_read(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, int count, unsigned char *buf);
void spmi_bulk_write(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, int count, unsigned char *buf);

/* Define shift */
#define SHIFT_BYTE			(8)
#define SHIFT_REG			(0)
#define SHIFT_TYPE			(8)
#define SHIFT_FUNC			(0)
#define SHIFT_DEST			(8)
#define SHIFT_CNT			(8)
#define SHIFT_WRITE_VAL			(8)
#define SHIFT_UPDATE_VAL		(8)
#define SHIFT_UPDATE_MASK		(16)
#define SHIFT_RETURN			(24)
#define SHIFT_BULK_VAL   		SHIFT_BYTE
#define SHIFT_SID			(24)

/* Define mask */
#define MASK_BYTE			(0xFF)
#define MASK_REG			MASK_BYTE
#define MASK_TYPE			MASK_BYTE
#define MASK_FUNC       		MASK_BYTE
#define MASK_DEST			MASK_BYTE
#define MASK_CNT			MASK_BYTE
#define MASK_WRITE_VAL			MASK_BYTE
#define MASK_UPDATE_VAL			MASK_BYTE
#define MASK_UPDATE_MASK		MASK_BYTE
#define MASK_RETURN			MASK_BYTE
#define MASK_BULK_VAL 			MASK_BYTE
#define MASK_SID			(0xF)

/* Command */
#define set_protocol(data, protocol) ((data & MASK_##protocol) << SHIFT_##protocol)
#define set_bulk_protocol(data, protocol, n)            ((data & MASK_##protocol) << (SHIFT_##protocol * (n)))
#define read_protocol(data, protocol) ((data >> SHIFT_##protocol) & MASK_##protocol)
#define read_bulk_protocol(data, protocol, n)           ((data >> (SHIFT_##protocol * (n))) & MASK_##protocol)

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

enum mfd_func {
        FUNC_READ,
        FUNC_WRITE,
        FUNC_UPDATE,
        FUNC_BULK_READ,
        FUNC_BULK_WRITE,
};

#endif /* EXYNOS_ACPM_H_ */
