/* SPDX-License-Identifier: GPL-2.0+
 * cmp.h - Header file of ALIF High-Speed Comparator (CMP)
 *
 * Copyright (c) 2021-2025, Alif Semicondutor
 * Author: Pankaj Pandey <pankaj.pandey@alifsemi.com>
 */
#ifndef __CMP_H
#define __CMP_H

#include <linux/io.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/completion.h>

void __iomem *cmp_base_cmp0;
void __iomem *analog_base_cmp0;

/* CMP0-CMP3 instances */
enum CMP_INSTANCE {
	CMP0_INSTANCE,
	CMP1_INSTANCE,
	CMP2_INSTANCE,
	CMP3_INSTANCE
};

/* Input selection enums */
enum CMP_POS {
	CMP_POS_IN0,
	CMP_POS_IN1,
	CMP_POS_IN2,
	CMP_POS_IN3
};

enum CMP_NEG {
	CMP_NEG_IN0,
	CMP_NEG_IN1,
	CMP_NEG_IN2,
	CMP_NEG_IN3
};

/* Hysteresis levels in mV */
enum CMP_HYST {
	CMP_HYST_0_mV,
	CMP_HYST_6_mV,
	CMP_HYST_12_mV,
	CMP_HYST_18_mV,
	CMP_HYST_24_mV,
	CMP_HYST_30_mV,
	CMP_HYST_36_mV,
	CMP_HYST_42_mV
};

/* Main comparator device structure */
struct cmp_device {
	void __iomem *regs;		/* Register base address */
	struct gpio_desc *gpiod;	/* Input GPIO from comparator output */
	struct gpio_desc *led;		/* Optional LED GPIO */
	struct miscdevice misc;		/* Misc device interface */
	int irq;			/* Interrupt number */
	enum CMP_INSTANCE instance;	/* Instance ID */
	atomic_t counter;		/* Result from interrupt */
	struct mutex lock_m;		/* Mutex for exclusive access */
	spinlock_t lock_s;		/* spinlock for threaded irq handler */
	struct completion completion;	/* For waiting on interrupt */
	const char *name;		/* Device name for instance check */

	/* Configuration parameters */
	u8 pos_input;			/* Positive input source */
	u8 neg_input;			/* Negative input source */
	u8 hysteresis;			/* Hysteresis level */
	u8 filter_taps;			/* Filter taps */
	u8 prescalar;			/* Prescaler value */
	u8 polarity;			/* Output polarity */
};

/* ANA Register */
#define ANA_BASE            0x1A60A000
#define ANA_VBAT_REG2       0x3C

/* CMP Register */
#define CMP_COMP_REG1 0x0
#define CMP_COMP_REG2 0x4
#define CMP_POLARITY_CTRL    (0x08)
#define CMP_WINDOW_CTRL      (0x0C)
#define CMP_FILTER_CTRL      (0x10)
#define CMP_PRESCALER_CTRL   (0x14)
#define CMP_STATUS           (0x18)
#define CMP_INTERRUPT_STATUS (0x20)
#define CMP_INTERRUPT_MASK   (0x24)

/* Bitfield definitions */
#define CMP0_ENABLE  (1U << 28)
#define CMP1_ENABLE  (1U << 29)
#define CMP2_ENABLE  (1U << 30)
#define CMP3_ENABLE  (1U << 31)

#define CMP_FILTER_CONTROL_ENABLE (1U << 0)
#define CMP_PRESCALER_MAX_VALUE   (0x3FU)
#define CMP_POLARITY_MAX_VALUE    (0x2U)
#define CMP_WINDOW_MAX_VALUE      (0x3U)
#define CMP_FILTER_MIN_VALUE      (0x2U)
#define CMP_FILTER_MAX_VALUE      (0x8U)

#define CMP_WINDOW_CONTROL_ENABLE (3U)
#define CMP_INT_STATUS_MASK       (1U)
/* CMP0 macro */
#define CMP0_IN_POS_SEL_POS (0)
#define CMP0_IN_NEG_SEL_POS (2)
#define CMP0_HYST_SEL_POS   (4)

/* CMP1 macro */
#define CMP1_IN_POS_SEL_POS (7)
#define CMP1_IN_NEG_SEL_POS (9)
#define CMP1_HYST_SEL_POS   (11)

/* CMP2 macro */
#define CMP2_IN_POS_SEL_POS (14)
#define CMP2_IN_NEG_SEL_POS (16)
#define CMP2_HYST_SEL_POS   (18)

/* CMP3 macro */
#define CMP3_IN_POS_SEL_POS (21)
#define CMP3_IN_NEG_SEL_POS (23)
#define CMP3_HYST_SEL_POS   (25)

#define CMP_COMP_REG2 0x4

/* REG2 settings for analog control */
#define DAC6_VREF_SCALE      (0x1U << 27)
#define DAC6_CONT            (0x20U << 21)
#define DAC6_EN              (0x1U << 20)
#define DAC12_VREF_CONT      (0x4U << 17)
#define ADC_VREF_BUF_RDIV_EN (0x0U << 16)
#define ADC_VREF_BUF_EN      (0x1U << 15)
#define ADC_VREF_CONT        (0x10U << 10)
#define ANA_PERIPH_LDO_CONT  (0xAU << 6)
#define ANA_PERIPH_BG_CONT   (0xAU << 1)

#define CMP_INT_MASK        (0x01UL)
#define CMP_INTERRUPT_CLEAR (0x01UL)

#define CMP_FILTER_EVENT0_CLEAR (1U << 0)
#define CMP_FILTER_EVENT1_CLEAR (1U << 1)

#define CMP_POLARITY_MASK    BIT(0)  // Single bit (bit 0)
#define CMP_FILTER_TAPS_MASK    GENMASK(3, 0)  // 4-bit field (bits 3:0)
#define CMP_PRESCALAR_MASK    GENMASK(3, 0)  // 4-bit field (bits 3:0)
#define CMP_INTERRUPT_MASK_VAL	0xFFFFFFFF

#endif /* __CMP_H */
