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

#include <common/ethosu_buffer.h>

#include <common/ethosu_device.h>
#include <common/ethosu_dma_mem.h>
#include <uapi/ethosu.h>

#include <linux/anon_inodes.h>
#include <linux/dma-mapping.h>
#include <linux/of_address.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/remoteproc.h>
#include <linux/uaccess.h>

/****************************************************************************
 * Variables
 ****************************************************************************/

static int ethosu_buffer_release(struct inode *inode,
				 struct file *file);

static long ethosu_buffer_ioctl(struct file *file,
				unsigned int cmd,
				unsigned long arg);

static loff_t ethosu_buffer_llseek(struct file *file,
				   loff_t offset,
				   int whence);

static ssize_t ethosu_buffer_read(struct file *file,
				  char __user *udata,
				  size_t size,
				  loff_t *offset);

static ssize_t ethosu_buffer_write(struct file *file,
				   const char __user *udata,
				   size_t size,
				   loff_t *offset);

static const struct file_operations ethosu_buffer_fops = {
	.release        = &ethosu_buffer_release,
	.unlocked_ioctl = &ethosu_buffer_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = &ethosu_buffer_ioctl,
#endif
	.llseek         = &ethosu_buffer_llseek,
	.write          = &ethosu_buffer_write,
	.read           = &ethosu_buffer_read,
};

/****************************************************************************
 * Functions
 ****************************************************************************/

static bool ethosu_buffer_verify(struct file *file)
{
	return file->f_op == &ethosu_buffer_fops;
}

static struct file *ethosu_buffer_fd_to_file(int fd)
{
	struct file *file = fget(fd);

	if (!file)
		return NULL;

	if (!ethosu_buffer_verify(file)) {
		fput(file);

		return NULL;
	}

	return file;
}

static void ethosu_buffer_destroy(struct kref *kref)
{
	struct ethosu_buffer *buf =
		container_of(kref, struct ethosu_buffer, kref);
	struct device *dev = buf->dev;

	memset(buf->dma_mem->cpu_addr + buf->offset, 0, buf->aligned_size);
	ethosu_dma_mem_put(buf->dma_mem);

	memset(buf, 0, sizeof(*buf));
	devm_kfree(dev, buf);
}

static int ethosu_buffer_release(struct inode *inode,
				 struct file *file)
{
	struct ethosu_buffer *buf = file->private_data;
	struct device *dev = buf->dev;

	dev_dbg(dev, "Buffer release. file=0x%pK, buf=0x%pK\n", file, buf);

	ethosu_buffer_put(buf);

	return 0;
}

static long ethosu_buffer_ioctl(struct file *file,
				unsigned int cmd,
				unsigned long arg)
{
	struct ethosu_buffer *buf = file->private_data;
	struct device *dev = buf->dev;
	int ret;

	ret = device_lock_interruptible(dev);
	if (ret)
		return ret;

	switch (cmd) {
	case ETHOSU_IOCTL_BUFFER_CLEAR: {
		memset(buf->dma_mem->cpu_addr + buf->offset, 0,
		       buf->aligned_size);
		file->f_pos = 0;
		ret = 0;
		break;
	}
	default: {
		dev_err(dev, "Invalid ioctl. cmd=%u, arg=%lu",
			cmd, arg);
		ret = -ENOIOCTLCMD;
		break;
	}
	}

	device_unlock(dev);

	return ret;
}

static loff_t ethosu_buffer_llseek(struct file *file,
				   loff_t offset,
				   int whence)
{
	struct ethosu_buffer *buf = file->private_data;

	switch (whence) {
	case SEEK_CUR:
		offset += file->f_pos;
		fallthrough;
	case SEEK_SET:
		if (offset < 0)
			return -EINVAL;
		else if (offset > buf->size)
			return -ENXIO;

		file->f_pos = offset;

		return file->f_pos;
	case SEEK_END:
		return buf->size;
	default:
		return -EINVAL;
	}
}

static ssize_t ethosu_buffer_read(struct file *file,
				  char __user *udata,
				  size_t size,
				  loff_t *offset)
{
	struct ethosu_buffer *buf = file->private_data;
	loff_t buf_offset = buf->offset + *offset;
	size_t read_size = min_t(size_t, size, buf->size - *offset);

	if (!size)
		return -EINVAL;

	if (*offset == buf->size)
		return 0;
	else if (*offset > buf->size)
		return -EINVAL;

	if (copy_to_user(udata, buf->dma_mem->cpu_addr + buf_offset, read_size))
		return -EFAULT;

	*offset += read_size;

	return read_size;
}

static ssize_t ethosu_buffer_write(struct file *file,
				   const char __user *udata,
				   size_t size,
				   loff_t *offset)
{
	struct ethosu_buffer *buf = file->private_data;
	loff_t buf_offset = buf->offset + *offset;
	size_t write_size = min_t(size_t, size, buf->size - *offset);

	if (!size)
		return -EINVAL;

	if (*offset == buf->size)
		return -ENOSPC;
	else if (*offset > buf->size)
		return -EINVAL;

	if (copy_from_user(buf->dma_mem->cpu_addr + buf_offset, udata,
			   write_size))
		return -EFAULT;

	*offset += write_size;

	return write_size;
}

static inline size_t ethosu_buffer_align_size(struct ethosu_device *edev,
					      size_t size)
{
	size_t alignment = edev->buffer_config.alignment;

	if (!alignment)
		return size;

	return round_up(size, alignment);
}

int ethosu_buffer_batch_create(struct ethosu_device *edev,
			       size_t *sizes,
			       int *fds,
			       uint32_t num)
{
	struct device *dev = &edev->dev;
	struct ethosu_dma_mem *dma_mem;
	struct ethosu_buffer **bufs;
	int ret = -ENOMEM;
	uint32_t i;
	size_t total_size = 0;
	size_t offset = 0;
	size_t *aligned_sizes = NULL;

	if (!num) {
		dev_err(dev, "Buffer batch create. Invalid number of buffers");

		return -EINVAL;
	}

	aligned_sizes = devm_kcalloc(dev, num, sizeof(size_t), GFP_KERNEL);
	if (!aligned_sizes) {
		dev_err(dev,
			"Buffer create. Failed to allocate memory for sizes");

		return -ENOMEM;
	}

	for (i = 0; i < num; i++) {
		if (!sizes[i]) {
			dev_err(dev, "Buffer batch create. Invalid zero size");

			ret = -EINVAL;
			goto free_aligned_sizes;
		}

		aligned_sizes[i] = ethosu_buffer_align_size(edev, sizes[i]);
		total_size += aligned_sizes[i];

		/* Ensure there was no overflow */
		if (aligned_sizes[i] < sizes[i] ||
		    total_size < aligned_sizes[i]) {
			dev_err(dev,
				"Buffer batch create. Requested buffer sizes too large");

			ret = -EINVAL;
			goto free_aligned_sizes;
		}
	}

	if (total_size > edev->buffer_config.max_size) {
		dev_err(dev,
			"Buffer batch create. Buffers exceed max allowed size. Total size supported: %zu bytes requested: %zu bytes",
			edev->buffer_config.max_size, total_size);

		ret = -EINVAL;
		goto free_aligned_sizes;
	}

	dma_mem = ethosu_dma_mem_alloc(dev, total_size);
	if (IS_ERR(dma_mem)) {
		ret = PTR_ERR(dma_mem);
		dev_err(dev,
			"Buffer create. Failed to allocate DMA memory. ret=%d",
			ret);

		goto free_aligned_sizes;
	}

	bufs = devm_kcalloc(dev, num, sizeof(*bufs), GFP_KERNEL);
	if (!bufs) {
		dev_err(dev, "Buffer create. Failed to allocate struct");
		ret = -ENOMEM;
		goto free_dma;
	}

	for (i = 0; i < num; i++) {
		struct ethosu_buffer *buf;
		bufs[i] = devm_kzalloc(dev, sizeof(struct ethosu_buffer),
				       GFP_KERNEL);
		if (!bufs[i]) {
			dev_err(dev,
				"Buffer create. Failed to allocate struct");
			goto free_bufs;
		}

		buf = bufs[i];
		buf->dev = dev;
		buf->offset = offset;
		buf->dma_mem = dma_mem;
		buf->size = sizes[i];
		buf->aligned_size = aligned_sizes[i];
		offset += aligned_sizes[i];

		ethosu_dma_mem_get(dma_mem);
		kref_init(&buf->kref);

		ret = anon_inode_getfd("ethosu-buffer", &ethosu_buffer_fops,
				       buf,
				       O_RDWR | O_CLOEXEC);
		if (ret < 0) {
			dev_err(dev,
				"Buffer create. Failed to get file descriptor. ret=%d",
				ret);
			devm_kfree(dev, buf);
			goto free_bufs;
		}

		buf->file = fget(ret);
		buf->file->f_mode |= FMODE_LSEEK;
		fput(buf->file);

		fds[i] = ret;
	}

	ret = 0;
	goto done;

free_bufs:
	while (i > 0) {
		/* Pre-decrement as the i will be one more than the index */
		struct ethosu_buffer *buf = bufs[--i];
		ethosu_dma_mem_put(buf->dma_mem);
		fput(buf->file);
		devm_kfree(dev, buf);
	}

done:
	memset(bufs, 0, sizeof(*bufs) * num);
	devm_kfree(dev, bufs);

free_dma:
	ethosu_dma_mem_put(dma_mem);

free_aligned_sizes:
	memset(aligned_sizes, 0, sizeof(size_t) * num);
	devm_kfree(dev, aligned_sizes);

	return ret;
}

void ethosu_buffer_free_fds(struct device *dev,
			    int *fds,
			    uint32_t num)
{
	uint32_t i;

	for (i = 0; i < num; i++) {
		struct file *file = ethosu_buffer_fd_to_file(fds[i]);
		if (!file)
			continue;

		fput(file);
	}
}

bool ethosu_buffer_from_same_batch(struct ethosu_buffer **buffers,
				   uint32_t num)
{
	uint32_t i;
	const struct ethosu_dma_mem *dma_mem;

	switch (num) {
	case 0:
		fallthrough;
	case 1:
		return true;
	default:
		dma_mem = buffers[0]->dma_mem;
		for (i = 1; i < num; i++)
			if (dma_mem != buffers[i]->dma_mem)
				return false;
	}

	return true;
}

struct ethosu_buffer *ethosu_buffer_get_from_fd(int fd)
{
	struct ethosu_buffer *buf;
	struct file *file = ethosu_buffer_fd_to_file(fd);

	if (!file)
		return ERR_PTR(-EINVAL);

	buf = file->private_data;
	ethosu_buffer_get(buf);
	fput(file);

	return buf;
}

dma_addr_t ethosu_buffer_to_dma_addr(const struct ethosu_buffer *buf)
{
	if (!buf)
		return 0U;

	return buf->dma_mem->dma_addr + buf->offset;
}

void ethosu_buffer_get(struct ethosu_buffer *buf)
{
	kref_get(&buf->kref);
}

void ethosu_buffer_put(struct ethosu_buffer *buf)
{
	kref_put(&buf->kref, ethosu_buffer_destroy);
}
