/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 * Copyright (c) 2024 Markuss Broks <markuss.broks@gmail.com>
 */
#include <stdint.h>
#include <string.h>
#include <drivers/framework.h>
#include <drivers/samsung/exynos-speedy.h>

/* SPEEDY Register MAP */
#define SPEEDY_CTRL					0x000
#define SPEEDY_FIFO_CTRL				0x004
#define SPEEDY_CMD					0x008
#define SPEEDY_INT_ENABLE				0x00c
#define SPEEDY_INT_STATUS				0x010
#define SPEEDY_FIFO_STATUS				0x030
#define SPEEDY_TX_DATA					0x034
#define SPEEDY_RX_DATA					0x038
#define SPEEDY_PACKET_GAP_TIME				0x044
#define SPEEDY_TIMEOUT_COUNT				0x048
#define SPEEDY_FIFO_DEBUG				0x100
#define SPEEDY_CTRL_STATUS				0x104

/* SPEEDY_CTRL Register bits */
#define SPEEDY_ENABLE					(1 << 0)
#define SPEEDY_TIMEOUT_CMD_DISABLE			(1 << 1)
#define SPEEDY_TIMEOUT_STANDBY_DISABLE			(1 << 2)
#define SPEEDY_TIMEOUT_DATA_DISABLE			(1 << 3)
#define SPEEDY_ALWAYS_PULLUP_EN				(1 << 7)
#define SPEEDY_DATA_WIDTH_8BIT				(0 << 8)
#define SPEEDY_REMOTE_RESET_REQ				(1 << 30)
#define SPEEDY_SW_RST					(1 << 31)

/* SPEEDY_FIFO_CTRL Register bits */
#define SPEEDY_RX_TRIGGER_LEVEL(x)			((x) << 0)
#define SPEEDY_TX_TRIGGER_LEVEL(x)			((x) << 8)
#define SPEEDY_FIFO_DEBUG_INDEX				(0 << 24)
#define SPEEDY_FIFO_RESET				(1 << 31)

/* SPEEDY_CMD Register bits */
#define SPEEDY_BURST_LENGTH(x)				((x) << 0)
#define SPEEDY_BURST_FIXED				(0 << 5)
#define SPEEDY_BURST_INCR				(1 << 5)
#define SPEEDY_BURST_EXTENSION				(2 << 5)
#define SPEEDY_ACCESS_BURST				(0 << 19)
#define SPEEDY_ACCESS_RANDOM				(1 << 19)
#define SPEEDY_DIRECTION_READ				(0 << 20)
#define SPEEDY_DIRECTION_WRITE				(1 << 20)

/* SPEEDY_INT_ENABLE Register bits */
#define SPEEDY_TRANSFER_DONE_EN				(1 << 0)
#define SPEEDY_TIMEOUT_CMD_EN				(1 << 1)
#define SPEEDY_TIMEOUT_STANDBY_EN			(1 << 2)
#define SPEEDY_TIMEOUT_DATA_EN				(1 << 3)
#define SPEEDY_FIFO_RX_ALMOST_FULL_EN			(1 << 8)
#define SPEEDY_FIFO_TX_ALMOST_EMPTY_EN			(1 << 4)
#define SPEEDY_RX_FIFO_INT_TRAILER_EN			(1 << 9)
#define SPEEDY_RX_MODEBIT_ERR_EN			(1 << 16)
#define SPEEDY_RX_GLITCH_ERR_EN				(1 << 17)
#define SPEEDY_RX_ENDBIT_ERR_EN				(1 << 18)
#define SPEEDY_TX_LINE_BUSY_ERR_EN			(1 << 20)
#define SPEEDY_TX_STOPBIT_ERR_EN			(1 << 21)
#define SPEEDY_REMOTE_RESET_REQ_EN			(1 << 31)

/* SPEEDY_INT_STATUS Register bits */
#define SPEEDY_TRANSFER_DONE				(1 << 0)
#define SPEEDY_TIMEOUT_CMD				(1 << 1)
#define SPEEDY_TIMEOUT_STANDBY				(1 << 2)
#define SPEEDY_TIMEOUT_DATA				(1 << 3)
#define SPEEDY_FIFO_TX_ALMOST_EMPTY			(1 << 4)
#define SPEEDY_FIFO_RX_ALMOST_FULL			(1 << 8)
#define SPEEDY_RX_FIFO_INT_TRAILER			(1 << 9)
#define SPEEDY_RX_MODEBIT_ERR				(1 << 16)
#define SPEEDY_RX_GLITCH_ERR				(1 << 17)
#define SPEEDY_RX_ENDBIT_ERR				(1 << 18)
#define SPEEDY_TX_LINE_BUSY_ERR				(1 << 20)
#define SPEEDY_TX_STOPBIT_ERR				(1 << 21)
#define SPEEDY_REMOTE_RESET_REQ_STAT			(1 << 31)

/* SPEEDY_FIFO_STATUS Register bits */
#define SPEEDY_VALID_DATA_CNT				(0 << 0)
#define SPEEDY_FIFO_FULL				(1 << 5)
#define SPEEDY_FIFO_EMPTY				(1 << 6)

/* SPEEDY_PACKET_GAP_TIME Register bits */
#define SPEEDY_FIFO_TX_ALMOST_EMPTY			(1 << 4)
#define SPEEDY_FSM_INIT					(1 << 1)
#define SPEEDY_FSM_TX_CMD				(1 << 2)
#define SPEEDY_FSM_STANDBY				(1 << 3)
#define SPEEDY_FSM_DATA					(1 << 4)
#define SPEEDY_FSM_TIMEOUT				(1 << 5)
#define SPEEDY_FSM_TRANS_DONE				(1 << 6)
#define SPEEDY_FSM_IO_RX_STAT_MASK			(3 << 7)
#define SPEEDY_FSM_IO_TX_IDLE				(1 << 9)
#define SPEEDY_FSM_IO_TX_GET_PACKET			(1 << 10)
#define SPEEDY_FSM_IO_TX_PACKET				(1 << 11)
#define SPEEDY_FSM_IO_TX_DONE				(1 << 12)

#define SPEEDY_RX_LENGTH(n)				((n) << 0)
#define SPEEDY_TX_LENGTH(n)				((n) << 8)

#define SPEEDY_SLAVE(x)					((x & 0xf) << 15)
#define SPEEDY_ADDRESS(x)				((x & 0xff) << 7)

static int speedy_fifo_reset(unsigned long base)
{
	writel(SPEEDY_FIFO_RESET, (void *)(base + SPEEDY_FIFO_CTRL));
	/* TODO: Implement a proper delay func */
	for (volatile int i = 0; i < 1000; i++);
	return 0;
}

static int speedy_int_clear(unsigned long base)
{
	writel(0xFFFFFFFF, (void *)(base + SPEEDY_INT_STATUS));
	/* TODO: Implement a proper delay func */
	for (volatile int i = 0; i < 1000; i++);
	return 0;
}

int speedy_read(struct speedy_transaction *tr)
{
	int ret = speedy_fifo_reset(tr->base);
	if (ret)
		return ret;

	writel(SPEEDY_RX_LENGTH(1) | SPEEDY_TX_LENGTH(1),
	       (void *)(tr->base + SPEEDY_FIFO_CTRL));

	unsigned int cmd = SPEEDY_ACCESS_RANDOM | SPEEDY_DIRECTION_READ |
			   SPEEDY_SLAVE(tr->slave) | SPEEDY_ADDRESS(tr->offset);

	writel(SPEEDY_TRANSFER_DONE | SPEEDY_FIFO_RX_ALMOST_FULL_EN |
	       SPEEDY_RX_FIFO_INT_TRAILER_EN | SPEEDY_RX_MODEBIT_ERR_EN |
	       SPEEDY_RX_GLITCH_ERR_EN | SPEEDY_RX_ENDBIT_ERR_EN |
	       SPEEDY_REMOTE_RESET_REQ_EN,
	       (void *)(tr->base + SPEEDY_INT_ENABLE));

	speedy_int_clear(tr->base);

	writel(cmd, (void *)(tr->base + SPEEDY_CMD));

	// Poll for completion
	int timeout = 500000;
	while (timeout-- > 0) {
		unsigned int int_status;
		int_status = readl((volatile uint32_t *)(tr->base +
							 SPEEDY_INT_STATUS));
		if (int_status & SPEEDY_TRANSFER_DONE) {
			speedy_int_clear(tr->base);
			tr->val = readl((volatile uint32_t *)
					(tr->base + SPEEDY_RX_DATA));
			return 0;
		}
	}

	return -1;
}

int speedy_write(struct speedy_transaction *tr)
{
	int ret = speedy_fifo_reset(tr->base);
	if (ret)
		return ret;

	writel(SPEEDY_RX_LENGTH(1) | SPEEDY_TX_LENGTH(1),
	       (void *)(tr->base + SPEEDY_FIFO_CTRL));

	unsigned int cmd = SPEEDY_ACCESS_RANDOM | SPEEDY_DIRECTION_WRITE |
			   SPEEDY_SLAVE(tr->slave) | SPEEDY_ADDRESS(tr->offset);

	writel(SPEEDY_TRANSFER_DONE_EN | SPEEDY_FIFO_TX_ALMOST_EMPTY_EN |
	       SPEEDY_TX_LINE_BUSY_ERR_EN | SPEEDY_TX_STOPBIT_ERR_EN |
	       SPEEDY_REMOTE_RESET_REQ_EN,
	       (void *)(tr->base + SPEEDY_INT_ENABLE));

	speedy_int_clear(tr->base);

	writel(cmd, (void *)(tr->base + SPEEDY_CMD));
	writel(tr->val, (void *)(tr->base + SPEEDY_TX_DATA));

	// Poll for completion
	int timeout = 500000;
	while (timeout-- > 0) {
		unsigned int int_status;
		int_status = readl((volatile uint32_t *)(tr->base +
							 SPEEDY_INT_STATUS));
		if (int_status & SPEEDY_TRANSFER_DONE) {
			speedy_int_clear(tr->base);
			return 0;
		}
	}

	return -1;
}
