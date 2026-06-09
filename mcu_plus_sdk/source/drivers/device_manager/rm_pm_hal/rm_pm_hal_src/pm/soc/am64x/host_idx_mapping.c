/*
 * Data version: 230209_212853
 *
 * Copyright (C) 2019-2026, Texas Instruments Incorporated
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
#include <soc/host_idx_mapping.h>
#include <soc/am64x/hosts.h>

const u8 soc_host_indexes[52] = {
	[HOST_ID_DMSC] = 0U,
	[HOST_ID_MAIN_0_R5_0] = 1U,
	[HOST_ID_MAIN_0_R5_1] = 2U,
	[HOST_ID_MAIN_0_R5_2] = 3U,
	[HOST_ID_MAIN_0_R5_3] = 4U,
	[HOST_ID_A53_0] = 5U,
	[HOST_ID_A53_1] = 6U,
	[HOST_ID_A53_2] = 7U,
	[HOST_ID_A53_3] = 8U,
	[HOST_ID_M4_0] = 9U,
	[HOST_ID_MAIN_1_R5_0] = 10U,
	[HOST_ID_MAIN_1_R5_1] = 11U,
	[HOST_ID_MAIN_1_R5_2] = 12U,
	[HOST_ID_MAIN_1_R5_3] = 13U,
	[HOST_ID_A53_4] = 14U,
	[HOST_ID_ICSSG_0] = 15U,
	[HOST_ID_ICSSG_1] = 16U,
	[1U] = HOST_IDX_NONE,
	[2U] = HOST_IDX_NONE,
	[3U] = HOST_IDX_NONE,
	[4U] = HOST_IDX_NONE,
	[5U] = HOST_IDX_NONE,
	[6U] = HOST_IDX_NONE,
	[7U] = HOST_IDX_NONE,
	[8U] = HOST_IDX_NONE,
	[9U] = HOST_IDX_NONE,
	[15U] = HOST_IDX_NONE,
	[16U] = HOST_IDX_NONE,
	[17U] = HOST_IDX_NONE,
	[18U] = HOST_IDX_NONE,
	[19U] = HOST_IDX_NONE,
	[20U] = HOST_IDX_NONE,
	[21U] = HOST_IDX_NONE,
	[22U] = HOST_IDX_NONE,
	[23U] = HOST_IDX_NONE,
	[24U] = HOST_IDX_NONE,
	[25U] = HOST_IDX_NONE,
	[26U] = HOST_IDX_NONE,
	[27U] = HOST_IDX_NONE,
	[28U] = HOST_IDX_NONE,
	[29U] = HOST_IDX_NONE,
	[31U] = HOST_IDX_NONE,
	[32U] = HOST_IDX_NONE,
	[33U] = HOST_IDX_NONE,
	[34U] = HOST_IDX_NONE,
	[39U] = HOST_IDX_NONE,
	[44U] = HOST_IDX_NONE,
	[45U] = HOST_IDX_NONE,
	[46U] = HOST_IDX_NONE,
	[47U] = HOST_IDX_NONE,
	[48U] = HOST_IDX_NONE,
	[49U] = HOST_IDX_NONE,
};
const u8 soc_host_indexes_sz = (u8) sizeof(soc_host_indexes);
