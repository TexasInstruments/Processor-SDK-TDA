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

#ifndef APP_HOST_TARGET_CFG_H_
#define APP_HOST_TARGET_CFG_H_

/**
 * This ID is used by target cores to identify which core is acting as the OpenVX host.
 * The value corresponds to the host configuration selected:
 *   0U = MPU_0 is the host (default)
 *   1U = MCU2_0 is the host
 *   2U = MCU2_1 is the host
 *
 * This value may be redefined based on the active host configuration below.
 */
#define TIOVX_REMOTE_HOST_ID 0U

/**
 * When ENABLE_TIOVX_HOST is defined for a specific CPU, that core acts as an
 * OpenVX host that is responsible for graph creation, verification, scheduling,
 * and coordinating execution across target cores.
 *
 * Possible values:
 *   1U = MCU2_0 is the OpenVX host
 *   2U = MCU2_1 is the OpenVX host
 *   undefined = This core is a target/remote core, not a host
 *
 * Multiple hosts can be enabled simultaneously in the system, allowing different
 * cores to act as independent OpenVX hosts for separate applications.
 */

#if defined (ENABLE_MCU2_0_TIOVX_HOST)
    #if defined (CPU_mcu2_0)
        #define ENABLE_TIOVX_HOST 1U
    #endif /* #ifdef CPU_mcu2_0 */
    #if defined (CPU_mcu4_0)
        #undef TIOVX_REMOTE_HOST_ID
        #define TIOVX_REMOTE_HOST_ID 1U
    #endif /* #ifdef CPU_mcu4_0 */

#endif /* #if defined (ENABLE_MCU2_0_TIOVX_HOST) */

#if defined (ENABLE_MCU2_1_TIOVX_HOST)
    #if defined (CPU_mcu2_1)
        #define ENABLE_TIOVX_HOST 2U
    #endif /* #ifdef CPU_mcu2_1 */
    #if defined (CPU_c7x_2)
        #undef TIOVX_REMOTE_HOST_ID
        #define TIOVX_REMOTE_HOST_ID 2U
    #endif /* #ifdef CPU_c7x_2 */
#endif /* #ifdef ENABLE_MCU2_1_TIOVX_HOST */

#endif /* APP_HOST_TARGET_CFG_H_ */
