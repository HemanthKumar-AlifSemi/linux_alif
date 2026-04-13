/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Alif Ensemble Clock Driver Interface
 *
 * Copyright 2026 Alif Semiconductor.
 */

#ifndef __LINUX_CLK_ENSEMBLE_CLK_H
#define __LINUX_CLK_ENSEMBLE_CLK_H

#include <linux/errno.h>

#ifdef CONFIG_COMMON_CLK

/**
 * ensemble_cpu_clk_set_rate - Set CPU clock frequency
 * @rate_hz: Target frequency in Hz
 *
 * This function is called by the cpufreq driver to change the CPU frequency.
 * It updates the HOSTCPUCLK_DIV1 divider register and notifies the clock
 * framework of the rate change.
 *
 * Returns 0 on success, negative error code on failure.
 */
int ensemble_cpu_clk_set_rate(unsigned long rate_hz);

/**
 * ensemble_cpu_clk_get_rate - Get current CPU clock frequency
 *
 * Returns the current CPU clock frequency in Hz, or 0 on error.
 */
unsigned long ensemble_cpu_clk_get_rate(void);

#else

static inline int ensemble_cpu_clk_set_rate(unsigned long rate_hz)
{
	return -ENODEV;
}

static inline unsigned long ensemble_cpu_clk_get_rate(void)
{
	return 0;
}

#endif /* CONFIG_COMMON_CLK */

#endif /* __LINUX_CLK_ENSEMBLE_CLK_H */
