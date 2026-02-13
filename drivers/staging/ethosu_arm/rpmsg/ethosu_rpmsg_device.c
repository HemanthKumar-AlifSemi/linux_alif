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

#include <rpmsg/ethosu_rpmsg_device.h>

#include <rpmsg/ethosu_rpmsg_cancel_inference.h>
#include <rpmsg/ethosu_rpmsg_capabilities.h>
#include <rpmsg/ethosu_rpmsg_inference.h>
#include <rpmsg/ethosu_rpmsg_network.h>
#include <rpmsg/ethosu_rpmsg_network_info.h>
#include <rpmsg/ethosu_rpmsg_version.h>

#include <linux/dma-mapping.h>
#include <linux/remoteproc.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/types.h>

/****************************************************************************
 * Functions
 ****************************************************************************/

struct ethosu_rpmsg_device *to_rpmsg_device(struct ethosu_device *edev)
{
	if (edev->type == ETHOSU_DEVICE_TYPE_RPMSG)
		return container_of(edev, struct ethosu_rpmsg_device, edev);

	return NULL;
}

static int ethosu_rpmsg_device_ping(struct ethosu_device *edev)
{
	struct ethosu_rpmsg_device *erp_dev = to_rpmsg_device(edev);

	if (!erp_dev)
		return -EINVAL;

	return ethosu_rpmsg_mailbox_ping(&erp_dev->mailbox);
}

static int ethosu_rpmsg_device_capabilities_get(struct ethosu_device *edev,
						struct ethosu_uapi_device_capabilities *cap)
{
	struct ethosu_rpmsg_device *erp_dev = to_rpmsg_device(edev);
	struct device *dev = &edev->dev;
	int ret;

	device_lock(dev);
	ret = ethosu_rpmsg_capabilities_request(dev, &erp_dev->mailbox,
						&erp_dev->edev.capabilities);
	device_unlock(dev);

	return ret;
}

static const struct ethosu_device_ops rpmsg_ops = {
	.capabilities_get    = &ethosu_rpmsg_device_capabilities_get,
	.ping                = &ethosu_rpmsg_device_ping,
	.network_create      = &ethosu_rpmsg_network_create,
	.network_destroy     = &ethosu_rpmsg_network_destroy,
	.network_check_index = &ethosu_rpmsg_network_check_index,
	.network_setup       = &ethosu_rpmsg_network_setup,
	.network_info        = &ethosu_rpmsg_network_info_request,
	.inference_create    = &ethosu_rpmsg_inference_create,
	.inference_destroy   = &ethosu_rpmsg_inference_destroy,
	.inference_setup     = &ethosu_rpmsg_inference_setup,
	.inference_release   = NULL,
	.inference_send      = &ethosu_rpmsg_inference_send,
	.inference_cancel    = &ethosu_rpmsg_cancel_inference_request,
};

static const struct ethosu_device_buffer_config buffer_config = {
	.alignment = 0U,
	.max_size  = DMA_BIT_MASK(32U),
};

static int ethosu_rpmsg_device_handle_msg(struct rpmsg_device *rpdev,
					  void *data,
					  int len,
					  void *priv,
					  u32 src)
{
	struct ethosu_rpmsg_device *rp_edev = dev_get_drvdata(&rpdev->dev);
	struct ethosu_device *edev = &rp_edev->edev;
	struct device *dev = &edev->dev;
	struct ethosu_rpmsg_mailbox *mbox = &rp_edev->mailbox;
	struct ethosu_rpmsg *rpmsg = data;
	int length = len - sizeof(rpmsg->header);
	int ret = 0;

	if (unlikely(rpmsg->header.magic != ETHOSU_RPMSG_MAGIC)) {
		dev_warn(dev, "Msg: Error invalid message magic. magic=0x%08x",
			 rpmsg->header.magic);

		return -EBADMSG;
	}

	device_lock(dev);

	dev_dbg(dev,
		"Msg: magic=0x%08x, type=%u, msg_id=%llu",
		rpmsg->header.magic, rpmsg->header.type, rpmsg->header.msg_id);

	switch (rpmsg->header.type) {
	case ETHOSU_RPMSG_ERR:
		if (length != sizeof(rpmsg->error)) {
			dev_warn(dev,
				 "Msg: Error message of incorrect size. size=%u, expected=%zu", length,
				 sizeof(rpmsg->error));
			ret = -EBADMSG;
			break;
		}

		rpmsg->error.msg[sizeof(rpmsg->error.msg) - 1] = '\0';
		dev_warn(dev, "Msg: Error. type=%u, msg=\"%s\"",
			 rpmsg->error.type, rpmsg->error.msg);

		rproc_report_crash(rproc_get_by_child(dev), RPROC_FATAL_ERROR);
		break;
	case ETHOSU_RPMSG_PING:
		dev_dbg(dev, "Msg: Ping");
		ret = ethosu_rpmsg_mailbox_pong(mbox);
		break;
	case ETHOSU_RPMSG_PONG:
		dev_dbg(dev, "Msg: Pong");
		break;
	case ETHOSU_RPMSG_INFERENCE_RSP:
		if (length != sizeof(rpmsg->inf_rsp)) {
			dev_warn(dev,
				 "Msg: Inference response of incorrect size. size=%u, expected=%zu", length,
				 sizeof(rpmsg->inf_rsp));
			ret = -EBADMSG;
			break;
		}

		dev_dbg(dev,
			"Msg: Inference response. ofm_count=%u, status=%u",
			rpmsg->inf_rsp.ofm_count, rpmsg->inf_rsp.status);

		ethosu_rpmsg_inference_rsp(mbox, rpmsg->header.msg_id,
					   &rpmsg->inf_rsp);
		break;
	case ETHOSU_RPMSG_CANCEL_INFERENCE_RSP:
		if (length != sizeof(rpmsg->cancel_rsp)) {
			dev_warn(dev,
				 "Msg: Cancel Inference response of incorrect size. size=%u, expected=%zu", length,
				 sizeof(rpmsg->cancel_rsp));
			ret = -EBADMSG;
			break;
		}

		dev_dbg(dev,
			"Msg: Cancel Inference response. status=%u",
			rpmsg->cancel_rsp.status);
		ethosu_rpmsg_cancel_inference_rsp(mbox,
						  rpmsg->header.msg_id,
						  &rpmsg->cancel_rsp);
		break;
	case ETHOSU_RPMSG_VERSION_RSP:
		if (length != sizeof(rpmsg->version_rsp)) {
			dev_warn(dev,
				 "Msg: Protocol version response of incorrect size. size=%u, expected=%zu", length,
				 sizeof(rpmsg->version_rsp));
			ret = -EBADMSG;
			break;
		}

		dev_dbg(dev, "Msg: Protocol version response %u.%u.%u",
			rpmsg->version_rsp.major, rpmsg->version_rsp.minor,
			rpmsg->version_rsp.patch);

		ethosu_rpmsg_version_rsp(mbox, rpmsg->header.msg_id,
					 &rpmsg->version_rsp);
		break;
	case ETHOSU_RPMSG_CAPABILITIES_RSP:
		if (length != sizeof(rpmsg->cap_rsp)) {
			dev_warn(dev,
				 "Msg: Capabilities response of incorrect size. size=%u, expected=%zu", length,
				 sizeof(rpmsg->cap_rsp));
			ret = -EBADMSG;
			break;
		}

		dev_dbg(dev,
			"Msg: Capabilities response vs%hhu v%hhu.%hhu p%hhu av%hhu.%hhu.%hhu dv%hhu.%hhu.%hhu mcc%hhu csv%hhu cd%hhu",
			rpmsg->cap_rsp.version_status,
			rpmsg->cap_rsp.version_major,
			rpmsg->cap_rsp.version_minor,
			rpmsg->cap_rsp.product_major,
			rpmsg->cap_rsp.arch_major_rev,
			rpmsg->cap_rsp.arch_minor_rev,
			rpmsg->cap_rsp.arch_patch_rev,
			rpmsg->cap_rsp.driver_major_rev,
			rpmsg->cap_rsp.driver_minor_rev,
			rpmsg->cap_rsp.driver_patch_rev,
			rpmsg->cap_rsp.macs_per_cc,
			rpmsg->cap_rsp.cmd_stream_version,
			rpmsg->cap_rsp.custom_dma);

		ethosu_capability_rsp(mbox, rpmsg->header.msg_id,
				      &rpmsg->cap_rsp);
		break;
	case ETHOSU_RPMSG_NETWORK_INFO_RSP:
		if (length != sizeof(rpmsg->net_info_rsp)) {
			dev_warn(dev,
				 "Msg: Network info response of incorrect size. size=%u, expected=%zu", length,
				 sizeof(rpmsg->net_info_rsp));
			ret = -EBADMSG;
			break;
		}

		dev_dbg(dev,
			"Msg: Network info response. status=%u",
			rpmsg->net_info_rsp.status);

		ethosu_rpmsg_network_info_rsp(mbox,
					      rpmsg->header.msg_id,
					      &rpmsg->net_info_rsp);

		break;
	default:
		/* This should not happen due to version checks */
		dev_warn(dev, "Msg: Protocol error. type=%u",
			 rpmsg->header.type);
		ret = -EPROTO;
		break;
	}

	device_unlock(dev);

	wake_up(&mbox->send_queue);

	return ret;
}

static struct rpmsg_endpoint *ethosu_rpmsg_device_create_ept(
	struct rpmsg_device *rpdev)
{
	struct device *dev = &rpdev->dev;
	struct rpmsg_channel_info info = { 0 };
	struct rpmsg_endpoint *ept;

	/* Create rpmsg endpoint */
	strncpy(info.name, rpdev->id.name, sizeof(info.name) - 1);
	info.src = 0;
	info.dst = rpdev->dst;

	dev_dbg(dev, "Creating rpmsg endpoint. name=%s, src=%u, dst=%u",
		info.name, info.src, info.dst);

	ept = rpmsg_create_ept(rpdev, ethosu_rpmsg_device_handle_msg, NULL,
			       info);
	if (!ept) {
		dev_err(&rpdev->dev, "Failed to create endpoint");

		return ERR_PTR(-EINVAL);
	}

	return ept;
}

int ethosu_rpmsg_device_init(struct rpmsg_device *rpdev,
			     struct class *class,
			     dev_t devt)
{
	struct device *dev = &rpdev->dev;
	struct rproc *rproc = rproc_get_by_child(dev);
	/* The reserved memory is assigned to the remoteproc parent */
	struct device *mem_dev = rproc->dev.parent;
	struct ethosu_rpmsg_device *erp_dev;
	int ret;

	erp_dev = kzalloc(sizeof(*erp_dev), GFP_KERNEL);
	if (!erp_dev) {
		dev_err(dev, "Failed to allocate Arm Ethos-U RPMSG device");

		return -ENOMEM;
	}

	erp_dev->rpdev = rpdev;

	ret = ethosu_device_init(&erp_dev->edev, ETHOSU_DEVICE_TYPE_RPMSG,
				 &rpmsg_ops, &buffer_config, dev, mem_dev,
				 class, &devt);
	if (ret)
		goto free_erp_dev;

	dev = &erp_dev->edev.dev;
	dev_set_drvdata(&rpdev->dev, erp_dev);

	/* Create RPMsg endpoint */
	erp_dev->ept = ethosu_rpmsg_device_create_ept(rpdev);
	if (IS_ERR(erp_dev->ept)) {
		ret = PTR_ERR(erp_dev->ept);
		goto device_unregister;
	}

	ret = ethosu_rpmsg_mailbox_init(&erp_dev->mailbox, dev, erp_dev->ept);
	if (ret)
		goto free_rpmsg_ept;

	device_lock(dev);
	ret = ethosu_rpmsg_version_check_request(dev, &erp_dev->mailbox);
	device_unlock(dev);
	if (ret) {
		dev_err(dev, "Protocol version check failed: %d", ret);
		goto deinit_mailbox;
	}

	ret = ethosu_device_finalize(&erp_dev->edev, devt);
	if (ret)
		goto deinit_mailbox;

	dev_info(dev,
		 "Created Arm Ethos-U RPMSG device. name=%s, major=%d, minor=%d",
		 dev_name(dev), MAJOR(devt), MINOR(devt));

	return 0;

deinit_mailbox:
	ethosu_rpmsg_mailbox_deinit(&erp_dev->mailbox);

free_rpmsg_ept:
	rpmsg_destroy_ept(erp_dev->ept);

device_unregister:
	ethosu_device_deinit(&erp_dev->edev);

free_erp_dev:
	kfree(erp_dev);

	return ret;
}

void ethosu_rpmsg_device_deinit(struct rpmsg_device *rpdev)
{
	struct ethosu_rpmsg_device *erp_dev;

	if (!rpdev)
		return;

	erp_dev = dev_get_drvdata(&rpdev->dev);
	if (!erp_dev)
		return;

	ethosu_rpmsg_mailbox_deinit(&erp_dev->mailbox);
	rpmsg_destroy_ept(erp_dev->ept);
	ethosu_device_deinit(&erp_dev->edev);
	kfree(erp_dev);
}
