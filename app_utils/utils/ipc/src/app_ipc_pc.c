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

#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/misc/include/app_misc.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/rtos/include/app_rtos.h>
#include <utils/pc_osal/include/dpl_osal.h>
#include <utils/console_io/include/app_log.h>
#include <utils/rtos/include/app_rtos.h>
#include <utils/ipc/include/mcu_sdk_ipc.h>

#include <RPMessage_Hostemu.h>
#include <Ipc_Notify_Hostemu.h>
#include <ClockP.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define APP_IPC_MAX_TASK_NAME                  (12u)
#define IPC_RPMESSAGE_MSG_SIZE                 (496U + 32U)
#define APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE   (64u*1024u)
#define APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT    (8192u)
#define APP_IPC_RPMESSAGE_RX_TASK_PRI          (10u)
#define APP_MCUSDK_IPC_CPU_INVALID             (0xFFFFFFFFU)

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

typedef struct {

    app_ipc_init_prm_t prm;
    uint32_t rpmsg_tx_endpt[APP_IPC_CPU_MAX];
    app_ipc_notify_handler_f ipc_notify_handler;
    app_rtos_task_handle_t task_handle;

    RPMessage_Object rpmsg_tx_handle[APP_IPC_CPU_MAX];
    RPMessage_Object rpmsg_rx_handle;

    uint32_t task_stack_size;
    uint8_t *task_stack;
    uint32_t task_pri;
    uint8_t  rpmsg_rx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    char     task_name[APP_IPC_MAX_TASK_NAME];

} app_ipc_obj_t;

static app_ipc_obj_t g_app_ipc_obj[APP_IPC_CPU_MAX];

static app_vdk_get_cpu_id_f g_app_vdk_get_cpu_id = NULL;

static uint8_t g_app_rpmessage_rx_task_stack[APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_IPC_RPMESSAGE_RX_TASK_ALIGNMENT)))
    ;

static void appIpcRpmsgRxTaskMain(void *arg0, void *arg1);
static int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj);
static void appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj);
static void appIpcRpmsgRxHandler(RPMessage_Object* rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt);

void appIpcVdkRegisterGetCpuId(app_vdk_get_cpu_id_f app_vdk_get_cpu_id)
{
    g_app_vdk_get_cpu_id = app_vdk_get_cpu_id;
}

static void appIpcRpmsgRxHandler(RPMessage_Object* rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt)
{
    uint32_t app_cpu_id, payload;
    app_ipc_obj_t *obj = arg;

    if((src_cpu_id<IPC_MAX_PROCS) && (len == sizeof(payload)))
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

        if((payload & 0xFFFF0000U) == 0xDEAD0000U)
        {
            /* echo this message back to src */
            (void)appIpcSendNotifyPort(app_cpu_id, payload, src_endpt);
        }
        else
        {
            if(NULL != obj->ipc_notify_handler)
            {
                obj->ipc_notify_handler(app_cpu_id, payload);
            }
        }
    }
}

static void appIpcRpmsgRxTaskMain(void *arg0, void *arg1)
{
    IpcNotify_Hal_VdkSetThreadName(*(uint32_t *)arg0);
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];
    uint32_t reply_endpt;
    uint32_t done = 0;
    uint16_t len, src_cpu_id;
    int32_t status = 0;

    while(!(bool)done)
    {
        len = (uint16_t)sizeof(obj->rpmsg_rx_msg_buf);

        status = RPMessage_recv(&obj->rpmsg_rx_handle,
                            &obj->rpmsg_rx_msg_buf,
                            &len,
                            &src_cpu_id,
                            &reply_endpt,
                            SystemP_WAIT_FOREVER
                            );
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
    }
}

int32_t appIpcSendNotifyPort(uint32_t dest_cpu_id, uint32_t payload, uint32_t port_id)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

    if((dest_cpu_id<APP_IPC_CPU_MAX) && (&obj->rpmsg_tx_handle[dest_cpu_id] != NULL))
    {
        uint32_t ipc_cpu_id = g_app_to_ipc_cpu_id[dest_cpu_id];

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
    }
    return status;
}

int32_t appIpcSendNotify(uint32_t dest_cpu_id, uint32_t payload)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

    if(dest_cpu_id<APP_IPC_CPU_MAX)
    {
        status = appIpcSendNotifyPort(dest_cpu_id, payload,
            (uint32_t)obj->prm.tiovx_rpmsg_port_id);
    }

    return status;
}

static int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj)
{
    app_rtos_task_params_t rtos_task_prms;
    int32_t status = 0;

    appRtosTaskParamsInit(&rtos_task_prms);

    rtos_task_prms.stacksize = obj->task_stack_size;
    rtos_task_prms.stack = obj->task_stack;
    rtos_task_prms.priority = obj->task_pri;
    rtos_task_prms.arg0 = (void *)&g_app_to_ipc_cpu_id[obj->prm.self_cpu_id];
    rtos_task_prms.arg1 = NULL;
    rtos_task_prms.name = (const char*)&obj->task_name[0];
    rtos_task_prms.taskfxn = &appIpcRpmsgRxTaskMain;

    (void)strncpy(obj->task_name, "IPC_RX", APP_IPC_MAX_TASK_NAME);
    obj->task_name[APP_IPC_MAX_TASK_NAME-1u] = (char)0;

    obj->task_handle = (void*)appRtosTaskCreate(&rtos_task_prms);
    if(obj->task_handle==NULL)
    {
        appLogPrintf("IPC: ERROR: Unable to create RX task \n");
        status = -1;
    }
    return status;
}

static void appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj)
{
    RPMessage_unblock(&obj->rpmsg_rx_handle);

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

int32_t appIpcInit(app_ipc_init_prm_t *prm)
{
    int32_t status = 0;
    uint32_t cpu_id = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj[prm->self_cpu_id];

    appLogPrintf("IPC: Init ... !!!\n");

    obj->prm = *prm;

    obj->ipc_notify_handler = NULL;
    obj->task_stack = g_app_rpmessage_rx_task_stack;
    obj->task_stack_size = APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE;
    obj->task_pri = APP_IPC_RPMESSAGE_RX_TASK_PRI;

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
    if(status==0)
    {
        for(cpu_id=0; cpu_id<prm->num_cpus; cpu_id++)
        {
            if(prm->enabled_cpu_id_list[cpu_id]>=APP_IPC_CPU_MAX)
            {
                appLogPrintf("IPC: ERROR: Invalid cpu id in enabled_cpu_id_list @ index %d !!!\n", cpu_id);
                status = -1;
            }
        }
    }

    if(status==0)
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
        if (prm->ipc_resource_tbl != NULL)
        {
            notifyParams.linuxCoreId = CORE_ID_A720_0;
        }

        /* initialize the IPC Notify module */
        status = IpcNotify_init(&notifyParams);

        if(status != SystemP_SUCCESS)
        {
            appLogPrintf("IPC: ERROR: IpcNotify_init failed !!!\n");
        }
    }
    if(status == SystemP_SUCCESS)
    {
        RPMessage_Params rpmsgParams;
        uint32_t src_cpu_id, dst_cpu_id, vringId;
        uint32_t rxTxMap[CORE_ID_MAX][CORE_ID_MAX];
        uint32_t self_core_id = g_app_to_ipc_cpu_id[prm->self_cpu_id];
        /* initialize parameters to default */

        RPMessage_Params_init(&rpmsgParams);

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
                    if (prm->ipc_resource_tbl != NULL)
                    {
                        rxTxMap[src_core_id][dst_core_id] = vringId;
                        vringId++;
                    }
                    else
                    {
                        rxTxMap[src_core_id][dst_core_id] = vringId;
                        vringId++;
                    }
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

        if (prm->ipc_resource_tbl != NULL)
        {
            rpmsgParams.linuxResourceTable = prm->ipc_resource_tbl;
            rpmsgParams.linuxCoreId = CORE_ID_A720_0;
        }

        /* initialize the IPC RP Message module */
        status = RPMessage_init(&rpmsgParams);
        if(status != SystemP_SUCCESS)
        {
            appLogPrintf("IPC: ERROR: RPMessage_init failed !!!\n");
        }
    }
    if(status == SystemP_SUCCESS)
    {
        if (prm->ipc_resource_tbl != NULL)
        {
            appLogPrintf("IPC: Waiting for HLOS to be ready ... !!!\n");
            /* This API MUST be called by applications when its ready to talk to Linux */
            status = RPMessage_waitForLinuxReady(RPMESSAGE_WAIT_FOREVER);
            appLogPrintf("IPC: HLOS is ready !!!\n");
        }
    }

    if(status == SystemP_SUCCESS)
    {
        RPMessage_CreateParams rpmsg_createTx_prms;

        for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
        {
            if(0U != appIpcIsCpuEnabled(cpu_id))
            {
                RPMessage_CreateParams_init(&rpmsg_createTx_prms);

                rpmsg_createTx_prms.localEndPt = (uint16_t)cpu_id;
                status = RPMessage_construct(&obj->rpmsg_tx_handle[cpu_id], &rpmsg_createTx_prms);

                if(status != SystemP_SUCCESS)
                {
                    appLogPrintf("IPC: ERROR: Unable to create rpmessage tx handle for cpu %d !!!\n", cpu_id);
                    break;
                }
            }
        }
    }
    if(status==0)
    {
        RPMessage_CreateParams rpmsg_createRx_prms;
        RPMessage_CreateParams_init(&rpmsg_createRx_prms);
        rpmsg_createRx_prms.localEndPt = (uint16_t)prm->tiovx_rpmsg_port_id;

        status = RPMessage_construct(&obj->rpmsg_rx_handle, &rpmsg_createRx_prms);

        if(SystemP_SUCCESS !=status)
        {
            appLogPrintf("IPC: ERROR: Unable to create rpmessage rx handle !!!\n");
        }

        /* NOTE: RPMessage_setCallback is not yet implemented */
    }

    if((status==0) &&
    (prm->enable_tiovx_ipc_announce == 1u))
    {
        /* use "rpmsg-proto" or "rpmsg_chrdev" depending on protocol selected in user space on Linux A72 */
        status = RPMessage_announce(CORE_ID_A720_0, (uint16_t)prm->tiovx_rpmsg_port_id, "rpmsg_chrdev");

        if(status != 0)
        {
            appLogPrintf("IPC: RPMessage_announce() for rpmsg-proto failed\n");
            status = -1;
        }
    }

    if(status==0)
    {
        status = appIpcCreateRpmsgRxTask(obj);
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: appIpcCreateRpmsgRxTask failed !!!\n");
        }
    }
    appLogPrintf("IPC: Init ... Done !!! status: %d\n", status);

    return status;
}
int32_t appIpcDeInit(void)
{
    int32_t status = 0;
    uint32_t cpu_id;

    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

    appLogPrintf("IPC: Deinit ... !!!\n");

    appIpcDeleteRpmsgRxTask(obj);

    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        RPMessage_destruct(&obj->rpmsg_tx_handle[cpu_id]);
    }

    RPMessage_destruct(&obj->rpmsg_rx_handle);
    IpcNotify_deInit();
    RPMessage_deInit();

    appLogPrintf("IPC: Deinit ... Done !!!\n");

    return status;
}

uint32_t appIpcIsCpuEnabled(uint32_t cpu_id)
{
    uint32_t is_enabled = 0, cur_cpu_id;
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

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

app_ipc_notify_handler_f appIpcGetNotifyHandler(void)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

    return obj->ipc_notify_handler;
}

int32_t appIpcRegisterNotifyHandler(app_ipc_notify_handler_f handler)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

    obj->ipc_notify_handler = handler;

    return status;
}

uint32_t appIpcGetSelfCpuId(void)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj[g_app_vdk_get_cpu_id()];

    return obj->prm.self_cpu_id;
}

void appRtosTaskYield(void)
{
    sched_yield();
    return;
}

void Utils_dataAndInstructionBarrier(void)
{
    return;
}

const char *SOC_getCoreName(uint16_t coreId)
{
    static char *coreIdNames[CORE_ID_MAX+1] = {
        "dmcu0",
        "mcu0",
        "dsp0",
        "dsp1",
        "a720-0",
        "rmcu0-0",
        "mcu1",
        "unknown"
    };
    const char *name;

    if(coreId < CORE_ID_MAX)
    {
        name = coreIdNames[coreId];
    }
    else
    {
        name = coreIdNames[CORE_ID_MAX];
    }
    return name;
}

const char *appIpcGetCpuName(uint32_t app_cpu_id)
{
    const char *name = "invalid";
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        name = SOC_getCoreName((uint16_t)g_app_to_ipc_cpu_id[app_cpu_id]);
    }
    return name;
}

uint32_t appIpcGetIpcCpuId(uint32_t app_cpu_id)
{
    uint32_t ipc_cpu_id = APP_MCUSDK_IPC_CPU_INVALID;
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        ipc_cpu_id = g_app_to_ipc_cpu_id[app_cpu_id];
    }
    return ipc_cpu_id;
}
