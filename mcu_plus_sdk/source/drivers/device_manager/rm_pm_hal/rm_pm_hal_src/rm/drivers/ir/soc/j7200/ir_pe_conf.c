/*
 * System Firmware Source File
 *
 * Static IR IRQ maps for Processing Entities for J7200 device
 *
 * Data version: 220622_142531
 *
 * Copyright (C) 2019-2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <devices.h>

#include "../../include/ir_init.h"
#include "ir_pe_conf.h"

const struct ir_init ir_soc_pe_init_list[IR_SOC_PE_INIT_NUM] = {
	/*
	 * For: A72_0 notify: COMPUTE_CLUSTER0_GIC500SS/spi_64,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_64, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_64
	 */
	[0] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x00U,
		.outp	= 0x0000U,
	},
	/*
	 * For: A72_0 response: COMPUTE_CLUSTER0_GIC500SS/spi_65,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_65, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_65
	 */
	[1] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x01U,
		.outp	= 0x0001U,
	},
	/*
	 * For: A72_1 notify: COMPUTE_CLUSTER0_GIC500SS/spi_66,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_66, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_66
	 */
	[2] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x02U,
		.outp	= 0x0002U,
	},
	/*
	 * For: A72_1 response: COMPUTE_CLUSTER0_GIC500SS/spi_67,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_67, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_67
	 */
	[3] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x03U,
		.outp	= 0x0003U,
	},
	/*
	 * For: A72_2 notify: COMPUTE_CLUSTER0_GIC500SS/spi_68,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_68, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_68
	 */
	[4] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x04U,
		.outp	= 0x0004U,
	},
	/*
	 * For: A72_2 response: COMPUTE_CLUSTER0_GIC500SS/spi_69,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_69, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_69
	 */
	[5] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x05U,
		.outp	= 0x0005U,
	},
	/*
	 * For: A72_3 notify: COMPUTE_CLUSTER0_GIC500SS/spi_70,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_70, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_70
	 */
	[6] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x06U,
		.outp	= 0x0006U,
	},
	/*
	 * For: A72_3 response: COMPUTE_CLUSTER0_GIC500SS/spi_71,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_71, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_71
	 */
	[7] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x07U,
		.outp	= 0x0007U,
	},
	/*
	 * For: A72_4 notify: COMPUTE_CLUSTER0_GIC500SS/spi_72,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_72, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_72
	 */
	[8] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x08U,
		.outp	= 0x0008U,
	},
	/*
	 * For: A72_4 response: COMPUTE_CLUSTER0_GIC500SS/spi_73,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_73, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_73
	 */
	[9] =  {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x09U,
		.outp	= 0x0009U,
	},
	/*
	 * For: MAIN_0_R5_0 notify: R5FSS0_CORE0/intr_224
	 */
	[10] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0AU,
		.outp	= 0x00C0U,
	},
	/*
	 * For: MAIN_0_R5_0 response: R5FSS0_CORE0/intr_225
	 */
	[11] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0BU,
		.outp	= 0x00C1U,
	},
	/*
	 * For: MAIN_0_R5_1 notify: R5FSS0_CORE0/intr_226
	 */
	[12] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0CU,
		.outp	= 0x00C2U,
	},
	/*
	 * For: MAIN_0_R5_1 response: R5FSS0_CORE0/intr_227
	 */
	[13] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0DU,
		.outp	= 0x00C3U,
	},
	/*
	 * For: MAIN_0_R5_2 notify: R5FSS0_CORE1/intr_224
	 */
	[14] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0EU,
		.outp	= 0x00E0U,
	},
	/*
	 * For: MAIN_0_R5_2 response: R5FSS0_CORE1/intr_225
	 */
	[15] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0FU,
		.outp	= 0x00E1U,
	},
	/*
	 * For: MAIN_0_R5_3 notify: R5FSS0_CORE1/intr_226
	 */
	[16] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x10U,
		.outp	= 0x00E2U,
	},
	/*
	 * For: MAIN_0_R5_3 response: R5FSS0_CORE1/intr_227
	 */
	[17] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x11U,
		.outp	= 0x00E3U,
	},
	/*
	 * For: A72_5 notify: COMPUTE_CLUSTER0_GIC500SS/spi_734,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_734, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_734
	 */
	[18] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0xFEU,
		.outp	= 0x00BEU,
	},
	/*
	 * For: A72_5 response: COMPUTE_CLUSTER0_GIC500SS/spi_735,
	 * COMPUTE_CLUSTER0_MSMC_1MB/gic_spi_735, COMPUTE_CLUSTER0_MSMC_EN/gic_spi_735
	 */
	[19] = {
		.id	= J7200_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0xFFU,
		.outp	= 0x00BFU,
	},
	/*
	 * For: MCU_0_R5_0 notify: MCU_R5FSS0_CORE0/intr_64
	 */
	[20] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x00U,
		.outp	= 0x0000U,
	},
	/*
	 * For: MCU_0_R5_0 response: MCU_R5FSS0_CORE0/intr_65
	 */
	[21] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x01U,
		.outp	= 0x0001U,
	},
	/*
	 * For: MCU_0_R5_1 notify: MCU_R5FSS0_CORE0/intr_66
	 */
	[22] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x02U,
		.outp	= 0x0002U,
	},
	/*
	 * For: MCU_0_R5_1 response: MCU_R5FSS0_CORE0/intr_67
	 */
	[23] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x03U,
		.outp	= 0x0003U,
	},
	/*
	 * For: MCU_0_R5_2 notify: MCU_R5FSS0_CORE1/intr_64
	 */
	[24] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x04U,
		.outp	= 0x0020U,
	},
	/*
	 * For: MCU_0_R5_2 response: MCU_R5FSS0_CORE1/intr_65
	 */
	[25] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x05U,
		.outp	= 0x0021U,
	},
	/*
	 * For: MCU_0_R5_3 notify: MCU_R5FSS0_CORE1/intr_66
	 */
	[26] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x06U,
		.outp	= 0x0022U,
	},
	/*
	 * For: MCU_0_R5_3 response: MCU_R5FSS0_CORE1/intr_67
	 */
	[27] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x07U,
		.outp	= 0x0023U,
	},
	/*
	 * For: DM err_event: MCU_R5FSS0_CORE0/intr_68
	 */
	[28] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x08U,
		.outp	= 0x0004U,
	},
	/*
	 * For: DM nonsec_high_priority_rx: MCU_R5FSS0_CORE0/intr_69
	 */
	[29] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x09U,
		.outp	= 0x0005U,
	},
	/*
	 * For: DM nonsec_high_priority_rx: MCU_R5FSS0_CORE0/intr_70
	 */
	[30] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0AU,
		.outp	= 0x0006U,
	},
	/*
	 * For: DM nonsec_low_priority_rx: MCU_R5FSS0_CORE0/intr_71
	 */
	[31] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0BU,
		.outp	= 0x0007U,
	},
	/*
	 * For: DM nonsec_low_priority_rx: MCU_R5FSS0_CORE0/intr_72
	 */
	[32] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0CU,
		.outp	= 0x0008U,
	},
	/*
	 * For: DM nonsec_notify_resp_rx: MCU_R5FSS0_CORE0/intr_73
	 */
	[33] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0DU,
		.outp	= 0x0009U,
	},
	/*
	 * For: DM nonsec_notify_resp_rx: MCU_R5FSS0_CORE0/intr_74
	 */
	[34] = {
		.id	= J7200_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0EU,
		.outp	= 0x000AU,
	},
};
