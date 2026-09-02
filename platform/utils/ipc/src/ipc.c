/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include <platform_util_ipc.h>
#include <utils/ipc/include/app_ipc.h>

core_config_t core_config[] = {
#ifdef ENABLE_IPC_MPU1
    {APP_IPC_CPU_MPU1_0, false, true},
#endif
#ifdef ENABLE_IPC_MCU0
    {APP_IPC_CPU_MCU0_M55, false, true},
#endif
#ifdef ENABLE_IPC_MCU1
    {APP_IPC_CPU_MCU1_M55, false, true},
#endif
#ifdef ENABLE_IPC_MCU2
    {APP_IPC_CPU_MCU2_M55, false, true},
#endif
#ifdef ENABLE_IPC_MCU4
    {APP_IPC_CPU_MCU3_M55, false, true},
#endif
#ifdef ENABLE_IPC_MCU4
    {APP_IPC_CPU_MCU4_M55, false, true},
#endif
#ifdef ENABLE_IPC_MCU1_0
    {APP_IPC_CPU_MCU1_0, true, true},
#endif
#ifdef ENABLE_IPC_MCU1_1
    {APP_IPC_CPU_MCU1_1, true, true},
#endif
#ifdef ENABLE_IPC_MCU2_0
    {APP_IPC_CPU_MCU2_0, true, true},
#endif
#ifdef ENABLE_IPC_MCU2_1
    {APP_IPC_CPU_MCU2_1, true, true},
#endif
#ifdef ENABLE_IPC_MCU3_0
    {APP_IPC_CPU_MCU3_0, true, true},
#endif
#ifdef ENABLE_IPC_MCU3_1
    {APP_IPC_CPU_MCU3_1, true, true},
#endif
#ifdef ENABLE_IPC_MCU4_0
    {APP_IPC_CPU_MCU4_0, true, true},
#endif
#ifdef ENABLE_IPC_MCU4_1
    {APP_IPC_CPU_MCU4_1, true, true},
#endif
#ifdef ENABLE_IPC_RMCU0_0
    {APP_IPC_CPU_RMCU0_0, false, true},
#endif
#ifdef ENABLE_IPC_RMCU0_1
    {APP_IPC_CPU_RMCU0_1, false, true},
#endif
#ifdef ENABLE_IPC_RMCU1_0
    {APP_IPC_CPU_RMCU1_0, false, true},
#endif
#ifdef ENABLE_IPC_RMCU1_1
    {APP_IPC_CPU_RMCU1_1, false, true},
#endif
#ifdef ENABLE_IPC_RMCU2_0
    {APP_IPC_CPU_RMCU2_0, false, true},
#endif
#ifdef ENABLE_IPC_RMCU2_1
    {APP_IPC_CPU_RMCU2_1, false, true},
#endif
#ifdef ENABLE_IPC_C6x_1
    {APP_IPC_CPU_C6x_1, true, true},
#endif
#ifdef ENABLE_IPC_C6x_2
    {APP_IPC_CPU_C6x_2, true, true},
#endif
#ifdef ENABLE_IPC_C7x_1
    {APP_IPC_CPU_C7x_1, true, true},
#endif
#ifdef ENABLE_IPC_C7x_2
    {APP_IPC_CPU_C7x_2, true, true},
#endif
#ifdef ENABLE_IPC_C7x_3
    {APP_IPC_CPU_C7x_3, true, true},
#endif
#ifdef ENABLE_IPC_C7x_4
    {APP_IPC_CPU_C7x_4, true, true},
#endif
};

uint8_t num_cores = sizeof(core_config)/sizeof(core_config[0]);
