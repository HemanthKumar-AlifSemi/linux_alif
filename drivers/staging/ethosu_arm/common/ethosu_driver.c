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

#include <common/ethosu_device.h>
#include <direct/config/ethosu_direct_axi_config_driver.h>
#include <direct/config/ethosu_direct_mem_config_driver.h>
#include <direct/ethosu_direct_device.h>
#include <rpmsg/ethosu_rpmsg_device.h>
#include <uapi/ethosu.h>

#include <linux/bitmap.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/rpmsg.h>
#include <linux/version.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

#define ETHOSU_DRIVER_STR(s) #s
#define ETHOSU_DRIVER_VERSION_STR(major, minor, patch) \
	ETHOSU_DRIVER_STR(major) "."		       \
	ETHOSU_DRIVER_STR(minor) "."		       \
	ETHOSU_DRIVER_STR(patch)
#define ETHOSU_DRIVER_VERSION ETHOSU_DRIVER_VERSION_STR( \
		ETHOSU_KERNEL_DRIVER_VERSION_MAJOR,	 \
		ETHOSU_KERNEL_DRIVER_VERSION_MINOR,	 \
		ETHOSU_KERNEL_DRIVER_VERSION_PATCH)

#define ETHOSU_DRIVER_NAME    "ethosu"
#define ETHOSU_DIRECT_DRIVER_NAME   "arm,ethosu-direct"

#define MINOR_BASE      0 /* Minor version starts at 0 */
#define MINOR_COUNT    64 /* Allocate minor versions */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6, 3, 13)
#define ethosu_class_create(driver_name) class_create(THIS_MODULE, driver_name)
#else
#define ethosu_class_create(driver_name) class_create(driver_name)
#endif

/****************************************************************************
 * Variables
 ****************************************************************************/

static struct class *ethosu_class;

static dev_t devt;

/****************************************************************************
 * Direct driver
 ****************************************************************************/

static int ethosu_direct_probe(struct platform_device *pdev)
{
	int ret = ethosu_direct_device_init(pdev, ethosu_class, devt);

	if (ret)
		dev_err(&pdev->dev, "Failed to setup direct device. ret=%d",
			ret);

	return ret;
}

static int ethosu_direct_remove_pdev(struct platform_device *pdev)
{
	of_platform_depopulate(&pdev->dev);
	ethosu_direct_device_deinit(pdev);

	return 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6, 10, 14)
static int ethosu_direct_remove(struct platform_device *pdev)
{
	return ethosu_direct_remove_pdev(pdev);
}

#else
static void ethosu_direct_remove(struct platform_device *pdev)
{
	(void)ethosu_direct_remove_pdev(pdev);
}

#endif

static const struct dev_pm_ops ethosu_direct_pm = {
	.resume        = ethosu_direct_device_resume,
	.suspend_noirq = ethosu_direct_device_suspend_noirq,
	.restore       = NULL,
	.freeze_noirq  = NULL,
	SET_RUNTIME_PM_OPS(
		ethosu_direct_device_runtime_suspend,
		ethosu_direct_device_runtime_resume,
		NULL
		)
};

static const struct of_device_id ethosu_direct_pdev_match[] = {
	{ .compatible = ETHOSU_DIRECT_DRIVER_NAME },
	{ /* Sentinel */ },
};

MODULE_DEVICE_TABLE(of, ethosu_direct_pdev_match);

static struct platform_driver ethosu_direct_pdev_driver = {
	.driver                 = {
		.name           = ETHOSU_DIRECT_DRIVER_NAME,
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(
			ethosu_direct_pdev_match),
		.pm             = &ethosu_direct_pm,
	},
	.probe                  = &ethosu_direct_probe,
	.remove                 = &ethosu_direct_remove,
};

static int ethosu_direct_platform_driver_register(void)
{
	pr_debug("Registering %s", ETHOSU_DIRECT_DRIVER_NAME);

	return platform_driver_register(&ethosu_direct_pdev_driver);
}

static void ethosu_direct_platform_driver_unregister(void)
{
	pr_debug("Unregistering %s", ETHOSU_DIRECT_DRIVER_NAME);
	platform_driver_unregister(&ethosu_direct_pdev_driver);
}

static int register_direct_platform_drivers(void)
{
	int ret = ethosu_direct_mem_config_platform_driver_register();

	if (ret)
		return ret;

	ret = ethosu_direct_axi_config_platform_driver_register();
	if (ret)
		goto unregister_mem_config;

	ret = ethosu_direct_platform_driver_register();
	if (ret)
		goto unregister_axi_config;

	return 0;

unregister_axi_config:
	ethosu_direct_axi_config_platform_driver_unregister();

unregister_mem_config:
	ethosu_direct_mem_config_platform_driver_unregister();

	return ret;
}

static void unregister_direct_platform_drivers(void)
{
	ethosu_direct_platform_driver_unregister();
	ethosu_direct_axi_config_platform_driver_unregister();
	ethosu_direct_mem_config_platform_driver_unregister();
}

/****************************************************************************
 * Rpmsg driver
 ****************************************************************************/

static int ethosu_rpmsg_probe(struct rpmsg_device *rpdev)
{
	int ret = ethosu_rpmsg_device_init(rpdev, ethosu_class, devt);

	if (ret)
		dev_err(&rpdev->dev, "Failed to setup rpmsg device. ret=%d",
			ret);

	return ret;
}

static void ethosu_rpmsg_remove(struct rpmsg_device *rpdev)
{
	ethosu_rpmsg_device_deinit(rpdev);
}

static int ethosu_rpmsg_cb(struct rpmsg_device *rpdev,
			   void *data,
			   int len,
			   void *priv,
			   u32 src)
{
	dev_err(&rpdev->dev, "%s", __FUNCTION__);

	return -EINVAL;
}

static struct rpmsg_device_id ethosu_rpmsg_driver_id_table[] = {
	{ .name = "ethos-u-0.0" },
	{},
};

MODULE_DEVICE_TABLE(rpmsg, ethosu_rpmsg_driver_id_table);

static struct rpmsg_driver ethosu_rpmsg_driver = {
	.drv                = {
		.name       = ETHOSU_DRIVER_NAME,
		.owner      = THIS_MODULE,
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.id_table           = ethosu_rpmsg_driver_id_table,
	.probe              = ethosu_rpmsg_probe,
	.callback           = ethosu_rpmsg_cb,
	.remove             = ethosu_rpmsg_remove,
};

/****************************************************************************
 * Module init and exit
 ****************************************************************************/

static void __exit ethosu_exit(void)
{
	unregister_direct_platform_drivers();
	unregister_rpmsg_driver(&ethosu_rpmsg_driver);
	unregister_chrdev_region(devt, MINOR_COUNT);
	class_destroy(ethosu_class);
}

static int __init ethosu_init(void)
{
	int ret;

	ethosu_class = ethosu_class_create(ETHOSU_DRIVER_NAME);
	if (IS_ERR(ethosu_class)) {
		pr_err("Failed to create class '%s'.\n", ETHOSU_DRIVER_NAME);

		return PTR_ERR(ethosu_class);
	}

	ret = alloc_chrdev_region(&devt, MINOR_BASE, MINOR_COUNT,
				  ETHOSU_DRIVER_NAME);
	if (ret) {
		pr_err("Failed to allocate chrdev region.\n");
		goto destroy_class;
	}

	ret = register_rpmsg_driver(&ethosu_rpmsg_driver);
	if (ret) {
		pr_err("Failed to register Arm Ethos-U rpmsg driver.\n");
		goto region_unregister;
	}

	ret = register_direct_platform_drivers();
	if (ret) {
		pr_err("Failed to register Arm Ethos-U direct driver.\n");
		goto rpmsg_unregister;
	}

	return 0;

rpmsg_unregister:
	unregister_rpmsg_driver(&ethosu_rpmsg_driver);

region_unregister:
	unregister_chrdev_region(devt, MINOR_COUNT);

destroy_class:
	class_destroy(ethosu_class);

	return ret;
}

module_init(ethosu_init)
module_exit(ethosu_exit)

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Arm Ltd");
MODULE_DESCRIPTION("Arm Ethos-U NPU Driver");
MODULE_VERSION(ETHOSU_DRIVER_VERSION);
