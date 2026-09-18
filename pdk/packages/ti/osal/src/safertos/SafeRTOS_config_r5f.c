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

/**
 *  \file   SafeRTOS_config_r5f.c
 *
 *  \brief The file implements the safertos configs specific for R5F.
 *         This includes various startup routines (for MPU configuration, 
 *         Set DLFO bit, clear all interrupts, etc..) and other hook functions.
 * 
 **/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* SafeRTOS Includes */
#include "SafeRTOS_API.h"
#include "SafeRTOS_priv.h"
#include "rtsPort.h"
#include "task.h"
#include "mpuARM.h"

/* PDK Includes */
#include <ti/osal/src/nonos/Nonos_config.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/csl/arch/csl_arch.h>
#include <TimerP.h>
#include <ti/csl/arch/r5/csl_arm_r5_pmu.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define UNUSED(x)   (x = x)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* Disable I/D caches */
extern uint32_t CSL_armR5StartupCacheEnableAllCache( uint32_t enable );
/* Disable force write-thru */
extern uint32_t CSL_armR5StartupCacheEnableForceWrThru( uint32_t enable );
/* Invalidate I/D caches */
extern uint32_t CSL_armR5StartupCacheInvalidateAllCache( void );
extern __attribute__((section(".startupCode"))) portBaseType xConfigureMPU(void);
void vFiqHandler( void );
__attribute__((section(".startupCode"))) void __mpu_init( void );
__attribute__((section(".startupCode"))) extern void Osal_disableMPU( void );

/*---------------------------------------------------------------------------*/

/* Hook function handlers targeting the TI PDK libraries. */

extern Osal_ISRHooks gOsalISRHooks;

/* Timer interrupt handler function. */
void vApplicationInterruptHandlerHook( void );

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static portTaskHandleType xIdleTaskHandle = NULL;

static portUInt32Type ulRtsLastTickCount = 0U;

static portBaseType xIdleTaskFirstExecution = pdTRUE;

/* TimerP Global Variable Imports to be updated every tick interrupt */
extern uint64_t gSafertosNumTicks;
extern uint32_t gSafertosPmuLastTickCycleCount;

/*-----------------------------------------------------------------------------
 * TI PDK variables required by the interrupt handler.
 *---------------------------------------------------------------------------*/

extern uintptr_t            gVimBaseAddr;
extern uint8_t              intrMap[ R5_VIM_INTR_NUM ];
extern void                 *argArray[ R5_VIM_INTR_NUM ];
extern IntrFuncPtr          fxnArray[ R5_VIM_INTR_NUM ];
extern uint32_t             intrSrcType[ R5_VIM_INTR_NUM ];

/* ========================================================================== */
/*                          Function Defintions                               */
/* ========================================================================== */

/*-----------------------------------------------------------------------------
 * Public function definitions.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Override functions for CSL startup code.
 *---------------------------------------------------------------------------*/

/*****************************************************************************/
/* \brief __MPU_INIT() - __mpu_init() is called in the C/C++ startup routine,*/
/* _c_int00(), and provides a mechanism for tailoring mpu init by device     */
/* prior to calling main().                                                  */
/*                                                                           */
/*****************************************************************************/
/*  Usage notes:
    On entry to this function from boot, R5F must be in System (privileged) mode. */

__attribute__((weak)) \
__attribute__((section(".startupCode"))) void __mpu_init( void )
{
    portBaseType xInitMpuResult;

    /* Disable the MPU region, before configuring it. */
    Osal_disableMPU();

    /* Override the startup code mpu initialisation. */

    /* Region 0 configuration: whole 4 GB */
    xInitMpuResult = xMPUConfigureWholeMemory( 0U,
                                                   mpuREGION_PRIVILEGED_READ_WRITE_USER_READ_WRITE,
                                                   0U );


    if( pdPASS == xInitMpuResult )
    {
        xInitMpuResult = xConfigureMPU();
    }

    /* Configure the regions in the MPU that are common to all tasks. */
    vMPUSetupMPU();
    /* Disable I/D caches */
    CSL_armR5StartupCacheEnableAllCache( CSL_CACHE_DISABLE );
    /* Disable force write-thru */
    CSL_armR5StartupCacheEnableForceWrThru( CSL_CACHE_DISABLE );
    /* Invalidate I/D caches */
    CSL_armR5StartupCacheInvalidateAllCache();
    /* Enable I/D caches */
    CSL_armR5StartupCacheEnableAllCache( CSL_CACHE_ENABLE );
}
/*-------------------------------------------------------------------------*/

/*****************************************************************************/
/* _SYSTEM_PRE_INIT() - _system_pre_init() is called in the C/C++ startup    */
/* routine (_c_int00()) and provides a mechanism for the user to             */
/* insert application specific low level initialization instructions prior   */
/* to calling main().  The return value of _system_pre_init() is used to     */
/* determine whether or not C/C++ global data initialization will be         */
/* performed (return value of 0 to bypass C/C++ auto-initialization).        */
/*                                                                           */
/* PLEASE NOTE THAT BYPASSING THE C/C++ AUTO-INITIALIZATION ROUTINE MAY      */
/* RESULT IN PROGRAM FAILURE.                                                */
/*                                                                           */
/* The version of _system_pre_init() below is skeletal and is provided to    */
/* illustrate the interface and provide default behavior.  To replace this   */
/* version rewrite the routine and include it as part of the current project.*/
/* The linker will include the updated version if it is linked in prior to   */
/* linking with the C/C++ runtime library.                                   */
/*****************************************************************************/
__attribute__((weak)) \
__attribute__((section(".startupCode")))  portBaseType _system_pre_init( void )
{
    extended_system_pre_init();
    return 1;
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/* _SYSTEM_POST_CINIT() - _system_post_cinit() is a hook function called in  */
/* the C/C++ auto-initialization function after cinit() and before pinit().  */
/*                                                                           */
/* The version of _system_post_cinit() below is skeletal and is provided to  */
/* illustrate the interface and provide default behavior.  To replace this   */
/* version rewrite the routine and include it as part of the current project.*/
/* The linker will include the updated version if it is linked in prior to   */
/* linking with the C/C++ runtime library.                                   */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* __TI_default_system_post_cinit indicates that the default                 */
__attribute__((weak)) \
__attribute__((section(".startupCode")))  void _system_post_cinit( void )
{
    osalArch_Config_t cfg;

    cfg.disableIrqOnInit = BTRUE;
    osalArch_Init(&cfg);

    uint32_t            loopCnt = 0U, regAddr;
    CSL_ArmR5CPUInfo    info = {0};
    uint32_t            maxIntrs;
    CSL_vimRegs         *pRegs;

    CSL_armR5GetCpuID(&info);

#if defined(CSL_MAIN_DOMAIN_VIM_BASE_ADDR0) && defined(CSL_MAIN_DOMAIN_VIM_BASE_ADDR1)
    if(CSL_ARM_R5_CLUSTER_GROUP_ID_0 == info.grpId)
    {
        /* MCU SS Pulsar R5 SS */
        regAddr = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                                 CSL_MCU_DOMAIN_VIM_BASE_ADDR0:
                                 CSL_MCU_DOMAIN_VIM_BASE_ADDR1;

    }
    else
    {
        /* MAIN SS Pulsar R5 SS */
        regAddr = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                                 CSL_MAIN_DOMAIN_VIM_BASE_ADDR0:
                                 CSL_MAIN_DOMAIN_VIM_BASE_ADDR1;

    }
#else
    /* MCU SS Pulsar R5 SS */
    regAddr = (CSL_ARM_R5_CPU_ID_0 == info.cpuID)?
                             CSL_MCU_DOMAIN_VIM_BASE_ADDR0:
                             CSL_MCU_DOMAIN_VIM_BASE_ADDR1;
#endif
    pRegs       = (CSL_vimRegs *)(uintptr_t) regAddr;
    maxIntrs    = pRegs->INFO;

#if defined (SOC_AM65XX) || defined (SOC_J721E)
    /* Limit the outstanding transactions to 2
     * only for AM65xx and J721e platforms
     * Later SoCs do not have this issue
     */

    /* Set DLFO bit is defined in SafeRTOS package portasm.S. */
    extern void vPortSetDLFOBit( void );

    vPortSetDLFOBit();
#else
    /* Fix for PRSDK-8161
     * For AM64x and J7200, there is no issue of limitting outstanding
     * transactions. R5F core can support full 7 outstanding transactions
     */
#endif

    /* Disable/Clear pending Interrupts in VIM before enabling CPU Interrupts */
    /* This is done to prevent serving any bogus interrupt */
    for(loopCnt = 0U ; loopCnt < maxIntrs; loopCnt++)
    {
        /* Disable interrupt in vim */
        CSL_vimSetIntrEnable((CSL_vimRegs *)(uintptr_t)regAddr,
                                    loopCnt,
                                    BFALSE);
        /* Clear interrupt status */
        CSL_vimClrIntrPending((CSL_vimRegs *)(uintptr_t)regAddr,
                                     loopCnt);
    }
    extended_system_post_cinit();
}

/*-----------------------------------------------------------------------------
 * Copy of the TI master ISR handler.
 * This function is not compiled as an interrupt handler, so won't change stack
 * or enable interrupts on exit.
 *---------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationInterruptHandlerHook( void )
{
    IntrFuncPtr        fxnPtr;
    volatile uint32_t  intNum;
    volatile uint32_t  dummy;
    int32_t FIQ_stat, IRQ_stat;

    /* Read to force prioritization logic to take effect */
    dummy = CSL_vimGetIrqVectorAddress( (CSL_vimRegs *)gVimBaseAddr);

    UNUSED(dummy);

    /* Process a pending FIQ interrupt before a pending IRQ interrupt */
    FIQ_stat = CSL_vimGetActivePendingIntr( (CSL_vimRegs *)gVimBaseAddr, CSL_VIM_INTR_MAP_FIQ, (uint32_t *)&intNum, (uint32_t *)0 );
    IRQ_stat = CSL_vimGetActivePendingIntr( (CSL_vimRegs *)gVimBaseAddr, CSL_VIM_INTR_MAP_IRQ, (uint32_t *)&intNum, (uint32_t *)0 );
    if( ( 0 == FIQ_stat ) || ( 0 == IRQ_stat ) )
    {
        /* Call registered pre-ISR hook */
        if ((IsrHookPtr)NULL != gOsalISRHooks.preISRHook)
        {
            gOsalISRHooks.preISRHook(gOsalISRHooks.preISRHookArgs);
        }

        /* Clear pulse-type interrupt before calling ISR */
        if( CSL_VIM_INTR_TYPE_PULSE == intrSrcType[intNum] )
        {
            CSL_vimClrIntrPending( (CSL_vimRegs *)gVimBaseAddr, intNum );
        }
        /* Call ISR */
        if( NULL != fxnArray[intNum] )
        {
            fxnPtr = fxnArray[intNum];
            fxnPtr(argArray[intNum]);
        }
        /* Clear level-type interrupt after calling ISR */
        if( CSL_VIM_INTR_TYPE_LEVEL == intrSrcType[intNum] )
        {
            CSL_vimClrIntrPending( (CSL_vimRegs *)gVimBaseAddr, intNum );
        }
        /* Acknowledge interrupt servicing */
        CSL_vimAckIntr( (CSL_vimRegs *)gVimBaseAddr, (CSL_VimIntrMap)intrMap[intNum] );
    
        /* Call registered post-ISR hook */
        if ((IsrHookPtr)NULL != gOsalISRHooks.postISRHook)
        {
            gOsalISRHooks.postISRHook(gOsalISRHooks.postISRHookArgs);
        }
    }
}

/*-------------------------------------------------------------------------*/

/* Hardware setup using the TI PDK libraries. */
__attribute__((weak)) \
portBaseType prvSetupHardware( void )
{
    portBaseType xStatus = pdPASS;
#if defined (SOC_J784S4)
    int32_t sciclientRet = CSL_PASS;
#endif
#if defined (SOC_J784S4)
    CSL_ArmR5CPUInfo info = {0};
    CSL_armR5GetCpuID(&info);

    if(CSL_ARM_R5_CLUSTER_GROUP_ID_0 != info.grpId)
    {
        Sciclient_ConfigPrms_t config;

        Sciclient_configPrmsInit(&config);
        sciclientRet = Sciclient_init(&config);
        if( CSL_PASS == sciclientRet )
        {
            uint32_t currState, resetState, contextLossState;
            /* on J7AHP, Main domain timers 8-19 are connected to LPSC_PER_SPARE_0 which is not powered ON by default.
             * All other timers are connected to LPSC which is ALWAYS_ON.
             * For J7AHP, MCU4_0 and MCU4_1 use DMTimer 8 and DMTimer 9 as tick timers.
             */
            sciclientRet = Sciclient_pmGetModuleState(TISCI_DEV_TIMER8, &currState, &resetState,
                                                &contextLossState, SCICLIENT_SERVICE_WAIT_FOREVER);
            if((CSL_PASS == sciclientRet) && ((uint32_t)TISCI_MSG_VALUE_DEVICE_SW_STATE_ON != currState))
            {
                sciclientRet = Sciclient_pmSetModuleState(TISCI_DEV_TIMER8, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                                    (TISCI_MSG_FLAG_AOP |TISCI_MSG_FLAG_DEVICE_RESET_ISO),
                                                    SCICLIENT_SERVICE_WAIT_FOREVER);
            }
        }
    }
    if( CSL_PASS == sciclientRet )
    {
	     xStatus = pdPASS;
    }
    else
    {
	     xStatus = pdFAIL;
    }
#endif

    return xStatus;
}

/*-------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationFiqHandlerHook( void )
{
    /* FIQ is not supported with SafeRTOS.
     *
     * Force an assert(), then stop here so application writers can catch the error. */
    DebugP_assert(BFALSE);
}

/*-------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationTickHook ( void )
{
#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
    vUpdateRTSFromTick();
#endif
    /* Ensure to add the logic in redefinition of vApplicationTickHook in order to 
     * sync usec level core timing across WFI calls */
    gSafertosNumTicks = gSafertosNumTicks + 1U;
    /* Log the PMU count at Tick demarkation */
    gSafertosPmuLastTickCycleCount = CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM);
}

/*-------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationTaskCreateHook( const xTCB *pxNewTaskHandle )
{
#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
    vInitialiseTaskRunTimeStatistics( ( xRTS * ) pxNewTaskHandle->pvObject );
#endif
}

/*---------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationTaskSwitchHook( const xTCB *pxTCBOfTaskSwitchedOut,
                                 const xTCB *pxTCBOfTaskSwitchedIn )
{
#if defined (configINCLUDE_RUNTIMESTATS) && ( configINCLUDE_RUNTIMESTATS == 1 )
    /* Update the statistics for the task switching out. */
    if( NULL != pxTCBOfTaskSwitchedOut )
    {
    vUpdateRunTimeStatistics( ( xRTS * ) pxTCBOfTaskSwitchedOut->pvObject );
    }
    /* Not used for RTS. */
    ( void ) pxTCBOfTaskSwitchedIn;
#endif
}
/*---------------------------------------------------------------------------*/

__attribute__((weak)) \
void vApplicationIdleHook( void )
{
    /* Idle Task Handle is neccessary for Run Time Stats implementation. If this hook is being re-defined 
     * ensure to have the handle populated and xGetIdleTaskHandle defined */
    if( pdFALSE != xIdleTaskFirstExecution )
    {
        xIdleTaskFirstExecution = pdFALSE;

        xIdleTaskHandle = xTaskGetCurrentTaskHandle();
    }
}

/*---------------------------------------------------------------------------*/

void vRtsInitialise( void )
{
    ulRtsLastTickCount = 0U;
}
/*---------------------------------------------------------------------------*/

void vRtsGetElapsedCPUTime( volatile portUInt32Type *pulTaskSwitchedInTime, portUInt32Type *pulElapsedTime )
{
    TimerP_Handle pxTickTimerHandle = TimerP_getTickTimerHandle();
    portUInt32Type ulCountCurr = ( portUInt32Type )(TimerP_getCount( pxTickTimerHandle ) - TimerP_getReloadCount( pxTickTimerHandle ));
    portUInt32Type ulCountPrev = ( portUInt32Type )( *pulTaskSwitchedInTime );
    portUInt32Type ulTickCurr = ( portUInt32Type ) xTickCount;
    portUInt32Type ulCounterMax = ( portUInt32Type )(TimerP_MAX_PERIOD - TimerP_getReloadCount( pxTickTimerHandle ));
    portUInt32Type ulTimeValue;

    /* Update missed ticks is scheduler is suspended */
    ulTickCurr += uxMissedTicks;

    if( ( ulTickCurr == ulRtsLastTickCount ) && ( ulCountCurr < ulCountPrev ) )
    {
        /* Increment tick if the counter has wrapped around. */
        ++ulTickCurr;
    }
    else if( ( ulTickCurr + 1U ) == ulRtsLastTickCount )
    {
        /* Increment tick if the counter has wrapped around but the tick has not been serviced
         * despite being called once before*/
        ++ulTickCurr;
    }
    else
    {
        /* This block is only here for MISRA compliance. */
    }

    /* Update for elapsed ticks. */
    ulTimeValue = ( ulTickCurr - ulRtsLastTickCount ) * ( ulCounterMax + 1U );

    /* Calculate the counter difference */
    ulTimeValue += ulCountCurr;
    ulTimeValue -= ulCountPrev;

    /* Save tick count for next update */
    ulRtsLastTickCount = ulTickCurr;

    /* Set new time value. */
    *pulElapsedTime = ulTimeValue;
    *pulTaskSwitchedInTime = ulCountCurr;
}
/*---------------------------------------------------------------------------*/

__attribute__((weak)) \
portTaskHandleType xGetIdleTaskHandle( void )
{
    return xIdleTaskHandle;
}

/*---------------------------------------------------------------------------*/

