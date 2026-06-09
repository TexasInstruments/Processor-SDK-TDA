/*
 *  Copyright (c) Texas Instruments Incorporated 2025
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
 *  \file main_rtos.c
 *
 *  \brief Main file for RTOS builds
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <stdio.h>
#if defined(LDRA_COVERAGE)
#include <examples/include/ldra_vhwa_coverage_main.h>
#endif

#include <examples/include/vhwa_examples_common.h>
#include <include/vhwa_m2mFlexConnect.h>
#include <examples/include/vhwa_viss_test_api.h>
#include <examples/include/vhwa_msc_api.h>
#include <examples/vhwa_viss_test/vhwa_viss_cfg.h>
#include <examples/vhwa_flexconnect_test/vhwa_flexconnect_viss_cfg.h>
#include <examples/vhwa_flexconnect_test/vhwa_flexconnect_msc_cfg.h>
#include <examples/include/vhwa_flexconnect_api.h>
#include <examples/vhwa_flexconnect_test/vhwa_flexconnect_cfg.h>

#if defined(VHWA_PDK)
#include "ti/osal/osal.h"
#include "ti/osal/TaskP.h"

#include <ti/board/board.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#elif defined(VHWA_MCU_PLUS_SDK)
#include <drivers/device_manager/sciclient.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include <kernel/dpl/SemaphoreP.h>
#include "FreeRTOS.h"
#include "task.h"
#include <drivers/uart.h>
#include <kernel/dpl/TimerP.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Set test application stack size and task priority 
 * higher than the default priority (1) */
#if defined(VHWA_MCU_PLUS_SDK)
#define MAIN_TASK_SIZE (16384U / sizeof(configSTACK_DEPTH_TYPE))
#define APP_FC_MAIN_TASKS_PRIORITY     (configMAX_PRIORITIES - 1)
#define APP_FC_TEST_TASKS_PRIORITY     (configMAX_PRIORITIES - 2)
#elif defined(VHWA_PDK)
#define APP_FC_TSK_STACK_MAIN          (64U * 1024U)
#define APP_FC_MAIN_TASKS_PRIORITY     (7U)
#define APP_FC_TEST_TASKS_PRIORITY     (5U)
#endif

/* Input Offset between two test cases, used mainly for zebu/qt,
 * so that buffers can be loaded in one shot */
#define VHWA_FC_TEST_IN_BUF_OFFSET     (8*1024*1024)

/* Output Offset between two test cases, used mainly for zebu/qt,
 * so that buffers can be saved in one shot.
 * Set to 80MB to accommodate 10 MSC outputs @ 8MB each for multi-handle scenarios */
#define VHWA_FC_TEST_OUT_BUF_OFFSET    (80*1024*1024)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

static VhwaAl_SemaphoreP_Struct waitForTaskCmpl[VHWA_FC_DRV_MAX_HANDLES];

#if defined(VHWA_MCU_PLUS_SDK)
/* Define task parameters storage once */
typedef struct
{
    AppFc_TestParams *testObj;
    uint32_t handleIdx;
} taskParams;
#endif

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void FcApp_mainTaskFunc(void* a0, void* a1);
static void FcApp_test(void* a0, void* a1);
static int32_t FcApp_init(void);
static void FcApp_run(uint32_t testCnt);
static void FcApp_deInit(void);
#if defined(VHWA_PDK)
static int32_t App_getNum(void);
#endif
#if defined(VHWA_MCU_PLUS_SDK)
static void FcApp_mainTaskFuncWrapper(void *args);
static void FcApp_testTaskWrapper(void *args);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Test application stack */
#if defined(VHWA_MCU_PLUS_SDK)
static StackType_t gMainTaskStack[MAIN_TASK_SIZE]
    __attribute__((aligned(32)));
static StaticTask_t gMainTaskObj;
static StackType_t taskStacks[VHWA_FC_DRV_MAX_HANDLES][MAIN_TASK_SIZE]
    __attribute__((aligned(32)));
static StaticTask_t taskBuffers[VHWA_FC_DRV_MAX_HANDLES];

#elif defined(SAFERTOS)
static uint8_t  gAppTskStackMain[APP_FC_TSK_STACK_MAIN]
    __attribute__((aligned(APP_FC_TSK_STACK_MAIN))) = { 0 };
static uint8_t  gAppTskStackTest[VHWA_FC_DRV_MAX_HANDLES][APP_FC_TSK_STACK_MAIN]
    __attribute__((aligned(APP_FC_TSK_STACK_MAIN))) = { 0 };
#else
static uint8_t  gAppTskStackMain[APP_FC_TSK_STACK_MAIN]
    __attribute__((aligned(32)));
static uint8_t  gAppTskStackTest[VHWA_FC_DRV_MAX_HANDLES][APP_FC_TSK_STACK_MAIN]
    __attribute__((aligned(32)));
#endif

/*
* Application Buffers
*/
#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
static uint64_t gFcTestSrcBuf =
    (uint64_t )(0xA0000000);
static uint64_t gFcTestDestBuf =
    (uint64_t )(0xC0000000u);
#elif defined VHWA_VPAC_IP_REV_VPAC3L
static uint64_t gFcTestSrcBuf =
    (uint64_t )(0xB0000000u);
static uint64_t gFcTestDestBuf =
    (uint64_t )(0xB2000000u);
#endif
static uint32_t gFcTestSrcBufFreeIdx = 0u;
static uint32_t gFcTestDstBufFreeIdx = 0u;

#if defined(VHWA_MCU_PLUS_SDK)
static VhwaAl_SemaphoreP_Struct gAllocLock;
#else
static VhwaAl_SemaphoreP_Struct gAllocLock = {NULL};
#endif

static VhwaAl_Udma_DrvObj_Struct gFcAppUdmaDrvObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    #if defined(VHWA_MCU_PLUS_SDK)
    /* Initialize MCU+ SDK system components */
    System_init();
    Board_init();

    #if defined(SOC_AM62AX)
    Sciclient_pmSetModuleState(TISCI_DEV_VPAC0,
                               TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                               TISCI_MSG_FLAG_AOP,
                               0xFFFFFFFFU);
    #endif
    Drivers_open();
    Board_driversOpen();

    #if defined(SOC_AM62AX)
    volatile int loop = 1;
    while (loop);
    #endif  

    TaskHandle_t gMainTask;

    /* Create main task using FreeRTOS */
    gMainTask = xTaskCreateStatic(
        FcApp_mainTaskFuncWrapper,    /* Function that implements the task */
        "fc_main_task",               /* Text name for debugging */
        MAIN_TASK_SIZE,               /* Stack size in words */
        NULL,                         /* No parameters needed */
        APP_FC_MAIN_TASKS_PRIORITY,   /* High priority */
        gMainTaskStack,               /* Stack buffer */
        &gMainTaskObj                 /* Static task buffer */
    );
    configASSERT(gMainTask != NULL);

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    DebugP_assertNoLog(0);
#else
    /* Original code for PDK platforms */
    TaskP_Handle task;
    TaskP_Params taskParams;

    OS_init();

    /* Initialize the task params */
    TaskP_Params_init(&taskParams);

    taskParams.priority     = APP_FC_MAIN_TASKS_PRIORITY;
    taskParams.stack        = gAppTskStackMain;
    taskParams.stacksize    = sizeof (gAppTskStackMain);

    task = TaskP_create(&FcApp_mainTaskFunc, &taskParams);
    if(NULL == task)
    {
        OS_stop();
    }

    OS_start();    /* does not return */
    #endif
    return(0);
}

static void FcApp_mainTaskFunc(void* a0, void* a1)
{
    int32_t             status;
    bool                done = UFALSE;
    int32_t             testIdx;
    #if defined (LDRA_COVERAGE)
    /*Expand gApprovedTestCases[] to include future testcases*/
    static const int32_t gApprovedPositiveTestCases [] = {0,1,14,16,17,18,22};
    static const int32_t gApprovedNegativeTestCases [] = {19,20,21,-1};
    uint32_t i = 0,j =0, PositiveTestcasesMax = 0; 
    PositiveTestcasesMax = sizeof(gApprovedPositiveTestCases)/sizeof(gApprovedPositiveTestCases[0]);
    #endif


    status = FcApp_init();
    #if defined(LDRA_COVERAGE)
    Vhwa_print("################### DYNAMIC CODE COVERAGE INITIALIZATION STARTED ######################\n");
    ldra_initialize();
    Vhwa_print("################### DYNAMIC CODE COVERAGE INITIALIZATION FINISHED ######################\n");
    #endif

    Vhwa_print("\n Starting FC Test ..\n");

    #if !defined(VHWA_MCU_PLUS_SDK)
    App_startTimer();
    #endif

    if (FVID2_SOK == status)
    {
        while(!done)
        {
#if defined (LDRA_COVERAGE)
/* In LDRA mode, fetch testIdx from predefined list */
if(i < PositiveTestcasesMax)
{
    testIdx = gApprovedPositiveTestCases[i];
    i++;
}
else
{
    testIdx = gApprovedNegativeTestCases[j];
    j++;
}
#else
Vhwa_print("\n Enter Test to Run (Enter '-1' to exit)  \n");
#if defined(VHWA_MCU_PLUS_SDK)
scanf("%d", &testIdx);
#elif defined(VHWA_PDK)
testIdx = App_getNum();
#endif
#endif
            switch(testIdx)
            {
                case -1:
                    done = UTRUE;
                    break;

                default:
                    if((uint32_t)testIdx < (sizeof(gAppFcTestPrms)/ sizeof(AppFc_TestParams)))
                    {
                        FcApp_run(testIdx);
                    }
                    else
                    {
                        Vhwa_print("Invalid Test ID. Enter Again!!\n");
                    }
                    break;
            }
        }
    }

#if defined(LDRA_COVERAGE)
    Vhwa_print("################### DYNAMIC CODE COVERAGE TERMINATION STARTED ######################\n");
    ldra_terminate();
    Vhwa_print(" Exiting_FC \n");
    Vhwa_print("################### DYNAMIC CODE COVERAGE TERMINATION FINISHED ######################\n");
#endif

    Vhwa_print (" Exiting \n");
    FcApp_deInit();

    return;
}

void FcApp_run(uint32_t testCnt)
{
    uint32_t            hndlCnt;
    AppFc_TestParams *tPrms = NULL;
        
    #if defined(VHWA_MCU_PLUS_SDK)
    /* FreeRTOS task handles */
    TaskHandle_t taskHandles[VHWA_FC_DRV_MAX_HANDLES];

    taskParams taskParams[VHWA_FC_DRV_MAX_HANDLES];

    #else
    TaskP_Handle        task[VHWA_FC_DRV_MAX_HANDLES];
    TaskP_Params        taskParams[VHWA_FC_DRV_MAX_HANDLES];
    #endif

    tPrms = &gAppFcTestPrms[testCnt];

    gFcTestSrcBufFreeIdx = 0U;
    gFcTestDstBufFreeIdx = 0U;

    if (UTRUE == tPrms->isEnableTest)
    {
        Vhwa_print ("\n Starting Test %s\n", tPrms->testName);

        for (hndlCnt = 0U; hndlCnt < tPrms->numHandles;
                hndlCnt ++)
        {
            #if defined(VHWA_MCU_PLUS_SDK)
            /* Set task parameters */
            taskParams[hndlCnt].testObj = tPrms;
            taskParams[hndlCnt].handleIdx = hndlCnt;

            /* Start a new Task */
            taskHandles[hndlCnt] = xTaskCreateStatic(
                FcApp_testTaskWrapper,            /* Function that implements the task */
                "FC_TEST",                        /* Text name for debugging */
                MAIN_TASK_SIZE,                   /* Stack size in words */
                &taskParams[hndlCnt],             /* Parameters needed */
                APP_FC_TEST_TASKS_PRIORITY,       /* High priority */
                taskStacks[hndlCnt],              /* Stack buffer */
                &taskBuffers[hndlCnt]              /* Static task buffer */
            );
            if(NULL == taskHandles[hndlCnt])
            {
                Vhwa_print (" Task%d create failed... \n", hndlCnt);
                /* Delete previously created tasks before returning */
                while (hndlCnt > 0U)
                {
                    hndlCnt--;
                    vTaskDelete(taskHandles[hndlCnt]);
                }
                return;
            }
            #else
            /* Initialize the task params */
            TaskP_Params_init(&taskParams[hndlCnt]);

            /* Set the task priority higher than the
                default priority (1) */
            taskParams[hndlCnt].priority  = APP_FC_TEST_TASKS_PRIORITY;
            taskParams[hndlCnt].stack     = gAppTskStackTest[hndlCnt];
            taskParams[hndlCnt].stacksize = sizeof (gAppTskStackTest[hndlCnt]);
            taskParams[hndlCnt].arg0      = (void*)tPrms;
            taskParams[hndlCnt].arg1      = (void*)hndlCnt;

            /* Start a new Task */
            task[hndlCnt] = TaskP_create(&FcApp_test,
                                            &taskParams[hndlCnt]);
            if(NULL == task[hndlCnt])
            {
                Vhwa_print (" Task%d create failed... \n", hndlCnt);
                OS_stop();
            }
            #endif
        }

        /* Wait for all tesks to get completed */
        for (hndlCnt = 0U; hndlCnt < tPrms->numHandles; hndlCnt ++)
        {
            VhwaAl_SemaphoreP_pend(waitForTaskCmpl[hndlCnt],
                SYSTEMP_WAIT_FOREVER);

            Vhwa_print (" Handle %d processing Completed \n", hndlCnt);
            /* Delete Task */
            #if defined(VHWA_MCU_PLUS_SDK)
            if (NULL != taskHandles[hndlCnt])   
            {
                vTaskDelete(taskHandles[hndlCnt]);
            }
            #else
            if (NULL != task[hndlCnt])
            {
                TaskP_delete(&task[hndlCnt]);
            }
            #endif
        }
        Vhwa_print("\n Test %s Completed \n", tPrms->testName);
    }
    else
    {
        Vhwa_print ("\n Test %d - %s is Disabled \n", testCnt, tPrms->testName);
    }

    return;
}

static void FcApp_test(void* a0, void* a1)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                hndlIdx = (uint32_t )a1;
    uint32_t                repCnt;
    uint64_t                srcBufOffset, dstBufOffset;
    AppFc_TestParams        *tPrms = (AppFc_TestParams *)a0;
    uint32_t                inFrmSize, outFrmSize;

    status = AppFc_Create(tPrms, hndlIdx);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" FC_TEST_APP: Create Failed for %d\n", hndlIdx);
        status = FVID2_EFAIL;
    }

    if (FVID2_SOK == status)
    {
        status = AppFc_SetAllConfig(tPrms, hndlIdx);
        if (FVID2_SOK != status)
        {
            Vhwa_print (" FC_TEST_APP: Failed to Set all Config\n");
            status = FVID2_EFAIL;
        }
    }

    if (FVID2_SOK == status)
    {
        VhwaAl_SemaphoreP_pend(gAllocLock, SYSTEMP_WAIT_FOREVER);
        srcBufOffset = hndlIdx * VHWA_FC_TEST_IN_BUF_OFFSET;
        dstBufOffset = hndlIdx * VHWA_FC_TEST_OUT_BUF_OFFSET;

        status = AppFc_AllocBuffers(tPrms, hndlIdx,
            gFcTestSrcBuf+gFcTestSrcBufFreeIdx+srcBufOffset, &inFrmSize,
            gFcTestDestBuf+gFcTestDstBufFreeIdx+dstBufOffset, &outFrmSize);

        
        VhwaAl_SemaphoreP_post(gAllocLock);

        if (FVID2_SOK != status)
        {
            Vhwa_print (" FC_TEST_APP: Failed to Set all Config\n");
        }
    }

    AppFc_PrepareRequest(tPrms, hndlIdx);

    #if defined(VHWA_PDK)
    Vhwa_print(" Load buffers and press any key to continue\n");
    UART_getc();
    #endif

    if (FVID2_SOK == status)
    {
        for (repCnt = 0U; repCnt < tPrms->repeatCnt; repCnt ++)
        {
            status = AppFc_SubmitRequest(tPrms, hndlIdx);
            if (FVID2_SOK != status)
            {
                Vhwa_print (" FC_TEST_APP: Failed to Submit Request\n");
                break;
            }
            else
            {
                status = AppFc_WaitForCompRequest(tPrms, hndlIdx);
                if (FVID2_SOK != status)
                {
                    Vhwa_print (" FC_TEST_APP: Failed to Get back Request\n");
                    break;
                }
            }

            if (FVID2_SOK == status)
            {
                Vhwa_print (" Completed HandleCnt %d RepeatCnt %d\n",
                        hndlIdx, repCnt);
            }
        }
    }

    AppFc_Delete(tPrms, hndlIdx);

    VhwaAl_SemaphoreP_post(waitForTaskCmpl[hndlIdx]);
}

static int32_t FcApp_init(void)
{
    int32_t        status;
    uint32_t       instId, hndlCnt;
    Udma_InitPrms  udmaInitPrms;
    
    Udma_DrvHandle drvHandle = &gFcAppUdmaDrvObj;
    
    
    #if defined VHWA_PDK
    Board_initCfg  boardCfg;
    Fvid2_InitPrms initPrmsFvid2;
    SemaphoreP_Params params[VHWA_FC_DRV_MAX_HANDLES];
    SemaphoreP_Params       semAllocPrms;
    boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);

    Fvid2InitPrms_init(&initPrmsFvid2);
    initPrmsFvid2.printFxn = Vhwa_print;
    status = Fvid2_init(&initPrmsFvid2);
    #else
    status = Fvid2_init(NULL);
    #endif

    if (FVID2_SOK != status)
    {
        Vhwa_print(" main: FVID2_Init Failed !!!\r\n");
    }

    if (FVID2_SOK == status)
    {
        /* Initialize UDMA and get the handle, it will be used in both CRC layer,
           as well as in the driver */
        /* UDMA driver init */
        instId = VHWA_UDMA_INST_ID_0;
        status = UdmaInitPrms_init(instId, &udmaInitPrms);
        if(UDMA_SOK != status)
        {
            Vhwa_print("[Error] UDMA prms init failed!!\n");
            status = FVID2_EFAIL;
        }
        #if defined VHWA_PDK
        udmaInitPrms.printFxn = &App_udmaPrint;
        #else        
        udmaInitPrms.instId = VHWA_UDMA_INST_ID_0;
        udmaInitPrms.enableUtc = UTRUE;
        #endif
        status = Udma_init(drvHandle, &udmaInitPrms);
        if(UDMA_SOK != status)
        {
            Vhwa_print("[Error] UDMA init failed!!\n");
            status = FVID2_EFAIL;
        }
    }

    status = AppFc_Init(drvHandle);
    #if defined VHWA_PDK
    if (FVID2_SOK == status)
    {
        /* Initialize the UDMA channel for CRC */
        status = AppFc_CrcInit(drvHandle);
    }
    #endif

    if (FVID2_SOK == status)
    {
        for (hndlCnt = 0U; hndlCnt < VHWA_FC_DRV_MAX_HANDLES; hndlCnt ++)
        {
            #if defined(VHWA_MCU_PLUS_SDK)
            /* Use static initialization for FreeRTOS */
            status = SemaphoreP_constructBinary(&waitForTaskCmpl[hndlCnt], 0);
            if (status != SystemP_SUCCESS)
            {
                Vhwa_print(" Could not Create Semaphore for handle %d\n", hndlCnt);
                status = FVID2_EFAIL;
                break;
            }
            #else
            /* Dynamic allocation for PDK */
            SemaphoreP_Params_init(&params[hndlCnt]);
            params[hndlCnt].mode = SemaphoreP_Mode_BINARY;
            waitForTaskCmpl[hndlCnt] =
                SemaphoreP_create(0U, &params[hndlCnt]);
            if (NULL == waitForTaskCmpl[hndlCnt])
            {
                Vhwa_print (" Could not create Semaphore \n");
                status = FVID2_EFAIL;
                break;
            }
            #endif
        }


        /* Create allocation lock semaphore */
        #if defined(VHWA_MCU_PLUS_SDK)
        status = SemaphoreP_constructBinary(&gAllocLock, 1);
        if (status != SystemP_SUCCESS)
        {
            Vhwa_print(" Could not Create Allocation Lock Semaphore\n");
            status = FVID2_EFAIL;
        }
        #else
        SemaphoreP_Params_init(&semAllocPrms);
        semAllocPrms.mode = SemaphoreP_Mode_BINARY;

        gAllocLock = SemaphoreP_create(1U, &semAllocPrms);
        if (NULL == gAllocLock)
        {
            Vhwa_print("[Error] Sem create failed!!\n");
            status = FVID2_EFAIL;
        }
        #endif
    }
    return (status);
}

static void FcApp_deInit()
{
    int32_t         status;
    uint32_t        cnt;
    Udma_DrvHandle  drvHandle = &gFcAppUdmaDrvObj;

    Vhwa_m2mFcDeInit();

    #if defined(VHWA_PDK)
    AppFc_CrcDeinit(drvHandle);
    #endif

    status = Udma_deinit(drvHandle);
    if(UDMA_SOK != status)
    {
        Vhwa_print("[Error] UDMA deinit failed!!\n");
    }

    for (cnt = 0U; cnt < VHWA_FC_DRV_MAX_HANDLES; cnt ++)
    {
        #if defined(VHWA_MCU_PLUS_SDK)
        SemaphoreP_destruct(&waitForTaskCmpl[cnt]);
    #else
        if (NULL != waitForTaskCmpl[cnt])
        {
            SemaphoreP_delete(waitForTaskCmpl[cnt]);
        }
    #endif
    }

    #if defined(VHWA_MCU_PLUS_SDK)
    SemaphoreP_destruct(&gAllocLock);
    #else
    if (gAllocLock != NULL)
    {
        SemaphoreP_delete(gAllocLock);
        gAllocLock = NULL;
    }
    #endif


    Fvid2_deInit(NULL);

    #if defined(VHWA_MCU_PLUS_SDK)
    /* For MCU+ SDK, close drivers and deinitialize board */
    Board_driversClose();
    Drivers_close();
    Board_deinit();
    System_deinit();
    #endif
}

#if defined(VHWA_PDK)
int32_t App_getNum(void)
{
    int32_t num;

    UART_scanFmt("%d", &num);

    return (num);
}
#endif

#if defined(VHWA_MCU_PLUS_SDK)
static void FcApp_mainTaskFuncWrapper(void *args)
{
    FcApp_mainTaskFunc(NULL, NULL);
    vTaskDelete(NULL);
}

static void FcApp_testTaskWrapper(void *args)
{
    taskParams *params = (taskParams *)args;

    /* Call the original test function with the expected parameters */
    FcApp_test(params->testObj, (void *)params->handleIdx);

    /* Delete the task when done */
    vTaskDelete(NULL);
}
#endif

