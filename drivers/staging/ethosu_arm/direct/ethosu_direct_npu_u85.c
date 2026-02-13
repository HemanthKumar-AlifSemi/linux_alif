/*
 * SPDX-FileCopyrightText: Copyright 2019-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <direct/ethosu_direct_device.h>
#include <direct/interface/ethosu_direct_interface_common.h>
#include <direct/interface/ethosu_direct_interface_u85.h>
#include <direct/npu/ethosu_direct_npu_common.h>
#include <direct/npu/ethosu_direct_npu_u85.h>

#include <linux/io.h>
#include <linux/platform_device.h>

/******************************************************************************
 * Defines
 ******************************************************************************/

#define DEV_DBG_CONFIG_R(dev, prefix, config)											\
	dev_dbg(dev,														\
		prefix ". product=%u, cmd_stream_version=%u, macs_per_cc=%u, num_axi_ext=%u, num_axi_sram=%u, custom_dma=%u\n",	\
		config.product,													\
		config.cmd_stream_version,											\
		config.macs_per_cc,												\
		1U << config.num_axi_ext,											\
		1U << config.num_axi_sram,											\
		config.custom_dma)

/******************************************************************************
 * Functions
 ******************************************************************************/

static int ethosu_direct_u85_get_hw_config(
	struct ethosu_direct_device *edirect_dev,
	struct ethosu_config *hw_cfg)
{
	struct config_r cfg = { 0 };

	cfg.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_CONFIG);

	hw_cfg->macs_per_cc = cfg.macs_per_cc;
	hw_cfg->cmd_stream_version = cfg.cmd_stream_version;
	hw_cfg->custom_dma = cfg.custom_dma;

	return 0;
}

static int ethosu_direct_u85_verify_hw_config(
	struct ethosu_direct_device *edirect_dev,
	uint32_t cfg,
	uint32_t id)
{
	struct device *dev = &edirect_dev->edev.dev;
	const struct config_r req_cfg = { .word = cfg };
	struct config_r npu_cfg = { 0 };
	bool valid_config = true;

	npu_cfg.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_CONFIG);

	DEV_DBG_CONFIG_R(dev, "NPU HW config", npu_cfg);
	DEV_DBG_CONFIG_R(dev, "Requested config", req_cfg);

	if (npu_cfg.word != req_cfg.word) {
		valid_config &= MATCH_CONFIG_FIELD(dev, product, npu_cfg,
						   req_cfg);
		valid_config &= MATCH_CONFIG_FIELD(dev, macs_per_cc, npu_cfg,
						   req_cfg);
		valid_config &= MATCH_CONFIG_FIELD(dev, cmd_stream_version,
						   npu_cfg, req_cfg);
		valid_config &= MATCH_CONFIG_FIELD(dev, custom_dma, npu_cfg,
						   req_cfg);
		valid_config &= MATCH_CONFIG_FIELD_SHIFT(dev, num_axi_ext,
							 npu_cfg, req_cfg);
		valid_config &= MATCH_CONFIG_FIELD_SHIFT(dev, num_axi_sram,
							 npu_cfg, req_cfg);
	}

	if (!valid_config)
		return -EINVAL;

	return ethosu_direct_npu_verify_optimizer_id(edirect_dev, id);
}

static void set_mem_attr_reg(struct ethosu_direct_device *edirect_dev)
{
	uint32_t index;
	uint32_t mem_attr_addr = NPU_REG_MEM_ATTR_BASE;
	struct device *dev = &edirect_dev->edev.dev;

	for (index = 0; index < NPU_REG_MEM_ATTR_ARRLEN; ++index) {
		const struct ethosu_mem_attr *e_mem_attr =
			&edirect_dev->mem_config.mem_attr[index];
		const struct mem_attr_r mem_attr = {
			.mem_domain = e_mem_attr->mem_domain,
			.memtype    = e_mem_attr->memtype,
			.axi_port   = e_mem_attr->axi_port
		};

		ethosu_direct_npu_write_reg(edirect_dev, mem_attr_addr,
					    mem_attr.word);
		dev_dbg(dev, "Mem attr reg val 0x%x", mem_attr.word);

		mem_attr_addr += NPU_REG_MEM_ATTR_OFFSET;
	}
}

static int ethosu_direct_u85_config_mem(
	struct ethosu_direct_device *edirect_dev)
{
	const struct ethosu_port_cfg *sram_cfgs =
		&edirect_dev->mem_config.sram_cfgs;
	const struct axi_sram_r axi_s = {
		.max_beats                = sram_cfgs->beats,
		.max_outstanding_read_m1  = sram_cfgs->outstanding_read,
		.max_outstanding_write_m1 = sram_cfgs->outstanding_write
	};

	const struct ethosu_port_cfg *ext_cfgs =
		&edirect_dev->mem_config.ext_cfgs;
	const struct axi_ext_r axi_e = {
		.max_beats                = ext_cfgs->beats,
		.max_outstanding_read_m1  = ext_cfgs->outstanding_read,
		.max_outstanding_write_m1 = ext_cfgs->outstanding_write
	};

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_AXI_SRAM, axi_s.word);
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_AXI_EXT, axi_e.word);

	/* Configure MEM_ATTR array. These are user configurable,
	 * and each region will be set to use one of the entries
	 * as its config. */
	set_mem_attr_reg(edirect_dev);

	return 0;
}

static int ethosu_direct_u85_handle_irq_status(
	struct ethosu_direct_device *edirect_dev)
{
	int ret;
	struct status_r status = { .word = 0 };

	/* If a fault has occured, the NPU needs to be reset */
	ret = ethosu_direct_npu_handle_common_irq_status(edirect_dev, &status);
	if (ret || status.branch_fault)
		return -EFAULT;

	return 0;
}

static int ethosu_direct_u85_pmu_setup(struct ethosu_direct_device *edirect_dev,
				       bool enable_cycle_count,
				       uint8_t *events,
				       size_t num_events)
{
	struct device *dev = &edirect_dev->edev.dev;
	uint32_t pmu_event_addr = NPU_REG_PMEVTYPER_BASE;
	size_t i = 0;
	struct pmcr_r pmcr = {
		.cnt_en = 1,
	};

	struct pmcntenset_r pmcntenset = {
		.CYCLE_CNT = enable_cycle_count,
	};

	struct pmccntr_cfg_r pmccntr_cfg = {
		.CYCLE_CNT_CFG_START = PMU_EVENT_NPU_ACTIVE,
		.CYCLE_CNT_CFG_STOP  = PMU_EVENT_NPU_IDLE,
	};

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_PMCR,
				    pmcr.word);

	if (num_events > NPU_REG_PMEVTYPER_ARRLEN) {
		num_events = NPU_REG_PMEVTYPER_ARRLEN;
		dev_warn(dev,
			 "PMU setup: Too many PMU events truncating to %d\n",
			 NPU_REG_PMEVTYPER_ARRLEN);
	}

	for (i = 0; i < num_events; ++i) {
		struct pmevtyper_r pmu_event = { .word = 0 };

		if (events[i] < PMU_EVENT_MAX_IDX)
			pmu_event.EV_TYPE = pmu_event_lookup[events[i]];
		else
			dev_warn(dev,
				 "PMU setup: Ignoring invalid type %u for event[%zu]\n",
				 events[i], i);

		ethosu_direct_npu_write_reg(edirect_dev, pmu_event_addr,
					    pmu_event.word);
		pmu_event_addr += NPU_REG_PMEVTYPER_OFFSET;
		pmcntenset.word |= 1U << i;
	}

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_PMCCNTR_CFG,
				    pmccntr_cfg.word);

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_PMCNTENSET,
				    pmcntenset.word);

	pmcr.event_cnt_rst = 1;
	pmcr.cycle_cnt_rst = 1;
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_PMCR, pmcr.word);

	return 0;
}

static int ethosu_direct_u85_pmu_get_values(
	struct ethosu_direct_device *edirect_dev,
	bool read_cycle_count,
	uint64_t *cycle_count,
	uint64_t *event_count,
	size_t num_events)
{
	size_t i;
	uint32_t pmu_event_counter_addr = NPU_REG_PMEVCNTR_BASE;

	if (read_cycle_count) {
		const uint32_t val_lo = ethosu_direct_npu_read_reg(edirect_dev,
								   NPU_REG_PMCCNTR);
		const uint32_t val_hi = ethosu_direct_npu_read_reg(edirect_dev,
								   NPU_REG_PMCCNTR_HI)
					&
					0xFFFF;
		*cycle_count = ((uint64_t)val_hi << 32) | val_lo;
	}

	/* PMU setup already warned about too many events so silently truncate
	 * here */
	num_events = min_t(size_t, num_events, NPU_REG_PMEVCNTR_ARRLEN);
	for (i = 0; i < num_events; ++i) {
		event_count[i] = ethosu_direct_npu_read_reg(edirect_dev,
							    pmu_event_counter_addr);
		pmu_event_counter_addr += NPU_REG_PMEVCNTR_OFFSET;
	}

	return 0;
}

static int ethosu_direct_u85_pmu_disable(
	struct ethosu_direct_device *edirect_dev)
{
	const struct pmcr_r pmcr = {
		.word = 0,
	};

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_PMCR,
				    pmcr.word);

	return 0;
}

static const struct ethosu_direct_npu_ops u85_ops = {
	.config_mem        = &ethosu_direct_u85_config_mem,
	.get_hw_cfg        = &ethosu_direct_u85_get_hw_config,
	.verify_hw_cfg     = &ethosu_direct_u85_verify_hw_config,
	.handle_irq_status = &ethosu_direct_u85_handle_irq_status,
	.pmu_setup         = &ethosu_direct_u85_pmu_setup,
	.pmu_get_values    = &ethosu_direct_u85_pmu_get_values,
	.pmu_disable       = &ethosu_direct_u85_pmu_disable,
};

void ethosu_direct_npu_u85_ops_set(struct ethosu_direct_device *edirect_dev)
{
	edirect_dev->npu_ops = &u85_ops;
}
