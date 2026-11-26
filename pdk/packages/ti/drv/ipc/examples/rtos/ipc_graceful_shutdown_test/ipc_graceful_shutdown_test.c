/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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
 *  \file ipc_graceful_shutdown_test.c
 *
 *  \brief IPC graceful shutdown test file:
 *         - mcu1_0, mcu2_0 and mcu2_1 do ping/pong to each other
 *         - mcu2_1 sends a shutdown message to mcu2_0
 *         - mcu2_0 shuts down after receiving the shutdown message
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdio.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/osal/osal.h>
#include <ti/osal/src/nonos/Nonos_config.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/ipc/ipc.h>
#include <ti/drv/ipc/examples/common/src/ipc_setup.h>
#include "ipc_graceful_shutdown_setup.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define IPC_APP_MSGSIZE                            256U
#define IPC_APP_SERVICE_PING                       "ti.ipc4.ping-pong"
#define IPC_APP_ENDPT_PING                         13U
#define IPC_APP_NUMMSGS                            10000 /* number of message sent per task */
#define IPC_APP_IPC_SETUP_TASK_PRI                 (3)
/**< Priority for sender and receiver tasks */
#if defined(BUILD_MCU1_0)
/**< Checker  Task stack size */
#define IPC_APP_CHECKER_TSK_STACK                  (32U * 1024U)
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct IpcApp_TestParams_s
{
    uint32_t endPt;
    char     name[32];
} IpcApp_TestParams;

IpcApp_TestParams IpcApp_ServicePing = { IPC_APP_ENDPT_PING, IPC_APP_SERVICE_PING };

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

extern int32_t Ipc_mailboxSend(uint32_t selfId,
                               uint32_t remoteProcId,
                               uint32_t val,
                               uint32_t timeoutCnt);

static void IpcApp_rpmsgResponderFxn(void *arg0, void *arg1);
static void IpcApp_rpmsgSenderFxn(void *arg0, void *arg1);
static void IpcApp_testPrint(const char *str);
static void IpcApp_rpMboxCallback(uint32_t remoteCoreId, uint32_t msgVal);
#if defined(BUILD_MCU1_0)
static uint32_t IpcApp_getEventId(uint32_t coreId);
static void IpcApp_checkerTask(void *arg0, void *arg1);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Variables defined by test config */
extern uint8_t  *gIpcApp_CntrlBufPtr;
extern uint8_t  *gIpcApp_TaskStackBufPtr;
extern uint8_t  *gIpcApp_SendBufPtr;
extern uint8_t  *gIpcApp_RspBufPtr;
extern uint8_t  *gIpcApp_SysVqBufPtr;
extern uint32_t  gIpcApp_SelfProcId;
extern uint32_t *gIpcApp_RemoteProcArray;
extern uint32_t  gIpcApp_NumRemoteProc;

/* IPC task variables */
uint32_t gIpcApp_RecvTaskBufIdx                = 0;
uint32_t gIpcApp_SendTaskBufIdx[IPC_MAX_PROCS] = {0};

/* Variables related to remote proc shutdown */
bool gIpcApp_ExitRespTsk                       = 0;
volatile uint32_t gIpcApp_Shutdown             = 0U;
volatile uint32_t gIpcApp_ShutdownRemotecoreId = 0U;

#if defined(BUILD_MCU1_0)
EventP_Handle gIpcApp_eventHandle;
static uint8_t gIpcApp_checkerTskStack[IPC_APP_CHECKER_TSK_STACK] __attribute__ ((aligned(IPC_APP_CHECKER_TSK_STACK)));
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t IpcApp_gracefulShutdownTest(void)
{
    uint32_t procIdx;
    uint32_t retVal  = IPC_SOK;
    uint32_t numProc = gIpcApp_NumRemoteProc;
    uint32_t taskIdx   = 0;
    Ipc_VirtIoParams vqParam;
    TaskP_Params     params;
    Ipc_InitPrms     initPrms;

    /* Step1 : Initialize the multiproc */
    Ipc_mpSetConfig(gIpcApp_SelfProcId, numProc, gIpcApp_RemoteProcArray);
    App_printf("IPC_echo_test (core : %s) .....\r\n", Ipc_mpGetSelfName());

    /* Initialize IPC init params */
    IpcInitPrms_init(0U, &initPrms);
    initPrms.printFxn = &IpcApp_testPrint;
    initPrms.rpMboxMsgFxn = &IpcApp_rpMboxCallback;

    /* Do IPC init */
    Ipc_init(&initPrms);

    /* Step2 : Initialize Virtio */
    vqParam.vqObjBaseAddr = (void*)gIpcApp_SysVqBufPtr;
    vqParam.vqBufSize     = numProc * Ipc_getVqObjMemoryRequiredPerCore();
    vqParam.vringBaseAddr = (void*)VRING_BASE_ADDRESS;
    vqParam.vringBufSize  = IPC_VRING_BUFFER_SIZE;
    vqParam.timeoutCnt    = 100;  /* Wait for counts */
    Ipc_initVirtIO(&vqParam);

#if defined (BUILD_MCU1_0)
    EventP_Params eventParams;
    TaskP_Params taskParams;

    EventP_Params_init(&eventParams);
    TaskP_Params_init(&taskParams);

    gIpcApp_eventHandle = EventP_create(&eventParams);
    taskParams.priority   = 4;
    taskParams.stack      = &gIpcApp_checkerTskStack;
    taskParams.stacksize  = IPC_APP_CHECKER_TSK_STACK;
    TaskP_create(&IpcApp_checkerTask, &taskParams);
#endif

    /* Step 3: Initialize RPMessage params */
    RPMessage_Params cntrlParam;
    RPMessageParams_init(&cntrlParam);

    /* Set memory for control task */
    cntrlParam.buf         = gIpcApp_CntrlBufPtr;
    cntrlParam.bufSize     = RPMSG_DATA_SIZE;
    cntrlParam.stackBuffer = &gIpcApp_TaskStackBufPtr[taskIdx++ * IPC_TASK_STACKSIZE];
    cntrlParam.stackSize   = IPC_TASK_STACKSIZE;
    RPMessage_init(&cntrlParam);

    /* Respond to messages coming into endpoint IPC_APP_ENDPT_PING */
    TaskP_Params_init(&params);
    params.priority   = IPC_APP_IPC_SETUP_TASK_PRI;
    params.stack      = &gIpcApp_TaskStackBufPtr[taskIdx++ * IPC_TASK_STACKSIZE];
    params.stacksize  = IPC_TASK_STACKSIZE;
    params.arg0       = (void *)&IpcApp_ServicePing.endPt;
    params.arg1       = (void *)&IpcApp_ServicePing.name[0];
    TaskP_create(&IpcApp_rpmsgResponderFxn, &params);

    for(procIdx = 0; procIdx < numProc; procIdx++, taskIdx++)
    {
        /* Store processor index in global array and pass it to sender task */
        gIpcApp_SendTaskBufIdx[procIdx] = procIdx;
        /* Send messages to peer(s) on IPC_APP_ENDPT_PING */
        TaskP_Params_init(&params);
        params.priority  = IPC_APP_IPC_SETUP_TASK_PRI;
        params.stack     = &gIpcApp_TaskStackBufPtr[taskIdx * IPC_TASK_STACKSIZE];
        params.stacksize = IPC_TASK_STACKSIZE;
        params.arg0      = (void *)&gIpcApp_RemoteProcArray[procIdx];
        params.arg1      = (void *)&gIpcApp_SendTaskBufIdx[procIdx];
        TaskP_create(&IpcApp_rpmsgSenderFxn, &params);
    }

    return retVal;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/*
 * This "Task" waits for a "ping/shutdown" message from processor
 * then replies with a "pong" for ping message and goes to
 * WFI for shutdown message
 */
static void IpcApp_rpmsgResponderFxn(void *arg0, void *arg1)
{
    uintptr_t key;
    uint32_t  endPt        = 0U;
    uint32_t  remoteEndPt  = 0U;
    uint32_t  remoteProcId = 0U;
    uint32_t  cores;
    uint32_t  requestedEpt = (uint32_t)(*(uint32_t*)arg0);
    uint16_t  len;
    int32_t   count;
    int32_t   status       = IPC_SOK;
    void      *recvBuf;
    char      *name        = (char *)arg1;
    char      str[IPC_APP_MSGSIZE];
    RPMessage_Handle handle;
    RPMessage_Params params;
    TaskP_Handle     selfTaskHandle;

    recvBuf = &gIpcApp_RspBufPtr[gIpcApp_RecvTaskBufIdx++ * RPMSG_DATA_SIZE];
    if(recvBuf == NULL)
    {
        status = IPC_EFAIL;
        App_printf("RecvTask: buffer allocation failed\n");
    }
    else
    {
        RPMessageParams_init(&params);
        params.requestedEndpt = requestedEpt;
        params.buf            = recvBuf;
        params.bufSize        = RPMSG_DATA_SIZE;

        handle = RPMessage_create(&params, &endPt);
        if(!handle)
        {
            App_printf("RecvTask: Failed to create endpoint\n");
        }

        status = RPMessage_announce(RPMESSAGE_ALL, endPt, name);
    }

    if(status != IPC_SOK)
    {
        App_printf("%s:RecvTask: RPMessage_announce() for %s failed\n",
                   Ipc_mpGetSelfName(),
                   name);
    }
    else
    {
        while(!gIpcApp_ExitRespTsk)
        {
            status = RPMessage_recv(handle,
                                    (Ptr)str,
                                    &len,
                                    &remoteEndPt,
                                    &remoteProcId,
                                    IPC_RPMESSAGE_TIMEOUT_FOREVER);

            if (gIpcApp_Shutdown == 1U)
            {
                break;
            }

            if(status != IPC_SOK)
            {
                App_printf("RecvTask: failed with code %d\n", status);
            }
            else
            {
                /* NULL terminated string */
                str[len] = '\0';
            }

            status = sscanf(str, "ping %d", &count);
            if(status == 1)
            {
                memset(str, 0, IPC_APP_MSGSIZE);
                key = HwiP_disable();
                len = snprintf(str, 255, "pong %d", count);
                HwiP_restore(key);
                if(len > 255)
                {
                    App_printf("RecvTask: snprintf failed, len %d\n", len);
                    len = 255;
                }
                str[len++] = '\0';
            }
            else
            {
                /* If this is not ping/pong message, just print the message */
                App_printf("%s <--> %s : %s recvd\n",
                           Ipc_mpGetSelfName(),
                           Ipc_mpGetName(remoteProcId),
                           str);
            }
            status = RPMessage_send(handle, remoteProcId, remoteEndPt, endPt, str, len);
            if (status != IPC_SOK)
            {
                App_printf("RecvTask: Sending msg \"%s\" len %d from %s to %s failed!!!\n",
                            str, len, Ipc_mpGetSelfName(),Ipc_mpGetName(remoteProcId));
            }
        }
    }

    /* This delay is required for MCU2_0 to wait before sending the shutdown message to MCU2_1 in the for loop below,
     * otherwise, if it sends early then MCU2_1 just resets its virtio while it is in communication with MCU1_0 */
    TaskP_sleepInMsecs(100);
    /* Below code will execute only in case of shutdown message */
    Sciclient_deinit();

    /* Disable CPU interrupts */
#if defined (BUILD_MCU)
    for (int loopCnt = 0; loopCnt < R5_VIM_INTR_NUM; loopCnt ++)
    {
        OsalArch_disableInterrupt(loopCnt);
        OsalArch_clearInterrupt(loopCnt);
    }
#endif

    HwiP_disable();

    for(cores = 0; cores < gIpcApp_NumRemoteProc; cores++)
    {
        Ipc_mailboxSend(gIpcApp_SelfProcId, gIpcApp_RemoteProcArray[cores], IPC_RP_MBOX_SHUTDOWN, 1u);
    }

#if (__ARM_ARCH_PROFILE == 'R') ||  (__ARM_ARCH_PROFILE == 'M')
    /* For ARM R and M cores*/
    __asm__ __volatile__ ("wfi"   "\n\t": : : "memory");
#endif

    selfTaskHandle = TaskP_self();
    TaskP_delete(&selfTaskHandle);

    App_printf("%s responder task exiting ...\n",Ipc_mpGetSelfName());
}

static void IpcApp_rpmsgSenderFxn(void *arg0, void *arg1)
{
    uintptr_t key;
    uint32_t remoteProcId = 0U;
    uint32_t remoteEndPt  = 0U;
    uint32_t endPt        = 0U;
    uint32_t dstProc      = 0U;
    uint32_t cntPing      = 0U;
    uint32_t cntPong      = 0U;
    uint16_t len;
    int32_t  i;
    int32_t  status       = IPC_SOK;
    char     buf[256];
    uint8_t  *sendBuf;
#if defined (BUILD_MCU2_1)
    uint32_t  remoteShutdown = IPC_MCU2_0;
#endif
    RPMessage_Handle handle;
    RPMessage_Params params;

    sendBuf = &gIpcApp_SendBufPtr[RPMSG_DATA_SIZE * (uint32_t)(*(uint32_t*)arg1)];
    dstProc = (uint32_t)(*(uint32_t*)arg0);

    /* Create the endpoint for receiving. */
    RPMessageParams_init(&params);
    params.numBufs = 2;
    params.buf     = sendBuf;
    params.bufSize = RPMSG_DATA_SIZE;

    handle = RPMessage_create(&params, &endPt);
    if(!handle)
    {
        App_printf("SendTask%d: Failed to create message endpoint\n", dstProc);
    }
    else
    {
        status = RPMessage_getRemoteEndPt(dstProc, IPC_APP_SERVICE_PING, &remoteProcId,
                                        &remoteEndPt, SemaphoreP_WAIT_FOREVER);
        if(dstProc != remoteProcId)
        {
            App_printf("SendTask%d: RPMessage_getRemoteEndPt() malfunctioned, status %d\n",
                        dstProc, status);
            status = IPC_EFAIL;
        }
    }

    if(IPC_SOK == status)
    {
        for (i = 0; i < IPC_APP_NUMMSGS; i++)
        {
            /* Send data to remote endPt: */
            memset(buf, 0, 256);
            key = HwiP_disable();
            len = snprintf(buf, 255, "ping %d", i);
            HwiP_restore(key);
            if(len > 255)
            {
                App_printf("SendTask%d: snprintf failed, len %d\n", dstProc, len);
                len = 255;
            }
            buf[len++] = '\0';

            /* Increase the Ping Counter */
            cntPing++;

            status = RPMessage_send(handle, dstProc, IPC_APP_ENDPT_PING, endPt, (Ptr)buf, len);
            if (status != IPC_SOK)
            {
                App_printf("SendTask%d: RPMessage_send Failed Msg-> \"%s\" from %s to %s...\n",
                            dstProc,buf, Ipc_mpGetSelfName(),Ipc_mpGetName(dstProc));
                break;
            }

            /* wait a for a response message: */
            status = RPMessage_recv(handle, (Ptr)buf, &len, &remoteEndPt,
                                    &remoteProcId, IPC_RPMESSAGE_TIMEOUT_FOREVER);

            if(status != IPC_SOK)
            {
                App_printf("SendTask%d: RPMessage_recv failed with code %d\n", dstProc, status);
                break;
            }

            /* Make it NULL terminated string */
            if(len >= IPC_APP_MSGSIZE)
            {
                buf[IPC_APP_MSGSIZE-1] = '\0';
            }
            else
            {
                buf[len] = '\0';
            }
            cntPong++;
        }
    }

    App_printf("%s <--> %s, Ping- %d, pong - %d completed\n",
               Ipc_mpGetSelfName(),
               Ipc_mpGetName(dstProc),
               cntPing,
               cntPong);

#if defined(BUILD_MCU1_0)
    if (cntPing !=IPC_APP_NUMMSGS || cntPong !=IPC_APP_NUMMSGS )
    {
        App_printf("ERROR !! Ping/Pong iteration between the core mcu1_0 and %s is not %d \nPing is %d , Pong is %d\n"
                    ,Ipc_mpGetName(dstProc),IPC_APP_NUMMSGS,cntPing, cntPong);
        App_printf("Some Test has failed\n");
    }
    else
    {
        EventP_post(gIpcApp_eventHandle,IpcApp_getEventId(dstProc));
    }
#endif

    /* Send shutdown message to MCU2_0 core */
#if defined (BUILD_MCU2_1)
    /* Need to wait before asking MCU2_0 to shutdown, because it might be in communication with the MCU1_0 core,
     * therefore the responder task for MCU2_0 must not exit before ping pong is over */
    TaskP_sleepInMsecs(1000);
    Ipc_mailboxSend(gIpcApp_SelfProcId, remoteShutdown, IPC_RP_MBOX_SHUTDOWN, 1U);
#endif

    /* Delete the RPMesg object now */
    RPMessage_delete(&handle);
}

static void IpcApp_testPrint(const char *str)
{
    App_printf("%s", str);

    return;
}

static void IpcApp_rpMboxCallback(uint32_t remoteCoreId, uint32_t msgVal)
{
    if (IPC_RP_MBOX_SHUTDOWN == msgVal) /* Shutdown request from the remotecore */
    {
#if defined (BUILD_MCU2_0)
        uint32_t i;

        #if defined LDRA_DYN_COVERAGE_EXIT
        UART_printf("\n LDRA ENTRY... \n");
        upload_execution_history();
        UART_printf("\n LDRA EXIT... \n");
        #endif

        if (IPC_MCU2_1 == remoteCoreId)
        {
            gIpcApp_Shutdown             = 1U;
            gIpcApp_ShutdownRemotecoreId = remoteCoreId;
            RPMessage_unblock((RPMessage_Handle)&gIpcApp_RspBufPtr[0]);
            RPMessage_unblock((RPMessage_Handle)&gIpcApp_RspBufPtr[RPMSG_DATA_SIZE]);
            for(i = 0; i < gIpcApp_NumRemoteProc; i++)
            {
                RPMessage_unblock((RPMessage_Handle)&gIpcApp_SendBufPtr[i * RPMSG_DATA_SIZE]);
            }
        }
        else
#endif
        {
            Ipc_resetCoreVirtIO(remoteCoreId);
        }
    }
}

#if defined(BUILD_MCU1_0)

static void IpcApp_checkerTask(void *arg0, void *arg1)
{
    uint32_t retEventMask;
    uint32_t eventMask = 0x0;
    for(int i = 0;i < gIpcApp_NumRemoteProc;i++)
    {
        eventMask += IpcApp_getEventId(gIpcApp_RemoteProcArray[i]);
    }

    retEventMask = EventP_wait(gIpcApp_eventHandle, eventMask, EventP_WaitMode_ALL, EventP_WAIT_FOREVER);
    if((retEventMask & eventMask) == eventMask)
    {
        App_printf("All tests have passed\n");
        EventP_delete(&gIpcApp_eventHandle);
    }
}

static uint32_t IpcApp_getEventId(uint32_t coreId)
{
    uint32_t retVal = EventP_ID_NONE;
    switch(coreId)
    {
        case 0:
            retVal = EventP_ID_00;
            break;
        case 1:
            retVal = EventP_ID_01;
            break;
        case 2:
            retVal = EventP_ID_02;
            break;
        case 3:
            retVal = EventP_ID_03;
            break;
        case 4:
            retVal = EventP_ID_04;
            break;
        case 5:
            retVal = EventP_ID_05;
            break;
        case 6:
            retVal = EventP_ID_06;
            break;
        case 7:
            retVal = EventP_ID_07;
            break;
        case 8:
            retVal = EventP_ID_08;
            break;
        case 9:
            retVal = EventP_ID_09;
            break;
        case 10:
            retVal = EventP_ID_10;
            break;
        case 11:
            retVal = EventP_ID_11;
            break;
    }

    return retVal;
}
#endif
