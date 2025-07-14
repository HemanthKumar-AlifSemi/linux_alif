// SPDX-License-Identifier: GPL-2.0
/*
 * SDHCI reset GPIO driver for Alif Ensemble platforms
 *
 * Copyright (c) 2025 Alif Semiconductor
 * Author: Pankaj Pandey <pankaj.pandey@alifsemi.com>
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of.h>

struct ensemble_sd_reset {
	struct gpio_desc *reset_gpio;
};

static int ensemble_sd_reset_probe(struct platform_device *pdev)
{
	struct ensemble_sd_reset *reset;
	int ret = 0;

	reset = devm_kzalloc(&pdev->dev, sizeof(*reset), GFP_KERNEL);
	if (!reset)
		return -ENOMEM;

	reset->reset_gpio = devm_gpiod_get_optional(&pdev->dev, "reset",
							GPIOD_OUT_HIGH);
	if (IS_ERR(reset->reset_gpio)) {
		ret = PTR_ERR(reset->reset_gpio);
		dev_err(&pdev->dev, "failed to get reset GPIO: %d\n", ret);
		return ret;
	}

	/* Perform the reset sequence if GPIO is available */
	if (reset->reset_gpio) {
		gpiod_set_value(reset->reset_gpio, 0);
		usleep_range(1000, 2000);  // Wait 1-2ms
		gpiod_set_value(reset->reset_gpio, 1);
		usleep_range(2000, 3000);  // Additional wait
	}

	platform_set_drvdata(pdev, reset);
	return 0;
}

static const struct of_device_id ensemble_sd_reset_of_match[] = {
	{ .compatible = "alif,sd-reset" }
};
MODULE_DEVICE_TABLE(of, ensemble_sd_reset_of_match);

static struct platform_driver ensemble_sd_reset_driver = {
	.driver = {
		.name = "alif-sd-reset",
		.of_match_table = ensemble_sd_reset_of_match,
	},
	.probe = ensemble_sd_reset_probe,
};

static int __init ensemble_sd_reset_init(void)
{
	return platform_driver_register(&ensemble_sd_reset_driver);
}
arch_initcall(ensemble_sd_reset_init);

MODULE_AUTHOR("Pankaj Pandey");
MODULE_DESCRIPTION("Ensemble SD Card Reset GPIO Driver");
MODULE_LICENSE("GPL");
