/*
 *  Copyright (C) 2026 Texas Instruments Incorporated
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

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ti/osal/osal.h>
#include <ti/osal/DebugP.h>
#include <ti/osal/HwiP.h>
#include <ti/osal/SemaphoreP.h>
#include <ti/osal/TaskP.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ti/csl/soc.h>
#include <FREERTOS_log.h>
#if defined (BUILD_MCU1_0)
#include <ti/board/board.h>
#endif

/* ========================================================================== */
/*                 Private Typedefs                                           */
/* ========================================================================== */

typedef void * C7XVectorReg[8];

struct TaskContextType {
    C7XVectorReg VB0;
    C7XVectorReg VB1;
    C7XVectorReg VB2;
    C7XVectorReg VB3;
    C7XVectorReg VB4;
    C7XVectorReg VB5;
    C7XVectorReg VB6;
    C7XVectorReg VB7;
    C7XVectorReg VB8;
    C7XVectorReg VB9;
    C7XVectorReg VB10;
    C7XVectorReg VB11;
    C7XVectorReg VB12;
    C7XVectorReg VB13;
    C7XVectorReg VB14;
    C7XVectorReg VB15;
    void * AM0;
    void * AM1;
    void * AM2;
    void * AM3;
    void * AM4;
    void * AM5;
    void * AM6;
    void * AM7;
};

/* ========================================================================== */
/*                 Private Definitions and Macros                             */
/* ========================================================================== */

/* Define a static structure to capture the context register file in each task */
static struct TaskContextType TaskContextSwitch_TaskContext __attribute__ ((section(".data:TaskContextSwitch_TaskContext")));

/* ========================================================================== */
/*                 Private Function Declarations                              */
/* ========================================================================== */

TaskHandle_t TaskP_getFreertosHandle(TaskP_Handle handle);
static void printTwoVectors(const char * name1, void * vectorReg1[], const char * name2, void * vectorReg2[]);

/*
 * IMPORTANT NOTES:
 *
 * Task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest
 * For this example any valid task priority can be set.
 *
 * See FreeRTOSConfig.h for configMAX_PRIORITIES and StackType_t type.
 * FreeRTOSConfig.h can be found under kernel/freertos/config/${device}/${cpu}/
 *
 * In this example,
 * We create task's, semaphore's, ISR's and stack for the tasks using static allocation.
 * We dont need to delete these semaphore's since static allocation is used.
 *
 * One MUST not return out of a FreeRTOS task instead one MUST call vTaskDelete instead.
 */


/* Define the number of switches for the application. */ 
#define NUM_TASK_SWITCHES      (100u)

#ifdef SOC_J721E
    #ifdef BUILD_C7X_1
        #define PING_INT_NUM           (14u)
        #define PING_EVT_ID            (0x4E0)
        #define PONG_INT_NUM           (15u)
        #define PONG_EVT_ID            (0x4E1)
    #endif
#endif

#ifdef SOC_J721S2
    #ifdef BUILD_C7X_1
        #define PING_INT_NUM           (16U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER2_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (17U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER3_INTR_PEND_0 + 992)
    #endif
    #ifdef BUILD_C7X_2
        #define PING_INT_NUM           (18U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER4_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (19U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER5_INTR_PEND_0 + 992)
    #endif
#endif

#ifdef SOC_J784S4
    #ifdef BUILD_C7X_1
        #define PING_INT_NUM           (16U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER2_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (17U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER3_INTR_PEND_0 + 992)
    #endif
    #ifdef BUILD_C7X_2
        #define PING_INT_NUM           (18U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER4_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (19U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER5_INTR_PEND_0 + 992)
    #endif
    #ifdef BUILD_C7X_3
        #define PING_INT_NUM           (20U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER6_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (21U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER7_INTR_PEND_0 + 992)
    #endif
    #ifdef BUILD_C7X_4
        #define PING_INT_NUM           (22U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER8_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (23U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER9_INTR_PEND_0 + 992)
    #endif
#endif

#ifdef SOC_J742S2
    #ifdef BUILD_C7X_1
        #define PING_INT_NUM           (16U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER2_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (17U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER3_INTR_PEND_0 + 992)
    #endif
    #ifdef BUILD_C7X_2
        #define PING_INT_NUM           (18U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER4_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (19U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER5_INTR_PEND_0 + 992)
    #endif
    #ifdef BUILD_C7X_3
        #define PING_INT_NUM           (20U)
        #define PING_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER6_INTR_PEND_0 + 992)
        #define PONG_INT_NUM           (21U)
        #define PONG_EVT_ID            (CSLR_COMPUTE_CLUSTER0_CLEC_SOC_EVENTS_IN_TIMER7_INTR_PEND_0 + 992)
    #endif
#endif

#define PING_TASK_PRI  (2u)
#define PONG_TASK_PRI  (3u)

#define PING_TASK_SIZE (4096u)
StackType_t gPingTaskStack[PING_TASK_SIZE] __attribute__((aligned(32)));

#define PONG_TASK_SIZE (4096u)
StackType_t gPongTaskStack[PONG_TASK_SIZE] __attribute__((aligned(32)));

TaskP_Handle gPingTask;
SemaphoreP_Handle gPingSem;

TaskP_Handle gPongTask;
SemaphoreP_Handle gPongSem;

static void ping_isr(uintptr_t arg)
{
    SemaphoreP_post(gPongSem);
}

static void pong_isr(uintptr_t arg)
{
    SemaphoreP_post(gPingSem);
}

void ping_main(void *arg0, void *arg1)
{
    uint32_t count; /* loop `count` times */
    volatile uint64_t curTime; /* time in units of usecs */

    FREERTOS_log("\r\n");
    FREERTOS_log("[FreeRTOS] ping task ... start !!!\r\n");

    /* Modify Vector Registers so that they can be 
     * verified for context save-restore */
    asm("\t vmvk64.l2    0x0000000000000000LL, vb0 \n"
        "\t vmvk64.l2    0x0000000000000011LL, vb1 \n"
        "\t vmvk64.l2    0x0000000000002200LL, vb2 \n"
        "\t nop                                    \n"
        "\t vaddd.c2      vb1, vb2, vb3            \n"
        "\t vmvk64.l2    0x0000000000000033LL, vb4 \n"
        "\t vmvk64.l2    0x0000000000004400LL, vb5 \n"
        "\t nop                                    \n"
        "\t vaddd.c2      vb4, vb5, vb6            \n"
        "\t vmvk64.l2    0x0000000000000055LL, vb7 \n"
        "\t vmvk64.l2    0x0000000000006600LL, vb8 \n"
        "\t nop                                    \n"
        "\t vaddd.c2      vb7, vb8, vb9            \n"
        "\t vaddd.c2      vb0, vb0, vb10           \n"
        "\t vaddd.c2      vb0, vb1, vb11           \n"
        "\t vaddd.c2      vb0, vb2, vb12           \n"
        "\t vaddd.c2      vb0, vb3, vb13           \n"
        "\t vaddd.c2      vb0, vb4, vb14           \n"
        "\t vaddd.c2      vb0, vb5, vb15           \n");
    
    /* Modify AM Registers so that they can be 
     * verified for context save-restore */
    asm("\t mvku32.m1    0x0000001ALL, am0  \n"
        "\t mvku32.m1    0x00001A00LL, am1  \n"
        "\t nop                             \n"
        "\t addd.m1      am0, am1, am2      \n"
        "\t mvku32.m1    0x0000003ALL, am3  \n"
        "\t mvku32.m1    0x00003A00LL, am4  \n"
        "\t nop                             \n"
        "\t addd.m1      am3, am4, am5      \n"
        "\t mvku32.m1    0x0000005ALL, am6  \n"
        "\t addd.m1      am6, am6, am7");

    { /* switch between ping and pong tasks using semaphores */
        count = NUM_TASK_SWITCHES;
        curTime = uiPortGetRunTimeCounterValue();
        while(count--)
        {
            SemaphoreP_post(gPongSem);
            SemaphoreP_pend(gPingSem, SemaphoreP_WAIT_FOREVER);
            
            if (count % 10 == 0)
            {
                /* Load the Test Register context onto the static structure */
                asm("\t addkpc.d1 $PCR_OFFSET(TaskContextSwitch_TaskContext), a3 \n"
                    "\t vst8d.d2    vb0, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb1, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb2, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb3, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb4, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb5, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb6, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb7, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb8, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb9, *a3                                     \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb10, *a3                                    \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb11, *a3                                    \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb12, *a3                                    \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb13, *a3                                    \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb14, *a3                                    \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t vst8d.d2    vb15, *a3                                    \n"
                    "\t addd.d1     a3, 64, a3                                   \n"
                    "\t mv.m1       am0, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am1, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am2, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am3, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am4, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am5, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am6, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n"
                    "\t addd.d1     a3, 8, a3                                    \n"
                    "\t mv.m1       am7, a0                                      \n"
                    "\t std.d1      a0, *a3                                      \n");

                /* Print the Register File Values every 10 iterations */
                FREERTOS_log("[PING TASK] At Loop Count = %d the following is the state of the registers: \n", count);
                FREERTOS_log("[PING TASK] State of VB Vector Test Registers: \n");
                printTwoVectors("VB0", TaskContextSwitch_TaskContext.VB0, "VB1", TaskContextSwitch_TaskContext.VB1);
                printTwoVectors("VB2", TaskContextSwitch_TaskContext.VB2, "VB3", TaskContextSwitch_TaskContext.VB3);
                printTwoVectors("VB4", TaskContextSwitch_TaskContext.VB4, "VB5", TaskContextSwitch_TaskContext.VB5);
                printTwoVectors("VB6", TaskContextSwitch_TaskContext.VB6, "VB7", TaskContextSwitch_TaskContext.VB7);
                printTwoVectors("VB8", TaskContextSwitch_TaskContext.VB8, "VB9", TaskContextSwitch_TaskContext.VB9);
                printTwoVectors("VB10", TaskContextSwitch_TaskContext.VB10, "VB11", TaskContextSwitch_TaskContext.VB11);
                printTwoVectors("VB12", TaskContextSwitch_TaskContext.VB12, "VB13", TaskContextSwitch_TaskContext.VB13);
                printTwoVectors("VB14", TaskContextSwitch_TaskContext.VB14, "VB15", TaskContextSwitch_TaskContext.VB15);

                FREERTOS_log("[PING TASK] State of AM Test Registers: \n");
                FREERTOS_log("AM0=0x%x AM1=0x%x\r\n", TaskContextSwitch_TaskContext.AM0, TaskContextSwitch_TaskContext.AM1);
                FREERTOS_log("AM2=0x%x AM3=0x%x\r\n", TaskContextSwitch_TaskContext.AM2, TaskContextSwitch_TaskContext.AM3);
                FREERTOS_log("AM4=0x%x AM5=0x%x\r\n", TaskContextSwitch_TaskContext.AM4, TaskContextSwitch_TaskContext.AM5);
                FREERTOS_log("AM6=0x%x AM7=0x%x\r\n", TaskContextSwitch_TaskContext.AM6, TaskContextSwitch_TaskContext.AM7);             
            }
        }
        curTime = uiPortGetRunTimeCounterValue() - curTime;

        FREERTOS_log("\r\n");
        FREERTOS_log("execution time for task switches = %d ms\r\n", (uint32_t)(curTime/1000));
        FREERTOS_log("number of task switches = %d \r\n", (uint32_t)NUM_TASK_SWITCHES*2);
        FREERTOS_log("time per task switch (semaphore give/take) = %d ns\r\n", (uint32_t)(curTime*1000/(NUM_TASK_SWITCHES*2)));
    }
    { /* switch between ping and pong tasks using direct-to-task notifications */
        count = NUM_TASK_SWITCHES;
        curTime = uiPortGetRunTimeCounterValue();
        while(count--)
        {
            xTaskNotifyGive( TaskP_getFreertosHandle(gPongTask)); /* wake up pong task */
            ulTaskNotifyTake( pdTRUE, portMAX_DELAY); /* wait for pong to signal */
        }
        curTime = uiPortGetRunTimeCounterValue() - curTime;

        FREERTOS_log("\r\n");
        FREERTOS_log("execution time for task switches = %d ms\r\n", (uint32_t)(curTime/1000));
        FREERTOS_log("number of task switches = %d \r\n", (uint32_t)NUM_TASK_SWITCHES*2);
        FREERTOS_log("time per task switch (direct-to-task notification give/take) = %d ns\r\n", (uint32_t)(curTime*1000/(NUM_TASK_SWITCHES*2)));
    }
    { /* switch from ping task to ISR to pong task and back to ping task using semaphores, here there is a task switch */
        HwiP_Params hwiParams;
        HwiP_Handle hHwi;
        HwiP_Status hwiStatus;

        HwiP_Params_init(&hwiParams);
#if defined (BUILD_C7X)
        hwiParams.evtId = PING_EVT_ID;
#endif
        hHwi = HwiP_create(PING_INT_NUM, ping_isr, &hwiParams);
        DebugP_assert(hHwi != NULL);


        count = NUM_TASK_SWITCHES;
        curTime = uiPortGetRunTimeCounterValue();
        while(count--)
        {
            HwiP_post(PING_INT_NUM);
            SemaphoreP_pend(gPingSem, SemaphoreP_WAIT_FOREVER);
        }
        curTime = uiPortGetRunTimeCounterValue() - curTime;

        hwiStatus = HwiP_delete(hHwi);
        DebugP_assert(hwiStatus == HwiP_OK);

        FREERTOS_log("\r\n");
        FREERTOS_log("execution time for task - ISR - task - task switches = %d ms\r\n", (uint32_t)(curTime/1000));
        FREERTOS_log("number of ISRs = %d \r\n", (uint32_t)NUM_TASK_SWITCHES*2);
        FREERTOS_log("time per task - ISR - task switch (semaphore give/take) = %d ns\r\n", (uint32_t)(curTime*1000/(2*NUM_TASK_SWITCHES)));
    }

    /* delay some time, just to show delay works */
    vTaskDelay( 100 / portTICK_PERIOD_MS);
    vTaskDelay( 101 / portTICK_PERIOD_MS);

    FREERTOS_log("\r\n");
    FREERTOS_log("[FreeRTOS] ping task ... done !!!\r\n");

#if defined LDRA_DYN_COVERAGE_EXIT
    FREERTOS_log("\n LDRA ENTRY... \n");
    upload_execution_history();
    FREERTOS_log("\n LDRA EXIT... \n");
#endif

    FREERTOS_log("\r\n");
    FREERTOS_log("All tests have passed!!\r\n");
}

void pong_main(void *arg0, void *arg1)
{
    uint32_t count; /* loop `count` times */

    /* Modify Vector Registers so that they can be 
     * verified for context save-restore */
    asm("\t vmvk64.l2    0x0000000011111111LL, vb0 \n"
        "\t vmvk64.l2    0x1100000000000000LL, vb1 \n"
        "\t vmvk64.l2    0x0022000000000000LL, vb2 \n"
        "\t nop                                    \n"
        "\t vaddd.c2      vb1, vb2, vb3            \n"
        "\t vmvk64.l2    0x3300000000000000LL, vb4 \n"
        "\t vmvk64.l2    0x0044000000000000LL, vb5 \n"
        "\t nop                                    \n"
        "\t vaddd.c2      vb4, vb5, vb6            \n"
        "\t vmvk64.l2    0x5500000000000000LL, vb7 \n"
        "\t vmvk64.l2    0x0066000000000000LL, vb8 \n"
        "\t nop                                    \n"
        "\t vaddd.c2      vb7, vb8, vb9            \n"
        "\t vaddd.c2      vb0, vb0, vb10           \n"
        "\t vaddd.c2      vb0, vb1, vb11           \n"
        "\t vaddd.c2      vb0, vb2, vb12           \n"
        "\t vaddd.c2      vb0, vb3, vb13           \n"
        "\t vaddd.c2      vb0, vb4, vb14           \n"
        "\t vaddd.c2      vb0, vb5, vb15           \n");
    
    /* Modify AM Registers so that they can be 
     * verified for context save-restore */
    asm("\t mvku32.m1     0x1A000000LL, am0 \n"
        "\t mvku32.m1     0x001A0000LL, am1 \n"
        "\t nop                                    \n"
        "\t addd.m1      am0, am1, am2             \n"
        "\t mvku32.m1    0x3A000000LL, am3  \n"
        "\t mvku32.m1    0x003A0000LL, am4  \n"
        "\t nop                                    \n"
        "\t addd.m1      am3, am4, am5             \n"
        "\t mvku32.m1    0x5A000000LL, am6  \n"
        "\t addd.m1      am6, am6, am7");

    count = NUM_TASK_SWITCHES;
    while(count--)
    {
        SemaphoreP_pend(gPongSem, SemaphoreP_WAIT_FOREVER);
        SemaphoreP_post(gPingSem);
        
        if (count % 10 == 0)
        {
            /* Load the Test Register context onto the static structure */
            asm("\t addkpc.d1 $PCR_OFFSET(TaskContextSwitch_TaskContext), a3 \n"
                "\t vst8d.d2    vb0, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb1, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb2, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb3, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb4, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb5, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb6, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb7, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb8, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb9, *a3                                     \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb10, *a3                                    \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb11, *a3                                    \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb12, *a3                                    \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb13, *a3                                    \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb14, *a3                                    \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t vst8d.d2    vb15, *a3                                    \n"
                "\t addd.d1     a3, 64, a3                                   \n"
                "\t mv.m1       am0, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am1, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am2, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am3, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am4, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am5, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am6, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n"
                "\t addd.d1     a3, 8, a3                                    \n"
                "\t mv.m1       am7, a0                                      \n"
                "\t std.d1      a0, *a3                                      \n");

            /* Print the Register File Values every 10 iterations */
            FREERTOS_log("[PONG TASK] At Loop Count = %d the following is the state of the registers: \n", count);
            FREERTOS_log("[PONG TASK] State of VB Vector Test Registers: \n");
            printTwoVectors("VB0", TaskContextSwitch_TaskContext.VB0, "VB1", TaskContextSwitch_TaskContext.VB1);
            printTwoVectors("VB2", TaskContextSwitch_TaskContext.VB2, "VB3", TaskContextSwitch_TaskContext.VB3);
            printTwoVectors("VB4", TaskContextSwitch_TaskContext.VB4, "VB5", TaskContextSwitch_TaskContext.VB5);
            printTwoVectors("VB6", TaskContextSwitch_TaskContext.VB6, "VB7", TaskContextSwitch_TaskContext.VB7);
            printTwoVectors("VB8", TaskContextSwitch_TaskContext.VB8, "VB9", TaskContextSwitch_TaskContext.VB9);
            printTwoVectors("VB10", TaskContextSwitch_TaskContext.VB8, "VB11", TaskContextSwitch_TaskContext.VB9);
            printTwoVectors("VB12", TaskContextSwitch_TaskContext.VB8, "VB13", TaskContextSwitch_TaskContext.VB9);
            printTwoVectors("VB14", TaskContextSwitch_TaskContext.VB8, "VB15", TaskContextSwitch_TaskContext.VB9);

            FREERTOS_log("[PONG TASK] State of AM Test Registers: \n");
            FREERTOS_log("AM0=0x%x AM1=0x%x\r\n", TaskContextSwitch_TaskContext.AM0, TaskContextSwitch_TaskContext.AM1);
            FREERTOS_log("AM2=0x%x AM3=0x%x\r\n", TaskContextSwitch_TaskContext.AM2, TaskContextSwitch_TaskContext.AM3);
            FREERTOS_log("AM4=0x%x AM5=0x%x\r\n", TaskContextSwitch_TaskContext.AM4, TaskContextSwitch_TaskContext.AM5);
            FREERTOS_log("AM6=0x%x AM7=0x%x\r\n", TaskContextSwitch_TaskContext.AM6, TaskContextSwitch_TaskContext.AM7);            
        }
    }
    count = NUM_TASK_SWITCHES;
    while(count--)
    {
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY); /* wait for ping to signal */
        xTaskNotifyGive( TaskP_getFreertosHandle(gPingTask)); /* wake up ping task */
    }
    {
        HwiP_Params hwiParams;
        HwiP_Handle hHwi;
        HwiP_Status hwiStatus;

        HwiP_Params_init(&hwiParams);
#if (defined(_TMS320C6X) || defined (BUILD_C7X))
        hwiParams.evtId = PONG_EVT_ID;
#endif
        hHwi = HwiP_create(PONG_INT_NUM, pong_isr, &hwiParams);
        DebugP_assert(hHwi != NULL);

        count = NUM_TASK_SWITCHES;
        while(count--)
        {
            SemaphoreP_pend(gPongSem, SemaphoreP_WAIT_FOREVER);
            HwiP_post(PONG_INT_NUM);
        }
        hwiStatus = HwiP_delete(hHwi);
        DebugP_assert(hwiStatus == HwiP_OK);
    }
}

void task_context_switch_main(void *args)
{
    SemaphoreP_Params semParams;
    TaskP_Params      taskParams;

    SemaphoreP_Params_init(&semParams);
    semParams.mode = SemaphoreP_Mode_BINARY;

    /* Create semaphores for Ping & Pong Tasks */
    gPingSem = SemaphoreP_create(0, &semParams);
    configASSERT(gPingSem != NULL);
    gPongSem = SemaphoreP_create(0, &semParams);
    configASSERT(gPongSem != NULL);

    /* Initialize the Parameters to create Pong Task */
    TaskP_Params_init(&taskParams);
    taskParams.name = "pong";
    taskParams.stacksize = sizeof(gPongTaskStack);
    taskParams.stack = gPongTaskStack;
    taskParams.priority = PONG_TASK_PRI;
    taskParams.arg0 = NULL;

    /* Create the Pong Task */
    gPongTask = TaskP_create(&pong_main, &taskParams);
    configASSERT(gPongTask != NULL);

    /* Initialize the Parameters to create Ping Task */
    TaskP_Params_init(&taskParams);
    taskParams.name = "ping";
    taskParams.stacksize = sizeof(gPingTaskStack);
    taskParams.stack = gPingTaskStack;
    taskParams.priority = PING_TASK_PRI;
    taskParams.arg0 = NULL;

    /* Create the Ping Task */
    gPingTask = TaskP_create(&ping_main, &taskParams);
    configASSERT(gPingTask != NULL);
}

static void printTwoVectors(const char * name1, void * vectorReg1[], const char * name2, void * vectorReg2[])
{
    int i;

    FREERTOS_log("\r\n%s=0x%x [0]    %s=0x%x [0]\r\n    ", name1, vectorReg1[0], name2, vectorReg2[0]);
    
    for (i = 1; i < 8; i++) {
        FREERTOS_log("0x%x [%d]        0x%x [%d]\r\n    ", vectorReg1[i], i, vectorReg2[i], i);
    }
}

#if defined (BUILD_C7X)
extern void Osal_initMmuDefault(void);
#include <ti/csl/arch/csl_arch.h>

void InitMmu(void)
{
    Osal_initMmuDefault();
}
#endif

