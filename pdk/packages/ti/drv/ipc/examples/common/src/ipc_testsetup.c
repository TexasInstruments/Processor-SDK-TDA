/*
 *  Copyright (c) Texas Instruments Incorporated 2018 - 2022
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
 *  \file ipc_testsetup.c
 *
 *  \brief IPC  example code
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ti/csl/arch/csl_arch.h>
#include <ti/drv/ipc/examples/common/src/ipc_setup.h>

/* OSAL Header files */
#include <ti/osal/osal.h>
#include <ti/osal/TaskP.h>
#include <ti/osal/EventP.h>
#include <ti/osal/src/nonos/Nonos_config.h>


#include <ti/drv/ipc/ipc.h>
#include <ti/osal/osal.h>
#include <ti/drv/sciclient/sciclient.h>

#ifndef BUILD_MPU1_0
#if defined(SOC_AM65XX)
#include <ti/drv/ipc/examples/common/src/ipc_am65xx_rsctable.h>
#else
#include <ti/drv/ipc/examples/common/src/ipc_rsctable.h>
#endif
#endif

#define MSGSIZE  256U
#define SERVICE_PING   "ti.ipc4.ping-pong"
#define ENDPT_PING     13U
#define SERVICE_CHRDEV "rpmsg_chrdev"
#define ENDPT_CHRDEV   14U
#if defined(SIM_BUILD)
#define NUMMSGS  1000
#else
#define NUMMSGS  10000 /* number of message sent per task */
#endif
//#define NUMMSGS  1000000   /* number of message sent per task */

typedef struct Ipc_TestParams_s
{
    uint32_t endPt;
    char     name[32];
} Ipc_TestParams;

Ipc_TestParams service_ping = { ENDPT_PING, SERVICE_PING };
Ipc_TestParams service_chrdev = { ENDPT_CHRDEV, SERVICE_CHRDEV };

extern uint8_t  *pCntrlBuf;
extern uint8_t  *pTaskBuf;
extern uint8_t  *pSendTaskBuf;
extern uint8_t  *pRecvTaskBuf;
extern uint8_t  *pSysVqBuf;

extern uint32_t  selfProcId;
extern uint32_t *pRemoteProcArray;
extern uint32_t  gNumRemoteProc;

uint32_t gSendTaskBufIdx[IPC_MAX_PROCS] = {0};

uint32_t rpmsgDataSize = RPMSG_DATA_SIZE;

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define IPC_APP_NUM_RESPONDER_TASKS         (2)
#define IPC_SETUP_TASK_PRI                  (3)
/**< Priority for sender and receiver tasks */

#if defined(BUILD_MCU1_0)
/**< Checker  Task stack size */
#define APP_CHECKER_TSK_STACK        (32U * 1024U)
#endif
/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
#if defined(BUILD_MCU1_0)
static uint32_t geteventID(uint32_t coreID);
static void ipc_checker_task(void *arg0, void *arg1);
#endif

extern int32_t Ipc_mailboxSend(uint32_t selfId,
                               uint32_t remoteProcId,
                               uint32_t val,
                               uint32_t timeoutCnt);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
#if defined(BUILD_MCU1_0)
/*Declaration  the Event variables*/
EventP_Handle   gEventhandle;

static uint8_t  gCheckerTskStack[APP_CHECKER_TSK_STACK]
__attribute__ ((aligned(APP_CHECKER_TSK_STACK)));

#endif

#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
static uint32_t		RecvEndPt = 0;
#endif

//#define DEBUG_PRINT

bool g_exitRespTsk = 0;
volatile uint32_t gbShutdown = 0U;
volatile uint32_t gbShutdownRemotecoreID = 0U;
volatile uint32_t gbSuspendSystem = 0U;
volatile uint32_t gbSuspendSystemRemotecoreID = 0U;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void rpmsg_exit_responseTask()
{
    g_exitRespTsk = 1;
}

/*
 * This "Task" waits for a "ping" message from any processor
 * then replies with a "pong" message.
 */
void rpmsg_responderFxn(void *arg0, void *arg1)
{
    RPMessage_Handle handle;
    RPMessage_Params params;
    TaskP_Handle     selfTaskHandle;
    uint32_t		 myEndPt = 0;
    uint32_t		 remoteEndPt;
    uint32_t		 remoteProcId;
    uint16_t		 len;
    int32_t		     n;
    int32_t		     status = 0;
    void		     *buf;
    uint32_t         requestedEpt = (uint32_t)(*(uint32_t*)arg0);
    char *           name = (char *)arg1;

    uintptr_t        key;
    uint32_t         bufSize = rpmsgDataSize;
    uint32_t         cores;
    char             str[MSGSIZE];

    if (requestedEpt == service_ping.endPt)
    {
        buf = &pRecvTaskBuf[0];
    }
    else if (requestedEpt == service_chrdev.endPt)
    {
        buf = &pRecvTaskBuf[rpmsgDataSize];
    }
    else
    {
        buf = NULL;
    }

    if(buf == NULL)
    {
        App_printf("RecvTask: buffer allocation failed\n");
        return;
    }

    RPMessageParams_init(&params);
    params.requestedEndpt = requestedEpt;
    params.buf = buf;
    params.bufSize = bufSize;

    handle = RPMessage_create(&params, &myEndPt);
    if(!handle)
    {
        App_printf("RecvTask: Failed to create endpoint\n");
        return;
    }

#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
    if (requestedEpt == ENDPT_PING)
    {
        RecvEndPt = myEndPt;
    }
#endif

    status = RPMessage_announce(RPMESSAGE_ALL, myEndPt, name);
    if(status != IPC_SOK)
    {
        App_printf("RecvTask: RPMessage_announce() for %s failed\n", name);
        return;
    }

    while(!g_exitRespTsk)
    {
        status = RPMessage_recv(handle, (Ptr)str, &len, &remoteEndPt, &remoteProcId,
                IPC_RPMESSAGE_TIMEOUT_FOREVER);
        if (gbShutdown == 1U)
        {
            break;
        }

        if (gbSuspendSystem)
        {
            /* ACK the suspend message */
            Ipc_mailboxSend(selfProcId, gbSuspendSystemRemotecoreID, IPC_RP_MBOX_SUSPEND_ACK, 1u);
            gbSuspendSystem = 0;
            continue;
        }

        if(status != IPC_SOK)
        {
            App_printf("RecvTask: failed with code %d\n", status);
        }
        else
        {
            /* NULL terminated string */
            str[len] = '\0';
#ifdef DEBUG_PRINT
            App_printf("RecvTask: Revcvd msg \"%s\" len %d from %s\n",
                    str, len, Ipc_mpGetName(remoteProcId));
#endif
        }

        status = sscanf(str, "ping %d", &n);
        if(status == 1)
        {
            memset(str, 0, MSGSIZE);
            /* Not having a Hwip_disable here causes C7X goes into a bad state. This is for SafeRTOS on C7X. */
            key = HwiP_disable();
            len = snprintf(str, 255, "pong %d", n);
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
#ifdef DEBUG_PRINT
        App_printf("RecvTask: Sending msg \"%s\" len %d from %s to %s\n",
                str, len, Ipc_mpGetSelfName(),
                Ipc_mpGetName(remoteProcId));
#endif
        status = RPMessage_send(handle, remoteProcId, remoteEndPt, myEndPt, str, len);
        if (status != IPC_SOK)
        {
            App_printf("RecvTask: Sending msg \"%s\" len %d from %s to %s failed!!!\n",
                str, len, Ipc_mpGetSelfName(),
                Ipc_mpGetName(remoteProcId));
        }
    }

    /* Below code will execute only in case of shutdown message */
    Sciclient_deinit();

    /* Disable CPU interrupts */
    #if defined (BUILD_MCU)
    for (int loopCnt = 0; loopCnt <R5_VIM_INTR_NUM; loopCnt ++)
    {
        OsalArch_disableInterrupt(loopCnt);
        OsalArch_clearInterrupt(loopCnt);
    }
    #endif
    /* Clear all the mailbox interrupts */
    for (cores = 0; cores < gNumRemoteProc; cores++)
    {
        Ipc_mailboxDisableNewMsgInt(selfProcId, pRemoteProcArray[cores]);
    }

    HwiP_disable();

    for(cores = 0; cores < gNumRemoteProc; cores++)
    {
        if(pRemoteProcArray[cores] == IPC_MPU1_0)
        {
            continue;
        }

        Ipc_mailboxSend(selfProcId, pRemoteProcArray[cores], IPC_RP_MBOX_SHUTDOWN, 1u);    
    }

    /* ACK the suspend message */
    Ipc_mailboxSend(selfProcId, gbShutdownRemotecoreID, IPC_RP_MBOX_SHUTDOWN_ACK, 1u);

#if (__ARM_ARCH_PROFILE == 'R') ||  (__ARM_ARCH_PROFILE == 'M')
    /* For ARM R and M cores*/
    __asm__ __volatile__ ("wfi"   "\n\t": : : "memory");
#endif
#if defined(BUILD_C7X)
    asm (" idle");
#endif

    selfTaskHandle = TaskP_self();
    TaskP_delete(&selfTaskHandle);

    App_printf("%s responder task exiting ...\n",
                    Ipc_mpGetSelfName());
}

void rpmsg_senderFxn(void *arg0, void *arg1)
{
    RPMessage_Handle    handle;
    RPMessage_Params    params;
    uint32_t            myEndPt = 0;
    uint32_t            remoteEndPt;
    uint32_t            remoteProcId;
    uint32_t            dstProc;
    uint16_t            len;
    int32_t             i;
    int32_t             status = 0;
    char                buf[256];
    uint8_t            *buf1;
    uintptr_t key;

    uint32_t            cntPing = 0;
    uint32_t            cntPong = 0;

    buf1 = &pSendTaskBuf[rpmsgDataSize * (uint32_t)(*(uint32_t*)arg1)];
    dstProc = (uint32_t)(*(uint32_t*)arg0);

    /* Create the endpoint for receiving. */
    RPMessageParams_init(&params);
    params.numBufs = 2;
    params.buf = buf1;
    params.bufSize = rpmsgDataSize;
    handle = RPMessage_create(&params, &myEndPt);
    if(!handle)
    {
        App_printf("SendTask%d: Failed to create message endpoint\n",
                dstProc);
        return;
    }

    status = RPMessage_getRemoteEndPt(dstProc, SERVICE_PING, &remoteProcId,
            &remoteEndPt, SemaphoreP_WAIT_FOREVER);
    if(dstProc != remoteProcId)
    {
        App_printf("SendTask%d: RPMessage_getRemoteEndPt() malfunctioned, status %d\n",
                dstProc, status);
        return;
    }

    for (i = 0; i < NUMMSGS; i++)
    {
        /* Send data to remote endPt: */
        memset(buf, 0, 256);
        /* Not having a Hwip_disable here causes C7X go into a bad state. This is for SafeRTOS on C7X */
        key = HwiP_disable();
        len = snprintf(buf, 255, "ping %d", i);
        HwiP_restore(key);
        if(len > 255)
        {
            App_printf("SendTask%d: snprintf failed, len %d\n", dstProc, len);
            len = 255;
        }
        buf[len++] = '\0';

#ifdef DEBUG_PRINT
        App_printf("SendTask%d: Sending \"%s\" from %s to %s...\n", dstProc,
                buf, Ipc_mpGetSelfName(),
                Ipc_mpGetName(dstProc));
#endif
        /* Increase the Ping Counter */
        cntPing++;

        status = RPMessage_send(handle, dstProc, ENDPT_PING, myEndPt, (Ptr)buf, len);
        if (status != IPC_SOK)
        {
            App_printf("SendTask%d: RPMessage_send Failed Msg-> \"%s\" from %s to %s...\n",
                dstProc,
                buf, Ipc_mpGetSelfName(),
                Ipc_mpGetName(dstProc));
            break;
        }


        /* wait a for a response message: */
        status = RPMessage_recv(handle, (Ptr)buf, &len, &remoteEndPt,
                &remoteProcId, IPC_RPMESSAGE_TIMEOUT_FOREVER);

        if(status != IPC_SOK)
        {
            App_printf("SendTask%d: RPMessage_recv failed with code %d\n",
                    dstProc, status);
            break;
        }

        /* Make it NULL terminated string */
        if(len >= MSGSIZE)
        {
            buf[MSGSIZE-1] = '\0';
        }
        else
        {
            buf[len] = '\0';
        }

#ifdef DEBUG_PRINT
        App_printf("SendTask%d: Received \"%s\" len %d from %s endPt %d \n",
                dstProc, buf, len, Ipc_mpGetName(remoteProcId),
                remoteEndPt);
#endif
        cntPong++;
        if((i+1)%50 == 0)
        {
            //App_printf("%s <--> %s, ping/pong iteration %d ...\n",
            //        Ipc_mpGetSelfName(), Ipc_mpGetName(dstProc), i);
        }
    }

    App_printf("%s <--> %s, Ping- %d, pong - %d completed\n",
            Ipc_mpGetSelfName(),
            Ipc_mpGetName(dstProc),
            cntPing, cntPong);
#if defined(BUILD_MCU1_0)
    if (cntPing !=NUMMSGS || cntPong !=NUMMSGS )
    {
        App_printf("ERROR !! Ping/Pong iteration between the core mcu1_0 and %s is not %d \nPing is %d , Pong is %d\n",Ipc_mpGetName(dstProc),NUMMSGS,cntPing, cntPong);
        App_printf("Some Test has failed\n");
    }
    else
    {
        EventP_post(gEventhandle,geteventID(dstProc));
    }
#endif

    /* Delete the RPMesg object now */
    RPMessage_delete(&handle);
}

/*
 * This "Task" waits for Linux vdev ready, and late create the vrings
 *
 */
#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
void rpmsg_vdevMonitorFxn(void* arg0, void* arg1)
{
    int32_t status;

    /* Wait for Linux VDev ready... */
    while(!Ipc_isRemoteReady(IPC_MPU1_0))
    {
        TaskP_sleep(10);
    }

    /* Create the VRing now ... */
    status = Ipc_lateVirtioCreate(IPC_MPU1_0);
    if(status != IPC_SOK)
    {
        App_printf("%s: Ipc_lateVirtioCreate failed\n", __func__);
        return;
    }

    status = RPMessage_lateInit(IPC_MPU1_0);
    if(status != IPC_SOK)
    {
        App_printf("%s: RPMessage_lateInit failed\n", __func__);
        return;
    }

    status = RPMessage_announce(IPC_MPU1_0, RecvEndPt, SERVICE_PING);
    if(status != IPC_SOK)
    {
        App_printf("rpmsg_vdevMonitorFxn: RPMessage_announce() failed\n");
    }
}
#endif /* !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)*/

static void IpcTestPrint(const char *str)
{
    App_printf("%s", str);

    return;
}

static void IpcRpMboxCallback(uint32_t remoteCoreId, uint32_t msgVal)
{
    uint32_t i;

    if (msgVal == IPC_RP_MBOX_SHUTDOWN) /* Shutdown request from the remotecore */
    {
        if (IPC_MPU1_0 == remoteCoreId)
        {
            gbShutdown = 1U;
            gbShutdownRemotecoreID = remoteCoreId;
            RPMessage_unblock((RPMessage_Handle)&pRecvTaskBuf[0]);
            RPMessage_unblock((RPMessage_Handle)&pRecvTaskBuf[rpmsgDataSize]);
            for(i = 0; i < gNumRemoteProc; i++)
            {
                if(pRemoteProcArray[i] == IPC_MPU1_0)
                    continue;

                RPMessage_unblock((RPMessage_Handle)&pSendTaskBuf[i * rpmsgDataSize]);
            }
        }
        else
        {
            Ipc_resetCoreVirtIO(remoteCoreId);
        }
    }
    if (msgVal == IPC_RP_MBOX_SUSPEND_SYSTEM) /* Suspend request from the remotecore */
    {
        if (IPC_MPU1_0 == remoteCoreId)
        {
            gbSuspendSystem = 1U;
            gbSuspendSystemRemotecoreID = remoteCoreId;
            RPMessage_unblock((RPMessage_Handle)&pRecvTaskBuf[rpmsgDataSize]);
            for(i = 0; i < gNumRemoteProc; i++)
            {
                if(pRemoteProcArray[i] == IPC_MPU1_0)
                    continue;

                RPMessage_unblock((RPMessage_Handle)&pSendTaskBuf[i * rpmsgDataSize]);
            }
        }
        else
        {
            Ipc_resetCoreVirtIO(remoteCoreId);
        }
    }
}

int32_t Ipc_echo_test(void)
{
    uint32_t          t;
    TaskP_Params      params;
    uint32_t          numProc = gNumRemoteProc;
    Ipc_VirtIoParams  vqParam;
    Ipc_InitPrms      initPrms;
    uint32_t          index = 0;

    /* Step1 : Initialize the multiproc */
    Ipc_mpSetConfig(selfProcId, numProc, pRemoteProcArray);

    App_printf("IPC_echo_test (core : %s) .....\r\n", Ipc_mpGetSelfName());

    /* Initialize params with defaults */
    IpcInitPrms_init(0U, &initPrms);

    initPrms.printFxn = &IpcTestPrint;
    initPrms.rpMboxMsgFxn = &IpcRpMboxCallback;

    Ipc_init(&initPrms);

    //App_printf("Required Local memory for Virtio_Object = %d\r\n",
    //   numProc * Ipc_getVqObjMemoryRequiredPerCore());

#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS)
    /* If A72 remote core is running Linux OS, then
     * load resource table
     */
    Ipc_loadResourceTable((void*)&ti_ipc_remoteproc_ResourceTable);

#if !defined(A72_LINUX_OS_IPC_ATTACH)
    /* Wait for Linux VDev ready... */
    for(t = 0; t < numProc; t++)
    {
        while(!Ipc_isRemoteReady(pRemoteProcArray[t]))
        {
            TaskP_sleep(10);
        }
    }
    //App_printf("Linux VDEV ready now .....\n");
#endif
#endif

    /* Step2 : Initialize Virtio */
    vqParam.vqObjBaseAddr = (void*)pSysVqBuf;
    vqParam.vqBufSize     = numProc * Ipc_getVqObjMemoryRequiredPerCore();
    vqParam.vringBaseAddr = (void*)VRING_BASE_ADDRESS;
    vqParam.vringBufSize  = IPC_VRING_BUFFER_SIZE;
    vqParam.timeoutCnt    = 100;  /* Wait for counts */
    Ipc_initVirtIO(&vqParam);

#if defined (BUILD_MCU1_0)
    EventP_Params      eventParams;
    TaskP_Params      taskParams;

    EventP_Params_init(&eventParams);

    gEventhandle = EventP_create(&eventParams);

    TaskP_Params_init(&taskParams);
    taskParams.priority   = 4;
    taskParams.stack      = &gCheckerTskStack;
    taskParams.stacksize  = APP_CHECKER_TSK_STACK;
    if (NULL == TaskP_create(&ipc_checker_task, &taskParams))
    {
        App_printf("IPC Checker task creation failed!!!");
    }
#endif
    /* Step 3: Initialize RPMessage */
    RPMessage_Params cntrlParam;

    //App_printf("Required Local memory for RPMessage Object = %d\n",
    //   RPMessage_getObjMemRequired());

    /* Initialize the param */
    RPMessageParams_init(&cntrlParam);

    /* Set memory for HeapMemory for control task */
    cntrlParam.buf         = pCntrlBuf;
    cntrlParam.bufSize     = rpmsgDataSize;
    cntrlParam.stackBuffer = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
    cntrlParam.stackSize   = IPC_TASK_STACKSIZE;
    cntrlParam.ctrlTaskPriority   = IPC_CTRL_TASK_PRIORITY;
    RPMessage_init(&cntrlParam);

    /* Respond to messages coming in to endPt ENDPT_PING */
    TaskP_Params_init(&params);
    params.priority   = 3;
    params.stack      = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
    params.stacksize  = IPC_TASK_STACKSIZE;
    params.arg0       = (void *)&service_ping.endPt;
    params.arg1       = (void *)&service_ping.name[0];
    if (NULL == TaskP_create(&rpmsg_responderFxn, &params))
    {
        App_printf("IPC Responder task creation failed!!!");
    }

#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS)
    /* Respond to messages coming in to endPt ENDPT_CHRDEV (for testing rpmsg_chrdev) */
    TaskP_Params_init(&params);
    params.priority   = IPC_SETUP_TASK_PRI;
    params.stack      = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
    params.stacksize  = IPC_TASK_STACKSIZE;
    params.arg0       = (void *)&service_chrdev.endPt;
    params.arg1       = (void *)&service_chrdev.name[0];
    if (NULL == TaskP_create(&rpmsg_responderFxn, &params))
    {
        App_printf("IPC Responder task creation failed!!!");
    }
#endif

    for(t = 0; t < numProc; t++, index++)
    {
#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS)
        /* Linux does not have a responder func running */
        if(pRemoteProcArray[t] == IPC_MPU1_0)
            continue;
#endif
        /* Store index in global array.
         * the pointer of which is to be passed as argument to sender task */
        gSendTaskBufIdx[t] = t;
        /* send messages to peer(s) on ENDPT_PING */
        TaskP_Params_init(&params);
        params.priority  = IPC_SETUP_TASK_PRI;
        params.stack     = &pTaskBuf[index * IPC_TASK_STACKSIZE];
        params.stacksize = IPC_TASK_STACKSIZE;
        params.arg0      = (void *)&pRemoteProcArray[t];
        params.arg1      = (void *)&gSendTaskBufIdx[t];
        if (NULL == TaskP_create(&rpmsg_senderFxn, &params))
        {
            App_printf("IPC Sender task creation failed!!!");
        }
    }

#if !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
    /* Respond to messages coming in to endPt ENDPT_PING */
    TaskP_Params_init(&params);
    params.priority = IPC_SETUP_TASK_PRI;
    params.stacksize = 0x1000;
    params.arg0 = 0;
    if (NULL == TaskP_create(&rpmsg_vdevMonitorFxn, &params))
    {
        App_printf("IPC VDEV Monitor task creation failed!!!");
    }
#endif /* !defined(BUILD_MPU1_0) && defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH) */

    return 1;
}

#if defined(BUILD_MCU1_0)

static void ipc_checker_task(void *arg0, void *arg1)
{
    uint32_t        retEventMask;
    uint32_t        eventMask = 0x0;
    for(int i=0;i<gNumRemoteProc;i++)
    {
        eventMask+=geteventID(pRemoteProcArray[i]);
    }

    retEventMask = EventP_wait(gEventhandle, eventMask, EventP_WaitMode_ALL, EventP_WAIT_FOREVER);
    
    #if defined LDRA_DYN_COVERAGE_EXIT
        UART_printf("\n LDRA ENTRY... \n");
        upload_execution_history();
        UART_printf("\n LDRA EXIT... \n");
    #endif
    
    if((retEventMask & eventMask) == eventMask)
    {
        App_printf("All tests have passed\n");
        EventP_delete(&gEventhandle);
    }
}

static uint32_t geteventID(uint32_t coreID)
{
    uint32_t retVal=EventP_ID_NONE;
    switch(coreID)
    {
        case 0:
            retVal=EventP_ID_00;
            break;
        case 1:
            retVal=EventP_ID_01;
            break;
        case 2:
            retVal=EventP_ID_02;
            break;
        case 3:
            retVal=EventP_ID_03;
            break;
        case 4:
            retVal=EventP_ID_04;
            break;
        case 5:
            retVal=EventP_ID_05;
            break;
        case 6:
            retVal=EventP_ID_06;
            break;
        case 7:
            retVal=EventP_ID_07;
            break;
        case 8:
            retVal=EventP_ID_08;
            break;
        case 9:
            retVal=EventP_ID_09;
            break;
        case 10:
            retVal=EventP_ID_10;
            break;
        case 11:
            retVal=EventP_ID_11;
            break;

    }
    return retVal;
}
#endif
