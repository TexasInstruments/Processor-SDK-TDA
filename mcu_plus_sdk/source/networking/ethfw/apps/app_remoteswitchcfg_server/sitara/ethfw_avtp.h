/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*!
 * \file ethfw_avtp.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware AVTP support.
 */

#ifndef ETHFW_AVTP_H_
#define ETHFW_AVTP_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "apps/app_remoteswitchcfg_server/sitara/app_cpswconfighandler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define ETHFW_TSN_AVTPD_TASK_PRIORITY              (2)
#define ETHFW_TSN_AUTOAMP_APP_TASK_PRIORITY        (31)
#define ETHFW_TSN_AUTOAMP_APP_RX_TASK_PRIORITY     (10)

#if defined(SAFERTOS)
#define TSN_TSK_STACK_SIZE                         (16U * 1024U)
#define TSN_TSK_STACK_ALIGN                        TSN_TSK_STACK_SIZE
#else
#if _DEBUG_ == 1
/* 16k is enough for debug mode */
#define TSN_TSK_STACK_SIZE                         (16U * 1024U)
#else
#define TSN_TSK_STACK_SIZE                         (8U * 1024U)
#endif
#define TSN_TSK_STACK_ALIGN                        (32U)
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint8_t gAvtpdStackBuf[TSN_TSK_STACK_SIZE] __attribute__ ((aligned(TSN_TSK_STACK_ALIGN)));

uint8_t gTxStackBuf[16U * 1024U] __attribute__ ((aligned(TSN_TSK_STACK_ALIGN)));

uint8_t gRx1StackBuf[TSN_TSK_STACK_SIZE] __attribute__ ((aligned(TSN_TSK_STACK_ALIGN)));

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int EnetApp_autoAmpAppInit();

void *EnetApp_avtpdTask(void *arg);

void *EnetApp_ListenerTask(void *arg);

void *EnetApp_talkerTask(void *arg);

void EnetApp_enableTsSync(Enet_Type enetType,
                          uint32_t instId);

int EnetApp_avtpInit();

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_AVTP_H_ */