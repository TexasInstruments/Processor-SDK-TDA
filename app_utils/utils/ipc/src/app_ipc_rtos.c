/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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

#include <string.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/misc/include/app_misc.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/rtos/include/app_rtos.h>
#include <HwiP.h>

#if defined(PDK)
#include <ipc/ipc.h>
#include <osal.h>
#elif defined(MCU_SDK)
#include <Ipc_Notify.h>
#include <RPMessage.h>
#include <ClockP.h>
#include <utils/ipc/include/mcu_sdk_ipc.h>
#elif defined(MCU_PLUS_SDK)
#include <ipc_rpmsg.h>
#include <ipc_notify.h>
#include <CacheP.h>
#include <ClockP.h>
#include <soc.h>
#include <utils/ipc/include/mcu_sdk_ipc.h>
#if defined(THREADX)
#include <tx_port.h>
#endif
#endif

/* #define APP_IPC_DEBUG */

#define APP_IPC_MAX_TASK_NAME       (12u)
#define IPC_RPMESSAGE_OBJ_SIZE      (256u)
#define IPC_RPMESSAGE_MSG_SIZE      (496U + 32U)
#define IPC_RPMESSAGE_BUF_SIZE(n)   ((IPC_RPMESSAGE_MSG_SIZE*(n))+IPC_RPMESSAGE_OBJ_SIZE)

#if defined(PDK)
#define IPC_VRING_OBJ_SIZE          (256u)
#define APP_IPC_VQ_OBJ_MEM_SIZE     (IPC_MAX_PROCS*IPC_VRING_OBJ_SIZE)
static uint8_t g_app_vq_obj_mem[APP_IPC_VQ_OBJ_MEM_SIZE] __attribute__ ((aligned(1024)));

#define APP_IPC_RPMESSAGE_CTRL_PARAMS_NUM_BUF   (16u)
#define APP_IPC_RPMESSAGE_CTRL_PARAMS_BUF_SIZE  IPC_RPMESSAGE_BUF_SIZE(APP_IPC_RPMESSAGE_CTRL_PARAMS_NUM_BUF)
static uint8_t g_app_rpmessage_ctrl_params_buf[APP_IPC_RPMESSAGE_CTRL_PARAMS_BUF_SIZE] __attribute__ ((aligned(1024)));

#define APP_IPC_RPMESSAGE_RPMSG_TX_NUM_BUF   (16u)
#define APP_IPC_RPMESSAGE_RPMSG_TX_BUF_SIZE  IPC_RPMESSAGE_BUF_SIZE(APP_IPC_RPMESSAGE_RPMSG_TX_NUM_BUF)
static uint8_t g_app_rpmessage_rpmsg_tx_buf[APP_IPC_CPU_MAX][APP_IPC_RPMESSAGE_RPMSG_TX_BUF_SIZE] __attribute__ ((aligned(1024)));

#define APP_IPC_RPMESSAGE_RPMSG_RX_NUM_BUF   (256u)
#define APP_IPC_RPMESSAGE_RPMSG_RX_BUF_SIZE  IPC_RPMESSAGE_BUF_SIZE(APP_IPC_RPMESSAGE_RPMSG_RX_NUM_BUF)
static uint8_t g_app_rpmessage_rpmsg_rx_buf[APP_IPC_RPMESSAGE_RPMSG_RX_BUF_SIZE] __attribute__ ((aligned(1024)));
#endif

/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16KB
 * - AND stack assigned for task context is "size - 8KB"
 * - 8KB chunk for the stack area is used for interrupt handling in this task context
 */

#if defined(R5F) || defined(R52P) || defined(M55)
#define APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE     (4u*1024u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE   (4u*1024u)

#elif defined (C7X_FAMILY)

#if defined (SOC_AM62A)
#define APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE     (64u*1024u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE   (64u*1024u)
#define APP_LPM_SUSPEND_TASK_STACK_SIZE          (64u*1024u)
#else
#define APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE     (32u*1024u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE   (32u*1024u)
#endif

#else
#define APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE     (32u*1024u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE   (32u*1024u)
#endif

#if defined(THREADX)
#define APP_IPC_RPMESSAGE_RX_TASK_PRI          (TX_MAX_PRIORITIES-11)
#else
#define APP_IPC_RPMESSAGE_RX_TASK_PRI          (10u)
#endif

#if defined (SOC_AM62A)
#define APP_LPM_SUSPEND_TASK_PRI               (10U)
#endif

#if defined(R5F) || defined(R52P) || defined(M55)

#if defined(SAFERTOS)
#define APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT      APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE
#define APP_IPC_RPMESSAGE_CTRL_TASK_ALIGNMENT    APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE
#else
#define APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT      (8192u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_ALIGNMENT    (8192u)
#endif

#elif defined (C7X_FAMILY)
#if defined (SOC_AM62A)
#define APP_LPM_SUSPEND_TASK_ALIGNMENT           (8192u)
#endif
#define APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT      (8192u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_ALIGNMENT    (8192u)

#else
#define APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT      (8192u)
#define APP_IPC_RPMESSAGE_CTRL_TASK_ALIGNMENT    (8192u)
#endif

#define APP_IPC_HW_SPIN_LOCK_MAX        (256u)
#if defined(MCU_PLUS_SDK)
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x2A000000u)
#elif defined(MCU_SDK)
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x30020000u)
#elif defined(PDK)
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x30E00000u)
#endif
#define APP_IPC_HW_SPIN_LOCK_OFFSET(x)  ((uint32_t)0x800u + ((uint32_t)4u*(uint32_t)(x)))

static uintptr_t key;

static uint8_t g_app_rpmessage_rx_task_stack[APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT)))
    ;

#if (defined (SOC_AM62A) && !(defined (R5F)))
static uint8_t g_app_lpm_suspend_task_stack[APP_LPM_SUSPEND_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_LPM_SUSPEND_TASK_ALIGNMENT)))
    ;
#endif

#if defined(PDK)
static uint8_t g_app_rpmessage_ctrl_task_stack[APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_IPC_RPMESSAGE_CTRL_TASK_ALIGNMENT)))
    ;
#endif /* #if defined(PDK) */

typedef struct {

    app_ipc_init_prm_t prm;
    uint32_t rpmsg_tx_endpt[APP_IPC_CPU_MAX];
    app_ipc_notify_handler_f ipc_notify_handler;
    app_rtos_task_handle_t task_handle;

    #if defined(PDK)
    RPMessage_Handle rpmsg_tx_handle[APP_IPC_CPU_MAX];
    RPMessage_Handle rpmsg_rx_handle;
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_Object rpmsg_tx_handle[APP_IPC_CPU_MAX];
    RPMessage_Object rpmsg_rx_handle;
    #endif

    uint32_t task_stack_size;
    uint8_t *task_stack;
    uint32_t task_pri;
    uint8_t  rpmsg_rx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    char     task_name[APP_IPC_MAX_TASK_NAME];
} app_ipc_obj_t;

static app_ipc_obj_t g_app_ipc_obj;

#if defined (SOC_AM62A)
app_rtos_task_handle_t lpm_task_handle;
static app_rtos_semaphore_handle_t g_lpm_suspend_sem;
#endif

#if defined (SOC_J721E)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    IPC_MPU1_0,
    IPC_MCU1_0,
    IPC_MCU1_1,
    IPC_MCU2_0,
    IPC_MCU2_1,
    IPC_MCU3_0,
    IPC_MCU3_1,
    IPC_C66X_1,
    IPC_C66X_2,
    IPC_C7X_1
};

static uint32_t g_ipc_to_app_cpu_id[IPC_MAX_PROCS] =
{
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MCU1_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_C6x_1,
    APP_IPC_CPU_C6x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_INVALID
};
#endif

#if defined (SOC_J721S2)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    IPC_MPU1_0,
    IPC_MCU1_0,
    IPC_MCU1_1,
    IPC_MCU2_0,
    IPC_MCU2_1,
    IPC_MCU3_0,
    IPC_MCU3_1,
    IPC_C7X_1,
    IPC_C7X_2
};

static uint32_t g_ipc_to_app_cpu_id[IPC_MAX_PROCS] =
{
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MCU1_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_INVALID
};
#endif

#if defined (SOC_J784S4)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    IPC_MPU1_0,
    IPC_MCU1_0,
    IPC_MCU1_1,
    IPC_MCU2_0,
    IPC_MCU2_1,
    IPC_MCU3_0,
    IPC_MCU3_1,
    IPC_MCU4_0,
    IPC_MCU4_1,
    IPC_C7X_1,
    IPC_C7X_2,
    IPC_C7X_3,
    IPC_C7X_4
};

static uint32_t g_ipc_to_app_cpu_id[IPC_MAX_PROCS] =
{
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MCU1_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_MCU4_0,
    APP_IPC_CPU_MCU4_1,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4,
    APP_IPC_CPU_INVALID
};
#endif

#if defined (SOC_TDA54)
#define APP_MCUSDK_IPC_CPU_INVALID (0xFFFFFFFFU)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CORE_ID_A720_0,
    CORE_ID_MCU0,
    CORE_ID_MCU1,
    CORE_ID_MCU2,
    CORE_ID_MCU3,
    CORE_ID_MCU4,
    CORE_ID_RMCU0_0,
    CORE_ID_RMCU0_1,
    CORE_ID_RMCU1_0,
    CORE_ID_RMCU1_1,
    CORE_ID_RMCU2_0,
    CORE_ID_RMCU2_1,
    CORE_ID_DSP0,
    CORE_ID_DSP1,
    CORE_ID_DSP2,
    CORE_ID_DSP3,
};

static uint32_t g_ipc_to_app_cpu_id[CORE_ID_MAX] =
{
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_IPC_CPU_MCU0_M55,
    APP_IPC_CPU_MCU1_M55,
    APP_IPC_CPU_MCU2_M55,
    APP_IPC_CPU_MCU3_M55,
    APP_IPC_CPU_MCU4_M55,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4,
    APP_IPC_CPU_MPU1_0,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_IPC_CPU_RMCU0_0,
    APP_IPC_CPU_RMCU0_1,
    APP_IPC_CPU_RMCU1_0,
    APP_IPC_CPU_RMCU1_1,
    APP_IPC_CPU_RMCU2_0,
    APP_IPC_CPU_RMCU2_1
};
#endif

#if defined (SOC_J742S2)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    IPC_MPU1_0,
    IPC_MCU1_0,
    IPC_MCU1_1,
    IPC_MCU2_0,
    IPC_MCU2_1,
    IPC_MCU3_0,
    IPC_MCU3_1,
    IPC_MCU4_0,
    IPC_MCU4_1,
    IPC_C7X_1,
    IPC_C7X_2,
    IPC_C7X_3
};

static uint32_t g_ipc_to_app_cpu_id[IPC_MAX_PROCS] =
{
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MCU1_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_MCU4_0,
    APP_IPC_CPU_MCU4_1,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_INVALID
};
#endif

#if defined (SOC_AM62A)
#define APP_MCUSDK_IPC_CPU_INVALID (0xFFFFFFFFU)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_R5FSS0_0,
    CSL_CORE_ID_C75SS0_0,
#ifdef QNX
    CSL_CORE_ID_MCU_R5FSS0_0,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID
#endif
};

static uint32_t g_ipc_to_app_cpu_id[CSL_CORE_ID_MAX] =
{
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MPU1_0,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_IPC_CPU_C7x_1
};
#endif

#if defined (SOC_J722S)
#define APP_MCUSDK_IPC_CPU_INVALID (0xFFFFFFFFU)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_WKUP_R5FSS0_0,
    CSL_CORE_ID_MAIN_R5FSS0_0,
    CSL_CORE_ID_C75SS0_0,
    CSL_CORE_ID_C75SS1_0,
    CSL_CORE_ID_MCU_R5FSS0_0
};

static uint32_t g_ipc_to_app_cpu_id[CSL_CORE_ID_MAX] =
{
    APP_IPC_CPU_MCU_R5F,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_MPU1_0,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID
};
#endif

#if defined(SOC_AM62A)
/* Declare the variables required for graceful shutdown */
  volatile uint8_t gbShutdown;
  volatile uint8_t gbShutdownRemotecoreID;
  volatile uint8_t gbSuspended;
  volatile uint8_t gbSuspendRemotecoreID;
  static uint32_t gVring_id[APP_IPC_CPU_MAX][APP_IPC_CPU_MAX];
#endif

static int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj);
static void appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj);
#if defined(SOC_AM62A)
void appIpcRpmboxCallback(uint16_t remoteCoreId, uint16_t clientId, uint32_t msgValue, void *args);
#if !(defined (R5F))
static int32_t appLpmCreateSuspendTask(void);
static void appLpmSuspendTaskMain(void *arg0, void *arg1);
#endif
#endif

#if defined(PDK)
static void appIpcRpmsgRxHandler(RPMessage_Handle rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt);
#elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
static void appIpcRpmsgRxHandler(RPMessage_Object* rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt);
#endif
static void appIpcRpmsgRxTaskMain(void *arg0, void *arg1);

#if defined(PDK)
static void appIpcRpmsgRxHandler(RPMessage_Handle rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt)
#elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
static void appIpcRpmsgRxHandler(RPMessage_Object* rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt)
#endif
{
    uint32_t app_cpu_id, payload;
    app_ipc_obj_t *obj = arg;

    if( (src_cpu_id<IPC_MAX_PROCS) && /* TIOVX-1913- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR001 */
        (len == sizeof(payload)) ) /* TIOVX-1913- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR002 */
    {
        app_cpu_id = g_ipc_to_app_cpu_id[src_cpu_id];
        payload = *(uint32_t*)data;

        #ifdef APP_IPC_DEBUG
        appLogPrintf("IPC: RX: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
            Ipc_mpGetName(src_cpu_id),
            (uint32_t)src_endpt,
            Ipc_mpGetSelfName(),
            (uint32_t)dst_endpt,
            payload);
        #endif

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM01
<justification end> */
        if((payload & 0xFFFF0000U) == 0xDEAD0000U)
        {
            /* echo this message back to src */
            (void)appIpcSendNotifyPort(app_cpu_id, payload, src_endpt);
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM01
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR003
<justification end> */
            if(NULL != obj->ipc_notify_handler)
            {
                obj->ipc_notify_handler(app_cpu_id, payload);
            }
/* LDRA_JUSTIFY_END */

        }
    }
}

#if (defined (SOC_AM62A) && !(defined (R5F)))
static void appLpmSuspendTaskMain(void *arg0, void *arg1)
{

    app_rtos_semaphore_params_t semParams; // used to initialise basic semaphore details

    appLogPrintf("Main task for LPM Suspend\n");
    /* Default parameter initialization */
    appRtosSemaphoreParamsInit(&semParams);
    semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
    semParams.initValue = 0U;

    g_lpm_suspend_sem = appRtosSemaphoreCreate(semParams);
    if (g_lpm_suspend_sem == NULL)
    {
       appLogPrintf("LPM: ERROR: Cannot create binary semaphore \n");
    }

    while (1)
    {
        /*Wait for suspend from linux*/
        appRtosSemaphorePend(g_lpm_suspend_sem,APP_RTOS_SEMAPHORE_WAIT_FOREVER);
        IpcNotify_sendMsg(gbSuspendRemotecoreID, IPC_NOTIFY_CLIENT_ID_RP_MBOX, (uint32_t)IPC_NOTIFY_RP_MBOX_SUSPEND_ACK, 1u);
        if (gbSuspended == 1u)
        {
            break;
        }
    }
    appRtosSemaphoreDelete(&g_lpm_suspend_sem);
    (void)appRtosTaskDelete(&lpm_task_handle);
}
#endif

static void appIpcRpmsgRxTaskMain(void *arg0, void *arg1)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;
    uint32_t reply_endpt;
#if defined(PDK)
    uint32_t  src_cpu_id;
    bool done = (bool)0;
    uint16_t len;
#elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    uint32_t done = 0;
    uint16_t len, src_cpu_id;
#endif
    int32_t status = 0;

    appUtilsTaskInit();

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR004
<justification end> */
    while(!(bool)done)
/* LDRA_JUSTIFY_END */
    {
    #if defined(PDK)
        len = 0;
        status = RPMessage_recv(obj->rpmsg_rx_handle,
                        &obj->rpmsg_rx_msg_buf,
                        &len,
                        &reply_endpt,
                        &src_cpu_id,
                        APP_RTOS_SEMAPHORE_WAIT_FOREVER
                        );

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM02
<justification end> */
        if(status == IPC_E_UNBLOCKED)
        {
            done = (bool)1;
        }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR005
<justification end> */
        if(status == IPC_SOK)
        {
            appIpcRpmsgRxHandler(obj->rpmsg_rx_handle,
                        obj,
                        obj->rpmsg_rx_msg_buf,
                        len,
                        src_cpu_id,
                        (uint16_t)reply_endpt,
                        (uint16_t)obj->prm.tiovx_rpmsg_port_id);
        }
/* LDRA_JUSTIFY_END */
	#elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
        len = (uint16_t)sizeof(obj->rpmsg_rx_msg_buf);
        status = RPMessage_recv(&obj->rpmsg_rx_handle,
                            &obj->rpmsg_rx_msg_buf,
                            &len,
                            &src_cpu_id,
                            &reply_endpt,
                            SystemP_WAIT_FOREVER
                            );
        #if defined(SOC_AM62A)
        if(gbShutdown == 1U)
        {
          break;
        }
        #endif
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR019
<justification end> */
        if(status == SystemP_SUCCESS)
        {
            appIpcRpmsgRxHandler(&obj->rpmsg_rx_handle,
                        obj,
                        obj->rpmsg_rx_msg_buf,
                        len,
                        src_cpu_id,
                        (uint16_t)reply_endpt,
                        (uint16_t)obj->prm.tiovx_rpmsg_port_id);
        }
/* LDRA_JUSTIFY_END */

	#endif
    }

    #if defined(SOC_AM62A)
    appIpcDeleteRpmsgRxTask(&g_app_ipc_obj);
    #endif

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM21
<justification end> */
}
/* LDRA_JUSTIFY_END */

#if defined(SOC_AM62A)
void appIpcSendShutdownAck(void)
{
    if(gbShutdownRemotecoreID)
    {
        /* ACK the shutdown message */
        IpcNotify_sendMsg(gbShutdownRemotecoreID, IPC_NOTIFY_CLIENT_ID_RP_MBOX, (uint32_t)IPC_NOTIFY_RP_MBOX_SHUTDOWN_ACK, 1u);
    }

}

uint32_t appIpcShutDownReceived(void)
{
    return gbShutdown;
}
#endif

#if (defined (SOC_AM62A) && !(defined (R5F)))
static int32_t appLpmCreateSuspendTask(void)
{
    app_rtos_task_params_t rtos_task_prms;
    int32_t status =0;

    appRtosTaskParamsInit(&rtos_task_prms);

    rtos_task_prms.stacksize = APP_LPM_SUSPEND_TASK_STACK_SIZE;
    rtos_task_prms.stack = g_app_lpm_suspend_task_stack;
    rtos_task_prms.priority = APP_LPM_SUSPEND_TASK_PRI;
    rtos_task_prms.arg0 = NULL;
    rtos_task_prms.arg1 = NULL;
    rtos_task_prms.name = "LPM_TASK";
    rtos_task_prms.taskfxn = &appLpmSuspendTaskMain;

    lpm_task_handle = (void*)appRtosTaskCreate(&rtos_task_prms);

    if(lpm_task_handle == NULL)
    {
        appLogPrintf("LPM: ERROR: Unable to create LPM Suspend task \n");
        status = -1;
    }

    return status;

}
#endif

static int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj)
{
    app_rtos_task_params_t rtos_task_prms;
    int32_t status = 0;

    appRtosTaskParamsInit(&rtos_task_prms);

    rtos_task_prms.stacksize = obj->task_stack_size;
    rtos_task_prms.stack = obj->task_stack;
    rtos_task_prms.priority = obj->task_pri;
    rtos_task_prms.arg0 = NULL;
    rtos_task_prms.arg1 = NULL;
    rtos_task_prms.name = (const char*)&obj->task_name[0];
    rtos_task_prms.taskfxn = &appIpcRpmsgRxTaskMain;

    (void)strncpy(obj->task_name, "IPC_RX", APP_IPC_MAX_TASK_NAME);
    obj->task_name[APP_IPC_MAX_TASK_NAME-1u] = (char)0;

    obj->task_handle = (void*)appRtosTaskCreate(&rtos_task_prms);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM03
<justification end> */
    if(obj->task_handle==NULL)
    {
        appLogPrintf("IPC: ERROR: Unable to create RX task \n");
        status = -1;
    }
/* LDRA_JUSTIFY_END */
    return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static void appIpcDeleteRpmsgRxTask.* <function end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM04
<justification end> */
static void appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj)
{
    #if defined(PDK)
    uint32_t sleep_time = 16U;

    RPMessage_unblock(obj->rpmsg_rx_handle);

    /* confirm task termination */
    while ( !(1u==appRtosTaskIsTerminated(obj->task_handle)) )
    {
        appLogWaitMsecs(sleep_time);
        sleep_time >>= 1U;
        if (sleep_time == 0U)
        {
            /* Force delete after timeout */
            break;
        }
    }
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_unblock(&obj->rpmsg_rx_handle);
    #endif

    (void)appRtosTaskDelete(&obj->task_handle);
}

void appIpcInitPrmSetDefault(app_ipc_init_prm_t *prm)
{
    uint32_t cpu_id = 0;

    prm->num_cpus = 0;
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        prm->enabled_cpu_id_list[cpu_id] = APP_IPC_CPU_INVALID;
    }
    prm->tiovx_rpmsg_port_id = APP_IPC_TIOVX_RPMSG_PORT_ID;
    prm->tiovx_obj_desc_mem = NULL;
    prm->tiovx_obj_desc_mem_size = 0;
    prm->ipc_vring_mem = NULL;
    prm->ipc_vring_mem_size = 0;
    prm->tiovx_log_rt_mem = NULL;
    prm->tiovx_log_rt_mem_size = 0;
    prm->self_cpu_id = APP_IPC_CPU_INVALID;
    prm->ipc_resource_tbl = NULL;
    prm->enable_tiovx_ipc_announce = 1;
}

#if defined(SOC_AM62A)
void sortEnabledCoreIdsAscending(uint32_t *arr, int size)
{
    uint8_t outerCoreId, innerCoreId;
    if((arr != NULL) && (size > 1))
    {
        for(outerCoreId = 0; outerCoreId < size-1; outerCoreId++)
        {
            for(innerCoreId = 0; innerCoreId < size-outerCoreId-1; innerCoreId++)
            {
                if(arr[innerCoreId] > arr[innerCoreId+1])
                {
                    uint32_t tempCoreId = arr[innerCoreId];
                    arr[innerCoreId] = arr[innerCoreId+1];
                    arr[innerCoreId+1] = tempCoreId;
                }
            }
        }
    }
}
void vringAllocateForCores(uint32_t enabled_Cores[], uint32_t numCores)
{
    uint32_t vringIndex = 0;
    uint32_t selfCoreId, remoteCoreId;
    uint8_t i, j;

    if((enabled_Cores != NULL) && (numCores > 1))
    {
        sortEnabledCoreIdsAscending(enabled_Cores, numCores);

        for( i = 0; i < numCores; i ++)
        {
            selfCoreId = enabled_Cores[i];
            if(selfCoreId < APP_IPC_CPU_MAX)
            {
                for(j = 0; j < numCores; j++)
                {
                    remoteCoreId = enabled_Cores[j];
                    if(remoteCoreId < APP_IPC_CPU_MAX)
                    {
                        if(selfCoreId == remoteCoreId)
                        {
                            gVring_id[selfCoreId][remoteCoreId] = UINT32_MAX;
                        }
                        else
                        {
                            gVring_id[selfCoreId][remoteCoreId] = vringIndex;
                            vringIndex++;
                        }
                    }

                }
            }

        }
    }
}
#endif

int32_t appIpcInit(app_ipc_init_prm_t *prm)
{
    int32_t status = 0;
    uint32_t cpu_id = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    appLogPrintf("IPC: Init ... !!!\n");

    obj->prm = *prm;

    #if defined(PDK)
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        obj->rpmsg_tx_handle[cpu_id] = NULL;
    }
    obj->task_handle = NULL;
    obj->rpmsg_rx_handle = NULL;
    #endif

    obj->ipc_notify_handler = NULL;
    obj->task_stack = g_app_rpmessage_rx_task_stack;
    obj->task_stack_size = APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE;
    obj->task_pri = APP_IPC_RPMESSAGE_RX_TASK_PRI;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM05
<justification end> */
    if(prm->num_cpus>APP_IPC_CPU_MAX)
    {
        appLogPrintf("IPC: ERROR: Invalid number of CPUs !!!\n");
        status = -1;
    }
    if( (prm->tiovx_obj_desc_mem==NULL) || (prm->tiovx_obj_desc_mem_size==0u) )
    {
        appLogPrintf("IPC: ERROR: Invalid tiovx obj desc memory address or size !!!\n");
        status = -1;
    }
    if( (prm->ipc_vring_mem==NULL) || (prm->ipc_vring_mem_size==0u) )
    {
        appLogPrintf("IPC: ERROR: Invalid ipc vring memory address or size !!!\n");
        status = -1;
    }
    if(prm->self_cpu_id>=APP_IPC_CPU_MAX)
    {
        appLogPrintf("IPC: ERROR: Invalid self cpu id !!!\n");
        status = -1;
    }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM06
<justification end> */
    #if defined(PDK)
    if(APP_IPC_VQ_OBJ_MEM_SIZE < (Ipc_getVqObjMemoryRequiredPerCore()*IPC_MAX_PROCS) )
    {
        appLogPrintf("IPC: ERROR: APP_IPC_VQ_OBJ_MEM_SIZE is less than Ipc_getVqObjMemoryRequiredPerCore()*IPC_MAX_PROCS (%d < %d) !!!\n",
            APP_IPC_VQ_OBJ_MEM_SIZE,
            Ipc_getVqObjMemoryRequiredPerCore()*IPC_MAX_PROCS
            );
        status = -1;
    }
    #endif

/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR006
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        for(cpu_id=0; cpu_id<prm->num_cpus; cpu_id++)
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM07
<justification end> */
            if(prm->enabled_cpu_id_list[cpu_id]>=APP_IPC_CPU_MAX)
            {
                appLogPrintf("IPC: ERROR: Invalid cpu id in enabled_cpu_id_list @ index %d !!!\n", cpu_id);
                status = -1;
            }
/* LDRA_JUSTIFY_END */
        }
    }

    #if defined(PDK)
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR007
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        status = Ipc_init(NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM08
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: Ipc_init failed !!!\n");
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
s<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR008
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        uint32_t ipc_proc_list[IPC_MAX_PROCS];
        uint32_t ipc_num_proc = 0;

        for(cpu_id=0; cpu_id<prm->num_cpus; cpu_id++)
        {
            if(prm->enabled_cpu_id_list[cpu_id] != prm->self_cpu_id)
            {
                ipc_proc_list[ipc_num_proc] = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[cpu_id]];
                ipc_num_proc++;
            }
        }

        /* +1 because self CPU is also participating in IPC */
        appLogPrintf("IPC: %d CPUs participating in IPC !!!\n", ipc_num_proc+1u);

        status = Ipc_mpSetConfig(
                    g_app_to_ipc_cpu_id[prm->self_cpu_id],
                    (uint16_t)ipc_num_proc,
                    ipc_proc_list
                    );
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM09
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: Ipc_mpSetConfig failed !!!\n");
        }
/* LDRA_JUSTIFY_END */

        if((status==0) && /* TIOVX-1913- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR009 */
           (prm->ipc_resource_tbl != NULL)) /* TIOVX-1913- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR010 */
        {
            /* If A72 remote core is running Linux OS, then load resource table */
            status = Ipc_loadResourceTable((void*)prm->ipc_resource_tbl);

            appLogPrintf("IPC: Waiting for HLOS to be ready ... !!!\n");
            /* Wait for Linux VDev ready... */
            for(cpu_id=0; cpu_id<ipc_num_proc; cpu_id++)
            {
                while(!(1u==Ipc_isRemoteReady((uint16_t)ipc_proc_list[cpu_id])))
                {
                    //appRtosTaskSleep(100);
                }
            }
            appLogPrintf("IPC: HLOS is ready !!!\n");
        }
    }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR011
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        Ipc_VirtIoParams  vq_prm;

        vq_prm.vqObjBaseAddr = g_app_vq_obj_mem;
        vq_prm.vqBufSize     = APP_IPC_VQ_OBJ_MEM_SIZE;
        vq_prm.vringBaseAddr = prm->ipc_vring_mem;
        vq_prm.vringBufSize  = prm->ipc_vring_mem_size;
        vq_prm.timeoutCnt    = 100;
        status = Ipc_initVirtIO(&vq_prm);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM10
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: Ipc_initVirtIO failed !!!\n");
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR012
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        RPMessage_Params cntrl_prm;

        /* Initialize the param */
        (void)RPMessageParams_init(&cntrl_prm);

        /* Set memory for HeapMemory for control task */
        cntrl_prm.buf = g_app_rpmessage_ctrl_params_buf;
        cntrl_prm.bufSize = APP_IPC_RPMESSAGE_CTRL_PARAMS_BUF_SIZE;
        cntrl_prm.numBufs = APP_IPC_RPMESSAGE_CTRL_PARAMS_NUM_BUF;
        cntrl_prm.stackSize   = APP_IPC_RPMESSAGE_CTRL_TASK_STACK_SIZE;
        cntrl_prm.stackBuffer = &g_app_rpmessage_ctrl_task_stack[0];
        status = RPMessage_init(&cntrl_prm);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM11
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: RPMessage_init failed !!!\n");
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR013
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        RPMessage_Params rpmsg_prm;

        for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
        {
            if(0U != appIpcIsCpuEnabled(cpu_id))
            {
                (void)RPMessageParams_init(&rpmsg_prm);

                rpmsg_prm.requestedEndpt = RPMESSAGE_ANY;
                rpmsg_prm.buf = g_app_rpmessage_rpmsg_tx_buf[cpu_id];
                rpmsg_prm.bufSize = APP_IPC_RPMESSAGE_RPMSG_TX_BUF_SIZE;
                rpmsg_prm.numBufs = APP_IPC_RPMESSAGE_RPMSG_TX_NUM_BUF;

                obj->rpmsg_tx_handle[cpu_id] =
                    RPMessage_create(&rpmsg_prm, &obj->rpmsg_tx_endpt[cpu_id]);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM12
<justification end> */
                if(obj->rpmsg_tx_handle[cpu_id]==NULL)
                {
                    appLogPrintf("IPC: ERROR: Unable to create rpmessage tx handle for cpu %d !!!\n", cpu_id);
                    status = -1;
                }
/* LDRA_JUSTIFY_END */
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM13
<justification end> */
            if(status!=0)
            {
                break;
            }
/* LDRA_JUSTIFY_END */
        }
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR014
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        RPMessage_Params rpmsg_prm;
        uint32_t rx_endpt;

        (void)RPMessageParams_init(&rpmsg_prm);

        rpmsg_prm.requestedEndpt = prm->tiovx_rpmsg_port_id;
        rpmsg_prm.buf = g_app_rpmessage_rpmsg_rx_buf;
        rpmsg_prm.bufSize = APP_IPC_RPMESSAGE_RPMSG_RX_BUF_SIZE;
        rpmsg_prm.numBufs = APP_IPC_RPMESSAGE_RPMSG_RX_NUM_BUF;

        obj->rpmsg_rx_handle =
            RPMessage_create(&rpmsg_prm, &rx_endpt);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM14
<justification end> */
        if(obj->rpmsg_rx_handle==NULL)
        {
            appLogPrintf("IPC: ERROR: Unable to create rpmessage rx handle !!!\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */

        /* NOTE: RPMessage_setCallback is not yet implemented */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR020
<justification end> */
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        IpcNotify_Params notifyParams;
        uint32_t ipc_num_proc = 0;

        /* initialize parameters to default */
        IpcNotify_Params_init(&notifyParams);

        /* specify the core on which this API is called */
        notifyParams.selfCoreId = (uint16_t)g_app_to_ipc_cpu_id[prm->self_cpu_id];

        /* list the cores that will do IPC Notify with this core
        * Make sure to NOT list 'self' core in the list below
        */

        for(cpu_id=0; cpu_id<prm->num_cpus; cpu_id++)
        {
            if(prm->enabled_cpu_id_list[cpu_id] != prm->self_cpu_id)
            {
                notifyParams.coreIdList[ipc_num_proc] = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[cpu_id]];
                ipc_num_proc++;
            }
        }

        /* +1 because self CPU is also participating in IPC */
        appLogPrintf("IPC: %d CPUs participating in IPC !!!\n", ipc_num_proc+1U);

        notifyParams.numCores = ipc_num_proc;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM28
<justification end> */
        if (prm->ipc_resource_tbl != NULL)
        {
            #if defined (MCU_PLUS_SDK)
            notifyParams.linuxCoreId = CSL_CORE_ID_A53SS0_0;
            #elif defined (MCU_SDK)
            notifyParams.linuxCoreId = CORE_ID_A720_0;
            #endif
        }
/* LDRA_JUSTIFY_END */

        /* initialize the IPC Notify module */
        status = IpcNotify_init(&notifyParams);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM22
<justification end> */
        if(status != SystemP_SUCCESS)
        {
            appLogPrintf("IPC: ERROR: IpcNotify_init failed !!!\n");
        }
/* LDRA_JUSTIFY_END */
        #if (defined(SOC_AM62A) && defined(__C7504__))
        status = IpcNotify_registerClient(IPC_NOTIFY_CLIENT_ID_RP_MBOX, &appIpcRpmboxCallback, NULL);
        if(status != SystemP_SUCCESS)
        {
            appLogPrintf("IPC: ERROR: IpcNotify_registerClient failed !!!\n");
        }
        #endif
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR021
<justification end> */
    if(status == SystemP_SUCCESS)
    {
/* LDRA_JUSTIFY_END */
        RPMessage_Params rpmsgParams;
        uint32_t src_cpu_id, dst_cpu_id, vringId;
        #if defined(MCU_SDK)
        uint32_t rxTxMap[CORE_ID_MAX][CORE_ID_MAX];
        #elif defined(MCU_PLUS_SDK)
        uint32_t rxTxMap[CSL_CORE_ID_MAX][CSL_CORE_ID_MAX];
        #endif
        uint32_t self_core_id = g_app_to_ipc_cpu_id[prm->self_cpu_id];

        /* initialize parameters to default */
        RPMessage_Params_init(&rpmsgParams);
        #if defined (SOC_AM62A)
        uint32_t enabled_Cores[prm->num_cpus];
        uint8_t coreIndex;
        for(coreIndex = 0U; coreIndex < prm->num_cpus; coreIndex++)
        {
            enabled_Cores[coreIndex] = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[coreIndex]];
        }

        vringAllocateForCores(enabled_Cores, prm->num_cpus);
        #endif
        /* VRING mapping from source core to destination core, '-1' means NO VRING */
        /* for each name, construct a N x N object mapping SRC CPU to DST CPU VRING ID,
        Assign VRING IDs to each SRC/DST pair, skip assignment when SRC == DST */
        vringId = 0U;
        for( src_cpu_id=0; src_cpu_id<prm->num_cpus; src_cpu_id++ )
        {
            uint32_t src_core_id = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[src_cpu_id]];
            for( dst_cpu_id=0; dst_cpu_id<prm->num_cpus; dst_cpu_id++ )
            {
                uint32_t dst_core_id = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[dst_cpu_id]];
                if(src_core_id != dst_core_id)  /* NO VRING for a CPU to itself */
                {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM23
<justification end> */
                    if (prm->ipc_resource_tbl != NULL)
/* LDRA_JUSTIFY_END */
                    {
                        rxTxMap[src_core_id][dst_core_id] = vringId;
                        vringId++;
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM23
<justification end> */
                    else
                    {
                        #if defined(MCU_SDK)
                        rxTxMap[src_core_id][dst_core_id] = vringId;
                        vringId++;
                        #elif defined (MCU_PLUS_SDK)
                        #if defined (SOC_AM62A)
                        rxTxMap[src_core_id][dst_core_id] = gVring_id[src_core_id][dst_core_id];
                        #else
                        rxTxMap[src_core_id][dst_core_id] = appIpcGetVringIndex(APP_IPC_CPU_MAX, src_core_id, dst_core_id);
                        #endif
                        #endif
                    }
/* LDRA_JUSTIFY_END */
                }
            }
        }

        /* Update Tx buffers */
        for( dst_cpu_id=0; dst_cpu_id<prm->num_cpus; dst_cpu_id++ )
        {
            uint32_t dst_core_id = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[dst_cpu_id]];
            if (dst_core_id != self_core_id)
            {
                uint32_t offset = (uint32_t)IPC_RPMESSAGE_VRING_SIZE * rxTxMap[self_core_id][dst_core_id];
                rpmsgParams.vringTxBaseAddr[dst_core_id] = (uintptr_t)(((uint8_t*)prm->ipc_vring_mem) + offset);
            }
        }

        /* Update Rx buffers */
        for( src_cpu_id=0; src_cpu_id<prm->num_cpus; src_cpu_id++ )
        {
            uint32_t src_core_id = g_app_to_ipc_cpu_id[prm->enabled_cpu_id_list[src_cpu_id]];
            if (src_core_id != self_core_id)
            {
                uint32_t offset = (uint32_t)IPC_RPMESSAGE_VRING_SIZE * rxTxMap[src_core_id][self_core_id];
                rpmsgParams.vringRxBaseAddr[src_core_id] = (uintptr_t)(((uint8_t*)prm->ipc_vring_mem) + offset);
            }
        }

        rpmsgParams.vringSize = IPC_RPMESSAGE_VRING_SIZE;
        rpmsgParams.vringNumBuf = IPC_RPMESSAGE_NUM_VRING_BUF;
        rpmsgParams.vringMsgSize = IPC_RPMESSAGE_MAX_VRING_BUF_SIZE;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM24
<justification end> */
        if (prm->ipc_resource_tbl != NULL)
/* LDRA_JUSTIFY_END */
        {
            rpmsgParams.linuxResourceTable = prm->ipc_resource_tbl;
            #if defined (MCU_PLUS_SDK)
            rpmsgParams.linuxCoreId = CSL_CORE_ID_A53SS0_0;
            #elif defined (MCU_SDK)
            rpmsgParams.linuxCoreId = CORE_ID_A720_0;
            #endif
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM24
<justification end> */
        else
        {
            /* Note: this is needed with QNX */
            #if defined (MCU_PLUS_SDK)
            rpmsgParams.vringAllocationPDK = 1u;
            rpmsgParams.vringAllocationQNX = 1u;
            #endif
        }
/* LDRA_JUSTIFY_END */

        /* initialize the IPC RP Message module */
        status = RPMessage_init(&rpmsgParams);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM25
<justification end> */
        if(status != SystemP_SUCCESS)
        {
            appLogPrintf("IPC: ERROR: RPMessage_init failed !!!\n");
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR022
<justification end> */
    if(status == SystemP_SUCCESS)
/* LDRA_JUSTIFY_END */
    {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM29
<justification end> */
        if (prm->ipc_resource_tbl != NULL)
        {
            appLogPrintf("IPC: Waiting for HLOS to be ready ... !!!\n");
            /* This API MUST be called by applications when its ready to talk to Linux */
            status = RPMessage_waitForLinuxReady(SystemP_WAIT_FOREVER);
            appLogPrintf("IPC: HLOS is ready !!!\n");
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR023
<justification end> */
    if(status == SystemP_SUCCESS)
/* LDRA_JUSTIFY_END */
    {
        RPMessage_CreateParams rpmsg_createTx_prms;

        for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
        {
            if(0U != appIpcIsCpuEnabled(cpu_id))
            {
                RPMessage_CreateParams_init(&rpmsg_createTx_prms);
                rpmsg_createTx_prms.localEndPt = (uint16_t)cpu_id;

                status = RPMessage_construct(&obj->rpmsg_tx_handle[cpu_id], &rpmsg_createTx_prms);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM26
<justification end> */
                if(status != SystemP_SUCCESS)
                {
                    appLogPrintf("IPC: ERROR: Unable to create rpmessage tx handle for cpu %d !!!\n", cpu_id);
                    break;
                }
/* LDRA_JUSTIFY_END */
            }
        }
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR024
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        RPMessage_CreateParams rpmsg_createRx_prms;

        RPMessage_CreateParams_init(&rpmsg_createRx_prms);
        rpmsg_createRx_prms.localEndPt = (uint16_t)prm->tiovx_rpmsg_port_id;

        status = RPMessage_construct(&obj->rpmsg_rx_handle, &rpmsg_createRx_prms);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM27
<justification end> */
        if(SystemP_SUCCESS !=status)
        {
            appLogPrintf("IPC: ERROR: Unable to create rpmessage rx handle !!!\n");
        }
/* LDRA_JUSTIFY_END */

        /* NOTE: RPMessage_setCallback is not yet implemented */
    }
#endif

    if((status==0) && /* TIOVX-1913- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR015 */
    (prm->enable_tiovx_ipc_announce == 1u)) /* TIOVX-1913- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR016 */
    {
        /* use "rpmsg-proto" or "rpmsg_chrdev" depending on protocol selected in user space on Linux A72 */
        #if defined(PDK)
        status = RPMessage_announce(RPMESSAGE_ALL, prm->tiovx_rpmsg_port_id, "rpmsg_chrdev");
        #elif defined(MCU_PLUS_SDK)
        status = RPMessage_announce(CSL_CORE_ID_A53SS0_0, (uint16_t)prm->tiovx_rpmsg_port_id, "rpmsg_chrdev");
        #elif defined(MCU_SDK)
        status = RPMessage_announce(CORE_ID_A720_0, (uint16_t)prm->tiovx_rpmsg_port_id, "rpmsg_chrdev");
        #endif
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM15
<justification end> */
        if(status != 0)
        {
            appLogPrintf("IPC: RPMessage_announce() for rpmsg-proto failed\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_RTOS_UBR017
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        status = appIpcCreateRpmsgRxTask(obj);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM16
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: appIpcCreateRpmsgRxTask failed !!!\n");
        }
        #if (defined (SOC_AM62A) && !(defined (R5F)))
        status = appLpmCreateSuspendTask();
        if(status!=0)
        {
            appLogPrintf("LPM: ERROR: appLpmCreateSuspendTask failed !!!\n");
        }
        #endif
/* LDRA_JUSTIFY_END */
    }
    appLogPrintf("IPC: Init ... Done !!!\n");

    return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t appIpcDeInit.* <function end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM17
<justification end> */
int32_t appIpcDeInit(void)
{
    int32_t status = 0;
    uint32_t cpu_id;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    appLogPrintf("IPC: Deinit ... !!!\n");

    appIpcDeleteRpmsgRxTask(obj);

    #if defined(PDK)
    int32_t temp_status = 0;

    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(obj->rpmsg_tx_handle[cpu_id]!=NULL)
        {
            temp_status = RPMessage_delete(&obj->rpmsg_tx_handle[cpu_id]);
            if (IPC_SOK != temp_status)
            {
                appLogPrintf("MSG_delete: Failed to delete endpt\n");
                status = temp_status;
            }
            obj->rpmsg_tx_handle[cpu_id] = NULL;
        }
    }
    if(obj->rpmsg_rx_handle!=NULL)
    {
        temp_status = RPMessage_delete(&obj->rpmsg_rx_handle);
        if (IPC_SOK != temp_status)
        {
            appLogPrintf("MSG_delete: Failed to delete endpt\n");
            status = temp_status;
        }
        obj->rpmsg_rx_handle = NULL;
    }

    RPMessage_deInit();
    temp_status = Ipc_deinit();
    if (IPC_SOK != temp_status)
    {
        appLogPrintf("Ipc_deinit: Failed to de Initialize IPC module\n");
        status = temp_status;
    }
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
      for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        RPMessage_destruct(&obj->rpmsg_tx_handle[cpu_id]);

    }
    RPMessage_destruct(&obj->rpmsg_rx_handle);

    IpcNotify_deInit();
    RPMessage_deInit();
    #endif

    appLogPrintf("IPC: Deinit ... Done !!!\n");

    return status;
}

int32_t appIpcRegisterNotifyHandler(app_ipc_notify_handler_f handler)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    obj->ipc_notify_handler = handler;

    return status;
}

int32_t appIpcSendNotifyPort(uint32_t dest_cpu_id, uint32_t payload, uint32_t port_id)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    #if defined(PDK)
    if( (dest_cpu_id<APP_IPC_CPU_MAX) && (obj->rpmsg_tx_handle[dest_cpu_id] != NULL))
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    if( (dest_cpu_id<APP_IPC_CPU_MAX) && (&obj->rpmsg_tx_handle[dest_cpu_id] != NULL))
    #endif
    {
        uint32_t ipc_cpu_id = g_app_to_ipc_cpu_id[dest_cpu_id];

        #if defined(PDK)
        #ifdef APP_IPC_DEBUG
        appLogPrintf("IPC: TX: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
            Ipc_mpGetSelfName(),
            obj->prm.tiovx_rpmsg_port_id,
            Ipc_mpGetName(ipc_cpu_id),
            port_id,
            payload);
        #endif

        status = RPMessage_send(
                    obj->rpmsg_rx_handle,
                    ipc_cpu_id,
                    port_id, /* dst end pt */
                    obj->prm.tiovx_rpmsg_port_id, /* src endpt */
                    &payload,
                    (uint16_t)sizeof(payload)
                    );
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM18
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: TX: FAILED: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
                Ipc_mpGetSelfName(),
                obj->prm.tiovx_rpmsg_port_id,
                Ipc_mpGetName(ipc_cpu_id),
                port_id,
                payload);
        }
/* LDRA_JUSTIFY_END */
        #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
        #ifdef APP_IPC_DEBUG
        appLogPrintf("IPC: TX: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
            Ipc_mpGetSelfName(),
            obj->prm.tiovx_rpmsg_port_id,
            Ipc_mpGetName(ipc_cpu_id),
            port_id,
            payload);
        #endif

        status = RPMessage_send(
                    &payload,
                    (uint16_t)sizeof(payload),
                    (uint16_t)ipc_cpu_id,
                    (uint16_t)port_id,    /* dst end pt */
                    (uint16_t)obj->prm.tiovx_rpmsg_port_id, /* src endpt */
                    SystemP_WAIT_FOREVER
                    );
        if(status!=0)
        {
            appLogPrintf("IPC: TX: FAILED: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
                Ipc_mpGetSelfName(),
                obj->prm.tiovx_rpmsg_port_id,
                Ipc_mpGetName((uint16_t)ipc_cpu_id),
                port_id,
                payload);
        }
        #endif
    }
    return status;
}

int32_t appIpcSendNotify(uint32_t dest_cpu_id, uint32_t payload)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    #if defined(PDK)
    if( (dest_cpu_id<APP_IPC_CPU_MAX) && (obj->rpmsg_tx_handle[dest_cpu_id] != NULL))
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    if(dest_cpu_id<APP_IPC_CPU_MAX)
    #endif
    {
        status = appIpcSendNotifyPort(dest_cpu_id, payload,
            (uint32_t)obj->prm.tiovx_rpmsg_port_id);
    }

    return status;
}

uint32_t appIpcGetSelfCpuId(void)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    return obj->prm.self_cpu_id;
}

uint32_t appIpcGetHostPortId(uint16_t cpu_id)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    return obj->prm.tiovx_rpmsg_port_id;
}

uint32_t appIpcIsCpuEnabled(uint32_t cpu_id)
{
    uint32_t is_enabled = 0, cur_cpu_id;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if(cpu_id>=APP_IPC_CPU_MAX)
    {
        is_enabled = 0;
    }
    for(cur_cpu_id=0; cur_cpu_id<obj->prm.num_cpus; cur_cpu_id++)
    {
        if(cpu_id==obj->prm.enabled_cpu_id_list[cur_cpu_id])
        {
            is_enabled = 1;
            break;
        }
    }
    return is_enabled;
}

int32_t appIpcGetTiovxObjDescSharedMemInfo(void **addr, uint32_t *size)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM19
<justification end> */
    if( (obj->prm.tiovx_obj_desc_mem==NULL) || (obj->prm.tiovx_obj_desc_mem_size==0u) )
    {
        *addr = NULL;
        *size = 0;
        status = -1;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM19
<justification end> */
    else
/* LDRA_JUSTIFY_END */
    {
        *addr = obj->prm.tiovx_obj_desc_mem;
        *size = obj->prm.tiovx_obj_desc_mem_size;
    }

    return status;
}

void appIpcGetTiovxLogRtSharedMemInfo(void **shm_base, uint32_t *shm_size)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM20
<justification end> */
    if( (obj->prm.tiovx_log_rt_mem==NULL) || (obj->prm.tiovx_log_rt_mem_size==0u) )
    {
        *shm_base = NULL;
        *shm_size = 0;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_RTOS_UM20
<justification end> */
    else
/* LDRA_JUSTIFY_END */
    {
        *shm_base = obj->prm.tiovx_log_rt_mem;
        *shm_size = obj->prm.tiovx_log_rt_mem_size;
    }
}

int32_t appIpcHwLockAcquire(uint32_t hw_lock_id, uint32_t timeout)
{
    int32_t status = -1;

    if( hw_lock_id < APP_IPC_HW_SPIN_LOCK_MAX)
    {
        volatile uint32_t *reg_addr;

        reg_addr =
                (volatile uint32_t*)(uint32_t)(
                    APP_IPC_HW_SPIN_LOCK_MMR_BASE +
                    APP_IPC_HW_SPIN_LOCK_OFFSET(hw_lock_id)
                        );

        key = HwiP_disable();
        /* spin until lock is free */
        while( *reg_addr == 1u )
        {
            HwiP_restore(key);
            appRtosTaskYield();
            key = HwiP_disable();
            /* keep spinning */
        }
        status = 0;
    }

    return status;
}

int32_t appIpcHwLockRelease(uint32_t hw_lock_id)
{
    int32_t status = -1;

    if(hw_lock_id < APP_IPC_HW_SPIN_LOCK_MAX)
    {
        volatile uint32_t *reg_addr;

        reg_addr =
                (volatile uint32_t*)(uint32_t)(
                    APP_IPC_HW_SPIN_LOCK_MMR_BASE +
                    APP_IPC_HW_SPIN_LOCK_OFFSET(hw_lock_id)
                        );

        *reg_addr = 0; /* free the lock */
        HwiP_restore(key);
        status = 0;
    }

    return status;
}

uint32_t appIpcGetIpcCpuId(uint32_t app_cpu_id)
{
    #if defined(PDK)
    uint32_t ipc_cpu_id = IPC_MP_INVALID_ID;
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        ipc_cpu_id = g_app_to_ipc_cpu_id[app_cpu_id];
    }
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    uint32_t ipc_cpu_id = APP_MCUSDK_IPC_CPU_INVALID;
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        ipc_cpu_id = g_app_to_ipc_cpu_id[app_cpu_id];
    }
    #endif
    return ipc_cpu_id;
}


uint32_t appIpcGetAppCpuId(char *name)
{
    uint32_t ipc_cpu_id;
    uint32_t app_cpu_id = APP_IPC_CPU_INVALID;

    ipc_cpu_id = Ipc_mpGetId(name);

    if(ipc_cpu_id < IPC_MAX_PROCS)
    {
        app_cpu_id = g_ipc_to_app_cpu_id[ipc_cpu_id];
    }
    return app_cpu_id;
}

const char *appIpcGetCpuName(uint32_t app_cpu_id)
{
    const char *name = "invalid";
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        name = Ipc_mpGetName(g_app_to_ipc_cpu_id[app_cpu_id]);
    }
    return name;
}

#if (defined(SOC_AM62A) && defined(__C7504__))
void appIpcRpmboxCallback(uint16_t remoteCoreId, uint16_t clientId, uint32_t msgValue, void *args)
{
  app_ipc_obj_t *obj = &g_app_ipc_obj;
  if (clientId == IPC_NOTIFY_CLIENT_ID_RP_MBOX)
  {
    if (msgValue == (uint32_t)IPC_NOTIFY_RP_MBOX_SHUTDOWN) /* Shutdown request from the remotecore */
    {
        gbShutdown = 1u;
        gbShutdownRemotecoreID = remoteCoreId;
        RPMessage_unblock(&obj->rpmsg_rx_handle);
    }
    else if (msgValue == (uint32_t)IPC_NOTIFY_RP_MBOX_SUSPEND_SYSTEM) /* Suspend request received from linux during LPM suspend */
    {
        gbSuspended = 1u;
        gbSuspendRemotecoreID = remoteCoreId;
        appRtosSemaphorePost(g_lpm_suspend_sem);
    }
    else
    {
        appLogPrintf("Unexpected message value %u from core %u\n", msgValue, remoteCoreId);
    }
  }
}
#endif
