// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Alif Semiconductor Ensemble Cortex-A32 CPUFreq Driver
 *
 * Copyright (C) 2026 Alif Semiconductor Pankaj Pandey <pankaj.pandey@alifsemi.com>
 *
 * This driver implements CPU frequency scaling for the Alif Ensemble
 * E-series SoCs featuring dual Cortex-A32 cores. Frequency scaling is
 * achieved via the Linux clock framework, which programs the HOSTCPUCLK
 * divider register. This ensures clk_summary reflects the actual CPU
 * frequency.
 *
 * No voltage scaling or power management firmware is required.
 */

#include <linux/clk.h>
#include <linux/clk/ensemble-clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#define ENSEMBLE_TRANSITION_LATENCY_NS (100 * NSEC_PER_MSEC)
/*
 * Frequency table in kHz.
 * driver_data stores the frequency in Hz for use with clk_set_rate().
 * The clock framework handles the divider calculation.
 */
static struct cpufreq_frequency_table ensemble_freq_table[] = {
	{ .driver_data = 800000000, .frequency = 800000 },  /* PLL / 1 */
	{ .driver_data = 400000000, .frequency = 400000 },  /* PLL / 2 */
	{ .driver_data = 266666667, .frequency = 266667 },  /* PLL / 3 */
	{ .driver_data = 200000000, .frequency = 200000 },  /* PLL / 4 */
	{ .driver_data = 160000000, .frequency = 160000 },  /* PLL / 5 */
	{ .driver_data = 133333333, .frequency = 133333 },  /* PLL / 6 */
	{ .driver_data = 114285714, .frequency = 114285 },  /* PLL / 7 */
	{ .driver_data = 100000000, .frequency = 100000 },  /* PLL / 8 */
	{ .driver_data = 88888889, .frequency = 88888 },   /* PLL / 9 */
	{ .driver_data = 80000000, .frequency = 80000 },   /* PLL / 10 */
	{ .driver_data = 72727273, .frequency = 72727 },   /* PLL / 11 */
	{ .driver_data = 66666667, .frequency = 66666 },   /* PLL / 12 */
	{ .driver_data = 61538462, .frequency = 61538 },   /* PLL / 13 */
	{ .driver_data = 57142857, .frequency = 57142 },   /* PLL / 14 */
	{ .driver_data = 53333333, .frequency = 53333 },   /* PLL / 15 */
	{ .driver_data = 50000000, .frequency = 50000 },   /* PLL / 16 */
	{ .driver_data = 47058824, .frequency = 47058 },   /* PLL / 17 */
	{ .driver_data = 44444444, .frequency = 44444 },   /* PLL / 18 */
	{ .driver_data = 42105263, .frequency = 42105 },   /* PLL / 19 */
	{ .driver_data = 40000000, .frequency = 40000 },   /* PLL / 20 */
	{ .driver_data = 38095238, .frequency = 38095 },   /* PLL / 21 */
	{ .driver_data = 36363636, .frequency = 36363 },   /* PLL / 22 */
	{ .driver_data = 34782609, .frequency = 34782 },   /* PLL / 23 */
	{ .driver_data = 33333333, .frequency = 33333 },   /* PLL / 24 */
	{ .driver_data = 32000000, .frequency = 32000 },   /* PLL / 25 */
	{ .driver_data = 30769231, .frequency = 30769 },   /* PLL / 26 */
	{ .driver_data = 29629630, .frequency = 29629 },   /* PLL / 27 */
	{ .driver_data = 28571429, .frequency = 28571 },   /* PLL / 28 */
	{ .driver_data = 27586207, .frequency = 27586 },   /* PLL / 29 */
	{ .driver_data = 26666667, .frequency = 26666 },   /* PLL / 30 */
	{ .driver_data = 25806452, .frequency = 25806 },   /* PLL / 31 */
	{ .driver_data = 25000000, .frequency = 25000 },   /* PLL / 32 */
	{ .frequency = CPUFREQ_TABLE_END },
};

static unsigned int cur_freq_khz; /* Boot default, read from DT */
static DEFINE_MUTEX(ensemble_cpufreq_lock);

static unsigned int ensemble_cpufreq_get(unsigned int cpu)
{
	unsigned long rate_hz;
	unsigned int freq_khz;

	/* Get actual rate from clock framework */
	rate_hz = ensemble_cpu_clk_get_rate();
	DIV_ROUND_CLOSEST(rate_hz, 1000);

	/* Fallback to cached value, protected by mutex */
	mutex_lock(&ensemble_cpufreq_lock);
	freq_khz = cur_freq_khz;
	mutex_unlock(&ensemble_cpufreq_lock);

	return freq_khz;
}

static int ensemble_cpufreq_target_index(struct cpufreq_policy *policy,
					 unsigned int index)
{
	unsigned long target_rate_hz;
	int ret;

	target_rate_hz = ensemble_freq_table[index].driver_data;

	mutex_lock(&ensemble_cpufreq_lock);

	/* Use clock framework to set the CPU frequency */
	ret = ensemble_cpu_clk_set_rate(target_rate_hz);
	if (ret) {
		mutex_unlock(&ensemble_cpufreq_lock);
		pr_err("ensemble-cpufreq: failed to set freq %lu Hz: %d\n",
		       target_rate_hz, ret);
		return ret;
	}

	cur_freq_khz = ensemble_freq_table[index].frequency;

	mutex_unlock(&ensemble_cpufreq_lock);

	pr_debug("ensemble-cpufreq: set freq=%u kHz via clock framework\n",
		 cur_freq_khz);

	return 0;
}

static int ensemble_cpufreq_init(struct cpufreq_policy *policy)
{
	unsigned long cur_rate_hz;

	/*
	 * Both A32 cores share the same clock divider, so they must
	 * always run at the same frequency. All possible CPUs are
	 * therefore part of this policy; cpufreq_generic_init() will
	 * set policy->cpus accordingly.
	 */
	cpufreq_generic_init(policy, ensemble_freq_table,
			     ENSEMBLE_TRANSITION_LATENCY_NS); /* 100 ms transition latency */

	mutex_lock(&ensemble_cpufreq_lock);
	/* Get current frequency from clock framework */
	cur_rate_hz = ensemble_cpu_clk_get_rate();
	if (cur_rate_hz) {
		cur_freq_khz = cur_rate_hz / 1000;
		policy->cur = cur_freq_khz;
	} else {
		policy->cur = ensemble_cpufreq_get(policy->cpu);
	}

	mutex_unlock(&ensemble_cpufreq_lock);

	pr_info("ensemble-cpufreq: CPU%u initialized, current freq=%u kHz\n",
		policy->cpu, policy->cur);

	return 0;
}

static int ensemble_cpufreq_verify(struct cpufreq_policy_data *policy)
{
	return cpufreq_generic_frequency_table_verify(policy);
}

static struct cpufreq_driver ensemble_cpufreq_driver = {
	.name		= "ensemble-cpufreq",
	.flags		= CPUFREQ_NEED_INITIAL_FREQ_CHECK,
	.init		= ensemble_cpufreq_init,
	.verify		= ensemble_cpufreq_verify,
	.target_index	= ensemble_cpufreq_target_index,
	.get		= ensemble_cpufreq_get,
	.attr		= cpufreq_generic_attr,
};

static int ensemble_cpufreq_probe(struct platform_device *pdev)
{
	int ret;
	u32 clock_freq_hz;

	/* Read clock-frequency from device tree (required) */
	if (of_property_read_u32(pdev->dev.of_node, "clock-frequency",
				 &clock_freq_hz)) {
		dev_err(&pdev->dev,
			"clock-frequency property missing in device tree\n");
		return -EINVAL;
	}

	dev_info(&pdev->dev, "clock-frequency: %u Hz (%u kHz)\n",
		 clock_freq_hz, clock_freq_hz / 1000);

	/* Convert Hz to kHz for cur_freq_khz */
	cur_freq_khz = clock_freq_hz / 1000;

	/* Verify the CPU clock is available from the clock framework */
	if (!ensemble_cpu_clk_get_rate()) {
		dev_err(&pdev->dev,
			"CPU clock not available from clock framework\n");
		return -ENODEV;
	}

	ret = cpufreq_register_driver(&ensemble_cpufreq_driver);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register cpufreq driver: %d\n",
			ret);
		return ret;
	}

	dev_info(&pdev->dev, "Alif Ensemble CPUFreq driver registered (using clock framework)\n");
	return 0;
}

static void ensemble_cpufreq_remove(struct platform_device *pdev)
{
	cpufreq_unregister_driver(&ensemble_cpufreq_driver);
}

static const struct of_device_id ensemble_cpufreq_match[] = {
	{ .compatible = "alif,ensemble-cpufreq" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ensemble_cpufreq_match);

static struct platform_driver ensemble_cpufreq_platdrv = {
	.probe	= ensemble_cpufreq_probe,
	.remove_new = ensemble_cpufreq_remove,
	.driver	= {
		.name		= "ensemble-cpufreq",
		.of_match_table	= ensemble_cpufreq_match,
	},
};
module_platform_driver(ensemble_cpufreq_platdrv);

MODULE_AUTHOR("Pankaj Pandey <pankaj.pandey@alifsemi.com>");
MODULE_DESCRIPTION("Alif Ensemble Cortex-A32 CPUFreq Driver");
MODULE_LICENSE("GPL");
