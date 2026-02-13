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

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <common/ethosu_inference.h>

#include <common/ethosu_buffer.h>
#include <common/ethosu_device.h>
#include <common/ethosu_network.h>

#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/poll.h>

/****************************************************************************
 * Variables
 ****************************************************************************/

static int ethosu_inference_release(struct inode *inode,
				    struct file *file);

static __poll_t ethosu_inference_poll(struct file *file,
				      poll_table *wait);

static long ethosu_inference_ioctl(struct file *file,
				   unsigned int cmd,
				   unsigned long arg);

static const struct file_operations ethosu_inference_fops = {
	.release        = &ethosu_inference_release,
	.poll           = &ethosu_inference_poll,
	.unlocked_ioctl = &ethosu_inference_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = &ethosu_inference_ioctl,
#endif
};

/****************************************************************************
 * Functions
 ****************************************************************************/

static uint32_t num_pmu_events_set(uint32_t *events,
				   uint32_t size)
{
	uint32_t num_events = size;
	ssize_t i;

	/* Use the last set event as the number of used events */
	for (i = size - 1; i > 0; --i) {
		if (events[i])
			break;

		num_events--;
	}

	return num_events;
}

static const char *status_to_string(const enum ethosu_uapi_status status)
{
	switch (status) {
	case ETHOSU_UAPI_STATUS_OK: {
		return "Ok";
	}
	case ETHOSU_UAPI_STATUS_ERROR: {
		return "Error";
	}
	case ETHOSU_UAPI_STATUS_RUNNING: {
		return "Running";
	}
	case ETHOSU_UAPI_STATUS_REJECTED: {
		return "Rejected";
	}
	case ETHOSU_UAPI_STATUS_ABORTED: {
		return "Aborted";
	}
	case ETHOSU_UAPI_STATUS_ABORTING: {
		return "Aborting";
	}
	default: {
		return "Unknown";
	}
	}
}

static void ethosu_inference_kref_destroy(struct kref *kref)
{
	struct ethosu_inference *inf =
		container_of(kref, struct ethosu_inference, kref);
	struct device *dev = &inf->edev->dev;

	dev_dbg(dev,
		"Inference destroy. inf=0x%pK, status=%d, ifm_count=%u, ofm_count=%u",
		inf, inf->status, inf->ifm_count, inf->ofm_count);

	while (inf->ifm_count-- > 0)
		ethosu_buffer_put(inf->ifm[inf->ifm_count]);

	while (inf->ofm_count-- > 0)
		ethosu_buffer_put(inf->ofm[inf->ofm_count]);

	ethosu_network_put(inf->net);
	ethosu_device_call_op(inf->edev, inference_destroy, inf);
}

static int ethosu_inference_release(struct inode *inode,
				    struct file *file)
{
	struct ethosu_inference *inf = file->private_data;
	struct device *dev = &inf->edev->dev;

	dev_dbg(dev,
		"Inference release. file=0x%pK, inf=0x%pK",
		file, inf);

	device_lock(dev);
	ethosu_device_call_op(inf->edev, inference_release, inf);
	ethosu_inference_put(inf);
	device_unlock(dev);

	return 0;
}

static __poll_t ethosu_inference_poll(struct file *file,
				      poll_table *wait)
{
	struct ethosu_inference *inf = file->private_data;
	__poll_t ret = 0;

	poll_wait(file, &inf->waitq, wait);

	if (inf->done)
		ret |= EPOLLIN;

	return ret;
}

static long ethosu_inference_ioctl(struct file *file,
				   unsigned int cmd,
				   unsigned long arg)
{
	struct ethosu_inference *inf = file->private_data;
	struct device *dev = &inf->edev->dev;
	void __user *udata = (void __user *)arg;
	int ret;

	ret = device_lock_interruptible(dev);
	if (ret)
		return ret;

	switch (cmd) {
	case ETHOSU_IOCTL_INFERENCE_STATUS: {
		struct ethosu_uapi_result_status uapi = { 0 };
		int i;

		uapi.status = inf->status;

		for (i = 0; i < inf->pmu_event_config_num; i++) {
			uapi.pmu_config.events[i] =
				inf->pmu_event_config[i];
			uapi.pmu_count.events[i] =
				inf->pmu_event_count[i];
		}

		uapi.pmu_config.cycle_count = inf->pmu_cycle_counter_enable;
		uapi.pmu_count.cycle_count = inf->pmu_cycle_counter_count;

		dev_dbg(dev,
			"Inference ioctl: Inference status. status=%s (%d)\n",
			status_to_string(uapi.status), uapi.status);

		ret = copy_to_user(udata, &uapi, sizeof(uapi)) ? -EFAULT : 0;

		break;
	}
	case ETHOSU_IOCTL_INFERENCE_CANCEL: {
		struct ethosu_uapi_cancel_inference_status uapi = { 0 };

		dev_dbg(dev,
			"Inference ioctl: Cancel Inference. Handle=%p\n",
			inf);

		ret = ethosu_device_call_op(inf->edev, inference_cancel, inf,
					    &uapi);
		if (ret)
			break;

		ret = copy_to_user(udata, &uapi, sizeof(uapi)) ? -EFAULT : 0;

		break;
	}
	default: {
		dev_err(dev, "Invalid ioctl. cmd=%u, arg=%lu\n",
			cmd, arg);
		ret = -ENOIOCTLCMD;
		break;
	}
	}

	device_unlock(dev);

	return ret;
}

int ethosu_inference_create(struct ethosu_device *edev,
			    struct ethosu_network *net,
			    struct ethosu_uapi_inference_create *uapi)
{
	struct device *dev = &edev->dev;
	struct ethosu_inference *inf;
	uint32_t i;
	int fd;
	int ret = -ENOMEM;
	size_t offset;

	if (uapi->ifm_count > ETHOSU_FD_MAX ||
	    uapi->ofm_count > ETHOSU_FD_MAX) {
		dev_err(dev,
			"Inference create. Too many IFM and/or OFM buffers for inference. ifm_count=%u, ofm_count=%u",
			uapi->ifm_count, uapi->ofm_count);

		return -EFAULT;
	} else if (uapi->ofm_count == 0) {
		dev_err(dev, "Inference create. No OFM buffers given");

		return -EFAULT;
	}

	ret = ethosu_device_call_op(edev, inference_create, edev, net, &inf);
	if (ret) {
		dev_err(dev,
			"Inference create. Failed to allocate struct");

		return ret;
	}

	inf->edev = edev;
	inf->net = net;
	inf->done = false;
	inf->status = ETHOSU_UAPI_STATUS_ERROR;
	kref_init(&inf->kref);
	init_waitqueue_head(&inf->waitq);

	/* Get pointer to IFM buffers */
	offset = 0U;
	for (i = 0; i < uapi->ifm_count; i++) {
		inf->ifm[i] = ethosu_buffer_get_from_fd(uapi->ifm_fd[i]);
		if (IS_ERR(inf->ifm[i])) {
			ret = PTR_ERR(inf->ifm[i]);
			dev_err(dev,
				"Inference create. Failed to get IFM buffer%u ret=%d",
				i, ret);
			goto put_ifm;
		}

		inf->ifm_count++;

		if (inf->ifm[i]->offset < offset) {
			ret = -EINVAL;
			dev_err(dev, "Inference create. Invalid IFM order\n");
			goto put_ifm;
		}

		offset = inf->ifm[i]->offset;
	}

	if (!ethosu_buffer_from_same_batch(inf->ifm, inf->ifm_count)) {
		ret = -EINVAL;
		dev_err(dev,
			"Inference create. Failed the IFMs are not from the same buffer batch");
		goto put_ifm;
	}

	dev_warn(dev, "Inference create. No IFMs given. Continuing anyway");

	offset = 0U;
	/* Get pointer to OFM buffer */
	for (i = 0; i < uapi->ofm_count; i++) {
		inf->ofm[i] = ethosu_buffer_get_from_fd(uapi->ofm_fd[i]);
		if (IS_ERR(inf->ofm[i])) {
			ret = PTR_ERR(inf->ofm[i]);
			dev_err(dev,
				"Inference create. Failed to get OFM buffer%u ret=%d",
				i, ret);
			goto put_ofm;
		}

		inf->ofm_count++;

		if (inf->ofm[i]->offset < offset) {
			ret = -EINVAL;
			dev_err(dev, "Inference create. Invalid OFM order\n");
			goto put_ofm;
		}

		offset = inf->ofm[i]->offset;
	}

	if (!ethosu_buffer_from_same_batch(inf->ofm, inf->ofm_count)) {
		ret = -EINVAL;
		dev_err(dev,
			"Inference create. Failed the OFMs are not from the same buffer batch");
		goto put_ofm;
	}

	/* Configure PMU and cycle counter */
	dev_dbg(dev,
		"Configuring events for PMU. events=[%u, %u, %u, %u]\n",
		uapi->pmu_config.events[0], uapi->pmu_config.events[1],
		uapi->pmu_config.events[2], uapi->pmu_config.events[3]);

	inf->pmu_event_config_num = num_pmu_events_set(uapi->pmu_config.events,
						       ETHOSU_PMU_EVENT_MAX);
	/* Configure events and reset count for all events */
	for (i = 0; i < inf->pmu_event_config_num; i++) {
		inf->pmu_event_config[i] = uapi->pmu_config.events[i];
		inf->pmu_event_count[i] = 0;
	}

	/* Configure cycle counter and reset any previous count */
	inf->pmu_cycle_counter_enable = !!uapi->pmu_config.cycle_count;
	inf->pmu_cycle_counter_count = 0;

	ret = ethosu_device_call_op(edev, inference_setup, inf, uapi);
	if (ret)
		goto put_ofm;

	/* Increment network reference count */
	ethosu_network_get(net);

	/* Send inference request to Arm Ethos-U subsystem */
	ret = ethosu_device_call_op(edev, inference_send, inf);
	if (ret)
		goto put_net;

	/* Create file descriptor */
	ret = fd = anon_inode_getfd("ethosu-inference",
				    &ethosu_inference_fops,
				    inf, O_RDWR | O_CLOEXEC);
	if (ret < 0) {
		dev_err(dev,
			"Inference create. Failed to get file descriptor. ret=%d",
			ret);

		goto put_net;
	}

	/* Store pointer to file structure */
	inf->file = fget(ret);
	fput(inf->file);

	dev_dbg(dev,
		"Inference create. file=0x%pK, fd=%d, inf=0x%p, net=0x%pK",
		inf->file, fd, inf, inf->net);

	return fd;

put_net:
	ethosu_network_put(net);

put_ofm:
	if (inf->ofm_count)
		while (inf->ofm_count-- > 0)
			ethosu_buffer_put(inf->ofm[inf->ofm_count]);

put_ifm:
	if (inf->ifm_count)
		while (inf->ifm_count-- > 0)
			ethosu_buffer_put(inf->ifm[inf->ifm_count]);

	ethosu_device_call_op(edev, inference_destroy, inf);

	return ret;
}

void ethosu_inference_get(struct ethosu_inference *inf)
{
	kref_get(&inf->kref);
}

int ethosu_inference_put(struct ethosu_inference *inf)
{
	return kref_put(&inf->kref, &ethosu_inference_kref_destroy);
}
