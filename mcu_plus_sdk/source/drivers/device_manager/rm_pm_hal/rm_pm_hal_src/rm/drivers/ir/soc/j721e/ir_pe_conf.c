/*
 * System Firmware Source File
 *
 * Static IR IRQ maps for Processing Entities for J721E device
 *
 * Data version: 210812_211103
 *
 * Copyright (C) 2018-2025, Texas Instruments Incorporated
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
	 * For: C6X_0_0 notify: C66SS0_CORE0/c66_event_in_sync_4
	 */
	[0] =  {
		.id	= J721E_DEV_C66SS0_INTROUTER0,
		.inp	= 0x61U,
		.outp	= 0x0000U,
	},
	/*
	 * For: C6X_0_0 response: C66SS0_CORE0/c66_event_in_sync_5
	 */
	[1] =  {
		.id	= J721E_DEV_C66SS0_INTROUTER0,
		.inp	= 0x62U,
		.outp	= 0x0001U,
	},
	/*
	 * For: C6X_0_1 notify: C66SS0_CORE0/c66_event_in_sync_6
	 */
	[2] =  {
		.id	= J721E_DEV_C66SS0_INTROUTER0,
		.inp	= 0x63U,
		.outp	= 0x0002U,
	},
	/*
	 * For: C6X_0_1 response: C66SS0_CORE0/c66_event_in_sync_7
	 */
	[3] =  {
		.id	= J721E_DEV_C66SS0_INTROUTER0,
		.inp	= 0x64U,
		.outp	= 0x0003U,
	},
	/*
	 * For: C6X_1_0 notify: C66SS1_CORE0/c66_event_in_sync_4
	 */
	[4] =  {
		.id	= J721E_DEV_C66SS1_INTROUTER0,
		.inp	= 0x61U,
		.outp	= 0x0000U,
	},
	/*
	 * For: C6X_1_0 response: C66SS1_CORE0/c66_event_in_sync_5
	 */
	[5] =  {
		.id	= J721E_DEV_C66SS1_INTROUTER0,
		.inp	= 0x62U,
		.outp	= 0x0001U,
	},
	/*
	 * For: C6X_1_1 notify: C66SS1_CORE0/c66_event_in_sync_6
	 */
	[6] =  {
		.id	= J721E_DEV_C66SS1_INTROUTER0,
		.inp	= 0x63U,
		.outp	= 0x0002U,
	},
	/*
	 * For: C6X_1_1 response: C66SS1_CORE0/c66_event_in_sync_7
	 */
	[7] =  {
		.id	= J721E_DEV_C66SS1_INTROUTER0,
		.inp	= 0x64U,
		.outp	= 0x0003U,
	},
	/*
	 * For: A72_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_64,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_64
	 */
	[8] =  {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x00U,
		.outp	= 0x0000U,
	},
	/*
	 * For: A72_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_65,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_65
	 */
	[9] =  {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x01U,
		.outp	= 0x0001U,
	},
	/*
	 * For: A72_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_66,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_66
	 */
	[10] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x02U,
		.outp	= 0x0002U,
	},
	/*
	 * For: A72_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_67,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_67
	 */
	[11] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x03U,
		.outp	= 0x0003U,
	},
	/*
	 * For: A72_2 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_68,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_68
	 */
	[12] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x04U,
		.outp	= 0x0004U,
	},
	/*
	 * For: A72_2 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_69,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_69
	 */
	[13] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x05U,
		.outp	= 0x0005U,
	},
	/*
	 * For: A72_3 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_70,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_70
	 */
	[14] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x06U,
		.outp	= 0x0006U,
	},
	/*
	 * For: A72_3 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_71,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_71
	 */
	[15] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x07U,
		.outp	= 0x0007U,
	},
	/*
	 * For: A72_4 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_72,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_72
	 */
	[16] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x08U,
		.outp	= 0x0008U,
	},
	/*
	 * For: A72_4 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_73,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_73
	 */
	[17] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x09U,
		.outp	= 0x0009U,
	},
	/*
	 * For: C7X_0 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_734,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_734
	 */
	[18] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0AU,
		.outp	= 0x00BEU,
	},
	/*
	 * For: C7X_0 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_735,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_735
	 */
	[19] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0BU,
		.outp	= 0x00BFU,
	},
	/*
	 * For: C7X_1 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_732,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_732
	 */
	[20] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0CU,
		.outp	= 0x00BCU,
	},
	/*
	 * For: C7X_1 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_733,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_733
	 */
	[21] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0DU,
		.outp	= 0x00BDU,
	},
	/*
	 * For: C6X_0_0 notify: C66SS0_INTROUTER0/in_96
	 */
	[22] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0EU,
		.outp	= 0x0158U,
	},
	/*
	 * For: C6X_0_0 response: C66SS0_INTROUTER0/in_97
	 */
	[23] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x0FU,
		.outp	= 0x0159U,
	},
	/*
	 * For: C6X_0_1 notify: C66SS0_INTROUTER0/in_98
	 */
	[24] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x10U,
		.outp	= 0x015AU,
	},
	/*
	 * For: C6X_0_1 response: C66SS0_INTROUTER0/in_99
	 */
	[25] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x11U,
		.outp	= 0x015BU,
	},
	/*
	 * For: C6X_1_0 notify: C66SS1_INTROUTER0/in_96
	 */
	[26] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x12U,
		.outp	= 0x0178U,
	},
	/*
	 * For: C6X_1_0 response: C66SS1_INTROUTER0/in_97
	 */
	[27] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x13U,
		.outp	= 0x0179U,
	},
	/*
	 * For: C6X_1_1 notify: C66SS1_INTROUTER0/in_98
	 */
	[28] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x14U,
		.outp	= 0x017AU,
	},
	/*
	 * For: C6X_1_1 response: C66SS1_INTROUTER0/in_99
	 */
	[29] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x15U,
		.outp	= 0x017BU,
	},
	/*
	 * For: MAIN_0_R5_0 notify: R5FSS0_CORE0/intr_224
	 */
	[30] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x16U,
		.outp	= 0x00C0U,
	},
	/*
	 * For: MAIN_0_R5_0 response: R5FSS0_CORE0/intr_225
	 */
	[31] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x17U,
		.outp	= 0x00C1U,
	},
	/*
	 * For: MAIN_0_R5_1 notify: R5FSS0_CORE0/intr_226
	 */
	[32] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x18U,
		.outp	= 0x00C2U,
	},
	/*
	 * For: MAIN_0_R5_1 response: R5FSS0_CORE0/intr_227
	 */
	[33] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x19U,
		.outp	= 0x00C3U,
	},
	/*
	 * For: MAIN_0_R5_2 notify: R5FSS0_CORE1/intr_224
	 */
	[34] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x1AU,
		.outp	= 0x00E0U,
	},
	/*
	 * For: MAIN_0_R5_2 response: R5FSS0_CORE1/intr_225
	 */
	[35] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x1BU,
		.outp	= 0x00E1U,
	},
	/*
	 * For: MAIN_0_R5_3 notify: R5FSS0_CORE1/intr_226
	 */
	[36] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x1CU,
		.outp	= 0x00E2U,
	},
	/*
	 * For: MAIN_0_R5_3 response: R5FSS0_CORE1/intr_227
	 */
	[37] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x1DU,
		.outp	= 0x00E3U,
	},
	/*
	 * For: MAIN_1_R5_0 notify: R5FSS1_CORE0/intr_224
	 */
	[38] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x1EU,
		.outp	= 0x0100U,
	},
	/*
	 * For: MAIN_1_R5_0 response: R5FSS1_CORE0/intr_225
	 */
	[39] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x1FU,
		.outp	= 0x0101U,
	},
	/*
	 * For: MAIN_1_R5_1 notify: R5FSS1_CORE0/intr_226
	 */
	[40] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x20U,
		.outp	= 0x0102U,
	},
	/*
	 * For: MAIN_1_R5_1 response: R5FSS1_CORE0/intr_227
	 */
	[41] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x21U,
		.outp	= 0x0103U,
	},
	/*
	 * For: MAIN_1_R5_2 notify: R5FSS1_CORE1/intr_224
	 */
	[42] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x22U,
		.outp	= 0x0120U,
	},
	/*
	 * For: MAIN_1_R5_2 response: R5FSS1_CORE1/intr_225
	 */
	[43] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x23U,
		.outp	= 0x0121U,
	},
	/*
	 * For: MAIN_1_R5_3 notify: R5FSS1_CORE1/intr_226
	 */
	[44] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x24U,
		.outp	= 0x0122U,
	},
	/*
	 * For: MAIN_1_R5_3 response: R5FSS1_CORE1/intr_227
	 */
	[45] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0x25U,
		.outp	= 0x0123U,
	},
	/*
	 * For: A72_5 notify: COMPUTE_CLUSTER0_CLEC/soc_events_in_730,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_730
	 */
	[46] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0xFEU,
		.outp	= 0x00BAU,
	},
	/*
	 * For: A72_5 response: COMPUTE_CLUSTER0_CLEC/soc_events_in_731,
	 * COMPUTE_CLUSTER0_GIC500SS/spi_731
	 */
	[47] = {
		.id	= J721E_DEV_NAVSS0_INTR_ROUTER_0,
		.inp	= 0xFFU,
		.outp	= 0x00BBU,
	},
	/*
	 * For: MCU_0_R5_0 notify: MCU_R5FSS0_CORE0/intr_64
	 */
	[48] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x00U,
		.outp	= 0x0000U,
	},
	/*
	 * For: MCU_0_R5_0 response: MCU_R5FSS0_CORE0/intr_65
	 */
	[49] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x01U,
		.outp	= 0x0001U,
	},
	/*
	 * For: MCU_0_R5_1 notify: MCU_R5FSS0_CORE0/intr_66
	 */
	[50] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x02U,
		.outp	= 0x0002U,
	},
	/*
	 * For: MCU_0_R5_1 response: MCU_R5FSS0_CORE0/intr_67
	 */
	[51] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x03U,
		.outp	= 0x0003U,
	},
	/*
	 * For: MCU_0_R5_2 notify: MCU_R5FSS0_CORE1/intr_64
	 */
	[52] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x04U,
		.outp	= 0x0020U,
	},
	/*
	 * For: MCU_0_R5_2 response: MCU_R5FSS0_CORE1/intr_65
	 */
	[53] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x05U,
		.outp	= 0x0021U,
	},
	/*
	 * For: MCU_0_R5_3 notify: MCU_R5FSS0_CORE1/intr_66
	 */
	[54] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x06U,
		.outp	= 0x0022U,
	},
	/*
	 * For: MCU_0_R5_3 response: MCU_R5FSS0_CORE1/intr_67
	 */
	[55] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x07U,
		.outp	= 0x0023U,
	},
	/*
	 * For: DM err_event: MCU_R5FSS0_CORE0/intr_69
	 */
	[56] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x08U,
		.outp	= 0x0005U,
	},
	/*
	 * For: DM nonsec_high_priority_rx: MCU_R5FSS0_CORE0/intr_70
	 */
	[57] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x09U,
		.outp	= 0x0006U,
	},
	/*
	 * For: DM nonsec_high_priority_rx: MCU_R5FSS0_CORE0/intr_71
	 */
	[58] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0AU,
		.outp	= 0x0007U,
	},
	/*
	 * For: DM nonsec_low_priority_rx: MCU_R5FSS0_CORE0/intr_72
	 */
	[59] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0BU,
		.outp	= 0x0008U,
	},
	/*
	 * For: DM nonsec_low_priority_rx: MCU_R5FSS0_CORE0/intr_73
	 */
	[60] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0CU,
		.outp	= 0x0009U,
	},
	/*
	 * For: DM nonsec_notify_resp_rx: MCU_R5FSS0_CORE0/intr_74
	 */
	[61] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0DU,
		.outp	= 0x000AU,
	},
	/*
	 * For: DM nonsec_notify_resp_rx: MCU_R5FSS0_CORE0/intr_75
	 */
	[62] = {
		.id	= J721E_DEV_MCU_NAVSS0_INTR_0,
		.inp	= 0x0EU,
		.outp	= 0x000BU,
	},
};
