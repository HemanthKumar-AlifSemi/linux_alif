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

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <direct/ethosu_direct_device.h>

#include <common/ethosu_buffer.h>
#include <common/ethosu_dma_mem.h>
#include <direct/ethosu_direct_inference.h>
#include <direct/ethosu_direct_network.h>
#include <direct/interface/ethosu_direct_interface_common.h>
#include <direct/npu/ethosu_direct_npu_common.h>
#include <direct/npu/ethosu_direct_npu_u65.h>
#include <direct/npu/ethosu_direct_npu_u85.h>

#include <linux/dma-direct.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/types.h>

#define ETHOSU_PLATFORM_IRQ_INDEX 0

#define ETHOSU_DIRECT_NPU_DEV_STR "Direct NPU"

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_direct_device *to_direct_device(struct ethosu_device *edev)
{
	if (edev->type == ETHOSU_DEVICE_TYPE_DIRECT)
		return container_of(edev, struct ethosu_direct_device, edev);

	return NULL;
}

/**
 * ethosu_irq_bottom() - IRQ bottom handler
 * @work:	Work structure part of NPU direct device
 *
 * Execute bottom half of interrupt in process context.
 */
static void ethosu_irq_bottom(struct work_struct *work)
{
	bool job_success = false;
	struct ethosu_direct_device *edirect_dev = container_of(work,
								struct ethosu_direct_device,
								irq_work);

	job_success = (ethosu_direct_call_npu_op(edirect_dev, handle_irq_status,
						 edirect_dev) == 0);
	if (!job_success)
		ethosu_direct_device_reset(edirect_dev);

	ethosu_direct_inference_done(edirect_dev, job_success);
}

static irqreturn_t ethosu_irq_top_handler(const int irq,
					  void *dev)
{
	struct ethosu_direct_device *edirect_dev =
		(struct ethosu_direct_device *)dev;
	int ret = ethosu_direct_npu_handle_interrupt(edirect_dev);

	if (ret == IRQ_HANDLED)
		/* Defer to work queue. */
		queue_work(edirect_dev->irq_work_q, &edirect_dev->irq_work);

	return ret;
}

static void ethosu_remove_interrupt(struct ethosu_direct_device *edirect_dev)
{
	struct device *dev = &edirect_dev->edev.dev;

	if (edirect_dev->irq_num) {
		devm_free_irq(dev, edirect_dev->irq_num, edirect_dev);
		edirect_dev->irq_num = 0;
	}

	if (edirect_dev->irq_work_q) {
		destroy_workqueue(edirect_dev->irq_work_q);
		edirect_dev->irq_work_q = NULL;
	}
}

static int ethosu_get_irq_from_dt(struct platform_device *pdev,
				  unsigned int *irq_num,
				  unsigned long *irq_flags)
{
	int ret;
	struct irq_data *data;

	ret = platform_get_irq(pdev, ETHOSU_PLATFORM_IRQ_INDEX);
	if (ret < 0)
		return ret;

	data = irq_get_irq_data(ret);
	if (!data)
		return -EINVAL;

	*irq_num = ret;
	*irq_flags = IRQF_SHARED |
		     (irqd_get_trigger_type(data) & IRQF_TRIGGER_MASK);

	return 0;
}

static int ethosu_setup_irq(struct platform_device *pdev)
{
	int ret;
	struct device *dev;
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(&pdev->dev);
	unsigned long irq_flags;

	if (!edirect_dev)
		return -EFAULT;

	dev = &edirect_dev->edev.dev;

	ret = ethosu_get_irq_from_dt(pdev, &edirect_dev->irq_num, &irq_flags);
	if (ret) {
		dev_err(dev, "Failed to get IRQ from device tree\n");

		return ret;
	}

	edirect_dev->irq_work_q = create_singlethread_workqueue(
		"ethosu_irq_workqueue");
	if (!edirect_dev->irq_work_q) {
		dev_err(dev, "Failed to create IRQ work queue\n");

		return -EINVAL;
	}

	INIT_WORK(&edirect_dev->irq_work, ethosu_irq_bottom);

	dev_dbg(dev, "Requesting IRQ %d with flags 0x%lx\n",
		edirect_dev->irq_num, irq_flags);

	ret = devm_request_irq(dev, edirect_dev->irq_num,
			       &ethosu_irq_top_handler,
			       irq_flags,
			       ETHOSU_DIRECT_NPU_DEV_STR,
			       edirect_dev);
	if (ret) {
		dev_err(dev, "Failed to request IRQ %d\n",
			edirect_dev->irq_num);

		destroy_workqueue(edirect_dev->irq_work_q);
		edirect_dev->irq_work_q = NULL;

		return ret;
	}

	return 0;
}

static int ethosu_direct_device_capabilities_get(struct ethosu_device *edev,
						 struct ethosu_uapi_device_capabilities *cap)
{
	struct ethosu_direct_device *edirect_dev;
	struct device *dev;
	struct ethosu_config hw = { 0 };
	struct ethosu_id id = { 0 };

	if (!edev)
		return -EFAULT;

	dev = &edev->dev;

	edirect_dev = to_direct_device(edev);
	if (!edirect_dev)
		return -EFAULT;

	device_lock(dev);
	ethosu_direct_npu_get_id(edirect_dev, &id);
	ethosu_direct_call_npu_op(edirect_dev, get_hw_cfg, edirect_dev, &hw);

	cap->hw_id.version_status = id.version_status;
	cap->hw_id.version_minor = id.version_minor;
	cap->hw_id.version_major = id.version_major;
	cap->hw_id.product_major = id.product_major;
	cap->hw_id.arch_patch_rev = id.arch_patch_rev;
	cap->hw_id.arch_minor_rev = id.arch_minor_rev;
	cap->hw_id.arch_major_rev = id.arch_major_rev;

	cap->driver_patch_rev = ETHOSU_KERNEL_DRIVER_VERSION_PATCH;
	cap->driver_minor_rev = ETHOSU_KERNEL_DRIVER_VERSION_MINOR;
	cap->driver_major_rev = ETHOSU_KERNEL_DRIVER_VERSION_MAJOR;

	cap->network_mask = 0U;

	cap->hw_cfg.macs_per_cc = hw.macs_per_cc;
	cap->hw_cfg.cmd_stream_version = hw.cmd_stream_version;
	cap->hw_cfg.custom_dma = hw.custom_dma;
	cap->hw_cfg.type = ETHOSU_UAPI_DEVICE_DIRECT;

	device_unlock(dev);

	dev_dbg(dev,
		"Capabilities response vs%u v%u.%u p%u av%u.%u.%u dv%u.%u.%u mcc%u csv%u cd%u",
		cap->hw_id.version_status,
		cap->hw_id.version_major,
		cap->hw_id.version_minor,
		cap->hw_id.product_major,
		cap->hw_id.arch_major_rev,
		cap->hw_id.arch_minor_rev,
		cap->hw_id.arch_patch_rev,
		cap->driver_major_rev,
		cap->driver_minor_rev,
		cap->driver_patch_rev,
		cap->hw_cfg.macs_per_cc,
		cap->hw_cfg.cmd_stream_version,
		cap->hw_cfg.custom_dma);

	return 0;
}

int ethosu_direct_device_resume(struct device *dev)
{
	struct ethosu_direct_device *edirect_dev;
	struct ethosu_device *edev;

	edirect_dev = dev_get_drvdata(dev);
	if (!edirect_dev) {
		dev_err(dev, "edirect_dev is NULL\n");

		return -EFAULT;
	}

	edev = &edirect_dev->edev;
	pm_runtime_mark_last_busy(&edev->dev);

	return ethosu_direct_device_reset(edirect_dev);
}

int ethosu_direct_device_suspend_noirq(struct device *dev)
{
	struct ethosu_direct_device *edirect_dev;
	struct ethosu_device *edev;
	int ret;

	edirect_dev = dev_get_drvdata(dev);
	if (!edirect_dev) {
		dev_err(dev, "edirect_dev is NULL\n");

		return -EFAULT;
	}

	edev = &edirect_dev->edev;
	pm_runtime_mark_last_busy(&edev->dev);
	pm_runtime_put(&edev->dev);

	ret = ethosu_direct_npu_soft_reset(edirect_dev, true, true);
	if (ret)
		dev_err(dev,
			"Failed to soft reset before suspend no irq. ret=%d",
			ret);

	return 0;
}

int ethosu_direct_device_runtime_resume(struct device *dev)
{
	struct ethosu_direct_device *edirect_dev;

	edirect_dev = dev_get_drvdata(dev);
	if (!edirect_dev) {
		dev_err(dev, "edirect_dev is NULL\n");

		return -EFAULT;
	}

	return ethosu_direct_device_reset(edirect_dev);
}

int ethosu_direct_device_runtime_suspend(struct device *dev)
{
	struct ethosu_direct_device *edirect_dev;
	int ret = 0;

	edirect_dev = dev_get_drvdata(dev);
	if (!edirect_dev) {
		dev_err(dev, "edirect_dev is NULL\n");
		ret = -EFAULT;
		goto exit_rpm_suspend;
	}

	if (ethosu_is_npu_running(edirect_dev)) {
		ret = -EBUSY;
		goto exit_rpm_suspend;
	}

	ret = ethosu_direct_npu_soft_reset(edirect_dev, true, true);
	if (ret)
		dev_err(dev,
			"Failed to soft reset before runtime suspend. ret=%d",
			ret);

exit_rpm_suspend:
	dev_dbg(dev, "Runtime suspend: %d\n", ret);

	return ret;
}

static int ethosu_direct_device_rpm_get_sync(struct ethosu_device *edev)
{
	int ret = 0;

	if (!edev)
		return -EFAULT;

	ret = pm_runtime_resume_and_get(&edev->dev);
	if (ret) {
		dev_err(&edev->dev,
			"%s %d Failed to get runtime PM with error %d\n",
			__func__, __LINE__, ret);
		pm_runtime_disable(&edev->dev);
	}

	return 0;
}

static int ethosu_direct_device_rpm_put(struct ethosu_device *edev)
{
	int ret = 0;

	if (!edev)
		return -EFAULT;

	pm_runtime_mark_last_busy(&edev->dev);
	ret = pm_runtime_put(&edev->dev);
	if (ret) {
		dev_err(&edev->dev,
			"%s %d Failed to get runtime PM with error %d\n",
			__func__, __LINE__, ret);
		pm_runtime_disable(&edev->dev);
	}

	return 0;
}

static const struct ethosu_device_ops direct_ops = {
	.capabilities_get           =
		&ethosu_direct_device_capabilities_get,
	.ping                       = NULL,      /* Not supported */
	.network_create             = &ethosu_direct_network_create,
	.network_destroy            = &ethosu_direct_network_destroy,
	.network_check_index        = NULL,      /* Not supported */
	.network_setup              = &ethosu_direct_network_setup,
	.network_info               = NULL,      /* Not supported */
	.inference_create           = &ethosu_direct_inference_create,
	.inference_destroy          = &ethosu_direct_inference_destroy,
	.inference_setup            = &ethosu_direct_inference_setup,
	.inference_release          = &ethosu_direct_inference_release,
	.inference_send             = &ethosu_direct_inference_send,
	.inference_cancel           = &ethosu_direct_inference_cancel,
	.rpm_get_sync               = &ethosu_direct_device_rpm_get_sync,
	.rpm_put_and_mark_last_busy = &ethosu_direct_device_rpm_put,
};

static const struct ethosu_device_buffer_config buffer_config = {
	.alignment = 16U,
	.max_size  = DMA_BIT_MASK(40U),
};

static int ethosu_set_npu_ops(struct platform_device *pdev)
{
	uint32_t config = 0;
	uint32_t prod_id = 0;
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(&pdev->dev);
	struct device *dev;

	if (!edirect_dev)
		return -EFAULT;

	dev = &edirect_dev->edev.dev;

	config = ioread32(ethosu_direct_npu_get_reg_addr(edirect_dev->reg_base,
							 ETHOSU_NPU_REG_CONFIG_OFFSET));

	prod_id = ETHOSU_GET_PROD_ID(config);
	switch (prod_id) {
	case ETHOSU_PRODUCT_U65:
		ethosu_direct_npu_u65_ops_set(edirect_dev);
		break;
	case ETHOSU_PRODUCT_U85:
		ethosu_direct_npu_u85_ops_set(edirect_dev);
		break;
	default:
		dev_err(dev, "Invalid NPU ID: %u\n", prod_id);

		return -ENODEV;
	}

	return 0;
}

static int ethosu_get_sram_region_from_dt(struct platform_device *pdev)
{
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(&pdev->dev);
	struct device *dev;
	struct device_node *sram_np;
	struct reserved_mem *sram_reserve_mem;
	void __iomem *cpu_addr;
	size_t size;
	dma_addr_t da;
	int ret;

	if (!edirect_dev)
		return -EFAULT;

	dev = &edirect_dev->edev.dev;
	sram_np = of_parse_phandle(pdev->dev.of_node, "sram", 0);
	if (!sram_np) {
		dev_err(dev, "sram property not found.");

		return -ENODEV;
	}

	sram_reserve_mem = of_reserved_mem_lookup(sram_np);
	if (!sram_reserve_mem) {
		dev_err(dev, "Reserved mem lookup failed for sram.");

		ret = -ENOMEM;
		goto done;
	}

	if (sram_reserve_mem->size > edirect_dev->edev.buffer_config.max_size) {
		dev_warn(dev,
			 "SRAM size exceeds supported NPU size. Trucating size to %zx",
			 edirect_dev->edev.buffer_config.max_size);
		size = edirect_dev->edev.buffer_config.max_size;
	} else {
		size = sram_reserve_mem->size;
	}

	da = translate_phys_to_dma(dev, sram_reserve_mem->base);
	if (da == DMA_MAPPING_ERROR) {
		dev_err(dev, "No mapping found for SRAM PA. pa=%pa, size=%zx",
			&sram_reserve_mem->base, size);

		ret = -ENOMEM;
		goto done;
	}

	cpu_addr = devm_ioremap(dev, sram_reserve_mem->base,
				size);
	if (IS_ERR(cpu_addr)) {
		dev_err(dev,
			"Failed to map SRAM base=%pa, size=%zx\n",
			&sram_reserve_mem->base, size);
		ret = -EFAULT;
		goto done;
	}

	edirect_dev->sram.cpu_addr = cpu_addr;
	edirect_dev->sram.size = size;
	edirect_dev->sram.npu_addr = da;
	dev_dbg(dev,
		"SRAM PA = 0x%pK, DMA addr = 0x%pad, Size = 0x%zx",
		edirect_dev->sram.cpu_addr,
		&edirect_dev->sram.npu_addr,
		edirect_dev->sram.size);

	ret = 0;
done:
	of_node_put(sram_np);

	return ret;
}

static int ethosu_get_region_configs_from_dt(struct platform_device *pdev)
{
	struct device_node *np;
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(&pdev->dev);
	struct device *dev;
	int ret;

	if (!edirect_dev)
		return -EFAULT;

	dev = &edirect_dev->edev.dev;
	np = pdev->dev.of_node;

	ret = of_property_count_u32_elems(np, "region-cfgs");
	if (ret < 0) {
		dev_err(dev,
			"Failed to read num of elements in region-cfgs with error %d\n",
			ret);

		return ret;
	}

	if (ret != ETHOSU_DEV_MAX_NUM_REGIONS) {
		dev_err(dev,
			"Wrong number of region-cfgs elements. Expected %d got %d\n",
			ETHOSU_DEV_MAX_NUM_REGIONS, ret);

		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "region-cfgs",
					 edirect_dev->mem_config.region_cfgs,
					 ret);
	if (ret < 0) {
		dev_err(dev,
			"Failed to read values in region-cfgs with error %d\n",
			ret);

		return ret;
	}

	ret = of_property_read_u32_index(np, "cs-region", 0,
					 &edirect_dev->mem_config.cs_region);
	if (ret)
		return ret;

	return 0;
}

static int ethosu_map_iomem(struct platform_device *pdev)
{
	struct resource *res;
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(&pdev->dev);
	struct device *dev;
	resource_size_t rsize;
	void __iomem *ptr;

	if (!edirect_dev)
		return -EFAULT;

	dev = &edirect_dev->edev.dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Failed to find I/O memory\n");

		return -ENODEV;
	}

	rsize = resource_size(res);
	dev_dbg(dev,
		"Mapping resource. name=%s, start=%pa, size=%pa\n",
		res->name, &res->start, &rsize);

	/* Reserve address space */
	if (!devm_request_mem_region(dev, res->start, rsize, res->name)) {
		dev_err(dev,
			"Can't request mem region for resource %pR\n",
			res);

		return -EBUSY;
	}

	/* Map address space */
	ptr = devm_ioremap(dev, res->start, rsize);
	if (IS_ERR(ptr)) {
		dev_err(dev,
			"Failed to map name=%s, start=%pa, size=%pa\n",
			res->name, &res->start, &rsize);
		devm_release_mem_region(dev, res->start, rsize);

		return -ENOMEM;
	}

	edirect_dev->reg_base = ptr;

	return 0;
}

static int ethosu_direct_device_config_from_dt(struct platform_device *pdev)
{
	int ret = 0;
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(&pdev->dev);
	struct device *dev;

	if (!edirect_dev)
		return -EFAULT;

	dev = &edirect_dev->edev.dev;

	ret = ethosu_get_sram_region_from_dt(pdev);
	if (ret) {
		dev_err(dev,
			"Failed to parse SRAM address and size from device tree. ret=%d",
			ret);

		return ret;
	}

	ret = ethosu_get_region_configs_from_dt(pdev);
	if (ret) {
		dev_err(dev,
			"Failed to parse region configuration from device tree. ret=%d",
			ret);

		return ret;
	}

	ret = ethosu_map_iomem(pdev);
	if (ret) {
		dev_err(dev,
			"Failed to map I/O memory from device tree. ret=%d",
			ret);

		return ret;
	}

	ret = ethosu_set_npu_ops(pdev);
	if (ret) {
		dev_err(dev, "Failed to find compatible NPU ret=%d", ret);

		return ret;
	}

	ret = ethosu_setup_irq(pdev);
	if (ret) {
		dev_err(dev, "Failed to setup interrupts. ret=%d", ret);

		return ret;
	}

	return 0;
}

int ethosu_direct_device_init(struct platform_device *pdev,
			      struct class *class,
			      dev_t devt)
{
	struct device *dev = &pdev->dev;
	struct ethosu_direct_device *edirect_dev;
	struct ethosu_device *edev;
	int ret;

	edirect_dev = devm_kzalloc(dev, sizeof(*edirect_dev), GFP_KERNEL);
	if (!edirect_dev) {
		dev_err(dev, "Failed to allocate Arm Ethos-U Direct device");

		return -ENOMEM;
	}

	ret = ethosu_device_init(&edirect_dev->edev, ETHOSU_DEVICE_TYPE_DIRECT,
				 &direct_ops, &buffer_config, dev, dev, class,
				 &devt);
	if (ret)
		goto free_edirect_dev;

	dev = &edirect_dev->edev.dev;
	dev_set_drvdata(&pdev->dev, edirect_dev);

	ret = ethosu_direct_device_config_from_dt(pdev);
	if (ret) {
		dev_err(dev,
			"Failed to configure device from device tree\n");

		goto device_unregister;
	}

	dev_dbg(&pdev->dev, "Populate other child platform devices\n");
	ret = of_platform_default_populate(pdev->dev.of_node, NULL, &pdev->dev);
	if (ret) {
		dev_err(dev, "Failed to populate child devices\n");

		goto device_unregister;
	}

	ethosu_direct_inference_queue_init(&edirect_dev->inf_queue);

	ret = ethosu_direct_device_reset(edirect_dev);
	if (ret) {
		dev_err(dev, "Failed to reset NPU %d\n", ret);

		goto device_unregister;
	}

	ret = ethosu_device_finalize(&edirect_dev->edev, devt);
	if (ret) {
		dev_err(dev,
			"Arm Ethos-U device finalize failed with error %d\n",
			ret);

		goto device_unregister;
	}

	edev = &edirect_dev->edev;
	pm_runtime_set_autosuspend_delay(&edev->dev,
					 ETHOSU_AUTOSUSPEND_DELAY_MS);
	pm_runtime_use_autosuspend(&edev->dev);
	pm_runtime_get_noresume(&edev->dev);
	pm_runtime_set_active(&edev->dev);
	devm_pm_runtime_enable(&edev->dev);

	dev_info(dev,
		 "Created Arm Ethos-U Direct device. name=%s, major=%d, minor=%d",
		 dev_name(&pdev->dev), MAJOR(devt), MINOR(devt));

	return 0;

device_unregister:
	ethosu_device_deinit(&edirect_dev->edev);

free_edirect_dev:
	devm_kfree(&pdev->dev, edirect_dev);

	return ret;
}

void ethosu_direct_device_deinit(struct platform_device *pdev)
{
	struct device *dev;
	struct ethosu_direct_device *edirect_dev;

	if (!pdev)
		return;

	dev = &pdev->dev;
	edirect_dev = dev_get_drvdata(dev);
	if (!edirect_dev)
		return;

	ethosu_remove_interrupt(edirect_dev);
	ethosu_device_deinit(&edirect_dev->edev);
	devm_kfree(dev, edirect_dev);

	platform_set_drvdata(pdev, NULL);

	dev_dbg(dev, "Device deinit\n");
}

int ethosu_direct_device_verify_hw_config(struct ethosu_device *edev,
					  uint32_t config,
					  uint32_t id,
					  uint64_t cache_size)
{
	struct device *dev;
	struct ethosu_direct_device *edirect_dev;

	if (!edev)
		return -EFAULT;

	dev = &edev->dev;

	edirect_dev = to_direct_device(edev);
	if (!edirect_dev)
		return -EFAULT;

	if (cache_size > edirect_dev->sram.size) {
		dev_err(dev,
			"NPU HW config mismatch. %llu bytes of cache requested but NPU has %zu\n",
			cache_size, edirect_dev->sram.size);

		return -EINVAL;
	}

	return ethosu_direct_call_npu_op(edirect_dev, verify_hw_cfg,
					 edirect_dev, config, id);
}

int ethosu_direct_device_run_inference(struct ethosu_direct_device *edirect_dev,
				       struct ethosu_direct_inference *d_inf)
{
	struct ethosu_direct_network *d_net;
	dma_addr_t base_addrs[5] = { 0U };

	if (!edirect_dev)
		return -EFAULT;

	if (!d_inf)
		return -EFAULT;

	d_net = to_direct_network(d_inf->inf.net);
	if (!d_net)
		return -EFAULT;

	base_addrs[0] = ethosu_buffer_to_dma_addr(d_inf->constant);
	base_addrs[1] = ethosu_buffer_to_dma_addr(d_inf->intermediate);
	base_addrs[2] = edirect_dev->sram.npu_addr;
	base_addrs[3] = ethosu_buffer_to_dma_addr(d_inf->inf.ifm[0]);
	base_addrs[4] = ethosu_buffer_to_dma_addr(d_inf->inf.ofm[0]);

	return ethosu_direct_npu_handle_command_stream(edirect_dev,
						       d_net->cmd_addr,
						       d_net->cmd_length,
						       base_addrs,
						       ARRAY_SIZE(base_addrs));
}

int ethosu_direct_device_reset(struct ethosu_direct_device *edirect_dev)
{
	int ret;

	if (!edirect_dev)
		return -EFAULT;

	ret = ethosu_reset_and_prepare_npu(edirect_dev, false, false);
	if (ret)
		return ret;

	ethosu_direct_device_clear_sram(edirect_dev);

	return 0;
}

void ethosu_direct_device_clear_sram(struct ethosu_direct_device *edirect_dev)
{
	if (!edirect_dev)
		return;

	memset_io(edirect_dev->sram.cpu_addr, 0, edirect_dev->sram.size);
}

void ethosu_direct_device_pmu_setup(struct ethosu_direct_device *edirect_dev,
				    bool enable_cycle_count,
				    uint8_t *events,
				    size_t num_events)
{
	struct device *dev;

	if (!edirect_dev)
		return;

	dev = &edirect_dev->edev.dev;

	if (ethosu_direct_call_npu_op(edirect_dev, pmu_setup, edirect_dev,
				      enable_cycle_count, events, num_events))
		dev_err(dev, "PMU setup: not supported\n");
}

void ethosu_direct_device_pmu_get_values(
	struct ethosu_direct_device *edirect_dev,
	bool read_cycle_count,
	uint64_t *cycle_count,
	uint64_t *event_count,
	size_t num_events)
{
	struct device *dev;

	if (!edirect_dev)
		return;

	dev = &edirect_dev->edev.dev;

	if (ethosu_direct_call_npu_op(edirect_dev, pmu_get_values, edirect_dev,
				      read_cycle_count, cycle_count,
				      event_count,
				      num_events))
		dev_err(dev, "PMU get values: not supported\n");
}

void ethosu_direct_device_pmu_disable(struct ethosu_direct_device *edirect_dev)
{
	struct device *dev;

	if (!edirect_dev)
		return;

	dev = &edirect_dev->edev.dev;

	if (ethosu_direct_call_npu_op(edirect_dev, pmu_disable, edirect_dev))
		dev_err(dev, "PMU disable: not supported\n");
}
