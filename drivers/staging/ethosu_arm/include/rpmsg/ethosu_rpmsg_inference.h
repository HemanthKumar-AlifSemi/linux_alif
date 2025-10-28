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

#ifndef _ETHOSU_RPMSG_INFERENCE_H_
#define _ETHOSU_RPMSG_INFERENCE_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <rpmsg/ethosu_rpmsg_mailbox.h>
#include <common/ethosu_inference.h>
#include <uapi/ethosu.h>

#include <linux/kref.h>
#include <linux/types.h>

/****************************************************************************
 * Types
 ****************************************************************************/

struct ethosu_buffer;
struct ethosu_rpmsg_inference_rsp;
struct ethosu_rpmsg_network;
struct ethosu_inference;
struct ethosu_uapi_inference_create;
struct file;

/**
 * struct ethosu_rpmsg_inference - Rpmsg Inference struct
 * @inf:			Inference
 * @dev:			Arm Ethos-U device
 * @net:			Pointer to network
 * @msg:			Mailbox message
 */
struct ethosu_rpmsg_inference {
	struct ethosu_inference         inf;
	struct device                   *dev;
	struct ethosu_rpmsg_mailbox     *mailbox;
	struct ethosu_rpmsg_network     *net;
	struct ethosu_rpmsg_mailbox_msg msg;
};

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_rpmsg_inference *__must_check to_rpmsg_inference(
	struct ethosu_inference *inf);

/**
 * ethosu_rpmsg_inference_create() - Create inference
 *
 * This function must be called in the context of a user space process.
 *
 * Return: fd on success, else error code.
 */
int ethosu_rpmsg_inference_create(struct ethosu_device *edev,
				  struct ethosu_network *net,
				  struct ethosu_inference **inf);

int ethosu_rpmsg_inference_destroy(struct ethosu_inference *inf);

int ethosu_rpmsg_inference_setup(struct ethosu_inference *inf,
				 struct ethosu_uapi_inference_create *uapi);

int ethosu_rpmsg_inference_send(struct ethosu_inference *inf);

/**
 * ethosu_rpmsg_inference_rsp() - Handle inference response
 */
void ethosu_rpmsg_inference_rsp(struct ethosu_rpmsg_mailbox *mailbox,
				int msg_id,
				struct ethosu_rpmsg_inference_rsp *rsp);

#endif /* _ETHOSU_RPMSG_INFERENCE_H_ */
