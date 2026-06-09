/*
* The MIT License (MIT)
*
* Copyright (C) 2024 Define Design Deploy Corp.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the “Software”), to deal in
* the Software without restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
* Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

/*!
 * @file        iss_sensor_imx390_discovery.h
 * @brief       Data structures, method interfaces and definitions common to different types of cameras
 * @details     ****************************************************************** 
 * @copyright   Copyright (C) 2024 Define Design Deploy Corp. All Rights Reserved.
 */
#ifndef ISS_SENSORS_IMX390_DISCOVERY_H_
#define ISS_SENSORS_IMX390_DISCOVERY_H_

#include <iss_sensors.h>
#include <iss_sensor_priv.h>
#include <iss_sensor_if.h>
#include <iss_sensor_serdes.h>
/**< ISS AEWB plugin is included here to get the default AEWB configuration
     from each sensor */

#define IMX390_OUT_WIDTH   (1936)
#define IMX390_OUT_HEIGHT  (1100)
#define IMX390_META_HEIGHT_AFTER (4)

#include "imx390_discovery_linear_1920x1080_config.h"
#include "imx390_discovery_wdr_config.h"
#include "imx390_discovery_wdr_config_60fps.h"
#include "imx390_discovery_gain_table.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
#define IMX390_CHIP_ID_REG_ADDR               (0x0330)
#define IMX390_CHIP_ID_REG_VAL                (0x15)

#define ISS_SENSOR_IMX390_DEFAULT_EXP_RATIO     (128U)

#define ISS_SENSOR_IMX390_DISCOVERY_FEATURES    ((uint32_t)ISS_SENSOR_FEATURE_MANUAL_EXPOSURE   |\
                                                (uint32_t)ISS_SENSOR_FEATURE_MANUAL_GAIN        |\
                                                (uint32_t)ISS_SENSOR_FEATURE_LINEAR_MODE        |\
                                                (uint32_t)ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE |\
                                                (uint32_t)ISS_SENSOR_FEATURE_CFG_UC1            |\
                                                (uint32_t)ISS_SENSOR_FEATURE_DCC_SUPPORTED)

#define ISS_IMX390_GAIN_TBL_STEP_SIZE           (100U)
#define ISS_IMX390_GAIN_TBL_STARTOFFSET         (10U)
#define ISS_IMX390_MAX_INTG_LINES               (2050U)
#define ISS_IMX390_VMAX                         (0x44c)
#define ISS_IMX390_RHS                          (0x85U)

#define IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR         (0x0018U)
#define IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR_HIGH    ( \
        IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR + 1U)
#define IMX390_SP1L_ANALOG_GAIN_CONTROL_REG_ADDR         (0x001AU)
#define IMX390_SP1L_ANALOG_GAIN_CONTROL_REG_ADDR_HIGH    ( \
        IMX390_SP1L_ANALOG_GAIN_CONTROL_REG_ADDR + 1U)

#define IMX390_VMAX                         (0x465U)

#define IMX390_AE_CONTROL_LONG_REG_ADDR_LOW   (0x000C)
#define IMX390_AE_CONTROL_LONG_REG_ADDR_HIGH   ( \
        IMX390_AE_CONTROL_LONG_REG_ADDR_LOW + 1)
#define IMX390_AE_CONTROL_LONG_REG_ADDR_TOP   ( \
        IMX390_AE_CONTROL_LONG_REG_ADDR_HIGH + 1)

#define IMX390_AE_CONTROL_SHORT_REG_ADDR_LOW   (0x0010)
#define IMX390_AE_CONTROL_SHORT_REG_ADDR_HIGH  ( \
        IMX390_AE_CONTROL_SHORT_REG_ADDR_LOW + 1)
#define IMX390_AE_CONTROL_SHORT_REG_ADDR_MSB   ( \
        IMX390_AE_CONTROL_SHORT_REG_ADDR_HIGH + 1)


#define IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR         (0x0018U)
#define IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR_HIGH    ( \
        IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR + 1U)

#define IMX390_SP1L_ANALOG_GAIN_CONTROL_REG_ADDR         (0x001AU)
#define IMX390_SP1L_ANALOG_GAIN_CONTROL_REG_ADDR_HIGH    ( \
        IMX390_SP1L_ANALOG_GAIN_CONTROL_REG_ADDR + 1U)

#define IMX390_RED_GAIN_REG_L                (0x0030U)
#define IMX390_RED_GAIN_REG_H                (0x0031U)
#define IMX390_GREEN1_GAIN_REG_L             (0x0032U)
#define IMX390_GREEN1_GAIN_REG_H             (0x0033U)
#define IMX390_GREEN2_GAIN_REG_L             (0x0034U)
#define IMX390_GREEN2_GAIN_REG_H             (0x0035U)
#define IMX390_BLUE_GAIN_REG_L               (0x0036U)
#define IMX390_BLUE_GAIN_REG_H               (0x0037U)
#define IMX390_ISP_GAIN_OFFSET               (1U)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

struct {

    uint32_t                    maxCoarseIntgTime;
    /**< Max Coarse integration time in milliseconds supported by sensor */
    uint32_t                     lineIntgTime;
    /**< Line Integration time in microseconds */
    uint32_t                     pixIntgTime;
    /**< Pixel Integration time in microseconds  */
} gImx390DeviceObj;

/*******************************************************************************
 *  Local Functions Declarations
 *******************************************************************************
 */

int32_t IMX390_Disco_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested);
int32_t IMX390_Disco_StreamOn(uint32_t chId, void *pSensorHdl);
int32_t IMX390_Disco_StreamOff(uint32_t chId, void *pSensorHdl);
int32_t IMX390_Disco_PowerOn(uint32_t chId, void *pSensorHdl);
int32_t IMX390_Disco_PowerOff(uint32_t chId, void *pSensorHdl);
int32_t IMX390_Disco_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms);
int32_t IMX390_Disco_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms);
int32_t IMX390_Disco_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms);
void IMX390_Disco_InitAewbConfig(uint32_t chId, void *pSensorHdl);
void IMX390_Disco_GetIspConfig (uint32_t chId, void *pSensorHdl);
void IMX390_Disco_Deinit (uint32_t chId, void *pSensorHdl);
int32_t IMX390_Disco_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg);
int32_t IMX390_Disco_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms);
int32_t IMX390_Disco_SetAwbParams(void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms);
int32_t IMX390_Disco_GetWBPrgFxn(uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms);

int32_t IMX390_Disco_WriteReg(uint8_t    i2cInstId,
                               uint8_t    i2cAddr,
                               uint16_t   regAddr,
                               uint8_t    regVal,
                               uint32_t   numRegs);

int32_t IMX390_Disco_ReadReg(uint8_t      i2cInstId,
                              uint8_t      i2cAddr,
                              uint16_t     regAddr,
                              uint8_t      *regVal,
                              uint32_t      numRegs);

#endif /* End of ISS_SENSORS_IMX390_DISCOVERY_H_*/