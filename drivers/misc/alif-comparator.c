// SPDX-License-Identifier: GPL-2.0+
/*
 * alif-comparator.c - Driver for ALIF High-Speed Comparator (CMP)
 *
 * The comparator is a rail-to-rail, multi-input analog comparator with
 * programmable reference voltage and hysteresis.
 *
 * Copyright (c) 2021-2025, Alif Semicondutor
 * Author: Pankaj Pandey <pankaj.pandey@alifsemi.com>
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mod_devicetable.h>
#include <linux/gpio/consumer.h>
#include <linux/spinlock.h>
#include <linux/bitfield.h>
#include "cmp.h"

#define DRIVER_NAME "cmp-module"
#define CMP_MAX_INSTANCES 4
#define DEFAULT_FILTER_TAPS 5
#define DEFAULT_PRESCALAR 8
#define DEFAULT_POLARITY 0
#define TIMEOUT_MS 1000

static struct gpio_desc *shared_led;
static uint32_t data_diff;

/* Forward declarations */
static int cmp_start(struct cmp_device *cmp);
static void cmp_set_config(struct cmp_device *cmp);
static int cmp_enable(struct cmp_device *cmp);

static void cmp_analog_config(void)
{
	void __iomem *va_base;
	u32 val;

	/* Enable LDO and BG for the ANALOG */
	va_base = ioremap(ANA_BASE, SZ_64);
	if (!va_base) {
		pr_err("Failed to map ANA_BASE\n");
		return;
	}

	val = readl(va_base + ANA_VBAT_REG2);
	val |= (BIT(22) | BIT(23));
	writel(val, va_base + ANA_VBAT_REG2);
	iounmap(va_base);

	val = ioread32(cmp_base_cmp0 + CMP_COMP_REG2);
	val |= (DAC6_VREF_SCALE | DAC6_CONT | DAC6_EN | DAC12_VREF_CONT | ADC_VREF_BUF_RDIV_EN
		| ADC_VREF_BUF_EN | ADC_VREF_CONT | ANA_PERIPH_LDO_CONT | ANA_PERIPH_BG_CONT);
	writel(val, cmp_base_cmp0 + CMP_COMP_REG2);
}

static void cmp_set_polarity(struct cmp_device *cmp)
{
	u32 val;

	val = readl(cmp->regs + CMP_POLARITY_CTRL);
	val |= FIELD_PREP(CMP_POLARITY_MASK, cmp->polarity);
	writel(val, cmp->regs + CMP_POLARITY_CTRL);
}

static void cmp_set_filter(struct cmp_device *cmp)
{
	u32 val;

	val = readl(cmp->regs + CMP_FILTER_CTRL);
	val |= CMP_FILTER_CONTROL_ENABLE |
			FIELD_PREP(CMP_FILTER_TAPS_MASK, cmp->filter_taps);
	writel(val, cmp->regs + CMP_FILTER_CTRL);
}

static void cmp_set_prescalar(struct cmp_device *cmp)
{
	u32 val;

	val = readl(cmp->regs + CMP_PRESCALER_CTRL);
	val |= FIELD_PREP(CMP_PRESCALAR_MASK, cmp->prescalar);
	writel(val, cmp->regs + CMP_PRESCALER_CTRL);
}

static void cmp_enable_interrupt(struct cmp_device *cmp)
{
	/* Unmask all interrupts */
	writel(0x0, cmp->regs + CMP_INTERRUPT_MASK);
}

static void cmp_set_config(struct cmp_device *cmp)
{
	void __iomem *regs = cmp_base_cmp0;

	switch (cmp->instance) {

	case CMP0_INSTANCE:
		data_diff |= cmp->pos_input << CMP0_IN_POS_SEL_POS |
		cmp->neg_input << CMP0_IN_NEG_SEL_POS |
		cmp->hysteresis << CMP0_HYST_SEL_POS;
		break;

	case CMP1_INSTANCE:
		data_diff |= cmp->pos_input << CMP1_IN_POS_SEL_POS |
		cmp->neg_input  << CMP1_IN_NEG_SEL_POS |
		cmp->hysteresis << CMP1_HYST_SEL_POS;
		break;

	case CMP2_INSTANCE:
		data_diff |= cmp->pos_input << CMP2_IN_POS_SEL_POS |
		cmp->neg_input  << CMP2_IN_NEG_SEL_POS |
		cmp->hysteresis << CMP2_HYST_SEL_POS;
		break;

	case CMP3_INSTANCE:
		data_diff |= cmp->pos_input << CMP3_IN_POS_SEL_POS |
		cmp->neg_input  << CMP3_IN_NEG_SEL_POS |
		cmp->hysteresis << CMP3_HYST_SEL_POS;
		break;
	}

	writel(data_diff, regs + CMP_COMP_REG1);
}

static int cmp_enable(struct cmp_device *cmp)
{
	void __iomem *regs = cmp_base_cmp0;
	const u32 enable_mask[] = {
		CMP0_ENABLE,
		CMP1_ENABLE,
		CMP2_ENABLE,
		CMP3_ENABLE
	};

	data_diff = readl(regs);

	if (cmp->instance >= CMP_MAX_INSTANCES)
		return -EINVAL;

	data_diff |= enable_mask[cmp->instance];
	writel(data_diff, regs);
	return 0;
}

static int cmp_start(struct cmp_device *cmp)
{
	int ret;

	/* Configure comparator settings */
	cmp_set_config(cmp);

	/* Set polarity control */
	cmp_set_polarity(cmp);

	/* Configure filter */
	cmp_set_filter(cmp);

	/* Set prescalar */
	cmp_set_prescalar(cmp);

	/* Enable interrupts */
	cmp_enable_interrupt(cmp);

	/* Enable the comparator */
	ret = cmp_enable(cmp);
	if (ret < 0)
		return ret;

	return 0;
}

static irqreturn_t alif_cmp_isr(int irq, void *dev_id)
{
	struct cmp_device *cmp = dev_id;
	uint8_t int_status = readl(cmp->regs + CMP_INTERRUPT_STATUS) & CMP_INT_STATUS_MASK;

	/* Clear interrupt flags (Top Half) */
	if (int_status == CMP_FILTER_EVENT0_CLEAR)
		writel(CMP_FILTER_EVENT0_CLEAR, cmp->regs + CMP_INTERRUPT_STATUS);
	else if (int_status == CMP_FILTER_EVENT1_CLEAR)
		writel(CMP_FILTER_EVENT1_CLEAR, cmp->regs + CMP_INTERRUPT_STATUS);

	/* Let the threaded handler process the rest */
	return IRQ_WAKE_THREAD;
}

static irqreturn_t alif_cmp_threaded_isr(int irq, void *dev_id)
{
	struct cmp_device *cmp = dev_id;
	unsigned long flags;
	int status = gpiod_get_value(cmp->gpiod);

	/* Process GPIO (Bottom Half) */
	if (status <= 1) {
		spin_lock_irqsave(&cmp->lock_s, flags);
		atomic_set(&cmp->counter, status);
		spin_unlock_irqrestore(&cmp->lock_s, flags);
	} else {
		return IRQ_NONE;
	}

	/* Notify userspace (e.g., sysfs, completion) */
	complete(&cmp->completion);

	return IRQ_HANDLED;
}

/* Sysfs attribute functions */
static ssize_t status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);
	int val = atomic_read(&cmp->counter);

	if (val < 0)
		return val;

	dev_info(dev, "%s\n", val ? "+Ve > -Ve" : "-Ve > +Ve");
	return sysfs_emit(buf, "%d\n", val);
}

static ssize_t status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	atomic_set(&cmp->counter, -1);
	ret = kstrtoul(buf, 0, &value);
	if (ret < 0 || value > 1)
		return -EINVAL;

	mutex_lock(&cmp->lock_m);
	gpiod_set_value(cmp->led, value);
	mutex_unlock(&cmp->lock_m);

	ret = wait_for_completion_interruptible_timeout(&cmp->completion,
			msecs_to_jiffies(TIMEOUT_MS));
	if (ret < 0) {
		gpiod_set_value(cmp->led, 0);
		return ret;
	} else if (ret == 0) {
		gpiod_set_value(cmp->led, 0);
		return -ETIMEDOUT;
	}

	reinit_completion(&cmp->completion);
	dev_info(dev, "LED %s\n", value ? "ON" : "OFF");

	return count;
}

static ssize_t pos_input_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", cmp->pos_input);
}

static ssize_t pos_input_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);
	u8 val;
	int ret;

	ret = kstrtou8(buf, 0, &val);
	if (ret)
		return ret;

	cmp->pos_input = val;
	dev_info(dev, "Positive input set to %u\n", val);
	return count;
}

static ssize_t neg_input_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", cmp->neg_input);
}

static ssize_t neg_input_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	cmp->neg_input = val;
	dev_info(dev, "Negative input set to %u\n", val);
	return count;
}

static ssize_t hysteresis_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", cmp->hysteresis);
}

static ssize_t hysteresis_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	cmp->hysteresis = val;
	dev_info(dev, "Hysteresis input set to %u\n", val);
	return count;
}

static ssize_t start_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct cmp_device *cmp = dev_get_drvdata(dev);

	cmp_start(cmp);
	dev_info(dev, "Comparator started %u\n", count);

	return count;
}


/* Similar show/store functions for position, neg_input and hysteresis, etc. */
static DEVICE_ATTR_RW(status);
static DEVICE_ATTR_RW(pos_input);
static DEVICE_ATTR_RW(neg_input);
static DEVICE_ATTR_RW(hysteresis);
static DEVICE_ATTR_WO(start);

static struct attribute *cmp_attrs[] = {
		&dev_attr_status.attr,
		&dev_attr_start.attr,
		&dev_attr_pos_input.attr,
		&dev_attr_neg_input.attr,
		&dev_attr_hysteresis.attr,
		NULL
};

static const struct attribute_group cmp_attr_group = {
		.attrs = cmp_attrs,
};

static void cmp_init_defaults(struct cmp_device *cmp)
{
	cmp->pos_input = CMP_POS_IN0;
	cmp->neg_input = CMP_NEG_IN3;
	cmp->hysteresis = CMP_HYST_42_mV;
	cmp->filter_taps = DEFAULT_FILTER_TAPS;
	cmp->prescalar = DEFAULT_PRESCALAR;
	cmp->polarity = DEFAULT_POLARITY;
	mutex_init(&cmp->lock_m);
	init_completion(&cmp->completion);
	spin_lock_init(&cmp->lock_s);
}

static int cmp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cmp_device *cmp;
	int ret;

	cmp = devm_kzalloc(dev, sizeof(*cmp), GFP_KERNEL);
	if (!cmp)
		return -ENOMEM;

	cmp->regs = devm_platform_ioremap_resource_byname(pdev, "cmp_reg");
	if (IS_ERR(cmp->regs))
		return PTR_ERR(cmp->regs);

	cmp->gpiod = devm_gpiod_get(dev, "cmp", GPIOD_IN);
	if (IS_ERR(cmp->gpiod))
		return PTR_ERR(cmp->gpiod);

	if (!shared_led) {
		shared_led = devm_gpiod_get(dev, "led", GPIOD_OUT_LOW);
		if (IS_ERR(shared_led))
			return PTR_ERR(shared_led);
	}
	cmp->led = shared_led;

	cmp->irq = platform_get_irq(pdev, 0);
	if (cmp->irq < 0)
		return cmp->irq;

	of_property_read_u32(dev->of_node, "instance", &cmp->instance);
	if (cmp->instance >= CMP_MAX_INSTANCES) {
		dev_err(dev, "Invalid instance number\n");
		return -EINVAL;
	}

	cmp->name = dev_name(dev);
	cmp_init_defaults(cmp);

	/* Instance-specific setup */
	if (cmp->instance == CMP0_INSTANCE) {
		cmp_base_cmp0 = cmp->regs;

		cmp->misc = (struct miscdevice){
			.minor = MISC_DYNAMIC_MINOR,
			.name = "cmp-dev",
			.mode = 0644,	/* Restricted access */
			.parent = dev,
		};
		ret = misc_register(&cmp->misc);
		if (ret) {
			dev_err(dev, "Failed to register misc device: %d\n", ret);
			return ret;
		}
	}

	ret = devm_request_threaded_irq(&pdev->dev, cmp->irq, alif_cmp_isr,
			alif_cmp_threaded_isr, IRQF_ONESHOT, DRIVER_NAME,
			cmp);

	if (ret < 0) {
		dev_err(&pdev->dev, "failed requesting threaded interrupt\n");
		goto err_misc;
	}

	ret = sysfs_create_group(&dev->kobj, &cmp_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs group: %d\n", ret);
		goto err_irq;
	}

	platform_set_drvdata(pdev, cmp);

	/* Initialize hardware */
	cmp_analog_config();
	ret = cmp_start(cmp);
	if (ret) {
		dev_err(dev, "Failed to start comparator hardware: %d\n", ret);
		goto err_hw;
	}

	dev_info(dev, "ALIF CMP driver loaded for IRQ %d\n", cmp->irq);
	return 0;

err_hw:
	/* Disable comparator hardware */
	{
		u32 val = readl(cmp_base_cmp0);

		switch (cmp->instance) {
		case CMP0_INSTANCE:
			val &= ~CMP0_ENABLE;
			break;
		case CMP1_INSTANCE:
			val &= ~CMP1_ENABLE;
			break;
		case CMP2_INSTANCE:
			val &= ~CMP2_ENABLE;
			break;
		case CMP3_INSTANCE:
			val &= ~CMP3_ENABLE;
			break;
		}
		writel(val, cmp_base_cmp0);

		/* Mask all interrupts */
		writel(CMP_INTERRUPT_MASK_VAL, cmp_base_cmp0 + CMP_INTERRUPT_MASK);
		sysfs_remove_group(&dev->kobj, &cmp_attr_group);
	}
err_irq:
err_misc:
	if (cmp->instance == CMP0_INSTANCE)
		misc_deregister(&cmp->misc);

	return ret;
}

static void cmp_remove(struct platform_device *pdev)
{
	struct cmp_device *cmp = platform_get_drvdata(pdev);

	sysfs_remove_group(&pdev->dev.kobj, &cmp_attr_group);

	if (cmp->instance == CMP0_INSTANCE)
		misc_deregister(&cmp->misc);
}

static const struct of_device_id cmp_of_match[] = {
		{ .compatible = "alif,cmp-module" },
		{},
};
MODULE_DEVICE_TABLE(of, cmp_of_match);

static struct platform_driver cmp_driver = {
		.probe = cmp_probe,
		.remove_new = cmp_remove,
		.driver = {
				.name = DRIVER_NAME,
				.of_match_table = cmp_of_match,
		},
};

module_platform_driver(cmp_driver);

MODULE_AUTHOR("Pankaj Pandey <pankaj.pandey@alifsemi.com>");
MODULE_DESCRIPTION("ALIF High-Speed Comparator Driver");
MODULE_LICENSE("GPL");
