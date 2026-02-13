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

#ifndef _ETHOSU_RPMSG_DEVICE_H_
#define _ETHOSU_RPMSG_DEVICE_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <common/ethosu_device.h>
#include <rpmsg/ethosu_rpmsg_mailbox.h>
#include <linux/rpmsg.h>
#include <linux/remoteproc.h>

struct rpmsg_device;
struct rpmsg_endpoint;

struct ethosu_rpmsg_device {
	struct ethosu_device        edev;
	struct ethosu_rpmsg_mailbox mailbox;
	struct rpmsg_device         *rpdev;
	struct rpmsg_endpoint       *ept;
};

struct ethosu_rpmsg_device *__must_check to_rpmsg_device(
	struct ethosu_device *edev);

int ethosu_rpmsg_device_init(struct rpmsg_device *rpdev,
			     struct class *class,
			     dev_t devt);

void ethosu_rpmsg_device_deinit(struct rpmsg_device *rpdev);

#endif /* _ETHOSU_RPMSG_DEVICE_H_ */
