/*
 *  Copyright (c) Texas Instruments Incorporated 2018
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
 *            This is IPC documentation specific to J7 SoC
 *
 *  @{
 */

/**
 *  \file V1/ipc_soc.h
 *
 *  \brief IPC Low Level Driver J7 SOC specific file.
 */
#ifndef IPC_SOC_V1_H_
#define IPC_SOC_V1_H_

#include <ti/drv/ipc/include/ipc_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief VRing Buffer Size required for all core
 * combinations.
 */
#if defined (SOC_J7200)
#define IPC_VRING_BUFFER_SIZE   (0x800000U)
#else
#define IPC_VRING_BUFFER_SIZE   (0x1C00000U)
#endif

/*
 * For VLAB toolbox 0.14.5.snapshot2 onward,
 * this must be defined for C66x interrupt to work
 * properly.
 */
#define    SUPPORT_C66X_BIT0

/** \brief Core definitions */
#define    IPC_MPU1_0           (0U)    /**< ARM A72 - VM0 */
#define    IPC_MCU1_0           (1U)    /**< ARM MCU  R5F - core0 */
#define    IPC_MCU1_1           (2U)    /**< ARM MCU  R5F - core1 */
#define    IPC_MCU2_0           (3U)    /**< ARM Main R5F - core0 */
#define    IPC_MCU2_1           (4U)    /**< ARM Main R5F - core1 */
#if defined (SOC_J721E)
#define    IPC_MCU3_0           (5U)    /**< ARM Main R5F - core2 */
#define    IPC_MCU3_1           (6U)    /**< ARM Main R5F - core3 */
#define    IPC_C66X_1           (7U)    /**< DSP C66x - core0 */
#define    IPC_C66X_2           (8U)    /**< DSP C66x - core1 */
#define    IPC_C7X_1            (9U)    /**< DSP C7x - core0 */
#define    IPC_MPU1_1          (10U)    /**< ARM A72 - VM1 */
#define    IPC_MAX_PROCS       (11U)    /**< Maximum Processors */
#elif defined (SOC_J7200)
#define    IPC_MPU1_1           (5U)    /**< ARM A72 - VM1 */
#define    IPC_MAX_PROCS        (6U)    /**< Maximum Processors */
#endif

#define    IPC_MAILBOX_CLUSTER_CNT              (12U)
#define    IPC_MAILBOX_USER_CNT                  (4U)
#define    MAIN_NAVSS_MAILBOX_INPUTINTR_MAX    (440U)
#define    MAIN_NAVSS_MAILBOX_OUTPUTINTR_MAX   (512U)

#define IPC_MCU_NAVSS0_INTR0_CFG_BASE    (CSL_NAVSS_MAIN_INTR_ROUTER_CFG_REGS_0_BASE)

/* Could not find the RAT address in CSL */
#define IPC_C66X_RAT_BASE        (0x07ff0030U)

/* Virtual Address used for C66x to access j7_c66_interrupt_router */
#define IPC_C66X_INTR_VA_BASE    (0x18000000U)

/* Sclient does not configure RAT, so it must
 * be done C66x core
 * RAT configuration for C66x_1 and C66x_2 */
#define   IPC_C66X_1_INTR_PA_BASE     (CSL_C66SS0_INTROUTER0_INTR_ROUTER_CFG_BASE)
#define   IPC_C66X_2_INTR_PA_BASE     (CSL_C66SS1_INTROUTER0_INTR_ROUTER_CFG_BASE)
#define   C66X1_MBINTR_INPUT_BASE     (74U)
#define   C66X1_MBINTR_OFFSET         (84U)
#define   C66X1_MBINTR_OUTPUT_BASE    (96U)
#define   C66X2_MBINTR_INPUT_BASE     (74U)
#define   C66X2_MBINTR_OUTPUT_BASE    (96U)
#define   C66X2_MBINTR_OFFSET         (84U)

/* For C7x, DMSC does not configure or map CLEC router
 * it must be done by C7x software.
 */
#define   C7X_CLEC_BASE_ADDR              (CSL_COMPUTE_CLUSTER0_CLEC_REGS_BASE)

/* CLEC Offset = 992:- soc_events_in #32 is connected to CLEC event #1024 */
#define   C7X_CLEC_OFFSET                 (1024U - 32U)

/* User selected IRQ number */
/* Start of C7x events associated to CLEC that IPC Driver will manage */
/* Events  0 - 15  : left for other drivers, Timer Interrupts etc.
 * Events 16 - 47  : For routing DRU Local Events from CLEC (done by Vision Apps/TIDL)
 * Events 48 - 58  : managed by UDMA for routing various UDMA events to C7x
 * Events 59 - 63  : managed by IPC for routing various Mailbox events to C7x */
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

#if defined(BUILD_C66X)
void Ipc_configC66xIntrRouter(uint32_t input);
#endif

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

#endif /* #ifndef IPC_SOC_V1_H_ */

/* @} */
