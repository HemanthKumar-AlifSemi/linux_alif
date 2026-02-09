/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright 2018-2020 Broadcom.
 */

#ifndef __UAPI_LINUX_CH201_H
#define __UAPI_LINUX_CH201_H

#include <linux/ioctl.h>
#include <linux/types.h>

/* I2C transfer with pointer-based buffer
 * Driver handles chunking internally for large firmware writes
 */
struct ch201_i2c_xfer {
	__u16 addr;		/* I2C address */
	__u16 flags;		/* I2C flags (I2C_M_RD for read) */
	__u16 len;		/* Total length of data to transfer */
	__u64 buf_ptr;		/* Pointer to userspace buffer */
	__u8 mem_read;		/* Non-zero if memory read (register address first) */
};

#define CH201_IOC_MAGIC				'C'
#define CH201_IOC_I2C_XFER			_IOWR(CH201_IOC_MAGIC, 1, struct ch201_i2c_xfer)
#define CH201_IOC_RESET_ASSERT			_IO(CH201_IOC_MAGIC, 2)
#define CH201_IOC_RESET_RELEASE			_IO(CH201_IOC_MAGIC, 3)
#define CH201_IOC_WAIT_IRQ			_IOW(CH201_IOC_MAGIC, 4, __u16)
#define CH201_IOC_PROG_SET			_IO(CH201_IOC_MAGIC, 5)
#define CH201_IOC_PROG_CLEAR			_IO(CH201_IOC_MAGIC, 6)
#define CH201_IOC_INT1_SET			_IO(CH201_IOC_MAGIC, 7)
#define CH201_IOC_INT1_CLEAR			_IO(CH201_IOC_MAGIC, 8)
#define CH201_IOC_INT1_DIR_IN			_IO(CH201_IOC_MAGIC, 9)
#define CH201_IOC_INT1_DIR_OUT			_IO(CH201_IOC_MAGIC, 10)
#define CH201_IOC_IRQ_ENABLE			_IO(CH201_IOC_MAGIC, 11)
#define CH201_IOC_IRQ_DISABLE			_IO(CH201_IOC_MAGIC, 12)

/* Chunk size for I2C write transfers from userspace */
#define CH201_I2C_WRITE_CHUNK_SIZE		256

/* Set according to the soniclibrary BSP layer settings */
#define CH201_REG_ADDR_SIZE			1

#endif /*__UAPI_LINUX_CH201_H*/
