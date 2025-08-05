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
#include <linux/err.h>

/* Register Definitions */
#define ADC_VREF_OFFSET      0x1000
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
	val |= ADC_VREF_BUF_EN;
	writel(val, pdata->adc_vref_regs);

	/* Wait for voltage stabilization */
	udelay(VREF_STAB_DELAY_US);

	/* Verify ADC VREF enable */
	val = readl(pdata->adc_vref_regs);
	if (!(val & ADC_VREF_BUF_EN)) {
		dev_err(&pdev->dev, "ADC VREF buffer failed to enable\n");
		ret = -EIO;
		goto err;
	}

	/* Initialize DAC6 */
	val = readl(pdata->dac6_regs);
	val |= (DAC6_CONT | DAC6_VREF_SCALE | DAC6_EN);
	writel(val, pdata->dac6_regs);

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
