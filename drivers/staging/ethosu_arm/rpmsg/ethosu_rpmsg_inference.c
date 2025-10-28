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

#include <rpmsg/ethosu_rpmsg_inference.h>

#include <rpmsg/ethosu_rpmsg.h>
#include <rpmsg/ethosu_rpmsg_device.h>
#include <rpmsg/ethosu_rpmsg_cancel_inference.h>
#include <rpmsg/ethosu_rpmsg_network.h>

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_rpmsg_inference *__must_check to_rpmsg_inference(
	struct ethosu_inference *inf)
{
	if (inf->edev->type == ETHOSU_DEVICE_TYPE_RPMSG)
		return container_of(inf, struct ethosu_rpmsg_inference, inf);

	return NULL;
}

int ethosu_rpmsg_inference_setup(struct ethosu_inference *inf,
				 struct ethosu_uapi_inference_create *uapi)
{
	/* No additional setup required */
	return 0;
}

int ethosu_rpmsg_inference_send(struct ethosu_inference *inf)
{
	struct ethosu_rpmsg_inference *rp_inf = to_rpmsg_inference(inf);
	struct ethosu_rpmsg_network *rp_net = to_rpmsg_network(inf->net);
	struct device *dev = &inf->edev->dev;
	int ret;

	inf->status = ETHOSU_UAPI_STATUS_ERROR;

	ret = ethosu_rpmsg_mailbox_inference(rp_inf->mailbox, &rp_inf->msg,
					     inf->ifm_count, inf->ifm,
					     inf->ofm_count, inf->ofm,
					     rp_net, inf->pmu_event_config,
					     ETHOSU_PMU_EVENT_MAX,
					     inf->pmu_cycle_counter_enable);
	if (ret) {
		dev_warn(dev,
			 "Failed to send inference request. inf=0x%pK, ret=%d",
			 inf, ret);

		return ret;
	}

	inf->status = ETHOSU_UAPI_STATUS_RUNNING;

	ethosu_inference_get(inf);

	return 0;
}

static void ethosu_rpmsg_inference_fail(struct ethosu_rpmsg_mailbox_msg *msg)
{
	struct ethosu_rpmsg_inference *rp_inf =
		container_of(msg, typeof(*rp_inf), msg);
	struct ethosu_inference *inf = &rp_inf->inf;
	int ret;

	if (inf->done)
		return;

	/* Decrement reference count if inference was pending reponse */
	ret = ethosu_inference_put(inf);
	if (ret)
		return;

	/* Set status accordingly to the inference state */
	inf->status = inf->status == ETHOSU_UAPI_STATUS_ABORTING ?
		      ETHOSU_UAPI_STATUS_ABORTED :
		      ETHOSU_UAPI_STATUS_ERROR;
	/* Mark it done and wake up the waiting process */
	inf->done = true;
	wake_up_interruptible(&inf->waitq);
}

int ethosu_rpmsg_inference_create(struct ethosu_device *edev,
				  struct ethosu_network *net,
				  struct ethosu_inference **inf)
{
	struct ethosu_rpmsg_device *rp_dev = to_rpmsg_device(edev);
	struct ethosu_rpmsg_network *rp_net = to_rpmsg_network(net);
	struct device *dev = &edev->dev;
	struct ethosu_rpmsg_inference *rp_inf;
	int ret;

	if (!rp_dev)
		return -EINVAL;

	if (!rp_net)
		return -EINVAL;

	rp_inf = devm_kzalloc(dev, sizeof(*rp_inf), GFP_KERNEL);
	if (!rp_inf) {
		dev_err(dev,
			"Inference create. Failed to allocate struct");

		return -ENOMEM;
	}

	rp_inf->dev = dev;
	rp_inf->mailbox = &rp_dev->mailbox;
	rp_inf->net = rp_net;
	rp_inf->msg.fail = ethosu_rpmsg_inference_fail;

	/* Add inference to pending list */
	ret = ethosu_rpmsg_mailbox_register(&rp_dev->mailbox, &rp_inf->msg);
	if (ret < 0)
		goto kfree;

	*inf = &rp_inf->inf;

	return 0;

kfree:
	memset(rp_inf, 0, sizeof(*rp_inf));
	devm_kfree(dev, rp_inf);

	return ret;
}

int ethosu_rpmsg_inference_destroy(struct ethosu_inference *inf)
{
	struct ethosu_rpmsg_inference *rp_inf = to_rpmsg_inference(inf);
	struct ethosu_rpmsg_device *rp_dev = to_rpmsg_device(inf->edev);
	struct device *dev;

	if (!rp_inf)
		return 0;

	if (!rp_dev)
		return 0;

	dev = &rp_dev->edev.dev;

	ethosu_rpmsg_mailbox_deregister(&rp_dev->mailbox, &rp_inf->msg);
	memset(rp_inf, 0, sizeof(*rp_inf));
	devm_kfree(dev, rp_inf);

	return 0;
}

void ethosu_rpmsg_inference_rsp(struct ethosu_rpmsg_mailbox *mailbox,
				int msg_id,
				struct ethosu_rpmsg_inference_rsp *rsp)
{
	struct device *dev = mailbox->dev;
	struct ethosu_rpmsg_mailbox_msg *msg;
	struct ethosu_rpmsg_inference *rp_inf;
	struct ethosu_inference *inf;
	int i;

	msg = ethosu_rpmsg_mailbox_find(mailbox, msg_id,
					ETHOSU_RPMSG_INFERENCE_REQ);
	if (IS_ERR(msg)) {
		dev_warn(dev,
			 "Id for inference msg not found. Id=0x%x: %ld\n",
			 msg_id, PTR_ERR(msg));

		return;
	}

	rp_inf = container_of(msg, typeof(*rp_inf), msg);
	inf = &rp_inf->inf;

	/*
	 * Don't handle the response if the inference is aborted or
	 * in the process of being aborted
	 */
	if (inf->status == ETHOSU_UAPI_STATUS_ABORTED ||
	    inf->status == ETHOSU_UAPI_STATUS_ABORTING) {
		inf->status = ETHOSU_UAPI_STATUS_ABORTED;
		goto done;
	}

	if (rsp->status == ETHOSU_RPMSG_STATUS_OK &&
	    inf->ofm_count <= ETHOSU_RPMSG_BUFFER_MAX)
		inf->status = ETHOSU_UAPI_STATUS_OK;
	else if (rsp->status == ETHOSU_RPMSG_STATUS_REJECTED)
		inf->status = ETHOSU_UAPI_STATUS_REJECTED;
	else if (rsp->status == ETHOSU_RPMSG_STATUS_ABORTED)
		inf->status = ETHOSU_UAPI_STATUS_ABORTED;
	else
		inf->status = ETHOSU_UAPI_STATUS_ERROR;

	if (inf->status == ETHOSU_UAPI_STATUS_OK) {
		for (i = 0; i < ETHOSU_RPMSG_PMU_MAX; i++) {
			inf->pmu_event_config[i] = rsp->pmu_event_config[i];
			inf->pmu_event_count[i] = rsp->pmu_event_count[i];
		}

		inf->pmu_cycle_counter_enable = rsp->pmu_cycle_counter_enable;
		inf->pmu_cycle_counter_count = rsp->pmu_cycle_counter_count;

		dev_dbg(dev,
			"PMU events. config=[%u, %u, %u, %u], count=[%llu, %llu, %llu, %llu]\n",
			inf->pmu_event_config[0], inf->pmu_event_config[1],
			inf->pmu_event_config[2], inf->pmu_event_config[3],
			inf->pmu_event_count[0], inf->pmu_event_count[1],
			inf->pmu_event_count[2], inf->pmu_event_count[3]);

		if (inf->pmu_cycle_counter_enable)
			dev_dbg(dev,
				"PMU cycle counter: count=%llu\n",
				inf->pmu_cycle_counter_count);
	}

done:
	inf->done = true;
	wake_up_interruptible(&inf->waitq);
	ethosu_inference_put(inf);
}
