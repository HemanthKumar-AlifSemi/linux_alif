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

#ifndef _ETHOSU_DIRECT_COP_2_H_
#define _ETHOSU_DIRECT_COP_2_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/types.h>
#include <direct/interface/ethosu_direct_interface_common.h>

/****************************************************************************
 * Types
 ****************************************************************************/

enum ehosu_direct_cop_2_entry_type {
	RESERVED       = 0U,
	COMMAND_STREAM = 1U,
};

struct ethosu_direct_cop_2_header {
	uint32_t fourcc;
	uint32_t major : 16;
	uint32_t minor : 16;
	uint64_t length;
} __packed;

struct ethosu_direct_cop_2_entry_header {
	uint32_t type;
	uint32_t length;
} __packed;

struct ethosu_direct_cop_2_entry_cmd_stream {
	uint32_t metadata_length;
	uint8_t  data[0];
} __packed;

struct ethosu_direct_cop_2_entry {
	struct ethosu_direct_cop_2_entry_header header;
	union {
		struct ethosu_direct_cop_2_entry_cmd_stream cmd_stream;
		uint8_t                                     data[0];
	};
} __packed;

struct ethosu_direct_cop_2 {
	struct ethosu_direct_cop_2_header *header;
	struct ethosu_direct_cop_2_entry  *entry;
	uint8_t                           *end;
};

struct device;

/****************************************************************************
 * Functions
 ****************************************************************************/

int ethosu_direct_cop_2_init(struct device *dev,
			     void *data,
			     size_t size,
			     struct ethosu_direct_cop_2 *cop);

bool ethosu_direct_cop_2_next_entry(struct ethosu_direct_cop_2 *cop);

#endif /* _ETHOSU_DIRECT_COP_2_H_ */
