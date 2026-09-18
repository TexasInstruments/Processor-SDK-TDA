/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \file ipc_invalid_endpt_test.c
 *
 *  \brief IPC example code to test invalid end point, sends msg from mcu1_0
 *  to mcu2_0 using invalid endpt which is ignored, then echo test is performed
 *  to validate that IPC is up after that.
 * 
 *  To debug/see the driver logs of invalid endpt received on other side,
 *  enable the DEBUG_PRINT macro and check for the endpt number.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <ti/osal/osal.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/drv/uart/UART_stdio.h>
#include "ipc_invalid_endpt_test.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define MAX_MSG_SIZE              64
#define TEST_ENDPOINT             22
#define TEST_SERVICE_NAME         "test-invalid-endpt"
#define TEST_NUM_SLEEP_TICKS      2500

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint8_t  gSysVqBuf[TEST_VQ_BUF_SIZE]  __attribute__ ((section ("ipc_data_buffer"), aligned (8)));
uint8_t  gCtrlTaskBuf[TEST_RPMSG_DATA_SIZE] __attribute__ ((section("ipc_data_buffer"), aligned (8)));
uint8_t  gTestRpmsgBuf[TEST_RPMSG_DATA_SIZE]  __attribute__ ((section ("ipc_data_buffer"), aligned (8)));
uint8_t  gCtrlTaskStack[TEST_CTRL_TASK_STACKSIZE];
uint32_t gMainCoreId = IPC_MCU1_0;
uint32_t gMsgEchoCount = 10;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void ipc_test_print(const char *str)
{
    UART_printf("%s", str);
}

void ipc_echo_main_core_start()
{
    uint32_t msg, i;
    char msgBuf[MAX_MSG_SIZE];
    int32_t status;
    uint16_t recdMsgSize;
    uint32_t remoteCoreId, msgSize;
    uint32_t remoteCoreEndPt;
    uint32_t allocatedEndPt;
    uint32_t numRemoteCores = TEST_NUM_REMOTE_CORES;
    RPMessage_Params    params;
    RPMessage_Handle    handle;
    uint32_t invalidDestEndPt = TEST_INVALID_DEST_ENDPT;

    RPMessageParams_init(&params);
    params.requestedEndpt = TEST_ENDPOINT;
    params.numBufs = TEST_NUM_BUF_RPMSG_CREATE;
    params.buf = gTestRpmsgBuf;
    handle = RPMessage_create(&params, &allocatedEndPt);
    if (handle == NULL)
    {
        UART_printf("RPMessage handle creation failed !!\n");
    }

    status = RPMessage_announce(RPMESSAGE_ALL, allocatedEndPt, TEST_SERVICE_NAME);
    
    if (status != IPC_SOK)
    {
        UART_printf("RPMessage announce failed !! \n");
    }

    for (uint32_t core = 0; core < numRemoteCores; core++)
    {
        RPMessage_getRemoteEndPt(gRemoteCores[core], TEST_SERVICE_NAME, &remoteCoreId, &remoteCoreEndPt, IPC_RPMESSAGE_TIMEOUT_FOREVER);
        if ((remoteCoreId != gRemoteCores[core]) 
            || (remoteCoreEndPt != TEST_ENDPOINT))
        {
            UART_printf("Invalid EndPt / Remote Core Id received, stopping the test !!\n");
            UART_printf("Failure details : remoteCoreId :  %d , remoteCoreEndpt %d \n", remoteCoreId, remoteCoreEndPt);
            return;
        }
    }

    UART_printf("[IPC RPMSG ECHO] Message exchange started by main core !!!\r\n");
    UART_printf("[IPC RPMSG ECHO] Send Message with invalid endpoint first !!!\r\n");

    snprintf(msgBuf, MAX_MSG_SIZE-1, "%s", "test message");
    msgBuf[MAX_MSG_SIZE-1] = 0;
    msgSize = strlen(msgBuf) + 1;
    status = RPMessage_send(
        handle,
        gRemoteCores[0],
        invalidDestEndPt,
        allocatedEndPt,
        (void *)msgBuf, msgSize);

    /*Sleep for 2 * (timeout ticks for remote core) so that remote core's RPMessage_recv call times out during this wait*/
    TaskP_sleep(2*TEST_NUM_SLEEP_TICKS);
    for(msg=0; msg<gMsgEchoCount; msg++)
    {
        snprintf(msgBuf, MAX_MSG_SIZE-1, "%d", msg);
        msgBuf[MAX_MSG_SIZE-1] = 0;
        msgSize = strlen(msgBuf) + 1; /* count the terminating char as well */

        /* send the same messages to all cores */
        for(i=0; i < numRemoteCores; i++ )
        {
            status = RPMessage_send(
                handle,
                gRemoteCores[i],
                remoteCoreEndPt,
                allocatedEndPt,
                (void *)msgBuf, msgSize);
            if (status != IPC_SOK)
            {
                UART_printf("[IPC RPMSG ECHO] : RPMessage_send failure with status %d !! \n", status);
            }
        }

        /* wait for response from all cores */
        for(i=0; i < numRemoteCores; i++ )
        {
            /* set 'msgSize' to size of recv buffer,
            * after return `msgSize` contains actual size of valid data in recv buffer
            */
            recdMsgSize = sizeof(msgBuf);
            status = RPMessage_recv(handle,
                msgBuf, &recdMsgSize,
                &remoteCoreEndPt,
                &remoteCoreId,
                IPC_RPMESSAGE_TIMEOUT_FOREVER);
            if (status != IPC_SOK)
            {
                UART_printf("[IPC RPMSG ECHO] : RPMessage_recv failure with status %d !! \n", status);
            }
        }
    }

    UART_printf("[IPC RPMSG ECHO] All echoed messages received by main core from %d remote cores !!!\r\n", numRemoteCores);
    UART_printf("[IPC RPMSG ECHO] Messages sent to each core = %d \r\n", gMsgEchoCount);

    RPMessage_delete(&handle);

    UART_printf("All tests have passed\n");
}

void ipc_echo_remote_core_start()
{
    char msgBuf[MAX_MSG_SIZE];
    int32_t status;
    uint16_t recdMsgSize = 0, msgCnt = 0;
    uint32_t remoteCoreId;
    uint32_t remoteCoreEndPt = 0U;
    uint32_t allocatedEndPt;
    RPMessage_Params    params;
    RPMessage_Handle    handle;
    uint32_t invalidEndPtTimeout = TEST_NUM_SLEEP_TICKS;

    RPMessageParams_init(&params);
    params.requestedEndpt = TEST_ENDPOINT;
    params.numBufs = TEST_NUM_BUF_RPMSG_CREATE;
    params.buf = gTestRpmsgBuf;
    params.bufSize = sizeof(gTestRpmsgBuf);
    handle = RPMessage_create(&params, &allocatedEndPt);
    if (handle == NULL)
    {
        UART_printf("RPMessage handle creation failed !!\n");
    }

    status = RPMessage_announce(RPMESSAGE_ALL, allocatedEndPt, TEST_SERVICE_NAME);
    UART_printf("[IPC RPMSG ECHO] Allocated end pt : %d !!!\r\n", allocatedEndPt);

    if (status != IPC_SOK)
    {
        UART_printf("RPMessage announce failed !! \n");
    }

    UART_printf("[IPC RPMSG ECHO] Remote core waiting for messages !!!\r\n");

    /* If we pend on a sem for given timeout then it returns gracefully so status will be correct 
    *  i.e no timeout status expected here for invalid endpt case */
    status = RPMessage_recv(handle,
            msgBuf, &recdMsgSize,
            &remoteCoreEndPt,
            &remoteCoreId,
            invalidEndPtTimeout);
    
    if ((recdMsgSize == 0U) || (remoteCoreEndPt == 0U))
    {
        UART_printf("[IPC RPMSG ECHO] Remote core ignored invalid endpt msg as expected !!!\r\n");
    }
    else
    {
        /* Recv would have received a message from echo loop and didn't return empty handed because of sem timeout */
        UART_printf("[IPC RPMSG ECHO] Timing Issue, recv API received echo test message, will cause hang !!\n");
    }

    while(1)
    {
        recdMsgSize = sizeof(msgBuf);
        status = RPMessage_recv(handle,
                    msgBuf, &recdMsgSize,
                    &remoteCoreEndPt,
                    &remoteCoreId,
                    IPC_RPMESSAGE_TIMEOUT_FOREVER);
        if (status != IPC_SOK)
        {
            UART_printf("[IPC RPMSG ECHO] : RPMessage_recv failure with status %d !! \n", status);
            break;
        }

        msgCnt++;

        status = RPMessage_send(
            handle,
            remoteCoreId,
            remoteCoreEndPt,
            allocatedEndPt,
            (void *)msgBuf, recdMsgSize);

        if (status != IPC_SOK)
        {
            UART_printf("[IPC RPMSG ECHO] RPMessage_send failed with status %d !!\n", status);
            break;
        }

        if (msgCnt >= gMsgEchoCount)
        {
            break;
        }
    }

    RPMessage_delete(&handle);
    UART_printf("[IPC RPMSG ECHO] Received and echoed %d messages ... !!!\r\n", gMsgEchoCount);
    UART_printf("All tests have passed\n");
}

int32_t ipc_invalid_endpt_test(void)
{
    Ipc_VirtIoParams  vqParam;
    Ipc_InitPrms      initPrms;
    uint32_t          numRemoteCores = TEST_NUM_REMOTE_CORES;

    Ipc_mpSetConfig(gSelfCoreId, numRemoteCores, gRemoteCores);

    UART_printf("IPC Invalid Endpoint Test (core : %s) .....\r\n", Ipc_mpGetSelfName());

    IpcInitPrms_init(0U, &initPrms);
    initPrms.printFxn = &ipc_test_print;
    Ipc_init(&initPrms);

    vqParam.vqObjBaseAddr = (void*)gSysVqBuf;
    vqParam.vqBufSize     = numRemoteCores * Ipc_getVqObjMemoryRequiredPerCore();
    vqParam.vringBaseAddr = (void *)TEST_VRING_BASE_ADDRESS;
    vqParam.vringBufSize  = IPC_VRING_BUFFER_SIZE;
    vqParam.timeoutCnt    = 100;
    Ipc_initVirtIO(&vqParam);

    RPMessage_Params cntrlParam;
    RPMessageParams_init(&cntrlParam);

    /* Set memory for HeapMemory for control task */
    cntrlParam.buf         = gCtrlTaskBuf;
    cntrlParam.bufSize     = TEST_RPMSG_DATA_SIZE;
    cntrlParam.stackBuffer = (void *)gCtrlTaskStack;
    cntrlParam.stackSize   = IPC_TASK_STACKSIZE;
    cntrlParam.ctrlTaskPriority   = IPC_CTRL_TASK_PRIORITY;
    RPMessage_init(&cntrlParam);


    if (gMainCoreId == gSelfCoreId)
    {
        ipc_echo_main_core_start();
    }
    else
    {
        ipc_echo_remote_core_start();
    }
    return 1;
}