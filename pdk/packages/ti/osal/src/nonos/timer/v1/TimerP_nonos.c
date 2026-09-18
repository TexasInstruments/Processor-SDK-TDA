/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
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
 *  ======== TimerP_csl_nonos.c ========
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <ti/osal/TimerP.h>
#include <ti/csl/soc.h>
#include <ti/csl/csl_timer.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/osal/src/nonos/Nonos_config.h>

#if defined(FREERTOS)
#include <FreeRTOS.h>
#endif

/* DM Timer Implementation */
extern void Osal_DebugP_assert(int32_t expression, const char *file, int32_t line);
#define TIMOSAL_Assert(expression) (Osal_DebugP_assert(((int32_t)((expression)?1:0)),\
                                                  __FILE__, __LINE__))

/* Local defines for the dm timer */
#define TIMERP_DM_TCLR_START_ONESHOT        (TIMER_ONESHOT_NOCMP_ENABLE)
#define TIMERP_DM_TCLR_START_CONTINUOUS     (TIMER_AUTORLD_CMP_ENABLE)
#define TIMERP_DM_TCLR_START_DYNAMIC        (0x43U)
#define TIMERP_DM_TCLR_STOP_MASK            (0xFFFFFFFEU)
#define TIMERP_DM_TWPS_W_PEND_TMAR          (0x10U)
#define TIMERP_DM_TWPS_W_PEND_TLDR          (0x4U)
#define TIMERP_DM_TWPS_W_PEND_TCRR          (0x2U)
#define TIMERP_DM_TWPS_W_PEND_TCLR          (0x1U)
#define TIMERP_DM_IRQSTATUS_OVF_IT_FLAG     (0x2U)
#define TIMERP_DM_IRQSTATUS_MAT_IT_FLAG     (0x1U)
#define TIMERP_DM_TIOCP_CFG_SOFTRESET_FLAG  (0x1U)
#define TIMERP_DM_TSICR_POSTED              (0x4U)
#define TIMERP_DM_TSICR_READMODE            (0x8U)
#define TIMERP_DM_MAX_PERIOD                (0xFFFFFFFFU)


/* Local Timer Struct */
typedef struct TimerP_Struct_s
{
  bool     used;       /* In use or not status */
  uint32_t timerId;    /* timer Id */
  uint32_t periodType; /* Period type, default micro seconds */
  uint32_t freqLo;     /* least siginificant 32-bits of frequency */
  uint32_t freqHi;     /* most siginificant 32-bits of frequency  */
  uint32_t startMode;  /* timer start mode */
  uint32_t runMode;    /* timer run mode   */
  uint32_t period;     /* Period of a tick */
  TimerP_Fxn tickFxn;  /* Timer Tick function */
  void*    arg;        /* Argument passed into the timer function. */
  uint32_t availMask;   /* Available timer mask */
  HwiP_Handle hwi;      /* Hwi handle for tickFxn */
#if (defined (_TMS320C6X) || defined (BUILD_C7X))
  uint32_t eventId;     /* Event Id for C66x */
#endif
  uint32_t intNum;      /* Interrupt Number */
  uint32_t rollovers;
  uint32_t savedCurrCount;
  uint32_t prevThreshold;
  uint32_t tmar;
  uint32_t tier;
  uint32_t twer;
  uint32_t tclr;
  uint32_t tsicr;
  uint32_t tiocpCfg;

}TimerP_Struct;

/* As static allocation is used, the following provides the structs
 * The TimerP_Struct pool servers to get an available timer
 * handle during timer create API call.
 */
TimerP_Struct gTimerStructs[OSAL_NONOS_CONFIGNUM_TIMER] = { 0U };
static        uint32_t    gTimerInitDone = UFALSE;
static        uint32_t    gTimerAnyMask;
#if defined (BUILD_MCU)
static        bool        gUpdateFlag = BTRUE;
#if defined(SAFERTOS)
/* Data Structure which maintains tick count for Safertos Applications*/
volatile uint64_t gSafertosNumTicks = 0U;
/* PMU cycle count logged at last tick interrupt in Safertos */
volatile uint32_t gSafertosPmuLastTickCycleCount = 0U;
#endif
#endif

/* external variables */
extern uint32_t  gOsalTimerAllocCnt, gOsalTimerPeak;

extern uint64_t uiPortGetRunTimeCounterValue64(void);

/* Local functions  */
static uintptr_t TimerP_getTimerBaseAddr(uint32_t timer_id);
static void TimerP_dmTimerStub(uintptr_t arg);
static void  TimerP_dmTimerInitDevice(TimerP_Struct *timer, uintptr_t baseAddr);
static bool TimerP_dmTimerCheckOverflow(uint32_t a, uint32_t b);
static bool TimerP_dmTimerSetMicroSeconds(TimerP_Struct *timer, uint32_t period);
static TimerP_Status TimerP_dmTimerDeviceCfg(TimerP_Struct *timer, uintptr_t baseAddr);
static TimerP_Status TimerP_dmTimerInitObj(TimerP_Struct *timer, TimerP_Fxn tickFxn, const TimerP_Params *params);
static TimerP_Status TimerP_dmTimerInstanceInit(TimerP_Struct *timer, uint32_t id, TimerP_Fxn tickFxn, const TimerP_Params *params);

/*
 * This private function returns the base address of the timer based on
 * the ID passed in.
 */
static uintptr_t TimerP_getTimerBaseAddr(uint32_t timer_id)
{
    uintptr_t addr;

    if (timer_id < TimerP_numTimerDevices) {
#if defined (BUILD_MCU)
        if (BTRUE == gUpdateFlag)
        {
            TimerP_updateDefaultInfoTbl();
            gUpdateFlag = BFALSE;
        }
#endif
      addr = gDmTimerPInfoTbl[timer_id].baseAddr;
    }
    else {
      addr = 0U;
    }
    return (addr);
}

/*
 * This private function is a stub function for the timer ISR
 * function that is registered from the application
 */
static void TimerP_dmTimerStub(uintptr_t arg)
{
  TimerP_Struct *timer = (TimerP_Struct *) arg;
  uintptr_t baseAddr = TimerP_getTimerBaseAddr(timer->timerId);

  /* Disable the Timer interrupts */
  (void)TIMERIntDisable(baseAddr, TIMER_INT_OVF_EN_FLAG);

  /* acknowledge the interrupt */
  (void)TIMERIntStatusClear(baseAddr, timer->tier);

  /* call the user's ISR */
  timer->tickFxn((uintptr_t)timer->arg);

   /* Enable the Timer interrupts */
  (void)TIMERIntEnable(baseAddr, TIMER_INT_OVF_EN_FLAG);
}

/*
 * This priviate function initializes the timer counter, period and compare
 * registers
 */
static void  TimerP_dmTimerInitDevice(TimerP_Struct *timer, uintptr_t baseAddr)
{
  uint32_t key, status;
  uint32_t tcrr = 0U, tldr = 0U;

  key = (uint32_t)HwiP_disable();

  (void)TIMERDisable(baseAddr);
  do {
    status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCLR);
  } while (0U != status);

  (void)TIMERCounterSet(baseAddr, tcrr);

   do {
    status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCRR);
  } while (0U != status);

  (void)TIMERReloadSet(baseAddr, tldr);

  do {
    status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TLDR);
  } while (0U != status);


  HwiP_restore(key);
}

/*
 * This priviate function checks if there is any overflow in
 * timer period computations
 */
static bool TimerP_dmTimerCheckOverflow(uint32_t a, uint32_t b)
{
  return ((b > 0U) && (a > (TimerP_MAX_PERIOD/b)));
}

/*
 * This is a private function to set the period of the timer
 * register for a specified micro second value
 */
static bool TimerP_dmTimerSetMicroSeconds(TimerP_Struct *timer, uint32_t period)
{
    uint64_t  counts;
    uint32_t  prdCounts;
    uint32_t  freqKHz;
    uint32_t  roundUp;
    uintptr_t  baseAddr = TimerP_getTimerBaseAddr(timer->timerId);
    uint32_t  status;
    bool retVal = BTRUE;

    (void)TimerP_stop(timer);

    roundUp = (500U <= (timer->freqLo % 1000U)) ? UTRUE : UFALSE;

    freqKHz = (timer->freqLo / 1000U) + roundUp;
    if (TimerP_dmTimerCheckOverflow(freqKHz, period/1000U)) {
            retVal = BFALSE;
    }
    else
    {
    counts = ((uint64_t)freqKHz * (uint64_t)period) / (uint64_t)1000U;
    if (0xFFFFFFFFU < counts) {
        retVal = BFALSE;
    }
    else
    {
       prdCounts = (uint32_t)(counts - (uint32_t) 1U);

       timer->period = prdCounts;
       timer->periodType = TimerP_PeriodType_COUNTS;

       if ((timer->runMode == TimerP_RunMode_CONTINUOUS) ||
         (timer->runMode == TimerP_RunMode_ONESHOT)) 
        {
           (void)TIMERCounterSet(baseAddr, TimerP_MAX_PERIOD - prdCounts);

           do {
             status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCRR);
           } while (0U != status);

           (void)TIMERReloadSet(baseAddr,TimerP_MAX_PERIOD - prdCounts);
           do {
             status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TLDR);
           } while (0U != status);
        }
        else
        {
          (void)TIMERCounterSet(baseAddr, 0U);
          do {
          status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCRR);
        } while (0U != status);

          (void)TIMERCompareSet(baseAddr, prdCounts);
          do {
          status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TMAR);
        } while (0U != status);
       }
     }
   }
    return(retVal);
}

/*
 * Thi is a private function to configure the timer registers for a given timer
 * period
 */
static TimerP_Status TimerP_dmTimerDeviceCfg(TimerP_Struct *timer, uintptr_t baseAddr)
{
    uint32_t key, tsicr;
    uint32_t status;
    TimerP_Status retVal=TimerP_OK;
    /* initialize the timer */
    TimerP_dmTimerInitDevice(timer, baseAddr);

    key = (uint32_t)HwiP_disable();

    /*if doing soft reset: do it first before setting other flags */
    if (0U < (timer->tiocpCfg & TIMERP_DM_TIOCP_CFG_SOFTRESET_FLAG)) {
     (void)TIMERReset(baseAddr);
    }

    /* Soft Reset(if required) is already issued. Now Set 'emulation mode' and 'idle mode' in TIOCP_CFG. */
    HW_WR_REG32((baseAddr + TIMER_TIOCP_CFG),
                 (timer->tiocpCfg & (~TIMER_TIOCP_CFG_SOFTRESET_SOFTRESET_VALUE_1)));
  

    /*xfer 'posted' setting if not current */
    tsicr = HW_RD_REG32(baseAddr + TIMER_TSICR);

    if (0U < (timer->tsicr & TIMERP_DM_TSICR_POSTED)) {
      if (0U == (tsicr & TIMERP_DM_TSICR_POSTED)) {
          (void)TIMERPostedModeConfig(baseAddr, (tsicr | TIMERP_DM_TSICR_POSTED));
      }
    }
    else {
        if (0U != (tsicr & TIMERP_DM_TSICR_POSTED)) {
          (void)TIMERPostedModeConfig(baseAddr, (tsicr & (~TIMERP_DM_TSICR_POSTED)));
        }
    }

    /* xfer 'readmode' setting if not current */
    tsicr = HW_RD_REG32(baseAddr + TIMER_TSICR);

    if (0U < (timer->tsicr & TIMERP_DM_TSICR_READMODE)) {
        if (0U == (tsicr & TIMERP_DM_TSICR_READMODE)) {
            (void)TIMERReadModeConfig(baseAddr, (tsicr | TIMERP_DM_TSICR_READMODE));
        }
    }
    else {
        if (0U != (tsicr & TIMERP_DM_TSICR_READMODE)) {
          (void)TIMERReadModeConfig(baseAddr, (tsicr | (~TIMERP_DM_TSICR_READMODE)));
        }
    }

    /* set the period */
    if (TimerP_PeriodType_MICROSECS == timer->periodType) {
        if (!TimerP_dmTimerSetMicroSeconds(timer, timer->period)) {
            HwiP_restore(key);
            retVal = TimerP_FAILURE;
        }
    }
    else {
        if ((TimerP_RunMode_CONTINUOUS == timer->runMode) ||
            (TimerP_RunMode_ONESHOT    == timer->runMode)) {

            (void)TIMERCounterSet(baseAddr, TimerP_MAX_PERIOD - timer->period);
            do {
              status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCRR);
            } while (0U != status);

            (void)TIMERReloadSet(baseAddr, TimerP_MAX_PERIOD - timer->period);
            do {
              status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TLDR);
            } while (0U != status);

        }
        else {
            (void)TIMERCounterSet(baseAddr,0);
            do {
              status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCRR);
            } while (0U != status);


            (void)TIMERCompareSet(baseAddr, timer->period);
            do {
              status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TMAR);
            } while (0U != status);


        }
    }
   
   if(TimerP_FAILURE != retVal)
   {
    if ((timer->runMode == TimerP_RunMode_CONTINUOUS) ||
        (timer->runMode == TimerP_RunMode_ONESHOT)) {
        (void)TIMERCompareSet(baseAddr, timer->tmar);
        do {
          status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TMAR);
        } while (0U != status);

    }
    /* Enable the Timer Wakeup events represented by wakeFlags */
    HW_WR_REG32(baseAddr + TIMER_IRQWAKEEN, timer->twer);
    HW_WR_REG32(baseAddr + TIMER_IRQENABLE_SET, timer->tier);
    HW_WR_REG32(baseAddr + TIMER_TCLR, timer->tclr);
    do {
      status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCLR);
    } while (0U != status);

     HwiP_restore(key);
   } 
    return (retVal);
}

/*
 * This is a private function to initialize the timer counter, period and compare
 * registers before starting the timers
 */
static TimerP_Status TimerP_dmTimerInitObj(TimerP_Struct *timer, TimerP_Fxn tickFxn, const TimerP_Params *params)
{
  uint8_t softreset = 1U, emufree = 0U, idlemode = 0U;
  uint8_t mat_it_ena = 0U, ovf_it_ena = 1U, tcar_it_ena = 0U;
  uint8_t mat_wup_ena = 0U, ovf_wup_ena = 0U, tcar_wup_ena = 0U;


  timer->tickFxn = tickFxn;

  timer->tiocpCfg = (uint32_t)( ((uint32_t)softreset << 0U) |
                                ((uint32_t)emufree   << 1U) |
                                ((uint32_t)idlemode  << 2U));

  timer->tier     = (uint32_t)( ((uint32_t)mat_it_ena << 0U) |
                                ((uint32_t)ovf_it_ena << 1U) |
                                ((uint32_t)tcar_it_ena << 2U));

  timer->twer     = (uint32_t)( ((uint32_t)mat_wup_ena << 0U) |
                                ((uint32_t)ovf_wup_ena << 1U) |
                                ((uint32_t)tcar_wup_ena << 2U));

  timer->tclr = 0U;

  timer->tsicr = 0U;

  /*
   * Update the default initializations to user configured values
   */
  if ( TimerP_USE_DEFAULT != params->extfreqLo ) {
    timer->freqLo            =  params->extfreqLo;
  }
  else  {
    if ( TimerP_USE_DEFAULT != params->intfreqLo )  {
      timer->freqLo            = params->intfreqLo;
    }
    else {
      timer->freqLo            =  TimerP_getDefaultFreqLo(timer->timerId);
    }
  }

  if ( TimerP_USE_DEFAULT != params->extfreqHi ) {
    timer->freqHi            =  params->extfreqHi;
  }
  else  {
    if ( TimerP_USE_DEFAULT != params->intfreqHi )  {
      timer->freqHi          = params->intfreqHi;
    }
    else {
      timer->freqHi          =  TimerP_getDefaultFreqHi(timer->timerId);
    }
  }

  if ( 0U != params->period) {
    timer->period                = params->period;
  }
  else  {
    timer->period                = 0U;
  }

   if ( NULL_PTR != params->arg ) {
     timer->arg                   = params->arg;
   }

   if ( TimerP_USE_DEFAULT != params->intNum ) {
     timer->intNum = params->intNum;
   }
   else {
     timer->intNum = (uint32_t)gDmTimerPInfoTbl[timer->timerId].intNum;
   }

#if (defined (_TMS320C6X) || defined (BUILD_C7X))
   if ( TimerP_USE_DEFAULT != params->eventId ) {
     timer->eventId = params->eventId;
   }
   else {
     timer->eventId = (uint32_t)gDmTimerPInfoTbl[timer->timerId].eventId;
   }
#endif

   timer->periodType = params->periodType;
   timer->startMode  = params->startMode;
   timer->runMode    = params->runMode;
   return(TimerP_OK);
}

static uint32_t TimerP_isDMTIdRestricted(uint32_t id)
{
    uint32_t isRestricted = 0U;
    uint32_t looper;
#if defined (BUILD_MCU)
	uint32_t coreId;
	volatile extern Osal_timerReserved Osal_mainMcuRestrictedTimerID[TimerP_ANY_MAX_RESTRICTIONS], Osal_mcuRestrictedTimerID[TimerP_ANY_MAX_RESTRICTIONS];
	coreId = Osal_getCoreId();
	if ((OSAL_MCU1_0 == coreId) || (OSAL_MCU1_1 == coreId))
	{
    for (looper = 0U; looper < TimerP_ANY_MAX_RESTRICTIONS; looper++)
    {
        if((OSAL_INVALID_CORE_ID != Osal_mcuRestrictedTimerID[looper].coreId) &&
           (id == TimerP_mapId(Osal_mcuRestrictedTimerID[looper].timerId, Osal_mcuRestrictedTimerID[looper].coreId)))
        {
            isRestricted = 1U;
            break;
        }
    }
	}
	else
	{
		for (looper = 0U; looper < TimerP_ANY_MAX_RESTRICTIONS; looper++)
    {
        if((OSAL_INVALID_CORE_ID != Osal_mainMcuRestrictedTimerID[looper].coreId) &&
           (id == TimerP_mapId(Osal_mainMcuRestrictedTimerID[looper].timerId, Osal_mainMcuRestrictedTimerID[looper].coreId)))
        {
            isRestricted = 1U;
            break;
        }
    }
	}
#elif defined (BUILD_C7X)
	volatile extern Osal_timerReserved Osal_c7xRestrictedTimerID[TimerP_ANY_MAX_RESTRICTIONS];
	for (looper = 0U; looper < TimerP_ANY_MAX_RESTRICTIONS; looper++)
  {
      if((OSAL_INVALID_CORE_ID != Osal_c7xRestrictedTimerID[looper].coreId) &&
         (id == TimerP_mapId(Osal_c7xRestrictedTimerID[looper].timerId, Osal_c7xRestrictedTimerID[looper].coreId)))
      {
          isRestricted = 1U;
          break;
      }
  }
#elif defined (BUILD_C66X)
	volatile extern Osal_timerReserved Osal_c66RestrictedTimerID[TimerP_ANY_MAX_RESTRICTIONS];
	for (looper = 0U; looper < TimerP_ANY_MAX_RESTRICTIONS; looper++)
  {
      if((OSAL_INVALID_CORE_ID != Osal_c66RestrictedTimerID[looper].coreId) &&
         (id == TimerP_mapId(Osal_c66RestrictedTimerID[looper].timerId, Osal_c66RestrictedTimerID[looper].coreId)))
      {
          isRestricted = 1U;
          break;
      }
  }
#elif defined (BUILD_MPU)
	volatile extern Osal_timerReserved Osal_mpuRestrictedTimerID[TimerP_ANY_MAX_RESTRICTIONS];
	for (looper = 0U; looper < TimerP_ANY_MAX_RESTRICTIONS; looper++)
  {
      if((OSAL_INVALID_CORE_ID != Osal_mpuRestrictedTimerID[looper].coreId) && 
         (id == TimerP_mapId(Osal_mpuRestrictedTimerID[looper].timerId, Osal_mpuRestrictedTimerID[looper].coreId)))
      {
          isRestricted = 1U;
          break;
      }
  }
#endif
    return isRestricted;
}

/*
 * This is a private function to initialize the timer instance, that sets up the timer ISR to
 * a specified timer id and sets up the timer compare, counter and period registers
 */
static TimerP_Status TimerP_dmTimerInstanceInit(TimerP_Struct *timer, uint32_t id, TimerP_Fxn tickFxn, const TimerP_Params *params)
{
  TimerP_Status ret = TimerP_OK;
  uint32_t      key;
  uint32_t      i;
  uint32_t      tempId = 0xFFFFU;
  OsalRegisterIntrParams_t interruptRegParams;
  uintptr_t     baseAddr;
  uint32_t      intNum;
  uint32_t      timerPAnyMask = TIMERP_ANY_MASK;
  uint32_t      mappedId = 0xFFFF;
  uint32_t      coreId = Osal_getCoreId();

#if defined (BUILD_MCU)
  if ((coreId == OSAL_MCU1_0) || (coreId == OSAL_MCU1_1))
  {
    timerPAnyMask = TIMERP_ANY_MASK_MCU_DOMAIN;
  }
#endif

  if ((TimerP_ANY != id) && (TimerP_numTimerDevices <= id)) {
    ret = TimerP_FAILURE;
  }

  /* Get the timer Id */
  if (TimerP_OK == ret)
  {
      /* Set the available timer id mask to all */
    if (UFALSE == gTimerInitDone)
    {
        gTimerAnyMask  = TIMERP_AVAILABLE_MASK;
    #if defined (BUILD_MCU)
        if ((coreId == OSAL_MCU1_0) || (coreId == OSAL_MCU1_1))
        {
          gTimerAnyMask = TIMERP_AVAILABLE_MASK_MCU_DOMAIN;
        }
    #endif
        gTimerInitDone = UTRUE;
    }

    timer->availMask = gTimerAnyMask;

    key = (uint32_t)HwiP_disable();

    if (TimerP_ANY == id)
    {
        for (i = 0U; i < TimerP_numTimerDevices; i++)
        {
            uint32_t shift, nshift;
            mappedId = TimerP_mapId(i, coreId);
            shift  = ((uint32_t) 1U) << i;
            nshift = ~shift;
            if (((uint32_t) 0U != (timerPAnyMask    & shift)) &&
                ((uint32_t) 0U != (timer->availMask & shift)) &&
                (0U == TimerP_isDMTIdRestricted(mappedId)))
            {
                 timer->availMask &= nshift;
                 tempId = mappedId;
                 break;
            }
        }
    }
    else
    {
        if (0U < (timer->availMask & ((uint32_t)1U << id))) 
        {
            uint32_t shift, nshift;
            shift  = ((uint32_t) 1U) << id;
            nshift = ~shift;
            timer->availMask &= nshift;
            tempId = id;
            tempId = TimerP_mapId(tempId, coreId);
        }
    }  

    gTimerAnyMask =  timer->availMask;

    /* Initialize the timer state object */
    timer->timerId = tempId; /* Record the timer Id */
    baseAddr = TimerP_getTimerBaseAddr(timer->timerId);

    HwiP_restore(key);

    if ((0xFFFFU == tempId) || ((uintptr_t)0x0U == baseAddr)) {
      ret = TimerP_NOT_AVAILABLE;
    }
  }

  /* Initialize the timer state object */
  if (TimerP_OK == ret) {
    ret = TimerP_dmTimerInitObj(timer, tickFxn, params);
  }

  /* Create the Hwi Funtion for the tick funtion */
  if (TimerP_OK == ret)
  {
    if ((TimerP_Fxn) NULL_PTR != timer->tickFxn)
    {
      /* PDK-6534 If the timers were indeed running (left running from previous)
            op. The provided ISR could be invoked at un-expected time.
            Ensure to reset the timer */

      /* tiocpCfg is populated by tiocpCfg */
      if (0U < (timer->tiocpCfg & TIMERP_DM_TIOCP_CFG_SOFTRESET_FLAG)) {
       (void)TIMERReset(baseAddr);
      }

      intNum          = timer->intNum;
      /* Initialize with defaults */
      Osal_RegisterInterrupt_initParams(&interruptRegParams);

      /* Populate the interrupt parameters */
      interruptRegParams.corepacConfig.arg=(uintptr_t) timer;
      interruptRegParams.corepacConfig.name= (char *) NULL_PTR;
      interruptRegParams.corepacConfig.isrRoutine=TimerP_dmTimerStub;

#if defined (__ARM_ARCH_7A__) || defined (__aarch64__) || ((__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'R') )
#if defined (SOC_AM335x) || defined (SOC_AM437x) || defined (SOC_AM65XX) || defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_AM64X) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
      interruptRegParams.corepacConfig.triggerSensitivity = OSAL_ARM_GIC_TRIG_TYPE_HIGH_LEVEL;
      interruptRegParams.corepacConfig.priority = 0x20U;
#else
      interruptRegParams.corepacConfig.triggerSensitivity = OSAL_ARM_GIC_TRIG_TYPE_EDGE;
#endif
#endif

#if defined(_TMS320C6X)
      interruptRegParams.corepacConfig.corepacEventNum = timer->eventId; /* Event going in to CPU */
#endif
#if defined (BUILD_C7X)
      interruptRegParams.corepacConfig.corepacEventNum = timer->eventId; /* Event going in to CPU */
      interruptRegParams.corepacConfig.priority = 0x1U;
#endif
      interruptRegParams.corepacConfig.intVecNum = intNum; /* Host Interrupt vector */

      /* Register interrupts */
      (void)Osal_RegisterInterrupt(&interruptRegParams,&(timer->hwi));

      if (NULL_PTR == timer->hwi)
      {
        ret = TimerP_ISR_HOOK_ERR;
      }
    }
    else
    {
      timer->hwi = NULL_PTR;
    }
  }

  /* timer post init */
  if (TimerP_OK == ret)
  {
     ret = TimerP_dmTimerDeviceCfg(timer, baseAddr);

     if (TimerP_StartMode_AUTO == timer->startMode) {
         (void)TimerP_start(timer);
     }
  }

  return (ret);
}


/* Interface functions */

/* Default parameter initialization module function
 * more details on the documentation of this interface
 * function can be found in the TimerP.h interface
 * header file */
void TimerP_Params_init(TimerP_Params *params)
{
    TIMOSAL_Assert(NULL_PTR == params);

    if(NULL_PTR != params) {
      /* Set the default values */
      params->arg         = NULL_PTR;
      params->extfreqHi   = TimerP_USE_DEFAULT;
      params->extfreqLo   = TimerP_USE_DEFAULT;
      params->intfreqHi   = TimerP_USE_DEFAULT;
      params->intfreqLo   = TimerP_USE_DEFAULT;
      params->name        = (char *) NULL_PTR;
      params->period      = 0;
      params->runMode     = TimerP_RunMode_CONTINUOUS;
      params->startMode   = TimerP_StartMode_AUTO;
      params->periodType  = TimerP_PeriodType_MICROSECS;
      params->intNum      = TimerP_USE_DEFAULT;
#if defined (_TMS320C6X)
    params->eventId     = TimerP_USE_DEFAULT;
#endif
    }  
    return;
}

/* Timer create module function
 * more details on the documentation of this interface
 * function can be found in the TimerP.h interface
 * header file */
TimerP_Handle TimerP_create(uint32_t id,
                            TimerP_Fxn tickFxn,
                            const TimerP_Params *inParams)

{
    uint32_t          i;
    uintptr_t         key;
    TimerP_Handle     ret_handle;
    TimerP_Params    params;

    if (NULL_PTR != inParams)
    {
        params = *inParams;
    }
    else
    {
        TimerP_Params_init(&params);
    }

    key = HwiP_disable();

    for (i = 0; i < OSAL_NONOS_CONFIGNUM_TIMER; i++)
    {
        if (BFALSE == gTimerStructs[i].used)
        {
            gTimerStructs[i].used = BTRUE;

            /* Update statistics */
            gOsalTimerAllocCnt++;
            if (gOsalTimerAllocCnt > gOsalTimerPeak)
            {
                gOsalTimerPeak = gOsalTimerAllocCnt;
            }

            gTimerStructs[i].timerId  = id;

            break;
        }
    }
    HwiP_restore(key);

    if (OSAL_NONOS_CONFIGNUM_TIMER == i)
    {
        ret_handle = NULL_PTR;
    }
    else
    {
        /* Start the timer for that period/mode */
        ret_handle = ((TimerP_Handle)&gTimerStructs[i]);
    }

    if (NULL_PTR != ret_handle)
    {
      if (TimerP_dmTimerInstanceInit(&gTimerStructs[i], id, tickFxn, &params) != TimerP_OK) {
        /* Free up the consumed timer memory */
        gTimerStructs[i].used = BFALSE;
        ret_handle = NULL_PTR;
      }
    }

    /* start timer if it is auto mode */
    if ((TimerP_StartMode_AUTO == params.startMode) &&
        (NULL_PTR              != ret_handle))
    {
       if(TimerP_OK != TimerP_start((TimerP_Handle)&gTimerStructs[i])) {
          gTimerStructs[i].used = BFALSE;
          ret_handle = NULL_PTR;
       }
    }
    return (ret_handle);
}


/* Timer delete module function
 * more details on the documentation of this interface
 * function can be found in the TimerP.h interface
 * header file */
TimerP_Status TimerP_delete(TimerP_Handle handle)
{
    /* Release or free the memory */
    uint32_t key;
    uint32_t id;
    uint32_t index = (uint32_t)(( ((uintptr_t) handle) - ((uintptr_t) gTimerStructs) )/sizeof(TimerP_Struct)); /* struct subtraction */
    TimerP_Status ret = TimerP_OK;
    TimerP_Struct *timer = (TimerP_Struct *) handle;
    TIMOSAL_Assert(NULL_PTR == handle);

    key = (uint32_t)HwiP_disable();

    
    id = gTimerStructs[index].timerId;
    id = TimerP_reverseMapId(id, Osal_getCoreId());

    if((NULL_PTR != timer) && (BTRUE == (gTimerStructs[index].used)))
    {
      /* clear the ISR that was set before */
      if(NULL_PTR != timer->hwi) 
      {
        (void)HwiP_delete(timer->hwi);
      }
      
      /* reset the timer's bit field in the mask and clear the used flag */
      gTimerStructs[index].used = BFALSE;
      uint32_t shift = ((uint32_t) 1U) << id;
      if(0U == (gTimerAnyMask & shift))
      {
        gTimerAnyMask |= shift;
      }
      else
      {
        ret = TimerP_FAILURE;	
      }

      /* Found the osal timer object to delete */
      if (0U < gOsalTimerAllocCnt)
      {
        gOsalTimerAllocCnt--;
      }
    } 
    else
    {
      ret = TimerP_FAILURE;	
    }

    HwiP_restore(key);

    return (ret);
}


/* Timer start module function
 * more details on the documentation of this interface
 * function can be found in the TimerP.h interface
 * header file */
TimerP_Status TimerP_start(TimerP_Handle handle)
{
  uint32_t  key, runMode, status;
  TimerP_Struct *timer = (TimerP_Struct *) handle;
  uint32_t max_period = (uint32_t) 0xFFFFFFFFU;
  TimerP_Status retVal = TimerP_OK;
  uint32_t  eventId;
  uintptr_t baseAddr = TimerP_getTimerBaseAddr(timer->timerId);

  if ((uintptr_t)0U == baseAddr) {
    retVal = TimerP_NOT_AVAILABLE;
  }
  else
  {

  key = (uint32_t)HwiP_disable();

  (void)TimerP_stop(handle);

  if ((TimerP_RunMode_CONTINUOUS == timer->runMode) ||
      (TimerP_RunMode_ONESHOT    == timer->runMode)) {
    (void)TIMERCounterSet(baseAddr, (max_period - timer->period));             /* set timer count back to period value */
  }
  else {
    (void)TIMERCounterSet(baseAddr, 0);             /* set timer count back to initial value */
    (void)TIMERCompareSet(baseAddr, timer->period); /* set threshold for first interrupt */

    timer->prevThreshold  = 0;    /* init previous threshold */
    timer->rollovers      = 0;    /* init total rollover count */
    timer->savedCurrCount = 0;
  }

  do {
    status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCRR);
  } while (0U != status);

  if(NULL_PTR != timer->hwi) {
#if defined (_TMS320C6X)
    eventId = timer->eventId;
#else
    eventId = 0U;
#endif
    Osal_ClearInterrupt(eventId,timer->intNum);
    Osal_EnableInterrupt(eventId,timer->intNum);
  }
  else {
      if (timer->tickFxn != NULL)
      {
        retVal = TimerP_FAILURE;
      }
  }

  if (TimerP_RunMode_CONTINUOUS == timer->runMode) {
      runMode = TIMERP_DM_TCLR_START_CONTINUOUS;
  }
  else if (TimerP_RunMode_ONESHOT == timer->runMode) {
      runMode = TIMERP_DM_TCLR_START_ONESHOT;
  }
  else {
      runMode = TIMERP_DM_TCLR_START_DYNAMIC;
  }

  if (TimerP_OK == retVal) {
      (void)TIMERModeConfigure(baseAddr, runMode);

      do {
        status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCLR);
      } while (0U != status);

      (void)TIMEREnable(baseAddr);
  }
  HwiP_restore(key);
  }
  return (retVal);
}

/* Timer stop module function
 * more details on the documentation of this interface
 * function can be found in the TimerP.h interface
 * header file */

TimerP_Status TimerP_stop(TimerP_Handle handle)
{
  uint32_t  tisr, status;
  TimerP_Struct *timer = (TimerP_Struct *) handle;
  TimerP_Status retVal = TimerP_OK;
  uint32_t eventId;
  uintptr_t baseAddr = TimerP_getTimerBaseAddr(timer->timerId);

  if ((uintptr_t)0U == baseAddr) {
    retVal = TimerP_NOT_AVAILABLE;
  }
  else
  {
    (void)TIMERDisable(baseAddr);

    do {
      status = (TIMERWritePostedStatusGet(baseAddr) & TIMERP_DM_TWPS_W_PEND_TCLR);
    } while (0U != status);


    tisr = TIMERIntStatusGet(baseAddr);


    if(0U < tisr) {
      /* Clear all pending interrupts */
      (void)TIMERIntStatusClear(baseAddr, tisr);
    }

    if(NULL_PTR != timer->hwi) {
#if defined (_TMS320C6X)
      eventId = timer->eventId;
#else
      eventId = 0U;
#endif
      Osal_ClearInterrupt(eventId,timer->intNum);
      Osal_DisableInterrupt(eventId,timer->intNum);
    }
    else {
      if (timer->tickFxn != NULL)
      {
          retVal = TimerP_FAILURE;
      }
    }
  }
  return(retVal);
}

/* Timer set micro seconds module function
 * more details on the documentation of this interface
 * function can be found in the TimerP.h interface
 * header file */
TimerP_Status TimerP_setPeriodMicroSecs(TimerP_Handle handle, uint32_t microsecs)
{
  TimerP_Struct *timer = (TimerP_Struct *) handle;
  TimerP_Status retVal = TimerP_OK;
  if (!TimerP_dmTimerSetMicroSeconds(timer, microsecs)) {
    retVal = TimerP_FAILURE;
  }
  return (retVal);
}

/* Get time in micro seconds */
uint64_t TimerP_getTimeInUsecs(void)
{
    uint64_t         curTime;
    #if defined(FREERTOS)
    /* In case of FreeRTOS get the current counter value from
     * the timer used by OS itself, instead of using an 
     * additional timer for the same */
    #if defined (BUILD_MCU)
      curTime = uiPortGetRunTimeCounterValue64();
    #else
      curTime = uiPortGetRunTimeCounterValue();
    #endif
    #elif defined(SAFERTOS) && defined(BUILD_MCU)
    osalArch_TimestampInit();
    uint64_t freqInMHz = osalArch_TimeStampGetFreqKHz() / 1000U;
    
    /* Create a critical section while reading interrupt  */
    uint32_t  key = (uint32_t)HwiP_disable();
    uint32_t currentPmuCount = CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM);
    /* Formula: Time in Usec = Number of Ticks * 1000U + Difference in PMU cycles/Frequency of CPU in MHz */
    curTime = gSafertosNumTicks * 1000U + ((uint64_t)(currentPmuCount - gSafertosPmuLastTickCycleCount)/freqInMHz);
    
    /* Deal with overflow case of PMU 32-bit counter */
    if (currentPmuCount <= gSafertosPmuLastTickCycleCount)
    {
      curTime += (((uint64_t)1U << 32U)/freqInMHz);
    }
    HwiP_restore(key);
    #else
    TimeStamp_Struct timestamp64;
    uint64_t         cur_ts, freq;
    uint32_t         tsFreqKHz;

    /* Get the timestamp */
    osalArch_TimestampGet64(&timestamp64);

    /* Get the frequency of the timeStamp provider */
    tsFreqKHz  = osalArch_TimeStampGetFreqKHz();
    cur_ts = ((uint64_t) timestamp64.hi << 32U) | timestamp64.lo;
    freq = (uint64_t) (tsFreqKHz);
    curTime = (cur_ts*1000U)/freq;
    #endif
    return (curTime);
}

/* Get timer reload count */
uint32_t TimerP_getReloadCount(TimerP_Handle handle)
{
  uint32_t      reloadCnt = 0U;
  TimerP_Struct *timer = (TimerP_Struct *) handle;
  uintptr_t      baseAddr = TimerP_getTimerBaseAddr(timer->timerId);

  if ((uintptr_t)0U != baseAddr) 
  {
    (void)TIMERReloadGet2(baseAddr, &reloadCnt);
  } 

  return (reloadCnt);
}

/* Get timer current count */
uint32_t TimerP_getCount(TimerP_Handle handle)
{
  uint32_t      count = 0U;
  TimerP_Struct *timer = (TimerP_Struct *) handle;
  uintptr_t     baseAddr = TimerP_getTimerBaseAddr(timer->timerId);

  if ((uintptr_t)0U != baseAddr) 
  {
    (void)TIMERCounterGet2(baseAddr, &count);
  } 

  return (count);

}

/* This file implements the DM timer osal functions on K3 devices */
