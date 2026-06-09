/*
 * System Firmware Source File
 *
 * Static IR IRQ maps for Processing Entities for J784S4 device
 *
 * Data version: 250223_004013
 *
 * Copyright (C) 2022-2025, Texas Instruments Incorporated
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
	 * For: A72_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_64,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_64
	 */
	[0] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x00U,
		.outp	= 0x0000U,
	},
	/*
	 * For: A72_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_65,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_65
	 */
	[1] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x01U,
		.outp	= 0x0001U,
	},
	/*
	 * For: A72_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_66,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_66
	 */
	[2] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x02U,
		.outp	= 0x0002U,
	},
	/*
	 * For: A72_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_67,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_67
	 */
	[3] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x03U,
		.outp	= 0x0003U,
	},
	/*
	 * For: A72_2 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_68,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_68
	 */
	[4] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x04U,
		.outp	= 0x0004U,
	},
	/*
	 * For: A72_2 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_69,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_69
	 */
	[5] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x05U,
		.outp	= 0x0005U,
	},
	/*
	 * For: A72_3 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_70,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_70
	 */
	[6] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x06U,
		.outp	= 0x0006U,
	},
	/*
	 * For: A72_3 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_71,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_71
	 */
	[7] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x07U,
		.outp	= 0x0007U,
	},
	/*
	 * For: A72_4 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_72,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_72
	 */
	[8] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x08U,
		.outp	= 0x0008U,
	},
	/*
	 * For: A72_4 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_73,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_73
	 */
	[9] =  {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x09U,
		.outp	= 0x0009U,
	},
	/*
	 * For: A72_5 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_74,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_74
	 */
	[10] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x0AU,
		.outp	= 0x000AU,
	},
	/*
	 * For: A72_5 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_75,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_75
	 */
	[11] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x0BU,
		.outp	= 0x000BU,
	},
	/*
	 * For: A72_6 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_76,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_76
	 */
	[12] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x0CU,
		.outp	= 0x000CU,
	},
	/*
	 * For: A72_6 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_77,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_77
	 */
	[13] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x0DU,
		.outp	= 0x000DU,
	},
	/*
	 * For: A72_7 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_78,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_78
	 */
	[14] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x0EU,
		.outp	= 0x000EU,
	},
	/*
	 * For: A72_7 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_79,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_79
	 */
	[15] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x0FU,
		.outp	= 0x000FU,
	},
	/*
	 * For: C7X_0_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_720,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_720
	 */
	[16] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x10U,
		.outp	= 0x00B0U,
	},
	/*
	 * For: C7X_0_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_721,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_721
	 */
	[17] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x11U,
		.outp	= 0x00B1U,
	},
	/*
	 * For: C7X_0_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_722,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_722
	 */
	[18] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x12U,
		.outp	= 0x00B2U,
	},
	/*
	 * For: C7X_0_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_723,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_723
	 */
	[19] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x13U,
		.outp	= 0x00B3U,
	},
	/*
	 * For: C7X_1_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_724,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_724
	 */
	[20] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x14U,
		.outp	= 0x00B4U,
	},
	/*
	 * For: C7X_1_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_725,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_725
	 */
	[21] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x15U,
		.outp	= 0x00B5U,
	},
	/*
	 * For: C7X_1_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_726,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_726
	 */
	[22] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x16U,
		.outp	= 0x00B6U,
	},
	/*
	 * For: C7X_1_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_727,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_727
	 */
	[23] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x17U,
		.outp	= 0x00B7U,
	},
	/*
	 * For: C7X_2_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_728,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_728
	 */
	[24] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x18U,
		.outp	= 0x00B8U,
	},
	/*
	 * For: C7X_2_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_729,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_729
	 */
	[25] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x19U,
		.outp	= 0x00B9U,
	},
	/*
	 * For: C7X_2_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_730,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_730
	 */
	[26] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x1AU,
		.outp	= 0x00BAU,
	},
	/*
	 * For: C7X_2_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_731,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_731
	 */
	[27] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x1BU,
		.outp	= 0x00BBU,
	},
	/*
	 * For: C7X_3_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_732,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_732
	 */
	[28] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x1CU,
		.outp	= 0x00BCU,
	},
	/*
	 * For: C7X_3_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_733,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_733
	 */
	[29] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x1DU,
		.outp	= 0x00BDU,
	},
	/*
	 * For: C7X_3_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_734,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_734
	 */
	[30] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x1EU,
		.outp	= 0x00BEU,
	},
	/*
	 * For: C7X_3_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_735,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_735
	 */
	[31] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x1FU,
		.outp	= 0x00BFU,
	},
	/*
	 * For: MAIN_0_R5_0 notify: R5FSS0_CORE0/intr_224
	 */
	[32] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x20U,
		.outp	= 0x00C0U,
	},
	/*
	 * For: MAIN_0_R5_0 response: R5FSS0_CORE0/intr_225
	 */
	[33] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x21U,
		.outp	= 0x00C1U,
	},
	/*
	 * For: MAIN_0_R5_1 notify: R5FSS0_CORE0/intr_226
	 */
	[34] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x22U,
		.outp	= 0x00C2U,
	},
	/*
	 * For: MAIN_0_R5_1 response: R5FSS0_CORE0/intr_227
	 */
	[35] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x23U,
		.outp	= 0x00C3U,
	},
	/*
	 * For: MAIN_0_R5_2 notify: R5FSS0_CORE1/intr_224
	 */
	[36] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x24U,
		.outp	= 0x00E0U,
	},
	/*
	 * For: MAIN_0_R5_2 response: R5FSS0_CORE1/intr_225
	 */
	[37] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x25U,
		.outp	= 0x00E1U,
	},
	/*
	 * For: MAIN_0_R5_3 notify: R5FSS0_CORE1/intr_226
	 */
	[38] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x26U,
		.outp	= 0x00E2U,
	},
	/*
	 * For: MAIN_0_R5_3 response: R5FSS0_CORE1/intr_227
	 */
	[39] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x27U,
		.outp	= 0x00E3U,
	},
	/*
	 * For: MAIN_1_R5_0 notify: R5FSS1_CORE0/intr_224
	 */
	[40] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x28U,
		.outp	= 0x0100U,
	},
	/*
	 * For: MAIN_1_R5_0 response: R5FSS1_CORE0/intr_225
	 */
	[41] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x29U,
		.outp	= 0x0101U,
	},
	/*
	 * For: MAIN_1_R5_1 notify: R5FSS1_CORE0/intr_226
	 */
	[42] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x2AU,
		.outp	= 0x0102U,
	},
	/*
	 * For: MAIN_1_R5_1 response: R5FSS1_CORE0/intr_227
	 */
	[43] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x2BU,
		.outp	= 0x0103U,
	},
	/*
	 * For: MAIN_1_R5_2 notify: R5FSS1_CORE1/intr_224
	 */
	[44] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x2CU,
		.outp	= 0x0120U,
	},
	/*
	 * For: MAIN_1_R5_2 response: R5FSS1_CORE1/intr_225
	 */
	[45] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x2DU,
		.outp	= 0x0121U,
	},
	/*
	 * For: MAIN_1_R5_3 notify: R5FSS1_CORE1/intr_226
	 */
	[46] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x2EU,
		.outp	= 0x0122U,
	},
	/*
	 * For: MAIN_1_R5_3 response: R5FSS1_CORE1/intr_227
	 */
	[47] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x2FU,
		.outp	= 0x0123U,
	},
	/*
	 * For: MAIN_2_R5_0 notify: R5FSS2_CORE0/intr_224
	 */
	[48] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x30U,
		.outp	= 0x0140U,
	},
	/*
	 * For: MAIN_2_R5_0 response: R5FSS2_CORE0/intr_225
	 */
	[49] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x31U,
		.outp	= 0x0141U,
	},
	/*
	 * For: MAIN_2_R5_1 notify: R5FSS2_CORE0/intr_226
	 */
	[50] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x32U,
		.outp	= 0x0142U,
	},
	/*
	 * For: MAIN_2_R5_1 response: R5FSS2_CORE0/intr_227
	 */
	[51] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x33U,
		.outp	= 0x0143U,
	},
	/*
	 * For: MAIN_2_R5_2 notify: R5FSS2_CORE1/intr_224
	 */
	[52] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x34U,
		.outp	= 0x0160U,
	},
	/*
	 * For: MAIN_2_R5_2 response: R5FSS2_CORE1/intr_225
	 */
	[53] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x35U,
		.outp	= 0x0161U,
	},
	/*
	 * For: MAIN_2_R5_3 notify: R5FSS2_CORE1/intr_226
	 */
	[54] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x36U,
		.outp	= 0x0162U,
	},
	/*
	 * For: MAIN_2_R5_3 response: R5FSS2_CORE1/intr_227
	 */
	[55] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0x37U,
		.outp	= 0x0163U,
	},
	/*
	 * For: A72_8 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_718,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_718
	 */
	[56] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0xFEU,
		.outp	= 0x00AEU,
	},
	/*
	 * For: A72_8 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_719,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_719
	 */
	[57] = {
		.id	= J784S4_DEV_NAVSS0_INTR_0,
		.inp	= 0xFFU,
		.outp	= 0x00AFU,
	},
	/*
	 * For: MCU_0_R5_0 notify: MCU_R5FSS0_CORE0/intr_64
	 */
	[58] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x00U,
		.outp	= 0x0000U,
	},
	/*
	 * For: MCU_0_R5_0 response: MCU_R5FSS0_CORE0/intr_65
	 */
	[59] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x01U,
		.outp	= 0x0001U,
	},
	/*
	 * For: MCU_0_R5_1 notify: MCU_R5FSS0_CORE0/intr_66
	 */
	[60] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x02U,
		.outp	= 0x0002U,
	},
	/*
	 * For: MCU_0_R5_1 response: MCU_R5FSS0_CORE0/intr_67
	 */
	[61] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x03U,
		.outp	= 0x0003U,
	},
	/*
	 * For: MCU_0_R5_2 notify: MCU_R5FSS0_CORE1/intr_64
	 */
	[62] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x04U,
		.outp	= 0x0020U,
	},
	/*
	 * For: MCU_0_R5_2 response: MCU_R5FSS0_CORE1/intr_65
	 */
	[63] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x05U,
		.outp	= 0x0021U,
	},
	/*
	 * For: MCU_0_R5_3 notify: MCU_R5FSS0_CORE1/intr_66
	 */
	[64] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x06U,
		.outp	= 0x0022U,
	},
	/*
	 * For: MCU_0_R5_3 response: MCU_R5FSS0_CORE1/intr_67
	 */
	[65] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x07U,
		.outp	= 0x0023U,
	},
	/*
	 * For: DM err_event: MCU_R5FSS0_CORE0/intr_69
	 */
	[66] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x10U,
		.outp	= 0x0005U,
	},
	/*
	 * For: DM nonsec_high_priority_rx: MCU_R5FSS0_CORE0/intr_70
	 */
	[67] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x11U,
		.outp	= 0x0006U,
	},
	/*
	 * For: DM nonsec_high_priority_rx: MCU_R5FSS0_CORE0/intr_71
	 */
	[68] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x12U,
		.outp	= 0x0007U,
	},
	/*
	 * For: DM nonsec_low_priority_rx: MCU_R5FSS0_CORE0/intr_72
	 */
	[69] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x13U,
		.outp	= 0x0008U,
	},
	/*
	 * For: DM nonsec_low_priority_rx: MCU_R5FSS0_CORE0/intr_73
	 */
	[70] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x14U,
		.outp	= 0x0009U,
	},
	/*
	 * For: DM nonsec_notify_resp_rx: MCU_R5FSS0_CORE0/intr_74
	 */
	[71] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x15U,
		.outp	= 0x000AU,
	},
	/*
	 * For: DM nonsec_notify_resp_rx: MCU_R5FSS0_CORE0/intr_75
	 */
	[72] = {
		.id	= J784S4_DEV_MCU_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x16U,
		.outp	= 0x000BU,
	},
};
