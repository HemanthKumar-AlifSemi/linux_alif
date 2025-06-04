// SPDX-License-Identifier: GPL-2.0-only
/*
 * Define machines for Alif Ensemble series
 *
 * Copyright (C) 2024 Alif Semiconductor
 * Author: Harith George <harith.g@alifsemi.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <linux/of.h>
#include <linux/of_platform.h>

static void __init ensemble_init(void)
{
}

static const char *const ensemble_dt_match[] __initconst = {
	"alif,ensemble",
	NULL
};

DT_MACHINE_START(ENSEMBLE_DT, "Alif Ensemble")
	.dt_compat      = ensemble_dt_match,
	.init_machine	= ensemble_init,
MACHINE_END
