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

#ifndef _ETHOSU_DIRECT_INFERENCE_H_
#define _ETHOSU_DIRECT_INFERENCE_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/mutex.h>
#include <linux/list.h>

#include <common/ethosu_inference.h>

/****************************************************************************
 * Types
 ****************************************************************************/

struct ethosu_buffer;
struct ethosu_network;
struct ethosu_uapi_inference_create;
struct ethosu_uapi_cancel_inference_status;

/**
 * struct ethosu_direct_inference_queue - Direct inference queue
 * @lock:	Lock to prevent concurrent access
 * @list:	List of inference to be processed
 * @running_inference:	Inference currently being processed
 */
struct ethosu_direct_inference_queue {
	struct mutex                   lock;
	struct list_head               list;
	struct ethosu_direct_inference *running_inference;
};

/**
 * struct ethosu_direct_inference - Direct inference
 * @inf:		Inference
 * @intermediate:	Intermediate data buffer
 * @constant:		Constant data buffer
 */
struct ethosu_direct_inference {
	struct ethosu_inference inf;
	struct ethosu_buffer    *intermediate;
	struct ethosu_buffer    *constant;
	struct list_head        list_entry;
};

struct ethosu_direct_device;

/****************************************************************************
 * Functions
 ****************************************************************************/

void ethosu_direct_inference_queue_init(struct ethosu_direct_inference_queue
					*queue);

int ethosu_direct_inference_create(struct ethosu_device *edev,
				   struct ethosu_network *net,
				   struct ethosu_inference **inf);

int ethosu_direct_inference_destroy(struct ethosu_inference *inf);

int ethosu_direct_inference_setup(struct ethosu_inference *inf,
				  struct ethosu_uapi_inference_create *uapi);

int ethosu_direct_inference_release(struct ethosu_inference *inf);

int ethosu_direct_inference_send(struct ethosu_inference *inf);

int ethosu_direct_inference_cancel(struct ethosu_inference *inf,
				   struct ethosu_uapi_cancel_inference_status *uapi);

int ethosu_direct_inference_done(struct ethosu_direct_device *edirect_dev,
				 bool success);

#endif /* _ETHOSU_DIRECT_INFERENCE_H_ */
