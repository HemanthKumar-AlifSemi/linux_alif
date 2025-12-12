// SPDX-License-Identifier: GPL-2.0
/*
 * rpmsg client driver using mailbox client interface
 *
 * Copyright (C) 2019 ARM Ltd.
 *
 */

#include <linux/platform_device.h>
#include <linux/bitmap.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/mailbox_client.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/processor.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/rpmsg.h>
#include <linux/platform_device.h>
#include "rpmsg_internal.h"
#include <linux/mailbox/arm_mhuv2_message.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define ARM_CHANNEL_NAME_MAX	32
#define RPMSG_NAME	"arm_rpmsg"
#define RPMSG_ADDR_ANY	0xFFFFFFFF

struct arm_channel {
	struct rpmsg_endpoint ept;
	struct mbox_client cl;
	struct mbox_chan *mbox;
	char name[ARM_CHANNEL_NAME_MAX];
};

#define RPMSG_IOCTL_MAGIC  'k'
#define RPMSG_IOCTL_SETVAL _IOW(RPMSG_IOCTL_MAGIC, 1, unsigned int)
#define RPMSG_IOCTL_GETVAL _IOR(RPMSG_IOCTL_MAGIC, 2, unsigned int)
#define RPMSG_IOCTL_MAXNR 2

#define arm_channel_from_rpmsg(_ept) container_of(_ept, struct arm_channel, ept)
#define arm_channel_from_mbox(_ept) container_of(_ept, struct arm_channel, cl)

static atomic_t kernel_value = ATOMIC_INIT(0);

/*
 * is_m55_channel() - Check if channel is M55 HP/HE endpoint
 * @name: Channel name to check
 *
 * M55 HP/HE channels (rxdb0-3) require data dereferencing from the
 * arm_mhuv2_mbox_msg structure. Other channels use legacy behavior.
 *
 * Return: true if M55 channel, false otherwise
 */
static bool is_m55_channel(const char *name)
{
	static const char * const m55_channels[] = {
		"rxdb0", "rxdb1", "rxdb2", "rxdb3"
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(m55_channels); i++) {
		if (!strcmp(name, m55_channels[i]))
			return true;
	}
	return false;
}

static void arm_msg_rx_handler(struct mbox_client *cl, void *mssg)
{
	struct arm_channel *channel = arm_channel_from_mbox(cl);
	int err;

	/*
	 * For M55 HP/HE channels (rxdb0-3), the mssg parameter is a pointer
	 * to struct arm_mhuv2_mbox_msg. We must dereference msg->data to get
	 * the actual payload. For SE channels (rxdb4-5), maintain legacy
	 * behavior by passing the pointer directly for backward compatibility.
	 */
	if (is_m55_channel(channel->name)) {
		struct arm_mhuv2_mbox_msg *msg = mssg;
		u32 data;

		/* Validate message pointer before dereferencing */
		if (!msg) {
			pr_err("ARM Mailbox: NULL message pointer (channel: %s)\n", channel->name);
			return;
		}
		if (!msg->data) {
			pr_err("ARM Mailbox: NULL message data pointer (channel: %s)\n",
			       channel->name);
			return;
		}

		data = *(u32 *)msg->data;
		err = channel->ept.cb(channel->ept.rpdev, &data, 4,
				channel->ept.priv, RPMSG_ADDR_ANY);
	} else {
		/* Legacy behavior for SE and other endpoints */
		err = channel->ept.cb(channel->ept.rpdev, mssg, 4,
				channel->ept.priv, RPMSG_ADDR_ANY);
	}
	if (err)
		pr_err("ARM Mailbox: Endpoint callback failed with error: %d", err);
}


static void arm_destroy_ept(struct rpmsg_endpoint *ept)
{
	struct arm_channel *channel = arm_channel_from_rpmsg(ept);

	mbox_free_channel(channel->mbox);
	kfree(channel);
}

static int arm_send(struct rpmsg_endpoint *ept, void *data, int len)
{
	struct arm_channel *channel = arm_channel_from_rpmsg(ept);

	mbox_send_message(channel->mbox, data);
	return 0;
}

static int arm_sendto(struct rpmsg_endpoint *ept, void *data, int len, u32 dest)
{
	struct arm_mhuv2_mbox_msg msg;
	struct arm_channel *channel = arm_channel_from_rpmsg(ept);

	msg.data = data;
	msg.len = len;
	mbox_send_message(channel->mbox, &msg);
	return 0;
}


static const struct rpmsg_endpoint_ops arm_endpoint_ops = {
	.destroy_ept = arm_destroy_ept,
	.send = arm_send,
	.sendto = arm_sendto,
};


static struct rpmsg_endpoint *arm_create_ept(struct rpmsg_device *rpdev,
		rpmsg_rx_cb_t cb, void *priv, struct rpmsg_channel_info chinfo)
{
	struct arm_channel *channel;

	channel = kzalloc(sizeof(*channel), GFP_KERNEL);

	// Initialize rpmsg endpoint
	kref_init(&channel->ept.refcount);
	channel->ept.rpdev = rpdev;
	channel->ept.cb = cb;
	channel->ept.priv = priv;
	channel->ept.ops = &arm_endpoint_ops;

	// Initialize mailbox client
	channel->cl.dev = rpdev->dev.parent;
	channel->cl.rx_callback = arm_msg_rx_handler;
	channel->cl.tx_done = NULL; /* operate in blocking mode */
	channel->cl.tx_block = true;
	channel->cl.tx_tout = 500; /* by half a second */
	channel->cl.knows_txdone = false; /* depending upon protocol */

	channel->mbox = mbox_request_channel_byname(&channel->cl, chinfo.name);
	if (IS_ERR_OR_NULL(channel->mbox)) {
		pr_err("RPMsg ARM: Cannot get channel by name: '%s'\n", chinfo.name);
		return NULL;
	}

	/* Store the name for protocol differentiation in rx_handler */
	strscpy(channel->name, chinfo.name, sizeof(channel->name));

	return &channel->ept;
}

static const struct rpmsg_device_ops arm_device_ops = {
	.create_ept = arm_create_ept,
};


static void arm_release_device(struct device *dev)
{
	struct rpmsg_device *rpdev = to_rpmsg_device(dev);

	kfree(rpdev);
}

static long rpmsg_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	uint32_t user_value, temp;
	int ret = 0;

	switch (cmd) {
	case RPMSG_IOCTL_SETVAL:
		if (copy_from_user(&user_value, (uint32_t __user *)arg, sizeof(user_value)))
			ret = -EFAULT;
		atomic_set(&kernel_value, user_value);
		break;

	case RPMSG_IOCTL_GETVAL:
		temp = readl((void __iomem *)(uintptr_t)atomic_read(&kernel_value));
		if (copy_to_user((uint32_t __user *)arg, &temp, sizeof(temp)))
			ret = -EFAULT;
		break;

	default:
		ret = -ENOTTY;
	}
	return ret;
}

static const struct file_operations rpmsg_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = rpmsg_ioctl,
	.compat_ioctl = rpmsg_ioctl,
};

static struct miscdevice rpmsg_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rpmsg_device",
	.fops = &rpmsg_fops,
	.mode = 0666,
};

static int client_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rpmsg_device *rpdev;
	int ret;

	rpdev = kzalloc(sizeof(*rpdev), GFP_KERNEL);
	if (!rpdev)
		return -ENOMEM;

	/* Assign callbacks for rpmsg_device */
	rpdev->ops = &arm_device_ops;

	/* Assign public information to the rpmsg_device */
	memcpy(rpdev->id.name, RPMSG_NAME, strlen(RPMSG_NAME));

	rpdev->dev.parent = dev;
	rpdev->dev.release = arm_release_device;

	ret = misc_register(&rpmsg_misc_device);
	if (ret) {
		dev_err(dev, "couldn't register misc device: %d\n", ret);
		goto misc_destroy_device;
	}

	return rpmsg_ctrldev_register_device(rpdev);

misc_destroy_device:
		misc_deregister(&rpmsg_misc_device);
		return ret;
}

static const struct of_device_id client_of_match[] = {
	{ .compatible = "arm,client", .data = NULL },
	{ /* Sentinel */ },
};

static struct platform_driver client_driver = {
	.driver = {
		.name = "arm-mhu-client",
		.of_match_table = client_of_match,
	},
	.probe = client_probe,
};

module_platform_driver(client_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ARM RPMSG Driver");
MODULE_AUTHOR("Tushar Khandelwal <tushar.khandelwal@arm.com>");
