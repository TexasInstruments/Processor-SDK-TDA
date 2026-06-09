/*
 *  Copyright (c) Texas Instruments Incorporated 2019-2021
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

#include <include/vhwa_m2mViss.h>
#include <examples/include/vhwa_viss_test_api.h>
#include "vhwa_viss_cfg.h"
#if defined(LDRA_COVERAGE)
#include <examples/include/ldra_vhwa_coverage_main.h>
#endif

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
#define APP_VISS_MAIN_TASKS_PRIORITY (configMAX_PRIORITIES - 1)
#define APP_VISS_TEST_TASKS_PRIORITY (configMAX_PRIORITIES - 2)
#elif defined(VHWA_PDK)
#define APP_VISS_TSK_STACK_MAIN (64U * 1024U)
#define APP_VISS_MAIN_TASKS_PRIORITY (7U)
#define APP_VISS_TEST_TASKS_PRIORITY (5U)
#endif

/* Input Offset between two test cases, used mainly for zebu/qt,
 * so that buffers can be loaded in one shot */
#define VHWA_VISS_TEST_IN_BUF_OFFSET (8 * 1024 * 1024)

/* Output Offset between two test cases, used mainly for zebu/qt,
 * so that buffers can be saved in one shot */
#define VHWA_VISS_TEST_OUT_BUF_OFFSET (16 * 1024 * 1024)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

#if defined(VHWA_MCU_PLUS_SDK)
/* Define task parameters storage once */
typedef struct
{
    AppViss_TestParams *testObj;
    uint32_t handleIdx;
} taskParams;
#endif

static VhwaAl_SemaphoreP_Struct waitForTaskCmpl[VHWA_M2M_VISS_MAX_HANDLES];

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void VissApp_mainTaskFunc(void *a0, void *a1);
static void VissApp_test(void *a0, void *a1);
static int32_t VissApp_init(void);
static void VissApp_run(uint32_t testCnt);
static void VissApp_deInit(void);
#if defined(VHWA_PDK)
static int32_t App_getNum(void);
#endif
#if defined(VHWA_MCU_PLUS_SDK)
static void VissApp_mainTaskFuncWrapper(void *args);
static void VissApp_testTaskFuncWrapper(void *args);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Test application stack */
#if defined(VHWA_MCU_PLUS_SDK)
static StackType_t gMainTaskStack[MAIN_TASK_SIZE]
    __attribute__((aligned(32)));
static StaticTask_t gMainTaskObj;
static StackType_t taskStacks[VHWA_M2M_VISS_MAX_HANDLES][MAIN_TASK_SIZE]
    __attribute__((aligned(32)));
static StaticTask_t taskBuffers[VHWA_M2M_VISS_MAX_HANDLES];

#elif defined(SAFERTOS)
static uint8_t gAppTskStackMain[APP_VISS_TSK_STACK_MAIN]
    __attribute__((aligned(APP_VISS_TSK_STACK_MAIN))) = {0};
static uint8_t gAppTskStackTest[VHWA_M2M_VISS_MAX_HANDLES][APP_VISS_TSK_STACK_MAIN]
    __attribute__((aligned(APP_VISS_TSK_STACK_MAIN))) = {0};
#else
static uint8_t gAppTskStackMain[APP_VISS_TSK_STACK_MAIN]
    __attribute__((aligned(32)));
static uint8_t gAppTskStackTest[VHWA_M2M_VISS_MAX_HANDLES][APP_VISS_TSK_STACK_MAIN]
    __attribute__((aligned(32)));
#endif

/*
* Application Buffers
*/
#if defined(VHWA_MCU_PLUS_SDK)
static uint64_t gVissTestSrcBuf = (uint64_t)(0xB0000000u);
static uint64_t gVissTestDestBuf = (uint64_t)(0xB8000000u);
#else
static uint64_t gVissTestSrcBuf =
    (uint64_t)(VHWA_EXAMPLE_BUFF_START_ADDR);
static uint64_t gVissTestDestBuf =
    (uint64_t)(VHWA_EXAMPLE_BUFF_START_ADDR + 0x10000000u);
#endif

#if defined(VHWA_MCU_PLUS_SDK)
static uint64_t gVissTestNsf4HistDestBuf =
    (uint64_t)(0xB0000000u + 0x20000000u);
static uint64_t gVissTestFcp1HistDestBuf =
    (uint64_t)(0xB8000000u + 0x20100000u);
#else
static uint64_t gVissTestNsf4HistDestBuf =
    (uint64_t)(VHWA_EXAMPLE_BUFF_START_ADDR + 0x20000000u);
static uint64_t gVissTestFcp1HistDestBuf =
    (uint64_t)(VHWA_EXAMPLE_BUFF_START_ADDR + 0x20100000u);

#if defined (VHWA_VPAC_IP_REV_VPAC3)
static uint64_t gVissTestFcp2HistDestBuf =
    (uint64_t)(VHWA_EXAMPLE_BUFF_START_ADDR + 0x20200000u);
#endif

static uint32_t *gVissTestConfigBuf =
    (uint32_t *)(VHWA_EXAMPLE_CONFIG_BUFF_START_ADDR);
static uint32_t gVissTestConfigBufFreeIdx = 0u;
#endif
static uint32_t gVissTestSrcBufFreeIdx = 0u;
static uint32_t gVissTestDstBufFreeIdx = 0u;

#if defined(VHWA_MCU_PLUS_SDK)
static VhwaAl_SemaphoreP_Struct gAllocLock;
#else
static VhwaAl_SemaphoreP_Struct gAllocLock = {NULL};
#endif

extern AppViss_TestParams gAppVissTestPrms[];

static VhwaAl_Udma_DrvObj_Struct gVissAppUdmaDrvObj;

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

    /* Create main task */
    gMainTask = xTaskCreateStatic(
        VissApp_mainTaskFuncWrapper,  /* Function that implements the task */
        "viss_main_task",             /* Text name for debugging */
        MAIN_TASK_SIZE,               /* Stack size in words */
        NULL,                         /* No parameters needed */
        APP_VISS_MAIN_TASKS_PRIORITY, /* High priority */
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

    taskParams.priority = APP_VISS_MAIN_TASKS_PRIORITY;
    taskParams.stack = gAppTskStackMain;
    taskParams.stacksize = sizeof(gAppTskStackMain);

    task = TaskP_create(&VissApp_mainTaskFunc, &taskParams);
    if (NULL == task)
    {
        OS_stop();
    }

    OS_start(); /* does not return */
#endif

    return (0);
}

static void VissApp_mainTaskFunc(void *a0, void *a1)
{
    int32_t status;
    bool done = UFALSE;
    int32_t testIdx;
    #if defined (LDRA_COVERAGE)
    static const int32_t gApprovedPositiveTestCases [] = {92,7,6,5,4,3,2,1,91,82,89,88,87,86,90,84,83,81,79,85,0,93,112,124,126,139,140,151,250,118,117} ;
    static const int32_t gApprovedNegativeTestCases [] = {236,237,238,239,240,243,244,245,246,247,224,225,226,229,230,231,232,233,234,235,214,215,216,217,219,221,202,206,207,208,209,210,211,199,200,201,191,192,193,194,195,161,162,164,165,166,167,168,169,170,171,172,173,174,175,176,178,179,180,181,182,183,184,150,152,153,154,155,156,143,144,134,130,131,125,113,249,198,119,120,129,103,104,106,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,203,218,-1};
    uint32_t i = 0,j =0, PositiveTestcasesMax = 0; 
    PositiveTestcasesMax = sizeof(gApprovedPositiveTestCases)/sizeof(gApprovedPositiveTestCases[0]);
    #endif

#if defined(LDRA_COVERAGE)
    Vhwa_print("################### DYNAMIC CODE COVERAGE INITIALIZATION STARTED ######################\n");
    ldra_initialize();
    Vhwa_print("################### DYNAMIC CODE COVERAGE INITIALIZATION FINISHED ######################\n");
#endif

    status = VissApp_init();

    Vhwa_print("\n Starting VISS Test ..\n");

#if !defined(VHWA_MCU_PLUS_SDK)
    App_startTimer();
#endif

    if (FVID2_SOK == status)
    {
        while (!done)
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
if(testIdx == -1)
{
    break;
}
#else
            Vhwa_print("\n Enter Test to Run (Enter '-1' to exit)  \n");
#if defined(VHWA_MCU_PLUS_SDK)
            scanf("%d", &testIdx);
#else
            testIdx = App_getNum();
#endif
#endif

            switch (testIdx)
            {
            case -1:
                done = UTRUE;
                break;

            default:
                if ((uint32_t)testIdx < (sizeof(gAppVissTestPrms) / sizeof(AppViss_TestParams)))
                {
                    VissApp_run(testIdx);
                }
                else
                {
                    Vhwa_print("Invalid Test ID. Enter Again!!\n");
                }
                break;
            }
        }
    }

    VissApp_deInit();
    #if defined (LDRA_COVERAGE)
    Vhwa_print("################### DYNAMIC CODE COVERAGE TERMINATION STARTED ######################\n");
    ldra_terminate();
    Vhwa_print (" Exiting_VISS \n");
    Vhwa_print("################### DYNAMIC CODE COVERAGE TERMINATION FINISHED ######################\n");
    #endif 
    Vhwa_print (" Exiting \n");
    return;
}

void VissApp_run(uint32_t testCnt)
{
    uint32_t hndlCnt;
    AppViss_TestParams *tPrms = NULL;

#if defined(VHWA_MCU_PLUS_SDK)
    /* FreeRTOS task handles */
    TaskHandle_t taskHandles[VHWA_M2M_VISS_MAX_HANDLES];

    taskParams taskParams[VHWA_M2M_VISS_MAX_HANDLES];

#else
    TaskP_Handle task[VHWA_M2M_VISS_MAX_HANDLES];
    TaskP_Params taskParams[VHWA_M2M_VISS_MAX_HANDLES];

#endif

    tPrms = &gAppVissTestPrms[testCnt];

    gVissTestSrcBufFreeIdx = 0U;
    gVissTestDstBufFreeIdx = 0U;

    if (UTRUE == tPrms->isEnableTest)
    {
        Vhwa_print("\n Starting Test %s\n", tPrms->testName);

        for (hndlCnt = 0U; hndlCnt < tPrms->numHandles;
             hndlCnt++)
        {
#if defined(VHWA_MCU_PLUS_SDK)
            /* Set task parameters */
            taskParams[hndlCnt].testObj = tPrms;
            taskParams[hndlCnt].handleIdx = hndlCnt;

            /* Create task */
            taskHandles[hndlCnt] = xTaskCreateStatic(
                VissApp_testTaskFuncWrapper,  /* Function that implements the task */
                "VISS_TEST",                  /* Task name */
                MAIN_TASK_SIZE,               /* Stack size in words */
                &taskParams[hndlCnt],         /* parameters needed */
                APP_VISS_TEST_TASKS_PRIORITY, /* High priority */
                taskStacks[hndlCnt],          /* Stack buffer */
                &taskBuffers[hndlCnt]         /* Static task buffer */
            );

            if (NULL == taskHandles[hndlCnt])
            {
                Vhwa_print(" Task%d create failed... \n", hndlCnt);
                /* Delete previously created tasks before returning */
                while (hndlCnt > 0U)
                {
                    hndlCnt--;
                    vTaskDelete(taskHandles[hndlCnt]);
                }
                return;
            }
#else
            /* Set task parameters for PDK */
            TaskP_Params_init(&taskParams[hndlCnt]);

            /* Set the task priority higher than the
                default priority (1) */
            taskParams[hndlCnt].priority = APP_VISS_TEST_TASKS_PRIORITY;
            taskParams[hndlCnt].stack = gAppTskStackTest[hndlCnt];
            taskParams[hndlCnt].stacksize = sizeof(gAppTskStackTest[hndlCnt]);
            taskParams[hndlCnt].arg0 = (void *)tPrms;
            taskParams[hndlCnt].arg1 = (void *)hndlCnt;

            /* Start a new Task */
            task[hndlCnt] = TaskP_create(&VissApp_test,
                                         &taskParams[hndlCnt]);
            if (NULL == task[hndlCnt])
            {
                Vhwa_print(" Task%d create failed... \n", hndlCnt);
                OS_stop();
            }
#endif
        }

        /* Wait for all tesks to get completed */
        for (hndlCnt = 0U; hndlCnt < tPrms->numHandles; hndlCnt++)
        {
            VhwaAl_SemaphoreP_pend(waitForTaskCmpl[hndlCnt],
                                   SYSTEMP_WAIT_FOREVER);

            Vhwa_print(" Handle %d processing Completed \n", hndlCnt);

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
        Vhwa_print("\n Test %d - %s is Disabled \n", testCnt, tPrms->testName);
    }

    return;
}

static void VissApp_test(void *a0, void *a1)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                hndlIdx = (uint32_t)a1;
    uint32_t                repCnt;
    uint64_t                timeCount;
    uint64_t                perf;
    uint64_t                srcBufOffset, dstBufOffset;
    AppViss_TestParams      *tPrms = (AppViss_TestParams *)a0;
    uint32_t                inFrmSize, outFrmSize;
#if defined VHWA_PDK
    Vhwa_M2mVissConfigAppBuff appBuffConfig;
#endif

    status = AppViss_Create(tPrms, hndlIdx);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" VISS_TEST_APP: Create Failed for %d\n", hndlIdx);
        status = FVID2_EFAIL;
    }

    if (FVID2_SOK == status)
    {
        VhwaAl_SemaphoreP_pend(gAllocLock, SYSTEMP_WAIT_FOREVER);

#if defined VHWA_PDK
        status = AppViss_GetConfigBufInfo(hndlIdx, &appBuffConfig);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Failed to Get config buf info\n");
            status = FVID2_EFAIL;
        }
        else
        {
            /* configThroughUdmaFlag should not be changed by application */
            if (true == appBuffConfig.configThroughUdmaFlag)
            {
                /* update config buffer pointer */
                appBuffConfig.bufferPtr = gVissTestConfigBuf + gVissTestConfigBufFreeIdx;

                status = AppViss_SetConfigBufInfo(hndlIdx, &appBuffConfig);
                if (FVID2_SOK != status)
                {
                    Vhwa_print(
                        " VISS_TEST_APP: Failed to Set config buf info\n");
                    status = FVID2_EFAIL;
                }
                else
                {
                    /* Move config Buffer Index in word size */
                    gVissTestConfigBufFreeIdx += (appBuffConfig.length / 4);
                }
            }
        }
#endif

        VhwaAl_SemaphoreP_post(gAllocLock);
    }

    if (FVID2_SOK == status)
    {
        status = AppViss_SetAllConfig(tPrms, hndlIdx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Failed to Set all Config\n");
            status = FVID2_EFAIL;
        }
    }

    if(FVID2_SOK == status)
    {
        if(UTRUE == tPrms->setHtsBwLimit)
        {
            status = AppViss_SetHtsLimitCfg(hndlIdx, tPrms->htsLimiterCfg);
            if (FVID2_SOK != status)
            {
                Vhwa_print (" VISS_TEST_APP: Failed to Set HTS Limit Config\n");
                status = FVID2_EFAIL;
            }
        }
    }

    if (FVID2_SOK == status)
    {
        VhwaAl_SemaphoreP_pend(gAllocLock, SYSTEMP_WAIT_FOREVER);
        srcBufOffset = hndlIdx * VHWA_VISS_TEST_IN_BUF_OFFSET;
        dstBufOffset = hndlIdx * VHWA_VISS_TEST_OUT_BUF_OFFSET;

        status = AppViss_AllocBuffers(tPrms, hndlIdx,
                                      gVissTestSrcBuf + gVissTestSrcBufFreeIdx + srcBufOffset, &inFrmSize,
                                      gVissTestDestBuf + gVissTestDstBufFreeIdx + dstBufOffset, &outFrmSize);

        /* If nsf4 Histogram is enabled, print the Buffer address */
        if (UTRUE == tPrms->nsf4RawHistEnable)
        {
            Vhwa_print(" NSF4 Histogram Buffer Address: 0x%llx\n",
                (unsigned long long)gVissTestNsf4HistDestBuf);
        }
        /* If fcp1 Histogram is enabled, print the Buffer address */
        if (UTRUE == tPrms->fcp1HistEnable)
        {
            Vhwa_print(" FCP1 Histogram Buffer Address: 0x%llx\n",
                (unsigned long long)gVissTestFcp1HistDestBuf);
        }
        #if defined (VHWA_VPAC_IP_REV_VPAC3)
        /* If fcp2 Histogram is enabled, print the Buffer address */
        if (UTRUE == tPrms->fcp2HistEnable)
        {
            Vhwa_print(" FCP2 Histogram Buffer Address: 0x%llx\n",
                (unsigned long long)gVissTestFcp2HistDestBuf);
        }
        #endif

        VhwaAl_SemaphoreP_post(gAllocLock);

        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Failed to Set all Config\n");
        }
    }

    AppViss_PrepareRequest(tPrms, hndlIdx);

#if defined(VHWA_PDK)
    Vhwa_print(" Load buffers and press any key to continue\n");
    UART_getc();
#endif

    if (tPrms->isPerformanceTest)
    {
        timeCount = Vhwa_getTimeUsec();
    }

    if (FVID2_SOK == status)
    {
        for (repCnt = 0U; repCnt < tPrms->repeatCnt; repCnt++)
        {
            status = AppViss_SubmitRequest(tPrms, hndlIdx);
            if (FVID2_SOK != status)
            {
                Vhwa_print(" VISS_TEST_APP: Failed to Submit Request\n");
                break;
            }
            else
            {
                status = AppViss_WaitForCompRequest(tPrms, hndlIdx, repCnt);
                if (FVID2_SOK != status)
                {
                    Vhwa_print(" VISS_TEST_APP: Failed to Get back Request\n");
                    break;
                }
            }
        }
        #if defined (VHWA_VPAC_IP_REV_VPAC3) || defined (VHWA_VPAC_IP_REV_VPAC3L)
        /* If nsf4 Histogram is enabled, take the histogram output */
        if (UTRUE == tPrms->nsf4RawHistEnable)
        {
            status = AppViss_getNsf4RawHistData(
                    hndlIdx,
                (void *)gVissTestNsf4HistDestBuf);
            if( FVID2_SOK != status)
            {
                Vhwa_print (" VISS_TEST_APP: Failed to get NSF4 Histogram\n");
            }
        }
        #endif
        /* If fcp1 Histogram is enabled, take the histogram output */
        if (UTRUE == tPrms->fcp1HistEnable)
        {
            status = AppViss_getFcp1HistData(
                    hndlIdx,
                (void *)gVissTestFcp1HistDestBuf);
            if( FVID2_SOK != status)
            {
                Vhwa_print (" VISS_TEST_APP: Failed to get FCP1 Histogram\n");
            }

        }
        #if defined (VHWA_VPAC_IP_REV_VPAC3)
        /* If fcp2 Histogram is enabled, take the histogram output */
        if (UTRUE == tPrms->fcp2HistEnable)
        {
            status = AppViss_getFcp2HistData(
                    hndlIdx,
                (void *)gVissTestFcp2HistDestBuf);
            if( FVID2_SOK != status)
            {
                Vhwa_print (" VISS_TEST_APP: Failed to get FCP2 Histogram\n");
            }
        }
        #endif

        if (tPrms->isPerformanceTest)
        {
            timeCount = Vhwa_getTimeUsec() - timeCount;
            Vhwa_print("Performance:\n\t FrameCount: %d: Time in uSec: %d\n",
                       tPrms->repeatCnt, timeCount);

            perf = (uint64_t)tPrms->testCfg[hndlIdx]->vissPrms.inFmt.width * (uint64_t)tPrms->testCfg[hndlIdx]->vissPrms.inFmt.height * (uint64_t)tPrms->repeatCnt;

            Vhwa_print("\t MPix/s: %d.%d\n",
                       (uint32_t)(perf / timeCount),
                       (uint32_t)(((perf * (uint64_t)100) / timeCount) % 100));
        }

        if (FVID2_SOK == status)
        {
            Vhwa_print(" Completed HandleCnt %d RepeatCnt %d\n",
                       hndlIdx, repCnt);
        }
    }

    AppViss_Delete(tPrms, hndlIdx);

    VhwaAl_SemaphoreP_post(waitForTaskCmpl[hndlIdx]);
}

static int32_t VissApp_init(void)
{
    int32_t                 status;
    uint32_t                instId, hndlCnt;
    Udma_InitPrms           udmaInitPrms;
    Udma_DrvHandle          drvHandle = &gVissAppUdmaDrvObj;

#if defined(VHWA_PDK)
    Board_initCfg boardCfg;
    Fvid2_InitPrms initPrmsFvid2;
    SemaphoreP_Params params[VHWA_M2M_VISS_MAX_HANDLES];
    SemaphoreP_Params semAllocPrms;
    /* Initialize board for PDK platforms */
    boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);

    /* Initialize FVID2 for PDK platforms */
    Fvid2InitPrms_init(&initPrmsFvid2);
    initPrmsFvid2.printFxn = Vhwa_print;
    status = Fvid2_init(&initPrmsFvid2);
#else
    /* For MCU+ SDK, FVID2 initialization is simpler */
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
        instId = VHWA_UDMA_INST_ID_0;
        status = UdmaInitPrms_init(instId, &udmaInitPrms);
        if (UDMA_SOK != status)
        {
            Vhwa_print("[Error] UDMA prms init failed!!\n");
            status = FVID2_EFAIL;
        }
        else
        {
#if defined(VHWA_PDK)
            udmaInitPrms.printFxn = &App_udmaPrint;
#else
            udmaInitPrms.instId = VHWA_UDMA_INST_ID_0;
            udmaInitPrms.enableUtc = UTRUE;
#endif
            status = Udma_init(drvHandle, &udmaInitPrms);
            if (UDMA_SOK != status)
            {
                Vhwa_print("[Error] UDMA init failed!!\n");
                status = FVID2_EFAIL;
            }
        }
    }

    /* Initialize the VISS module */
    if (FVID2_SOK == status)
    {
        status = AppViss_Init(drvHandle);
    }

    /* Initialize CRC functionality if needed */
    if (FVID2_SOK == status)
    {
#if defined(VHWA_PDK)
        status = AppViss_CrcInit(drvHandle);
#endif
    }

    /* Create synchronization semaphores */
    if (FVID2_SOK == status)
    {
        /* Create semaphores for task completion */
        for (hndlCnt = 0U; hndlCnt < VHWA_M2M_VISS_MAX_HANDLES; hndlCnt++)
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
            waitForTaskCmpl[hndlCnt] = SemaphoreP_create(0U, &params[hndlCnt]);
            if (NULL == waitForTaskCmpl[hndlCnt])
            {
                Vhwa_print(" Could not Create Semaphore for handle %d\n", hndlCnt);
                status = FVID2_EFAIL;
                break;
            }
#endif
        }
    }

    /* Create allocation lock semaphore */
    if (FVID2_SOK == status)
    {
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
            Vhwa_print(" Could not Create Allocation Lock Semaphore\n");
            status = FVID2_EFAIL;
        }
#endif
    }

    return status;
}
static void VissApp_deInit()
{
    int32_t status;
    uint32_t cnt;
    Udma_DrvHandle drvHandle = &gVissAppUdmaDrvObj;

    Vhwa_m2mVissDeInit();

#if defined(VHWA_PDK)
    AppViss_CrcDeinit(drvHandle);
#endif

    status = Udma_deinit(drvHandle);
    if (UDMA_SOK != status)
    {
        Vhwa_print("[Error] UDMA deinit failed!!\n");
    }

    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt++)
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
static void VissApp_mainTaskFuncWrapper(void *args)
{
    VissApp_mainTaskFunc(NULL, NULL);

    vTaskDelete(NULL);
}

static void VissApp_testTaskFuncWrapper(void *args)
{
    taskParams *params = (taskParams *)args;

    VissApp_test(params->testObj, (void *)params->handleIdx);

    vTaskDelete(NULL);
}
#endif