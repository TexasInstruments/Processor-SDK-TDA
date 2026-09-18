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


/* SafeRTOS Includes */
#include "SafeRTOS_API.h"
#include "aborts.h"
#include "safertos_config.h"

#include <ti/osal/TimerP.h>
#include <ti/osal/CycleprofilerP.h>

#if defined (BUILD_MCU)
#include <ti/csl/arch/r5/csl_arm_r5_pmu.h>
#endif

#if defined (BUILD_C7X)
#include <ti/csl/arch/csl_arch.h>
#include <ti/osal/src/nonos/Nonos_config.h>
#endif

#if defined (BUILD_C7X) || defined (BUILD_MCU)
#define configTIMER_ID  configTICK_TIMER_ID
#endif


#if defined (BUILD_MCU)
/* TimerP Global Variable Imports to be updated every tick interrupt */
extern uint64_t gSafertosNumTicks;
extern uint32_t gSafertosPmuLastTickCycleCount;
#endif

/* Timer Hook function handlers targeting the TI PDK libraries. */

/* Timer interrupt handler function. */
static void prvTimerTickIsr( uintptr_t arg );
#if defined (BUILD_C7X)
void vPortInitTimerCLECCfg( uint32_t timerId, uint32_t timerIntNum );
#endif
void vApplicationSetupTickInterruptHook( portUInt32Type ulTimerClockHz,
                                         portUInt32Type ulTickRateHz );

/*-------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Private function definitions.
 *---------------------------------------------------------------------------*/

static void prvTimerTickIsr( uintptr_t arg )
{
    ( void ) arg;
    vTaskProcessSystemTickFromISR();
}

/*-------------------------------------------------------------------------*/

#if defined (BUILD_C7X)
void vPortInitTimerCLECCfg( uint32_t timerId, uint32_t timerIntNum )
{
    CSL_ClecEventConfig   cfgClec;
    CSL_CLEC_EVTRegs     *clecBaseAddr = (CSL_CLEC_EVTRegs*)CSL_COMPUTE_CLUSTER0_CLEC_REGS_BASE;
    uint32_t input         = gDmTimerPInfoTbl[timerId].eventId;
    uint32_t corepackEvent = timerIntNum;

    /* Configure CLEC */
    cfgClec.secureClaimEnable = UFALSE;
    cfgClec.evtSendEnable     = UTRUE;
    cfgClec.rtMap             = CSL_CLEC_RTMAP_CPU_ALL;
    cfgClec.extEvtNum         = 0;
    cfgClec.c7xEvtNum         = corepackEvent;
    CSL_clecClearEvent(clecBaseAddr, input);
    CSL_clecConfigEventLevel(clecBaseAddr, input, 0); /* configure interrupt as pulse */
    CSL_clecConfigEvent(clecBaseAddr, input, &cfgClec);
}
/*---------------------------------------------------------------------------*/
#endif

/*-----------------------------------------------------------------------------
 * Public function definitions.
 *---------------------------------------------------------------------------*/

/* Tick timer setup using TI PDK timers and interrupts. */
void vApplicationSetupTickInterruptHook( portUInt32Type ulTimerClockHz,
                                         portUInt32Type ulTickRateHz )
{
    TimerP_Handle pxTickTimerHandle = NULL;
    TimerP_Params xTimerParams;
    
    ( void ) ulTimerClockHz;

#if defined (BUILD_C7X)
    vPortInitTimerCLECCfg( configTICK_TIMER_ID,
                           configTICK_TIMER_INT_NUM );
#endif

    TimerP_Params_init( &xTimerParams );
    xTimerParams.runMode    = TimerP_RunMode_CONTINUOUS;
    xTimerParams.startMode  = TimerP_StartMode_USER;
    xTimerParams.periodType = TimerP_PeriodType_MICROSECS;
    xTimerParams.period     = ( 1000000UL / ulTickRateHz );
#if defined (BUILD_C66X)
    xTimerParams.intNum     = configTIMER_INT_NUM;
    xTimerParams.eventId    = configTIMER_EVENT_ID;
#endif
#if defined (BUILD_C7X)
    xTimerParams.intNum     = configTICK_TIMER_INT_NUM;
    xTimerParams.eventId    = TimerP_USE_DEFAULT;
#endif


    pxTickTimerHandle = TimerP_create( configTIMER_ID,
                                       &prvTimerTickIsr,
                                       &xTimerParams );

    /* don't expect the handle to be null or the timer to start OK */
    if( ( NULL == pxTickTimerHandle ) ||
        ( TimerP_OK != TimerP_start( pxTickTimerHandle ) ) )
    {
        vApplicationAbort();
    }
}

void vApplicationTickHook ( void )
{
    CycleprofilerP_refreshCounter();

#if defined (BUILD_MCU)
    /* Ensure to add the logic in redefinition of vApplicationTickHook in order to 
     * sync usec level core timing across WFI calls */
    gSafertosNumTicks = gSafertosNumTicks + 1U;
    /* Log the PMU count at Tick demarkation */
    gSafertosPmuLastTickCycleCount = CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM);
#endif
}

/*-------------------------------------------------------------------------*/

