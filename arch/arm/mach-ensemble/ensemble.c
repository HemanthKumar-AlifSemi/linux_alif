// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * Define machines for Alif Ensemble series
 *
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
	/* Maintainer: Harith George <harith.g@alifsemi.com> */
	.dt_compat      = ensemble_dt_match,
	.init_machine	= ensemble_init,
MACHINE_END
