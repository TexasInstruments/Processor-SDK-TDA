/*
 * System Firmware Source File
 *
 * Host IDs for J7200 device
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
#ifndef J7200_HOSTS_H
#define J7200_HOSTS_H

/** DMSC(Secure): Security Controller */
#define HOST_ID_DMSC (0U)
/** DM(Non Secure): Device Management */
#define HOST_ID_DM (254U)
/** MCU_0_R5_0(Non Secure): Cortex R5 context 0 on MCU island */
#define HOST_ID_MCU_0_R5_0 (3U)
/** MCU_0_R5_1(Secure): Cortex R5 context 1 on MCU island(Boot) */
#define HOST_ID_MCU_0_R5_1 (4U)
/** MCU_0_R5_2(Non Secure): Cortex R5 context 2 on MCU island */
#define HOST_ID_MCU_0_R5_2 (5U)
/** MCU_0_R5_3(Secure): Cortex R5 context 3 on MCU island */
#define HOST_ID_MCU_0_R5_3 (6U)
/** A72_0(Secure): Cortex A72 context 0 on Main island */
#define HOST_ID_A72_0 (10U)
/** A72_1(Secure): Cortex A72 context 1 on Main island */
#define HOST_ID_A72_1 (11U)
/** A72_2(Non Secure): Cortex A72 context 2 on Main island */
#define HOST_ID_A72_2 (12U)
/** A72_3(Non Secure): Cortex A72 context 3 on Main island */
#define HOST_ID_A72_3 (13U)
/** A72_4(Non Secure): Cortex A72 context 4 on Main island */
#define HOST_ID_A72_4 (14U)
/** MAIN_0_R5_0(Non Secure): Cortex R5_0 context 0 on Main island */
#define HOST_ID_MAIN_0_R5_0 (35U)
/** MAIN_0_R5_1(Secure): Cortex R5_0 context 1 on Main island */
#define HOST_ID_MAIN_0_R5_1 (36U)
/** MAIN_0_R5_2(Non Secure): Cortex R5_0 context 2 on Main island */
#define HOST_ID_MAIN_0_R5_2 (37U)
/** MAIN_0_R5_3(Secure): Cortex R5_0 context 3 on Main island */
#define HOST_ID_MAIN_0_R5_3 (38U)
/** DM2DMSC(Secure): DM to DMSC communication */
#define HOST_ID_DM2DMSC (250U)
/** DMSC2DM(Non Secure): DMSC to DM communication */
#define HOST_ID_DMSC2DM (251U)
/** A72_5(Non Secure): Cortex A72 context 5 on Main island */
#define HOST_ID_A72_5 (240U)

/**
 * Host catch all. Used in board configuration resource assignments to define
 * resource ranges useable by all hosts. Cannot be used
 */
#define HOST_ID_ALL (128U)

/** Number of unique hosts on the SoC */
#define HOST_ID_CNT (18U)

#endif /* J7200_HOSTS_H */
