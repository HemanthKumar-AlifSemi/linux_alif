// SPDX-License-Identifier: GPL-2.0
/*
 * ADC_VREF and DAC6 Initialization Driver
 * for Ensemble SoC Analog Devices
 *
 * Copyright (c) 2025 Alif Semiconductor
 * Author: Pankaj Pandey <pankaj.pandey@alifsemi.com>
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/sizes.h>
#include <linux/err.h>

/* Register Definitions */
#define CMP_BASE		0x49023000
#define ANA_BASE		0x1A60A000
#define VBAT_ANA_REG2		0x3C
#define CMP_COMP_REG2		0x004
#define DAC12_VREF_CONT		(0x4 << 17)	/* Program DAC12_VREF_CONT for DAC12 */
#define ANA_PERIPH_BG_ENA	BIT(22)		/* Enable Analog Peripherals LDO */
#define ANA_PERIPH_LDO_EN	BIT(23)

#define ADC_VREF_OFFSET      0x1000
#define ADC_VREF_CONT        BIT(14)
#define ADC_VREF_BUF_EN      BIT(15)
#define DAC6_EN		     BIT(20)
#define DAC6_VREF_SCALE      BIT(27)
#define DAC6_CONT            (0x20U << 21)


/* Stabilization delays */
#define VREF_STAB_DELAY_US   500
#define DAC6_STAB_DELAY_US   200

struct ensemble_adc_dac6 {
	void __iomem *dac6_regs;
	void __iomem *adc_vref_regs;
	struct device *dev;
};

/*
 * Helper function to enable DAC12 reference voltage and analog peripherals
 */
static int enable_dac12_vref_and_analog(struct device *dev)
{
	void __iomem *cmp_base;
	void __iomem *ana_base;
	u32 reg;

	/* Map CMP block to enable DAC12 reference voltage */
	cmp_base = ioremap(CMP_BASE, SZ_64);
	reg = readl(cmp_base + CMP_COMP_REG2);
	reg |= DAC12_VREF_CONT;
	writel(reg, cmp_base + CMP_COMP_REG2);
	iounmap(cmp_base);

	/* Map ANA block to enable analog peripherals */
	ana_base = ioremap(ANA_BASE, SZ_64);
	reg = readl(ana_base + VBAT_ANA_REG2);
	reg |= (ANA_PERIPH_BG_ENA | ANA_PERIPH_LDO_EN);
	writel(reg, ana_base + VBAT_ANA_REG2);
	iounmap(ana_base);
	return 0;
}

static int ensemble_adc_dac6_probe(struct platform_device *pdev)
{
	struct ensemble_adc_dac6 *pdata;
	struct resource *res;
	u32 val;
	int ret;

	/* Allocate and initialize private data */
	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	pdata->dev = &pdev->dev;
	platform_set_drvdata(pdev, pdata);


	/* Map registers */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pdata->dac6_regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pdata->dac6_regs)) {
		ret = PTR_ERR(pdata->dac6_regs);
		dev_err(&pdev->dev, "failed to map registers: %d\n", ret);
		goto err;
	}
	pdata->adc_vref_regs = pdata->dac6_regs + ADC_VREF_OFFSET;

	/* Initialize ADC Voltage Reference */
	val = readl(pdata->adc_vref_regs);
	val |= (ADC_VREF_BUF_EN | ADC_VREF_CONT);
	writel(val, pdata->adc_vref_regs);

	/* Wait for voltage stabilization */
	udelay(VREF_STAB_DELAY_US);

	/* Verify ADC VREF enable */
	val = readl(pdata->adc_vref_regs);
	if (!(val & (ADC_VREF_BUF_EN | ADC_VREF_CONT))) {
		dev_err(&pdev->dev, "ADC VREF buffer failed to enable\n");
		ret = -EIO;
		goto err;
	}

	/* Initialize DAC6 */
	val = readl(pdata->dac6_regs);
	val |= (DAC6_CONT | DAC6_VREF_SCALE | DAC6_EN);
	writel(val, pdata->dac6_regs);

	/* Call helper to enable DAC12 reference and analog peripherals */
	ret = enable_dac12_vref_and_analog(&pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable DAC12 VREF or analog peripherals\n");
		goto err;
	}

	/* Wait for DAC stabilization */
	udelay(DAC6_STAB_DELAY_US);

	dev_info(&pdev->dev, "initialized successfully\n");
	return 0;

err:
	return ret;
}


static const struct of_device_id ensemble_adc_dac6_of_match[] = {
		{ .compatible = "alif,adc-dac6" }
};
MODULE_DEVICE_TABLE(of, ensemble_adc_dac6_of_match);

static struct platform_driver ensemble_adc_dac6_driver = {
		.driver = {
				.name = "alif-adc-dac6",
				.of_match_table = ensemble_adc_dac6_of_match,
		},
		.probe = ensemble_adc_dac6_probe,
};

static int __init ensemble_adc_dac6_init(void)
{
	return platform_driver_register(&ensemble_adc_dac6_driver);
}
postcore_initcall_sync(ensemble_adc_dac6_init);

MODULE_AUTHOR("Pankaj Pandey <pankaj.pandey@alifsemi.com>");
MODULE_DESCRIPTION("Alif Semiconductor Ensemble ADC_VREF/DAC6 Initialization Driver");
MODULE_LICENSE("GPL");
