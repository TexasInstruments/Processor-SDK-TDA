/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Sony Semiconductor Solutions Corporation
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
/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ISS_SENSOR_IMX728_H_
#define ISS_SENSOR_IMX728_H_
    
#include <iss_sensors.h>
#include <iss_sensor_priv.h>
#include <iss_sensor_if.h>
#include <iss_sensor_serdes.h>
/**< ISS AEWB plugin is included here to get the default AEWB configuration
     from each sensor */
    
#define IMX728_OUT_WIDTH   (3840)
#define IMX728_OUT_HEIGHT  (2160)
#define IMX728_META_HEIGHT_AFTER (0)
    
#include "imx728_gain_table.h"
    
/*******************************************************************************
 *  Defines
 *******************************************************************************
    */
    
#define ISS_SENSOR_IMX728_DEFAULT_EXP_RATIO     (128U)
    
#define ISS_SENSOR_IMX728_FEATURES ((uint32_t)ISS_SENSOR_FEATURE_MANUAL_EXPOSURE    |\
                                    (uint32_t)ISS_SENSOR_FEATURE_MANUAL_GAIN        |\
                                    (uint32_t)ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE |\
                                    (uint32_t)ISS_SENSOR_FEATURE_CFG_UC1            |\
                                    (uint32_t)ISS_SENSOR_FEATURE_DCC_SUPPORTED)
    
#define IMX728_REG_UPDATE_ADDR                  (0x95C6U)
#define IMX728_REG_UPDATE                       (0x01U)
#define IMX728_VMAX_REG_ADDR                    (0x6140U)
#define IMX728_SP1_EXPOSURE_TIME_FID0_ADDR      (0xBF08U)
#define IMX728_SP2_EXPOSURE_TIME_FID0_ADDR      (0xBF0CU)
#define IMX728_FME_ISP_GAIN_FID0_ADDR           (0xBF04U)

#define IMX728_NV_STATUS_REG_ADDR               (0x60AEU)

#define IMX728_I2C_ACK_RESPONSE_ADDR            (0x8002U)
#define IMX728_I2C_ACK_RESPONSE                 (0x01U)
#define IMX728_CK_INCK_FREQ_I2C_EN_ADDR         (0x1B1CU)
#define IMX728_CK_INCK_FREQ_EN                  (0x01U)
#define IMX728_CK_INCK_FREQ_I2C_ADDR            (0x1B20U)
#define IMX728_CK_INCK_FREQ_18MHZ               (0x18U)
#define IMX728_CK_SLEEP_H_ADDR                  (0x1B05U)
#define IMX728_CK_SLEEP_H_TO_STANDBY	        (0xFFU)
#define IMX728_MODE_SEL_ADDR                    (0x9728U)
#define IMX728_CK_STRM_H_ADDR                   (0x1B04U)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
    */
    
struct {
    
    uint32_t                    maxCoarseIntgTime;
    /**< Max Coarse integration time in milliseconds supported by sensor */
    uint32_t                    lineIntgTime;
    /**< Line Integration time in microseconds */
    uint32_t                    pixIntgTime;
    /**< Pixel Integration time in microseconds  */
} gImx728SonyDeviceObj;
    
/*******************************************************************************
 *  Local Functions Declarations
 *******************************************************************************
    */
    
static int32_t IMX728_Probe(uint32_t chId, void *pSensorHdl);
static int32_t IMX728_Config(uint32_t chId, void *pSensorHdl, uint32_t sensorFeaturesRequested);
static int32_t IMX728_StreamOn(uint32_t chId, void *pSensorHdl);
static int32_t IMX728_StreamOff(uint32_t chId, void *pSensorHdl);
static int32_t IMX728_PowerOn(uint32_t chId, void *pSensorHdl);
static int32_t IMX728_PowerOff(uint32_t chId, void *pSensorHdl);
static int32_t IMX728_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms);
static int32_t IMX728_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms);
static int32_t IMX728_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms);
static void IMX728_InitAewbConfig(uint32_t chId, void *pSensorHdl);
static void IMX728_GetIspConfig (uint32_t chId, void *pSensorHdl);
static void IMX728_deinit (uint32_t chId, void *pSensorHdl);
static int32_t IMX728_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg);
static int32_t IMX728_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms);
    
    
static int32_t IMX728_ReadReg(uint8_t      i2cInstId,
                                uint8_t      i2cAddr,
                                uint16_t     regAddr,
                                uint8_t      *regVal,
                                uint32_t     numRegs,
                                uint8_t      skip_error);
    
static int32_t IMX728_WriteReg(uint8_t    i2cInstId,
                               uint8_t    i2cAddr,
                               uint16_t   regAddr,
                               uint8_t    *regVal,
                               uint32_t   numRegs,
                               uint8_t    skip_error);
    
#endif // ISS_SENSOR_IMX728_H_

