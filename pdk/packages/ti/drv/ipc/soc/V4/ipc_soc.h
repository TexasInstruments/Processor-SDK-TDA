/*
 *  Copyright (c) Texas Instruments Incorporated 2022
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \ingroup DRV_IPC_MODULE
 *  \defgroup DRV_IPC_SOC_MODULE IPC SoC Config
 *            This is IPC documentation specific to J784S4/J742S2 SoC
 *
 *  @{
 */

/**
 *  \file V4/ipc_soc.h
 *
 *  \brief IPC Low Level Driver J784S4/J742S2 SOC specific file.
 */
#ifndef IPC_SOC_V4_H_
#define IPC_SOC_V4_H_

#include <ti/drv/ipc/include/ipc_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief VRing Buffer Size required for all core combinations.
 */
#define IPC_VRING_BUFFER_SIZE   (0x3000000U)

/** \brief Core definitions */
#define    IPC_MPU1_0           (0U)    /**< ARM A72 - VM0 */
#define    IPC_MCU1_0           (1U)    /**< ARM MCU  R5F - core0 */
#define    IPC_MCU1_1           (2U)    /**< ARM MCU  R5F - core1 */
#define    IPC_MCU2_0           (3U)    /**< ARM Main R5F - core0 */
#define    IPC_MCU2_1           (4U)    /**< ARM Main R5F - core1 */
#define    IPC_MCU3_0           (5U)    /**< ARM Main R5F - core2 */
#define    IPC_MCU3_1           (6U)    /**< ARM Main R5F - core3 */
#define    IPC_MCU4_0           (7U)    /**< ARM Main R5F - core4 */
#define    IPC_MCU4_1           (8U)    /**< ARM Main R5F - core5 */
#define    IPC_C7X_1            (9U)    /**< DSP C7x - core0 */
#define    IPC_C7X_2           (10U)    /**< DSP C7x - core1 */
#define    IPC_C7X_3           (11U)    /**< DSP C7x - core2 */
#if defined (SOC_J784S4)
#define    IPC_C7X_4           (12U)    /**< DSP C7x - core3 */
#define    IPC_MPU1_1          (13U)    /**< ARM A72 - VM1 */
#define    IPC_MAX_PROCS       (14U)    /**< Maximum Processors */
#elif defined (SOC_J742S2)
#define    IPC_MPU1_1          (12U)    /**< ARM A72 - VM1 */
#define    IPC_MAX_PROCS       (13U)    /**< Maximum Processors */ 
#endif

#define    IPC_MAILBOX_CLUSTER_CNT              (18U)
#define    IPC_MAILBOX_USER_CNT                  (4U)
#define    MAIN_NAVSS_MAILBOX_INPUTINTR_MAX    (440U)
#define    MAIN_NAVSS_MAILBOX_OUTPUTINTR_MAX   (512U)

#define IPC_MCU_NAVSS0_INTR0_CFG_BASE    (CSL_NAVSS0_INTR0_INTR_ROUTER_CFG_BASE)

/* For C7x, DMSC does not configure or map CLEC router
 * it must be done by C7x software.
 */
#define   C7X_CLEC_BASE_ADDR              (CSL_COMPUTE_CLUSTER0_CLEC_REGS_BASE)

/* CLEC Offset = 992:- soc_events_in #32 is connected to CLEC event #1024 */
/* CLEC is shared b/w all C7x cores and the offset can be common.
 * ClecEvent # will be different for a particular C7x since the
 * range is returned from BoardCfg based on core specific allocation */
#define   C7X_CLEC_OFFSET                 (1024U - 32U)

/* User selected IRQ number */
/* Start of C7x events associated to CLEC that IPC Driver will manage */
/* Events  0 - 15  : left for other drivers, Timer Interrupts etc.
 * Events 16 - 47  : For routing DRU Local Events from CLEC (done by Vision Apps/TIDL)
 * Events 48 - 58  : managed by UDMA for routing various UDMA events to C7x
 * Events 59 - 63  : managed by IPC for routing various Mailbox events to C7x */ 
/* Even though same CLEC is shared b/w two C7x cores, CLEC can broadcast the
 * event to any C7x core and CPU IRQ is core specific.
 * Hence same Mailbox Interrupt offset can be used for both C7x cores.  */
#define   IPC_C7X_MBINTR_OFFSET            (59U)

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */



/* @} */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if defined(BUILD_C7X)
uint32_t Ipc_configClecRouter(uint32_t corePackEvent, uint32_t corePackEventBase);
#endif
/* remote core id is the first arguement for Ipc_sciclientIrqRelease
 * and Ipc_sciclientIrqSet*/
int32_t Ipc_sciclientIrqRelease(uint16_t coreId, uint32_t clusterId,
        uint32_t userId, uint32_t intNumber);
int32_t Ipc_sciclientIrqSet(uint16_t coreId, uint32_t clusterId,
        uint32_t userId, uint32_t intNumber);
int32_t Ipc_getIntNumRange(uint32_t coreIndex, uint16_t *rangeStartP,
        uint16_t *rangeNumP);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IPC_SOC_V4_H_ */

/* @} */
