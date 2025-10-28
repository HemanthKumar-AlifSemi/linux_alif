/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#include <direct/ethosu_direct_inference.h>
#include <direct/ethosu_direct_device.h>

#include <common/ethosu_device.h>
#include <common/ethosu_buffer.h>

/****************************************************************************
 * Functions
 ****************************************************************************/

static struct ethosu_direct_inference *__must_check to_direct_inference(
	struct ethosu_inference *inf)
{
	if (inf && inf->edev->type == ETHOSU_DEVICE_TYPE_DIRECT)
		return container_of(inf, struct ethosu_direct_inference, inf);

	return NULL;
}

static void ethosu_direct_inference_complete_locked(
	struct ethosu_direct_inference *d_inf,
	enum ethosu_uapi_status status)
{
	d_inf->inf.status = status;
	d_inf->inf.done = true;
	wake_up_interruptible(&d_inf->inf.waitq);
	ethosu_inference_put(&d_inf->inf);
}

static int ethosu_direct_inference_schedule_next(
	struct ethosu_direct_device *edirect_dev)
{
	struct device *dev = &edirect_dev->edev.dev;
	struct ethosu_direct_inference_queue *inf_queue =
		&edirect_dev->inf_queue;
	struct ethosu_direct_inference *d_inf;
	int ret;

	ret = mutex_lock_interruptible(&inf_queue->lock);
	if (ret)
		return ret;

	if (inf_queue->running_inference || list_empty(&inf_queue->list) ||
	    !atomic_read(&edirect_dev->running)) {
		mutex_unlock(&edirect_dev->inf_queue.lock);

		return 0;
	}

	d_inf = list_first_entry(&inf_queue->list, typeof(*d_inf), list_entry);
	list_del_init(&d_inf->list_entry);

	d_inf->inf.status = ETHOSU_UAPI_STATUS_RUNNING;
	inf_queue->running_inference = d_inf;

	ethosu_direct_device_pmu_setup(edirect_dev,
				       d_inf->inf.pmu_cycle_counter_enable,
				       d_inf->inf.pmu_event_config,
				       d_inf->inf.pmu_event_config_num);

	ret = ethosu_direct_device_run_inference(edirect_dev, d_inf);

	mutex_unlock(&edirect_dev->inf_queue.lock);

	if (ret) {
		dev_err(dev, "Failed to run inference %d\n", ret);

		return ethosu_direct_inference_done(edirect_dev, false);
	}

	return ret;
}

int ethosu_direct_inference_done(struct ethosu_direct_device *edirect_dev,
				 bool success)
{
	struct ethosu_direct_inference_queue *inf_queue =
		&edirect_dev->inf_queue;
	struct ethosu_direct_inference *d_inf;
	int ret;

	ret = mutex_lock_interruptible(&inf_queue->lock);
	if (ret)
		return ret;

	if (!inf_queue->running_inference)
		goto done;

	d_inf = inf_queue->running_inference;

	if (success) {
		ethosu_direct_device_pmu_get_values(edirect_dev,
						    d_inf->inf.pmu_cycle_counter_enable,
						    &d_inf->inf.pmu_cycle_counter_count,
						    d_inf->inf.pmu_event_count,
						    d_inf->inf.pmu_event_config_num);

		ethosu_direct_device_pmu_disable(edirect_dev);
	}

	ethosu_direct_inference_complete_locked(d_inf,
						success ? ETHOSU_UAPI_STATUS_OK : ETHOSU_UAPI_STATUS_ERROR);
	inf_queue->running_inference = NULL;

	/* Only need to clear SRAM on success because a failed inference will
	 * cause a NPU reset which will clear the SRAM */
	if (success)
		ethosu_direct_device_clear_sram(edirect_dev);

done:
	mutex_unlock(&inf_queue->lock);

	return ethosu_direct_inference_schedule_next(edirect_dev);
}

void ethosu_direct_inference_queue_init(
	struct ethosu_direct_inference_queue *queue)
{
	INIT_LIST_HEAD(&queue->list);
	mutex_init(&queue->lock);
	queue->running_inference = NULL;
}

int ethosu_direct_inference_create(struct ethosu_device *edev,
				   struct ethosu_network *net,
				   struct ethosu_inference **inf)
{
	struct device *dev = &edev->dev;
	struct ethosu_direct_inference *d_inf;

	d_inf = devm_kzalloc(dev, sizeof(*d_inf), GFP_KERNEL);
	if (!d_inf) {
		dev_err(dev,
			"Inference create. Failed to allocate struct");

		return -ENOMEM;
	}

	*inf = &d_inf->inf;

	return 0;
}

int ethosu_direct_inference_destroy(struct ethosu_inference *inf)
{
	struct ethosu_direct_inference *d_inf = to_direct_inference(inf);
	struct device *dev;

	if (!d_inf)
		return 0;

	dev = &d_inf->inf.edev->dev;

	if (d_inf->constant)
		ethosu_buffer_put(d_inf->constant);

	if (d_inf->intermediate)
		ethosu_buffer_put(d_inf->intermediate);

	memset(d_inf, 0, sizeof(*d_inf));
	devm_kfree(dev, d_inf);

	return 0;
}

int ethosu_direct_inference_release(struct ethosu_inference *inf)
{
	struct ethosu_direct_inference *d_inf = to_direct_inference(inf);
	struct ethosu_direct_inference_queue *inf_queue;
	struct ethosu_direct_device *edirect_dev;
	int ret;

	if (!d_inf)
		return -EFAULT;

	edirect_dev = to_direct_device(d_inf->inf.edev);

	if (!edirect_dev)
		return -EFAULT;

	inf_queue = &edirect_dev->inf_queue;

	ret = mutex_lock_interruptible(&inf_queue->lock);
	if (ret)
		return ret;

	if (inf_queue->running_inference == d_inf) {
		ethosu_direct_device_reset(edirect_dev);
		inf_queue->running_inference = NULL;
	} else {
		list_del_init(&d_inf->list_entry);
	}

	if (!d_inf->inf.done)
		ethosu_direct_inference_complete_locked(d_inf,
							ETHOSU_UAPI_STATUS_ABORTED);

	mutex_unlock(&inf_queue->lock);

	return ethosu_direct_inference_schedule_next(edirect_dev);
}

int ethosu_direct_inference_setup(struct ethosu_inference *inf,
				  struct ethosu_uapi_inference_create *uapi)
{
	struct ethosu_direct_inference *d_inf = to_direct_inference(inf);
	struct device *dev;
	struct ethosu_buffer *buf;
	int ret;

	if (!d_inf)
		return -EFAULT;

	dev = &d_inf->inf.edev->dev;

	buf = uapi->intermediate_fd > 0 ? ethosu_buffer_get_from_fd(
		uapi->intermediate_fd) : NULL;
	if (IS_ERR(buf)) {
		ret = PTR_ERR(buf);
		dev_err(dev,
			"Inference setup. Failed to get intermediate buffer ret=%d",
			ret);
		goto error;
	}

	d_inf->intermediate = buf;

	buf = uapi->constant_fd > 0 ? ethosu_buffer_get_from_fd(
		uapi->constant_fd) : NULL;
	if (IS_ERR(buf)) {
		ret = PTR_ERR(buf);
		dev_err(dev,
			"Inference setup. Failed to get constant buffer ret=%d",
			ret);
		goto put_intermediate;
	}

	d_inf->constant = buf;

	return 0;

put_intermediate:
	if (d_inf->intermediate) {
		ethosu_buffer_put(d_inf->intermediate);
		d_inf->intermediate = NULL;
	}

error:

	return ret;
}

int ethosu_direct_inference_send(struct ethosu_inference *inf)
{
	struct ethosu_direct_inference *d_inf = to_direct_inference(inf);
	struct ethosu_direct_device *edirect_dev;
	int ret;

	if (!d_inf)
		return -EFAULT;

	edirect_dev = to_direct_device(d_inf->inf.edev);
	if (!edirect_dev)
		return -EFAULT;

	ret = mutex_lock_interruptible(&edirect_dev->inf_queue.lock);
	if (ret)
		return ret;

	ethosu_inference_get(inf);

	list_add_tail(&d_inf->list_entry, &edirect_dev->inf_queue.list);
	d_inf->inf.status = ETHOSU_UAPI_STATUS_PENDING;

	mutex_unlock(&edirect_dev->inf_queue.lock);

	return ethosu_direct_inference_schedule_next(edirect_dev);
}

int ethosu_direct_inference_cancel(struct ethosu_inference *inf,
				   struct ethosu_uapi_cancel_inference_status *uapi)
{
	return 0;
}
