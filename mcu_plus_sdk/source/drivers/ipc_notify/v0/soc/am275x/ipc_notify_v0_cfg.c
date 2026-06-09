/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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

#include <drivers/ipc_notify/v0/ipc_notify_v0.h>

/* values to use when a mailbox config is not used in gIpcNotifyMailboxConfig */
#define MAILBOX_UNUSED      0U, 0U, 0U

/* All mailbox base addresses */
uint32_t gIpcNotifyMailboxBaseAddr[IPC_NOTIFY_MAILBOX_MAX_INSTANCES + 1U] = {
    (uint32_t) CSL_MAILBOX0_REGS0_BASE,
    (uint32_t) CSL_MAILBOX0_REGS1_BASE,
    (uint32_t) CSL_MAILBOX0_REGS2_BASE,
    (uint32_t) CSL_MAILBOX0_REGS3_BASE,
    (uint32_t) CSL_MAILBOX0_REGS4_BASE,
    (uint32_t) CSL_MAILBOX0_REGS5_BASE,
    (uint32_t) CSL_MAILBOX0_REGS6_BASE,
    (uint32_t) CSL_MAILBOX0_REGS7_BASE,
    (uint32_t) 0x0U /* MUST be terminated by 0x0 */
};


/* Pre-defined mailbox config to allow any CPU to send and receive messages from any CPU on this SOC
 *
 * Each element consists of [maibox base addr, hw fifo, user id]
 * These assignments need to be in sync with gIpcNotifyInterruptConfig* done later below,
 *
 * This is a 2D array
 * - 1st indexed by source core ID
 * - then indexed by destination core ID
 *
 *
 * The following table shows the interrrupt connectivity of clusters to all the cores
 * --------------------------------------------------------------------------------------------------------------
 *   Core          | Mailbox 0 | Mailbox 1 | Mailbox 2 | Mailbox 3 | Mailbox 4 | Mailbox 5 | Mailbox 6 | Mailbox 7
 * --------------------------------------------------------------------------------------------------------------
 *   WKUP R5       |           |           |           |           |           |     0     |     0     |    0
 * --------------------------------------------------------------------------------------------------------------
 *   R5SS0 Core 0  |     0     |     0     |           |     0     |           |     1     |           |
 * --------------------------------------------------------------------------------------------------------------
 *   R5SS0 Core 1  |     1     |           |     0     |     1     |           |           |     1     |
 * --------------------------------------------------------------------------------------------------------------
 *   R5SS1 Core 0  |     2     |     1     |           |           |     0     |           |     2     |
 * --------------------------------------------------------------------------------------------------------------
 *   R5SS1 Core 1  |     3     |           |     1     |           |     1     |     2     |           |
 * --------------------------------------------------------------------------------------------------------------
 *   C7xv 0        |           |     2     |     2     |     2     |     2     |           |           |     1
 * --------------------------------------------------------------------------------------------------------------
 *   C7xv 1        |           |     3     |     3     |     3     |     3     |           |           |     2
 * --------------------------------------------------------------------------------------------------------------
 *   HSM M4F       |           |           |           |           |           |     3     |     3     |     3
 * --------------------------------------------------------------------------------------------------------------
 *
 *
 *
 * The table below shows the cluster and FIFO usage
 * ---------------------------------------------
 *  Cluster | FIFO | UserID |        IPC
 * ---------------------------------------------
 *    0     |  0   |    0   | R5F0-1   -> R5F0-0
 *    0     |  1   |    0   | R5F1-0   -> R5F0-0
 *    1     |  0   |    0   | R5F1-1   -> R5F0-0
 *    3     |  0   |    0   | C7x-0    -> R5F0-0
 *    5     |  0   |    1   | C7x-1    -> R5F0-0
 *    1     |  1   |    0   | WKUP R5F -> R5F0-0
 * -----------------------------------------------
 *    0     |  2   |    1   | R5F0-0   -> R5F0-1
 *    0     |  3   |    1   | R5F1-0   -> R5F0-1
 *    2     |  0   |    0   | R5F1-1   -> R5F0-1
 *    3     |  1   |    1   | C7x-0    -> R5F0-1
 *    6     |  0   |    1   | C7x-1    -> R5F0-1
 *    2     |  1   |    0   | WKUP R5F -> R5F0-1
 * -----------------------------------------------
 *    0     |  4   |    2   | R5F0-0   -> R5F1-0
 *    0     |  5   |    2   | R5F0-1   -> R5F1-0
 *    1     |  2   |    1   | R5F1-1   -> R5F1-0
 *    4     |  0   |    0   | C7x-0    -> R5F1-0
 *    6     |  1   |    2   | C7x-1    -> R5F1-0
 *    1     |  3   |    1   | WKUP R5F -> R5F1-0
 * -----------------------------------------------
 *    0     |  6   |    3   | R5F0-0   -> R5F1-1
 *    0     |  7   |    3   | R5F0-1   -> R5F1-1
 *    2     |  2   |    1   | R5F1-0   -> R5F1-1
 *    4     |  1   |    1   | C7x-0    -> R5F1-1
 *    5     |  1   |    2   | C7x-1    -> R5F1-1
 *    2     |  3   |    1   | WKUP R5F -> R5F1-1
 * -----------------------------------------------
 *    1     |  4   |    2   | R5F0-0   -> C7x-0
 *    2     |  4   |    2   | R5F0-1   -> C7x-0
 *    3     |  2   |    2   | R5F1-0   -> C7x-0
 *    4     |  2   |    2   | R5F1-1   -> C7x-0
 *    7     |  0   |    1   | C7x-1    -> C7x-0
 *    4     |  3   |    2   | WKUP R5F -> C7x-0
 * -----------------------------------------------
 *    1     |  5   |    3   | R5F0-0   -> C7x-1
 *    2     |  5   |    3   | R5F0-1   -> C7x-1
 *    3     |  3   |    3   | R5F1-0   -> C7x-1
 *    4     |  4   |    3   | R5F1-1   -> C7x-1
 *    7     |  1   |    2   | C7x-0    -> C7x-1
 *    4     |  5   |    3   | WKUP R5F -> C7x-1
 * -----------------------------------------------
 *    5     |  2   |    0   | R5F0-0   -> WKUP R5F
 *    5     |  3   |    0   | R5F0-1   -> WKUP R5F
 *    6     |  2   |    0   | R5F1-0   -> WKUP R5F
 *    6     |  3   |    0   | R5F1-1   -> WKUP R5F
 *    7     |  2   |    0   | C7x-0    -> WKUP R5F
 *    7     |  3   |    0   | C7x-1    -> WKUP R5F
 *
 */
IpcNotify_MailboxConfig gIpcNotifyMailboxConfig[CSL_CORE_ID_MAX][CSL_CORE_ID_MAX] =
{
    /* from WKUP_R5FSS0_0 */
    {
        { /* to WKUP_R5FSS0_0 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS0_0 */
            1U, 1U, 0U
        },
        { /* to R5FSS0_1 */
            2U, 1U, 0U
        },
        { /* to R5FSS1_0 */
            1U, 3U, 1U
        },
        { /* to R5FSS1_1 */
            2U, 3U, 1U
        },
        { /* to C75SS0_0 */
            4U, 3U, 2U
        },
        { /* to C75SS1_0 */
            4U, 5U, 3U
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from R5FSS0_0 */
    {
        { /* to WKUP_R5FSS0_0 */
            5U, 2U, 0U
        },
        { /* to R5FSS0_0 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS0_1 */
            0U, 2U, 1U
        },
        { /* to R5FSS1_0 */
            0U, 4U, 2U
        },
        { /* to R5FSS1_1 */
            0U, 6U, 3U
        },
        { /* to C75SS0_0 */
            1U, 4U, 2U
        },
        { /* to C75SS1_0 */
            1U, 5U, 3U
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from R5FSS0_1 */
    {
        { /* to WKUP_R5FSS0_0 */
            5U, 3U, 0U
        },
        { /* to R5FSS0_0 */
            0U, 0U, 0U
        },
        { /* to R5FSS0_1 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS1_0 */
            0U, 5U, 2U
        },
        { /* to R5FSS1_1 */
            0U, 7U, 3U
        },
        { /* to C75SS0_0 */
            2U, 4U, 2U
        },
        { /* to C75SS1_0 */
            2U, 5U, 3U
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from R5FSS1_0 */
    {
        { /* to WKUP_R5FSS0_0 */
            6U, 2U, 0U
        },
        { /* to R5FSS0_0 */
            0U, 1U, 0U
        },
        { /* to R5FSS0_1 */
            0U, 3U, 1U
        },
        { /* to R5FSS1_0 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS1_1 */
            2U, 2U, 1U
        },
        { /* to C75SS0_0 */
            3U, 2U, 2U
        },
        { /* to C75SS1_0 */
            3U, 3U, 3U
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from R5FSS1_1 */
    {
        { /* to WKUP_R5FSS0_0 */
            6U, 3U, 0U
        },
        { /* to R5FSS0_0 */
            1U, 0U, 0U
        },
        { /* to R5FSS0_1 */
            2U, 0U, 0U
        },
        { /* to R5FSS1_0 */
            1U, 2U, 1U
        },
        { /* to R5FSS1_1 */
            MAILBOX_UNUSED
        },
        { /* to C75SS0_0 */
            4U, 2U, 2U
        },
        { /* to C75SS1_0 */
            4U, 4U, 3U
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from C75SS0_0 */
    {
        { /* to WKUP_R5FSS0_0 */
            7U, 2U, 0U
        },
        { /* to R5FSS0_0 */
            3U, 0U, 0U
        },
        { /* to R5FSS0_1 */
            3U, 1U, 1U
        },
        { /* to R5FSS1_0 */
            4U, 0U, 0U
        },
        { /* to R5FSS1_1 */
            4U, 1U, 1U
        },
        { /* to C75SS0_0 */
            MAILBOX_UNUSED
        },
        { /* to C75SS1_0 */
            7U, 1U, 2U
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from C75SS1_0 */
    {
        { /* to WKUP_R5FSS0_0 */
            7U, 3U, 0U
        },
        { /* to R5FSS0_0 */
            5U, 0U, 1U
        },
        { /* to R5FSS0_1 */
            6U, 0U, 1U
        },
        { /* to R5FSS1_0 */
            6U, 1U, 2U
        },
        { /* to R5FSS1_1 */
            5U, 1U, 2U
        },
        { /* to C75SS0_0 */
            7U, 0U, 1U
        },
        { /* to C75SS1_0 */
            MAILBOX_UNUSED
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
    /* from HSM_M4FSS0_0 */
    {
        { /* to WKUP_R5FSS0_0 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS0_0 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS0_1 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS1_0 */
            MAILBOX_UNUSED
        },
        { /* to R5FSS1_1 */
            MAILBOX_UNUSED
        },
        { /* to C75SS0_0 */
            MAILBOX_UNUSED
        },
        { /* to C75SS1_0 */
            MAILBOX_UNUSED
        },
        { /* to HSM_M4FSS0_0 */
            MAILBOX_UNUSED
        },
    },
};

/* Interrupt config for WKUP_R5FSS0_0 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_WKUP_R5FSS0_0_NUM   (3u)
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_wkup_r5fss0_0[IPC_NOFTIY_INTERRUPT_CONFIG_WKUP_R5FSS0_0_NUM] = {
    {
        .intNum = CSLR_WKUP_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_5_MAILBOX_CLUSTER_PEND_0,   /* interrupt line on WKUP_R5FSS0_0 CPU */
        .eventId = 0U,   /* not used */
        .numCores = 2U,  /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = { /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_0,
            CSL_CORE_ID_R5FSS0_1,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_WKUP_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_6_MAILBOX_CLUSTER_PEND_0,   /* interrupt line on WKUP_R5FSS0_0 CPU */
        .eventId = 0U,   /* not used */
        .numCores = 2U,  /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = { /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_0,
            CSL_CORE_ID_R5FSS1_1,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_WKUP_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_7_MAILBOX_CLUSTER_PEND_0,   /* interrupt line on WKUP_R5FSS0_0 CPU */
        .eventId = 0U,   /* not used */
        .numCores = 2U,  /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = { /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS0_0,
            CSL_CORE_ID_C75SS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    }
};
uint32_t gIpcNotifyInterruptConfigNum_wkup_r5fss0_0 = IPC_NOFTIY_INTERRUPT_CONFIG_WKUP_R5FSS0_0_NUM;

/* Interrupt config for R5FSS0_0 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS0_0_NUM   (4u)
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_r5fss0_0[IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS0_0_NUM] = {
    {
        .intNum = CSLR_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_0_MAILBOX_CLUSTER_PEND_0,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_1,
            CSL_CORE_ID_R5FSS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_1_MAILBOX_CLUSTER_PEND_0,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_1,
            CSL_CORE_ID_WKUP_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_3_MAILBOX_CLUSTER_PEND_0,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_5_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
};
uint32_t gIpcNotifyInterruptConfigNum_r5fss0_0 = IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS0_0_NUM;

/* Interrupt config for R5FSS0_1 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS0_1_NUM   (4u)
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_r5fss0_1[IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS0_1_NUM] = {
    {
        .intNum = CSLR_R5FSS0_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_0_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_0,
            CSL_CORE_ID_R5FSS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS0_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_2_MAILBOX_CLUSTER_PEND_0,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_1,
            CSL_CORE_ID_WKUP_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS0_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_3_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS0_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_6_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
};
uint32_t gIpcNotifyInterruptConfigNum_r5fss0_1 = IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS0_0_NUM;

/* Interrupt config for R5FSS1_0 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS1_0_NUM   (4u)
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_r5fss1_0[IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS1_0_NUM] = {
    {
        .intNum = CSLR_R5FSS1_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_0_MAILBOX_CLUSTER_PEND_2,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_0,
            CSL_CORE_ID_R5FSS0_1,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS1_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_1_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_1,
            CSL_CORE_ID_WKUP_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS1_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_4_MAILBOX_CLUSTER_PEND_0,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS1_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_6_MAILBOX_CLUSTER_PEND_2,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
};
uint32_t gIpcNotifyInterruptConfigNum_r5fss1_0 = IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS1_0_NUM;


/* Interrupt config for R5FSS1_1 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS1_1_NUM   (4u)
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_r5fss1_1[IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS1_1_NUM] = {
    {
        .intNum = CSLR_R5FSS1_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_0_MAILBOX_CLUSTER_PEND_3,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_0,
            CSL_CORE_ID_R5FSS0_1,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS1_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_2_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_0,
            CSL_CORE_ID_WKUP_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS1_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_4_MAILBOX_CLUSTER_PEND_1,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = CSLR_R5FSS1_CORE1_INTR_MAILBOX0_MAILBOX_CLUSTER_5_MAILBOX_CLUSTER_PEND_2,
        .eventId = 0U,    /* not used */
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
};
uint32_t gIpcNotifyInterruptConfigNum_r5fss1_1 = IPC_NOFTIY_INTERRUPT_CONFIG_R5FSS1_0_NUM;

/* Interrupt config for C75SS0_0 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_C75SS0_0_NUM   (5u)
#define CLEC_SOC_INT_START                         (192u)   /* (192 - soc interrupt start) */
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_c75ss0_0[IPC_NOFTIY_INTERRUPT_CONFIG_C75SS0_0_NUM] = {
    {
        .intNum = 7,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V0_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_1_MAILBOX_CLUSTER_PEND_2,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 8,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V0_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_2_MAILBOX_CLUSTER_PEND_2,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_1,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 9,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V0_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_3_MAILBOX_CLUSTER_PEND_2,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 10,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V0_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_4_MAILBOX_CLUSTER_PEND_2,
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_1,
            CSL_CORE_ID_WKUP_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 11,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V0_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_7_MAILBOX_CLUSTER_PEND_1,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
};
uint32_t gIpcNotifyInterruptConfigNum_c75ss0_0 = IPC_NOFTIY_INTERRUPT_CONFIG_C75SS0_0_NUM;

/* Interrupt config for C75SS0_0 */
#define IPC_NOFTIY_INTERRUPT_CONFIG_C75SS1_0_NUM   (5u)
IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_c75ss1_0[IPC_NOFTIY_INTERRUPT_CONFIG_C75SS1_0_NUM] = {
    {
        .intNum = 7,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_1_MAILBOX_CLUSTER_PEND_3,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 8,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_2_MAILBOX_CLUSTER_PEND_3,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS0_1,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 9,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_3_MAILBOX_CLUSTER_PEND_3,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 10,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_4_MAILBOX_CLUSTER_PEND_3,
        .numCores = 2U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_R5FSS1_1,
            CSL_CORE_ID_WKUP_R5FSS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
    {
        .intNum = 11,
        .eventId = CLEC_SOC_INT_START +
                   CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_7_MAILBOX_CLUSTER_PEND_2,
        .numCores = 1U,   /* number of cores that send messages which tied to this interrupt line */
        .coreIdList = {   /* core ID's tied to this interrupt line */
            CSL_CORE_ID_C75SS0_0,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID,
            CSL_CORE_ID_INVALID
        },
    },
};
uint32_t gIpcNotifyInterruptConfigNum_c75ss1_0 = IPC_NOFTIY_INTERRUPT_CONFIG_C75SS1_0_NUM;
