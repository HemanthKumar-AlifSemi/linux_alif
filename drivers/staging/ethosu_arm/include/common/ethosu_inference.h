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

#ifndef _ETHOSU_INFERENCE_H_
#define _ETHOSU_INFERENCE_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <uapi/ethosu.h>

#include <linux/kref.h>
#include <linux/types.h>
#include <linux/wait.h>

/****************************************************************************
 * Types
 ****************************************************************************/

struct ethosu_buffer;
struct ethosu_device;
struct ethosu_network;
struct ethosu_uapi_inference_create;
struct file;

/**
 * struct ethosu_inference - Inference struct
 * @edev:			Arm Ethos-U device
 * @file:			File handle
 * @kref:			Reference counter
 * @waitq:			Wait queue
 * @done:			Wait condition is done
 * @ifm:			Pointer to IFM buffer
 * @ofm:			Pointer to OFM buffer
 * @net:			Pointer to network
 * @status:			Inference status
 * @pmu_event_config:		PMU event configuration
 * @pmu_event_config_num:	Number of PMU events in configuration
 * @pmu_event_count:		PMU event count after inference
 * @pmu_cycle_counter_enable:	PMU cycle counter enable
 * @pmu_cycle_counter_count:	PMU cycle counter count after inference
 */
struct ethosu_inference {
	struct ethosu_device    *edev;
	struct file             *file;
	struct kref             kref;
	wait_queue_head_t       waitq;
	bool                    done;
	uint32_t                ifm_count;
	struct ethosu_buffer    *ifm[ETHOSU_FD_MAX];
	uint32_t                ofm_count;
	struct ethosu_buffer    *ofm[ETHOSU_FD_MAX];
	struct ethosu_network   *net;
	enum ethosu_uapi_status status;
	uint8_t                 pmu_event_config[ETHOSU_PMU_EVENT_MAX];
	uint32_t                pmu_event_config_num;
	uint64_t                pmu_event_count[ETHOSU_PMU_EVENT_MAX];
	bool                    pmu_cycle_counter_enable;
	uint64_t                pmu_cycle_counter_count;
};

/****************************************************************************
 * Functions
 ****************************************************************************/

int ethosu_inference_create(struct ethosu_device *edev,
			    struct ethosu_network *net,
			    struct ethosu_uapi_inference_create *uapi);

void ethosu_inference_get(struct ethosu_inference *inf);

int ethosu_inference_put(struct ethosu_inference *inf);

#endif /* _ETHOSU_INFERENCE_H_ */
