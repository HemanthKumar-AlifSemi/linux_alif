/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#ifndef _ETHOSU_DIRECT_NPU_COMMON_H_
#define _ETHOSU_DIRECT_NPU_COMMON_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <common/ethosu_device.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

#define MATCH_CONFIG_FIELD(dev, field, npu_cfg, req_cfg)		      \
	({								      \
		bool _match = true;					      \
		if (npu_cfg.field != req_cfg.field) {			      \
			dev_err(dev,					      \
				"NPU HW config mismatch. npu." #field	      \
				"=%u requested." #			      \
				field "=%u\n", npu_cfg.field, req_cfg.field); \
			_match = false;					      \
		}							      \
		_match;							      \
	})

#define MATCH_CONFIG_FIELD_SHIFT(dev, field, npu_cfg, req_cfg)	      \
	({							      \
		bool _match = true;				      \
		if (npu_cfg.field != req_cfg.field) {		      \
			dev_err(dev,				      \
				"NPU HW config mismatch. npu." #field \
				"=%u requested." #		      \
				field "=%u\n", 1U << npu_cfg.field,   \
				1U << req_cfg.field);		      \
			_match = false;				      \
		}						      \
		_match;						      \
	})

/****************************************************************************
 * Types
 ****************************************************************************/

struct ethosu_direct_npu_ops {
	int (*config_mem)(struct ethosu_direct_device *edirect_dev);
	int (*get_hw_cfg)(struct ethosu_direct_device *edirect_dev,
			  struct ethosu_config *hw);
	int (*verify_hw_cfg)(struct ethosu_direct_device *edirect_dev,
			     uint32_t config,
			     uint32_t id);
	int (*handle_irq_status)(struct ethosu_direct_device *edirect_dev);
	int (*pmu_setup)(struct ethosu_direct_device *edirect_dev,
			 bool enable_cycle_count,
			 uint8_t *events,
			 size_t num_events);
	int (*pmu_get_values)(struct ethosu_direct_device *edirect_dev,
			      bool read_cycle_count,
			      uint64_t *cycle_count,
			      uint64_t *event_count,
			      size_t num_events);
	int (*pmu_disable)(struct ethosu_direct_device *edirect_dev);
};

/****************************************************************************
 * Functions
 ****************************************************************************/

void __iomem *ethosu_direct_npu_get_reg_addr(void __iomem *const npu_regs,
					     const u32 offset);

void ethosu_direct_npu_write_reg(struct ethosu_direct_device *edirect_dev,
				 const uint32_t offset,
				 const uint32_t value);

uint32_t ethosu_direct_npu_read_reg(
	const struct ethosu_direct_device *edirect_dev,
	const uint32_t offset);

int ethosu_reset_and_prepare_npu(struct ethosu_direct_device *const edirect_dev,
				 bool is_clk_q_iface_en,
				 bool is_pwr_q_iface_en);

int ethosu_direct_npu_soft_reset(struct ethosu_direct_device *edirect_dev,
				 bool is_clk_q_iface_en,
				 bool is_pwr_q_iface_en);

int ethosu_direct_npu_handle_interrupt(struct ethosu_direct_device *edirect_dev);

void ethosu_direct_npu_get_id(struct ethosu_direct_device *edirect_dev,
			      struct ethosu_id *id_info);

int ethosu_direct_npu_verify_optimizer_id(
	struct ethosu_direct_device *edirect_dev,
	uint32_t id);

int ethosu_direct_npu_handle_common_irq_status(
	struct ethosu_direct_device *edirect_dev,
	struct status_r *status);

bool ethosu_is_npu_running(struct ethosu_direct_device *edirect_dev);

int ethosu_direct_npu_handle_command_stream(
	struct ethosu_direct_device *edirect_dev,
	dma_addr_t cmd_addr,
	uint32_t cmd_size,
	dma_addr_t *base_addrs,
	size_t num_base_addr);

#endif /* _ETHOSU_DIRECT_NPU_COMMON_H_ */
