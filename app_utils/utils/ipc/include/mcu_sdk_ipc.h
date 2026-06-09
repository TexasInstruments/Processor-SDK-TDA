/*
 *
 * Copyright (c) 2022-2026 Texas Instruments Incorporated
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

#ifndef MCU_SDK_IPC_H_
#define MCU_SDK_IPC_H_

#if defined(MCU_PLUS_SDK)
#include <drivers/ipc_notify.h>
#include <drivers/soc.h>
#elif defined(MCU_SDK)
#include <Ipc_Notify.h>
#include <RPMessage.h>
#include <device.h>
#if defined(PC)
#include <Ipc_Notify_Hostemu.h>
#endif
#endif

/* macros */
#if defined(MCU_PLUS_SDK) || defined(MCU_SDK)
/* Number of a buffers in a VRING, i.e depth of VRING queue */
#define IPC_RPMESSAGE_NUM_VRING_BUF       (256U)

/* Max size of a buffer in a VRING */
#define IPC_RPMESSAGE_MAX_VRING_BUF_SIZE  (512U)

/* Size of each VRING is
 *     2 x number of buffers x size of each buffer
 */
#define IPC_RPMESSAGE_VRING_SIZE          (2U * IPC_RPMESSAGE_NUM_VRING_BUF * IPC_RPMESSAGE_MAX_VRING_BUF_SIZE)
#endif

/* typedefs */
#if defined(PC)
/**
 * \brief Retrieve current CPU ID
 *
 * \return valid core ID on success, else -1
 */
typedef int16_t (*app_vdk_get_cpu_id_f)(void);
#endif /* #if defined(PC) */

/* functions */
#if defined(MCU_PLUS_SDK)
#define Ipc_mpGetSelfName()     SOC_getCoreName(IpcNotify_getSelfCoreId())
#define Ipc_mpGetName(name)     SOC_getCoreName(name)
#define Ipc_mpGetId(name)       SOC_getCoreId(name)
#elif defined(MCU_SDK)
#define Ipc_mpGetSelfName()  Device_GetCoreName(IpcNotify_getSelfCoreId())

#if defined(PC)
/**
 * \brief Register function that returns current CPU ID
 *
 * \param app_vdk_get_cpu_id [in] Pointer to function that returns current CPU ID
 */
void appIpcVdkRegisterGetCpuId(app_vdk_get_cpu_id_f app_vdk_get_cpu_id);

/**
 * \brief Register function that returns current CPU ID
 *
 * \param app_vdk_get_cpu_id [in] Pointer to function that returns current CPU ID
 */
void appRemoteServiceVdkRegisterGetCpuId(app_vdk_get_cpu_id_f app_vdk_get_cpu_id);
#endif /* #if defined(PC) */

#define Ipc_mpGetName(name)  Device_GetCoreName(name)
#define Ipc_mpGetId(name)    Device_GetCoreId((const char *)name)
#endif
/* #defines */
#define IPC_SOK                             SystemP_SUCCESS
/* Update LOCAL ENDPT to a number more than APP_IPC_CPU_MAX to avoid conflicts */
#define RPMESSAGE_LOCAL_ENDPT               (30U)

#endif
