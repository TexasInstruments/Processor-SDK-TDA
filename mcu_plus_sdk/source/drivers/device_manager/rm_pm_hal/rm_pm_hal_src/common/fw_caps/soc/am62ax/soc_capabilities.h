/*
 * Data version: 250904_120008
 *
 * Copyright (C) 2021-2026, Texas Instruments Incorporated
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
#ifndef SOC_AM62AX_CAPABILITIES_H
#define SOC_AM62AX_CAPABILITIES_H

#include <config.h>
#include <types/short_types.h>

#define TISCI_MSG_FLAG_FW_CAP_MAX (0x0BU)
const u64 soc_fw_caps[TISCI_MSG_FLAG_FW_CAP_MAX] = {
	TISCI_MSG_FLAG_FW_CAP_GENERIC_CAP,
	TISCI_MSG_FLAG_FW_CAP_LPM_DEEP_SLEEP,
	TISCI_MSG_FLAG_FW_CAP_LPM_MCU_ONLY,
	TISCI_MSG_FLAG_FW_CAP_LPM_PARTIAL_IO_ON,
	TISCI_MSG_FLAG_FW_CAP_LPM_DM_MANAGED,
	TISCI_MSG_FLAG_FW_CAP_LPM_IO_ONLY_PLUS_DDR,
	TISCI_MSG_FLAG_FW_CAP_IO_ISOLATION,
	TISCI_MSG_FLAG_FW_CAP_DM,
	TISCI_MSG_FLAG_FW_CAP_LPM_ABORT,
	TISCI_MSG_FLAG_FW_CAP_CLOCK_SSC,
	TISCI_MSG_FLAG_FW_CAP_GET_DEVICE_MULTIPLE,
};

#endif /* SOC_AM62AX_CAPABILITIES_H */
