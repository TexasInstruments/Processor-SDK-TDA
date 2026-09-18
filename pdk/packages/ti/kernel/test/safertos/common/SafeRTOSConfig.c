/*
 *  Copyright ( C ) 2022 Texas Instruments Incorporated
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
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Generic headers */
#include <stdlib.h>

/* SafeRTOS includes */
#include "SafeRTOS_API.h"

/* Program Includes */
#include "aborts.h"     /* Required for Hook Function Prototypes */
#include "safertos_config.h"

#include <string.h>
#include <ti/osal/TimerP.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/osal/src/nonos/Nonos_config.h>

/*-----------------------------------------------------------------------------
 * Local Constants
 *---------------------------------------------------------------------------*/

/* Scheduler Initialisation Definitions */

/* SafeRTOS inspects the vector table to ensure
 * the necessary handlers have been installed. */
#define configSTACK_CHECK_MARGIN            ( 0U )

/* The user configuration for the timer module. */
/* SafeRTOS package defines this. We want to use the OSAL definition in PDK */
#undef configTIMER_TASK_PRIORITY
#define configTIMER_TASK_PRIORITY           ( configMAX_PRIORITIES - 1U )
#define configTIMER_CMD_QUEUE_LEN           ( 50U )
#define configTIMER_CMD_QUEUE_BUFFER_SIZE   ( ( configTIMER_CMD_QUEUE_LEN * sizeof( timerQueueMessageType ) ) + safertosapiQUEUE_OVERHEAD_BYTES )

#if defined (BUILD_MCU)
#define configTIMER_TASK_STACK_SIZE         ( configTIMER_STACK_SIZE_WITH_FPU )
#define configIDLE_TASK_STACK_SIZE          ( configMINIMAL_STACK_SIZE_WITH_FPU )
#else
#define configTIMER_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE )
#define configIDLE_TASK_STACK_SIZE          ( configMINIMAL_STACK_SIZE )
#endif

#if defined (BUILD_C7X)
/* Yeild Interrupt Number */
#define configC7X_YEILD_INT_NUM             ( 13U )
#endif

/*-----------------------------------------------------------------------------
 * Local Prototypes
 *---------------------------------------------------------------------------*/

/* The call to xTaskInitializeScheduler is included within a wrapper
 * initialisation function. */
portBaseType xInitializeScheduler( void );

void vApplicationFiqHandlerHook( void );

/*-----------------------------------------------------------------------------
 * Local Variables
 *---------------------------------------------------------------------------*/
/* For SafeRTOS on R5F with FFI Support, task stack should be aligned to the stack size */
#if defined (BUILD_MCU)
static portInt8Type acIdleTaskStack[ configIDLE_TASK_STACK_SIZE ] __attribute__( ( aligned ( configIDLE_TASK_STACK_SIZE ) ) ) = { 0 };

/* Declare the stack for the timer task, it cannot be done in the timer
 * module as the syntax for alignment is port specific. Also the callback
 * functions are executed in the timer task and their complexity/stack
 * requirements are application specific. */
static portInt8Type acTimerTaskStack[ configTIMER_TASK_STACK_SIZE ] __attribute__( ( aligned ( configTIMER_TASK_STACK_SIZE ) ) ) = { 0 };
#else
static portInt8Type acTimerTaskStack[ configTIMER_TASK_STACK_SIZE ] __attribute__( ( aligned ( safertosapiSTACK_ALIGNMENT ) ) )  = { 0 };
static portInt8Type acIdleTaskStack[ configIDLE_TASK_STACK_SIZE ] __attribute__( ( aligned ( safertosapiSTACK_ALIGNMENT ) ) )  = { 0 };
#endif /* defined (BUILD_MCU) */

/* The buffer for the timer command queue. */
static portInt8Type acTimerCommandQueueBuffer[ configTIMER_CMD_QUEUE_BUFFER_SIZE ] __attribute__( ( aligned ( safertosapiWORD_ALIGNMENT ) ) ) = { 0 };

#if defined (BUILD_MCU) || defined (BUILD_C7X)
/* A linker defined symbol that gives the start address of the Idle task
 * data section. */
extern portUInt32Type lnkIdleHookDataStartAddr;
#endif

/*-----------------------------------------------------------------------------
 * Routines
 *---------------------------------------------------------------------------*/

portBaseType xInitializeScheduler( void )
{
    portBaseType xInitSchedResult;

    /* The structure passed to xTaskInitializeScheduler() to configure the kernel
     * with the application defined constants and call back functions. */
    const xPORT_INIT_PARAMETERS xPortInit =
    {
        configSYSTICK_CLOCK_HZ,             /* ulTimerClockHz */
        configTICK_RATE_HZ,                 /* ulTickRateHz */
#if defined (BUILD_C7X)
    	configC7X_YEILD_INT_NUM,            /* uxYieldInterruptNumber */
#endif

        /* Hook Functions */
        NULL,                               /* pvSvcHookFunction */
        
        /* System Stack parameters */
        configSTACK_CHECK_MARGIN,           /* uxAdditionalStackCheckMarginBytes */

        /* Idle Task parameters */
        acIdleTaskStack,                    /* pcIdleTaskStackBuffer */
        configIDLE_TASK_STACK_SIZE,         /* uxIdleTaskStackSizeBytes */
#if defined (BUILD_MCU)
        pdFALSE,                            /* xIdleTaskUsingFPU */

        /* xIdleTaskMPUParameters */
        {
            mpuUNPRIVILEGED_TASK,           /* The idle hook will be executed in unprivileged mode. */
            {
                { NULL, 0U, 0U, 0U },
                { NULL, 0U, 0U, 0U }
            }
        },
#endif
#if defined (BUILD_C66X)
    safertosapiUNPRIVILEGED_TASK,           /* The idle hook will not be executed in privileged mode. */
#endif
        NULL,                               /* pvIdleTaskTLSObject */

        /* Timer feature initialisation parameters */
        configTIMER_TASK_PRIORITY,          /* uxTimerTaskPriority */
        configTIMER_TASK_STACK_SIZE,        /* uxTimerTaskStackSize */
        acTimerTaskStack,                   /* pcTimerTaskStackBuffer */
        configTIMER_CMD_QUEUE_LEN,          /* uxTimerCommandQueueLength */
        configTIMER_CMD_QUEUE_BUFFER_SIZE,  /* uxTimerCommandQueueBufferSize */
        acTimerCommandQueueBuffer,          /* pcTimerCommandQueueBuffer */
#if defined (BUILD_MCU)
        pdTRUE                              /* xEnableCache */
#endif
    };

    /* Initialise the kernel by passing in a pointer to the xPortInit structure
     * and return the resulting error code. */
    xInitSchedResult = xTaskInitializeScheduler( &xPortInit );

    return xInitSchedResult;
}

/*---------------------------------------------------------------------------*/

void vApplicationErrorHook( portTaskHandleType xHandleOfTaskWithError,
                            portBaseType xErrorCode )
{
    /* The parameters are not used, these lines prevent compiler warnings. */
    ( void ) xHandleOfTaskWithError;
    ( void ) xErrorCode;

    /* Will only get here if an internal kernel error occurs. */
    vApplicationAbort();
}
/*---------------------------------------------------------------------------*/
#if defined (BUILD_MCU)
void vApplicationFiqHandlerHook( void )
{
    while( 1 ); /* vFiqhandler not used in SAFERTOS */
}
#endif
/*-------------------------------------------------------------------------*/
