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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <direct/ethosu_direct_device.h>
#include <direct/interface/ethosu_direct_interface_common.h>
#include <direct/npu/ethosu_direct_npu_common.h>

#include <linux/delay.h>
#include <linux/types.h>

/******************************************************************************
 * Defines
 ******************************************************************************/

#define DEV_DBG_ARCH_ID_R(dev, prefix, id)		  \
	dev_dbg(dev, prefix ". arch version: %u.%u.%u\n", \
		id.arch_major_rev,			  \
		id.arch_minor_rev,			  \
		id.arch_patch_rev)

/******************************************************************************
 * Functions
 ******************************************************************************/

void __iomem *ethosu_direct_npu_get_reg_addr(void __iomem *const npu_regs,
					     const u32 offset)
{
	return ((uint8_t __iomem *)npu_regs + offset);
}

void ethosu_direct_npu_write_reg(struct ethosu_direct_device *edirect_dev,
				 const uint32_t offset,
				 const uint32_t value)
{
	if (WARN_ON(!edirect_dev))
		return;

	if (WARN_ON(!edirect_dev->reg_base))
		return;

	iowrite32(value, ethosu_direct_npu_get_reg_addr(edirect_dev->reg_base,
							offset));
}

uint32_t ethosu_direct_npu_read_reg(
	const struct ethosu_direct_device *edirect_dev,
	const uint32_t offset)
{
	if (WARN_ON(!edirect_dev))
		return 0;

	if (WARN_ON(!edirect_dev->reg_base))
		return 0;

	return ioread32(ethosu_direct_npu_get_reg_addr(edirect_dev->reg_base,
						       offset));
}

static void ethosu_direct_npu_set_region_cfg(
	struct ethosu_direct_device *edirect_dev)
{
	const struct qconfig_r qcfg = {
		.cmd_region0 = edirect_dev->mem_config.cs_region
	};
	const struct regioncfg_r rcfg = {
		.region0 = edirect_dev->mem_config.region_cfgs[0],
		.region1 = edirect_dev->mem_config.region_cfgs[1],
		.region2 = edirect_dev->mem_config.region_cfgs[2],
		.region3 = edirect_dev->mem_config.region_cfgs[3],
		.region4 = edirect_dev->mem_config.region_cfgs[4],
		.region5 = edirect_dev->mem_config.region_cfgs[5],
		.region6 = edirect_dev->mem_config.region_cfgs[6],
		.region7 = edirect_dev->mem_config.region_cfgs[7]
	};

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_QCONFIG, qcfg.word);
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_REGIONCFG, rcfg.word);
}

bool ethosu_is_npu_running(struct ethosu_direct_device *edirect_dev)
{
	struct status_r status = { .word = 0 };

	status.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_STATUS);

	return !!status.state;
}

static bool is_access_state_set(struct ethosu_direct_device *edirect_dev)
{
	struct prot_r prot = { 0 };

	prot.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_PROT);

	return prot.active_CSL == SECURITY_LEVEL_NON_SECURE &&
	       prot.active_CPL == PRIVILEGE_LEVEL_PRIVILEGED;
}

/**
 * ethosu_direct_npu_set_clock_and_power() - Set clk and power.
 * @edirect_dev: Pointer to the direct device.
 *
 * Enable/Disable using the clock q-interface & requester clock gate.
 * Enable/Disable using the power q-interface.
 */
static void ethosu_direct_npu_set_clock_and_power(
	struct ethosu_direct_device *edirect_dev,
	bool is_clk_q_iface_en,
	bool is_pwr_q_iface_en)
{
	struct cmd_r cmd = { 0 };

	cmd.word =
		ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_CMD) &
		NPU_CMD_PWR_CLK_MASK;

	cmd.clock_q_enable = is_clk_q_iface_en ? 1 : 0;
	cmd.power_q_enable = is_pwr_q_iface_en ? 1 : 0;

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_CMD, cmd.word);
}

/**
 * ethosu_direct_npu_soft_reset() - Soft reset & set protection level.
 * @edirect_dev: Pointer to the direct device.
 *
 * This function
 *   1. Performs a soft reset,
 *   2. Sets the security and privilege level,
 *   3. Verifies if the access state is set properly and
 *   4. Sets the clock and power
 * Note that after a soft-reset, the NPU is unconditionally powered
 * until the next CMD gets written.
 *
 * Return: 0 for success. Negative error code for failure.
 */
int ethosu_direct_npu_soft_reset(struct ethosu_direct_device *edirect_dev,
				 bool is_clk_q_iface_en,
				 bool is_pwr_q_iface_en)
{
	uint32_t cnt;
	struct device *dev = &edirect_dev->edev.dev;
	struct status_r status;
	const struct reset_r reset = {
		.pending_CPL = PRIVILEGE_LEVEL_PRIVILEGED,
		.pending_CSL = SECURITY_LEVEL_NON_SECURE
	};

	atomic_set(&edirect_dev->running, 0);
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_RESET, reset.word);

	for (cnt = 0; cnt < ETHOSU_RESET_TIMEOUT_US; cnt++) {
		status.word =
			ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_STATUS);
		if (status.reset_status == 0)
			break;

		udelay(ETHOSU_RESET_WAIT_US);
	}

	status.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_STATUS);
	if (status.reset_status != 0) {
		dev_err(dev, "Soft reset timed out\n");

		return -ETIMEDOUT;
	}

	if (!is_access_state_set(edirect_dev)) {
		dev_err(dev,
			"Failed to switch security state and privilege level\n");

		return -EINVAL;
	}

	ethosu_direct_npu_set_clock_and_power(edirect_dev,
					      is_clk_q_iface_en,
					      is_pwr_q_iface_en);

	return 0;
}

/**
 * ethosu_reset_and_prepare_npu() - Reset and configure the NPU.
 * @edirect_dev: Pointer to the Ethos-U direct device.
 * @is_clk_q_iface_en: Enable/Disable using the clock q-interface &
 *                     requester clock gate
 * @is_pwr_q_iface_en: Enable/Disable using the power q-interface.
 *
 * In addition to soft reset, this function also configures the memory
 * for further usage of NPU
 *
 * Return: 0 for success. Negative error code for failure.
 */
int ethosu_reset_and_prepare_npu(struct ethosu_direct_device *edirect_dev,
				 bool is_clk_q_iface_en,
				 bool is_pwr_q_iface_en)
{
	int ret;

	if (!edirect_dev || !edirect_dev->npu_ops)
		return -EFAULT;

	ret = ethosu_direct_npu_soft_reset(edirect_dev, is_clk_q_iface_en,
					   is_pwr_q_iface_en);
	if (ret) {
		dev_err(&edirect_dev->edev.dev,
			"Soft reset of the NPU failed.\n");

		return ret;
	}

	ethosu_direct_npu_set_region_cfg(edirect_dev);

	ret = ethosu_direct_call_npu_op(edirect_dev, config_mem, edirect_dev);
	if (ret)
		return ret;

	atomic_set(&edirect_dev->running, 1);

	return 0;
}

/**
 * ethosu_direct_npu_handle_interrupt() - Interrupt handler.
 * @edirect_dev: Pointer to the direct device.
 *
 * Clears the irq and stores the interrupt status register value
 * to edirect_dev struct for further processing in the bottom half.
 *
 * Context: Interrupt context
 *
 * Return: None
 */
int ethosu_direct_npu_handle_interrupt(struct ethosu_direct_device *edirect_dev)
{
	struct cmd_r cmd;
	struct status_r status;

	status.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_STATUS);

	if (!status.irq_raised)
		return IRQ_NONE;

	/* Clear interrupt */
	cmd.word =
		ethosu_direct_npu_read_reg(edirect_dev,
					   NPU_REG_CMD) & NPU_CMD_PWR_CLK_MASK;
	cmd.clear_irq = 1;
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_CMD, cmd.word);

	atomic_or(status.word, &edirect_dev->irq_status);

	return IRQ_HANDLED;
}

void ethosu_direct_npu_get_id(struct ethosu_direct_device *edirect_dev,
			      struct ethosu_id *id_info)
{
	struct id_r id = { 0 };

	id.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_ID);

	id_info->arch_major_rev = id.arch_major_rev;
	id_info->arch_minor_rev = id.arch_minor_rev;
	id_info->arch_patch_rev = id.arch_patch_rev;

	id_info->product_major = id.product_major;
	id_info->version_major = id.version_major;
	id_info->version_minor = id.version_minor;
	id_info->version_status = id.version_status;
}

int ethosu_direct_npu_verify_optimizer_id(
	struct ethosu_direct_device *edirect_dev,
	uint32_t id)
{
	struct device *dev = &edirect_dev->edev.dev;
	const struct id_r opt_id = { .word = id };
	struct id_r npu_id = { 0 };

	npu_id.word = ethosu_direct_npu_read_reg(edirect_dev, NPU_REG_ID);

	DEV_DBG_ARCH_ID_R(dev, "NPU arch", npu_id);
	DEV_DBG_ARCH_ID_R(dev, "Requested arch", opt_id);

	if ((npu_id.arch_major_rev != opt_id.arch_major_rev) ||
	    (npu_id.arch_minor_rev < opt_id.arch_minor_rev)) {
		dev_err(dev,
			"NPU arch mismatch. npu.arch=%u.%u.%u, requested.arch=%u.%u.%u",
			npu_id.arch_major_rev,
			npu_id.arch_minor_rev,
			npu_id.arch_patch_rev,
			opt_id.arch_major_rev,
			opt_id.arch_minor_rev,
			opt_id.arch_patch_rev);

		return -EINVAL;
	}

	return 0;
}

int ethosu_direct_npu_handle_common_irq_status(
	struct ethosu_direct_device *edirect_dev,
	struct status_r *status)
{
	/* Read and clear the IRQ status bits. */
	status->word = atomic_xchg(&edirect_dev->irq_status, 0);
	/* If a fault has occured, the NPU needs to be reset */
	if (status->bus_status || status->cmd_parse_error ||
	    status->ecc_fault || !status->cmd_end_reached) {
		dev_err(&edirect_dev->edev.dev,
			"Irq status %x",
			status->word);

		return -EFAULT;
	}

	return 0;
}

int ethosu_direct_npu_handle_command_stream(
	struct ethosu_direct_device *edirect_dev,
	dma_addr_t cmd_addr,
	uint32_t cmd_size,
	dma_addr_t *base_addrs,
	size_t num_base_addr)
{
	struct device *dev = &edirect_dev->edev.dev;
	size_t i;
	uint32_t bp_offset_lo = NPU_REG_BASEP_BASE;
	uint32_t bp_offset_hi = NPU_REG_BASEP_BASE + sizeof(uint32_t);
	struct cmd_r cmd = { 0 };

	if (num_base_addr > NPU_REG_BASEP_ARRLEN) {
		dev_err(dev,
			"Handle command stream. Invalid number of base pointer addresses");

		return -EINVAL;
	}

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_QBASE,
				    cmd_addr & 0xFFFFFFFF);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_QBASE_HI,
				    cmd_addr >> 32);
#else
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_QBASE_HI, 0);
#endif
	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_QSIZE, cmd_size);

	for (i = 0; i < num_base_addr; ++i) {
		const uint32_t addr_lo = base_addrs[i] & 0xFFFFFFFF;
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		const uint32_t addr_hi = base_addrs[i] >> 32;
#else
		const uint32_t addr_hi = 0;
#endif
		ethosu_direct_npu_write_reg(edirect_dev, bp_offset_lo, addr_lo);
		ethosu_direct_npu_write_reg(edirect_dev, bp_offset_hi, addr_hi);

		bp_offset_lo += sizeof(struct basep_r);
		bp_offset_hi += sizeof(struct basep_r);
	}

	for (; i < NPU_REG_BASEP_ARRLEN; ++i) {
		ethosu_direct_npu_write_reg(edirect_dev, bp_offset_lo, 0U);
		ethosu_direct_npu_write_reg(edirect_dev, bp_offset_hi, 0U);

		bp_offset_lo += sizeof(struct basep_r);
		bp_offset_hi += sizeof(struct basep_r);
	}

	cmd.word =
		ethosu_direct_npu_read_reg(edirect_dev,
					   NPU_REG_CMD) & NPU_CMD_PWR_CLK_MASK;
	cmd.transition_to_running_state = 1;

	ethosu_direct_npu_write_reg(edirect_dev, NPU_REG_CMD, cmd.word);

	return 0;
}
