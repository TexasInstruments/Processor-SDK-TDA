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
/**
 *  \ingroup DRV_OSAL_MODULE
 *  \defgroup DRV_OSAL_TaskP TaskP
 *            TaskP interface
 *
 *  @{
 */
/** ============================================================================
 *  @file       TaskP.h
 *
 *  @brief      Task module for the RTOS Porting Interface
 *
 *
 *  ============================================================================
 */

#ifndef ti_osal_TaskP__include
#define ti_osal_TaskP__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*!
 *  @brief    Status codes for TaskP APIs
 */
typedef enum TaskP_Status_e
{
    /*! API completed successfully */
    TaskP_OK         = 0,
    /*! API failed */
    TaskP_FAILURE    = (-(int32_t)1)
} TaskP_Status;

/*!
 *  @brief    Number Ticks by OS in 1 millisecond
 */
#define OS_TICKS_IN_MILLI_SEC   1 /* 1000us tick */

#define TASK_SPECIFIC_MPU_REGIONS  (2U)

/*!
 *  @brief    Opaque client reference to an instance of a TaskP
 *
 *  A TaskP_Handle returned from the ::TaskP_create represents that
 *  instance and  is used in the other instance based functions
 */
typedef  void *TaskP_Handle;

/*!
 *  @brief    [SafeRTOS only] Privilege of the task in safertos
 *
 *  A  TaskP_Privilege_Mode can have two values as below.
 *  mpuUNPRIVILEGED_TASK value is 0.
 *  mpuPRIVILEGED_TASK value is 1.
 */
typedef unsigned long TaskP_Privilege_Mode;

/*!
 *  @brief  Prototype for TaskP function.
 */
typedef void ( * TaskP_Fxn )( void *arg0, void *arg1 );

typedef struct TaskP_MPURegCfg_s
{
    void     *regionBase;
    uint64_t regionLengthInBytes;
    uint32_t regionAccessPerms;
    uint32_t subregionControl;
} TaskP_MPURegCfg;

/*!
 *  @brief    Basic TaskP Parameters
 *
 *  Structure that contains the parameters are passed into ::TaskP_create
 *  when creating a TaskP instance.
 */
typedef struct TaskP_Params_s
{
    const char *name;                                  /*!< Name of the task instance.                  */
    void *pErrBlk;                                     /*!< Pointer to the error block for task Create */
    int8_t priority;                                   /*!< The priority of the task                    */
    uint32_t stacksize;                                /*!< The stack size of the task                  */
    void *arg0;                                        /*!< argument 0                                        */
    void *arg1;                                        /*!< argument 1                                        */
    void *stack;                                       /*!< pointer to stack memory, shall be non-null value */
    void *userData;                                    /*!< [SafeRTOS, FreeRTOS only] Pointer to user-defined data */
    TaskP_MPURegCfg mpuCfg[TASK_SPECIFIC_MPU_REGIONS];    /*!< [SafeRTOS only] Task specific MPU Region configurations*/
    TaskP_Privilege_Mode taskPrivilege;                /*!< [SafeRTOS only] Privilege mode of the task */
} TaskP_Params;

/*!
 *  @brief  Function to create a task.
 *
 *  @param  taskfxn  Function pointer of the task.
 *
 *  @param  params  Pointer to the instance configuration parameters.
 *
 *  @return A TaskP_Handle on success or a NULL on an error
 */
extern TaskP_Handle TaskP_create(TaskP_Fxn taskfxn,
                                 const TaskP_Params *params);
/*!
 *  @brief  Function to delete a task.
 *
 *  @param  handle  A TaskP_Handle returned from TaskP_create
 *
 *  @return Status of the functions
 *    - TaskP__OK: Deleted the semaphore instance
 *    - TaskP_FAILURE: Failed to delete the semaphore instance
 */
extern TaskP_Status TaskP_delete(TaskP_Handle *handle);

/*!
 *  @brief  Initialize params structure to default values.
 *
 *  @param params  Pointer to the instance configuration parameters.
 */
extern void TaskP_Params_init(TaskP_Params *params);

/*!
 *  @brief  Function for Task sleep in units of OS tick
 *
 *  @param timeout  sleep ticks.
 */
extern void TaskP_sleep(uint32_t timeout);

/*!
 *  @brief  Function for Task sleep in units of msecs
 *  @note   This API gaurantees the timeout interval specified as the 
 *          minimum delay in number of ticks.
 *
 *  @param timeoutInMsecs  sleep in units of msecs.
 */
extern void TaskP_sleepInMsecs(uint32_t timeoutInMsecs);

/*!
 *  @brief  Function to update Task priority.
 *
 *  @param handle  A TaskP_Handle returned from TaskP_create.
 *  @param priority  New priority to be set.
 */
extern void TaskP_setPrio(TaskP_Handle handle, uint32_t priority);

/*!
 *  @brief  Function returns the Task handle of current task
 *
 */
extern TaskP_Handle TaskP_self(void);

/*!
 *  @brief  Function returns the Task handle of current task.This is an inline function
 *
 */
extern TaskP_Handle TaskP_selfmacro(void);

/*!
 *  @brief  Function Yield processor to equal priority task
 *
 */
extern void TaskP_yield(void);

/*!
 *  @brief  Check if task is terminated.
 *
 * Typically a task MUST be terminated before it can be deleted.
 *
 *  @return 0: task is not terminated, 1: task is terminated
 */
extern uint32_t TaskP_isTerminated(TaskP_Handle handle);

/*!
 *  @brief  Disable the task scheduler.
 *
 * Control Task scheduling. This disables all other Tasks from running until ::TaskP_restore is called.
 * Interrupts can still run.
 *
 *  @return key for use with restore
 */
extern uint32_t TaskP_disable(void);

/*!
 *  @brief  Restore Task scheduling state
 *
 *  Resumes the scheduler after it was disabled using a call to ::TaskP_disable.
 *
 *  @param key  key to restore previous Task scheduler state
 */
extern void TaskP_restore(uint32_t key);

/*!
 *  @brief  Return the task stack high watermark value (in bytes)
 *
 *  This represents the smallest distance in bytes thus far before experiencing a stack overflow
 *
 *  @param handle  A TaskP_Handle returned from TaskP_create.
 */
extern uint32_t TaskP_getTaskStackHighWatermark(TaskP_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_osal_TaskP__include */
/* @} */
