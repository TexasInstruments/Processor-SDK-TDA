/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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

/*!
 * \file  ethfw_osal.h
 *
 * \brief Ethernet Firmware OSAL interface.
 */

#ifndef ETHFW_OSAL_H_
#define ETHFW_OSAL_H_
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ethfw_al.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup ETHFW_OSAL_ABSTRACT Ethernet Firmware OSAL Abstraction APIs
 *
 * This section contains Ethernet Firmware OSAL APIs. This is a common header
 * file for both Jacinto and Sitara which holds macros and declarations for OSAL
 * related APIs.
 * @{
 */
/*! @} */

/*!
 * \addtogroup ETHFW_OSAL_ABSTRACT
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! Timer will be started by the user */
#define ETHFWCLOCK_STARTMODE_USER               ((uint8_t) 0U)
/*! Timer starts automatically after create or scheduler start */
#define ETHFWCLOCK_STARTMODE_AUTO               ((uint8_t) 1U)
/*! Timer runs for a single period values and stops */
#define ETHFWCLOCK_RUNMODE_ONESHOT              ((uint8_t) 0U)
/*! Timer is periodic and runs continuously */
#define ETHFWCLOCK_RUNMODE_CONTINUOUS           ((uint8_t) 1U)

/*! EventP_wait will return when ANY of the bits set in mask are set in the Event bits */
#define ETHFWEVENT_WAITMODE_ANY                 ((uint8_t) 0U)
/*! EventP_wait will return when ALL the bits set in mask are set in the Event bits */
#define ETHFWEVENT_WAITMODE_ALL                 ((uint8_t) 1U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Task configuration params
 *
 * Common task config params for Jacinto and Sitara
 */
typedef struct EthFwOsal_TaskParams_s
{
    /*! Name of the task instance */
    const char *name;
    /*! The priority of the task */
    int8_t priority;
    /*! pointer to stack memory, shall be non-null value */
    void *stack;
    /*! The stack size of the task */
    uint32_t stacksize;
    /*! argument 0 */
    void *arg0;
    /*! [SafeRTOS, FreeRTOS only] Pointer to user-defined data */
    void *userData;
} EthFwOsal_TaskParams;

/*!
 * \brief Clock configuration params
 *
 * common clock config params for Jacinto and Sitara
 */
typedef struct EthFwOsal_ClockParams_s
{
    /*! Timer Start Mode */
    uint8_t  startMode;
    /*! The clock period in units of clock ticks */
    uint32_t period;
    /*! Timer Run Mode */
    uint8_t  runMode;
    /*! User argument that is available inside the callback */
    void *arg;
} EthFwOsal_ClockParams;

/*!
 * \brief Event configuration params
 *
 * Event config params for Jacinto, don't care for Sitara
 */
typedef struct EthFwOsal_EventParams_s
{
    /*! Name of the task instance.*/
    void *instance;
} EthFwOsal_EventParams;

/*!
 * \brief Mailbox params for Jacinto and Sitara
 */
typedef MailboxP_Params EthFwOsal_MailboxParams;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/*!
 * \brief ETHFW OSAL Task Handle
 */
typedef void *EthFwOsal_TaskHandle;

/*!
 * \brief ETHFW OSAL Semaphore Handle
 */
typedef void *EthFwOsal_SemHandle;

/*!
 * \brief ETHFW OSAL Clock Handle
 */
typedef void *EthFwOsal_ClockHandle;

/*!
 * \brief ETHFW OSAL Event Handle
 */
typedef void *EthFwOsal_EventHandle;

/*!
 * \brief ETHFW OSAL Mailbox Handle
 */
typedef void *EthFwOsal_MailboxHandle;

/*!
 * \brief ETHFW OSAL Mutex Handle
 */
#if (MCU_PLUS_SDK)
typedef SemaphoreP_enetOsal *EthFwOsal_MutexHandle;
#else
typedef EnetOsalDflt_Mutex *EthFwOsal_MutexHandle;
#endif

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief ETHFW OSAL init
 * 
 * Based on the caller of the function (Jacinto or Sitara), this function initiates
 * the OSAL module in ETHFW. For Jacinto, this is a empty function and does nothing,
 * but for Sitara, this inits and sets the global OSAL pools to zero, which is needed
 * to support static memory allocation of the OS modules.
 */
void EthFwOsal_init(void);

/* -----------------------TaskP APIs---------------------------------------*/

/*!
 * \brief ETHFW OSAL Task init configuration parameters.
 *
 * Sets the Task init configuration parameters. This function sets to default init
 * params for Task init.
 *
 * \param params Init configuration params.
 * 
 */
void EthFwOsal_initTaskParams(EthFwOsal_TaskParams *params);

/*!
 * \brief ETHFW OSAL function to create a task.
 *
 * \param func Function pointer of the task.
 * 
 * \param params Pointer to the instance configuration parameters.
 * 
 * \return \ref EthFwOsal_TaskHandle on success or a NULL on an error
 * 
 */
EthFwOsal_TaskHandle EthFwOsal_createTask(void (*func)(void*), EthFwOsal_TaskParams *params);

/*!
 * \brief ETHFW OSAL function to delete a task.
 *
 * \param taskHandle \ref EthFwOsal_TaskHandle returned from \ref EthFwOsal_createTask
 * 
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_deleteTask(EthFwOsal_TaskHandle *taskHandle);

/*!
 * \brief ETHFW OSAL function to exit from a task.
 *
 * This function does nothing when the caller is from Jacinto baseline, but the same
 * exits at the end of the task for Sitara.
 */
void EthFwOsal_exitTask(void);

/*!
 * \brief ETHFW OSAL function to check if a given task is terminated
 *
 * This function is not supported for Sitara baseline. For a Jacinto caller this function
 * returns the current status of a task if terminated.
 * 
 * \param taskHandle \ref EthFwOsal_TaskHandle returned from \ref EthFwOsal_createTask
 * 
 * \return 0 if task is not terminated 1 if task is terminated
 */
uint32_t EthFwOsal_isTaskTerminated(EthFwOsal_TaskHandle taskHandle);

/*!
 * \brief ETHFW OSAL function returns the Task handle of current task
 *
 * This function is not supported for Sitara baseline. For a Jacinto caller this function
 * returns the Task handle of current task.
 * 
 * \return \ref EthFwOsal_TaskHandle of current task
 */
EthFwOsal_TaskHandle EthFwOsal_getTaskSelf(void);

/*!
 * \brief ETHFW OSAL function to update Task priority
 *
 * This function is not supported for Sitara baseline. For a Jacinto caller this function
 * to update Task priority.
 * 
 * \param taskHandle \ref EthFwOsal_TaskHandle of task
 * \param priority New priority to be set
 */
void EthFwOsal_setTaskPrio(EthFwOsal_TaskHandle taskHandle, uint32_t priority);

/*!
 * \brief ETHFW OSAL function Task sleep in units of msecs
 */
void EthFwOsal_sleepTaskinMsecs(uint32_t timeoutInMsecs);

/*!
 * \brief ETHFW OSAL function to get time in micro seconds
 */
uint64_t EthFwOsal_getTimeInUsecs(void);

/*!
 * \brief ETHFW OSAL function Task sleep in units of OS tick
 */
void EthFwOsal_sleepTask(uint32_t timeout);

/*!
 * \brief ETHFW OSAL function to Yield processor to equal priority task
 */
void EthFwOsal_yieldTask(void);

/* -----------------------CacheP APIs---------------------------------------*/

/*!
 * \brief ETHFW OSAL function to write back cache lines
 * 
 * \param addr Start address of the cache line/s
 * \param size size (in bytes) of the memory to be written back
 */
void EthFwOsal_wbCache(const void *addr, uint32_t size);

/*!
 * \brief ETHFW OSAL function to invalidate cache lines
 * 
 * \param addr Start address of the cache line/s
 * \param size size (in bytes) of the memory to be written back
 */
void EthFwOsal_invCache(const void *addr, uint32_t size);

/*!
 * \brief ETHFW OSAL function to write back and invalidate cache lines
 * 
 * \param addr Start address of the cache line/s
 * \param size size (in bytes) of the memory to be written back
 */
void EthFwOsal_wbInvCache(const void *addr, uint32_t size);

/* -----------------------SemaphoreP APIs------------------------------------*/

/*!
 * \brief ETHFW OSAL function to create a semaphore.
 *
 * \param count Initial count of the semaphore. For binary semaphores,
 *              only values of 0 or 1 are valid.
 * 
 * \return \ref EthFwOsal_SemHandle on success or a NULL on an error
 * 
 */
EthFwOsal_SemHandle EthFwOsal_createSemaphore(uint32_t count);

/*!
 * \brief ETHFW OSAL function to delete a semaphore.
 *
 * \param handle \ref EthFwOsal_SemHandle returned from \ref EthFwOsal_createSemaphore
 * 
 * \return Status of the function
 */
int32_t EthFwOsal_deleteSemaphore(EthFwOsal_SemHandle handle);

/*!
 * \brief ETHFW OSAL function to pend a semaphore.
 *
 * \param handle \ref EthFwOsal_SemHandle returned from \ref EthFwOsal_createSemaphore
 * \param timeout Timeout (in ticks) to wait for the semaphore to be signalled (posted)
 * 
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_pendSemaphore(EthFwOsal_SemHandle handle, uint32_t timeout);

/*!
 * \brief ETHFW OSAL function to post a semaphore.
 *
 * \param handle \ref EthFwOsal_SemHandle returned from \ref EthFwOsal_createSemaphore
 * 
 * \return Status of the function
 */
int32_t EthFwOsal_postSemaphore(EthFwOsal_SemHandle handle);

/* -----------------------MutexP APIs------------------------------------*/

/*!
 * \brief ETHFW OSAL function to create a mutex.
 *
 * \return \ref EthFwOsal_MutexHandle on success or a NULL on an error
 * 
 */
EthFwOsal_MutexHandle EthFwOsal_createMutex(void);

/*!
 * \brief ETHFW OSAL function to post a mutex.
 *
 * \param hMutex \ref EthFwOsal_MutexHandle returned from \ref EthFwOsal_createMutex
 * 
 */
void EthFwOsal_deleteMutex(EthFwOsal_MutexHandle hMutex);

/*!
 * \brief ETHFW OSAL function to lock a muex.
 *
 * \param hMutex \ref EthFwOsal_MutexHandle returned from \ref EthFwOsal_createMutex
 * 
 */
void EthFwOsal_lockMutex(EthFwOsal_MutexHandle hMutex);

/*!
 * \brief ETHFW OSAL function to unlock a mutex.
 *
 * \param hMutex \ref EthFwOsal_MutexHandle returned from \ref EthFwOsal_createMutex
 * 
 */
void EthFwOsal_unlockMutex(EthFwOsal_MutexHandle hMutex);

/* -----------------------ClockP APIs------------------------------------*/

/*!
 * \brief ETHFW OSAL Clock init configuration parameters.
 *
 * Sets the Clock init configuration parameters. This function sets to default init
 * params for Clock init.
 *
 * \param params Init configuration params.
 * 
 */
void EthFwOsal_initClockParams(EthFwOsal_ClockParams *params);

/*!
 * \brief ETHFW OSAL function to create a clock.
 *
 * \param func Function pointer of the clock function.
 * 
 * \param params Pointer to the instance configuration parameters.
 * 
 * \return \ref EthFwOsal_ClockHandle on success or a NULL on an error
 * 
 */
EthFwOsal_ClockHandle EthFwOsal_createClock(void (*func)(void*), EthFwOsal_ClockParams *params);

/*!
 * \brief ETHFW OSAL function to stack a clock.
 *
 * \param handle \ref EthFwOsal_ClockHandle returned from \ref EthFwOsal_createClock
 * 
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_startClock(EthFwOsal_ClockHandle handle);

/*!
 * \brief ETHFW OSAL function to stop a clock.
 *
 * \param handle \ref EthFwOsal_ClockHandle returned from \ref EthFwOsal_createClock
 * 
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_stopClock(EthFwOsal_ClockHandle handle);

/*!
 * \brief ETHFW OSAL function to delete a clock.
 *
 * \param handle \ref EthFwOsal_ClockHandle returned from \ref EthFwOsal_createClock
 * 
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_deleteClock(EthFwOsal_ClockHandle handle);

/* -----------------------EventP APIs------------------------------------*/

/*!
 * \brief ETHFW OSAL Event init configuration parameters.
 *
 * Sets the Event init configuration parameters. This function sets to default init
 * params for Event init.
 *
 * \param params Init configuration params.
 * 
 */
void EthFwOsal_initEventParams(EthFwOsal_EventParams *params);

/*!
 * \brief ETHFW OSAL function to create a event.
 *
 * \param params Pointer to the instance configuration parameters.
 * 
 * \return \ref EthFwOsal_EventHandle on success or a NULL on an error
 * 
 */
EthFwOsal_EventHandle EthFwOsal_createEvent(EthFwOsal_EventParams *params);

/*!
 * \brief ETHFW OSAL function to wait for an event.
 *
 * \param handle \ref EthFwOsal_EventHandle returned during \ref EthFwOsal_createEvent
 * \param eventMask mask of eventIds to pend on (must be non-zero)
 *                  Only supports upto 24 bits.
 * \param waitMode Event wait mode.
 * \param timeout return from wait() after this many system time units
 * 
 * \return All consumed events or zero if timeout
 * 
 */
uint32_t EthFwOsal_waitEvent(EthFwOsal_EventHandle handle,
                            uint32_t eventMask,
                            uint8_t waitMode,
                            uint32_t timeout);

/*!
 * \brief ETHFW OSAL function to post an event.
 *
 * \param handle \ref EthFwOsal_EventHandle returned during \ref EthFwOsal_createEvent
 * \param eventBits mask of eventIds to post (must be non-zero)
 *
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_postEvent(EthFwOsal_EventHandle handle, uint32_t eventBits);

/* ----------------------MailboxP APIs------------------------------------*/

/*!
 * \brief ETHFW OSAL Mailbox init configuration parameters.
 *
 * Sets the Mailbox init configuration parameters. This function sets to default init
 * params for Mailbox init.
 *
 * \param params Init configuration params.
 * 
 */
void EthFwOsal_initMailboxParams(EthFwOsal_MailboxParams *params);

/*!
 * \brief ETHFW OSAL function to create a Mailbox.
 *
 * \param params Pointer to the instance configuration parameters.
 * 
 * \return \ref EthFwOsal_MailboxHandle on success or a NULL on an error
 * 
 */
EthFwOsal_MailboxHandle EthFwOsal_createMailbox(EthFwOsal_MailboxParams *params);

/*!
 * \brief ETHFW OSAL function to delete a Mailbox.
 *
 * \param handle \ref EthFwOsal_MailboxHandle returned from \ref EthFwOsal_createMailbox
 * 
 * \return Status of the function
 * 
 */
int32_t EthFwOsal_deleteMailbox(EthFwOsal_MailboxHandle handle);

/*!
 * \brief ETHFW OSAL function to post a Mailbox.
 *
 * \param handle \ref EthFwOsal_MailboxHandle returned from \ref EthFwOsal_createMailbox
 * \param msg Pointer to the message to post
 * \param timeout Timeout (in milliseconds) to wait for post a
 *                message to the mailbox.
 * 
 * \return Status of the function
 */
int32_t EthFwOsal_postMailbox(EthFwOsal_MailboxHandle handle,
                              void *msg,
                              uint32_t timeout);

/*!
 * \brief ETHFW OSAL function to pend a Mailbox.
 *
 * \param handle \ref EthFwOsal_MailboxHandle returned from \ref EthFwOsal_createMailbox
 * \param msg Pointer to the message to pend
 * \param timeout Timeout (in milliseconds) to wait for pend a
 *                message to the mailbox.
 * 
 * \return Status of the function
 */
int32_t EthFwOsal_pendMailbox(EthFwOsal_MailboxHandle handle,
                              void *msg,
                              uint32_t timeout);


#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_OSAL_H_ */
