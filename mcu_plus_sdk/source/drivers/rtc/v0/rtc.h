/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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
 *  \defgroup DRV_RTC_MODULE APIs for RTC
 *  \ingroup DRV_MODULE
 *
 *  This module contains APIs to program and use the RTC module.
 *
 *  @{
 */

/**
 *  \file v0/rtc.h
 *
 *  \brief RTC Driver API/interface file.
 */

#ifndef RTC_H_
#define RTC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdbool.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/HwiP.h>
#include <drivers/hw_include/cslr_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief 32.768 kHz Crystal Oscillator Frequency*/
#define OSCILLATOR_FREQ_HZ              ((uint32_t) 32768U)

/** \brief Enable Freeze Mode*/
#define RTC_FREEZE_MODE             ((uint32_t) 0x02000000U)

/** \brief O32K_OSC_DEP_EN_MASK*/
#define O32K_OSC_DEP_EN_MASK        (CSL_RTC_GENRAL_CTL_O32K_OSC_DEP_EN_MASK)

/** \brief Read Pend Status*/
#define RD_PEND_MASK                (CSL_RTC_SYNCPEND_RD_PEND_MASK)
/** \brief Write Pend Status*/
#define WR_PEND_MASK                (CSL_RTC_SYNCPEND_WR_PEND_MASK)

/** \brief O32K_CLK_OBS_BIT_MASK*/
#define O32K_CLK_OBS_BIT_MASK       (CSL_RTC_SYNCPEND_O32K_CLK_OBS_MASK)

/** \brief RTC_SW_OFF_MASK*/
#define RTC_SW_OFF_MASK             (CSL_RTC_GENRAL_CTL_SW_OFF_MASK)

/** \brief Unlock State Status*/
#define RTC_UNLOCK_MASK             (CSL_RTC_GENRAL_CTL_UNLOCK_MASK)

/** \brief Wait time for stabalizing OSC*/
#define OSC_LOCK_WAIT               (3U)

/** \brief RTC Kick unlock values*/
#define KICK0_UNLOCK_VALUE          (uint32_t)0x83E70B13U
#define KICK1_UNLOCK_VALUE          (uint32_t)0x95A4F1E0U

/** \brief RTC Kick lock values*/
#define KICK0_LOCK_VALUE (CSL_RTC_KICK0_RESETVAL)
#define KICK1_LOCK_VALUE (CSL_RTC_KICK1_RESETVAL)

/** \brief RTC Base Address*/
#define RTC_BASEADDR                (uint32_t)0x2B1F0000U

/**
 * \anchor RTC_TmrIntFlags
 * \name RTC Timer Interrupt Set/Indication Flags
 *
 *  @{
 */
/** \brief ON_OFF status interrupt */
#define RTC_TMR_INT_INT0_STATUS_FLAG        (CSL_RTC_IRQSTATUS_SYS_EVENT_ON_OFF_MASK)
/** \brief OFF_ON status interrupt */
#define RTC_TMR_INT_INT1_STATUS_FLAG        (CSL_RTC_IRQSTATUS_SYS_EVENT_OFF_ON_MASK)

/** \brief ON_OFF clear interrupt */
#define RTC_TMR_INT_INT0_CLEAR_FLAG        (CSL_RTC_IRQENABLE_CLR_SYS_EN_EVENT_ON_OFF_MASK)
/** \brief OFF_ON clear interrupt */
#define RTC_TMR_INT_INT1_CLEAR_FLAG        (CSL_RTC_IRQENABLE_CLR_SYS_EN_EVENT_OFF_ON_MASK)

/** \brief ON_OFF set interrupt */
#define RTC_TMR_INT_INT0_SET_FLAG        (CSL_RTC_IRQENABLE_SET_SYS_EN_EVENT_ON_OFF_MASK)
/** \brief OFF_ON set interrupt */
#define RTC_TMR_INT_INT1_SET_FLAG        (CSL_RTC_IRQENABLE_SET_SYS_EN_EVENT_OFF_ON_MASK)

/** \brief ON_OFF raw status interrupt */
#define RTC_TMR_INT_INT0_RAW_STATUS_FLAG        (CSL_RTC_IRQSTATUS_RAW_SYS_EVENT_ON_OFF_MASK)
/** \brief OFF_ON raw status interrupt */
#define RTC_TMR_INT_INT1_RAW_STATUS_FLAG        (CSL_RTC_IRQSTATUS_RAW_SYS_EVENT_OFF_ON_MASK)

/** \brief All interrupt status mask */
#define RTC_TMR_INT_STATUS_ALL       (RTC_TMR_INT_INT0_STATUS_FLAG |   \
                                      RTC_TMR_INT_INT1_STATUS_FLAG)
/** \brief All interrupt clear mask */
#define RTC_TMR_INT_CLEAR_ALL       (RTC_TMR_INT_INT0_CLEAR_FLAG |   \
                                     RTC_TMR_INT_INT1_CLEAR_FLAG)
/** \brief All interrupt set mask */
#define RTC_TMR_INT_REQ_ALL          (RTC_TMR_INT_INT0_SET_FLAG |   \
                                      RTC_TMR_INT_INT1_SET_FLAG)
/** \brief All interrupt raw status mask */
#define RTC_TMR_INT_RAW_STATUS_ALL       (RTC_TMR_INT_INT0_RAW_STATUS_FLAG |   \
                                          RTC_TMR_INT_INT1_RAW_STATUS_FLAG)
/** @} */

/* ========================================================================== */
/*                          Structure Declarations                            */
/* ========================================================================== */

/**
 *  \brief A handle that is returned from a RTC_open() call.
 */
typedef struct RTC_Config_s     *RTC_Handle;

/* ========================================================================== */
/*                      Function pointers Declarations                        */
/* ========================================================================== */

/**
 *  \brief  The definition of a ON_OFF callback function used by
 *  the RTC driver
 *
 *  The callback will be called if the ON_OFF interrupt is enabled.
 *
 *  \param  handle              RTC_Handle
 */
typedef void (*RTC_OnOffCallbackFxn) (RTC_Handle handle);

/**
 *  \brief  The definition of a OFF_ON callback function used by
 *  the RTC driver
 *
 *  *  The callback will be called if the OFF_ON interrupt is enabled.
 *
 *  \param  handle              RTC_Handle
 */
typedef void (*RTC_OffOnCallbackFxn) (RTC_Handle handle);

/* ========================================================================== */
/*                          Structure Declarations                            */
/* ========================================================================== */

/**
 *  \brief   RTC Hardware attributes
 */
typedef struct  RTC_HwAttrs_s
{

/** RTC Peripheral base address */
    uint32_t                    baseAddr;
/** RTC Peripheral interrupt vector */
    uint32_t                    intNum;
/** RTC Peripheral event id*/
    uint16_t                    eventId;
/** RTC input clock */
    uint32_t                    inputClk;
/** Enable Interrupt */
    bool                        enableIntr;
/** Interrupt Priority */
    uint8_t                     intrPriority;

}  RTC_HwAttrs;

/**
 *  \brief   RTC Parameters
 *
 *   RTC parameters are used to with the  RTC_open() call. Default values
 *  for these parameters are set using  RTC_paramsInit().
 */
typedef struct  RTC_Params_s
{

/** RTC freeze Mode Enabled/Disabled */
    uint32_t                        isfreezemode;
/** RTC On Off Callback */
    bool                            onOffCallback;
/** RTC Off On Callback */
    bool                            offOnCallback;

}  RTC_Params;

/**
 *  \brief  RTC driver object
 */
typedef struct  RTC_Object_s
{
/** Grants exclusive access to this RTC Instance */
    SemaphoreP_Object           mutex;
/** HwiP object */
    HwiP_Object                 hwiObj;
/** Flag to indicate whether the instance is opened already */
    bool                        isOpen;
/**  RTC open parameters as provided by user */
    RTC_Params                  RTC_params;

/**
 * @name RTC Interrupt Callback Functions
 */

/** Callback Function Pointer for ON_OFF Interrupt */
    RTC_OnOffCallbackFxn         On_OffCallbackFunction;
/** Callback Function Pointer for OFF_ON Interrupt */
    RTC_OffOnCallbackFxn         Off_OnCallbackFunction;

}  RTC_Object;

/**
 *  \brief  RTC Global Configuration
 *
 *  The  RTC_Config structure contains a set of pointers used to characterise
 *  the  RTC driver implementation.
 *
 *  This structure needs to be defined before calling  RTC_init() and it must
 *  not be changed thereafter.
 */
typedef struct  RTC_Config_s
{
/** Pointer to a driver specific data object */
    RTC_Object                      *object;
/** Pointer to a driver specific hardware attributes structure */
    RTC_HwAttrs const               *hwAttrs;
}  RTC_Config;

/**
 *  \brief  RTC Time Structure
 */
typedef struct RTC_Time_s
{
/** Time in Seconds */
    uint32_t            second;
/** Time in minute */
    uint32_t            minute;
/** Time in hour */
    uint32_t            hour;
/** Time in day */
    uint32_t            day;
/** Time in month */
    uint32_t            month;
/** Time in year */
    uint32_t            year;
}  RTC_Time;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Initialize the RTC module.
 */
void RTC_init(void);

/**
 * \brief De-initialize the RTC module.
 */
void RTC_deinit(void);

/**
 * \brief Function to set default values of RTC_Params in params
 *
 * \param params [IN] pointer to the structure to be initialized
 */
void RTC_paramsInit(RTC_Params *params);

/**
 * \brief Open the RTC at index idx with parameters params
 *
 * \param idx [IN] Index of RTC to open in global config
 * \param params [IN] RTC_Params values to use for opening
 *
 * \return RTC_Handle
 */
RTC_Handle RTC_open(uint32_t idx, const RTC_Params *params);

/**
 * \brief Function to close the RTC Peripheral specified by the handle
 *
 * \pre #RTC_open() has to be called first
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 *
 * \sa #RTC_open()
 */
void RTC_close(RTC_Handle handle);

/**
 * \brief Set the RTC time
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param rtc_time [IN] Pointer to RTC_Time structure
 *
 * \return RTC_StatusCode
 */
int32_t RTC_setTime(RTC_Handle handle, const RTC_Time *rtc_time);

/**
 * \brief Get the current RTC time
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param rtc_time [OUT] Pointer to RTC_Time structure
 *
 * \return RTC_StatusCode
 */
int32_t RTC_getTime(RTC_Handle handle, RTC_Time *rtc_time);

/**
 * \brief Set the off-on timer event / Use as timer 1 for RTC
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param rtc_time [IN] Pointer to RTC_Time structure
 *
 * \return RTC_StatusCode
 */
int32_t RTC_setOff_OnTimerEvent(RTC_Handle handle, const RTC_Time *rtc_time);

/**
 * \brief Set the on-off timer event / Use as timer 2 for RTC
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param rtc_time [IN] Pointer to RTC_Time structure
 *
 * \return RTC_StatusCode
 */
int32_t RTC_setOn_OffTimerEvent(RTC_Handle handle, const RTC_Time *rtc_time);

/**
 * \brief Read a value from a specific RTC scratch register
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param scratchIndex [IN] Index of scratch register to read
 *
 * \return Value from the specified scratch register
 */
uint32_t RTC_readScratchRegister(RTC_Handle handle, uint32_t scratchIndex);

/**
 * \brief Write a value to a specific RTC scratch register
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param scratchIndex [IN] Index of scratch register to write
 * \param value [IN] Value to write to the scratch register
 *
 * \return RTC_StatusCode
 */
int32_t RTC_writeScratchRegister(RTC_Handle handle, uint32_t scratchIndex, uint32_t value);

/**
 * \brief Calculate Elapsed Time
 *
 * \param startTime [IN] #RTC_Time structure having start time
 * \param endTime   [IN] #RTC_Time structure having end time
 */
RTC_Time RTC_calculateElapsedTime(const RTC_Time *startTime, const RTC_Time *endTime);

/**
 * \brief Calculate Elapsed Time
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 * \param measuredDriftInSeconds   [IN] Drift Value to be adjusted in Seconds
 *
 * \return RTC_StatusCode
 */
int32_t RTC_adjustForDrift(RTC_Handle handle, Float32 measuredDriftInSeconds);

/**
 * \brief Stop the RTC
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 *
 * \return RTC_StatusCode
 */
int32_t RTC_stop(RTC_Handle handle);

/**
 * \brief Start the RTC after it's stopped
 *
 * \param handle [IN] #RTC_Handle returned from RTC_open()
 *
 * \return RTC_StatusCode
 */
int32_t RTC_start(RTC_Handle handle);

/**
 * \brief RTC Erratum i2327 Workaround
 *
 * Due to a bug in initial synchronization out of cold power on,
 * IRQ status can get locked infinitely if we do not:
 * a) unlock RTC
 *
 * This workaround *must* be applied within 1 second of power on,
 * So, this is closest point to be able to guarantee the max
 * timing.
 *
 */
void RTC_erratumi2327Init(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef RTC_H_ */

/** @} */
