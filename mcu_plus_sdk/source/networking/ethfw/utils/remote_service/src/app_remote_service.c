/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include <ti/drv/ipc/ipc.h>
#include <ti/osal/TaskP.h>
#include <ti/osal/SemaphoreP.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/remote_service/src/app_remote_service_priv.h>
#include <utils/console_io/include/app_log.h>

/* #define APP_REMOTE_SERVICE_DEBUG */

#define APP_REMOTE_SERVICE_MAX_TASK_NAME (12u)

/** \brief RPMsg Port used for Remote service */
#define APP_IPC_REMOTE_SERVICE_RPMSG_PORT_ID      (21u)

#define IPC_RPMESSAGE_OBJ_SIZE      (256u)
#define IPC_RPMESSAGE_BUF_SIZE(n)   (IPC_RPMESSAGE_MSG_SIZE*(n)+IPC_RPMESSAGE_OBJ_SIZE)

#define APP_REMOTE_SERVICE_HANDLERS_MAX       (8u)
#define APP_REMOTE_SERVICE_RPMSG_TX_NUM_BUF   (2u)
#define APP_REMOTE_SERVICE_RPMSG_TX_BUF_SIZE  IPC_RPMESSAGE_BUF_SIZE(APP_REMOTE_SERVICE_RPMSG_TX_NUM_BUF)
static uint8_t g_app_remote_service_rpmsg_tx_buf[APP_REMOTE_SERVICE_RPMSG_TX_BUF_SIZE] __attribute__ ((aligned(1024)));

#define APP_REMOTE_SERVICE_RPMSG_RX_NUM_BUF   (8u)
#define APP_REMOTE_SERVICE_RPMSG_RX_BUF_SIZE  IPC_RPMESSAGE_BUF_SIZE(APP_REMOTE_SERVICE_RPMSG_RX_NUM_BUF)
static uint8_t g_app_remote_service_rpmsg_rx_buf[APP_REMOTE_SERVICE_RPMSG_RX_BUF_SIZE] __attribute__ ((aligned(1024)));

/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16KB
 * - AND stack assigned for task context is "size - 8KB"
 *       - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
#define APP_REMOTE_SERVICE_RX_TASK_STACK_SIZE   (128*1024u)
#define APP_REMOTE_SERVICE_RX_TASK_PRI          (10u)
#if defined(SAFERTOS)
#define APP_REMOTE_SERVICE_RX_TASK_STACK_ALIGN  APP_REMOTE_SERVICE_RX_TASK_STACK_SIZE
#else
#define APP_REMOTE_SERVICE_RX_TASK_STACK_ALIGN  (8192u)
#endif

static uint8_t g_app_remote_service_rx_task_stack[APP_REMOTE_SERVICE_RX_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_REMOTE_SERVICE_RX_TASK_STACK_ALIGN)));


typedef struct {

    app_remote_service_init_prms_t prm;
    RPMessage_Handle rpmsg_tx_handle;
    uint32_t rpmsg_tx_endpt;
    RPMessage_Handle rpmsg_rx_handle;
    TaskP_Handle task_handle;
    uint32_t task_stack_size;
    uint8_t *task_stack;
    uint32_t task_pri;
    uint8_t rpmsg_rx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    uint8_t rpmsg_tx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    SemaphoreP_Handle tx_lock;
    SemaphoreP_Handle rx_lock;
    app_remote_service_handler_t handlers[APP_REMOTE_SERVICE_HANDLERS_MAX];
    char service_name[APP_REMOTE_SERVICE_HANDLERS_MAX][APP_REMOTE_SERVICE_NAME_MAX];
    char task_name[APP_REMOTE_SERVICE_MAX_TASK_NAME];
} app_remote_service_obj_t;

app_remote_service_obj_t g_app_remote_service_obj;

static int32_t appRemoteServiceRunHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t i, is_found = 0;

    SemaphoreP_pend(obj->rx_lock, SemaphoreP_WAIT_FOREVER);
    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        if(obj->handlers[i]!=NULL && (strcmp(obj->service_name[i], service_name)==0))
        {
            is_found = 1;
            status = obj->handlers[i](service_name, cmd, prm, prm_size, flags);
            break;
        }
    }
    SemaphoreP_post(obj->rx_lock);
    if(!is_found)
    {
        appLogPrintf("REMOTE_SERVICE: ERROR: Unable to find handler for service [%s]\n", service_name);
    }
    return status;
}

static void appRemoteServiceRxTaskMain(void *arg0, void *arg1)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t done = 0, reply_endpt, src_cpu_id;
    uint16_t len;

    while(!done)
    {
        len = 0;
        status = RPMessage_recv(obj->rpmsg_rx_handle,
                        &obj->rpmsg_rx_msg_buf,
                        &len,
                        &reply_endpt,
                        &src_cpu_id,
                        SemaphoreP_WAIT_FOREVER
                        );

        if(status == IPC_E_UNBLOCKED)
        {
            done = 1;
        }
        if(status == IPC_SOK)
        {
            app_service_msg_header_t *header;

            header = (app_service_msg_header_t *)&obj->rpmsg_rx_msg_buf[0];

            #ifdef APP_REMOTE_SERVICE_DEBUG
            appLogPrintf("REMOTE_SERVICE: RX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... !!!\n",
                         Ipc_mpGetName(src_cpu_id),
                         reply_endpt,
                         Ipc_mpGetSelfName(),
                         obj->prm.rpmsg_rx_endpt,
                         header->cmd, header->prm_size);
            #endif

            header->status = appRemoteServiceRunHandler(
                                (char*)header->service_name,
                                header->cmd,
                                &obj->rpmsg_rx_msg_buf[sizeof(app_service_msg_header_t)],
                                header->prm_size,
                                header->flags);
            if(header->flags & APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK)
            {
                /* No need to send ACK */
            }
            else
            {
                /* send ack */
                status = RPMessage_send(
                            obj->rpmsg_rx_handle,
                            src_cpu_id,
                            reply_endpt,
                            obj->prm.rpmsg_rx_endpt,
                            &obj->rpmsg_rx_msg_buf,
                            len
                        );
                if(status!=0)
                {
                    appLogPrintf("REMOTE_SERVICE: TX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... Failed !!!\n",
                                 Ipc_mpGetSelfName(),
                                 obj->prm.rpmsg_rx_endpt,
                                 Ipc_mpGetName(src_cpu_id),
                                 reply_endpt,
                                 header->cmd, header->prm_size);
                }
                else
                {
                    #ifdef APP_REMOTE_SERVICE_DEBUG
                    appLogPrintf("REMOTE_SERVICE: TX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... !!!\n",
                                 Ipc_mpGetSelfName(),
                                 obj->prm.rpmsg_rx_endpt,
                                 Ipc_mpGetName(src_cpu_id),
                                 reply_endpt,
                                 header->cmd, header->prm_size);
                    #endif
                }
            }
        }
    }
}

int32_t appRemoteServiceRun(uint32_t dst_app_cpu_id, char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = 0;

    if(prm_size > APP_REMOTE_SERVICE_PRM_SIZE_MAX)
    {
        appLogPrintf("REMOTE_SERVICE: ERROR: Parameter size of %d bytes exceeds message buffer size of %ld bytes\n",
                     prm_size, APP_REMOTE_SERVICE_PRM_SIZE_MAX);
        status = -1;
    }

    if(status==0)
    {
        if(dst_app_cpu_id == Ipc_mpGetSelfId())
        {
            /* destination CPU is self CPU so call the handler locally */
            status = appRemoteServiceRunHandler(service_name, cmd, prm, prm_size, flags);
        }
        else
        {
            uint16_t tx_payload_size, rx_payload_size;
            uint32_t rx_endpt, rx_cpu_id;
            uint32_t dst_ipc_cpu_id;
            app_service_msg_header_t *header;

            if(prm==NULL)
                prm_size = 0;

            dst_ipc_cpu_id = Ipc_mpGetRemoteProcId(dst_app_cpu_id);

            /* take a lock since to make this call thread safe */
            SemaphoreP_pend(obj->tx_lock, SemaphoreP_WAIT_FOREVER);

            /* copy content to temp buffer */
            header = (app_service_msg_header_t *)&obj->rpmsg_tx_msg_buf[0];
            strncpy((char*)header->service_name, service_name, APP_REMOTE_SERVICE_NAME_MAX);
            header->service_name[APP_REMOTE_SERVICE_NAME_MAX-1]=0;
            header->cmd = cmd;
            header->flags = flags;
            header->status = status;
            header->prm_size = prm_size;
            if(prm!=NULL)
            {
                memcpy(&obj->rpmsg_tx_msg_buf[sizeof(app_service_msg_header_t)], prm  , prm_size);
            }
            tx_payload_size = prm_size + sizeof(app_service_msg_header_t);

            #ifdef APP_REMOTE_SERVICE_DEBUG
            appLogPrintf("REMOTE_SERVICE: TX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ...\n",
                         Ipc_mpGetSelfName(),
                         obj->rpmsg_tx_endpt,
                         Ipc_mpGetName(dst_ipc_cpu_id),
                         obj->prm.rpmsg_rx_endpt,
                         cmd, prm_size);
            #endif
            /* send to destination */
            status = RPMessage_send(
                        obj->rpmsg_tx_handle,
                        dst_ipc_cpu_id,
                        obj->prm.rpmsg_rx_endpt,
                        obj->rpmsg_tx_endpt,
                        &obj->rpmsg_tx_msg_buf,
                        tx_payload_size
                        );
            if(status!=0)
            {
                appLogPrintf("REMOTE_SERVICE: TX: FAILED: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes\n",
                             Ipc_mpGetSelfName(),
                             obj->rpmsg_tx_endpt,
                             Ipc_mpGetName(dst_ipc_cpu_id),
                             obj->prm.rpmsg_rx_endpt,
                             cmd, prm_size);
            }
            if(status==0)
            {
                if(flags & APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK)
                {
                    /* No need to wait for ACK */
                }
                else
                {
                    /* wait for ACK from destination */
                    rx_payload_size = 0;
                    status = RPMessage_recv(obj->rpmsg_tx_handle,
                                    &obj->rpmsg_tx_msg_buf,
                                    &rx_payload_size,
                                    &rx_endpt,
                                    &rx_cpu_id,
                                    SemaphoreP_WAIT_FOREVER
                                    );
                    if(status == IPC_SOK
                        && rx_payload_size == tx_payload_size
                        && rx_endpt == obj->prm.rpmsg_rx_endpt
                        && rx_cpu_id == dst_ipc_cpu_id)
                    {
                        status = header->status;
                        if(prm!=NULL)
                        {
                            memcpy(prm, &obj->rpmsg_tx_msg_buf[sizeof(app_service_msg_header_t)], prm_size);
                        }

                        #ifdef APP_REMOTE_SERVICE_DEBUG
                        appLogPrintf("REMOTE_SERVICE: TX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... Done !!!\n",
                                     Ipc_mpGetSelfName(),
                                     obj->rpmsg_tx_endpt,
                                     Ipc_mpGetName(dst_ipc_cpu_id),
                                     obj->prm.rpmsg_rx_endpt,
                                     cmd, prm_size);
                        #endif
                    }
                    else
                    {
                        appLogPrintf("REMOTE_SERVICE: TX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... Failed !!!\n",
                                     Ipc_mpGetSelfName(),
                                     obj->rpmsg_tx_endpt,
                                     Ipc_mpGetName(dst_ipc_cpu_id),
                                     obj->prm.rpmsg_rx_endpt,
                                     cmd, prm_size);
                    }
                }
            }
            /* take a lock since to make this call thread safe */
            SemaphoreP_post(obj->tx_lock);
        }
    }

    return status;
}

static int32_t appRemoteServiceCreateRpmsgRxTask(app_remote_service_obj_t *obj)
{
    TaskP_Params task_prms;
    int32_t status = 0;

    TaskP_Params_init(&task_prms);

    task_prms.stacksize = obj->task_stack_size;
    task_prms.stack = obj->task_stack;
    task_prms.priority = obj->task_pri;
    task_prms.arg0 = (void*)(obj);
    task_prms.arg1 = NULL;
    task_prms.name = (const char*)&obj->task_name[0];

    strncpy(obj->task_name, "REMOTE_SRV", APP_REMOTE_SERVICE_MAX_TASK_NAME);
    obj->task_name[APP_REMOTE_SERVICE_MAX_TASK_NAME-1] = 0;

    obj->task_handle = (void*)TaskP_create(
                            &appRemoteServiceRxTaskMain,
                            &task_prms);
    if(obj->task_handle==NULL)
    {
        appLogPrintf("REMOTE_SERVICE: ERROR: Unable to create RX task \n");
        status = -1;
    }
    else
    {
        appPerfStatsRegisterTask(obj->task_handle, obj->task_name);
    }
    return status;
}

static void appRemoteServiceDeleteRpmsgRxTask(app_remote_service_obj_t *obj)
{
    uint32_t sleep_time = 16U;

    RPMessage_unblock(obj->rpmsg_rx_handle);

    /* confirm task termination */
    while ( ! TaskP_isTerminated(obj->task_handle) )
    {
        TaskP_sleepInMsecs(sleep_time);
        sleep_time >>= 1U;
        if (sleep_time == 0U)
        {
            /* Force delete after timeout */
            break;
        }
    }
    TaskP_delete(&obj->task_handle);
}

void appRemoteServiceInitSetDefault(app_remote_service_init_prms_t *prm)
{
    prm->rpmsg_rx_endpt = APP_IPC_REMOTE_SERVICE_RPMSG_PORT_ID;
}

int32_t appRemoteServiceInit(app_remote_service_init_prms_t *prm)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = 0;
    SemaphoreP_Params semParams;
    uint32_t i;

    appLogPrintf("REMOTE_SERVICE: Init ... !!!\n");

    obj->prm = *prm;
    obj->rpmsg_tx_handle = NULL;
    obj->rpmsg_rx_handle = NULL;
    obj->tx_lock = NULL;
    obj->rx_lock = NULL;
    obj->task_handle = NULL;
    obj->task_stack = g_app_remote_service_rx_task_stack;
    obj->task_stack_size = APP_REMOTE_SERVICE_RX_TASK_STACK_SIZE;
    obj->task_pri = APP_REMOTE_SERVICE_RX_TASK_PRI;

    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        obj->handlers[i] = NULL;
        obj->service_name[i][0] = 0;
    }

    SemaphoreP_Params_init(&semParams);
    semParams.mode = SemaphoreP_Mode_BINARY;
    obj->tx_lock = SemaphoreP_create(1U, &semParams);
    if(obj->tx_lock==NULL)
    {
        appLogPrintf("REMOTE_SERVICE: Unable to create tx semaphore\n");
        status = -1;
    }
    if(status==0)
    {
        SemaphoreP_Params_init(&semParams);
        semParams.mode = SemaphoreP_Mode_BINARY;
        obj->rx_lock = SemaphoreP_create(1U, &semParams);
        if(obj->rx_lock==NULL)
        {
            appLogPrintf("REMOTE_SERVICE: Unable to create rx semaphore\n");
            status = -1;
        }
    }
    if(status==0)
    {
        RPMessage_Params rpmsg_prm;

        RPMessageParams_init(&rpmsg_prm);

        rpmsg_prm.requestedEndpt = RPMESSAGE_ANY;
        rpmsg_prm.buf = g_app_remote_service_rpmsg_tx_buf;
        rpmsg_prm.bufSize = APP_REMOTE_SERVICE_RPMSG_TX_BUF_SIZE;
        rpmsg_prm.numBufs = APP_REMOTE_SERVICE_RPMSG_TX_NUM_BUF;

        obj->rpmsg_tx_handle =
            RPMessage_create(&rpmsg_prm, &obj->rpmsg_tx_endpt);

        if(obj->rpmsg_tx_handle==NULL)
        {
            appLogPrintf("REMOTE_SERVICE: ERROR: Unable to create rpmessage tx handle !!!\n");
            status = -1;
        }
    }
    if(status==0)
    {
        RPMessage_Params rpmsg_prm;
        uint32_t rx_endpt;

        RPMessageParams_init(&rpmsg_prm);

        rpmsg_prm.requestedEndpt = prm->rpmsg_rx_endpt;
        rpmsg_prm.buf = g_app_remote_service_rpmsg_rx_buf;
        rpmsg_prm.bufSize = APP_REMOTE_SERVICE_RPMSG_RX_BUF_SIZE;
        rpmsg_prm.numBufs = APP_REMOTE_SERVICE_RPMSG_RX_NUM_BUF;

        obj->rpmsg_rx_handle =
            RPMessage_create(&rpmsg_prm, &rx_endpt);

        if(obj->rpmsg_rx_handle==NULL)
        {
            appLogPrintf("REMOTE_SERVICE: ERROR: Unable to create rpmessage rx handle !!!\n");
            status = -1;
        }

        /* annouce service to linux side */
        status = RPMessage_announce(RPMESSAGE_ALL, prm->rpmsg_rx_endpt, "rpmsg_chrdev");
        if(status != 0)
        {
            appLogPrintf("REMOTE_SERVICE: RPMessage_announce() for rpmsg-proto failed\n");
            status = -1;
        }
    }
    if(status==0)
    {
        status = appRemoteServiceCreateRpmsgRxTask(obj);
        if(status!=0)
        {
            appLogPrintf("REMOTE_SERVICE: ERROR: appRemoteServiceCreateRpmsgRxTask failed !!!\n");
        }
    }
    appLogPrintf("REMOTE_SERVICE: Init ... Done !!!\n");

    return status;
}

int32_t appRemoteServiceRegister(char *service_name, app_remote_service_handler_t handler)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t i;

    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        if(obj->handlers[i]==NULL)
        {
            strncpy(obj->service_name[i], service_name, APP_REMOTE_SERVICE_NAME_MAX);
            obj->service_name[i][APP_REMOTE_SERVICE_NAME_MAX-1]=0;
            obj->handlers[i] = handler;
            status = 0;
            break;
        }
    }
    if(status!=0)
    {
        appLogPrintf("REMOTE_SERVICE: ERROR: Unable to register handler for service [%s]\n", service_name);
    }

    return status;
}

int32_t appRemoteServiceUnRegister(char *service_name)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t i;

    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        if(strcmp(service_name, obj->service_name[i])==0)
        {
            obj->service_name[i][0] = 0;
            obj->handlers[i] = NULL;
            status = 0;
            break;
        }
    }
    if(status!=0)
    {
        appLogPrintf("REMOTE_SERVICE: ERROR: Unable to unregister handler for service [%s]\n", service_name);
    }

    return status;
}

int32_t appRemoteServiceDeInit()
{
    int32_t status = 0;
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;

    appLogPrintf("REMOTE_SERVICE: Deinit ... !!!\n");

    appRemoteServiceDeleteRpmsgRxTask(obj);

    if(obj->rpmsg_tx_handle!=NULL)
    {
        RPMessage_delete(&obj->rpmsg_tx_handle);
        obj->rpmsg_tx_handle = NULL;
    }
    if(obj->rpmsg_rx_handle!=NULL)
    {
        RPMessage_delete(&obj->rpmsg_rx_handle);
        obj->rpmsg_rx_handle = NULL;
    }
    SemaphoreP_delete(obj->tx_lock);
    SemaphoreP_delete(obj->rx_lock);

    appLogPrintf("REMOTE_SERVICE: Deinit ... Done !!!\n");

    return status;
}
