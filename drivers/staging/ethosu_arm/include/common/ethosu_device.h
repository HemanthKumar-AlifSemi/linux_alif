/*
 * SPDX-FileCopyrightText: Copyright 2020-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#ifndef _ETHOSU_DEVICE_H_
#define _ETHOSU_DEVICE_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <uapi/ethosu.h>

#include <linux/device.h>
#include <linux/cdev.h>

/****************************************************************************
 * Types
 ****************************************************************************/

enum ethosu_device_type {
	ETHOSU_DEVICE_TYPE_UNKNOWN = 0,
	ETHOSU_DEVICE_TYPE_RPMSG,
	ETHOSU_DEVICE_TYPE_DIRECT,
};

/**
 * struct ethosu_device_buffer_config - Device buffer configuration
 * @alignment:	Buffer alignment in bytes (must be a power of two)
 * @max_size:	Max buffer size supported in bytes
 */
struct ethosu_device_buffer_config {
	size_t alignment;
	size_t max_size;
};

/**
 * struct ethosu_device - Device structure
 */
struct ethosu_device {
	enum ethosu_device_type                type;
	struct device                          dev;
	struct cdev                            cdev;
	bool                                   cdev_setup;
	struct class                           *class;
	const struct ethosu_device_ops         *ops;
	struct ethosu_device_buffer_config     buffer_config;
	struct ethosu_uapi_device_capabilities capabilities;
};

struct ethosu_network;
struct ethosu_inference;

struct ethosu_device_ops {
	int (*capabilities_get)(struct ethosu_device *edev,
				struct ethosu_uapi_device_capabilities *cap);
	int (*ping)(struct ethosu_device *edev);
	int (*network_create)(struct ethosu_device *edev,
			      struct ethosu_network **net);
	int (*network_destroy)(struct ethosu_network *net);
	int (*network_check_index)(struct ethosu_device *net,
				   uint32_t index);
	int (*network_setup)(struct ethosu_network *net);
	int (*network_info)(struct ethosu_network *net,
			    struct ethosu_uapi_network_info *uapi);
	int (*inference_create)(struct ethosu_device *edev,
				struct ethosu_network *net,
				struct ethosu_inference **inf);
	int (*inference_destroy)(struct ethosu_inference *inf);
	int (*inference_setup)(struct ethosu_inference *inf,
			       struct ethosu_uapi_inference_create *uapi);
	int (*inference_release)(struct ethosu_inference *inf);
	int (*inference_send)(struct ethosu_inference *inf);
	int (*inference_cancel)(struct ethosu_inference *inf,
				struct ethosu_uapi_cancel_inference_status *uapi);
	/* Increment device usage and block if needed until NPU is ready */
	int (*rpm_get_sync)(struct ethosu_device *edev);

	/* Decrement device usage and mark the last busy time for the NPU (Busy
	 * time is used for the auto suspend grace period) */
	int (*rpm_put_and_mark_last_busy)(struct ethosu_device *edev);
};

/****************************************************************************
 * Defines
 ****************************************************************************/

#define ethosu_device_call_op(edev, op, ...) ({	\
		edev->ops && edev->ops->op ?	\
		edev->ops->op(__VA_ARGS__) : -EOPNOTSUPP; })

/****************************************************************************
 * Functions
 ****************************************************************************/

/**
 * ethosu_device_init() - Initialize and register Arm Ethos-U NPU device
 * @edev: NPU device to setup
 * @type: NPU device type
 * @ops: NPU device operations
 * @buf_config: NPU device buffer configuration
 * @parents: Device parent
 * @memory_dev: Reserved memory device the NPU device shall use
 * @class: Device class type
 * @devt: Device type
 *
 * This is the first step in the device creation where the device will be
 * initialized, setup and registered. Creation of the character device is done
 * in the second step in ethosu_device_finalize()
 *
 * Return: 0 on success, else error code.
 */
int ethosu_device_init(struct ethosu_device *edev,
		       enum ethosu_device_type type,
		       const struct ethosu_device_ops *ops,
		       const struct ethosu_device_buffer_config *buf_config,
		       struct device *parent,
		       struct device *memory_dev,
		       struct class *class,
		       dev_t *devt);

/**
 * ethosu_device_finalize() - Finalize the Arm Ethos-U NPU device
 * @edev: NPU device to setup
 * @devt: Device type
 *
 * This is the second and final step in the device creation where the character
 * device is created and must be called after the device has been initialized
 * with ethosu_device_init()
 *
 * Return: 0 on success, else error code.
 */
int ethosu_device_finalize(struct ethosu_device *edev,
			   dev_t devt);

/**
 * ethosu_device_deinit() - Deinitalize the Arm Ethos-U NPU device
 * @edev: NPU device to deinitalize
 */
void ethosu_device_deinit(struct ethosu_device *edev);

#endif /* _ETHOSU_DEVICE_H_ */
