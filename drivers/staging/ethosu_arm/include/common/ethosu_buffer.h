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

#ifndef _ETHOSU_BUFFER_H_
#define _ETHOSU_BUFFER_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/kref.h>
#include <linux/types.h>

/****************************************************************************
 * Types
 ****************************************************************************/

struct device;
struct ethosu_device;
struct ethosu_dma_mem;

/**
 * struct ethosu_buffer - User data buffer
 * @dev:	Device
 * @file:	File
 * @kref:	Reference counting
 * @size:	Buffer size requested by user
 * @size:	Buffer size with alignment
 * @offset:	Buffer offset in DMA memory
 * @dma_mem:	DMA memory allocated for the buffer
 */
struct ethosu_buffer {
	struct device         *dev;
	struct file           *file;
	struct kref           kref;
	size_t                size;
	size_t                aligned_size;
	size_t                offset;
	struct ethosu_dma_mem *dma_mem;
};

/****************************************************************************
 * Functions
 ****************************************************************************/

int ethosu_buffer_batch_create(struct ethosu_device *edev,
			       size_t *sizes,
			       int *fds,
			       uint32_t num);

void ethosu_buffer_free_fds(struct device *dev,
			    int *fds,
			    uint32_t num);

/**
 * ethosu_buffer_from_same_batch() - Check that buffers are from the same batch
 */
bool ethosu_buffer_from_same_batch(struct ethosu_buffer **buffers,
				   uint32_t num);

/**
 * ethosu_buffer_get_from_fd() - Get buffer handle from fd
 *
 * This function must be called from a user space context.
 *
 * Return: Pointer on success, else ERR_PTR.
 */
struct ethosu_buffer *ethosu_buffer_get_from_fd(int fd);

/**
 * ethosu_buffer_get() - Put buffer
 */
void ethosu_buffer_get(struct ethosu_buffer *buf);

/**
 * ethosu_buffer_put() - Put buffer
 */
void ethosu_buffer_put(struct ethosu_buffer *buf);

/**
 * ethosu_buffer_to_dma_addr() - Get DMA address for buffer
 *
 * Return: DMA address on success, else 0U.
 */
dma_addr_t ethosu_buffer_to_dma_addr(const struct ethosu_buffer *buf);

#endif /* _ETHOSU_BUFFER_H_ */
