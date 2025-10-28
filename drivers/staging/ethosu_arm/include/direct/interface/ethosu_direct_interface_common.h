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

#ifndef _ETHOSU_DIRECT_INTERFACE_COMMON_H_
#define _ETHOSU_DIRECT_INTERFACE_COMMON_H_

#include <linux/types.h>

#define NPU_CMD_PWR_CLK_MASK 0xC

#define NPU_REG_ID 0x0000
#define NPU_REG_STATUS 0x0004
#define NPU_REG_CMD 0x0008
#define NPU_REG_RESET 0x000C
#define NPU_REG_QBASE 0x0010
#define NPU_REG_QBASE_HI 0x0014
#define NPU_REG_QREAD 0x0018
#define NPU_REG_QCONFIG 0x001C
#define NPU_REG_QSIZE 0x0020
#define NPU_REG_PROT 0x0024
#define NPU_REG_QCONFIG 0x001C
#define NPU_REG_REGIONCFG 0x003C

/* Register subpage BASE_POINTERS */

#define NPU_REG_BASEP_BASE 0x0080
#define NPU_REG_BASEP_ARRLEN 0x0008
#define BASE_POINTERS_REGISTERS_SIZE 0x0100

#define ETHOSU_NPU_REG_CONFIG_OFFSET 0x0028
#define ETHOSU_NPU_PROD_ID_MASK 0xF0000000
#define ETHOSU_GET_PROD_ID(config) ((config & ETHOSU_NPU_PROD_ID_MASK) >> 28)

#define ETHOSU_PRODUCT_U85 2
#define ETHOSU_PRODUCT_U65 1

enum privilege_level {
	PRIVILEGE_LEVEL_USER       = 0,
	PRIVILEGE_LEVEL_PRIVILEGED = 1,
};

enum security_level {
	SECURITY_LEVEL_SECURE     = 0,
	SECURITY_LEVEL_NON_SECURE = 1,
};

/* Register type structs */

/* id_r - ID register */
struct id_r {
	union {
		struct {
			uint32_t version_status : 4; /* This value is the
			                              * version of the product
			                              **/
			uint32_t version_minor : 4;  /* This value is the n for
			                              * the P part of an RnPn
			                              * release number */
			uint32_t version_major : 4;  /* This value is the n for
			                              * the R part of an RnPn
			                              * release number */
			uint32_t product_major : 4;  /* Product major ID number
			                              * (unique per base
			                              * product) */
			uint32_t arch_patch_rev : 4; /* This value is the patch
			                              * number of the
			                              * architecture version a.b
			                              **/
			uint32_t arch_minor_rev : 8; /* This value is the minor
			                              * architecture version
			                              * number, b in the
			                              * architecture version a.b
			                              **/
			uint32_t arch_major_rev : 4; /* This value is the major
			                              * architecture version
			                              * number, a in the
			                              * architecture version a.b
			                              **/
		};
		uint32_t word;
	};
};

/* status_r - Register describes the current operating status of the NPU */

/* Bit 7 is reserved for ARM Ethos-U85. For ARM Ethos-U65 it represents
 * wd_fault*/
/* Bit 9 is reserved for ARM Ethos-U65. For ARM Ethos-U85 it represents
 * branch_fault*/
struct status_r {
	union {
		struct {
			uint32_t state : 1;              /* 0 = NPU is in
			                                  * stopped state. 1 =
			                                  * NPU is in running
			                                  * state */
			uint32_t irq_raised : 1;         /* 0 = IRQ not raised.
			                                  * 1 = IRQ raised */
			uint32_t bus_status : 1;         /* 0 = No bus fault. 1
			                                  * = Bus abort detected
			                                  * and processing
			                                  * halted */
			uint32_t reset_status : 1;       /* 0 = No reset in
			                                  * progress. 1 = Reset
			                                  * in progress */
			uint32_t cmd_parse_error : 1;    /* 0 = No parsing
			                                  * error. 1 = Command
			                                  * stream parsing error
			                                  * detected */
			uint32_t cmd_end_reached : 1;    /* 0 = Command stream
			                                  * end is not reached.
			                                  * 1 = Command stream
			                                  * end is reached */
			uint32_t pmu_irq_raised : 1;     /* 0 = No PMU IRQ
			                                  * raised. 1 = PMU IRQ
			                                  * raised */
			uint32_t wd_fault : 1;           /* Weight decoder
			                                  * state: 0=no fault
			                                  * 1=weight decoder
			                                  * decompression fault.
			                                  * Can only be
			                                  * cleared by reset */
			uint32_t ecc_fault : 1;          /* 0 = No ECC fault
			                                  * detected. 1 = ECC
			                                  * fault detected */
			uint32_t branch_fault : 1;       /* 0 = No branch fault.
			                                  * 1 = Branch fault
			                                  * detected */
			uint32_t reserved1 : 1;
			uint32_t faulting_interface : 1; /* The faulting
			                                  * interface on bus
			                                  * abort */
			uint32_t faulting_channel : 4;   /* The faulting channel
			                                  * on a bus abort */
			uint32_t irq_history_mask : 16;  /* The IRQ History mask
			                                  * */
		};
		uint32_t word;
	};
};

/* cmd_r - The command register. This register reads as last written command */
struct cmd_r {
	union {
		struct {
			uint32_t transition_to_running_state : 1; /* Write 1 to
			                                           * transition
			                                           * the NPU to
			                                           * running
			                                           * state.
			                                           * Writing 0
			                                           * has no
			                                           * effect */
			uint32_t clear_irq : 1;                   /* Write 1 to
			                                           * clear the
			                                           * IRQ status
			                                           * in the
			                                           * STATUS
			                                           * register.
			                                           * Writing 0
			                                           * has no
			                                           * effect */
			uint32_t clock_q_enable : 1;              /* Write 1 to
			                                           * this bit to
			                                           * enable
			                                           * clock off
			                                           * using clock
			                                           * q-interface
			                                           * and enable
			                                           * the
			                                           * requester
			                                           * clock gate
			                                           **/
			uint32_t power_q_enable : 1;              /* Write 1 to
			                                           * this bit to
			                                           * enable
			                                           * power off
			                                           * using power
			                                           * q-interface
			                                           **/
			uint32_t stop_request : 1;                /* Write 1 to
			                                           * this bit to
			                                           * request
			                                           * STOP after
			                                           * completing
			                                           * any
			                                           * already-started
			                                           * commands */
			uint32_t reserved0 : 11;
			uint32_t clear_irq_history : 16;          /* Clears the
			                                           * IRQ history
			                                           * mask */
		};
		uint32_t word;
	};
};

/* reset_r - Request reset and new security mode */
struct reset_r {
	union {
		struct {
			uint32_t pending_CPL : 1; /* Current privilege level. 0
			                           * = User and 1 = Privileged
			                           **/
			uint32_t pending_CSL : 1; /* Current security level. 0 =
			                           * Secure and 1 = Non-secure
			                           **/
			uint32_t reserved0 : 30;
		};
		uint32_t word;
	};
};

/* qbase_r - The base address of the command stream in bytes */
struct qbase_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

/* qread_r - The read offset in the command stream in bytes. Multiple of four in
 * the range 0-16MB */
struct qread_r {
	union {
		struct {
			uint32_t QREAD : 32; /* The read offset of the current
			                      * command under execution */
		};
		uint32_t word;
	};
};

/* qconfig_r - The AXI configuration for the command stream in the range 0-3.
 * Same encoding as for REGIONCFG */
struct qconfig_r {
	union {
		struct {
			uint32_t cmd_region0 : 2; /* The command region
			                           * configuration number */
			uint32_t reserved0 : 30;
		};
		uint32_t word;
	};
};

/* qsize_r - The size of the command stream in bytes. Multiple of four in the
 * range 0-16MB */
struct qsize_r {
	union {
		struct {
			uint32_t QSIZE : 32; /* The size of the next command
			                      * stream to be executed by the NPU
			                      **/
		};
		uint32_t word;
	};
};

/* prot_r - The protection level configured for the NPU when acting as an AXI
 * Requester */
struct prot_r {
	union {
		struct {
			uint32_t active_CPL : 1; /* The current privilege level.
			                          * 0 = User and 1 = Privileged
			                          **/
			uint32_t active_CSL : 1; /* The current security level
			                          * 0=Secure 1=Non secure */
			uint32_t reserved0 : 30;
		};
		uint32_t word;
	};
};

/* regioncfg_r - Region memory type configuration. Bits[2*k+1:2*k] give the
 * memory type for REGION[k] */
struct regioncfg_r {
	union {
		struct {
			uint32_t region0 : 2; /* Bits for Region0 Configuration
			                       * */
			uint32_t region1 : 2; /* Bits for Region1 Configuration
			                       * */
			uint32_t region2 : 2; /* Bits for Region2 Configuration
			                       * */
			uint32_t region3 : 2; /* Bits for Region3 Configuration
			                       * */
			uint32_t region4 : 2; /* Bits for Region4 Configuration
			                       * */
			uint32_t region5 : 2; /* Bits for Region5 Configuration
			                       * */
			uint32_t region6 : 2; /* Bits for Region6 Configuration
			                       * */
			uint32_t region7 : 2; /* Bits for Region7 Configuration
			                       * */
			uint32_t reserved0 : 16;
		};
		uint32_t word;
	};
};

/* basep_r - The driver can use this address to relocate the command stream on
 * region 0. If the region contains data */
/* requiring A-byte alignment then the base must be a multiple of A */
struct basep_r {
	union {
		struct {
			uint32_t offset_LO : 32; /* Offset - LSB */
			uint32_t offset_HI : 8;  /* Offset - MSB */
			uint32_t reserved0 : 24;
		};
		uint32_t word[2];
	};
};

#endif /* _ETHOSU_DIRECT_INTERFACE_COMMON_H_ */
