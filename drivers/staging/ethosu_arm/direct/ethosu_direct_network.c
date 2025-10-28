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

#include <direct/ethosu_direct_network.h>
#include <direct/ethosu_direct_device.h>
#include <direct/ethosu_direct_cop_2.h>

#include <common/ethosu_network.h>
#include <common/ethosu_device.h>
#include <common/ethosu_dma_mem.h>

#include <linux/align.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

/* Max supported command stream length is 16 MiBs */
#define ETHOSU_MAX_CMD_LEN 0x1000000
/* A command stream must at least have 4 bytes */
#define ETHOSU_MIN_CMD_LEN 0x4

/* Vela Ethos-U Metadata */
#define ETHOSU_METADATA_FOURCC ('M' << 24 | 'U' << 16 | 'E' << 8 | 'V')
#define ETHOSU_METADATA_SUPPORTED_MAJOR 1
#define ETHOSU_METADATA_SUPPORTED_MINOR 0
#define ETHOSU_METADATA_LENGTH_SIZE sizeof_field( \
		struct ethosu_direct_cop_2_entry_cmd_stream, metadata_length)

#define ETHOSU_METADATA_SUPPORTED_FEATURES 0x2

/****************************************************************************
 * Types
 ****************************************************************************/

struct ethosu_direct_metadata_header {
	uint32_t fourcc;
	uint32_t major : 16;
	uint32_t minor : 16;
	uint32_t length;
} __packed;

struct ethosu_direct_metadata_data {
	uint32_t config;
	uint32_t id;
	uint32_t features;
	uint64_t cache_size;
} __packed;

struct ethosu_direct_metadata {
	struct ethosu_direct_metadata_header header;
	struct ethosu_direct_metadata_data   data;
} __packed;

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_direct_network *to_direct_network(struct ethosu_network *net)
{
	if (net && net->edev->type == ETHOSU_DEVICE_TYPE_DIRECT)
		return container_of(net, struct ethosu_direct_network, net);

	return NULL;
}

static int ethosu_direct_network_validate_metadata(struct ethosu_device *edev,
						   void *data,
						   size_t size)
{
	int ret;
	struct device *dev = &edev->dev;
	struct ethosu_direct_metadata *metadata = data;

	if (!data)
		return -EINVAL;

	if (size < sizeof(struct ethosu_direct_metadata_header)) {
		dev_err(dev, "Network metadata. Insufficient data\n");

		return -EINVAL;
	}

	if (metadata->header.fourcc != ETHOSU_METADATA_FOURCC) {
		dev_err(dev, "Network metadata. Invalid identifier: 0x%02x\n",
			metadata->header.fourcc);

		return -EINVAL;
	}

	if (metadata->header.major != ETHOSU_METADATA_SUPPORTED_MAJOR) {
		dev_err(dev,
			"Network metadata. Unsupported major version: %u.X\n",
			metadata->header.major);

		return -EINVAL;
	}

	if (metadata->header.minor > ETHOSU_METADATA_SUPPORTED_MINOR)
		dev_warn(dev,
			 "Network metadata. Minor version %u is larger than supported version %u. Only %u.%u metadata will be supported\n",
			 metadata->header.minor, ETHOSU_METADATA_SUPPORTED_MINOR,
			 ETHOSU_METADATA_SUPPORTED_MAJOR,
			 ETHOSU_METADATA_SUPPORTED_MINOR);

	if (!metadata->header.length) {
		dev_err(dev, "Network metadata. Invalid zero length\n");

		return -EINVAL;
	}

	if (metadata->header.length <
	    sizeof(struct ethosu_direct_metadata_data)) {
		dev_err(dev, "Network metadata. Data size mismatch\n");

		return -EINVAL;
	}

	if (metadata->data.features != ETHOSU_METADATA_SUPPORTED_FEATURES) {
		dev_err(dev,
			"Network metadata. Network compiled with unsupported features: 0x%02x\n",
			metadata->data.features);

		return -EINVAL;
	}

	ret = ethosu_direct_device_verify_hw_config(edev, metadata->data.config,
						    metadata->data.id,
						    metadata->data.cache_size);
	if (ret)
		dev_err(dev,
			"Network metadata. Network compiled for another NPU configuration");

	return ret;
}

static int ethosu_direct_network_get_cmd_stream(struct device *dev,
						struct ethosu_direct_cop_2_entry *entry,
						uint8_t **cmd_ptr_out,
						size_t *cmd_length_out)
{
	uint8_t *cmd_ptr;
	ssize_t cmd_length = entry->header.length -
			     entry->cmd_stream.metadata_length -
			     ETHOSU_METADATA_LENGTH_SIZE;

	if (cmd_length < ETHOSU_MIN_CMD_LEN) {
		dev_err(dev,
			"Network command stream. Invalid command stream length\n");

		return -EINVAL;
	}

	if (cmd_length > ETHOSU_MAX_CMD_LEN) {
		dev_err(dev,
			"Network command stream. Command stream length exceeds 16 MiBs\n");

		return -EINVAL;
	}

	if (!IS_ALIGNED(cmd_length, 4)) {
		dev_err(dev,
			"Network command stream. Command stream length must be a multiple of four\n");

		return -EINVAL;
	}

	cmd_ptr = entry->cmd_stream.data + entry->cmd_stream.metadata_length;
	if (!IS_ALIGNED((uintptr_t)cmd_ptr, 16)) {
		dev_err(dev,
			"Network command stream. Command stream address is not 16-byte aligned\n");

		return -EINVAL;
	}

	*cmd_ptr_out = cmd_ptr;
	*cmd_length_out = cmd_length;

	return 0;
}

int ethosu_direct_network_create(struct ethosu_device *edev,
				 struct ethosu_network **net)
{
	struct device *dev = &edev->dev;
	struct ethosu_direct_network *d_net;

	d_net = devm_kzalloc(dev, sizeof(*d_net), GFP_KERNEL);
	if (!d_net) {
		dev_err(dev,
			"Direct network create. Failed to allocate struct");

		return -ENOMEM;
	}

	*net = &d_net->net;

	return 0;
}

int ethosu_direct_network_destroy(struct ethosu_network *net)
{
	struct ethosu_direct_network *d_net = to_direct_network(net);
	struct device *dev;

	if (!d_net)
		return 0;

	dev = &d_net->net.edev->dev;

	memset(d_net, 0, sizeof(*d_net));
	devm_kfree(dev, net);

	return 0;
}

int ethosu_direct_network_setup(struct ethosu_network *net)
{
	struct device *dev;
	struct ethosu_device *edev;
	struct ethosu_direct_network *d_net = to_direct_network(net);
	struct ethosu_direct_cop_2 cop = { 0 };
	ptrdiff_t cmd_offset = 0U;
	size_t cmd_stream_count = 0;
	size_t cmd_length;
	uint8_t *cmd_ptr;
	bool cmd_found = false;
	int ret;

	if (!d_net)
		return -EINVAL;

	edev = d_net->net.edev;
	dev = &edev->dev;

	ret = ethosu_direct_cop_2_init(dev, net->dma_mem->cpu_addr,
				       net->dma_mem->size, &cop);
	if (ret)
		return ret;

	do {
		switch (cop.entry->header.type) {
		case COMMAND_STREAM:
			if (cop.entry->header.length <
			    ETHOSU_METADATA_LENGTH_SIZE) {
				dev_err(dev,
					"Network setup. Invalid command stream entry length\n");

				return -EINVAL;
			}

			cmd_stream_count++;
			ret = ethosu_direct_network_validate_metadata(edev,
								      cop.entry->cmd_stream.data,
								      cop.entry->cmd_stream.metadata_length);
			if (ret)
				continue;

			if (cmd_found) {
				dev_err(dev,
					"Network setup. Multiple compatible command streams found. Only one is supported\n");

				return -EINVAL;
			}

			ret = ethosu_direct_network_get_cmd_stream(dev,
								   cop.entry,
								   &cmd_ptr,
								   &cmd_length);
			if (ret)
				return ret;

			cmd_offset = (void *)cmd_ptr - net->dma_mem->cpu_addr;
			d_net->cmd_addr = net->dma_mem->dma_addr + cmd_offset;
			d_net->cmd_length = cmd_length;
			cmd_found = true;
			break;
		default:
			dev_warn(dev,
				 "Network setup. Skipping unknown custom operator payload entry type: 0x%02x\n",
				 cop.entry->header.type);
		}
	} while(ethosu_direct_cop_2_next_entry(&cop));

	if (!cmd_found) {
		if (cmd_stream_count)
			dev_err(dev,
				"Network setup. Found %zu command streams and none were compatible\n",
				cmd_stream_count);
		else
			dev_err(dev,
				"Network setup. No command streams found\n");

		return -EINVAL;
	}

	return 0;
}
