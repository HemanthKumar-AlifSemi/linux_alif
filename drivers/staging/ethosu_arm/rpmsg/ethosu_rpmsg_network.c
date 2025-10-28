/*
 * SPDX-FileCopyrightText: Copyright 2020, 2022-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#include <rpmsg/ethosu_rpmsg_network.h>

#include <common/ethosu_device.h>
#include <rpmsg/ethosu_rpmsg_device.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

/* Network mask has 32 bits */
#define MAX_INDEX 31

#define IS_INDEX_AVAILABLE(bits, index) (bits & (1U << index))

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_rpmsg_network *to_rpmsg_network(struct ethosu_network *net)
{
	if (net->edev->type == ETHOSU_DEVICE_TYPE_RPMSG)
		return container_of(net, struct ethosu_rpmsg_network, net);

	return NULL;
}

int ethosu_rpmsg_network_create(struct ethosu_device *edev,
				struct ethosu_network **net)
{
	struct ethosu_rpmsg_device *rp_dev = to_rpmsg_device(edev);
	struct device *dev = &rp_dev->edev.dev;
	struct ethosu_rpmsg_network *rp_net;

	if (!rp_dev)
		return -EINVAL;

	rp_net = devm_kzalloc(dev, sizeof(*rp_net), GFP_KERNEL);
	if (!rp_net) {
		dev_err(dev,
			"Rpmsg network create. Failed to allocate struct");

		return -ENOMEM;
	}

	rp_net->dev = dev;
	rp_net->mailbox = &rp_dev->mailbox;

	*net = &rp_net->net;

	return 0;
}

int ethosu_rpmsg_network_destroy(struct ethosu_network *net)
{
	struct ethosu_rpmsg_network *rp_net = to_rpmsg_network(net);
	struct device *dev;

	if (!rp_net)
		return 0;

	dev = rp_net->dev;

	memset(rp_net, 0, sizeof(*rp_net));
	devm_kfree(dev, rp_net);

	return 0;
}

int ethosu_rpmsg_network_setup(struct ethosu_network *net)
{
	return 0;
}

int ethosu_rpmsg_network_check_index(struct ethosu_device *edev,
				     uint32_t index)
{
	struct device *dev = &edev->dev;

	if (index > MAX_INDEX) {
		dev_err(dev,
			"Rpmsg network check index. Network index out of range");

		return -EINVAL;
	}

	if (!IS_INDEX_AVAILABLE(edev->capabilities.network_mask, index)) {
		dev_err(dev,
			"Rpmsg network check index. No network with index %u available",
			index);

		return -ENOENT;
	}

	return 0;
}
