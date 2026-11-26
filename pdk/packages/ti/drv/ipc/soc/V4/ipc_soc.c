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
 *  \file ipc_soc.c
 *
 *  \brief File containing the IPC driver - J784S4/J742S2 SOC specific implementation.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <ti/drv/ipc/ipc.h>
#include <ti/drv/ipc/soc/ipc_soc.h>
#include <ti/drv/ipc/src/ipc_priv.h>
#include <ti/drv/ipc/src/ipc_mailbox.h>
#include <ti/drv/ipc/src/ipc_utils.h>

#include <ti/csl/soc.h>
#include <ti/csl/csl_navss_main.h>
#include <ti/csl/csl_intr_router.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/drv/sciclient/sciclient.h>

#define MAILBOX_REG_SIZE                    (0x200)

#define IPC_INTERRUPT_OFFSET                (6U)

/**
 * \brief Main NavSS512 - Mailbox input line
 */
static uint32_t g_Navss512MbInput[IPC_MAILBOX_CLUSTER_CNT] =
{
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_0_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_1_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_2_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_3_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_4_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_5_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_6_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_7_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_8_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_9_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_10_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX_11_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX1_0_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX1_1_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX1_2_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX1_3_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX1_4_PEND_INTR_0,
    CSLR_NAVSS0_INTR_0_IN_INTR_NAVSS0_MAILBOX1_5_PEND_INTR_0
};

/**
 *  \brief Processor IDs to name mapping for all processor in Jacinto7
 */
static Ipc_ProcInfo g_Ipc_mp_procInfo[IPC_MAX_PROCS] =
{
    {IPC_MPU1_0,      "mpu1_0"},      /**< ARM A72 - VM0 */
    {IPC_MCU1_0,      "mcu1_0"},      /**< ARM MCU  R5F - core0 */
    {IPC_MCU1_1,      "mcu1_1"},      /**< ARM MCU  R5F - core1 */
    {IPC_MCU2_0,      "mcu2_0"},      /**< ARM Main R5F - core0 */
    {IPC_MCU2_1,      "mcu2_1"},      /**< ARM Main R5F - core1 */
    {IPC_MCU3_0,      "mcu3_0"},      /**< ARM Main R5F - core2 */
    {IPC_MCU3_1,      "mcu3_1"},      /**< ARM Main R5F - core3 */
    {IPC_MCU4_0,      "mcu4_0"},      /**< ARM Main R5F - core4 */
    {IPC_MCU4_1,      "mcu4_1"},      /**< ARM Main R5F - core5 */
    {IPC_C7X_1,       "c7x_1"},       /**< DSP C7x - core0 */
    {IPC_C7X_2,       "c7x_2"},       /**< DSP C7x - core1 */
    {IPC_C7X_3,       "c7x_3"},       /**< DSP C7x - core2 */
#if defined (SOC_J784S4)
    {IPC_C7X_4,       "c7x_4"},       /**< DSP C7x - core3 */
#endif
    {IPC_MPU1_1,      "mpu1_1"}       /**< ARM A72 - VM1 */
};

/* Mailbox Cluster Base Address */
static uintptr_t g_IPC_Mailbox_BasePhyAddr[IPC_MAILBOX_CLUSTER_CNT] =
{
    CSL_NAVSS_MAIN_MAILBOX_REGS_0_BASE,     /* Mailbox - cluster0   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_1_BASE,     /* Mailbox - cluster1   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_2_BASE,     /* Mailbox - cluster2   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_3_BASE,     /* Mailbox - cluster3   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_4_BASE,     /* Mailbox - cluster4   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_5_BASE,     /* Mailbox - cluster5   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_6_BASE,     /* Mailbox - cluster6   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_7_BASE,     /* Mailbox - cluster7   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_8_BASE,     /* Mailbox - cluster8   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_9_BASE,     /* Mailbox - cluster9   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_10_BASE,    /* Mailbox - cluster10  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_11_BASE,    /* Mailbox - cluster11  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_12_BASE,    /* Mailbox - cluster12  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_13_BASE,    /* Mailbox - cluster13  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_14_BASE,    /* Mailbox - cluster14  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_15_BASE,    /* Mailbox - cluster15  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_16_BASE,    /* Mailbox - cluster16  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_17_BASE,    /* Mailbox - cluster17  */
};

#if defined(BUILD_MPU1_0) && defined(QNX_OS)
/* Mailbox Cluster Base Address  - virtual address for HLOS */
static uintptr_t g_IPC_Mailbox_BaseVirtAddr[IPC_MAILBOX_CLUSTER_CNT] =
{
    0
};
#endif

static Ipc_MailboxInfo   g_IPC_MailboxInfo[IPC_MAX_PROCS][IPC_MAX_PROCS] =
{
    /* Host Processor - A72-vm0	*/
    {
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,    0U} },  /* Self - A72-vm0 */
        { {    0U,    0U,  0U}, {    0U,    0U,    1U} },  /* mcu-r5f0 */
        { {    0U,    0U,  2U}, {    0U,    0U,    3U} },  /* mcu-r5f1 */
        { {    1U,    0U,  0U}, {    1U,    0U,    1U} },  /* main-r5f0 */
        { {    1U,    0U,  2U}, {    1U,    0U,    3U} },  /* main-r5f1 */
        { {    2U,    0U,  0U}, {    2U,    0U,    1U} },  /* main-r5f2 */
        { {    2U,    0U,  2U}, {    2U,    0U,    3U} },  /* main-r5f3 */
        { {    3U,    0U,  0U}, {    3U,    0U,    1U} },  /* main-r5f4 */
        { {    3U,    0U,  2U}, {    3U,    0U,    3U} },  /* main-r5f5 */
        { {    4U,    0U,  0U}, {    4U,    0U,    1U} },  /* C7x-1 */
        { {    4U,    0U,  2U}, {    4U,    0U,    3U} },  /* C7x-2 */
        { {    5U,    0U,  0U}, {    5U,    0U,    1U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {    5U,    0U,  2U}, {    5U,    0U,    3U} },  /* C7x-4 */
#endif
        { {    0U,    0U, 10U}, {    0U,    0U,   11U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu1_0 	*/
    {
        { {    0U,    1U,  1U }, {    0U,    1U,  0U} },  /* A72-vm0 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - mcu-r5f0 */
        { {    0U,    1U,  4U }, {    0U,    1U,  5U} },  /* mcu-r5f1 */
        { {   12U,    0U,  0U }, {    6U, 0xFFU,  4U} },  /* main-r5f0 */
        { {   12U,    0U,  1U }, {    7U, 0xFFU,  4U} },  /* main-r5f1 */
        { {   12U,    0U,  2U }, {    8U, 0xFFU,  4U} },  /* main-r5f2 */
        { {   12U,    0U,  3U }, {    9U, 0xFFU,  4U} },  /* main-r5f3 */
        { {   12U,    0U,  4U }, {   10U, 0xFFU,  4U} },  /* main-r5f4 */
        { {   12U,    1U,  5U }, {   11U, 0xFFU,  4U} },  /* main-r5f5 */
        { {   12U,    1U,  6U }, {   14U, 0xFFU,  6U} },  /* C7x-1 */
        { {   12U,    1U,  7U }, {   15U, 0xFFU,  6U} },  /* C7x-2 */
        { {   12U,    1U,  8U }, {   16U, 0xFFU,  6U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {   12U,    1U,  9U }, {   17U, 0xFFU,  6U} },  /* C7x-4 */
#endif
        { {    0U,    1U,  7U }, {    0U,    1U,  6U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu1_1 */
    {
        { {    0U,    2U,  3U }, {    0U,    2U,  2U} },  /* A72-vm0 */
        { {    0U,    2U,  5U }, {    0U,    2U,  4U} },  /* mcu-r5f0 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - mcu-r5f1 */
        { {   13U,    0U,  0U }, {    6U, 0xFFU,  5U} },  /* main-r5f0 */
        { {   13U,    0U,  1U }, {    7U, 0xFFU,  5U} },  /* main-r5f1 */
        { {   13U,    0U,  2U }, {    8U, 0xFFU,  5U} },  /* main-r5f2 */
        { {   13U,    0U,  3U }, {    9U, 0xFFU,  5U} },  /* main-r5f3 */
        { {   13U,    0U,  4U }, {   10U, 0xFFU,  5U} },  /* main-r5f4 */
        { {   13U,    1U,  5U }, {   11U, 0xFFU,  5U} },  /* main-r5f5 */
        { {   13U,    1U,  6U }, {   14U, 0xFFU,  7U} },  /* C7x-1 */
        { {   13U,    1U,  7U }, {   15U, 0xFFU,  7U} },  /* C7x-2 */
        { {   13U,    1U,  8U }, {   16U, 0xFFU,  7U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {   13U,    1U,  9U }, {   17U, 0xFFU,  7U} },  /* C7x-4 */
#endif
        { {    0U,    2U,  9U }, {    0U,    2U,  8U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu2_0  */
    {
        { {    1U,    1U,  1U}, {    1U,    1U, 0U} },  /* A72-vm0 */
        { {    6U,    0U,  4U}, {   12U, 0xFFU, 0U} },  /* mcu-r5f0 */
        { {    6U,    1U,  5U}, {   13U, 0xFFU, 0U} },  /* mcu-r5f1 */
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU, 0U} },  /* Self - main-r5f0 */
        { {    1U,    1U,  4U}, {    1U,    1U, 5U} },  /* main-r5f1 */
        { {    6U,    0U,  0U}, {    8U, 0xFFU, 0U} },  /* main-r5f2 */
        { {    6U,    0U,  1U}, {    9U, 0xFFU, 0U} },  /* main-r5f3 */
        { {    6U,    0U,  2U}, {   10U, 0xFFU, 0U} },  /* main-r5f4 */
        { {    6U,    0U,  3U}, {   11U, 0xFFU, 0U} },  /* main-r5f5 */
        { {    6U,    1U,  6U}, {   14U, 0xFFU, 0U} },  /* C7x-1 */
        { {    6U,    1U,  7U}, {   15U, 0xFFU, 0U} },  /* C7x-2 */
        { {    6U,    1U,  8U}, {   16U, 0xFFU, 0U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {    6U,    1U,  9U}, {   17U, 0xFFU, 0U} },  /* C7x-4 */
#endif
        { {    1U,    1U,  7U}, {    1U,    1U, 6U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu2_1 */
    {
        { {    1U,    2U,  3U }, {    1U,    2U, 2U} },  /* A72-vm0 */
        { {    7U,    0U,  4U }, {   12U, 0xFFU, 1U} },  /* mcu-r5f0 */
        { {    7U,    1U,  5U }, {   13U, 0xFFU, 1U} },  /* mcu-r5f1 */
        { {    1U,    2U,  5U }, {    1U,    2U, 4U} },  /* main-r5f0 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU, 0U} },  /* Self - main-r5f1 */
        { {    7U,    0U,  0U }, {    8U, 0xFFU, 1U} },  /* main-r5f2 */
        { {    7U,    0U,  1U }, {    9U, 0xFFU, 1U} },  /* main-r5f3 */
        { {    7U,    0U,  2U }, {   10U, 0xFFU, 1U} },  /* main-r5f4 */
        { {    7U,    0U,  3U }, {   11U, 0xFFU, 1U} },  /* main-r5f5 */
        { {    7U,    1U,  6U }, {   14U, 0xFFU, 1U} },  /* C7x-1 */
        { {    7U,    1U,  7U }, {   15U, 0xFFU, 1U} },  /* C7x-2 */
        { {    7U,    1U,  8U }, {   16U, 0xFFU, 1U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {    7U,    1U,  9U }, {   17U, 0xFFU, 1U} },  /* C7x-4 */
#endif
        { {    1U,    2U,  9U }, {    1U,    2U, 8U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu3_0 */
    {
        { {    2U,    1U,  1U }, {    2U,    1U,  0U} },  /* A72-vm0 */
        { {    8U,    0U,  4U }, {   12U, 0xFFU,  2U} },  /* mcu-r5f0 */
        { {    8U,    1U,  5U }, {   13U, 0xFFU,  2U} },  /* mcu-r5f1 */
        { {    8U,    0U,  0U }, {    6U, 0xFFU,  0U} },  /* main-r5f0 */
        { {    8U,    0U,  1U }, {    7U, 0xFFU,  0U} },  /* main-r5f1 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - main-r5f2 */
        { {    2U,    1U,  4U }, {    2U,    1U,  5U} },  /* main-r5f3 */
        { {    8U,    0U,  2U }, {   10U, 0xFFU,  2U} },  /* main-r5f4 */
        { {    8U,    0U,  3U }, {   11U, 0xFFU,  2U} },  /* main-r5f5 */
        { {    8U,    1U,  6U }, {   14U, 0xFFU,  2U} },  /* C7x-1 */
        { {    8U,    1U,  7U }, {   15U, 0xFFU,  2U} },  /* C7x-2 */
        { {    8U,    1U,  8U }, {   16U, 0xFFU,  2U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {    8U,    1U,  9U }, {   17U, 0xFFU,  2U} },  /* C7x-4 */
#endif
        { {    2U,    1U,  7U }, {    2U,    1U,  6U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu3_1	*/
    {
        { {    2U,    2U,  3U }, {    2U,    2U,  2U} },  /* A72-vm0 */
        { {    9U,    0U,  4U }, {   12U, 0xFFU,  3U} },  /* mcu-r5f0 */
        { {    9U,    1U,  5U }, {   13U, 0xFFU,  3U} },  /* mcu-r5f1 */
        { {    9U,    0U,  0U }, {    6U, 0xFFU,  1U} },  /* main-r5f0 */
        { {    9U,    0U,  1U }, {    7U, 0xFFU,  1U} },  /* main-r5f1 */
        { {    2U,    2U,  5U }, {    2U,    2U,  4U} },  /* main-r5f2 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - main-r5f3 */
        { {    9U,    0U,  2U }, {   10U, 0xFFU,  3U} },  /* main-r5f4 */
        { {    9U,    0U,  3U }, {   11U, 0xFFU,  3U} },  /* main-r5f5 */
        { {    9U,    1U,  6U }, {   14U, 0xFFU,  3U} },  /* C7x-1 */
        { {    9U,    1U,  7U }, {   15U, 0xFFU,  3U} },  /* C7x-2 */
        { {    9U,    1U,  8U }, {   16U, 0xFFU,  3U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {    9U,    1U,  9U }, {   17U, 0xFFU,  3U} },  /* C7x-4 */
#endif
        { {    2U,    2U,  9U }, {    2U,    2U,  8U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu4_0 */
    {
        { {    3U,    1U,  1U }, {    3U,    1U,  0U} },  /* A72-vm0 */
        { {   10U,    0U,  4U }, {   12U, 0xFFU,  4U} },  /* mcu-r5f0 */
        { {   10U,    1U,  5U }, {   13U, 0xFFU,  4U} },  /* mcu-r5f1 */
        { {   10U,    0U,  0U }, {    6U, 0xFFU,  2U} },  /* main-r5f0 */
        { {   10U,    0U,  1U }, {    7U, 0xFFU,  2U} },  /* main-r5f1 */
        { {   10U,    0U,  2U }, {    8U, 0xFFU,  2U} },  /* main-r5f2 */
        { {   10U,    0U,  3U }, {    9U, 0xFFU,  2U} },  /* main-r5f3 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - main-r5f4 */
        { {    3U,    1U,  4U }, {    3U,    1U,  5U} },  /* main-r5f5 */
        { {   10U,    1U,  6U }, {   14U, 0xFFU,  4U} },  /* C7x-1 */
        { {   10U,    1U,  7U }, {   15U, 0xFFU,  4U} },  /* C7x-2 */
        { {   10U,    1U,  8U }, {   16U, 0xFFU,  4U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {   10U,    1U,  9U }, {   17U, 0xFFU,  4U} },  /* C7x-4 */
#endif
        { {    3U,    1U,  7U }, {    3U,    1U,  6U} }   /* A72-vm1 */
    },
    /* Host Processor - mcu4_1	*/
    {
        { {    3U,    2U,  3U }, {    3U,    2U,  2U} },  /* A72-vm0 */
        { {   11U,    0U,  4U }, {   12U, 0xFFU,  5U} },  /* mcu-r5f0 */
        { {   11U,    1U,  5U }, {   13U, 0xFFU,  5U} },  /* mcu-r5f1 */
        { {   11U,    0U,  0U }, {    6U, 0xFFU,  3U} },  /* main-r5f0 */
        { {   11U,    0U,  1U }, {    7U, 0xFFU,  3U} },  /* main-r5f1 */
        { {   11U,    0U,  2U }, {    8U, 0xFFU,  3U} },  /* main-r5f2 */
        { {   11U,    0U,  3U }, {    9U, 0xFFU,  3U} },  /* main-r5f3 */
        { {    3U,    2U,  5U }, {    3U,    2U,  4U} },  /* main-r5f4 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - main-r5f5 */
        { {   11U,    1U,  6U }, {   14U, 0xFFU,  5U} },  /* C7x-1 */
        { {   11U,    1U,  7U }, {   15U, 0xFFU,  5U} },  /* C7x-2 */
        { {   11U,    1U,  8U }, {   16U, 0xFFU,  5U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {   11U,    1U,  9U }, {   17U, 0xFFU,  5U} },  /* C7x-4 */
#endif
        { {    3U,    2U,  9U }, {    3U,    2U,  8U} }   /* A72-vm1 */
    },
    /* Host Processor - c7x_1	*/
    {
        { {    4U,    1U,  1U}, {    4U,    1U,  0U} },  /* A72-vm0 */
        { {   14U,    1U,  6U}, {   12U, 0xFFU,  6U} },  /* mcu-r5f0 */
        { {   14U,    1U,  7U}, {   13U, 0xFFU,  6U} },  /* mcu-r5f1 */
        { {   14U,    0U,  0U}, {    6U, 0xFFU,  6U} },  /* main-r5f0 */
        { {   14U,    0U,  1U}, {    7U, 0xFFU,  6U} },  /* main-r5f1 */
        { {   14U,    0U,  2U}, {    8U, 0xFFU,  6U} },  /* main-r5f2 */
        { {   14U,    0U,  3U}, {    9U, 0xFFU,  6U} },  /* main-r5f3 */
        { {   14U,    0U,  4U}, {   10U, 0xFFU,  6U} },  /* main-r5f4 */
        { {   14U,    1U,  5U}, {   11U, 0xFFU,  6U} },  /* main-r5f5 */
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,  0U} },  /* Self - C7x-1 */
        { {    4U,    1U,  4U}, {    4U,    1U,  5U} },  /* C7x-2 */
        { {   14U,    1U,  8U}, {   16U, 0xFFU,  8U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {   14U,    1U,  9U}, {   17U, 0xFFU,  8U} },  /* C7x-4 */
#endif
        { {    4U,    1U,  7U}, {    4U,    1U,  6U} }   /* A72-vm1 */
    },
    /* Host Processor - c7x_2	*/
    {
        { {    4U,    2U,  3U}, {    4U,    2U,  2U} },  /* A72-vm0 */
        { {   15U,    1U,  6U}, {   12U, 0xFFU,  7U} },  /* mcu-r5f0 */
        { {   15U,    1U,  7U}, {   13U, 0xFFU,  7U} },  /* mcu-r5f1 */
        { {   15U,    0U,  0U}, {    6U, 0xFFU,  7U} },  /* main-r5f0 */
        { {   15U,    0U,  1U}, {    7U, 0xFFU,  7U} },  /* main-r5f1 */
        { {   15U,    0U,  2U}, {    8U, 0xFFU,  7U} },  /* main-r5f2 */
        { {   15U,    0U,  3U}, {    9U, 0xFFU,  7U} },  /* main-r5f3 */
        { {   15U,    0U,  4U}, {   10U, 0xFFU,  7U} },  /* main-r5f4 */
        { {   15U,    1U,  5U}, {   11U, 0xFFU,  7U} },  /* main-r5f5 */
        { {    4U,    2U,  5U}, {    4U,    2U,  4U} },  /* C7x-1 */
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,  0U} },  /* Self - C7x-2 */
        { {   15U,    1U,  8U}, {   16U, 0xFFU,  9U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {   15U,    1U,  9U}, {   17U, 0xFFU,  9U} },  /* C7x-4 */
#endif
        { {    4U,    2U,  9U}, {    4U,    2U,  8U} }   /* A72-vm1 */
    },
    /* Host Processor - c7x_3	*/
    {
        { {    5U,    1U,  1U}, {    5U,    1U,  0U} },  /* A72-vm0 */
        { {   16U,    1U,  6U}, {   12U, 0xFFU,  8U} },  /* mcu-r5f0 */
        { {   16U,    1U,  7U}, {   13U, 0xFFU,  8U} },  /* mcu-r5f1 */
        { {   16U,    0U,  0U}, {    6U, 0xFFU,  8U} },  /* main-r5f0 */
        { {   16U,    0U,  1U}, {    7U, 0xFFU,  8U} },  /* main-r5f1 */
        { {   16U,    0U,  2U}, {    8U, 0xFFU,  8U} },  /* main-r5f2 */
        { {   16U,    0U,  3U}, {    9U, 0xFFU,  8U} },  /* main-r5f3 */
        { {   16U,    0U,  4U}, {   10U, 0xFFU,  8U} },  /* main-r5f4 */
        { {   16U,    1U,  5U}, {   11U, 0xFFU,  8U} },  /* main-r5f5 */
        { {   16U,    1U,  8U}, {   14U, 0xFFU,  8U} },  /* C7x-1 */
        { {   16U,    1U,  9U}, {   15U, 0xFFU,  8U} },  /* C7x-2 */
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,  0U} },  /* Self - C7x-3 */
#if defined (SOC_J784S4)
        { {    5U,    1U,  4U}, {    5U,    1U,  5U} },  /* C7x-4 */
#endif
        { {    5U,    1U,  7U}, {    5U,    1U,  6U} }   /* A72-vm1 */
    },
#if defined (SOC_J784S4)
    /* Host Processor - c7x_4	*/
    {
        { {    5U,    2U,  3U}, {    5U,    2U,  2U} },  /* A72-vm0 */
        { {   17U,    1U,  6U}, {   12U, 0xFFU,  9U} },  /* mcu-r5f0 */
        { {   17U,    1U,  7U}, {   13U, 0xFFU,  9U} },  /* mcu-r5f1 */
        { {   17U,    0U,  0U}, {    6U, 0xFFU,  9U} },  /* main-r5f0 */
        { {   17U,    0U,  1U}, {    7U, 0xFFU,  9U} },  /* main-r5f1 */
        { {   17U,    0U,  2U}, {    8U, 0xFFU,  9U} },  /* main-r5f2 */
        { {   17U,    0U,  3U}, {    9U, 0xFFU,  9U} },  /* main-r5f3 */
        { {   17U,    0U,  4U}, {   10U, 0xFFU,  9U} },  /* main-r5f4 */
        { {   17U,    1U,  5U}, {   11U, 0xFFU,  9U} },  /* main-r5f5 */
        { {   17U,    1U,  8U}, {   14U, 0xFFU,  9U} },  /* C7x-1 */
        { {   17U,    1U,  9U}, {   15U, 0xFFU,  9U} },  /* C7x-2 */
        { {    5U,    2U,  5U}, {    5U,    2U,  4U} },  /* C7x-3 */
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,  0U} },  /* Self - C7x-4 */
        { {    5U,    2U,  9U}, {    5U,    2U,  8U} }   /* A72-vm1 */
    },
#endif
    /* Host Processor - A72-vm1	*/
    {
        { {    0U,    3U, 11U}, {    0U,    3U, 10U} },  /* A72-vm0 */
        { {    0U,    3U,  6U}, {    0U,    3U,  7U} },  /* mcu-r5f0 */
        { {    0U,    3U,  8U}, {    0U,    3U,  9U} },  /* mcu-r5f1 */
        { {    1U,    3U,  6U}, {    1U,    3U,  7U} },  /* main-r5f0 */
        { {    1U,    3U,  8U}, {    1U,    3U,  9U} },  /* main-r5f1 */
        { {    2U,    3U,  6U}, {    2U,    3U,  7U} },  /* main-r5f2 */
        { {    2U,    3U,  8U}, {    2U,    3U,  9U} },  /* main-r5f3 */
        { {    3U,    3U,  6U}, {    3U,    3U,  7U} },  /* main-r5f4 */
        { {    3U,    3U,  8U}, {    3U,    3U,  9U} },  /* main-r5f5 */
        { {    4U,    3U,  6U}, {    4U,    3U,  7U} },  /* C7x-1 */
        { {    4U,    3U,  8U}, {    4U,    3U,  9U} },  /* C7x-2 */
        { {    5U,    3U,  6U}, {    5U,    3U,  7U} },  /* C7x-3 */
#if defined (SOC_J784S4)
        { {    5U,    3U,  8U}, {    5U,    3U,  9U} },  /* C7x-4 */
#endif
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,  0U} }   /* Self - A72-vm1 */
    }
};

static int32_t Ipc_sciclientIrqTranslate(uint16_t coreId, uint32_t eventId,
        uint16_t *procIrq);


int32_t Ipc_getMailboxInfoTx(uint32_t selfId, uint32_t remoteId,
                 uint32_t *clusterId, uint32_t *userId, uint32_t *queueId)
{
    int32_t retVal = -1;

    if( (selfId < IPC_MAX_PROCS) &&
        (remoteId < IPC_MAX_PROCS))
    {
        Ipc_MailboxInfo   *pMailboxInfo = &g_IPC_MailboxInfo[selfId][remoteId];

        *clusterId = pMailboxInfo->tx.cluster;
        *userId    = pMailboxInfo->tx.user;
        *queueId   = pMailboxInfo->tx.fifo;
        retVal = 0;
    }

    return retVal;
}

int32_t Ipc_getMailboxInfoRx(uint32_t selfId, uint32_t remoteId,
                 uint32_t *clusterId, uint32_t *userId, uint32_t *queueId)
{
    int32_t retVal = -1;

    if( (selfId < IPC_MAX_PROCS) &&
        (remoteId < IPC_MAX_PROCS))
    {
        Ipc_MailboxInfo   *pMailboxInfo = &g_IPC_MailboxInfo[selfId][remoteId];

        *clusterId = pMailboxInfo->rx.cluster;
        *userId    = pMailboxInfo->rx.user;
        *queueId   = pMailboxInfo->rx.fifo;
        retVal = 0;
    }

    return retVal;

}

uintptr_t Ipc_getMailboxBaseAddr(uint32_t clusterId)
{
    uintptr_t baseAddr = 0x00000000U;

    if( clusterId < IPC_MAILBOX_CLUSTER_CNT)
    {
        baseAddr = g_IPC_Mailbox_BasePhyAddr[clusterId];

#if defined(BUILD_MPU1_0) && defined(QNX_OS)
        if (g_IPC_Mailbox_BaseVirtAddr[clusterId] == 0)
        {
            g_IPC_Mailbox_BaseVirtAddr[clusterId] =
                IpcUtils_getMemoryAddress(baseAddr, MAILBOX_REG_SIZE);
        }
        baseAddr = g_IPC_Mailbox_BaseVirtAddr[clusterId];
#endif
    }

    return baseAddr;
}

uint32_t Ipc_getNavss512MailboxInputIntr(uint32_t clusterId, uint32_t userId)
{
    uint32_t   mailboxIntrNum = 0U;

    if( (clusterId != MAILBOX_CLUSTER_INVALID) &&
        (clusterId < IPC_MAILBOX_CLUSTER_CNT)  &&
        (userId != MAILBOX_USER_INVALID)       &&
        (userId < IPC_MAILBOX_USER_CNT))
    {
        mailboxIntrNum = (uint32_t)(g_Navss512MbInput[clusterId] + userId);
    }
    return mailboxIntrNum;
}

int32_t Ipc_setCoreEventId(uint32_t selfId, Ipc_MbConfig* cfg, uint32_t intrCnt)
{
    int32_t    retVal          = IPC_SOK;
    uint32_t   outIntrBaseNum  = 0;
    uint32_t   outIntrNum      = 0;
    uint32_t   vimEventBaseNum = 0;
    uint32_t   vimEventNum     = 0;
    uint16_t   proc_irq        = 0;

    /*
     * static variable used to store the base and count of
     * Interrupt Router o/p # allocated for the core.
     * In subsequent call, it uses the offset of intrCnt from base
     */
    static uint16_t   start    = 0;
    static uint16_t   range    = 0;
    uint16_t   offset   = 0;

    /* Get available CorePack IRQ number from DMSC */
    if( (start == 0U) && (range == 0U))
    {
        /* Query the Interrupt Router o/p # alloacted for the core. */
        retVal = Ipc_getIntNumRange(selfId, &start, &range);
    }

    if((retVal == IPC_SOK) && (range >= 1U))
    {
        /* Allocate the last 6 interrupts for IPC. Note that the IR allocation is
         * static so this needs to be carefully set. Currently first interrupt is
         * used by UDMA and middle one's are used by other modules like CPSW9G so
         * we are using last 6 as a safe option.
         */
        if(range >= IPC_INTERRUPT_OFFSET)
        {
            offset = IPC_INTERRUPT_OFFSET;
        }
        else
        {
            offset = range;
        }
        outIntrBaseNum = ((uint32_t)start + (uint32_t)range) - (uint32_t)offset;
        outIntrNum = (uint32_t)(outIntrBaseNum + intrCnt);

        /* Translate to CorePack IRQ number */
        /* Translation must happen after this offset */
        retVal = Ipc_sciclientIrqTranslate((uint16_t)selfId, outIntrBaseNum,
                                           &proc_irq);
        if (CSL_PASS == retVal)
        {
            vimEventBaseNum = proc_irq;
        }

        /* Translation must happen after this offset */
        proc_irq = 0;
        retVal = Ipc_sciclientIrqTranslate((uint16_t)selfId, outIntrNum,
                                           &proc_irq);
        if (CSL_PASS == retVal)
        {
            vimEventNum = proc_irq;
        }

    }
    else
    {
        retVal = IPC_EFAIL;
    }

    cfg->outputIntrNum = outIntrNum;
    cfg->eventId       = vimEventNum;
    cfg->eventIdBase   = vimEventBaseNum;

    return retVal;
}


int32_t Ipc_getMailboxIntrRouterCfg(uint32_t selfId, uint32_t clusterId,
        uint32_t userId, Ipc_MbConfig* cfg, uint32_t cnt)
{
    int32_t    retVal         = IPC_SOK;
    uint32_t   mailboxIntrNum = 0;

    /* Get Navss512 input interrupt number for mailbox */
    mailboxIntrNum = Ipc_getNavss512MailboxInputIntr(clusterId, userId);

    cfg->inputIntrNum  = mailboxIntrNum;
    cfg->priority      = 1U;
    retVal = Ipc_setCoreEventId(selfId, cfg, cnt);

    return retVal;
}

const char* Ipc_getCoreName(uint32_t procId)
{
    char*     p = (char*)0;
    uint32_t id = procId;

    if(id < IPC_MAX_PROCS)
    {
       p = g_Ipc_mp_procInfo[id].name;
    }
    return p;
}

#if defined(BUILD_C7X)
uint32_t Ipc_configClecRouter(uint32_t corePackEvent, uint32_t corePackEventBase)
{
    uint32_t              input;
    CSL_ClecEventConfig   cfgClec;
    CSL_CLEC_EVTRegs     *clecBaseAddr = (CSL_CLEC_EVTRegs*)C7X_CLEC_BASE_ADDR;
    uint32_t              corepackIrq;

   /* Even though same CLEC is shared b/w all C7x cores, CLEC can broadcast the
    * event to any C7x core and CPU IRQ(corepackIrq) is core specific.
    * Hence same Mailbox Interrupt offset can be used for both C7x cores.  */
    corepackIrq = (uint32_t)(g_ipc_mBoxCnt + IPC_C7X_MBINTR_OFFSET);

   /* corePackEvent is derived from the NAVSS IR o/p range returned from BoardCfg,
    * based on core specific allocation. And this is different for each C7x.
    * Hence same C7X_CLEC_OFFSET can be used for both C7x cores. */
    input = (uint32_t)(corePackEvent + C7X_CLEC_OFFSET);

    /* Configure CLEC */
    cfgClec.secureClaimEnable = FALSE;
    cfgClec.evtSendEnable     = TRUE;
#if defined(BUILD_C7X_1)
    cfgClec.rtMap             = CSL_CLEC_RTMAP_CPU_4;
#elif defined(BUILD_C7X_2)
    cfgClec.rtMap             = CSL_CLEC_RTMAP_CPU_5;
#elif defined(BUILD_C7X_3)
    cfgClec.rtMap             = CSL_CLEC_RTMAP_CPU_6;
#elif defined(BUILD_C7X_4)
    cfgClec.rtMap             = CSL_CLEC_RTMAP_CPU_7;
#endif
    cfgClec.extEvtNum         = 0U;
    cfgClec.c7xEvtNum         = corepackIrq;
    CSL_clecConfigEvent(clecBaseAddr, input, &cfgClec);

    return corepackIrq;
}
#endif

/* Indexed list of dst ids */
static const uint16_t map_dst_id[] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_GIC500SS,
    TISCI_DEV_MCU_R5FSS0_CORE0,
    TISCI_DEV_MCU_R5FSS0_CORE1,
    TISCI_DEV_R5FSS0_CORE0,
    TISCI_DEV_R5FSS0_CORE1,
    TISCI_DEV_R5FSS1_CORE0,
    TISCI_DEV_R5FSS1_CORE1,
    TISCI_DEV_R5FSS2_CORE0,
    TISCI_DEV_R5FSS2_CORE1,
    TISCI_DEV_COMPUTE_CLUSTER0_CLEC,
    TISCI_DEV_COMPUTE_CLUSTER0_CLEC,
    TISCI_DEV_COMPUTE_CLUSTER0_CLEC,
    TISCI_DEV_COMPUTE_CLUSTER0_CLEC
};

/* Indexed list of src ids */
static const uint16_t map_src_id[] =
{
    TISCI_DEV_NAVSS0_MAILBOX_0,
    TISCI_DEV_NAVSS0_MAILBOX_1,
    TISCI_DEV_NAVSS0_MAILBOX_2,
    TISCI_DEV_NAVSS0_MAILBOX_3,
    TISCI_DEV_NAVSS0_MAILBOX_4,
    TISCI_DEV_NAVSS0_MAILBOX_5,
    TISCI_DEV_NAVSS0_MAILBOX_6,
    TISCI_DEV_NAVSS0_MAILBOX_7,
    TISCI_DEV_NAVSS0_MAILBOX_8,
    TISCI_DEV_NAVSS0_MAILBOX_9,
    TISCI_DEV_NAVSS0_MAILBOX_10,
    TISCI_DEV_NAVSS0_MAILBOX_11,
    TISCI_DEV_NAVSS0_MAILBOX1_0,
    TISCI_DEV_NAVSS0_MAILBOX1_1,
    TISCI_DEV_NAVSS0_MAILBOX1_2,
    TISCI_DEV_NAVSS0_MAILBOX1_3,
    TISCI_DEV_NAVSS0_MAILBOX1_4,
    TISCI_DEV_NAVSS0_MAILBOX1_5
};

/* Indexed list of req type */
static const uint16_t req_type[] =
{
    /* NOTE: This list should match the Core index */
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0,
    TISCI_DEV_NAVSS0_INTR_0
};

/* Indexed list of req subtype */
static const uint16_t req_subtype[] =
{
    /* NOTE: This list should match the Core index */
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT
};

static int32_t Ipc_sciclientIrqTranslate(uint16_t coreId, uint32_t eventId,
        uint16_t *procIrq)
{
    return Sciclient_rmIrqTranslateIrOutput(req_type[coreId],
                                            (uint16_t)eventId,
                                            map_dst_id[coreId],
                                            procIrq);
}

int32_t Ipc_sciclientIrqRelease(uint16_t coreId, uint32_t clusterId,
        uint32_t userId, uint32_t intNumber)
{
    int32_t                               retVal = IPC_SOK;
    struct tisci_msg_rm_irq_release_req   rmIrqRel;

    rmIrqRel.ia_id                  = 0U;
    rmIrqRel.vint                   = 0U;
    rmIrqRel.global_event           = 0U;
    rmIrqRel.vint_status_bit_index  = 0U;

    rmIrqRel.valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID |
                              TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
    rmIrqRel.src_id         = map_src_id[clusterId];
    rmIrqRel.src_index      = (uint16_t)userId;
    rmIrqRel.dst_id         = (uint16_t)map_dst_id[coreId];
    rmIrqRel.dst_host_irq   = (uint16_t)intNumber;
    rmIrqRel.secondary_host = (uint8_t)TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

    retVal = Sciclient_rmIrqRelease(&rmIrqRel, IPC_SCICLIENT_TIMEOUT);

    return retVal;
}

int32_t Ipc_sciclientIrqSet(uint16_t coreId, uint32_t clusterId,
        uint32_t userId, uint32_t intNumber)
{
    int32_t                           retVal = IPC_SOK;
    struct tisci_msg_rm_irq_set_req   rmIrqReq;
    struct tisci_msg_rm_irq_set_resp  rmIrqResp;

    memset(&rmIrqResp, 0, sizeof(rmIrqResp));
    rmIrqReq.ia_id                  = 0U;
    rmIrqReq.vint                   = 0U;
    rmIrqReq.global_event           = 0U;
    rmIrqReq.vint_status_bit_index  = 0U;
    rmIrqReq.valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID |
                              TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
    rmIrqReq.src_id         = map_src_id[clusterId];
    rmIrqReq.src_index      = (uint16_t)userId;
    rmIrqReq.dst_id         = (uint16_t)map_dst_id[coreId];
    rmIrqReq.dst_host_irq   = (uint16_t)intNumber;
    rmIrqReq.secondary_host = (uint8_t)TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

    /* Config event */
    retVal = Sciclient_rmIrqSet(&rmIrqReq, &rmIrqResp, IPC_SCICLIENT_TIMEOUT);

    return retVal;
}

int32_t Ipc_getIntNumRange(uint32_t coreIndex,
        uint16_t *rangeStartP, uint16_t *rangeNumP)
{
    int32_t                                     retVal = IPC_SOK;
    struct tisci_msg_rm_get_resource_range_resp res;
    struct tisci_msg_rm_get_resource_range_req  req;

    memset(&res, 0, sizeof(res));
    memset(&req, 0, sizeof(req));

    req.type           = req_type[coreIndex];
    req.subtype        = (uint8_t)req_subtype[coreIndex];
    req.secondary_host = (uint8_t)TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

    res.range_num = 0;
    res.range_start = 0;

    /* Get interrupt number range */
    retVal =  Sciclient_rmGetResourceRange(
                &req,
                &res,
                IPC_SCICLIENT_TIMEOUT);
    if ((CSL_PASS != retVal) || (res.range_num == 0U)) {
        /* Try with HOST_ID_ALL */
        req.type           = req_type[coreIndex];
        req.subtype        = (uint8_t)req_subtype[coreIndex];
        req.secondary_host = TISCI_HOST_ID_ALL;

        retVal = Sciclient_rmGetResourceRange(
                &req,
                &res,
                IPC_SCICLIENT_TIMEOUT);
    }
    if (CSL_PASS == retVal)
    {
        *rangeStartP = res.range_start;
        *rangeNumP = res.range_num;
    }
    return retVal;
}

uint32_t Ipc_getCoreId(void)
{
    uint32_t selfId =  IPC_INVALID_PROCID;

#if defined(BUILD_MPU1_0)
    selfId = IPC_MPU1_0;
#elif defined(BUILD_MCU1_0)
    selfId = IPC_MCU1_0;
#elif defined(BUILD_MCU1_1)
    selfId = IPC_MCU1_1;
#elif defined(BUILD_MCU2_0)
    selfId = IPC_MCU2_0;
#elif defined(BUILD_MCU2_1)
    selfId = IPC_MCU2_1;
#elif defined(BUILD_MCU3_0)
    selfId = IPC_MCU3_0;
#elif defined(BUILD_MCU3_1)
    selfId = IPC_MCU3_1;
#elif defined(BUILD_MCU4_0)
    selfId = IPC_MCU4_0;
#elif defined(BUILD_MCU4_1)
    selfId = IPC_MCU4_1;
#elif defined(BUILD_C7X_1)
    selfId = IPC_C7X_1;
#elif defined(BUILD_C7X_2)
    selfId = IPC_C7X_2;
#elif defined(BUILD_C7X_3)
    selfId = IPC_C7X_3;
#elif defined(BUILD_C7X_4)
    selfId = IPC_C7X_4;
#else
#error "Unsupported core Id"
#endif

    return (selfId);
}

uint32_t Ipc_isCacheCoherent(void)
{
    uint32_t isCacheCoherent;

#if defined (BUILD_MPU1_0) || defined(BUILD_C7X)
    isCacheCoherent = TRUE;
#else
    isCacheCoherent = FALSE;
#endif

    return (isCacheCoherent);
}
