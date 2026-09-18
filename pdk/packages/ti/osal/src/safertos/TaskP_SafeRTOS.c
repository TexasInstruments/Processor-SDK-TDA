/*
 * Copyright ( c ) 2018, Texas Instruments Incorporated
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
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT ( INCLUDING NEGLIGENCE OR
 * OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== TaskP_safertos.c ========
 */
#include <ti/osal/TaskP.h>

#include "SafeRTOS_priv.h"
#include "SafeRTOS_config.h"

portTaskHandleType TaskP_getSafeRTOSHandle(TaskP_Handle handle);
uint32_t TaskP_getTaskId(TaskP_Handle handle);
extern void LoadP_addTask(TaskP_Handle handle, uint32_t tskId);
extern void LoadP_removeTask(uint32_t tskId);

/**
 * \brief Value to be used for lowest priority task
 */
#define TaskP_PRIORITY_LOWEST       ( 0 )

/**
 * \brief Value to be used for highest priority task
 */
#define TaskP_PRIORITY_HIGHEST      ((int8_t)configMAX_PRIORITIES - 1)

/*
 * Defines the prototype of the task main functions.
 */
typedef void ( * TaskP_mainFunction_t )(  void *arg0, void *arg1 );

/*!
 *  @brief    Task structure
 */
typedef struct TaskP_SafeRTOS_s {
    bool                    used;
    xTCB                    taskObj;
#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
    xRTS                    taskRTS;
#endif
    uint32_t                tskId;
    portTaskHandleType      taskHndl;
    TaskP_mainFunction_t    taskfxn;
    void                    *arg0;
    void                    *arg1;
    bool                    terminated;
} TaskP_SafeRTOS;


/* The function that implements the task being created. */
static void TaskP_Function (void *arg);

/* global pool of statically allocated task pools */
static TaskP_SafeRTOS gOsalTaskPSafeRTOSPool[OSAL_SAFERTOS_CONFIGNUM_TASK];

uint32_t  gOsalTaskAllocCnt = 0U, gOsalTaskPeak = 0U;

extern uint32_t gSaftRtosInitDone;

static void TaskP_Function ( void *arg )
{
    TaskP_SafeRTOS *handle = ( TaskP_SafeRTOS * )( arg );

    /* Call the application function. */
    ( *handle->taskfxn )( handle->arg0, handle->arg1 );

    /* Task Fxn completed execution. */
    handle->terminated = BTRUE;
    /* Put vTaskSuspend in a loop just in case some calls vTaskResume, it will go back to suspend. */
    while (BTRUE)
    {
        xTaskSuspend(NULL);
    }
}

/*
 *  ======== TaskP_create ========
 */
TaskP_Handle TaskP_create(TaskP_Fxn taskfxn, const TaskP_Params *params )
{
    TaskP_Handle    ret_handle;
    TaskP_SafeRTOS *handle = ( TaskP_SafeRTOS * ) NULL_PTR;
    TaskP_SafeRTOS *taskPool;
    uint32_t        i;
    uintptr_t       key;
    uint32_t        maxTasks;
    int8_t          taskPriority;

    DebugP_assert( NULL != taskfxn );
    DebugP_assert( NULL_PTR != params );
    DebugP_assert( NULL_PTR != params->stack );
    /* Check if the OS_init is done. */
    DebugP_assert( UTRUE == gSaftRtosInitDone );
    /* Pick up the internal static memory block */
    taskPool        = ( TaskP_SafeRTOS * ) &gOsalTaskPSafeRTOSPool[0];
    maxTasks        = OSAL_SAFERTOS_CONFIGNUM_TASK;

    if( 0U == gOsalTaskAllocCnt )
    {
        ( void )memset(  ( void * )gOsalTaskPSafeRTOSPool,0,sizeof( gOsalTaskPSafeRTOSPool ) );
    }

    key = HwiP_disable(  );

     for( i = 0U; i < maxTasks; i++ )
     {
         if( BFALSE == taskPool[i].used )
         {
             taskPool[i].used = BTRUE;
             /* Update statistics */
             gOsalTaskAllocCnt++;
             if ( gOsalTaskAllocCnt > gOsalTaskPeak )
             {
                 gOsalTaskPeak = gOsalTaskAllocCnt;
             }
             break;
         }
     }
    HwiP_restore( key );

    if( i < maxTasks )
    {
        /* Grab the memory */
        handle = ( TaskP_SafeRTOS * ) &taskPool[i];
        handle->tskId = i;
    }

    if( NULL_PTR == handle ) {
        ret_handle = NULL_PTR;
    }
    else
    {
        portBaseType xCreateResult;

        taskPriority = params->priority;
        /* if prority is out of range, adjust to bring it in range */
        if( TaskP_PRIORITY_HIGHEST < taskPriority )
        {
            taskPriority = TaskP_PRIORITY_HIGHEST;
        }
        if( TaskP_PRIORITY_LOWEST >= taskPriority )
        {
            taskPriority = TaskP_PRIORITY_LOWEST;
        }
        /* Store the task function and args in taskP object. */
        handle->taskfxn = ( TaskP_mainFunction_t )( taskfxn );
        handle->arg0 = params->arg0;
        handle->arg1 = params->arg1;
        handle->terminated = BFALSE;


        /* The structure passed to xTaskCreate(  ) to create the check task. */
         xTaskParameters xTaskPParams =
         {
             ( pdTASK_CODE )&TaskP_Function,/* The function that implements the task being created. */
             ( portCharType* )params->name, /* The name of the task being created. The kernel does not use this itself, its just to assist debugging. */
             &handle->taskObj,              /* TCB for the task. */
             (portInt8Type *)params->stack,                 /* The buffer allocated for use as the task stack. */
             params->stacksize,             /* The size of the buffer allocated for use as the task stack - note this is in BYTES! */
             handle,                        /* The task parameter. */
             (portUnsignedBaseType)taskPriority,     /* The priority to assigned to the task being created. */
            #if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
                &handle->taskRTS,              /* User-defined data. */
            #else
                params->userData, 
            #endif
#if defined (BUILD_MCU)
             pdTRUE,                            /* Check task does not use the FPU. */
             {                                   /* MPU task parameters. */
                 params->taskPrivilege,          /* Check task is privileged. */
                 {
                     { NULL, 0U, 0U, 0U },       /* No additional region definitions are required. */
                     { NULL, 0U, 0U, 0U },
                 }
             }
#endif
         };

#if defined(BUILD_MCU)
         for (i=0; i<TASK_SPECIFIC_MPU_REGIONS; i++)
         {
            xTaskPParams.xMPUParameters.axRegions[i].pvBaseAddress = params->mpuCfg[i].regionBase;
            xTaskPParams.xMPUParameters.axRegions[i].ulLengthInBytes = (portUInt32Type)params->mpuCfg[i].regionLengthInBytes;
            xTaskPParams.xMPUParameters.axRegions[i].ulAccessPermissions = (portUInt32Type)params->mpuCfg[i].regionAccessPerms;
            xTaskPParams.xMPUParameters.axRegions[i].ulSubRegionControl = (portUInt32Type)params->mpuCfg[i].subregionControl;
         }
#endif

         /* Create the check task. */
        xCreateResult = xTaskCreate(&xTaskPParams,      /* The structure containing the task parameters created at the start of this function. */
                                    &handle->taskHndl); /* This parameter can be used to receive a handle to the created task, but is not used in this case. */

        if( ( pdFAIL == xCreateResult ) || ( NULL == handle->taskHndl ) )
        {
            /* If there was an error reset the task object and return NULL. */
            key = HwiP_disable(  );
            handle->used = BFALSE;
            /* Found the osal task object to delete */
            if( 0U < gOsalTaskAllocCnt )
            {
                gOsalTaskAllocCnt--;
            }
            HwiP_restore( key );
            ret_handle = NULL_PTR;
        }
        else
        {
        #if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
            LoadP_addTask((TaskP_Handle)handle, handle->tskId);
        #endif
            ret_handle = ( ( TaskP_Handle )handle );
        }
    }

    return ( ret_handle );
}

/*
 *  ======== TaskP_delete ========
 */
TaskP_Status TaskP_delete(TaskP_Handle *hTaskPtr)
{
    uintptr_t          key;
    TaskP_Status       ret = TaskP_OK;
    portTaskHandleType currentTaskHndl;
    portBaseType       xReturn;

    if(NULL == hTaskPtr)
    {
        ret = TaskP_FAILURE;
    }
    else
    {
        TaskP_Handle       hTask = *hTaskPtr;
        TaskP_SafeRTOS     *task = ( TaskP_SafeRTOS * )hTask;

        if((NULL_PTR != task) && (BTRUE == task->used))
        {
            currentTaskHndl = xTaskGetCurrentTaskHandle();
            if(currentTaskHndl == task->taskHndl)
            {
                /* This is task deleting itself. */
                DebugP_log0("Warning: xTaskDelete will not return when the task is deleting itself.\n"
                          "Resource freeing should be handled in deleteHookcallback which is not done currently\n"
                          "So there will be resource leak\n");
            }

            xReturn = xTaskDelete(task->taskHndl);
            task->terminated = BTRUE;
            DebugP_assert( pdPASS == xReturn );
        #if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
            LoadP_removeTask(task->tskId);
        #endif
            key = HwiP_disable(  );
            task->used      = BFALSE;
            task->taskHndl  = NULL;
            task->taskfxn   = NULL;
            task->arg0      = NULL;
            task->arg1      = NULL;

            (void)memset( ( void *)&task->taskObj, 0, sizeof(task->taskObj));

            /* Found the osal task object to delete */
            if(0U < gOsalTaskAllocCnt)
            {
                gOsalTaskAllocCnt--;
            }
            HwiP_restore( key );

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
void TaskP_Params_init( TaskP_Params *params )
{
#if defined (BUILD_MCU)
    uint32_t looper;
#endif

    params->name = ( const char * )"SafeRTOS_TASK";
    params->stacksize = 0U;
    params->stack = NULL;
    params->priority = ( TaskP_PRIORITY_HIGHEST - TaskP_PRIORITY_LOWEST ) / 2;
    params->arg0 = NULL;
    params->arg1 = NULL;
    params->userData = NULL;
    /* By default task will be privileged task, until set by the user */
#if defined (BUILD_MCU)
    params->taskPrivilege = mpuPRIVILEGED_TASK;
    for (looper=0; looper<TASK_SPECIFIC_MPU_REGIONS; looper++)
    {
        memset(&(params->mpuCfg[looper]), 0U, sizeof(TaskP_MPURegCfg));
    }
#endif
#if defined (BUILD_C66X)
    params->taskPrivilege = safertosapiPRIVILEGED_TASK;      /* Check task is privileged. */
#endif
}

void TaskP_sleep( uint32_t timeout )
{
    ( void )xTaskDelay( timeout );
}

void TaskP_sleepInMsecs( uint32_t timeoutInMsecs )
{
    uint32_t ticks;

    /* configTICK_RATE_MS is in units of msecs */
    ticks = timeoutInMsecs / (uint32_t)configTICK_RATE_MS;

    /* This API gaurantees a minimum delay for the number of mentioned milliseconds 
     * Adding a tick gaurantees the minimum delay limit */
    ( void )xTaskDelay( ticks + 1U );
}

void TaskP_setPrio( TaskP_Handle handle, uint32_t priority )
{
    TaskP_SafeRTOS *taskHandle = ( TaskP_SafeRTOS * )handle;

    DebugP_assert( NULL_PTR != handle );
    ( void )xTaskPrioritySet( taskHandle->taskHndl, priority );
}

TaskP_Handle TaskP_self( void )
{
    portTaskHandleType taskHndl;
    TaskP_Handle retHandle = NULL_PTR;
    uint32_t        i, maxTasks;

    taskHndl = xTaskGetCurrentTaskHandle();
    if(NULL_PTR != taskHndl)
    {
        /* Now get the corresponding TaskP Handle */
        maxTasks        = OSAL_SAFERTOS_CONFIGNUM_TASK;
        for(i = 0U; i < maxTasks; i++)
        {
            if((BTRUE == gOsalTaskPSafeRTOSPool[i].used) &&
               (gOsalTaskPSafeRTOSPool[i].taskHndl == taskHndl))
            {
                retHandle = (TaskP_Handle) (&gOsalTaskPSafeRTOSPool[i]);
                break;
            }
        }
    }

    return retHandle;
}

TaskP_Handle TaskP_selfmacro( void )
{
    /* For safertos task self is not implemented as inline macro.
     * So call the TaskP_self API itself.
     */
    return TaskP_self();
}

void TaskP_yield( void ) {
    safertosapiYIELD();
}

uint32_t TaskP_isTerminated( TaskP_Handle handle )
{
    uint32_t isTaskTerminated = UFALSE;
    TaskP_SafeRTOS *taskHandle = ( TaskP_SafeRTOS * )handle;

    DebugP_assert( NULL_PTR != handle );
    if(BFALSE == taskHandle->terminated)
    {
        isTaskTerminated = UFALSE;
    }
    else
    {
        isTaskTerminated = UTRUE;
    }
    return isTaskTerminated;
}

portTaskHandleType TaskP_getSafeRTOSHandle( TaskP_Handle handle )
{
    TaskP_SafeRTOS *taskHandle = ( TaskP_SafeRTOS * )handle;

    DebugP_assert( NULL_PTR != handle );
    DebugP_assert( BFALSE != taskHandle->used );

    return ( taskHandle->taskHndl );
}

uint32_t TaskP_getTaskId( TaskP_Handle handle )
{
    TaskP_SafeRTOS *taskHandle = ( TaskP_SafeRTOS * )handle;

    DebugP_assert( NULL_PTR != handle );
    DebugP_assert( BFALSE != taskHandle->used );

    return ( taskHandle->tskId );
}

uint32_t TaskP_getTaskStackHighWatermark(TaskP_Handle handle)
{
    TaskP_SafeRTOS *taskHandle = (TaskP_SafeRTOS *)handle;

    DebugP_assert(NULL_PTR != handle);
    DebugP_assert(BFALSE != taskHandle->used);

    /* SafeRTOS doesn't support this functionality, return 0 */

    return (0U);
}

void OS_start(void)
{
    /* Check if the OS_init is done. */
    DebugP_assert( UTRUE == gSaftRtosInitDone );

#if defined (BUILD_C7X)
    Hwi_switchFromBootStack();
#endif

    xTaskStartScheduler();
}

void OS_stop(void)
{
    vTaskSuspendScheduler();
}


/* Nothing past this point */
