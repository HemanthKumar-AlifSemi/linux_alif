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

#ifndef _ETHOSU_DIRECT_DEVICE_H_
#define _ETHOSU_DIRECT_DEVICE_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <common/ethosu_device.h>
#include <direct/ethosu_direct_inference.h>

#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#define ETHOSU_AUTOSUSPEND_DELAY_MS 500
/* Wait time in us after NPU reset */
#define ETHOSU_RESET_TIMEOUT_US         (1000 * 1000)
#define ETHOSU_RESET_WAIT_US            1000

#define ETHOSU_DEV_MAX_AXI_LIM 4
#define ETHOSU_DEV_MAX_MEM_ATTR 4
#define ETHOSU_DEV_MAX_NUM_REGIONS 8

struct ethosu_driver_version {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
};

struct ethosu_id {
	uint32_t version_status; /* Version status */
	uint32_t version_minor;  /* Version minor */
	uint32_t version_major;  /* Version major */
	uint32_t product_major;  /* Product major */
	uint32_t arch_patch_rev; /* Architecture version patch */
	uint32_t arch_minor_rev; /* Architecture version minor */
	uint32_t arch_major_rev; /* Architecture version major */
};

struct ethosu_config {
	uint32_t macs_per_cc;        /* MACs per clock cycle */
	uint32_t cmd_stream_version; /* NPU command stream version */
	uint32_t custom_dma;         /* Custom DMA enabled */
};

struct ethosu_port_cfg {
	uint32_t beats;
	uint32_t outstanding_read;
	uint32_t outstanding_write;
};

struct ethosu_mem_attr {
	uint32_t mem_domain;
	uint32_t memtype;
	uint32_t axi_port;
};

struct ethosu_axi_limit {
	uint32_t beats;
	uint32_t memtype;
	uint32_t outstanding_read;
	uint32_t outstanding_write;
};

/**
 * struct ethosu_mem_config - Parameters for configuring the memory.
 * @sram_cfgs: AXI configuration for SRAM ports.
 * @ext_cfgs: AXI configuration for EXT ports.
 * @mem_attr: memory attributes for various regions.
 *
 * Memory configuration values read from the device tree.
 */
struct ethosu_mem_config {
	uint32_t                cs_region;
	uint32_t                region_cfgs[ETHOSU_DEV_MAX_NUM_REGIONS];
	struct ethosu_port_cfg  sram_cfgs;
	struct ethosu_port_cfg  ext_cfgs;
	struct ethosu_mem_attr  mem_attr[ETHOSU_DEV_MAX_MEM_ATTR];
	struct ethosu_axi_limit axi_limit[ETHOSU_DEV_MAX_AXI_LIM];
};

struct ethosu_sram_region {
	void __iomem *cpu_addr;
	dma_addr_t   npu_addr;
	size_t       size;
};

#define ethosu_direct_call_npu_op(edirect_dev, op, ...)  ({	   \
		edirect_dev->npu_ops && edirect_dev->npu_ops->op ? \
		edirect_dev->npu_ops->op(__VA_ARGS__) : -EOPNOTSUPP; })

struct ethosu_direct_npu_ops;

struct ethosu_direct_device {
	struct ethosu_device                 edev;
	unsigned int                         irq_num;
	struct workqueue_struct              *irq_work_q;
	struct work_struct                   irq_work;
	atomic_t                             irq_status;
	void __iomem                         *reg_base;
	struct ethosu_mem_config             mem_config;
	struct ethosu_sram_region            sram;
	struct ethosu_direct_inference_queue inf_queue;
	const struct ethosu_direct_npu_ops   *npu_ops;
	atomic_t                             running;
};

struct ethosu_direct_device *__must_check to_direct_device(
	struct ethosu_device *edev);

int ethosu_direct_device_init(struct platform_device *pdev,
			      struct class *class,
			      dev_t devt);

void ethosu_direct_device_deinit(struct platform_device *pdev);

int ethosu_direct_device_resume(struct device *dev);

int ethosu_direct_device_suspend_noirq(struct device *dev);

int ethosu_direct_device_runtime_resume(struct device *dev);

int ethosu_direct_device_runtime_suspend(struct device *dev);

int ethosu_direct_device_verify_hw_config(struct ethosu_device *edev,
					  uint32_t config,
					  uint32_t id,
					  uint64_t cache_size);

int ethosu_direct_device_run_inference(struct ethosu_direct_device *edirect_dev,
				       struct ethosu_direct_inference *d_inf);

int ethosu_direct_device_reset(struct ethosu_direct_device *edirect_dev);

void ethosu_direct_device_clear_sram(struct ethosu_direct_device *edirect_dev);

void ethosu_direct_device_pmu_setup(struct ethosu_direct_device *edirect_dev,
				    bool enable_cycle_count,
				    uint8_t *events,
				    size_t num_events);

void ethosu_direct_device_pmu_get_values(
	struct ethosu_direct_device *edirect_dev,
	bool read_cycle_count,
	uint64_t *cycle_count,
	uint64_t *event_count,
	size_t num_events);

void ethosu_direct_device_pmu_disable(struct ethosu_direct_device *edirect_dev);

#endif /* _ETHOSU_DIRECT_DEVICE_H_ */
