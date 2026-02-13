/*
 * SPDX-FileCopyrightText: Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 */

#ifndef _ETHOSU_DIRECT_AXI_CONFIG_DRIVER_H_
#define _ETHOSU_DIRECT_AXI_CONFIG_DRIVER_H_

int ethosu_direct_axi_config_platform_driver_register(void);

void ethosu_direct_axi_config_platform_driver_unregister(void);

#endif /* _ETHOSU_DIRECT_AXI_CONFIG_DRIVER_H_ */
