/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Register definitions for Alif Semiconductor UTIMER IP
 *
 * Copyright (C) 2021-2025 Alif Semiconductor
 */
#ifndef __ALIF_UTIMER_REGISTERS_H
#define __ALIF_UTIMER_REGISTERS_H
#include <linux/bits.h>
/**
 * @name Global Configuration
 * @{
 */
#define UTIMER_OFFSET			0x1000	/**< Offset between channel register blocks */
#define UT_NUM_COUNTERS			12	/**< Number of counter channels */
#define MAX_INTERRUPTS			96	/**< Maximum number of interrupts */
/** @} */
/**
 * @name Default Values
 * @{
 */
#define DRIVER_OUT_ENABLE		0xffffffff	/**< Driver output enable mask */
#define UTIMER_CLK_ENABLE		0xffff		/**< Clock enable mask */
#define UTIMER_CLK_DISABLE		0x0		/**< Clock disable value */
#define DEFAULT_COUNTER_STATUS		0		/**< Initial counter status */
#define DEFAULT_ELAPSED_TIME		0		/**< Initial elapsed time */
/** @} */
/**
 * @name Channel Registers
 * @{
 */
#define UT_CNTR_CTRL			0x80		/**< Channel Control Register offset */
/* UT_CNTR_CTRL bits */
#define CNTR_EN				BIT(0)		/**< Counter enable bit */
#define CNTR_START			BIT(1)		/**< Counter start trigger */
#define CNTR_TYPE_SHIFT			2		/**< Counter mode field shift */
#define CNTR_DIR			BIT(8)		/**< Count direction (0=up, 1=down) */
#define CNTR_TYPE_MASK			0x7		/**< Counter mode bitmask */
#define UT_START_1_SRC			0x4		/**< Start source config register */
#define UT_STOP_1_SRC			0xc		/**< Stop source config register */
#define UT_CLEAR_1_SRC			0x14		/**< Clear source config register */
#define UT_CNTR				0xA0		/**< Current counter value register */
#define UT_CNTR_PTR			0xA4		/**< Counter compare/pointer register */
#define UT_CHAN_STATUS			0x114		/**< Channel status register */
#define UT_CHAN_INT			0x118		/**< Channel interrupt status register */
#define UT_CHAN_INT_MASK		0x11c		/**< Channel interrupt mask register */
/** @} */
/**
 * @name Global Registers
 * @{
 */
#define UT_GLB_CNTR_START		0x0		/**< Global counter start control */
#define UT_GLB_CNTR_STOP		0x4		/**< Global counter stop control */
#define UT_GLB_CNTR_CLEAR		0x8		/**< Global counter clear control */
#define UT_GLB_CNTR_RUNNING		0xc		/**< Global running status register */
#define UT_GLB_DRIVER_OEN		0x10		/**< Global driver output enable */
#define UT_GLB_DRIVER_CLK_ENABLE	0x20		/**< Global clock enable control */
/** @} */
/**
 * @name Interrupt Definitions
 * @{
 */
#define CHAN_INTERRUPT_OVER_FLOW_BIT	7		/**< Overflow interrupt bit position */
#define CHAN_INTERRUPT_OVER_FLOW	BIT(7)		/**< Overflow interrupt mask */
#define CHAN_INTERRUPT_UNDER_FLOW_BIT	6		/**< Underflow interrupt bit position */
#define CHAN_INTERRUPT_UNDER_FLOW	BIT(6)		/**< Underflow interrupt mask */
/** @} */
/**
 * @name Source Programming
 * @{
 */
#define CNTR_SRC1_PGM_EN_BIT		31		/**< Source programming enable bit */
#define CNTR_SRC1_PGM_EN		BIT(31)		/**< Source programming enable */
#define CNTR_SRC1_PGM_DISABLE		0x0		/**< Disable source programming */
/** @} */
#endif /* __ALIF_UTIMER_REGISTERS_H */
