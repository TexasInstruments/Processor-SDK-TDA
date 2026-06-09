/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#ifndef APP_CFG_MCU2_0_H_
#define APP_CFG_MCU2_0_H_

#include <app_cfg.h>

#define L3_MEM_SIZE (MAIN_OCRAM_MCU2_0_SIZE)

#define DDR_HEAP_MEM_SIZE               (DDR_MCU2_0_LOCAL_HEAP_SIZE)
#define DDR_VISS_HEAP_MEM_SIZE          (DDR_MCU2_0_VISS_CONFIG_HEAP_SIZE)


/* There are several external board interfaces to the SoC that can either be controlled by the MCU RTOS core
 * or the MPU HLOS core.  The top level build option BUILD_MCU_BOARD_DEPENDENCIES can be used to optionally enable
 * these interfaces (user can override specific flags in this file), or disable them.
 */
#ifdef BUILD_MCU_BOARD_DEPENDENCIES

    #define ENABLE_CSI2RX
    #define ENABLE_CSI2TX
    #undef  ENABLE_DSS_HDMI

    /* IMPORANT NOTE:
     * - Only one of ENABLE_DSS_SINGLE or ENABLE_DSS_DUAL should be defined
     * - When ENABLE_DSS_SINGLE is defined, only one of ENABLE_DSS_DSI or ENABLE_DSS_EDP should be defined
     * - When ENABLE_DSS_DUAL is defined, ENABLE_DSS_DSI and ENABLE_DSS_EDP are not used, both EDP and DSI are enabled unconditionally
     */
    #define ENABLE_DSS_SINGLE
    #undef  ENABLE_DSS_DUAL

#if defined(ENABLE_DSS_DUAL)
    #undef  ENABLE_DSS_SINGLE
    #define ENABLE_DSS_EDP
    #define ENABLE_DSS_DSI
#elif defined(ENABLE_DSS_SINGLE)
    #undef  ENABLE_DSS_DUAL
    #undef  ENABLE_DSS_DSI
    #define ENABLE_DSS_EDP
#endif
#if defined(ENABLE_DSS_DSI)
    #undef  ENABLE_CSI2TX
#endif
    #define ENABLE_I2C
    #define ENABLE_BOARD
#else

    #undef ENABLE_CSI2RX
    #undef ENABLE_CSI2TX
    #undef ENABLE_DSS_SINGLE
    #undef ENABLE_DSS_DUAL
    #undef ENABLE_DSS_EDP
    #undef ENABLE_DSS_HDMI
    #undef ENABLE_DSS_DSI
    #undef ENABLE_I2C
    #undef ENABLE_BOARD

#endif


#define ENABLE_FVID2
#define ENABLE_VHWA_VPAC0
#undef ENABLE_VHWA_DMPAC
#define ENABLE_AEWB_KERNELS

#if defined (ENABLE_DSS_DSI) && !defined(ENABLE_I2C)
#error "DSI output depends on I2C, Include ENABLE_I2C"
#endif
#if defined (ENABLE_DSS_HDMI) && !defined(ENABLE_I2C)
#error "HDMI output depends on I2C, Include ENABLE_I2C"
#endif

#if defined (ENABLE_DSS_DSI) && defined (ENABLE_CSI2TX)
#error "CSI2TX and DSI cannot be active at the same time"
#endif

#endif /* APP_CFG_MCU2_0_H_ */
