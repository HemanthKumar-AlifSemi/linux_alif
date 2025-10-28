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

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <direct/ethosu_direct_cop_2.h>

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/math.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

/* Custom Operator Payload 2 */
#define ETHOSU_COP2_FOURCC ('2' << 24 | 'P' << 16 | 'O' << 8 | 'C')
#define ETHOSU_COP2_SUPPORTED_MAJOR 1
#define ETHOSU_COP2_SUPPORTED_MINOR 0

/****************************************************************************
 * Functions
 ****************************************************************************/

int ethosu_direct_cop_2_init(struct device *dev,
			     void *data,
			     size_t size,
			     struct ethosu_direct_cop_2 *cop)
{
	struct ethosu_direct_cop_2_header *header = data;

	if (!header || !cop)
		return -EINVAL;

	if (size < sizeof(*header)) {
		dev_err(dev, "COP init. Insufficient data\n");

		return -EINVAL;
	}

	if (header->fourcc != ETHOSU_COP2_FOURCC) {
		dev_err(dev, "COP init. Invalid identifier: 0x%02x\n",
			header->fourcc);

		return -EINVAL;
	}

	if (header->major != ETHOSU_COP2_SUPPORTED_MAJOR) {
		dev_err(dev, "COP init. Unsupported major version: %u.X\n",
			header->major);

		return -EINVAL;
	}

	if (header->minor > ETHOSU_COP2_SUPPORTED_MINOR)
		dev_warn(dev,
			 "COP Init. Minor version %u is larger than supported version %u. Only %u.%u features will be supported\n",
			 header->minor, ETHOSU_COP2_SUPPORTED_MINOR, ETHOSU_COP2_SUPPORTED_MAJOR,
			 ETHOSU_COP2_SUPPORTED_MINOR);

	if (!header->length) {
		dev_err(dev, "COP init. Invalid zero length\n");

		return -EINVAL;
	}

	if (header->length < sizeof(struct ethosu_direct_cop_2_entry_header)) {
		dev_err(dev, "COP init. No entrys in payload\n");

		return -EINVAL;
	}

	if (header->length > (size - sizeof(*header))) {
		dev_err(dev, "COP init. Invalid total payload length \n");

		return -EINVAL;
	}

	cop->header = header;
	cop->entry = (typeof(cop->entry))(++header);
	cop->end = ((uint8_t *)cop->entry) + cop->header->length;

	return 0;
}

bool ethosu_direct_cop_2_next_entry(struct ethosu_direct_cop_2 *cop)
{
	struct ethosu_direct_cop_2_entry_header *header;
	struct ethosu_direct_cop_2_entry *entry;
	size_t offset;

	if (!cop)
		return false;

	header = &cop->entry->header;
	offset = round_up(sizeof(*header) + header->length, 4);
	entry = (typeof(entry))((uint8_t *)cop->entry + offset);

	if (((uint8_t *)entry) >= cop->end)
		return false;

	cop->entry = entry;

	return true;
}
