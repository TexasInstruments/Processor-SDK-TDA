/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2026
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(PDK)
#include <ipc/ipc.h>
#include <osal.h>
#elif defined(MCU_PLUS_SDK)
#include <ClockP.h>
#include <ipc_rpmsg.h>
#include <ipc_notify.h>
#include <utils/ipc/include/mcu_sdk_ipc.h>
static uint16_t grpmsg_endpt = 0;
#if defined(THREADX)
#include <tx_port.h>
#endif
#elif defined(MCU_SDK)
#include <Ipc_Notify_Hal_Cfg.h>
#include <RPMessage.h>
#include <Ipc_Notify.h>
#include <ClockP.h>
#include <utils/ipc/include/mcu_sdk_ipc.h>
static uint16_t grpmsg_endpt = 0;
#endif

#include <utils/console_io/include/app_log.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/misc/include/app_misc.h>
#include <utils/rtos/include/app_rtos.h>

/* #define APP_IPC_ECHO_TEST_DEBUG */

#define MSGSIZE  512u
#define SERVICE  "ti.ipc4.ping-pong"
#define ENDPT1   APP_IPC_ECHO_TEST_RPMSG_PORT_ID
#define NUMMSGS  2   /* number of message sent per task */

/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16KB
 * - AND stack assigned for task context is "size - 8KB"
 * - 8KB chunk for the stack area is used for interrupt handling in this task context
 */

#if defined(R5F) || defined(R52P) || defined(M55)
#define APP_IPC_ECHO_TEST_TASK_STACKSIZE       (4*1024)

#elif defined(C7X_FAMILY)
#if defined (SOC_AM62A)
#define APP_IPC_ECHO_TEST_TASK_STACKSIZE       (64*1024)
#else
#define APP_IPC_ECHO_TEST_TASK_STACKSIZE       (32*1024)
#endif

#else
#define APP_IPC_ECHO_TEST_TASK_STACKSIZE       (32*1024)
#endif

#if defined(R5F) || defined(R52P) || defined(M55)

#if defined(SAFERTOS)
#define APP_IPC_ECHO_TEST_TASK_ALIGNMENT    (32*1024)
#define APP_IPC_SENDBUF_TASK_ALIGNMENT      (262144u)
#define APP_IPC_RSPBUF_TASK_ALIGNMENT       (16384u)
#else
#define APP_IPC_ECHO_TEST_TASK_ALIGNMENT    (8192u)
#define APP_IPC_SENDBUF_TASK_ALIGNMENT      (128u)
#define APP_IPC_RSPBUF_TASK_ALIGNMENT       (128u)
#endif

#elif defined(C7X_FAMILY)
#define APP_IPC_ECHO_TEST_TASK_ALIGNMENT    (8192u)
#define APP_IPC_SENDBUF_TASK_ALIGNMENT      (128u)
#define APP_IPC_RSPBUF_TASK_ALIGNMENT       (128u)

#else
#define APP_IPC_ECHO_TEST_TASK_ALIGNMENT    (8192u)
#define APP_IPC_SENDBUF_TASK_ALIGNMENT      (128u)
#define APP_IPC_RSPBUF_TASK_ALIGNMENT       (128u)
#endif

/* this should be >= RPMessage_getObjMemRequired() */
#define IPC_RPMESSAGE_OBJ_SIZE  (256u)

#if defined (MCU_PLUS_SDK) || defined (MCU_SDK)
#define RPMSG_DATA_SIZE         ((IPC_RPMESSAGE_NUM_VRING_BUF * MSGSIZE) + IPC_RPMESSAGE_OBJ_SIZE)
#elif defined(PDK)
#define RPMSG_DATA_SIZE         (256u * MSGSIZE + IPC_RPMESSAGE_OBJ_SIZE)
#endif

#define APP_IPC_ECHO_TEST_MAX_TASK_NAME     (12u)
#define BUF_SIZE                            (1024u)

static uint8_t  g_taskStackBuf[APP_IPC_CPU_MAX][APP_IPC_ECHO_TEST_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_IPC_ECHO_TEST_TASK_ALIGNMENT)))
    ;

#if defined(PDK)
static uint8_t  g_sendBuf[(uint32_t)RPMSG_DATA_SIZE * APP_IPC_CPU_MAX]  __attribute__ ((aligned (APP_IPC_SENDBUF_TASK_ALIGNMENT)));
static uint8_t  g_rspBuf[RPMSG_DATA_SIZE]  __attribute__ ((aligned (APP_IPC_RSPBUF_TASK_ALIGNMENT)));
#endif

uint32_t g_ipc_echo_test_status[APP_IPC_CPU_MAX];
static app_rtos_semaphore_handle_t g_ipc_echo_test_lock;

static char g_rpmsg_responder_task_name[APP_IPC_ECHO_TEST_MAX_TASK_NAME];
static char g_rpmsg_sender_task_name[APP_IPC_CPU_MAX][APP_IPC_ECHO_TEST_MAX_TASK_NAME];

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
#endif

#if defined (SOC_TDA54)
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
#endif

#if defined (SOC_AM62A)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_R5FSS0_0,
    CSL_CORE_ID_C75SS0_0
};
#endif

#if defined (SOC_J722S)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_WKUP_R5FSS0_0,
    CSL_CORE_ID_MAIN_R5FSS0_0,
    CSL_CORE_ID_C75SS0_0,
    CSL_CORE_ID_C75SS1_0
};
#endif

static void appIpcEchoTestLock(void)
{
    if(g_ipc_echo_test_lock!=NULL)
    {
        appRtosSemaphorePend(g_ipc_echo_test_lock, APP_RTOS_SEMAPHORE_WAIT_FOREVER);
    }
}

static void appIpcEchoTestUnLock(void)
{
    if(g_ipc_echo_test_lock!=NULL)
    {
        appRtosSemaphorePost(g_ipc_echo_test_lock);
    }
}

static void appIpcEchoTestStatusUpdate(uint32_t dst_ipc_lld_cpu_id, uint32_t value)
{
    appIpcEchoTestLock();
    if(dst_ipc_lld_cpu_id < APP_IPC_CPU_MAX)
    {
        g_ipc_echo_test_status[dst_ipc_lld_cpu_id] = value;
    }
    appIpcEchoTestUnLock();
}

static void appIpcEchoTestStatusShow(void)
{
    char buf[BUF_SIZE];
    uint32_t count = 0;
    uint32_t cpu_id;
    uint32_t all_pass;

    appIpcEchoTestLock();

    buf[count] = (char)0;

    all_pass = 1;

    count += (uint32_t)snprintf(&buf[count], BUF_SIZE-count, "IPC: Echo status: ");
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            char status_char = 's';

            if(cpu_id!=appIpcGetSelfCpuId())
            {
                if(g_ipc_echo_test_status[cpu_id]==(uint32_t)-1)
                {
                    status_char = 'x';
                    all_pass = 0;
                }
                else
                if(g_ipc_echo_test_status[cpu_id]==(uint32_t)NUMMSGS)
                {
                    status_char = 'P';
                }
                else
                {
                    status_char = '.';
                    all_pass = 0;
                }
            }
            if (NULL != Ipc_mpGetName(g_app_to_ipc_cpu_id[cpu_id]))
            {
                count += (uint32_t)snprintf(&buf[count], BUF_SIZE-count, "%s[%c] ",
                    Ipc_mpGetName(g_app_to_ipc_cpu_id[cpu_id]),
                    status_char
                    );
            }
        }
    }
    if(all_pass)
    {
        count += (uint32_t)snprintf(&buf[count], BUF_SIZE-count, " - ALL PASSED !!!\n");
    }
    else
    {
        count += (uint32_t)snprintf(&buf[count], BUF_SIZE-count, "\n");
    }

    appLogPrintf(buf);
    appIpcEchoTestUnLock();
}

/*
 * This "Task" waits for a "ping" message from any processor
 * then replies with a "pong" message.
 */
static void rpmsg_responderFxn(void* arg0, void* arg1)
{
    #if defined(PDK)
    RPMessage_Handle    handle;
    RPMessage_Params    params;
    uint32_t    myEndPt = 0;
    uint32_t    remoteProcId;
    void        *buf;
    uint32_t    bufSize = RPMSG_DATA_SIZE;
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_Object*    handle;
    RPMessage_CreateParams params;
    uint32_t    myEndPt = 0;
    uint16_t    remoteProcId;
    RPMessage_Object    rpMsgRspObject;
    #endif
    uint16_t    len;
    uint32_t    remoteEndPt;
    uint32_t    count;
    int32_t     n;
    int32_t     status = 0;
    char        str[MSGSIZE];

    appUtilsTaskInit();

    #ifdef APP_IPC_ECHO_TEST_DEBUG
    appLogPrintf("IPC: RecvTask: Started ...\n");
    #endif

    #if defined(PDK)
    buf = g_rspBuf;
    RPMessageParams_init(&params);
    params.requestedEndpt = ENDPT1;
    params.buf = buf;
    params.bufSize = bufSize;

    handle = RPMessage_create(&params, &myEndPt);
    if(!handle)
    {
        appLogPrintf("IPC: RecvTask: Failed to create endpoint\n");
        status = -1;
    }
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_CreateParams_init(&params);
    params.localEndPt = ENDPT1;
    status = RPMessage_construct(&rpMsgRspObject, &params);
    myEndPt = params.localEndPt;
    if(status != SystemP_SUCCESS)
    {
        appLogPrintf("IPC: RecvTask: Failed to create endpoint\n");
    }
    else
    {
        handle = &rpMsgRspObject;
    }
    #endif

    if (status == 0)
    {
        #if defined(PDK)
        status = RPMessage_announce(RPMESSAGE_ALL, myEndPt, SERVICE);
        if(status != 0)
        {
            appLogPrintf("IPC: RecvTask: RPMessage_announce() failed\n");
            status = -1;
        }
        #elif defined(MCU_PLUS_SDK)
        status = RPMessage_announce(CSL_CORE_ID_A53SS0_0, myEndPt, SERVICE);
        #elif defined(MCU_SDK)
        status = RPMessage_announce(CORE_ID_A720_0, myEndPt, SERVICE);
        #endif

        #if defined(MCU_PLUS_SDK) || defined(MCU_SDK)
        if(status != 0)
        {
            appLogPrintf("IPC: RecvTask: RPMessage_announce() failed\n");
            status = -1;
        }
        /* wait for all cores to be ready */
        /* Note: the syncAll command is not needed for QNX */
        if (APP_HOST_TYPE_LINUX == appGetHostOSType())
        {
            IpcNotify_syncAll(SystemP_WAIT_FOREVER);
        }
        else
        {
            ClockP_usleep(500*1000); /* wait for log messages from remote cores to be flushed, otherwise this delay is not needed */
        }
        #endif
        count = 0;

        #ifdef APP_IPC_ECHO_TEST_DEBUG
        appLogPrintf("IPC: RecvTask: Waiting for messages ...\n");
        #endif

        while(1)
        {
            #if defined(PDK)
            status = RPMessage_recv(handle, (void*)str, &len, &remoteEndPt, &remoteProcId,
            IPC_RPMESSAGE_TIMEOUT_FOREVER);
            #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
            len = sizeof(str);
            status = RPMessage_recv(handle, (void*)str, &len, &remoteProcId, &remoteEndPt,
                                SystemP_WAIT_FOREVER);
            #endif
            if(status != IPC_SOK)
            {
                appLogPrintf("IPC: RecvTask: failed with code %d\n", status);
            }
            else
            {
                count++;
                /* NULL terminated string */
                str[len] = '\0';
                #ifdef APP_IPC_ECHO_TEST_DEBUG
                appLogPrintf("IPC: RecvTask: Revcvd msg #%d \"%s\" len %d bytes from %s\n",
                    count, str, len, Ipc_mpGetName(remoteProcId));
                #endif
            }

            status = sscanf(str, "ping %d", &n);
            if(status == 1)
            {
                len = (uint16_t)snprintf(str, MSGSIZE-(1u), "pong %d", n);
            }
            str[len++] = '\0';

            #ifdef APP_IPC_ECHO_TEST_DEBUG
            appLogPrintf("IPC: RecvTask: Sending msg #%d \"%s\" len %d bytes from %s to %s\n",
                count, str, len, Ipc_mpGetSelfName(),
                Ipc_mpGetName(remoteProcId));
            #endif

            #if defined(PDK)
            status = RPMessage_send(handle, remoteProcId, remoteEndPt, myEndPt, str, len);
            #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
            status = RPMessage_send(str, len, remoteProcId, remoteEndPt, myEndPt, SystemP_WAIT_FOREVER);
            #endif

            if (status != IPC_SOK)
            {
                appLogPrintf("IPC: RecvTask: RPMessage_send "
                    " failed, status %d procId %d\n", status, remoteProcId);
            }
        }
    }

    /* Delete the RPMesg object now */
    #if defined(PDK)
    RPMessage_delete(&handle);
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_destruct(handle);
    #if !defined(THREADX)
    #if defined(SAFERTOS)
    #if defined(R5F)
    signed long xTaskDelete( void* xTaskToDelete );
    xTaskDelete(NULL);
    #else
    signed long long xTaskDeleteKrnl( void* xTaskToDelete );
    xTaskDeleteKrnl(NULL);
    #endif
    #else
    void vTaskDelete( void* xTaskToDelete );
    vTaskDelete(NULL);
    #endif
    #endif
    #endif

}

static void rpmsg_senderFxn(void* arg0, void* arg1)
{
    #if defined(PDK)
    RPMessage_Handle    handle;
    RPMessage_Params    params;
    uint32_t            myEndPt = 0;
    uint32_t            remoteProcId;
    uint8_t            *buf1;
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_Object*    handle;
    RPMessage_CreateParams params;
    uint32_t            myEndPt = 0;
    uint16_t            remoteProcId;
    RPMessage_Object    rpMsgSendObject;
    #endif
    uint32_t            remoteEndPt;
    uint16_t            dstProc;
    uint32_t            appDstProcId;
    uint16_t            len;
    int32_t             i;
    int32_t             status = 0;
    char                buf[MSGSIZE];

    appUtilsTaskInit();

    #if defined(PDK)
    dstProc = (uint16_t)((uintptr_t)arg1);
    appDstProcId = (uint32_t)((uintptr_t)arg0);
    buf1 = &g_sendBuf[RPMSG_DATA_SIZE * appDstProcId];
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    appDstProcId = (uint16_t)((uintptr_t)arg0);
    dstProc = g_app_to_ipc_cpu_id[appDstProcId];
    #endif

    /* appIpcEchoTestStatusShow(); */

    #ifdef APP_IPC_ECHO_TEST_DEBUG
    appLogPrintf("IPC: SendTask%d: Started ... \n", dstProc);
    #endif

    /* Create the endpoint for receiving. */
    #if defined(PDK)
    RPMessageParams_init(&params);
    params.numBufs = 2;
    params.buf = buf1;
    params.bufSize = RPMSG_DATA_SIZE;
    handle = RPMessage_create(&params, &myEndPt);
    if(!handle)
    {
        appLogPrintf("IPC: SendTask%d: Failed to create message endpoint\n",
             dstProc);
        status = -1;
    }

    status = RPMessage_getRemoteEndPt(dstProc, SERVICE, &remoteProcId,
                             &remoteEndPt, APP_RTOS_SEMAPHORE_WAIT_FOREVER);

    if(dstProc != remoteProcId)
    {
        appLogPrintf("IPC: SendTask%d: RPMessage_getRemoteEndPt() malfunctioned, status %d\n",
             dstProc, status);
        status = -1;
    }
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_CreateParams_init(&params);
    params.localEndPt = RPMESSAGE_LOCAL_ENDPT + grpmsg_endpt;
    grpmsg_endpt ++;
    status = RPMessage_construct(&rpMsgSendObject, &params);
    myEndPt = params.localEndPt;
    if(status != SystemP_SUCCESS)
    {
        appLogPrintf("IPC: SendTask: Failed to create endpoint\n");
    }
    else
    {
        handle = &rpMsgSendObject;
    }

    /* wait for all cores to be ready */
    /* Note: the syncAll command is not needed for QNX */
    if (APP_HOST_TYPE_LINUX == appGetHostOSType())
    {
        IpcNotify_syncAll(SystemP_WAIT_FOREVER);
    }
    else
    {
        ClockP_usleep(500*1000); /* wait for log messages from remote cores to be flushed, otherwise this delay is not needed */
    }
    #endif

    if (status == 0)
    {
        appIpcEchoTestStatusUpdate(appDstProcId, 0);
        /* appIpcEchoTestStatusShow(); */

        #ifdef APP_IPC_ECHO_TEST_DEBUG
        appLogPrintf("IPC: SendTask%d: Sending messages to %s ... \n", dstProc, Ipc_mpGetName(dstProc));
        #endif

        for (i = 0; i < NUMMSGS; i++)
        {
            /* Send data to remote endPt: */
            len = (uint16_t)snprintf(buf, MSGSIZE-(1u), "ping %d", i);
            buf[len++] = '\0';
            #ifdef APP_IPC_ECHO_TEST_DEBUG
            appLogPrintf("IPC: SendTask%d: Sending msg #%d \"%s\" from %s to %s...\n", dstProc,
                    i, buf, Ipc_mpGetSelfName(),
                    Ipc_mpGetName(dstProc));
            #endif
            #if defined(PDK)
            status = RPMessage_send(handle, dstProc, ENDPT1, myEndPt, (Ptr)buf, len);

            #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
            status = RPMessage_send(buf, len, dstProc, ENDPT1, myEndPt, SystemP_WAIT_FOREVER);
            #endif
            if (status != IPC_SOK)
            {
                appLogPrintf("IPC: SendTask%d: rpmsg_senderFxn: RPMessage_send "
                " failed status %d\n", dstProc, status);
            }
            if (status == IPC_SOK)
            {
                /* wait a for a response message: */
                #if defined(PDK)
                status = RPMessage_recv(handle, (void*)buf, &len, &remoteEndPt,
                                 &remoteProcId, IPC_RPMESSAGE_TIMEOUT_FOREVER);

                #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
                len = sizeof(buf);
                status = RPMessage_recv(handle, (void*)buf, &len, &remoteProcId, &remoteEndPt,
                                    SystemP_WAIT_FOREVER);
                #endif

                if(status != IPC_SOK)
                {
                    appLogPrintf("IPC: SendTask%d: RPMessage_recv failed with status %d\n",
                            dstProc, status);
                }

                /* Make it NULL terminated string */
                if(len >= MSGSIZE)
                {
                    buf[MSGSIZE-(1u)] = '\0';
                }
                else
                {
                    buf[len] = '\0';
                }
                if(status==IPC_SOK)
                {
                    #ifdef APP_IPC_ECHO_TEST_DEBUG
                    appLogPrintf("IPC: SendTask%d: Received msg #%d \"%s\" len %d bytes from %s endPt %d \n",
                        dstProc, i, buf, len, Ipc_mpGetName(remoteProcId),
                        remoteEndPt);
                    #endif
                    appIpcEchoTestStatusUpdate(appDstProcId, (uint32_t)i+1u);
                }
            }
        }
        #ifdef APP_IPC_ECHO_TEST_DEBUG
        appLogPrintf("IPC: SendTask%d: Finished test with %s\n", dstProc,
             Ipc_mpGetName(dstProc));
        #endif
        appIpcEchoTestStatusShow();
    }


    /* Delete the RPMesg object now */
    #if defined(PDK)
    RPMessage_delete(&handle);
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    RPMessage_destruct(handle);
    #if !defined(THREADX)
    #if defined(SAFERTOS)
    #if defined(R5F)
    signed long xTaskDelete( void* xTaskToDelete );
    xTaskDelete(NULL);
    #else
    signed long long xTaskDeleteKrnl( void* xTaskToDelete );
    xTaskDeleteKrnl(NULL);
    #endif
    #else
    void vTaskDelete( void* xTaskToDelete );
    vTaskDelete(NULL);
    #endif
    #endif
    #endif
}

int32_t appIpcEchoTestStart(void)
{
    uint32_t          cpu_id;
    app_rtos_task_params_t      params;
    uint32_t          numProc = APP_IPC_CPU_MAX;
    int32_t status = 0;
    app_rtos_semaphore_params_t semParams;
    app_rtos_task_handle_t rx_task, tx_task[APP_IPC_CPU_MAX];

    appLogPrintf("IPC: Starting echo test ...\n");

    /* Default parameter initialization */
    appRtosSemaphoreParamsInit(&semParams);

    semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
    semParams.initValue = 1U;

    g_ipc_echo_test_lock = appRtosSemaphoreCreate(semParams);
    if(g_ipc_echo_test_lock==NULL)
    {
        appLogPrintf("IPC: ERROR: Failed to create lock !!!\n");
        status = -1;
    }

    if(status==0)
    {
        for(cpu_id = 0; cpu_id < APP_IPC_CPU_MAX; cpu_id++)
        {
            appIpcEchoTestStatusUpdate(cpu_id, (uint32_t)-1);
        }

        #ifdef APP_IPC_ECHO_TEST_DEBUG
        appLogPrintf("IPC: RecvTask: Creating ...\n");
        #endif

        /* Respond to messages coming in to endPt ENDPT1 */
        cpu_id = appIpcGetSelfCpuId();

        appRtosTaskParamsInit(&params);
        #if defined THREADX
        params.priority = TX_MAX_PRIORITIES-4;
        #else
        params.priority = 3;
        #endif
        params.stack     = g_taskStackBuf[cpu_id];
        params.stacksize = APP_IPC_ECHO_TEST_TASK_STACKSIZE;
        params.arg0 = 0;
        params.name = (const char*)&g_rpmsg_responder_task_name[0];
        params.taskfxn   = &rpmsg_responderFxn;

        (void)strncpy(g_rpmsg_responder_task_name, "IPC_TEST_RX", APP_IPC_ECHO_TEST_MAX_TASK_NAME -1U);
        g_rpmsg_responder_task_name[APP_IPC_ECHO_TEST_MAX_TASK_NAME-1u] = (char)0;

        rx_task = appRtosTaskCreate(&params);
        if(rx_task==NULL)
        {
            appLogPrintf("IPC: ERROR: Failed to create RX task !!!\n");
            status = -1;
        }

        if(status==0)
        {
            for(cpu_id = 0; cpu_id < numProc; cpu_id++)
            {
                if((cpu_id != appIpcGetSelfCpuId()) && appIpcIsCpuEnabled(cpu_id))
                {
                    uint32_t ipc_lld_cpu_id = g_app_to_ipc_cpu_id[cpu_id];

                    #ifdef APP_IPC_ECHO_TEST_DEBUG
                    appLogPrintf("IPC: SendTask%d: Creating ...\n", ipc_lld_cpu_id);
                    #endif
                    /* send messages to peer(s) on ENDPT1 */
                    appRtosTaskParamsInit(&params);
                    #if defined THREADX
                    params.priority = TX_MAX_PRIORITIES-4;
                    #else
                    params.priority = 3;
                    #endif
                    params.stack     = g_taskStackBuf[cpu_id];
                    params.stacksize = APP_IPC_ECHO_TEST_TASK_STACKSIZE;
                    params.arg0     = (void*)cpu_id;
                    params.arg1     = (void*)ipc_lld_cpu_id;
                    params.name     = (const char*)&g_rpmsg_sender_task_name[cpu_id][0];
                    params.taskfxn   = &rpmsg_senderFxn;

                    (void)strncpy(g_rpmsg_sender_task_name[cpu_id], "IPC_TEST_TX", APP_IPC_ECHO_TEST_MAX_TASK_NAME);
                    g_rpmsg_sender_task_name[cpu_id][APP_IPC_ECHO_TEST_MAX_TASK_NAME-1u] = (char)0;

                    tx_task[cpu_id] = appRtosTaskCreate(&params);
                    if(tx_task[cpu_id]==NULL)
                    {
                        appLogPrintf("IPC: ERROR: Failed to create TX task %d !!!\n", cpu_id);
                        status = -1;
                    }
                }
            }
        }
    }

    return status;
}
