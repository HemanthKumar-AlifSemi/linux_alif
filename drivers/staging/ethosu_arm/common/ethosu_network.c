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

#include <common/ethosu_network.h>

#include <common/ethosu_device.h>
#include <common/ethosu_dma_mem.h>
#include <common/ethosu_inference.h>
#include <uapi/ethosu.h>

#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

/****************************************************************************
 * Variables
 ****************************************************************************/

static int ethosu_network_release(struct inode *inode,
				  struct file *file);

static long ethosu_network_ioctl(struct file *file,
				 unsigned int cmd,
				 unsigned long arg);

static const struct file_operations ethosu_network_fops = {
	.release        = &ethosu_network_release,
	.unlocked_ioctl = &ethosu_network_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = &ethosu_network_ioctl,
#endif
};

/****************************************************************************
 * Functions
 ****************************************************************************/

static void ethosu_network_destroy(struct kref *kref)
{
	struct ethosu_network *net =
		container_of(kref, struct ethosu_network, kref);
	struct device *dev = &net->edev->dev;

	dev_dbg(dev, "Network destroy. net=0x%pK\n", net);

	if (net->dma_mem != NULL)
		ethosu_dma_mem_put(net->dma_mem);

	ethosu_device_call_op(net->edev, network_destroy, net);
}

static int ethosu_network_release(struct inode *inode,
				  struct file *file)
{
	struct ethosu_network *net = file->private_data;
	struct device *dev = &net->edev->dev;

	dev_dbg(dev, "Network release. file=0x%pK, net=0x%pK\n",
		file, net);

	ethosu_network_put(net);

	return 0;
}

static long ethosu_network_ioctl(struct file *file,
				 unsigned int cmd,
				 unsigned long arg)
{
	struct ethosu_network *net = file->private_data;
	struct ethosu_device *edev = net->edev;
	struct device *dev = &edev->dev;
	void __user *udata = (void __user *)arg;
	int ret;

	ret = device_lock_interruptible(dev);
	if (ret)
		return ret;

	switch (cmd) {
	case ETHOSU_IOCTL_NETWORK_INFO: {
		struct ethosu_uapi_network_info uapi = { 0 };

		dev_dbg(dev, "Network ioctl: Network info. net=0x%pK", net);

		ret = ethosu_device_call_op(edev, network_info, net, &uapi);
		if (ret)
			break;

		ret = copy_to_user(udata, &uapi, sizeof(uapi)) ? -EFAULT : 0;
		break;
	}
	case ETHOSU_IOCTL_INFERENCE_CREATE: {
		struct ethosu_uapi_inference_create uapi = { 0 };

		if (copy_from_user(&uapi, udata, sizeof(uapi))) {
			dev_err(dev,
				"Network ioctl: Failed to copy inference request");
			ret = -EFAULT;
			break;
		}

		ret = ethosu_inference_create(edev, net, &uapi);
		break;
	}
	default: {
		dev_err(dev, "Invalid ioctl. cmd=%u, arg=%lu", cmd, arg);
		ret = -ENOIOCTLCMD;
		break;
	}
	}

	device_unlock(dev);

	return ret;
}

int ethosu_network_create(struct ethosu_device *edev,
			  struct ethosu_uapi_network_create *uapi)
{
	struct device *dev = &edev->dev;
	struct ethosu_network *net;
	const void __user *data;
	int ret;

	ret = ethosu_device_call_op(edev, network_create, edev, &net);
	if (ret)
		return ret;

	net->edev = edev;
	kref_init(&net->kref);

	switch (uapi->type) {
	case ETHOSU_UAPI_NETWORK_USER_BUFFER:
		if (!uapi->network.data_ptr) {
			dev_err(dev,
				"Network create. Invalid network data ptr");
			ret = -EINVAL;
			goto free_net;
		}

		if (!uapi->network.size) {
			dev_err(dev,
				"Network create. Invalid network data size");
			ret = -EINVAL;
			goto free_net;
		}

		net->dma_mem = ethosu_dma_mem_alloc(dev, uapi->network.size);
		if (IS_ERR(net->dma_mem)) {
			ret = PTR_ERR(net->dma_mem);
			dev_err(dev,
				"Network create. Failed to allocate DMA memory. ret=%d",
				ret);
			goto free_net;
		}

		data = u64_to_user_ptr(uapi->network.data_ptr);
		ret = copy_from_user(net->dma_mem->cpu_addr, data,
				     uapi->network.size);
		if (ret) {
			dev_err(dev,
				"Network create. Failed to copy network data from user buffer. ret=%d",
				ret);
			goto free_dma_mem;
		}

		ret = ethosu_device_call_op(edev, network_setup, net);
		if (ret)
			goto free_dma_mem;

		break;
	case ETHOSU_UAPI_NETWORK_INDEX:
		ret = ethosu_device_call_op(edev, network_check_index, edev,
					    uapi->index);
		if (ret)
			goto free_net;

		net->index = uapi->index;
		break;
	default:
		dev_err(dev, "Network create. Invalid buffer type. type=%u",
			uapi->type);
		ret = -EINVAL;
		goto free_net;
	}

	ret = anon_inode_getfd("ethosu-network", &ethosu_network_fops,
			       net,
			       O_RDWR | O_CLOEXEC);
	if (ret < 0) {
		dev_err(dev,
			"Network create. Failed to get file descriptor. ret=%d",
			ret);
		goto free_dma_mem;
	}

	net->file = fget(ret);
	fput(net->file);

	dev_dbg(dev,
		"Network create. file=0x%pK, fd=%d, net=0x%pK, buf=0x%pK, index=%u",
		net->file, ret, net, net->dma_mem, net->index);

	return ret;

free_dma_mem:
	if (net->dma_mem != NULL)
		ethosu_dma_mem_put(net->dma_mem);

free_net:
	ethosu_device_call_op(edev, network_destroy, net);

	return ret;
}

void ethosu_network_get(struct ethosu_network *net)
{
	kref_get(&net->kref);
}

int ethosu_network_put(struct ethosu_network *net)
{
	return kref_put(&net->kref, ethosu_network_destroy);
}
