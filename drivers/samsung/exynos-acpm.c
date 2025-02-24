/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <drivers/samsung/exynos-acpm.h>

void acpm_ipc_send_data_async(void *reg_base, unsigned int ipc_ch, unsigned int cmd0, unsigned int cmd1)
{
	unsigned int acpm_ipc = 0;

	acpm_ipc = (1 << ipc_ch) << INTR_FLAG_OFFSET;

	writel(cmd0, reg_base + SR0);
	writel(cmd1, reg_base + SR1);
	__asm__ volatile ("dsb	sy");
	writel(acpm_ipc, reg_base + INTGR0);
}

void acpm_ipc_send_data_wait(void *reg_base, unsigned int ipc_ch, unsigned int *cmd0, unsigned int *cmd1)
{
	unsigned int reg = 0;

	do  {
		reg = readl(reg_base + INTSR1) & (0x1 << ipc_ch);
	} while (!(reg));

	writel(0x1 << ipc_ch, reg_base + INTCR1);

	*cmd0 = readl(reg_base + SR2);
	*cmd1 = readl(reg_base + SR3);
}

void spmi_read(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, unsigned char *dest)
{
	unsigned int command[4] = {0,};
	int ret = 0;

	command[0] = set_protocol(type, TYPE) | set_protocol(reg, REG);
	command[1] = set_protocol(FUNC_READ, FUNC) | set_protocol(sid, SID);

	acpm_ipc_send_data_async(reg_base, IPC_AP_SPMI, command[0], command[1]);

	acpm_ipc_send_data_wait(reg_base, IPC_AP_SPMI, &command[2], &command[3]);

	*dest = read_protocol(command[3], DEST);
	ret = read_protocol(command[3], RETURN);
	if (ret) {
		;
	}
}

void spmi_bulk_read(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, int count, unsigned char *buf)
{
	unsigned int command[6] = {0,};
	int i, ret;

	command[0] = set_protocol(type, TYPE) | set_protocol(reg, REG);
	command[1] = set_protocol(FUNC_BULK_READ, FUNC) | set_protocol(count, CNT) | set_protocol(sid, SID);

	acpm_ipc_send_data_async(reg_base, IPC_AP_SPMI, command[0], command[1]);

	acpm_ipc_send_data_wait(reg_base, IPC_AP_SPMI, &command[2], &command[3]);

	ret = read_protocol(command[3], RETURN);
	if (ret)
		return;

	command[4] = readl(reg_base + SR0);
	command[5] = readl(reg_base + SR1);

	for (i = 0; i < count; i++) {
		if (i < 4)
			buf[i] = read_bulk_protocol(command[4], BULK_VAL, i);
		else
			buf[i] = read_bulk_protocol(command[5], BULK_VAL, i - 4);
	}
}

void spmi_write(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, unsigned char value)
{
	unsigned int command[4] = {0,};
	int ret = 0;

	command[0] = set_protocol(type, TYPE) | set_protocol(reg, REG);
	command[1] = set_protocol(FUNC_WRITE, FUNC) | set_protocol(value, WRITE_VAL) | set_protocol(sid, SID);


	acpm_ipc_send_data_async(reg_base, IPC_AP_SPMI, command[0], command[1]);

	acpm_ipc_send_data_wait(reg_base, IPC_AP_SPMI, &command[2], &command[3]);

	ret = read_protocol(command[3], RETURN);
	if (ret) {
		;
	}
}

void spmi_bulk_write(void *reg_base, unsigned char sid, unsigned char type, unsigned char reg, int count, unsigned char *buf)
{
	unsigned int command[4] = {0,};
	int ret = 0, i;

	command[0] = set_protocol(type, TYPE) | set_protocol(reg, REG);
	command[1] = set_protocol(FUNC_BULK_WRITE, FUNC) | set_protocol(count, CNT) | set_protocol(sid, SID);

	for (i = 0; i < count; i++) {
		if (i < 4)
			command[2] |= set_bulk_protocol(buf[i], BULK_VAL, i);
		else
			command[3] |= set_bulk_protocol(buf[i], BULK_VAL, i - 4);
	}

	writel(command[2], reg_base + SR2);
	writel(command[3], reg_base + SR3);

	acpm_ipc_send_data_async(reg_base, IPC_AP_SPMI, command[0], command[1]);

	acpm_ipc_send_data_wait(reg_base, IPC_AP_SPMI, &command[2], &command[3]);

	ret = read_protocol(command[3], RETURN);
	if (ret) {
		;
	}
}
