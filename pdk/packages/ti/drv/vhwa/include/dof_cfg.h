/*
 *   Copyright (c) Texas Instruments Incorporated 2018
 *   All rights reserved.
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
 *
 */

/**
 *  \ingroup  DRV_DOF_MODULE
 *  \defgroup DRV_DOF_MODULE_CFG DOF Configuration
 *            This is DOF Configuration file
 *
 *  @{
 */
/**
 *  \file dof_cfg.h
 *
 *  \brief  Defines the structures / control operations that could be used to
 *          configure / control DOF of DMPAC
 */

#ifndef DMPAC_CFG_DOF_H_
#define DMPAC_CFG_DOF_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <string.h>
#include <vhwa_dmpac_abstraction_layer.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Number of decesion tree parameters for DOF */
#define DOF_NUM_DECISION_TREES          (16U)

/** \brief Max number of entries supported in histogram */
#define DOF_MAX_HISTOGRAM_SIZE          (16U)

/** \brief Number of decesion tree parameters for DOF */
#define DOF_MAX_PYR_LVL_SUPPORTED        (7U)

/**
 *  \anchor DOF_SupportedResolution
 *  \name   DOF Supported Resolution
 *  \brief  Maximum and minimum supported image resolution by DOF
 *
 *  @{
 */
#define DOF_MINIMUM_WIDTH               (32U)
#define DOF_MINIMUM_HEIGHT              (16U)
#define DOF_MAXIMUM_WIDTH               (2048U)
#define DOF_MAXIMUM_HEIGHT              (1024U)
/** @} */

/**
 *  \anchor DOF_SupportedSR
 *  \name   DOF Supported Search range
 *  \brief  Supported horizontal and vertical search range
 *
 *  @{
 */
#define DOF_MAXIMUM_HSR               (191U)
#define DOF_MAXIMUM_VSR               (62U)
#define DOF_MAXIMUM_TVSR_WTIH_191_HSR (112U)
#define DOF_MAXIMUM_HSR_WTIH_124_VSR  (170U)
/** @} */

/**
 *  \anchor DOF_Predictors
 *  \name   DOF Predictors
 *  \brief  Predictors available for DOF
 *
 *  @{
 */
/** \brief No Predictor used */
#define DOF_PREDICTOR_NONE                  (0U)

/** \brief Delayed left predictor */
#define DOF_PREDICTOR_DELEY_LEFT            (1U)

/** \brief Temporal predictor, Need flow vector output from previous image pair
           as Temporal input, can only be set for base layer
 */
#define DOF_PREDICTOR_TEMPORAL              (2U)

/** \brief Pyramidal left predictor, can be set for base and intermediate layers
 */
#define DOF_PREDICTOR_PYR_LEFT              (3U)

/** \brief Pyramidal colocated predictor, can be set for base and intermediate
           layers
 */
#define DOF_PREDICTOR_PYR_COLOCATED         (4U)
/** @} */

/**
 *  \anchor Dof_ErrorEvents
 *  \name   DOF Error Events
 *  \brief  Macro's to define DOF Error events, for which DOF
 *          callback could be registered.
 *
 *          Caution: These macro values are directly used by driver
 *          for enabling events.
 *  @{
 */
/**< \brief Error on DOF VBUSM Read interface */
#define VHWA_DOF_RD_ERR                         (0x04U)
/**< \brief Error on DOF SL2 VBSUM Write interface */
#define VHWA_DOF_WR_ERR                         (0x08U)
/**< \brief Error on DOF MP0 Read Status Error */
#define VHWA_DOF_MP0_RD_STATUS_ERR              (0x10U)
/**< \brief Error on FOCO0 SL2 VBSUM Write interface */
#define VHWA_DOF_FOCO0_SL2_WR_ERR               (0x400000U)
/**< \brief Error on FOCO0 VBUSM Read interface */
#define VHWA_DOF_FOCO0_VBUSM_RD_ERR             (0x200000U)
/**< \brief Bitmask for all DOF Error Events */
#define VHWA_DOF_ALL_ERR_EVENTS                                                \
  (VHWA_DOF_RD_ERR | VHWA_DOF_WR_ERR | VHWA_DOF_MP0_RD_STATUS_ERR |            \
   VHWA_DOF_FOCO0_SL2_WR_ERR | VHWA_DOF_FOCO0_VBUSM_RD_ERR)
/**< \brief Watchdog timer Error on DOF */
#define VHWA_DOF_WDTIMER_ERR                    (0x01U)
/**< \brief Watchdog timer Error on FOCO0 */
#define VHWA_DOF_FOCO0_WDTIMER_ERR              (0x04U)
/**< \brief Bitmask for all DOF Watchdog Timer Error Events */
#define VHWA_DOF_ALL_WDTIMER_ERR_EVENTS                                        \
  (VHWA_DOF_WDTIMER_ERR | VHWA_DOF_FOCO0_WDTIMER_ERR)
/** @} */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/**
 * \brief Prototype for the Error Event for DOF.
 *        The callback for the DOF error events can be registered
 *        using #Dof_ErrEventParams. One of the parameter in this
 *        is call back. Driver calls this callback when error occurs.
 *
 * \param handle                FVID2 driver handle, for which error
 *                              has occurred.
 * \param errEvents             Error Events occured,
 *                              Refer to \ref Dof_ErrorEvents for valid values.
 *
 * \return None.
 */
typedef void (*Dof_ErrEventCbFxn)(Fvid2_Handle handle, uint32_t errEvents,
    void *appData);

/**
 * \brief Prototype for the Watchdog Timeout Error Event for DOF.
 *        The callback for the DOF Watchdog Timeout error events can be
 *        registered using #Dof_WdTimerErrEventParams. One of the parameter in this
 *        is call back. Driver calls this callback when error occurs.
 *
 * \param handle                FVID2 driver handle, for which error
 *                              has occurred.
 * \param errEvents             Error Events occured,
 *                              Refer to \ref Dof_ErrorEvents for valid values.
 *
 * \return None.
 */
typedef void (*Dof_WdTimerErrEventCbFxn)(Fvid2_Handle handle, uint32_t errEvents,
    void *appData);

/**
 * \brief Prototype for the config Register Memory Compare Event for DOF.
 *        The callback for the DOF register memory compare events can be registered
 *        using #Dof_ConfigRegMemParams. One of the parameters in this
 *        is the callback. Driver calls this callback when the event occurs.
 *
 * \param handle                FVID2 driver handle, for which the event
 *                              has occurred.
 * \param appData               Application-specific data pointer.
 *
 * \return None.
 */
typedef int32_t (*Dof_ConfigRegCompareCbFxn)(Fvid2_Handle handle, void *configRegPrms);

/**
 *  struct Dof_Config
 *  \brief Connfiguration parameters of DMPAC DOF.
 */
typedef struct
{
  uint32_t enableDOF;
  /**<Set to 0x1 to enable
   * DOF engine.
   */
  uint32_t width;
  /**< Width of frame (in pixel) to be processed by Optical Flow engine.
   * 2048 pixel max and 32 pixels min.
   */
  uint32_t height;
  /**< Height of frame (in pixel) to be processed by Optical Flow engine.
   * 1024 pixel max and 16 pixels min.
   */
  uint32_t horizontalSearchRange;
  /**<Horizontal Search Range in pixel in both the directions.
   */
  uint32_t topSearchRange;
  /**<Negative or Upward direction Vertical Search Range in pixel.
   */
  uint32_t bottomSearchRange;
  /**<Positive or Downward direction Vertical Search Range in pixel.
   */
  uint32_t pyramidalTopColPred;
  /**<Pyramidal Top Co-located Predictor Enable bit. Set to 0x1 to
   * enable use of pyramidal top colocated predictor during DOF processing.
   */
  uint32_t pyramidalTopLeftPred;

  /**<Pyramidal Top Left Predictor Enable bit. Set to 0x1 to enable
   * use of pyramidal top left predictor during DOF processing.
   */
  uint32_t temporalPred;
  /**<Temporal Predictor Enable bit. Set to 0x1 to
   * enable use of temporal predictor during the DOF processing.
   */
  uint32_t delayedLeftPred;

  /**<Delayed Left Predictor Enable bit. Set to 0x1 to enable
   * use of delayed left predictor during the DOF processing.
   */
  uint32_t lkConfidanceScore;
  /**<LK refinement and confidence score generation enable bit.
   * Set to 0x1 to enable LK and CS processing.
   */
  uint32_t medianFilter;
  /**<Median filter enable bit. Set to 0x1 to enable median
   * filter processing.
   */
  uint32_t enableSof;
  /**<sparse optical flow enable bit. Set to 0x1 to enable
   * sparse optical flow processing.
   */
  uint32_t currentCensusTransform;
  /**<Census transform type for current image.
  * 0: 24 bits generated for census transform by using a
  * 5x5 neighborhood around the pixel.
  * 1: 32 bits generated for census transform by using a 7x7 neighborhood
  * around the pixel where only 8 pixels are used from the periphery.
  */
  uint32_t referenceCensusTransform;
  /**<Census transform type for reference image.
  * 0: 24 bits generated for census transform by using a
  * 5x5 neighborhood around the pixel.
  * 1: 32 bits generated for census transform by using a 7x7 neighborhood
  * around the pixel where only 8 pixels are used from the periphery.
  */
  uint32_t maxMVsInSof;
  /**<Maximum number of MV output per line in case of
   * sparse optical flow processing.
   */
  uint32_t motionSmoothnessFactor;
  /**<Motion smoothness factor.
   */
  uint32_t iirFilterAlpha;
  /**<Coefficient for IIR filter used for smoothing
   * horizontal flow vector gradient. The usage can be illustrated as:
   * SmoothU(i,j)= [ U(i,j)*alpha + SmoothU(i-1,j)*beta + round ]>>8
   * where beta = 256-alpha
   * round = 128
   */
} Dof_Config;

/**
 *  struct Dof_DecisionTree
 *  \brief Decision Tree Connfiguration parameters of DOF.
 */
typedef struct
{
  uint32_t index[3];
  /**<index value for Confidence Score Decision Tree.
   */
  uint32_t threshold[3];
  /**<Threshold value for Confidence Score Decision Tree.
   */
  uint32_t weight[4];
  /**<Weigts value for Confidence Score Decision Tree.
   */
} Dof_DecisionTree;

/**
 *  struct Dof_ConfScoreParam
 *  \brief Confidance Score parameters of DMPAC DOF.
 */
typedef struct
{
  uint32_t confidanceScoreGain ;
  /**<Multiplier factor (Gain) for the combined confidence score.
   * The sum of individual scores from different decision trees are
   * multiplied by CS Gain before scaling to final 4bit (16 levels)
   * confidence score value.
   */
  Dof_DecisionTree decisionTree[DOF_NUM_DECISION_TREES];
  /**< Decision Tree Connfiguration parameters of DMPAC DOF
  */
} Dof_ConfScoreParam;

/**
 *  struct Dof_ErrEventParams
 *  \brief Structure for error event parameters
 *         Used to register callback for the given set of events.
 */
typedef struct
{
    uint32_t                    errEvents;
    /**< bitmask of error events,
     *   Multiple error events can be registered with the same callback,
     *   Driver returns bitmask of events, which has occured,
     *   Refer to \ref Dof_ErrorEvents for valid values. */
    Dof_ErrEventCbFxn           cbFxn;
    /**< Callback function to be call on error events */
    void                       *appData;
    /**< private data */
} Dof_ErrEventParams;

/**
 *  struct Dof_WdTimerErrEventParams
 *  \brief WD Timer parameters
 *         Used to configure WD Timer parameters
 *         Must be called after SET_PARAMS ioctl
 *         Used with IOCTL_VHWA_M2M_DOF_CONFIG_WDTIMER ioctl
 */
typedef struct
{
    uint32_t                    wdTimeoutCycles;
    /**< Used to select timeout cycles for DOF
     *   Refer to #VHWA_WdTimeOutCycles for valid values */
    uint32_t                    focoWdTimeoutCycles;
    /**< Used to select timeout cycles for FOCO
     *   Refer to #VHWA_WdTimeOutCycles for valid values */
    uint32_t                    wdTimerErrEvents;
    /**< bitmask of watchdog timer error events,
     *   VHWA_DOF_WDTIMER_ERR and VHWA_DOF_FOCO0_WDTIMER_ERR
     *   watchdog timer error events
     *   can be registered with the same callback,
     *   Driver returns bitmask of events, which has occured,
     *   Refer to \ref Dof_ErrorEvents for valid values. */
    Dof_WdTimerErrEventCbFxn    cbFxn;
    /**< Callback function to be call on error events */
    void                       *appData;
    /**< private data */
} Dof_WdTimerErrEventParams;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/**
 *  \brief This function should be used to initialize variable of type
 *          #Dof_Config.
 *
 *  \param dofCfg   A pointer of type Dof_Config
 *
 *  \return         None
 */
static inline void DofCfg_init(Dof_Config *dofCfg);

/**
 *  \brief This function should be used to initialize variable of type
 *          #Dof_ConfScoreParam.
 *
 *  \param dofCfg   A pointer of type Dof_ConfScoreParam
 *
 *  \return         None
 */
static inline void Dof_DecisionTreesInit(Dof_ConfScoreParam *dofCSParam);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static inline void DofCfg_init(Dof_Config *dofCfg)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != dofCfg)
    /* LDRA_JUSTIFY_END */
    {
        (void)memset(dofCfg, 0, sizeof(Dof_Config));
    }

    return;
}

static inline void Dof_DecisionTreesInit(Dof_ConfScoreParam *dofCSParam)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != dofCSParam)
    /* LDRA_JUSTIFY_END */
    {
        (void)memset(dofCSParam, 0, sizeof(Dof_ConfScoreParam));
    }

    return;
}


#ifdef __cplusplus
}
#endif

#endif /* DMPAC_CFG_DOF_H_ */

/** @} */
