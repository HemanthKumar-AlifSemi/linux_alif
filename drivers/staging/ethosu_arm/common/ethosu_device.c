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

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <common/ethosu_device.h>

#include <common/ethosu_buffer.h>
#include <common/ethosu_network.h>
#include <uapi/ethosu.h>

#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/remoteproc.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

#define MINOR_BASE      0 /* Minor version starts at 0 */
#define MINOR_COUNT    64 /* Allocate minor versions */

/****************************************************************************
 * Variables
 ****************************************************************************/

static DECLARE_BITMAP(minors, MINOR_COUNT);

/****************************************************************************
 * Functions
 ****************************************************************************/

static int ethosu_open(struct inode *inode,
		       struct file *file)
{
	struct cdev *cdev = inode->i_cdev;
	struct ethosu_device *edev = container_of(cdev, struct ethosu_device,
						  cdev);
	struct device *dev = &edev->dev;

	dev_dbg(dev, "Device open. file=0x%pK", file);

	file->private_data = edev;

	return nonseekable_open(inode, file);
}

static long ethosu_ioctl(struct file *file,
			 unsigned int cmd,
			 unsigned long arg)
{
	struct ethosu_device *edev = file->private_data;
	struct device *dev = &edev->dev;
	void __user *udata = (void __user *)arg;
	int ret;

	switch (cmd) {
	case ETHOSU_IOCTL_DRIVER_VERSION_GET: {
		const struct ethosu_uapi_kernel_driver_version version = {
			.major = ETHOSU_KERNEL_DRIVER_VERSION_MAJOR,
			.minor = ETHOSU_KERNEL_DRIVER_VERSION_MINOR,
			.patch = ETHOSU_KERNEL_DRIVER_VERSION_PATCH,
		};

		ret = copy_to_user(udata, &version,
				   sizeof(version)) ? -EFAULT : 0;
		break;
	}
	case ETHOSU_IOCTL_CAPABILITIES_REQ: {
		dev_dbg(dev, "Device ioctl: Capabilities request");

		ret = copy_to_user(udata, &edev->capabilities,
				   sizeof(edev->capabilities)) ? -EFAULT : 0;
		break;
	}
	case ETHOSU_IOCTL_PING: {
		ret = device_lock_interruptible(dev);
		if (ret)
			return ret;

		dev_dbg(dev, "Device ioctl: Send ping");

		ret = ethosu_device_call_op(edev, ping, edev);

		device_unlock(dev);

		break;
	}
	case ETHOSU_IOCTL_BUFFERS_CREATE: {
		struct ethosu_uapi_buffers_create uapi = { 0 };

		if (copy_from_user(&uapi, udata, sizeof(uapi))) {
			ret = -EFAULT;
			break;
		}

		if (uapi.num > ETHOSU_FD_MAX) {
			ret = -EINVAL;
			break;
		}

		ret = device_lock_interruptible(dev);
		if (ret)
			break;

		dev_dbg(dev, "Device ioctl: Buffers create. num=%u", uapi.num);

		ret = ethosu_buffer_batch_create(edev, uapi.sizes, uapi.fds,
						 uapi.num);
		if (ret) {
			device_unlock(dev);
			break;
		}

		ret = copy_to_user(udata, &uapi, sizeof(uapi)) ? -EFAULT : 0;
		if (ret)
			ethosu_buffer_free_fds(dev, uapi.fds, uapi.num);

		device_unlock(dev);

		break;
	}
	case ETHOSU_IOCTL_NETWORK_CREATE: {
		struct ethosu_uapi_network_create uapi;

		if (copy_from_user(&uapi, udata, sizeof(uapi))) {
			ret = -EFAULT;
			break;
		}

		ret = device_lock_interruptible(dev);
		if (ret)
			return ret;

		dev_dbg(dev,
			"Device ioctl: Network create. type=%u\n", uapi.type);

		ret = ethosu_network_create(edev, &uapi);

		device_unlock(dev);

		break;
	}
	default: {
		dev_err(dev, "Invalid ioctl. cmd=%u, arg=%lu",
			cmd, arg);
		ret = -ENOIOCTLCMD;
		break;
	}
	}

	return ret;
}

static const struct file_operations fops = {
	.owner          = THIS_MODULE,
	.open           = &ethosu_open,
	.unlocked_ioctl = &ethosu_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = &ethosu_ioctl,
#endif
};

static void ethosu_device_release(struct device *dev)
{
	struct ethosu_device *edev = dev_get_drvdata(dev);

	clear_bit(MINOR(edev->cdev.dev), minors);
	device_destroy(edev->class, edev->cdev.dev);
}

static int ethosu_device_setup_memory(struct device *dev,
				      struct device *parent,
				      struct device *mem_dev)
{
	int ret;

	/* Inherit DMA mask from memory device */
	ret = dma_coerce_mask_and_coherent(dev, dma_get_mask(mem_dev));
	if (ret) {
		dev_err(parent, "Failed to set DMA mask. ret=%d", ret);

		return ret;
	}

	/* Inherit DMA configuration from memory device */
	ret = of_dma_configure(dev, mem_dev->of_node, false);
	if (ret) {
		dev_err(parent, "Failed to configure DMA. ret=%d", ret);

		return ret;
	}

	/* Inherit reserved memory from memory device */
	ret = of_reserved_mem_device_init_by_idx(dev, mem_dev->of_node, 0);
	if (ret) {
		dev_err(parent, "Failed to initialize reserved memory. ret=%d",
			ret);

		return ret;
	}

	return 0;
}

int ethosu_device_init(struct ethosu_device *edev,
		       enum ethosu_device_type type,
		       const struct ethosu_device_ops *ops,
		       const struct ethosu_device_buffer_config *buf_config,
		       struct device *parent,
		       struct device *memory_dev,
		       struct class *class,
		       dev_t *devt)
{
	struct device *dev = &edev->dev;
	int minor;
	int ret;

	/* Reserve minor number for device node */
	minor = find_first_zero_bit(minors, MINOR_COUNT);
	if (minor >= MINOR_COUNT) {
		dev_err(parent, "No more minor numbers.");

		return -ENOMEM;
	}

	edev->class = class;
	edev->type = type;
	edev->ops = ops;
	edev->buffer_config.alignment = buf_config->alignment;
	edev->buffer_config.max_size = buf_config->max_size;

	device_initialize(dev);
	dev->parent = parent;
	dev->release = ethosu_device_release;
	dev_set_drvdata(dev, edev);

	*devt = MKDEV(MAJOR(*devt), minor);
	ret = dev_set_name(dev, "ethosu%u", MINOR(*devt));
	if (ret) {
		dev_err(parent, "Failed to set device name. ret=%d", ret);

		goto free_device;
	}

	ret = ethosu_device_setup_memory(dev, parent, memory_dev);
	if (ret) {
		dev_err(parent, "Failed to set up device memory. ret=%d", ret);

		goto free_device;
	}

	ret = device_add(dev);
	if (ret) {
		dev_err(parent, "Failed to add device. ret=%d", ret);

		goto free_device;
	}

	set_bit(minor, minors);

	return 0;

free_device:
	put_device(dev);

	return ret;
}

int ethosu_device_finalize(struct ethosu_device *edev,
			   dev_t devt)
{
	struct device *dev = &edev->dev;
	struct device *sysdev;
	int ret;

	ret = ethosu_device_call_op(edev, capabilities_get, edev,
				    &edev->capabilities);
	if (ret) {
		dev_err(dev, "Failed to get device capabilities: %d", ret);

		return ret;
	}

	/* Create device node */
	cdev_init(&edev->cdev, &fops);
	edev->cdev.owner = THIS_MODULE;

	cdev_set_parent(&edev->cdev, &dev->kobj);

	ret = cdev_add(&edev->cdev, devt, 1);
	if (ret) {
		dev_err(dev, "Failed to add character device.");

		return ret;
	}

	sysdev = device_create(edev->class, NULL, devt, edev,
			       "ethosu%u", MINOR(devt));
	if (IS_ERR(sysdev)) {
		cdev_del(&edev->cdev);
		dev_err(dev, "Failed to create device.");
		ret = PTR_ERR(sysdev);

		return ret;
	}

	edev->cdev_setup = true;

	return 0;
}

void ethosu_device_deinit(struct ethosu_device *edev)
{
	if (!edev)
		return;

	if (edev->cdev_setup)
		cdev_del(&edev->cdev);

	device_unregister(&edev->dev);
}
