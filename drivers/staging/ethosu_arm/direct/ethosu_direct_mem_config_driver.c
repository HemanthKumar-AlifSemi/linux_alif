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

#include <direct/config/ethosu_direct_mem_config_driver.h>

#include <direct/ethosu_direct_device.h>

#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

#define MEM_CONFIG_DRIVER_NAME   "arm,ethosu-mem-config"

/****************************************************************************
 * Functions
 ****************************************************************************/

static int ethosu_get_port_config_from_dt(const struct device_node *np,
					  const char *propname,
					  struct ethosu_port_cfg *cfg)
{
	/*
	 * Port config property
	 * <beats outstanding_read outstanding_write>
	 * NPU takes outstanding read/write as value - 1
	 */
	int ret = of_property_read_u32_index(np, propname, 0,
					     &cfg->beats);

	if (ret)
		return ret;

	ret = of_property_read_u32_index(np, propname, 1,
					 &cfg->outstanding_read);
	if (ret)
		return ret;

	if (cfg->outstanding_read)
		--cfg->outstanding_read;

	ret = of_property_read_u32_index(np, propname, 2,
					 &cfg->outstanding_write);
	if (ret)
		return ret;

	if (cfg->outstanding_write)
		--cfg->outstanding_write;

	return 0;
}

static int ethosu_config_mem_attr_from_dt(struct platform_device *pdev)
{
	const uint32_t num_elements = sizeof(struct ethosu_mem_attr) /
				      sizeof(uint32_t);
	const struct device_node *np = pdev->dev.of_node;
	struct ethosu_direct_device *edirect_dev = dev_get_drvdata(
		pdev->dev.parent);
	int ret;
	uint32_t cnt;

	ret = ethosu_get_port_config_from_dt(np, "sram",
					     &edirect_dev->mem_config.sram_cfgs);
	if (ret)
		return ret;

	ret = ethosu_get_port_config_from_dt(np, "ext",
					     &edirect_dev->mem_config.ext_cfgs);
	if (ret)
		return ret;

	/*
	 * Memory attribute property
	 * <mem_domain mem_type axi_port> x 4
	 */
	for (cnt = 0; cnt < ETHOSU_DEV_MAX_MEM_ATTR; cnt++) {
		const uint32_t index = cnt * num_elements;
		struct ethosu_mem_attr *mem_attr =
			&edirect_dev->mem_config.mem_attr[cnt];

		ret = of_property_read_u32_index(np, "configs", index,
						 &mem_attr->mem_domain);
		if (ret)
			return ret;

		ret = of_property_read_u32_index(np, "configs", index + 1,
						 &mem_attr->memtype);
		if (ret)
			return ret;

		ret = of_property_read_u32_index(np, "configs", index + 2,
						 &mem_attr->axi_port);
		if (ret)
			return ret;
	}

	return 0;
}

static int ethosu_direct_mem_config_pdev_probe(struct platform_device *pdev)
{
	int ret;
	struct ethosu_direct_device *edirect_dev;

	edirect_dev = dev_get_drvdata(pdev->dev.parent);
	if (!edirect_dev)
		return -EFAULT;

	ret = ethosu_config_mem_attr_from_dt(pdev);
	if (ret) {
		dev_err(&pdev->dev,
			"NPU memory configuration from device tree failed with error %d\n",
			ret);

		return ret;
	}

	return 0;
}

static const struct of_device_id ethosu_direct_mem_config_child_pdev_match[] = {
	{ .compatible = MEM_CONFIG_DRIVER_NAME },
	{ /* Sentinel */ },
};

MODULE_DEVICE_TABLE(of, ethosu_direct_mem_config_child_pdev_match);

static struct platform_driver ethosu_direct_mem_config_pdev_driver = {
	.probe                  = &ethosu_direct_mem_config_pdev_probe,
	.driver                 = {
		.name           = MEM_CONFIG_DRIVER_NAME,
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(
			ethosu_direct_mem_config_child_pdev_match),
		.pm             = NULL,
	},
};

int ethosu_direct_mem_config_platform_driver_register(void)
{
	pr_debug("Registering %s", MEM_CONFIG_DRIVER_NAME);

	return platform_driver_register(&ethosu_direct_mem_config_pdev_driver);
}

void ethosu_direct_mem_config_platform_driver_unregister(void)
{
	pr_debug("Unregistering %s", MEM_CONFIG_DRIVER_NAME);
	platform_driver_unregister(&ethosu_direct_mem_config_pdev_driver);
}
