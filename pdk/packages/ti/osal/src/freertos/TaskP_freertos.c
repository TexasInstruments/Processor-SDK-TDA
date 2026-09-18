/*
 * Copyright (c) 2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== TaskP_freertos.c ========
 */
#include <ti/osal/TaskP.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ti/csl/csl_types.h>
#include <ti/osal/osal.h>
#include <ti/osal/DebugP.h>
#include <ti/osal/soc/osal_soc.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/csl/arch/csl_arch.h>

#if defined (_TMS320C6X) && defined (SOC_J721E)
#include <ti/drv/sciclient/sciclient.h>
#include <ti/csl/csl_chipAux.h>
#endif

#include <FreeRTOS.h>
#include <task.h>

TaskHandle_t TaskP_getFreertosHandle(TaskP_Handle handle);
uint32_t TaskP_getTaskId(TaskP_Handle handle);
extern void LoadP_addTask(TaskP_Handle handle, uint32_t tskId);
extern void LoadP_removeTask(uint32_t tskId);
static int32_t prvC66xTickInterruptConfig( void );
static int32_t prvSetupTimerPSC( void );

/* The function that implements the task being created. */
static void TaskP_Function (void *arg);

/**
 * \brief Value to be used for lowest priority task
 */
#define TaskP_PRIORITY_LOWEST       (0)

/**
 * \brief Value to be used for highest priority task
 */
#define TaskP_PRIORITY_HIGHEST      (configMAX_PRIORITIES - 1)

/*
 * Defines the prototype of the task main functions.
 */
typedef void (* TaskP_mainFunction_t)( void *arg0, void *arg1);

/*!
 *  @brief    Task structure
 */
typedef struct TaskP_freertos_s {
    bool                    used;
    uint32_t                tskId;
    StaticTask_t            *taskObj;
    TaskHandle_t            taskHndl;
    TaskP_mainFunction_t    taskfxn;
    void                    *arg0;
    void                    *arg1;
    bool                    terminated;
} TaskP_freertos;

/* task obj is too large to keep in structure. So keep it in global array. */
static StaticTask_t   gOsalFreertosTaskObjPool[OSAL_FREERTOS_CONFIGNUM_TASK];
/* global pool of statically allocated task pools */
static TaskP_freertos gOsalTaskPfreertosPool[OSAL_FREERTOS_CONFIGNUM_TASK];

uint32_t  gOsalTaskAllocCnt = 0U, gOsalTaskPeak = 0U;

static void TaskP_Function (void *arg)
{
    TaskP_freertos *handle = (TaskP_freertos *)(arg);

    /* Call the application function. */
    (*handle->taskfxn)(handle->arg0, handle->arg1);

    /* Task Fxn completed execution. */
    handle->terminated = BTRUE;
    /* Put vTaskSuspend in a loop just in case some calls vTaskResume, it will go back to suspend. */
    while (BTRUE)
    {
        vTaskSuspend(NULL);
    }
    
}

/*
 *  ======== TaskP_create ========
 */
TaskP_Handle TaskP_create(TaskP_Fxn taskfxn, const TaskP_Params *params)
{
    TaskP_Handle    ret_handle;
    TaskP_freertos *handle = (TaskP_freertos *) NULL_PTR;
    TaskP_freertos *taskPool;
    uint32_t        i;
    uintptr_t       key;
    uint32_t        maxTasks;
    int8_t          taskPriority;

    DebugP_assert(NULL != taskfxn);
    DebugP_assert(NULL_PTR != params);
    DebugP_assert(NULL_PTR != params->stack);

    /* Pick up the internal static memory block */
    taskPool        = (TaskP_freertos *) &gOsalTaskPfreertosPool[0];
    maxTasks        = OSAL_FREERTOS_CONFIGNUM_TASK;
    
    if(0U == gOsalTaskAllocCnt) 
    {
        (void)memset( (void *)gOsalTaskPfreertosPool,0,sizeof(gOsalTaskPfreertosPool));
        (void)memset( (void *)gOsalFreertosTaskObjPool,0,sizeof(gOsalFreertosTaskObjPool));
    }

    key = HwiP_disable();

     for (i = 0; i < maxTasks; i++)
     {
         if (BFALSE == taskPool[i].used)
         {
             taskPool[i].used = BTRUE;
             taskPool[i].taskObj = &gOsalFreertosTaskObjPool[i];
             /* Update statistics */
             gOsalTaskAllocCnt++;
             if (gOsalTaskAllocCnt > gOsalTaskPeak)
             {
                 gOsalTaskPeak = gOsalTaskAllocCnt;
             }
             break;
         }
     }
    HwiP_restore(key);

    if (i < maxTasks)
    {
        /* Grab the memory */
        handle = (TaskP_freertos *) &taskPool[i];
        handle->tskId = i;
    }

    if (NULL_PTR == handle) {
        ret_handle = NULL_PTR;
    }
    else
    {
        taskPriority = params->priority;
        /* if prority is out of range, adjust to bring it in range */
        if(TaskP_PRIORITY_HIGHEST < taskPriority)
        {
            taskPriority = TaskP_PRIORITY_HIGHEST;
        }
        if(TaskP_PRIORITY_LOWEST >= taskPriority)
        {
            taskPriority = TaskP_PRIORITY_LOWEST;
        }
        /* Store the task function and args in taskP object. */
        handle->taskfxn = (TaskP_mainFunction_t)(taskfxn);
        handle->arg0 = params->arg0;
        handle->arg1 = params->arg1;
        handle->terminated = BFALSE;

        handle->taskHndl = xTaskCreateStatic((TaskFunction_t)TaskP_Function, /* Pointer to the function that implements the task. */
                                      (char *)params->name,              /* Text name for the task.  This is to facilitate debugging only. */
                                      (uint32_t)(params->stacksize/(sizeof(UBaseType_t))),  /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                      handle,       /* task specific args */
                                      (UBaseType_t)taskPriority,   /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                      (StackType_t*)params->stack,      /* pointer to stack base */
                                      handle->taskObj); /* pointer to statically allocated task object memory */
        if(NULL == handle->taskHndl)
        {
            /* If there was an error reset the task object and return NULL. */
            key = HwiP_disable();
            handle->used = BFALSE;
            /* Found the osal task object to delete */
            if (0U < gOsalTaskAllocCnt)
            {
                gOsalTaskAllocCnt--;
            }
            HwiP_restore(key);
            ret_handle = NULL_PTR;
        }
        else
        {
            LoadP_addTask((TaskP_Handle)handle, handle->tskId);
            ret_handle = ((TaskP_Handle)handle);
#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0)
            vTaskSetThreadLocalStoragePointer(handle->taskHndl, 0, params->userData);
#endif
        }
    }

    return (ret_handle);
}

/*
 *  ======== TaskP_delete ========
 */
TaskP_Status TaskP_delete(TaskP_Handle *hTaskPtr)
{
    uintptr_t       key;
    TaskP_Status    ret = TaskP_OK;
    TaskHandle_t    currentTaskHndl;

    if(NULL == hTaskPtr)
    {
        ret = TaskP_FAILURE;
    }
    else
    {
        TaskP_Handle    hTask = *hTaskPtr;
        TaskP_freertos  *task = (TaskP_freertos *)hTask;

        if((NULL_PTR != task) && (BTRUE == task->used))
        {
            currentTaskHndl = xTaskGetCurrentTaskHandle();
            if(currentTaskHndl == task->taskHndl)
            {
                /* This is task deleting itself. */
                DebugP_log0("Warning: vTaskDelete will not return when the task is deleting itself.\n"
                            "Resource freeing should be handled in deleteHookcallback which is not done currently\n"
                            "So there will be resource leak\n");
            }

            vTaskDelete(task->taskHndl);
            LoadP_removeTask(task->tskId);

            key = HwiP_disable();
            /* In FreeRTOS, task is deleted in the idle task.
            * So the TaskObj should not be memset to 0 */
            /* (void )memset( (void *)task->taskObj, 0, sizeof(task->taskObj)); */
            task->used      = BFALSE;
            task->taskObj   = NULL;
            task->taskHndl  = NULL;
            task->taskfxn   = NULL;
            task->arg0      = NULL;
            task->arg1      = NULL;

            /* Found the osal task object to delete */
            if(0U < gOsalTaskAllocCnt)
            {
                gOsalTaskAllocCnt--;
            }
            HwiP_restore(key);

            ret = TaskP_OK;
        }
        else
        {
            ret = TaskP_FAILURE;
        }
    }
    return (ret);
}

/*
 *  ======== TaskP_Params_init ========
 */
void TaskP_Params_init(TaskP_Params *params)
{
    if(NULL_PTR != params)
    {
        params->name = (const char *)"FREERTOS_TASK";
        params->stacksize = 0U;
        params->stack = NULL;
        params->priority = (TaskP_PRIORITY_HIGHEST - TaskP_PRIORITY_LOWEST) / 2;
        params->arg0 = NULL;
        params->arg1 = NULL;
        params->userData = NULL;
        params->taskPrivilege = 0U; /* Not Used */
    }
}

void TaskP_sleep(uint32_t timeout)
{
    vTaskDelay(timeout);
}

void TaskP_sleepInMsecs(uint32_t timeoutInMsecs)
{
    uint32_t ticks;

    /* portTICK_PERIOD_MS is in units of msecs */
    ticks = timeoutInMsecs / portTICK_PERIOD_MS;
    
    /* This API gaurantees a minimum delay for the number of mentioned milliseconds 
     * Adding a tick gaurantees the minimum delay limit */
    vTaskDelay(ticks + 1U);
}

void TaskP_setPrio(TaskP_Handle handle, uint32_t priority)
{
    TaskP_freertos *taskHandle = (TaskP_freertos *)handle;

    DebugP_assert(NULL_PTR != handle);
    vTaskPrioritySet(taskHandle->taskHndl, priority);
}

TaskP_Handle TaskP_self(void)
{
    TaskHandle_t taskHndl;
    TaskP_Handle retHandle = NULL_PTR;
    uint32_t        i, maxTasks;

    taskHndl = xTaskGetCurrentTaskHandle();
    if (NULL_PTR != taskHndl)
    {
        /* Now get the corresponding TaskP Handle */
        maxTasks        = OSAL_FREERTOS_CONFIGNUM_TASK;
        for (i = 0; i < maxTasks; i++)
        {
            if ((BTRUE == gOsalTaskPfreertosPool[i].used) &&
                (gOsalTaskPfreertosPool[i].taskHndl == taskHndl))
            {
                retHandle = (TaskP_Handle) (&gOsalTaskPfreertosPool[i]);
                break;
            }
        }
    }

    return retHandle;
}

TaskP_Handle TaskP_selfmacro(void)
{
    /* For freertos task self is not implemented as inline macro.
     * So call the TaskP_self API itself.
     */
    return TaskP_self();
}

void TaskP_yield(void) {
    taskYIELD();
}

uint32_t TaskP_isTerminated(TaskP_Handle handle)
{
    uint32_t isTaskTerminated = UFALSE;
    TaskP_freertos *taskHandle = (TaskP_freertos *)handle;

    DebugP_assert(NULL_PTR != handle);
    if((BTRUE == taskHandle->terminated) || (eDeleted == eTaskGetState(taskHandle->taskHndl)))
    {
        isTaskTerminated = UTRUE;
    }
    else
    {
        isTaskTerminated = UFALSE;
    }
    return isTaskTerminated;
}

uint32_t TaskP_disable(void)
{
    vTaskSuspendAll();

    return UFALSE;
}

void TaskP_restore(uint32_t key)
{
    (void)xTaskResumeAll();
}

TaskHandle_t TaskP_getFreertosHandle(TaskP_Handle handle)
{
    TaskP_freertos *taskHandle = (TaskP_freertos *)handle;

    DebugP_assert(NULL_PTR != handle);
    DebugP_assert(BFALSE != taskHandle->used);

    return (taskHandle->taskHndl);
}

uint32_t TaskP_getTaskId(TaskP_Handle handle)
{
    TaskP_freertos *taskHandle = (TaskP_freertos *)handle;

    DebugP_assert(NULL_PTR != handle);
    DebugP_assert(BFALSE != taskHandle->used);

    return (taskHandle->tskId);
}

uint32_t TaskP_getTaskStackHighWatermark(TaskP_Handle handle)
{
    TaskP_freertos *taskHandle = (TaskP_freertos *)handle;

    DebugP_assert(NULL_PTR != handle);
    DebugP_assert(BFALSE != taskHandle->used);

    return (uint32_t)(uxTaskGetStackHighWaterMark(taskHandle->taskHndl)*sizeof(UBaseType_t));
}

static int32_t prvSetupTimerPSC( void )
{
    int32_t xStatus = CSL_PASS;
#if (defined (SOC_J784S4) || defined (SOC_J742S2)) && defined (BUILD_MCU)
    CSL_ArmR5CPUInfo info = {0U, 0U, 0U};

    CSL_armR5GetCpuID(&info);
    if (CSL_ARM_R5_CLUSTER_GROUP_ID_0 != info.grpId)
    {
        Sciclient_ConfigPrms_t config;

        (void)Sciclient_configPrmsInit(&config);

        xStatus = Sciclient_init(&config);
        if(CSL_PASS == xStatus)
        {
            uint32_t currState, resetState, contextLossState;
            /* on J7AHP, Main domain timers 8-19 are connected to LPSC_PER_SPARE_0 which is not powered ON by default. 
             * All other timers are connected to LPSC which is ALWAYS_ON */
            xStatus = Sciclient_pmGetModuleState(TISCI_DEV_TIMER8, &currState, &resetState,
                                                &contextLossState, SCICLIENT_SERVICE_WAIT_FOREVER);
            if((CSL_PASS == xStatus) && ((uint32_t)TISCI_MSG_VALUE_DEVICE_SW_STATE_ON != currState))
            {
                xStatus = Sciclient_pmSetModuleState(TISCI_DEV_TIMER8, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                                    (TISCI_MSG_FLAG_AOP |TISCI_MSG_FLAG_DEVICE_RESET_ISO),
                                                    SCICLIENT_SERVICE_WAIT_FOREVER);
            }
        }
    }
#endif
    return xStatus;
}

void OS_init( void )
{
    int32_t xStatus = CSL_PASS;

    xStatus = prvC66xTickInterruptConfig();

    if(CSL_PASS == xStatus)
    {
        xStatus = prvSetupTimerPSC();
    }

    DebugP_assert(CSL_PASS == xStatus);
}

void OS_start(void)
{
    vTaskStartScheduler();
}

void OS_stop(void)
{
    vTaskEndScheduler();
}

static int32_t prvC66xTickInterruptConfig( void )
{
    int32_t xStatus = CSL_PASS;
#if defined (_TMS320C6X) && defined (SOC_J721E)
    Sciclient_ConfigPrms_t config;

    Sciclient_configPrmsInit(&config);

    xStatus = Sciclient_init(&config);
    if( CSL_PASS == xStatus )
    {
        struct tisci_msg_rm_irq_set_req     rmIrqReq;
        struct tisci_msg_rm_irq_set_resp    rmIrqResp;

        rmIrqReq.valid_params           = TISCI_MSG_VALUE_RM_DST_ID_VALID |
                                          TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
        rmIrqReq.src_index              = 0U;
        if ( 0U == CSL_chipReadDNUM() )
        {
            rmIrqReq.src_id                 = TISCI_DEV_TIMER0;
            rmIrqReq.dst_id                 = TISCI_DEV_C66SS0_CORE0;
            rmIrqReq.dst_host_irq           = 21U;
        }
        else
        {
            rmIrqReq.src_id                 = TISCI_DEV_TIMER1;
            rmIrqReq.dst_id                 = TISCI_DEV_C66SS1_CORE0;
            rmIrqReq.dst_host_irq           = 20U;
        }
        /* Unused params */
        rmIrqReq.global_event           = 0U;
        rmIrqReq.ia_id                  = 0U;
        rmIrqReq.vint                   = 0U;
        rmIrqReq.vint_status_bit_index  = 0U;
        rmIrqReq.secondary_host         = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

        xStatus =  Sciclient_rmIrqSet(&rmIrqReq, &rmIrqResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    }
#endif

    return xStatus;
}
/* Nothing past this point */
