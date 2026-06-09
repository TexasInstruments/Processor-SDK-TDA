/*
 *  Copyright (c) Texas Instruments Incorporated 2024
 *  All rights reserved.
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
 *  \defgroup SAFETY_CHECKERS Safety checkers
 */

 /**
 *  \ingroup  SAFETY_CHECKERS
 *  \defgroup CSIRX_SAFETY_CHECKERS CSIRX Safety Checkers Library
 *
 *  @{
 */
/**
 *  \ingroup  CSIRX_SAFETY_CHECKERS
 *  \defgroup CSIRX_SAFETY_CHECKERS_INTERFACE CSIRX Safety Checkers Library Interface
 *
 *  @{
 */
/**
 *  \file     safety_checkers_csirx.h
 *
 *  \brief    This file contains CSIRX safety checkers library interfaces and related data structures.
 *
 */

#ifndef SAFETY_CHECKERS_CSIRX_H_
#define SAFETY_CHECKERS_CSIRX_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include "safety_checkers_csirx_soc.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Structure to hold the base address and the length of a instance of
 *        CSIRX module
 */
typedef struct
{
    uint32_t instance;
    /**< CSIRX instance number*/
    uint32_t baseAddr;
    /**< Base address of CSIRX module */
    uint32_t *regOffsetArr;
    /**< Pointer to the register offset array for each of the register type */
    uint32_t length;
    /**< Total number of registers for each register type  */
} SafetyCheckers_CsirxInstData;

/**
 * \brief Structure to hold the register type and instance info of the CSIRX
 *        module registers
 */
typedef struct
{
    uint32_t regType;
    /**< Type of CSIRX module registers */
    SafetyCheckers_CsirxInstData instData[SAFETY_CHECKERS_CSIRX_INSTANCES_MAX];
    /**< Instance of CSIRX module registers */
} SafetyCheckers_CsirxRegData;

/**
 *  \brief Structure to store driver information.
 */
typedef struct
{
    const Fvid2_DrvOps *drvOps;
    /**< Driver operation table pointer. */
    uint32_t numOpens;
    /**< Number of times the driver is opened using create API. */
    uint32_t isUsed;
    /**< Flag indicating whether the object is used or not. */
} SafetyCheckers_CsirxFdmDriver;

/**
 *  \brief Structure to store channel information.
 */
typedef struct
{
    SafetyCheckers_CsirxFdmDriver *drv;
    /**< Pointer to the driver object to which this channel is created. */
    Fdrv_Handle drvHandle;
    /**< Driver handle returned by the actual driver. */
    Fvid2_CbParams cbParams;
    /**< Application call back parameters. */
    uint32_t isUsed;
    /**< Flag indicating whether the object is used or not. */
} SafetyCheckers_CsirxFdmChannel;

/**
 *  \brief Structure to store data format and string pair.
 */
typedef struct
{
    uint32_t dataFmt;
    /**< Data format. Refer \ref Fvid2_DataFormat*/
    const char *dataFmtStr;
    /**< Pointer to data format string. */
} SafetyCheckers_CsirxFdmDataFmtString;

/**
 *  struct Fdm_StdString
 *  \brief Structure to store standard and string pair.
 */
typedef struct
{
    uint32_t standard;
    /**< Standard. Refer \ref Fvid2_Standard */
    const char *stdStr;
    /**< Pointer to data format string. */
} SafetyCheckers_CsirxFdmStdString;

/**
 *  \anchor SafetyCheckers_CsirxRegType
 *  \name Type of CSIRX module registers
 *
 *  @{
 */
#define SAFETY_CHECKERS_CSIRX_REG_TYPE_STRM_CTRL                    (0x0U)
#define SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_CONFIG                  (0x1U)
#define SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_PLL                     (0x2U)
#define SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_LANE_CONFIG             (0x3U)
#define SAFETY_CHECKERS_CSIRX_REG_TYPE_VIRTUAL_CHANNEL              (0x4U)
#define SAFETY_CHECKERS_CSIRX_REG_TYPE_DATATYPE_FRAMESIZE           (0x5U)
#define SAFETY_CHECKERS_CSIRX_NUM_REGTYPE_MAX                       (0x6U)
/** @} */
#define SAFETY_CHECKERS_CSIRX_MAX_FRAME_SIZE                        (((uint32_t)2.5*1024U*1024U*1024U)/2U)
#define SAFETY_CHECKERS_CSIRX_EVENT_INIT_DONE                       (0x1U)
#define SAFETY_CHECKERS_CSIRX_SENSOR_CFG_END_MARKER                 (0xFFFU)

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \ingroup CSIRX_SAFETY_CHECKERS_INTERFACE
 *
 * \defgroup SAFETY_CHECKERS_CSIRX_API  SAFETY_CHECKERS_CSIRX API
 *
 *  @{
 */

/**
 *  \brief Function to get register configuration of requested type
 *
 *  \param regCfg   Pointer to register configuration
 *  \param regType  Type of register configuration
 *  \param instance CSIRX instance ID
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *
 */
int32_t SafetyCheckers_csirxGetRegCfg(uintptr_t *regCfg,
                                      uint32_t regType,
                                      uint32_t instance);

/**
 *  \brief Function to verify register configuration of requested type
 *
 *  \param regCfg   Pointer to register configuration
 *  \param regType  Type of register configuration
 *  \param instance CSIRX instance ID
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *          SAFETY_CHECKERS_REG_DATA_MISMATCH  if verification is failed
 *
 */
int32_t SafetyCheckers_csirxVerifyRegCfg(const uintptr_t *regCfg,
                                         uint32_t regType,
                                         uint32_t instance);

/**
 *  \brief Function to verify if requested configuration is within CSIRX IP
 *  limits
 *
 *  \param drvHandle Fvid2 driver handle
 *  \param fps       Requested fps (frames per second)
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *
 */
int32_t SafetyCheckers_csirxVerifyCsiAvailBandwidth(void *drvHandle, uint32_t fps);

/**
 *  \brief Function to get vim register configuration
 *
 *  \param drvHandle  Fvid2 driver handle
 *  \param vimCfg     Pointer to vim register configuration
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *
 */
int32_t SafetyCheckers_csirxGetVimCfg(void *drvHandle,
                                      SafetyCheckers_CsirxVimCfg *vimCfg);

/**
 *  \brief Function to verify vim register configuration
 *
 *  \param drvHandle  Fvid2 driver handle
 *  \param vimCfg     Pointer to vim register configuration
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *          SAFETY_CHECKERS_REG_DATA_MISMATCH  if verification is failed
 *
 */
int32_t SafetyCheckers_csirxVerifyVimCfg(void *drvHandle, SafetyCheckers_CsirxVimCfg *vimCfg);

/**
 *  \brief Function to get sensor configuration
 *
 *  \param i2cHandle  I2C driver handle to access sensor
 *  \param slaveAddr  I2C slave address of sensor
 *  \param regData    Pointer to sensor configuration
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *
 */
int32_t  SafetyCheckers_csirxGetSensorCfg(void *i2cHandle,
                                          uint32_t slaveAddr,
                                          uint16_t (*regData)[3]);

/**
 *  \brief Function to verify sensor configuration
 *
 *  \param handle     I2C driver handle to access sensor
 *  \param slaveAddr  I2C slave address of sensor
 *  \param regData    Pointer to sensor configuration
 *
 *  \return SAFETY_CHECKERS_SOK if successful
 *          SAFETY_CHECKERS_FAIL if NULL params are passed
 *          SAFETY_CHECKERS_REG_DATA_MISMATCH  if verification is failed
 *
 */
int32_t  SafetyCheckers_csirxVerifySensorCfg(void *handle,
                                             uint32_t slaveAddr,
                                             uint16_t (*regData)[3]);

/** @} */
/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SAFETY_CHECKERS_CSIRX_H_ */

/** @} */
/** @} */
