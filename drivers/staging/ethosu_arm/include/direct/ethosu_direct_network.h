/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#ifndef _ETHOSU_DIRECT_NETWORK_H_
#define _ETHOSU_DIRECT_NETWORK_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <common/ethosu_network.h>

/****************************************************************************
 * Types
 ****************************************************************************/

struct ethosu_device;

struct ethosu_direct_network {
	struct ethosu_network net;
	dma_addr_t            cmd_addr;
	uint32_t              cmd_length;
};

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_direct_network *__must_check to_direct_network(
	struct ethosu_network *net);

int ethosu_direct_network_create(struct ethosu_device *edev,
				 struct ethosu_network **net);

int ethosu_direct_network_destroy(struct ethosu_network *net);

int ethosu_direct_network_setup(struct ethosu_network *net);

#endif /* _ETHOSU_DIRECT_NETWORK_H_ */
