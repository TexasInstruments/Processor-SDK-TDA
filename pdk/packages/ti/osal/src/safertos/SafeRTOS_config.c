/*
 *  Copyright (c) Texas Instruments Incorporated 2021-2022
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
 *  \file  SafeRTOS_config.c
 *
 *  \brief The file implements the safertos OS init configuration.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/osal/TimerP.h>
#include <ti/osal/src/nonos/Nonos_config.h>
#if defined(BUILD_C7X)
#include <ti/csl/arch/csl_arch.h>
#endif

#include "SafeRTOS_priv.h"
#include "SafeRTOS_config.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Scheduler Initialisation Definitions */

/* SafeRTOS inspects the vector table to ensure
 * the necessary handlers have been installed. */
#define configSTACK_CHECK_MARGIN            ( 0U )

/* The user configuration for the timer module. */
/* SafeRTOS package defines this. We want to use the OSAL definition in PDK */
#undef configTIMER_TASK_PRIORITY
#define configTIMER_TASK_PRIORITY           ( (uint32_t)configMAX_PRIORITIES - 1U )
/* SafeRTOS package defines this. We want to use the OSAL definition in PDK */
#undef configTIMER_QUEUE_LENGTH
#define configTIMER_QUEUE_LENGTH            ( 50U )
#define configTIMER_CMD_QUEUE_BUFFER_SIZE   ( ( configTIMER_QUEUE_LENGTH * sizeof( timerQueueMessageType ) ) + safertosapiQUEUE_OVERHEAD_BYTES )

#if defined (BUILD_MCU)
#define configTIMER_TASK_STACK_SIZE         ( configTIMER_STACK_SIZE_WITH_FPU )
/* The user configuration for the idle task. */
#define configIDLE_TASK_STACK_SIZE          ( configMINIMAL_STACK_SIZE_WITH_FPU )
#endif

#if defined (BUILD_C66X)
/* The user configuration for the idle task. */
#define configIDLE_TASK_STACK_SIZE          ( configMINIMAL_STACK_SIZE )
#endif

#if defined (BUILD_C7X)
#define configTIMER_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE )
/* The user configuration for the idle task. */
#define configIDLE_TASK_STACK_SIZE          ( configMINIMAL_STACK_SIZE )
#endif

/* ========================================================================== */
/*                            Global Structure Definition                     */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* Timer interrupt handler function. */
static void prvTimerTickIsr( uintptr_t arg );

static TimerP_Handle pxTickTimerHandle = NULL;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/* For SafeRTOS on R5F with FFI Support, task stack should be aligned to the stack size */
#if defined (BUILD_MCU)
static portInt8Type acIdleTaskStack[ configIDLE_TASK_STACK_SIZE ] __attribute__( ( aligned( configIDLE_TASK_STACK_SIZE ) ) ) = { 0 };

/* Declare the stack for the timer task, it cannot be done in the timer
 * module as the syntax for alignment is port specific. Also the callback
 * functions are executed in the timer task and their complexity/stack
 * requirements are application specific. */
static portInt8Type acTimerTaskStack[ configTIMER_TASK_STACK_SIZE ] __attribute__( ( aligned( configTIMER_TASK_STACK_SIZE ) ) ) = { 0 };
#else
static portInt8Type acTimerTaskStack[ configTIMER_TASK_STACK_SIZE ] __attribute__( ( aligned ( safertosapiSTACK_ALIGNMENT ) ) )   = { 0 };
static portInt8Type acIdleTaskStack[ configIDLE_TASK_STACK_SIZE ] __attribute__( ( aligned ( safertosapiSTACK_ALIGNMENT ) ) )   = { 0 };
#endif /* defined (BUILD_MCU) */

#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
/* RTS structs for all the tasks. */
static xRTS xIdleTaskRTS = { 0 };
#endif

/* The buffer for the timer command queue. */
static portInt8Type acTimerCommandQueueBuffer[ configTIMER_CMD_QUEUE_BUFFER_SIZE ]  __attribute__( ( aligned ( safertosapiWORD_ALIGNMENT ) ) ) = { 0 };

/* The structure passed to xTaskInitializeScheduler() to configure the kernel
 * with the application defined constants and call back functions. */
const xPORT_INIT_PARAMETERS gSafertosPortInit =
{
    configSYSTICK_CLOCK_HZ,             /* ulCPUClockHz */
    (uint32_t)configTICK_RATE_HZ,       /* ulTickRateHz */
#if defined (BUILD_C7X)
    OSAL_SAFERTOS_OS_YEILD_INT_NUM_C7X, /* uxYieldInterruptNumber */
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
    {                                   /* xIdleTaskMPUParameters */
        mpuPRIVILEGED_TASK,             /* The idle hook will be executed in privileged mode. */
        {
            { NULL, 0U, 0U, 0U },
            { NULL, 0U, 0U, 0U },
        }
    },
#endif
#if defined (BUILD_C66X)
    safertosapiUNPRIVILEGED_TASK,       /* The idle hook will not be executed in privileged mode. */
#endif
#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
        &xIdleTaskRTS,                      /* RTS struct passed in as pvIdleTaskTLSObject */
#else
        NULL,                               /* pvIdleTaskTLSObject */
#endif
    /* Timer feature initialisation parameters */
    configTIMER_TASK_PRIORITY,          /* uxTimerTaskPriority */
    configTIMER_TASK_STACK_SIZE,        /* uxTimerTaskStackSize */
    acTimerTaskStack,                   /* pcTimerTaskStackBuffer */
    configTIMER_QUEUE_LENGTH,           /* uxTimerCommandQueueLength */
    configTIMER_CMD_QUEUE_BUFFER_SIZE,  /* uxTimerCommandQueueBufferSize */
    acTimerCommandQueueBuffer,          /* pcTimerCommandQueueBuffer */

#if defined (BUILD_MCU)
    pdFALSE,                             /* xEnableCache */
#endif
};

uint32_t gSaftRtosInitDone = UFALSE;

/* ========================================================================== */
/*                          Function Defintions                               */
/* ========================================================================== */

void OS_init( void )
{
    portBaseType xInitSchedResult;

    xInitSchedResult = prvSetupHardware();

    if(pdPASS == xInitSchedResult)
    {
        /* Initialise the kernel by passing in a pointer to the xPORT_INIT_PARAMETERS structure
        * and return the resulting error code. */
        xInitSchedResult = xTaskInitializeScheduler( &gSafertosPortInit );
    }

    /* Assert that xInitSchedResult is successful */
    DebugP_assert(pdPASS == xInitSchedResult);

    gSaftRtosInitDone = UTRUE;

    return;
}

/*---------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationErrorHook( portTaskHandleType xHandleOfTaskWithError,
                            portBaseType xErrorCode )
{
    /* The parameters are not used, these lines prevent compiler warnings. */
    ( void ) xHandleOfTaskWithError;
    ( void ) xErrorCode;

    /* Will only get here if an internal kernel error occurs. */
    DebugP_assert(BFALSE);
}

/*---------------------------------------------------------------------------*/

/* Tick timer setup using TI PDK timers and interrupts. */
__attribute__((weak)) \
void vApplicationSetupTickInterruptHook( portUInt32Type ulTimerClockHz,
                                         portUInt32Type ulTickRateHz )
{
    TimerP_Params xTimerParams;

    Safertos_OSTimerParams xOSTimerParams;

    ( void ) ulTimerClockHz;

    TimerP_Params_init( &xTimerParams );
    xTimerParams.runMode    = TimerP_RunMode_CONTINUOUS;
    xTimerParams.startMode  = TimerP_StartMode_USER;
    xTimerParams.periodType = TimerP_PeriodType_MICROSECS;
    xTimerParams.period     = (uint32_t)( 1000000UL / ulTickRateHz );
    prvGetOSTimerParams( &xOSTimerParams );
#if defined (BUILD_C66X) || defined (BUILD_C7X)
    xTimerParams.intNum     = xOSTimerParams.intNum;
    xTimerParams.eventId    = xOSTimerParams.eventId;
#endif

    pxTickTimerHandle = TimerP_create( xOSTimerParams.timerId,
                                       &prvTimerTickIsr,
                                       &xTimerParams );

    /* don't expect the handle to be null or the timer to start OK */
    if( ( NULL == pxTickTimerHandle ) ||
        ( TimerP_OK != TimerP_start( pxTickTimerHandle ) ) )
    {
            DebugP_assert(BFALSE);
    }

#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
    /*Initialise Run Time Stats */
    vInitialiseRunTimeStatistics();
#endif

}

/*-------------------------------------------------------------------------*/

void prvGetOSTimerParams( Safertos_OSTimerParams *params)
{
#if defined (BUILD_C66X)
    if(0U == CSL_chipReadDNUM())
    {   
        params->timerId = OSAL_SAFERTOS_OS_TIMER_ID_C66X_1;
        params->eventId = OSAL_SAFERTOS_OS_TIMER_EVENT_ID_C66X_1;
        params->intNum  = OSAL_SAFERTOS_OS_TIMER_INT_NUM_C66X_1;
    }
    else
    {
        params->timerId = OSAL_SAFERTOS_OS_TIMER_ID_C66X_2;
        params->eventId = OSAL_SAFERTOS_OS_TIMER_EVENT_ID_C66X_2;
        params->intNum  = OSAL_SAFERTOS_OS_TIMER_INT_NUM_C66X_2;
    }
#endif
#if defined (BUILD_MCU)
    CSL_ArmR5CPUInfo info = {0};

    CSL_armR5GetCpuID(&info);
    if(CSL_ARM_R5_CLUSTER_GROUP_ID_0 == info.grpId)
    {
        params->timerId = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                                    OSAL_SAFERTOS_OS_TIMER_ID_MCU1_0:
                                        OSAL_SAFERTOS_OS_TIMER_ID_MCU1_1;
    }
    else if(CSL_ARM_R5_CLUSTER_GROUP_ID_1 == info.grpId)
    {     
        params->timerId = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                                    OSAL_SAFERTOS_OS_TIMER_ID_MCU2_0:
                                        OSAL_SAFERTOS_OS_TIMER_ID_MCU2_1;
    }
#if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4)
    else if(CSL_ARM_R5_CLUSTER_GROUP_ID_2 == info.grpId)
    {    
        params->timerId = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                                    OSAL_SAFERTOS_OS_TIMER_ID_MCU3_0:
                                        OSAL_SAFERTOS_OS_TIMER_ID_MCU3_1;
    }
#endif
#if defined (SOC_J784S4)
    else if(CSL_ARM_R5_CLUSTER_GROUP_ID_3 == info.grpId)
    {    
        params->timerId = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                                    OSAL_SAFERTOS_OS_TIMER_ID_MCU4_0:
                                        OSAL_SAFERTOS_OS_TIMER_ID_MCU4_1;
    } 
#endif
    else
    {
          /*   Do nothing  */
    }
#elif defined (BUILD_C7X)
    uint32_t rtMapCpuId;
    rtMapCpuId = CSL_clecGetC7xRtmapCpuId();
    if(CSL_CLEC_RTMAP_CPU_4 == rtMapCpuId)
    {
        params->timerId = OSAL_SAFERTOS_OS_TIMER_ID_C7X_1;
        params->intNum  = OSAL_SAFERTOS_OS_TIMER_INT_NUM_C7X_1;
        params->eventId = TimerP_USE_DEFAULT;
    }
#if defined (SOC_J721S2) || defined (SOC_J784S4)
    else if(CSL_CLEC_RTMAP_CPU_5 == rtMapCpuId)
    {
        params->timerId = OSAL_SAFERTOS_OS_TIMER_ID_C7X_2;
        params->intNum  = OSAL_SAFERTOS_OS_TIMER_INT_NUM_C7X_2;
        params->eventId = TimerP_USE_DEFAULT;
    }
#endif
#if defined (SOC_J784S4)
    else if(CSL_CLEC_RTMAP_CPU_6 == rtMapCpuId)
    {
        params->timerId = OSAL_SAFERTOS_OS_TIMER_ID_C7X_3;
        params->intNum  = OSAL_SAFERTOS_OS_TIMER_INT_NUM_C7X_3;
        params->eventId = TimerP_USE_DEFAULT;
    }
    else if(CSL_CLEC_RTMAP_CPU_7 == rtMapCpuId)
    {
        params->timerId = OSAL_SAFERTOS_OS_TIMER_ID_C7X_4;
        params->intNum  = OSAL_SAFERTOS_OS_TIMER_INT_NUM_C7X_4;
        params->eventId = TimerP_USE_DEFAULT;
    }
#endif
    else
    {
      /* Do nothing */
    }
#endif
}

TimerP_Handle TimerP_getTickTimerHandle( void )
{
    return pxTickTimerHandle;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static void prvTimerTickIsr( uintptr_t arg )
{
    ( void ) arg;
    vTaskProcessSystemTickFromISR();
}

/*-------------------------------------------------------------------------*/
