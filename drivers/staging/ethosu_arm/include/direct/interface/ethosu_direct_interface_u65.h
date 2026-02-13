/*
 * SPDX-FileCopyrightText: Copyright 2020-2021, 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#ifndef _ETHOSU_DIRECT_INTERFACE_U65_H_
#define _ETHOSU_DIRECT_INTERFACE_U65_H_

#include <linux/types.h>

#define NNX_ARCH_VERSION_MAJOR 1
#define NNX_ARCH_VERSION_MINOR 0
#define NNX_ARCH_VERSION_PATCH 6

/* Register offsets */

/* Register subpage BASE */

#define NPU_REG_CONFIG 0x0028
#define NPU_REG_LOCK 0x002C
#define NPU_REG_AXI_LIMIT_BASE 0x0040
#define NPU_REG_AXI_LIMIT_ARRLEN 0x0004
#define NPU_REG_AXI_LIMIT_OFFSET 0x0004
#define BASE_REGISTERS_SIZE 0x0080

/* Register subpage DEBUG */

#define NPU_REG_WD_STATUS 0x0100
#define NPU_REG_MAC_STATUS 0x0104
#define NPU_REG_AO_STATUS 0x0108
#define NPU_REG_DMA_STATUS0 0x0110
#define NPU_REG_DMA_STATUS1 0x0114
#define NPU_REG_CLKFORCE 0x0140
#define NPU_REG_DEBUG_ADDRESS 0x0144
#define NPU_REG_DEBUG_MISC 0x0148
#define NPU_REG_DEBUGCORE 0x014C
#define NPU_REG_DEBUG_BLOCK 0x0150
#define DEBUG_REGISTERS_SIZE 0x0180

/* Register subpage PMU */

#define NPU_REG_PMCR 0x0180
#define NPU_REG_PMCNTENSET 0x0184
#define NPU_REG_PMCNTENCLR 0x0188
#define NPU_REG_PMOVSSET 0x018C
#define NPU_REG_PMOVSCLR 0x0190
#define NPU_REG_PMINTSET 0x0194
#define NPU_REG_PMINTCLR 0x0198
#define NPU_REG_PMCCNTR 0x01A0
#define NPU_REG_PMCCNTR_HI 0x01A4
#define NPU_REG_PMCCNTR_CFG 0x01A8
#define NPU_REG_PMCAXI_CHAN 0x01AC
#define PMU_REGISTERS_SIZE 0x0200

/* Register subpage TSU_DEBUG */

#define NPU_REG_KERNEL_X 0x0200
#define NPU_REG_KERNEL_Y 0x0204
#define NPU_REG_KERNEL_W_M1 0x0208
#define NPU_REG_KERNEL_H_M1 0x020C
#define NPU_REG_OFM_CBLK_WIDTH_M1 0x0210
#define NPU_REG_OFM_CBLK_HEIGHT_M1 0x0214
#define NPU_REG_OFM_CBLK_DEPTH_M1 0x0218
#define NPU_REG_IFM_CBLK_DEPTH_M1 0x021C
#define NPU_REG_OFM_X 0x0220
#define NPU_REG_OFM_Y 0x0224
#define NPU_REG_OFM_Z 0x0228
#define NPU_REG_IFM_Z 0x022C
#define NPU_REG_PAD_TOP 0x0230
#define NPU_REG_PAD_LEFT 0x0234
#define NPU_REG_IFM_CBLK_WIDTH 0x0238
#define NPU_REG_IFM_CBLK_HEIGHT 0x023C
#define NPU_REG_DMA_IFM_SRC 0x0240
#define NPU_REG_DMA_IFM_SRC_HI 0x0244
#define NPU_REG_DMA_IFM_DST 0x0248
#define NPU_REG_DMA_OFM_SRC 0x024C
#define NPU_REG_DMA_OFM_DST 0x0250
#define NPU_REG_DMA_OFM_DST_HI 0x0254
#define NPU_REG_DMA_WEIGHT_SRC 0x0258
#define NPU_REG_DMA_WEIGHT_SRC_HI 0x025C
#define NPU_REG_DMA_CMD_SRC 0x0260
#define NPU_REG_DMA_CMD_SRC_HI 0x0264
#define NPU_REG_DMA_CMD_SIZE 0x0268
#define NPU_REG_DMA_M2M_SRC 0x026C
#define NPU_REG_DMA_M2M_SRC_HI 0x0270
#define NPU_REG_DMA_M2M_DST 0x0274
#define NPU_REG_DMA_M2M_DST_HI 0x0278
#define NPU_REG_CURRENT_QREAD 0x027C
#define NPU_REG_DMA_SCALE_SRC 0x0280
#define NPU_REG_DMA_SCALE_SRC_HI 0x0284
#define NPU_REG_CURRENT_BLOCK 0x02B4
#define NPU_REG_CURRENT_OP 0x02B8
#define NPU_REG_CURRENT_CMD 0x02BC
#define TSU_DEBUG_REGISTERS_SIZE 0x02C0

/* Register subpage PMU_COUNTERS */

#define NPU_REG_PMEVCNTR_BASE 0x0300
#define NPU_REG_PMEVCNTR_ARRLEN 0x0004
#define NPU_REG_PMEVCNTR_OFFSET 0x0004
#define NPU_REG_PMEVTYPER_BASE 0x0380
#define NPU_REG_PMEVTYPER_ARRLEN 0x0004
#define NPU_REG_PMEVTYPER_OFFSET 0x0004
#define PMU_COUNTERS_REGISTERS_SIZE 0x0400

/* Register subpage SHARED_BUFFER */

#define NPU_REG_SHARED_BUFFER_BASE 0x0400
#define NPU_REG_SHARED_BUFFER_ARRLEN 0x0100
#define SHARED_BUFFER_REGISTERS_SIZE 0x0800

/* Register subpage TSU_IFM */

#define NPU_REG_IFM_PAD_TOP 0x0800
#define NPU_REG_IFM_PAD_LEFT 0x0804
#define NPU_REG_IFM_PAD_RIGHT 0x0808
#define NPU_REG_IFM_PAD_BOTTOM 0x080C
#define NPU_REG_IFM_DEPTH_M1 0x0810
#define NPU_REG_IFM_PRECISION 0x0814
#define NPU_REG_IFM_UPSCALE 0x081C
#define NPU_REG_IFM_ZERO_POINT 0x0824
#define NPU_REG_IFM_WIDTH0_M1 0x0828
#define NPU_REG_IFM_HEIGHT0_M1 0x082C
#define NPU_REG_IFM_HEIGHT1_M1 0x0830
#define NPU_REG_IFM_IB_END 0x0834
#define NPU_REG_IFM_REGION 0x083C
#define TSU_IFM_REGISTERS_SIZE 0x0840

/* Register subpage TSU_OFM */

#define NPU_REG_OFM_WIDTH_M1 0x0844
#define NPU_REG_OFM_HEIGHT_M1 0x0848
#define NPU_REG_OFM_DEPTH_M1 0x084C
#define NPU_REG_OFM_PRECISION 0x0850
#define NPU_REG_OFM_BLK_WIDTH_M1 0x0854
#define NPU_REG_OFM_BLK_HEIGHT_M1 0x0858
#define NPU_REG_OFM_BLK_DEPTH_M1 0x085C
#define NPU_REG_OFM_ZERO_POINT 0x0860
#define NPU_REG_OFM_WIDTH0_M1 0x0868
#define NPU_REG_OFM_HEIGHT0_M1 0x086C
#define NPU_REG_OFM_HEIGHT1_M1 0x0870
#define NPU_REG_OFM_REGION 0x087C
#define TSU_OFM_REGISTERS_SIZE 0x0880

/* Register subpage TSU_KERNEL */

#define NPU_REG_KERNEL_WIDTH_M1 0x0880
#define NPU_REG_KERNEL_HEIGHT_M1 0x0884
#define NPU_REG_KERNEL_STRIDE 0x0888
#define NPU_REG_PARALLEL_MODE 0x088C
#define NPU_REG_ACC_FORMAT 0x0890
#define NPU_REG_ACTIVATION 0x0894
#define NPU_REG_ACTIVATION_MIN 0x0898
#define NPU_REG_ACTIVATION_MAX 0x089C
#define NPU_REG_WEIGHT_REGION 0x08A0
#define NPU_REG_SCALE_REGION 0x08A4
#define NPU_REG_AB_START 0x08B4
#define NPU_REG_BLOCKDEP 0x08BC
#define TSU_KERNEL_REGISTERS_SIZE 0x08C0

/* Register subpage TSU_DMA */

#define NPU_REG_DMA0_SRC_REGION 0x08C0
#define NPU_REG_DMA0_DST_REGION 0x08C4
#define NPU_REG_DMA0_SIZE0 0x08C8
#define NPU_REG_DMA0_SIZE1 0x08CC
#define TSU_DMA_REGISTERS_SIZE 0x0900

/* Register subpage TSU_IFM2 */

#define NPU_REG_IFM2_BROADCAST 0x0900
#define NPU_REG_IFM2_SCALAR 0x0904
#define NPU_REG_IFM2_PRECISION 0x0914
#define NPU_REG_IFM2_ZERO_POINT 0x0924
#define NPU_REG_IFM2_WIDTH0_M1 0x0928
#define NPU_REG_IFM2_HEIGHT0_M1 0x092C
#define NPU_REG_IFM2_HEIGHT1_M1 0x0930
#define NPU_REG_IFM2_IB_START 0x0934
#define NPU_REG_IFM2_REGION 0x093C
#define TSU_IFM2_REGISTERS_SIZE 0x0940

/* Register subpage TSU_IFM_BASE */

#define NPU_REG_IFM_BASE0 0x0A00
#define NPU_REG_IFM_BASE0_HI 0x0A04
#define NPU_REG_IFM_BASE1 0x0A08
#define NPU_REG_IFM_BASE1_HI 0x0A0C
#define NPU_REG_IFM_BASE2 0x0A10
#define NPU_REG_IFM_BASE2_HI 0x0A14
#define NPU_REG_IFM_BASE3 0x0A18
#define NPU_REG_IFM_BASE3_HI 0x0A1C
#define NPU_REG_IFM_STRIDE_X 0x0A20
#define NPU_REG_IFM_STRIDE_X_HI 0x0A24
#define NPU_REG_IFM_STRIDE_Y 0x0A28
#define NPU_REG_IFM_STRIDE_Y_HI 0x0A2C
#define NPU_REG_IFM_STRIDE_C 0x0A30
#define NPU_REG_IFM_STRIDE_C_HI 0x0A34
#define TSU_IFM_BASE_REGISTERS_SIZE 0x0A40

/* Register subpage TSU_OFM_BASE */

#define NPU_REG_OFM_BASE0 0x0A40
#define NPU_REG_OFM_BASE0_HI 0x0A44
#define NPU_REG_OFM_BASE1 0x0A48
#define NPU_REG_OFM_BASE1_HI 0x0A4C
#define NPU_REG_OFM_BASE2 0x0A50
#define NPU_REG_OFM_BASE2_HI 0x0A54
#define NPU_REG_OFM_BASE3 0x0A58
#define NPU_REG_OFM_BASE3_HI 0x0A5C
#define NPU_REG_OFM_STRIDE_X 0x0A60
#define NPU_REG_OFM_STRIDE_X_HI 0x0A64
#define NPU_REG_OFM_STRIDE_Y 0x0A68
#define NPU_REG_OFM_STRIDE_Y_HI 0x0A6C
#define NPU_REG_OFM_STRIDE_C 0x0A70
#define NPU_REG_OFM_STRIDE_C_HI 0x0A74
#define TSU_OFM_BASE_REGISTERS_SIZE 0x0A80

/* Register subpage TSU_WS_BASE */

#define NPU_REG_WEIGHT_BASE 0x0A80
#define NPU_REG_WEIGHT_BASE_HI 0x0A84
#define NPU_REG_WEIGHT_LENGTH 0x0A88
#define NPU_REG_WEIGHT_LENGTH_HI 0x0A8C
#define NPU_REG_SCALE_BASE 0x0A90
#define NPU_REG_SCALE_BASE_HI 0x0A94
#define NPU_REG_SCALE_LENGTH 0x0A98
#define NPU_REG_SCALE_LENGTH_HI 0x0A9C
#define NPU_REG_OFM_SCALE 0x0AA0
#define NPU_REG_OFM_SCALE_SHIFT 0x0AA4
#define NPU_REG_OPA_SCALE 0x0AA8
#define NPU_REG_OPA_SCALE_SHIFT 0x0AAC
#define NPU_REG_OPB_SCALE 0x0AB0
#define TSU_WS_BASE_REGISTERS_SIZE 0x0AC0

/* Register subpage TSU_DMA_BASE */

#define NPU_REG_DMA0_SRC 0x0AC0
#define NPU_REG_DMA0_SRC_HI 0x0AC4
#define NPU_REG_DMA0_DST 0x0AC8
#define NPU_REG_DMA0_DST_HI 0x0ACC
#define NPU_REG_DMA0_LEN 0x0AD0
#define NPU_REG_DMA0_LEN_HI 0x0AD4
#define NPU_REG_DMA0_SKIP0 0x0AD8
#define NPU_REG_DMA0_SKIP0_HI 0x0ADC
#define NPU_REG_DMA0_SKIP1 0x0AE0
#define NPU_REG_DMA0_SKIP1_HI 0x0AE4
#define TSU_DMA_BASE_REGISTERS_SIZE 0x0B00

/* Register subpage TSU_IFM2_BASE */

#define NPU_REG_IFM2_BASE0 0x0B00
#define NPU_REG_IFM2_BASE0_HI 0x0B04
#define NPU_REG_IFM2_BASE1 0x0B08
#define NPU_REG_IFM2_BASE1_HI 0x0B0C
#define NPU_REG_IFM2_BASE2 0x0B10
#define NPU_REG_IFM2_BASE2_HI 0x0B14
#define NPU_REG_IFM2_BASE3 0x0B18
#define NPU_REG_IFM2_BASE3_HI 0x0B1C
#define NPU_REG_IFM2_STRIDE_X 0x0B20
#define NPU_REG_IFM2_STRIDE_X_HI 0x0B24
#define NPU_REG_IFM2_STRIDE_Y 0x0B28
#define NPU_REG_IFM2_STRIDE_Y_HI 0x0B2C
#define NPU_REG_IFM2_STRIDE_C 0x0B30
#define NPU_REG_IFM2_STRIDE_C_HI 0x0B34
#define TSU_IFM2_BASE_REGISTERS_SIZE 0x0B40

/* Register subpage TSU_WS1_BASE */

#define NPU_REG_WEIGHT1_BASE 0x0B40
#define NPU_REG_WEIGHT1_BASE_HI 0x0B44
#define NPU_REG_WEIGHT1_LENGTH 0x0B48
#define NPU_REG_WEIGHT1_LENGTH_HI 0x0B4C
#define NPU_REG_SCALE1_BASE 0x0B50
#define NPU_REG_SCALE1_BASE_HI 0x0B54
#define NPU_REG_SCALE1_LENGTH 0x0B58
#define NPU_REG_SCALE1_LENGTH_HI 0x0B5C
#define TSU_WS1_BASE_REGISTERS_SIZE 0x0B80

/* Register subpage TSU_USER_BASE */

#define TSU_USER_BASE_REGISTERS_SIZE 0x0BC0

/* Register subpage TSU_DMA_EBASE */

#define TSU_DMA_EBASE_REGISTERS_SIZE 0x0C00

/* Register subpage ID */

#define NPU_REG_REVISION 0x0FC0
#define NPU_REG_PID4 0x0FD0
#define NPU_REG_PID5 0x0FD4
#define NPU_REG_PID6 0x0FD8
#define NPU_REG_PID7 0x0FDC
#define NPU_REG_PID0 0x0FE0
#define NPU_REG_PID1 0x0FE4
#define NPU_REG_PID2 0x0FE8
#define NPU_REG_PID3 0x0FEC
#define NPU_REG_CID0 0x0FF0
#define NPU_REG_CID1 0x0FF4
#define NPU_REG_CID2 0x0FF8
#define NPU_REG_CID3 0x0FFC
#define ID_REGISTERS_SIZE 0x1000

enum acc_format {
	ACC_FORMAT_I32 = 0,
	ACC_FORMAT_I40 = 1,
	ACC_FORMAT_F16 = 2,
};

enum activation_clip_range {
	ACTIVATION_CLIP_RANGE_OFM_PRECISION = 0,
	ACTIVATION_CLIP_RANGE_FORCE_UINT8   = 2,
	ACTIVATION_CLIP_RANGE_FORCE_INT8    = 3,
	ACTIVATION_CLIP_RANGE_FORCE_INT16   = 5,
};

enum activation_format {
	ACTIVATION_FORMAT_NHWC    = 0,
	ACTIVATION_FORMAT_NHCWB16 = 1,
};

enum activation_function {
	ACTIVATION_FUNCTION_RELU    = 0,
	ACTIVATION_FUNCTION_TANH    = 3,
	ACTIVATION_FUNCTION_SIGMOID = 4,
	ACTIVATION_FUNCTION_TABLE_0 = 16,
	ACTIVATION_FUNCTION_TABLE_1 = 17,
	ACTIVATION_FUNCTION_TABLE_2 = 18,
	ACTIVATION_FUNCTION_TABLE_3 = 19,
	ACTIVATION_FUNCTION_TABLE_4 = 20,
	ACTIVATION_FUNCTION_TABLE_5 = 21,
	ACTIVATION_FUNCTION_TABLE_6 = 22,
	ACTIVATION_FUNCTION_TABLE_7 = 23,
};

enum activation_precision {
	ACTIVATION_PRECISION_B8  = 0,
	ACTIVATION_PRECISION_B16 = 1,
	ACTIVATION_PRECISION_B32 = 2,
	ACTIVATION_PRECISION_B64 = 3,
};

enum activation_type {
	ACTIVATION_TYPE_UNSIGNED = 0,
	ACTIVATION_TYPE_SIGNED   = 1,
};

enum axi_mem_encoding {
	AXI_MEM_ENCODING_DEVICE_NON_BUFFERABLE                 = 0,
	AXI_MEM_ENCODING_DEVICE_BUFFERABLE                     = 1,
	AXI_MEM_ENCODING_NORMAL_NON_CACHEABLE_NON_BUFFERABLE   = 2,
	AXI_MEM_ENCODING_NORMAL_NON_CACHEABLE_BUFFERABLE       = 3,
	AXI_MEM_ENCODING_WRITE_THROUGH_NO_ALLOCATE             = 4,
	AXI_MEM_ENCODING_WRITE_THROUGH_READ_ALLOCATE           = 5,
	AXI_MEM_ENCODING_WRITE_THROUGH_WRITE_ALLOCATE          = 6,
	AXI_MEM_ENCODING_WRITE_THROUGH_READ_AND_WRITE_ALLOCATE = 7,
	AXI_MEM_ENCODING_WRITE_BACK_NO_ALLOCATE                = 8,
	AXI_MEM_ENCODING_WRITE_BACK_READ_ALLOCATE              = 9,
	AXI_MEM_ENCODING_WRITE_BACK_WRITE_ALLOCATE             = 10,
	AXI_MEM_ENCODING_WRITE_BACK_READ_AND_WRITE_ALLOCATE    = 11,
};

enum broadcast_mode {
	BROADCAST_MODE_DISABLE = 0,
	BROADCAST_MODE_ENABLE  = 1,
};

enum cmd0_opcode {
	CMD0_OPCODE_NPU_OP_STOP               = 0,
	CMD0_OPCODE_NPU_OP_IRQ                = 1,
	CMD0_OPCODE_NPU_OP_CONV               = 2,
	CMD0_OPCODE_NPU_OP_DEPTHWISE          = 3,
	CMD0_OPCODE_NPU_OP_POOL               = 5,
	CMD0_OPCODE_NPU_OP_ELEMENTWISE        = 6,
	CMD0_OPCODE_NPU_OP_DMA_START          = 16,
	CMD0_OPCODE_NPU_OP_DMA_WAIT           = 17,
	CMD0_OPCODE_NPU_OP_KERNEL_WAIT        = 18,
	CMD0_OPCODE_NPU_OP_PMU_MASK           = 19,
	CMD0_OPCODE_NPU_SET_IFM_PAD_TOP       = 256,
	CMD0_OPCODE_NPU_SET_IFM_PAD_LEFT      = 257,
	CMD0_OPCODE_NPU_SET_IFM_PAD_RIGHT     = 258,
	CMD0_OPCODE_NPU_SET_IFM_PAD_BOTTOM    = 259,
	CMD0_OPCODE_NPU_SET_IFM_DEPTH_M1      = 260,
	CMD0_OPCODE_NPU_SET_IFM_PRECISION     = 261,
	CMD0_OPCODE_NPU_SET_IFM_UPSCALE       = 263,
	CMD0_OPCODE_NPU_SET_IFM_ZERO_POINT    = 265,
	CMD0_OPCODE_NPU_SET_IFM_WIDTH0_M1     = 266,
	CMD0_OPCODE_NPU_SET_IFM_HEIGHT0_M1    = 267,
	CMD0_OPCODE_NPU_SET_IFM_HEIGHT1_M1    = 268,
	CMD0_OPCODE_NPU_SET_IFM_IB_END        = 269,
	CMD0_OPCODE_NPU_SET_IFM_REGION        = 271,
	CMD0_OPCODE_NPU_SET_OFM_WIDTH_M1      = 273,
	CMD0_OPCODE_NPU_SET_OFM_HEIGHT_M1     = 274,
	CMD0_OPCODE_NPU_SET_OFM_DEPTH_M1      = 275,
	CMD0_OPCODE_NPU_SET_OFM_PRECISION     = 276,
	CMD0_OPCODE_NPU_SET_OFM_BLK_WIDTH_M1  = 277,
	CMD0_OPCODE_NPU_SET_OFM_BLK_HEIGHT_M1 = 278,
	CMD0_OPCODE_NPU_SET_OFM_BLK_DEPTH_M1  = 279,
	CMD0_OPCODE_NPU_SET_OFM_ZERO_POINT    = 280,
	CMD0_OPCODE_NPU_SET_OFM_WIDTH0_M1     = 282,
	CMD0_OPCODE_NPU_SET_OFM_HEIGHT0_M1    = 283,
	CMD0_OPCODE_NPU_SET_OFM_HEIGHT1_M1    = 284,
	CMD0_OPCODE_NPU_SET_OFM_REGION        = 287,
	CMD0_OPCODE_NPU_SET_KERNEL_WIDTH_M1   = 288,
	CMD0_OPCODE_NPU_SET_KERNEL_HEIGHT_M1  = 289,
	CMD0_OPCODE_NPU_SET_KERNEL_STRIDE     = 290,
	CMD0_OPCODE_NPU_SET_PARALLEL_MODE     = 291,
	CMD0_OPCODE_NPU_SET_ACC_FORMAT        = 292,
	CMD0_OPCODE_NPU_SET_ACTIVATION        = 293,
	CMD0_OPCODE_NPU_SET_ACTIVATION_MIN    = 294,
	CMD0_OPCODE_NPU_SET_ACTIVATION_MAX    = 295,
	CMD0_OPCODE_NPU_SET_WEIGHT_REGION     = 296,
	CMD0_OPCODE_NPU_SET_SCALE_REGION      = 297,
	CMD0_OPCODE_NPU_SET_AB_START          = 301,
	CMD0_OPCODE_NPU_SET_BLOCKDEP          = 303,
	CMD0_OPCODE_NPU_SET_DMA0_SRC_REGION   = 304,
	CMD0_OPCODE_NPU_SET_DMA0_DST_REGION   = 305,
	CMD0_OPCODE_NPU_SET_DMA0_SIZE0        = 306,
	CMD0_OPCODE_NPU_SET_DMA0_SIZE1        = 307,
	CMD0_OPCODE_NPU_SET_IFM2_BROADCAST    = 384,
	CMD0_OPCODE_NPU_SET_IFM2_SCALAR       = 385,
	CMD0_OPCODE_NPU_SET_IFM2_PRECISION    = 389,
	CMD0_OPCODE_NPU_SET_IFM2_ZERO_POINT   = 393,
	CMD0_OPCODE_NPU_SET_IFM2_WIDTH0_M1    = 394,
	CMD0_OPCODE_NPU_SET_IFM2_HEIGHT0_M1   = 395,
	CMD0_OPCODE_NPU_SET_IFM2_HEIGHT1_M1   = 396,
	CMD0_OPCODE_NPU_SET_IFM2_IB_START     = 397,
	CMD0_OPCODE_NPU_SET_IFM2_REGION       = 399,
};

enum cmd1_opcode {
	CMD1_OPCODE_NPU_SET_IFM_BASE0      = 0,
	CMD1_OPCODE_NPU_SET_IFM_BASE1      = 1,
	CMD1_OPCODE_NPU_SET_IFM_BASE2      = 2,
	CMD1_OPCODE_NPU_SET_IFM_BASE3      = 3,
	CMD1_OPCODE_NPU_SET_IFM_STRIDE_X   = 4,
	CMD1_OPCODE_NPU_SET_IFM_STRIDE_Y   = 5,
	CMD1_OPCODE_NPU_SET_IFM_STRIDE_C   = 6,
	CMD1_OPCODE_NPU_SET_OFM_BASE0      = 16,
	CMD1_OPCODE_NPU_SET_OFM_BASE1      = 17,
	CMD1_OPCODE_NPU_SET_OFM_BASE2      = 18,
	CMD1_OPCODE_NPU_SET_OFM_BASE3      = 19,
	CMD1_OPCODE_NPU_SET_OFM_STRIDE_X   = 20,
	CMD1_OPCODE_NPU_SET_OFM_STRIDE_Y   = 21,
	CMD1_OPCODE_NPU_SET_OFM_STRIDE_C   = 22,
	CMD1_OPCODE_NPU_SET_WEIGHT_BASE    = 32,
	CMD1_OPCODE_NPU_SET_WEIGHT_LENGTH  = 33,
	CMD1_OPCODE_NPU_SET_SCALE_BASE     = 34,
	CMD1_OPCODE_NPU_SET_SCALE_LENGTH   = 35,
	CMD1_OPCODE_NPU_SET_OFM_SCALE      = 36,
	CMD1_OPCODE_NPU_SET_OPA_SCALE      = 37,
	CMD1_OPCODE_NPU_SET_OPB_SCALE      = 38,
	CMD1_OPCODE_NPU_SET_DMA0_SRC       = 48,
	CMD1_OPCODE_NPU_SET_DMA0_DST       = 49,
	CMD1_OPCODE_NPU_SET_DMA0_LEN       = 50,
	CMD1_OPCODE_NPU_SET_DMA0_SKIP0     = 51,
	CMD1_OPCODE_NPU_SET_DMA0_SKIP1     = 52,
	CMD1_OPCODE_NPU_SET_IFM2_BASE0     = 128,
	CMD1_OPCODE_NPU_SET_IFM2_BASE1     = 129,
	CMD1_OPCODE_NPU_SET_IFM2_BASE2     = 130,
	CMD1_OPCODE_NPU_SET_IFM2_BASE3     = 131,
	CMD1_OPCODE_NPU_SET_IFM2_STRIDE_X  = 132,
	CMD1_OPCODE_NPU_SET_IFM2_STRIDE_Y  = 133,
	CMD1_OPCODE_NPU_SET_IFM2_STRIDE_C  = 134,
	CMD1_OPCODE_NPU_SET_WEIGHT1_BASE   = 144,
	CMD1_OPCODE_NPU_SET_WEIGHT1_LENGTH = 145,
	CMD1_OPCODE_NPU_SET_SCALE1_BASE    = 146,
	CMD1_OPCODE_NPU_SET_SCALE1_LENGTH  = 147,
};

enum cmd_ctrl {
	CMD_CTRL_CMD0_CTRL = 0,
	CMD_CTRL_CMD1_CTRL = 1,
};

enum custom_dma {
	CUSTOM_DMA_NOT_IMPLEMENTED = 0,
	CUSTOM_DMA_IMPLEMENTED     = 1,
};

enum dma_fault_src {
	DMA_FAULT_SRC_AXI_M0 = 0,
	DMA_FAULT_SRC_AXI_M1 = 1,
};

enum dma_region_mode {
	DMA_REGION_MODE_EXTERNAL = 0,
	DMA_REGION_MODE_INTERNAL = 1,
};

enum dma_stride_mode {
	DMA_STRIDE_MODE_D1 = 0,
	DMA_STRIDE_MODE_D2 = 1,
	DMA_STRIDE_MODE_D3 = 2,
};

enum elementwise_mode {
	ELEMENTWISE_MODE_MUL   = 0,
	ELEMENTWISE_MODE_ADD   = 1,
	ELEMENTWISE_MODE_SUB   = 2,
	ELEMENTWISE_MODE_MIN   = 3,
	ELEMENTWISE_MODE_MAX   = 4,
	ELEMENTWISE_MODE_LRELU = 5,
	ELEMENTWISE_MODE_ABS   = 6,
	ELEMENTWISE_MODE_CLZ   = 7,
	ELEMENTWISE_MODE_SHR   = 8,
	ELEMENTWISE_MODE_SHL   = 9,
};

enum functional_safety {
	FUNCTIONAL_SAFETY_NOT_IMPLEMENTED = 0,
	FUNCTIONAL_SAFETY_IMPLEMENTED     = 1,
};

enum ifm2_operand_order {
	IFM2_OPERAND_ORDER_ORDER_B = 0,
	IFM2_OPERAND_ORDER_ORDER_A = 1,
};

enum ifm_scale_mode {
	IFM_SCALE_MODE_OPA_OPB_16 = 0,
	IFM_SCALE_MODE_OPA_32     = 1,
	IFM_SCALE_MODE_OPB_32     = 2,
};

enum ifm_upscale_mode {
	IFM_UPSCALE_MODE_NONE    = 0,
	IFM_UPSCALE_MODE_NEAREST = 1,
	IFM_UPSCALE_MODE_ZEROS   = 2,
};

enum kernel_decomposition {
	KERNEL_DECOMPOSITION_D8X8 = 0,
	KERNEL_DECOMPOSITION_D4X4 = 1,
};

enum kernel_dilation {
	KERNEL_DILATION_NONE = 0,
	KERNEL_DILATION_X2   = 1,
};

enum max_beats {
	MAX_BEATS_B64  = 0,
	MAX_BEATS_B128 = 1,
	MAX_BEATS_B256 = 2,
};

enum mem_attr {
	MEM_ATTR_AXI0_OUTSTANDING_COUNTER0 = 0,
	MEM_ATTR_AXI0_OUTSTANDING_COUNTER1 = 1,
	MEM_ATTR_AXI1_OUTSTANDING_COUNTER2 = 2,
	MEM_ATTR_AXI1_OUTSTANDING_COUNTER3 = 3,
};

enum ofm_scale_mode {
	OFM_SCALE_MODE_PER_CHANNEL = 0,
	OFM_SCALE_MODE_GLOBAL      = 1,
};

enum parallel_mode {
	PARALLEL_MODE_SINGLE_CORE     = 0,
	PARALLEL_MODE_DUAL_CORE_DEPTH = 1,
};

enum pmu_axi_channel {
	PMU_AXI_CHANNEL_RD_CMD        = 0,
	PMU_AXI_CHANNEL_RD_IFM        = 1,
	PMU_AXI_CHANNEL_RD_WEIGHTS    = 2,
	PMU_AXI_CHANNEL_RD_SCALE_BIAS = 3,
	PMU_AXI_CHANNEL_RD_MEM2MEM    = 4,
	PMU_AXI_CHANNEL_WR_OFM        = 8,
	PMU_AXI_CHANNEL_WR_MEM2MEM    = 9,
};

enum pmu_event {
	PMU_EVENT_NO_EVENT                     = 0,
	PMU_EVENT_CYCLE                        = 17,
	PMU_EVENT_NPU_IDLE                     = 32,
	PMU_EVENT_CC_STALLED_ON_BLOCKDEP       = 33,
	PMU_EVENT_CC_STALLED_ON_SHRAM_RECONFIG = 34,
	PMU_EVENT_NPU_ACTIVE                   = 35,
	PMU_EVENT_MAC_ACTIVE                   = 48,
	PMU_EVENT_MAC_ACTIVE_8BIT              = 49,
	PMU_EVENT_MAC_ACTIVE_16BIT             = 50,
	PMU_EVENT_MAC_DPU_ACTIVE               = 51,
	PMU_EVENT_MAC_STALLED_BY_WD_ACC        = 52,
	PMU_EVENT_MAC_STALLED_BY_WD            = 53,
	PMU_EVENT_MAC_STALLED_BY_ACC           = 54,
	PMU_EVENT_MAC_STALLED_BY_IB            = 55,
	PMU_EVENT_MAC_ACTIVE_32BIT             = 56,
	PMU_EVENT_MAC_STALLED_BY_INT_W         = 57,
	PMU_EVENT_MAC_STALLED_BY_INT_ACC       = 58,
	PMU_EVENT_AO_ACTIVE                    = 64,
	PMU_EVENT_AO_ACTIVE_8BIT               = 65,
	PMU_EVENT_AO_ACTIVE_16BIT              = 66,
	PMU_EVENT_AO_STALLED_BY_OFMP_OB        = 67,
	PMU_EVENT_AO_STALLED_BY_OFMP           = 68,
	PMU_EVENT_AO_STALLED_BY_OB             = 69,
	PMU_EVENT_AO_STALLED_BY_ACC_IB         = 70,
	PMU_EVENT_AO_STALLED_BY_ACC            = 71,
	PMU_EVENT_AO_STALLED_BY_IB             = 72,
	PMU_EVENT_WD_ACTIVE                    = 80,
	PMU_EVENT_WD_STALLED                   = 81,
	PMU_EVENT_WD_STALLED_BY_WS             = 82,
	PMU_EVENT_WD_STALLED_BY_WD_BUF         = 83,
	PMU_EVENT_WD_PARSE_ACTIVE              = 84,
	PMU_EVENT_WD_PARSE_STALLED             = 85,
	PMU_EVENT_WD_PARSE_STALLED_IN          = 86,
	PMU_EVENT_WD_PARSE_STALLED_OUT         = 87,
	PMU_EVENT_WD_TRANS_WS                  = 88,
	PMU_EVENT_WD_TRANS_WB                  = 89,
	PMU_EVENT_WD_TRANS_DW0                 = 90,
	PMU_EVENT_WD_TRANS_DW1                 = 91,
	PMU_EVENT_AXI0_RD_TRANS_ACCEPTED       = 128,
	PMU_EVENT_AXI0_RD_TRANS_COMPLETED      = 129,
	PMU_EVENT_AXI0_RD_DATA_BEAT_RECEIVED   = 130,
	PMU_EVENT_AXI0_RD_TRAN_REQ_STALLED     = 131,
	PMU_EVENT_AXI0_WR_TRANS_ACCEPTED       = 132,
	PMU_EVENT_AXI0_WR_TRANS_COMPLETED_M    = 133,
	PMU_EVENT_AXI0_WR_TRANS_COMPLETED_S    = 134,
	PMU_EVENT_AXI0_WR_DATA_BEAT_WRITTEN    = 135,
	PMU_EVENT_AXI0_WR_TRAN_REQ_STALLED     = 136,
	PMU_EVENT_AXI0_WR_DATA_BEAT_STALLED    = 137,
	PMU_EVENT_AXI0_ENABLED_CYCLES          = 140,
	PMU_EVENT_AXI0_RD_STALL_LIMIT          = 142,
	PMU_EVENT_AXI0_WR_STALL_LIMIT          = 143,
	PMU_EVENT_AXI_LATENCY_ANY              = 160,
	PMU_EVENT_AXI_LATENCY_32               = 161,
	PMU_EVENT_AXI_LATENCY_64               = 162,
	PMU_EVENT_AXI_LATENCY_128              = 163,
	PMU_EVENT_AXI_LATENCY_256              = 164,
	PMU_EVENT_AXI_LATENCY_512              = 165,
	PMU_EVENT_AXI_LATENCY_1024             = 166,
	PMU_EVENT_ECC_DMA                      = 176,
	PMU_EVENT_ECC_SB0                      = 177,
	PMU_EVENT_AXI1_RD_TRANS_ACCEPTED       = 384,
	PMU_EVENT_AXI1_RD_TRANS_COMPLETED      = 385,
	PMU_EVENT_AXI1_RD_DATA_BEAT_RECEIVED   = 386,
	PMU_EVENT_AXI1_RD_TRAN_REQ_STALLED     = 387,
	PMU_EVENT_AXI1_WR_TRANS_ACCEPTED       = 388,
	PMU_EVENT_AXI1_WR_TRANS_COMPLETED_M    = 389,
	PMU_EVENT_AXI1_WR_TRANS_COMPLETED_S    = 390,
	PMU_EVENT_AXI1_WR_DATA_BEAT_WRITTEN    = 391,
	PMU_EVENT_AXI1_WR_TRAN_REQ_STALLED     = 392,
	PMU_EVENT_AXI1_WR_DATA_BEAT_STALLED    = 393,
	PMU_EVENT_AXI1_ENABLED_CYCLES          = 396,
	PMU_EVENT_AXI1_RD_STALL_LIMIT          = 398,
	PMU_EVENT_AXI1_WR_STALL_LIMIT          = 399,
	PMU_EVENT_ECC_SB1                      = 433,
};

#define PMU_EVENT_MAX_IDX 74
static uint32_t pmu_event_lookup[PMU_EVENT_MAX_IDX] = {
	PMU_EVENT_NO_EVENT,
	PMU_EVENT_CYCLE,
	PMU_EVENT_NPU_IDLE,
	PMU_EVENT_CC_STALLED_ON_BLOCKDEP,
	PMU_EVENT_CC_STALLED_ON_SHRAM_RECONFIG,
	PMU_EVENT_NPU_ACTIVE,
	PMU_EVENT_MAC_ACTIVE,
	PMU_EVENT_MAC_ACTIVE_8BIT,
	PMU_EVENT_MAC_ACTIVE_16BIT,
	PMU_EVENT_MAC_DPU_ACTIVE,
	PMU_EVENT_MAC_STALLED_BY_WD_ACC,
	PMU_EVENT_MAC_STALLED_BY_WD,
	PMU_EVENT_MAC_STALLED_BY_ACC,
	PMU_EVENT_MAC_STALLED_BY_IB,
	PMU_EVENT_MAC_ACTIVE_32BIT,
	PMU_EVENT_MAC_STALLED_BY_INT_W,
	PMU_EVENT_MAC_STALLED_BY_INT_ACC,
	PMU_EVENT_AO_ACTIVE,
	PMU_EVENT_AO_ACTIVE_8BIT,
	PMU_EVENT_AO_ACTIVE_16BIT,
	PMU_EVENT_AO_STALLED_BY_OFMP_OB,
	PMU_EVENT_AO_STALLED_BY_OFMP,
	PMU_EVENT_AO_STALLED_BY_OB,
	PMU_EVENT_AO_STALLED_BY_ACC_IB,
	PMU_EVENT_AO_STALLED_BY_ACC,
	PMU_EVENT_AO_STALLED_BY_IB,
	PMU_EVENT_WD_ACTIVE,
	PMU_EVENT_WD_STALLED,
	PMU_EVENT_WD_STALLED_BY_WS,
	PMU_EVENT_WD_STALLED_BY_WD_BUF,
	PMU_EVENT_WD_PARSE_ACTIVE,
	PMU_EVENT_WD_PARSE_STALLED,
	PMU_EVENT_WD_PARSE_STALLED_IN,
	PMU_EVENT_WD_PARSE_STALLED_OUT,
	PMU_EVENT_WD_TRANS_WS,
	PMU_EVENT_WD_TRANS_WB,
	PMU_EVENT_WD_TRANS_DW0,
	PMU_EVENT_WD_TRANS_DW1,
	PMU_EVENT_AXI0_RD_TRANS_ACCEPTED,
	PMU_EVENT_AXI0_RD_TRANS_COMPLETED,
	PMU_EVENT_AXI0_RD_DATA_BEAT_RECEIVED,
	PMU_EVENT_AXI0_RD_TRAN_REQ_STALLED,
	PMU_EVENT_AXI0_WR_TRANS_ACCEPTED,
	PMU_EVENT_AXI0_WR_TRANS_COMPLETED_M,
	PMU_EVENT_AXI0_WR_TRANS_COMPLETED_S,
	PMU_EVENT_AXI0_WR_DATA_BEAT_WRITTEN,
	PMU_EVENT_AXI0_WR_TRAN_REQ_STALLED,
	PMU_EVENT_AXI0_WR_DATA_BEAT_STALLED,
	PMU_EVENT_AXI0_ENABLED_CYCLES,
	PMU_EVENT_AXI0_RD_STALL_LIMIT,
	PMU_EVENT_AXI0_WR_STALL_LIMIT,
	PMU_EVENT_AXI_LATENCY_ANY,
	PMU_EVENT_AXI_LATENCY_32,
	PMU_EVENT_AXI_LATENCY_64,
	PMU_EVENT_AXI_LATENCY_128,
	PMU_EVENT_AXI_LATENCY_256,
	PMU_EVENT_AXI_LATENCY_512,
	PMU_EVENT_AXI_LATENCY_1024,
	PMU_EVENT_ECC_DMA,
	PMU_EVENT_ECC_SB0,
	PMU_EVENT_AXI1_RD_TRANS_ACCEPTED,
	PMU_EVENT_AXI1_RD_TRANS_COMPLETED,
	PMU_EVENT_AXI1_RD_DATA_BEAT_RECEIVED,
	PMU_EVENT_AXI1_RD_TRAN_REQ_STALLED,
	PMU_EVENT_AXI1_WR_TRANS_ACCEPTED,
	PMU_EVENT_AXI1_WR_TRANS_COMPLETED_M,
	PMU_EVENT_AXI1_WR_TRANS_COMPLETED_S,
	PMU_EVENT_AXI1_WR_DATA_BEAT_WRITTEN,
	PMU_EVENT_AXI1_WR_TRAN_REQ_STALLED,
	PMU_EVENT_AXI1_WR_DATA_BEAT_STALLED,
	PMU_EVENT_AXI1_ENABLED_CYCLES,
	PMU_EVENT_AXI1_RD_STALL_LIMIT,
	PMU_EVENT_AXI1_WR_STALL_LIMIT,
	PMU_EVENT_ECC_SB1,
};

enum pooling_mode {
	POOLING_MODE_MAX        = 0,
	POOLING_MODE_AVERAGE    = 1,
	POOLING_MODE_REDUCE_SUM = 2,
};

enum round_mode {
	ROUND_MODE_DBL      = 0,
	ROUND_MODE_TRUNCATE = 1,
	ROUND_MODE_NATURAL  = 2,
};

enum state {
	STATE_STOPPED = 0,
	STATE_RUNNING = 1,
};

enum wd_core_slice_state {
	WD_CORE_SLICE_STATE_HEADER  = 0,
	WD_CORE_SLICE_STATE_PALETTE = 1,
	WD_CORE_SLICE_STATE_WEIGHTS = 2,
};

enum wd_ctrl_state {
	WD_CTRL_STATE_IDLE     = 0,
	WD_CTRL_STATE_DRAIN    = 1,
	WD_CTRL_STATE_OFD_INIT = 2,
	WD_CTRL_STATE_OFD_RUN  = 3,
};

enum weight_order {
	WEIGHT_ORDER_DEPTH_FIRST       = 0,
	WEIGHT_ORDER_PART_KERNEL_FIRST = 1,
};

/* Register type structs */

/* config_r - RTL configuration */
struct config_r {
	union {
		struct {
			uint32_t macs_per_cc : 4;        /* The log2(macs/clock
			                                  * cycle) */
			uint32_t cmd_stream_version : 4; /* command stream
			                                  * version accepted by
			                                  * this NPU */
			uint32_t shram_size : 8;         /* Total size in KB of
			                                  * internal SHRAM */
			uint32_t reserved0 : 10;
			uint32_t functional_safety : 1;  /* Functional safety
			                                  * configuration */
			uint32_t custom_dma : 1;         /* Custom DMA
			                                  * configuration */
			uint32_t product : 4;            /* Product
			                                  * configuration */
		};
		uint32_t word;
	};
};

/* lock_r - Lock register. This register is designed for driver use and does not
 * affect NPU functionality */
struct lock_r {
	union {
		struct {
			uint32_t LOCK : 32; /* 32 bit value for LOCK
			                     * configuration */
		};
		uint32_t word;
	};
};

/* axi_limit0_r - AXI limits for port 0 counter 0 */
struct axi_limit0_r {
	union {
		struct {
			uint32_t max_beats : 2; /* Burst split alignment */
			uint32_t reserved0 : 2;
			uint32_t memtype : 4;   /* Memtype to be used to encode
			                         * AxCACHE signals */
			uint32_t reserved1 : 8;
			uint32_t
				 max_outstanding_read_m1 : 6;  /* Maximum number
			                                        * of outstanding
			                                        * AXI read
			                                        * transactions -
			                                        * 1 in range 0
			                                        * to 63 */
			uint32_t reserved2 : 2;
			uint32_t max_outstanding_write_m1 : 5; /* Maximum number
			                                        * of outstanding
			                                        * AXI write
			                                        * transactions -
			                                        * 1 in range */
			                                       /* 0 to 31 */
			uint32_t reserved3 : 3;
		};
		uint32_t word;
	};
};

/* axi_limit1_r - AXI limits for port 0 counter 1 */
struct axi_limit1_r {
	union {
		struct {
			uint32_t max_beats : 2; /* Burst split alignment */
			uint32_t reserved0 : 2;
			uint32_t memtype : 4;   /* Memtype to be used to encode
			                         * AxCACHE signals */
			uint32_t reserved1 : 8;
			uint32_t
				 max_outstanding_read_m1 : 6;  /* Maximum number
			                                        * of outstanding
			                                        * AXI read
			                                        * transactions -
			                                        * 1 in range 0
			                                        * to 63 */
			uint32_t reserved2 : 2;
			uint32_t max_outstanding_write_m1 : 5; /* Maximum number
			                                        * of outstanding
			                                        * AXI write
			                                        * transactions -
			                                        * 1 in range */
			                                       /* 0 to 31 */
			uint32_t reserved3 : 3;
		};
		uint32_t word;
	};
};

/* axi_limit2_r - AXI limits for port 1 counter 2 */
struct axi_limit2_r {
	union {
		struct {
			uint32_t max_beats : 2; /* Burst split alignment */
			uint32_t reserved0 : 2;
			uint32_t memtype : 4;   /* Memtype to be used to encode
			                         * AxCACHE signals */
			uint32_t reserved1 : 8;
			uint32_t
				 max_outstanding_read_m1 : 6;  /* Maximum number
			                                        * of outstanding
			                                        * AXI read
			                                        * transactions -
			                                        * 1 in range 0
			                                        * to 63 */
			uint32_t reserved2 : 2;
			uint32_t max_outstanding_write_m1 : 5; /* Maximum number
			                                        * of outstanding
			                                        * AXI write
			                                        * transactions -
			                                        * 1 in range */
			                                       /* 0 to 31 */
			uint32_t reserved3 : 3;
		};
		uint32_t word;
	};
};

/* axi_limit3_r - AXI limits for port 1 counter 3 */
struct axi_limit3_r {
	union {
		struct {
			uint32_t max_beats : 2; /* Burst split alignment */
			uint32_t reserved0 : 2;
			uint32_t memtype : 4;   /* Memtype to be used to encode
			                         * AxCACHE signals */
			uint32_t reserved1 : 8;
			uint32_t
				 max_outstanding_read_m1 : 6;  /* Maximum number
			                                        * of outstanding
			                                        * AXI read
			                                        * transactions -
			                                        * 1 in range 0
			                                        * to 63 */
			uint32_t reserved2 : 2;
			uint32_t max_outstanding_write_m1 : 5; /* Maximum number
			                                        * of outstanding
			                                        * AXI write
			                                        * transactions -
			                                        * 1 in range */
			                                       /* 0 to 31 */
			uint32_t reserved3 : 3;
		};
		uint32_t word;
	};
};

/* wd_status_r - WD_STATUS */
struct wd_status_r {
	union {
		struct {
			uint32_t core_slice_state : 2; /* WD core slice parser
			                                * state */
			uint32_t core_idle : 1;        /* Core idle */
			uint32_t ctrl_state : 2;       /* WD control state */
			uint32_t ctrl_idle : 1;        /* All stripe jobs idle
			                                * (all weights consumed)
			                                **/
			uint32_t write_buf_index0 : 3; /* current write index
			                                * for next data from
			                                * core */
			uint32_t write_buf_valid0 : 1; /* write buf valid (full)
			                                * */
			uint32_t write_buf_idle0 : 1;  /* write buf idle (empty)
			                                * */
			uint32_t write_buf_index1 : 3; /* current write index
			                                * for next data from
			                                * core */
			uint32_t write_buf_valid1 : 1; /* write buf valid (full)
			                                * */
			uint32_t write_buf_idle1 : 1;  /* write buf idle (empty)
			                                * */
			uint32_t events : 12;          /* WD events mapped as
			                                * appendix A */
			uint32_t reserved0 : 4;
		};
		uint32_t word;
	};
};

/* mac_status_r - MAC_STATUS */
struct mac_status_r {
	union {
		struct {
			uint32_t block_cfg_valid : 1;     /* MAC has a valid
			                                   * block configuration
			                                   **/
			uint32_t trav_en : 1;             /* MAC is doing block
			                                   * traversal */
			uint32_t wait_for_ib : 1;         /* MAC is waiting for
			                                   * an Input Buffer to
			                                   * become available */
			uint32_t wait_for_acc_buf : 1;    /* MAC is waiting for
			                                   * an Accumulator
			                                   * Buffer to become
			                                   * available */
			uint32_t wait_for_weights : 1;    /* MAC is waiting for
			                                   * a Weight Block to
			                                   * become available */
			uint32_t stall_stripe : 1;        /* MAC is stalling
			                                   * between two stripes
			                                   **/
			uint32_t dw_sel : 1;              /* Currently used
			                                   * weight interface in
			                                   * MAC AI */
			uint32_t wait_for_dw0_ready : 1;  /* MAC AI is waiting
			                                   * for MAC DPU to send
			                                   * dw0_ready to WD */
			uint32_t wait_for_dw1_ready : 1;  /* MAC AI is waiting
			                                   * for MAC DPU to send
			                                   * dw1_ready to WD */
			uint32_t acc_buf_sel_ai : 1;      /* Currently used
			                                   * AccBuf interface in
			                                   * MAC AI */
			uint32_t wait_for_acc0_ready : 1; /* MAC AI is waiting
			                                   * for acc0_ready from
			                                   * AO */
			uint32_t wait_for_acc1_ready : 1; /* MAC AI is waiting
			                                   * for acc1_ready from
			                                   * AO */
			uint32_t acc_buf_sel_aa : 1;      /* Currently used
			                                   * AccBuf interface in
			                                   * MAC ADDER_ARRAY */
			uint32_t acc0_valid : 1;          /* MAC outgoing value
			                                   * of acc0_valid */
			uint32_t acc1_valid : 1;          /* MAC outgoing value
			                                   * of acc1_valid */
			uint32_t reserved0 : 1;
			uint32_t events : 11;             /* Mapped to MAC
			                                   * events described in
			                                   * Appendix A */
			uint32_t reserved1 : 5;
		};
		uint32_t word;
	};
};

/* ao_status_r - AO_STATUS */
struct ao_status_r {
	union {
		struct {
			uint32_t cmd_sbw_valid : 1; /* Block command to shared
			                             * buffer write module is
			                             * valid */
			uint32_t cmd_act_valid : 1; /* Block command to
			                             * activation function
			                             * module is valid */
			uint32_t cmd_ctl_valid : 1; /* Block command to control
			                             * module is valid */
			uint32_t cmd_scl_valid : 1; /* Block command to scale
			                             * module is valid */
			uint32_t cmd_sbr_valid : 1; /* Block command to shared
			                             * buffer read module is
			                             * valid */
			uint32_t cmd_ofm_valid : 1; /* Block command to ofm
			                             * parameter module is valid
			                             **/
			uint32_t blk_cmd_ready : 1; /* Ready to accept block
			                             * command */
			uint32_t blk_cmd_valid : 1; /* Block command from CC is
			                             * valid */
			uint32_t reserved0 : 8;
			uint32_t events : 8;        /* Mapped to AO events
			                             * described in Appendix A
			                             **/
			uint32_t reserved1 : 8;
		};
		uint32_t word;
	};
};

/* dma_status0_r - DMA_STATUS0 */
struct dma_status0_r {
	union {
		struct {
			uint32_t cmd_idle : 1;              /* When this bit is
			                                     * high means that
			                                     * the CMD block is
			                                     * not busy in
			                                     * generating
			                                     * addresses */
			                                    /* for a CMD job */
			uint32_t ifm_idle : 1;              /* When this bit is
			                                     * high means that
			                                     * there are no
			                                     * ongoing IFM jobs
			                                     **/
			uint32_t wgt_idle_c0 : 1;           /* When this bit is
			                                     * high means that
			                                     * the WGT block is
			                                     * not busy in
			                                     * generating */
			                                    /* addresses for a
			                                     * WGT job */
			uint32_t bas_idle_c0 : 1;           /* When this bit is
			                                     * high means that
			                                     * the BAS block is
			                                     * not busy in
			                                     * generating */
			                                    /* addresses for a
			                                     * BAS job */
			uint32_t m2m_idle : 1;              /* When this bit is
			                                     * high means that
			                                     * there are no
			                                     * ongoing M2M jobs
			                                     **/
			uint32_t ofm_idle : 1;              /* When this bit is
			                                     * high means that
			                                     * there are no
			                                     * ongoing OFM jobs
			                                     **/
			uint32_t halt_req : 1;              /* CPM has requested
			                                     * to HALT AXI bus
			                                     * before soft reset
			                                     **/
			uint32_t halt_ack : 1;              /* DMA is in
			                                     * condition to halt
			                                     * the AXI bus since
			                                     * there are no
			                                     * pending
			                                     * transactions */
			uint32_t pause_req : 1;             /* CC has requested
			                                     * to pause the AXI
			                                     **/
			uint32_t pause_ack : 1;             /* DMA is in
			                                     * condition to
			                                     * pause the AXI bus
			                                     * since there are
			                                     * no pending
			                                     * transactions */
			uint32_t ib0_ai_valid_c0 : 1;       /* Data for AI to be
			                                     * read in IFM input
			                                     * buffer 0 - Core 0
			                                     **/
			uint32_t ib0_ai_ready_c0 : 1;       /* Data consumed
			                                     * from AI in IFM
			                                     * input buffer 0 -
			                                     * Core 0 */
			uint32_t ib1_ai_valid_c0 : 1;       /* Data for AI to be
			                                     * read in IFM input
			                                     * buffer 1 - Core 0
			                                     **/
			uint32_t ib1_ai_ready_c0 : 1;       /* Data consumed
			                                     * from AI in IFM
			                                     * input buffer 1 -
			                                     * Core 0 */
			uint32_t ib0_ao_valid_c0 : 1;       /* Data for AO to be
			                                     * read in IFM input
			                                     * buffer 0 - Core 0
			                                     **/
			uint32_t ib0_ao_ready_c0 : 1;       /* Data consumed
			                                     * from AO in IFM
			                                     * input buffer 0 -
			                                     * Core 0 */
			uint32_t ib1_ao_valid_c0 : 1;       /* Data for AO to be
			                                     * read in IFM input
			                                     * buffer 0 - Core 0
			                                     **/
			uint32_t ib1_ao_ready_c0 : 1;       /* Data consumed
			                                     * from AO in IFM
			                                     * input buffer 1 -
			                                     * Core 0 */
			uint32_t ob0_valid_c0 : 1;          /* Data for DMA
			                                     * ready to be
			                                     * consumed in OFM
			                                     * output buffer 0 -
			                                     *  Core 0 */
			uint32_t ob0_ready_c0 : 1;          /* Data consumed
			                                     * from DMA in OFM
			                                     * output buffer 0 -
			                                     * Core 0 */
			uint32_t ob1_valid_c0 : 1;          /* Data for DMA
			                                     * ready to be
			                                     * consumed in OFM
			                                     * output buffer 1 -
			                                     *  Core 0 */
			uint32_t ob1_ready_c0 : 1;          /* Data consumed
			                                     * from DMA in OFM
			                                     * output buffer 1 -
			                                     * Core 0 */
			uint32_t cmd_valid : 1;             /* New command word
			                                     * for CC to be
			                                     * consumed */
			uint32_t cmd_ready : 1;             /* command word
			                                     * consumed by CC */
			uint32_t wd_bitstream_valid_c0 : 1; /* New weight word
			                                     * for WD to be
			                                     * consumed - Core 0
			                                     **/
			uint32_t wd_bitstream_ready_c0 : 1; /* Weight word
			                                     * consumed by WD -
			                                     * Core 0 */
			uint32_t bs_bitstream_valid_c0 : 1; /* New BaS word for
			                                     * AO to be consumed
			                                     * - Core 0 */
			uint32_t bs_bitstream_ready_c0 : 1; /* BaS word consumed
			                                     * by AO - Core 0 */
			uint32_t axi0_ar_stalled : 1;       /* Read transfer
			                                     * request stalled
			                                     * on arready low
			                                     * AXI0 (due to
			                                     * memory system) */
			uint32_t axi0_rd_limit_stall : 1;   /* Read stalled due
			                                     * to one AXI0 limit
			                                     * counter being
			                                     * reached */
			uint32_t axi0_aw_stalled : 1;       /* Write transfer
			                                     * request stalled
			                                     * on awready low
			                                     * AXI0 (due to
			                                     * memory system) */
			uint32_t axi0_w_stalled : 1;        /* Write transfer
			                                     * stalled on
			                                     * awready low AXI0
			                                     * (due to memory
			                                     * system) */
		};
		uint32_t word;
	};
};

/* dma_status1_r - DMA_STATUS1 */
struct dma_status1_r {
	union {
		struct {
			uint32_t axi0_wr_limit_stall : 1;   /* Write stalled due
			                                     * to one AXI0 limit
			                                     * counter being
			                                     * reached */
			uint32_t axi1_ar_stalled : 1;       /* Read transfer
			                                     * request stalled
			                                     * on arready low
			                                     * AXI1 (due to
			                                     * memory system) */
			uint32_t axi1_rd_limit_stall : 1;   /* Read stalled due
			                                     * to one AXI1 limit
			                                     * counter being
			                                     * reached */
			uint32_t axi1_wr_stalled : 1;       /* Write transfer
			                                     * request stalled
			                                     * on awready low
			                                     * AXI1 (due to
			                                     * memory system) */
			uint32_t axi1_w_stalled : 1;        /* Write transfer
			                                     * stalled on wready
			                                     * low AXI1 (due to
			                                     * memory system) */
			uint32_t axi1_wr_limit_stall : 1;   /* Write stalled due
			                                     * to one AXI1 limit
			                                     * counter being
			                                     * reached */
			uint32_t wgt_idle_c1 : 1;           /* When this bit is
			                                     * high means that
			                                     * the WGT block is
			                                     * not busy in
			                                     * generating */
			                                    /* addresses for a
			                                     * WGT job */
			uint32_t bas_idle_c1 : 1;           /* When this bit is
			                                     * high means that
			                                     * the BAS block is
			                                     * not busy in
			                                     * generating */
			                                    /* addresses for a
			                                     * BAS job */
			uint32_t ib0_ai_valid_c1 : 1;       /* Data for AI to be
			                                     * read in IFM input
			                                     * buffer 0 - Core 1
			                                     **/
			uint32_t ib0_ai_ready_c1 : 1;       /* Data consumed
			                                     * from AI in IFM
			                                     * input buffer 0 -
			                                     * Core 1 */
			uint32_t ib1_ai_valid_c1 : 1;       /* Data for AI to be
			                                     * read in IFM input
			                                     * buffer 1 - Core 1
			                                     **/
			uint32_t ib1_ai_ready_c1 : 1;       /* Data consumed
			                                     * from AI in IFM
			                                     * input buffer 1 -
			                                     * Core 1 */
			uint32_t ib0_ao_valid_c1 : 1;       /* Data for AO to be
			                                     * read in IFM input
			                                     * buffer 0 - Core 1
			                                     **/
			uint32_t ib0_ao_ready_c1 : 1;       /* Data consumed
			                                     * from AO in IFM
			                                     * input buffer 0 -
			                                     * Core 1 */
			uint32_t ib1_ao_valid_c1 : 1;       /* Data for AO to be
			                                     * read in IFM input
			                                     * buffer 0 - Core 1
			                                     **/
			uint32_t ib1_ao_ready_c1 : 1;       /* Data consumed
			                                     * from AO in IFM
			                                     * input buffer 1 -
			                                     * Core 1 */
			uint32_t ob0_valid_c1 : 1;          /* Data for DMA
			                                     * ready to be
			                                     * consumed in OFM
			                                     * output buffer 0 -
			                                     * Core 1 */
			uint32_t ob0_ready_c1 : 1;          /* Data consumed
			                                     * from DMA in OFM
			                                     * output buffer 0 -
			                                     * Core 1 */
			uint32_t ob1_valid_c1 : 1;          /* Data for DMA
			                                     * ready to be
			                                     * consumed in OFM
			                                     * output buffer 1 -
			                                     * Core 1 */
			uint32_t ob1_ready_c1 : 1;          /* Data consumed
			                                     * from DMA in OFM
			                                     * output buffer 1 -
			                                     * Core 1 */
			uint32_t wd_bitstream_valid_c1 : 1; /* New weight word
			                                     * for WD to be
			                                     * consumed - Core 1
			                                     **/
			uint32_t wd_bitstream_ready_c1 : 1; /* Weight word
			                                     * consumed by WD -
			                                     * Core 1 */
			uint32_t bs_bitstream_valid_c1 : 1; /* New BaS word for
			                                     * AO to be consumed
			                                     * - Core 1 */
			uint32_t bs_bitstream_ready_c1 : 1; /* BaS word consumed
			                                     * by AO - Core 1 */
			uint32_t reserved0 : 8;
		};
		uint32_t word;
	};
};

/* clkforce_r - Force clocks on for clock gating */
struct clkforce_r {
	union {
		struct {
			uint32_t top_level_clk : 1; /* set to 1 to force on TOP
			                             * level clock */
			uint32_t cc_clk : 1;        /* set to 1 to force on CC
			                             * clock */
			uint32_t dma_clk : 1;       /* set to 1 to force on DMA
			                             * clock */
			uint32_t mac_clk : 1;       /* set to 1 to force on MAC
			                             * clock */
			uint32_t ao_clk : 1;        /* set to 1 to force on AO
			                             * clock */
			uint32_t wd_clk : 1;        /* set to 1 to force on WD
			                             * clock */
			uint32_t reserved0 : 26;
		};
		uint32_t word;
	};
};

/* debug_address_r - Set debug address for register reads 0x400-0x7FF. The
 * address must be 1KB aligned */
struct debug_address_r {
	union {
		struct {
			uint32_t addr : 32; /* Register address */
		};
		uint32_t word;
	};
};

/* debug_misc_r - 32-bit read/write register for driver debug use. This does not
 * affect NPU function */
struct debug_misc_r {
	union {
		struct {
			uint32_t misc : 32; /* Debug misc */
		};
		uint32_t word;
	};
};

/* debugcore_r - Select core number for debug registers (0x200-0x2FF) and RAM
 * reads (0x400-0x7FF). Value is 0 or 1 */
struct debugcore_r {
	union {
		struct {
			uint32_t core : 32; /* Debug core */
		};
		uint32_t word;
	};
};

/* debug_block_r - Set from which of four block banks the TSU registers are
 * read. 0 = read from the current bank 256+n = */
/* force to read from bank n where n is in the range 0 to 3 */
struct debug_block_r {
	union {
		struct {
			uint32_t block : 32; /* Debug block */
		};
		uint32_t word;
	};
};

/* pmcr_r - PMU Register control */
struct pmcr_r {
	union {
		struct {
			uint32_t cnt_en : 1;        /* Enable counter */
			uint32_t event_cnt_rst : 1; /* Reset event counter */
			uint32_t cycle_cnt_rst : 1; /* Reset cycle counter */
			uint32_t mask_en : 1;       /* PMU can be
			                             * enabled/disabled by
			                             * command stream operation
			                             * NPU_OP_PMU_MASK */
			uint32_t reserved0 : 7;
			uint32_t num_event_cnt : 5; /* Number of event counters
			                             * */
			uint32_t reserved1 : 16;
		};
		uint32_t word;
	};
};

/* pmcntenset_r - Count enable set register */
struct pmcntenset_r {
	union {
		struct {
			uint32_t EVENT_CNT_0 : 1; /* Event counter enable bit
			                           * for PMEVCNTR0 */
			uint32_t EVENT_CNT_1 : 1; /* Event counter enable bit
			                           * for PMEVCNTR1 */
			uint32_t EVENT_CNT_2 : 1; /* Event counter enable bit
			                           * for PMEVCNTR2 */
			uint32_t EVENT_CNT_3 : 1; /* Event counter enable bit
			                           * for PMEVCNTR3 */
			uint32_t reserved0 : 27;
			uint32_t CYCLE_CNT : 1;   /* PMCCNTR enable bit */
		};
		uint32_t word;
	};
};

/* pmcntenclr_r - Count enable clear register */
struct pmcntenclr_r {
	union {
		struct {
			uint32_t EVENT_CNT_0 : 1; /* Event counter disable bit
			                           * for PMEVCNTR0 */
			uint32_t EVENT_CNT_1 : 1; /* Event counter disable bit
			                           * for PMEVCNTR1 */
			uint32_t EVENT_CNT_2 : 1; /* Event counter disable bit
			                           * for PMEVCNTR2 */
			uint32_t EVENT_CNT_3 : 1; /* Event counter disable bit
			                           * for PMEVCNTR3 */
			uint32_t reserved0 : 27;
			uint32_t CYCLE_CNT : 1;   /* PMCCNTR disable bit */
		};
		uint32_t word;
	};
};

/* pmovsset_r - Overflow flag status set register */
struct pmovsset_r {
	union {
		struct {
			uint32_t EVENT_CNT_0_OVF : 1; /* Event counter overflow
			                               * set bit for PMEVCNTR0
			                               **/
			uint32_t EVENT_CNT_1_OVF : 1; /* Event counter overflow
			                               * set bit for PMEVCNTR1
			                               **/
			uint32_t EVENT_CNT_2_OVF : 1; /* Event counter overflow
			                               * set bit for PMEVCNTR2
			                               **/
			uint32_t EVENT_CNT_3_OVF : 1; /* Event counter overflow
			                               * set bit for PMEVCNTR3
			                               **/
			uint32_t reserved0 : 27;
			uint32_t CYCLE_CNT_OVF : 1;   /* PMCCNTR overflow set
			                               * bit */
		};
		uint32_t word;
	};
};

/* pmovsclr_r - Overflow flag status clear register */
struct pmovsclr_r {
	union {
		struct {
			uint32_t EVENT_CNT_0_OVF : 1; /* Event counter overflow
			                               * clear bit for PMEVCNTR0
			                               **/
			uint32_t EVENT_CNT_1_OVF : 1; /* Event counter overflow
			                               * clear bit for PMEVCNTR1
			                               **/
			uint32_t EVENT_CNT_2_OVF : 1; /* Event counter overflow
			                               * clear bit for PMEVCNTR2
			                               **/
			uint32_t EVENT_CNT_3_OVF : 1; /* Event counter overflow
			                               * clear bit for PMEVCNTR3
			                               **/
			uint32_t reserved0 : 27;
			uint32_t CYCLE_CNT_OVF : 1;   /* PMCCNTR overflow clear
			                               * bit */
		};
		uint32_t word;
	};
};

/* pmintset_r - Interrupt enable set register */
struct pmintset_r {
	union {
		struct {
			uint32_t EVENT_CNT_0_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * enable bit for
			                               * PMEVCNTR0 */
			uint32_t EVENT_CNT_1_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * enable bit for
			                               * PMEVCNTR1 */
			uint32_t EVENT_CNT_2_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * enable bit for
			                               * PMEVCNTR2 */
			uint32_t EVENT_CNT_3_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * enable bit for
			                               * PMEVCNTR3 */
			uint32_t reserved0 : 27;
			uint32_t CYCLE_CNT_INT : 1;   /* PMCCNTR overflow
			                               * interrupt request
			                               * enable bit */
		};
		uint32_t word;
	};
};

/* pmintclr_r - Interrupt enable clear register */
struct pmintclr_r {
	union {
		struct {
			uint32_t EVENT_CNT_0_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * disable bit for
			                               * PMEVCNTR0 */
			uint32_t EVENT_CNT_1_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * disable bit for
			                               * PMEVCNTR1 */
			uint32_t EVENT_CNT_2_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * disable bit for
			                               * PMEVCNTR2 */
			uint32_t EVENT_CNT_3_INT : 1; /* Event counter overflow
			                               * interrupt request
			                               * disable bit for
			                               * PMEVCNTR3 */
			uint32_t reserved0 : 27;
			uint32_t CYCLE_CNT_INT : 1;   /* PMCCNTR overflow
			                               * interrupt request
			                               * disable bit */
		};
		uint32_t word;
	};
};

/* pmccntr_r - Performance monitor cycle count register */
struct pmccntr_r {
	union {
		struct {
			uint32_t CYCLE_CNT_LO : 32; /* Cycle count - LSB */
			uint32_t CYCLE_CNT_HI : 16; /* Cycle count - MSB */
			uint32_t reserved0 : 16;
		};
		uint32_t word[2];
	};
};

/* pmccntr_cfg_r - Set start/stop event on the cycle counter */
struct pmccntr_cfg_r {
	union {
		struct {
			uint32_t CYCLE_CNT_CFG_START : 10; /* Cycle counter
			                                    * start event */
			uint32_t reserved0 : 6;
			uint32_t CYCLE_CNT_CFG_STOP : 10;  /* Cycle counter stop
			                                    * event */
			uint32_t reserved1 : 6;
		};
		uint32_t word;
	};
};

/* pmcaxi_chan_r - Set which AXI channel to monitor for latency measurements in
 * PMU */
struct pmcaxi_chan_r {
	union {
		struct {
			uint32_t CH_SEL : 4;       /* Channel select for latency
			                            * measurements */
			uint32_t reserved0 : 4;
			uint32_t AXI_CNT_SEL : 2;  /* AXI counter to monitor for
			                            * latency measurements */
			uint32_t BW_CH_SEL_EN : 1; /* Bandwidth channel selector
			                            * */
			uint32_t reserved1 : 21;
		};
		uint32_t word;
	};
};

/* kernel_x_r - Kernel X offset of in kernel decomposition */
struct kernel_x_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* kernel_y_r - Kernel Y offset of in kernel decomposition */
struct kernel_y_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* kernel_w_m1_r - Kernel (width-1) of current block */
struct kernel_w_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* kernel_h_m1_r - Kernel (height-1) of current block */
struct kernel_h_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_cblk_width_m1_r - OFM current block (width-1) */
struct ofm_cblk_width_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_cblk_height_m1_r - OFM current block (height-1) */
struct ofm_cblk_height_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_cblk_depth_m1_r - OFM current block (depth-1) */
struct ofm_cblk_depth_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_cblk_depth_m1_r - IFM current block (depth-1) */
struct ifm_cblk_depth_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_x_r - Block X coordinate in OFM */
struct ofm_x_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_y_r - Block Y coordinate in OFM */
struct ofm_y_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_z_r - Block Z (channel) coordinate in OFM */
struct ofm_z_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_z_r - Block Z (channel) coordinate in IFM */
struct ifm_z_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* pad_top_r - Block top pad */
struct pad_top_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* pad_left_r - Block left pad */
struct pad_left_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_cblk_width_r - IFM current block derived width */
struct ifm_cblk_width_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_cblk_height_r - IFM current block derived height */
struct ifm_cblk_height_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma_ifm_src_r - DMA IFM channel source position on AXI */
struct dma_ifm_src_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* dma_ifm_dst_r - DMA IFM channel destination position in SHRAM */
struct dma_ifm_dst_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma_ofm_src_r - DMA OFM channel source position in SHRAM */
struct dma_ofm_src_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma_ofm_dst_r - DMA OFM channel destination position on AXI */
struct dma_ofm_dst_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* dma_weight_src_r - DMA weight channel source position on AXI */
struct dma_weight_src_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* dma_cmd_src_r - DMA command channel source position on AXI */
struct dma_cmd_src_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* dma_cmd_size_r - DMA command channel number of bytes buffered */
struct dma_cmd_size_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma_m2m_src_r - DMA memory to memory source position on AXI */
struct dma_m2m_src_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* dma_m2m_dst_r - DMA memory to memory destination position on AXI */
struct dma_m2m_dst_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* current_qread_r - QREAD position being issued (rather than completed) */
struct current_qread_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma_scale_src_r - DMA scale and bias channel source position on AXI */
struct dma_scale_src_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* current_block_r - 0-3. Current block bank being executed by the TSU or last
 * one executed if TSU is stopped */
struct current_block_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* current_op_r - Current NPU OP command being executed by the TSU */
struct current_op_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* current_cmd_r - Current 32-bit command being parsed by the command stream
 * parser */
struct current_cmd_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* pmevcntr_r - Performance monitor event 0 count register */
struct pmevcntr_r {
	union {
		struct {
			uint32_t count : 32; /* Count word */
		};
		uint32_t word;
	};
};

/* pmevtyper_r - Performance monitor event type register 0 */
struct pmevtyper_r {
	union {
		struct {
			uint32_t EV_TYPE : 10; /* Event Type */
			uint32_t reserved0 : 22;
		};
		uint32_t word;
	};
};

/* shared_buffer_r - Shared buffer debug access. Only valid in STOPPED state */
struct shared_buffer_r {
	union {
		struct {
			uint32_t mem_word : 32; /* Memory word */
		};
		uint32_t word;
	};
};

/* ifm_pad_top_r - None */
struct ifm_pad_top_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_pad_left_r - None */
struct ifm_pad_left_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_pad_right_r - None */
struct ifm_pad_right_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_pad_bottom_r - None */
struct ifm_pad_bottom_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_depth_m1_r - None */
struct ifm_depth_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_precision_r - None */
struct ifm_precision_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_upscale_r - None */
struct ifm_upscale_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_zero_point_r - None */
struct ifm_zero_point_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_width0_m1_r - None */
struct ifm_width0_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_height0_m1_r - None */
struct ifm_height0_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_height1_m1_r - None */
struct ifm_height1_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_ib_end_r - None */
struct ifm_ib_end_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_region_r - None */
struct ifm_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_width_m1_r - None */
struct ofm_width_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_height_m1_r - None */
struct ofm_height_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_depth_m1_r - None */
struct ofm_depth_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_precision_r - None */
struct ofm_precision_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_blk_width_m1_r - None */
struct ofm_blk_width_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_blk_height_m1_r - None */
struct ofm_blk_height_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_blk_depth_m1_r - None */
struct ofm_blk_depth_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_zero_point_r - None */
struct ofm_zero_point_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_width0_m1_r - None */
struct ofm_width0_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_height0_m1_r - None */
struct ofm_height0_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_height1_m1_r - None */
struct ofm_height1_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_region_r - None */
struct ofm_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* kernel_width_m1_r - None */
struct kernel_width_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* kernel_height_m1_r - None */
struct kernel_height_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* kernel_stride_r - None */
struct kernel_stride_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* parallel_mode_r - None */
struct parallel_mode_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* acc_format_r - None */
struct acc_format_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* activation_r - None */
struct activation_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* activation_min_r - None */
struct activation_min_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* activation_max_r - None */
struct activation_max_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* weight_region_r - None */
struct weight_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* scale_region_r - None */
struct scale_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ab_start_r - None */
struct ab_start_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* blockdep_r - None */
struct blockdep_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma0_src_region_r - None */
struct dma0_src_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma0_dst_region_r - None */
struct dma0_dst_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma0_size0_r - None */
struct dma0_size0_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma0_size1_r - None */
struct dma0_size1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_broadcast_r - None */
struct ifm2_broadcast_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_scalar_r - None */
struct ifm2_scalar_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_precision_r - None */
struct ifm2_precision_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_zero_point_r - None */
struct ifm2_zero_point_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_width0_m1_r - None */
struct ifm2_width0_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_height0_m1_r - None */
struct ifm2_height0_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_height1_m1_r - None */
struct ifm2_height1_m1_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_ib_start_r - None */
struct ifm2_ib_start_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm2_region_r - None */
struct ifm2_region_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ifm_base0_r - None */
struct ifm_base0_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm_base1_r - None */
struct ifm_base1_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm_base2_r - None */
struct ifm_base2_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm_base3_r - None */
struct ifm_base3_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm_stride_x_r - None */
struct ifm_stride_x_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm_stride_y_r - None */
struct ifm_stride_y_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm_stride_c_r - None */
struct ifm_stride_c_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_base0_r - None */
struct ofm_base0_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_base1_r - None */
struct ofm_base1_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_base2_r - None */
struct ofm_base2_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_base3_r - None */
struct ofm_base3_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_stride_x_r - None */
struct ofm_stride_x_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_stride_y_r - None */
struct ofm_stride_y_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_stride_c_r - None */
struct ofm_stride_c_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* weight_base_r - None */
struct weight_base_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* weight_length_r - None */
struct weight_length_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* scale_base_r - None */
struct scale_base_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* scale_length_r - None */
struct scale_length_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ofm_scale_r - None */
struct ofm_scale_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* ofm_scale_shift_r - None */
struct ofm_scale_shift_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* opa_scale_r - None */
struct opa_scale_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* opa_scale_shift_r - None */
struct opa_scale_shift_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* opb_scale_r - None */
struct opb_scale_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* dma0_src_r - None */
struct dma0_src_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* dma0_dst_r - None */
struct dma0_dst_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* dma0_len_r - None */
struct dma0_len_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* dma0_skip0_r - None */
struct dma0_skip0_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* dma0_skip1_r - None */
struct dma0_skip1_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_base0_r - None */
struct ifm2_base0_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_base1_r - None */
struct ifm2_base1_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_base2_r - None */
struct ifm2_base2_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_base3_r - None */
struct ifm2_base3_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_stride_x_r - None */
struct ifm2_stride_x_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_stride_y_r - None */
struct ifm2_stride_y_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* ifm2_stride_c_r - None */
struct ifm2_stride_c_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* weight1_base_r - None */
struct weight1_base_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* weight1_length_r - None */
struct weight1_length_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* scale1_base_r - None */
struct scale1_base_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* scale1_length_r - None */
struct scale1_length_r {
	union {
		struct {
			uint32_t value_LO : 32; /* 64-bit register value - LSB
			                         * */
			uint32_t value_HI : 32; /* 64-bit register value - MSB
			                         * */
		};
		uint32_t word[2];
	};
};

/* revision_r - Internal FPGA build revision: first 32-bits of the Ultan Git
 * hash used for the build */
struct revision_r {
	union {
		struct {
			uint32_t value : 32; /* 32-bit register value */
		};
		uint32_t word;
	};
};

/* pid4_r - Peripheral ID byte 4 (Arm=code 4) */
struct pid4_r {
	union {
		struct {
			uint32_t PID4 : 32; /* Byte 4 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid5_r - Peripheral ID byte 5 (reserved) */
struct pid5_r {
	union {
		struct {
			uint32_t PID5 : 32; /* Byte 5 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid6_r - Peripheral ID byte 6 (reserved) */
struct pid6_r {
	union {
		struct {
			uint32_t PID6 : 32; /* Byte 6 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid7_r - Peripheral ID byte 7 (reserved) */
struct pid7_r {
	union {
		struct {
			uint32_t PID7 : 32; /* Byte 7 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid0_r - Peripheral ID byte 0. This is bits[7:0] of the part number */
struct pid0_r {
	union {
		struct {
			uint32_t PID0 : 32; /* Byte 0 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid1_r - Peripheral ID byte 1. This is bits[11:8] of the part number in
 * bits[3:0], and bits[3:0] of the Arm ID in */
/* bits[7:4] */
struct pid1_r {
	union {
		struct {
			uint32_t PID1 : 32; /* Byte 1 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid2_r - Peripheral ID byte 2. This is bits[6:4] of the Arm ID in bits[2:0],
 * and bit 3 indicates format B */
struct pid2_r {
	union {
		struct {
			uint32_t PID2 : 32; /* Byte 2 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* pid3_r - Peripheral ID byte 3 */
struct pid3_r {
	union {
		struct {
			uint32_t PID3 : 32; /* Byte 1 of Peripheral ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* cid0_r - Component ID byte 0 */
struct cid0_r {
	union {
		struct {
			uint32_t CID0 : 32; /* Byte 0 of Component ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* cid1_r - Component ID byte 1 */
struct cid1_r {
	union {
		struct {
			uint32_t CID1 : 32; /* Byte 1 of Component ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* cid2_r - Component ID byte 2 */
struct cid2_r {
	union {
		struct {
			uint32_t CID2 : 32; /* Byte 2 of Component ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

/* cid3_r - Component ID byte 3 */
struct cid3_r {
	union {
		struct {
			uint32_t CID3 : 32; /* Byte 3 of Component ID (Lower 8
			                     * bits valid) */
		};
		uint32_t word;
	};
};

#endif /* _ETHOSU_DIRECT_INTERFACE_U65_H_ */
