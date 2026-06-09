/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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
#ifndef ISS_SENSORS_OX05C1S_H_
#define ISS_SENSORS_OX05C1S_H_

#include <iss_sensors.h>
#include <iss_sensor_priv.h>
#include <iss_sensor_if.h>
#include <iss_sensor_serdes.h>

#include "ox05c1s_sensor_config.h"
#include "ox05c1s_gain_table.h"


/*
 *******************************************************************************
 *  Defines
 *******************************************************************************
*/
#define OX05C1S_OUT_WIDTH   (2592)
#define OX05C1S_OUT_HEIGHT  (1944)
#define OX05C1S_META_HEIGHT_BEFORE (0)
#define OX05C1S_META_HEIGHT_AFTER (0)

#define ISS_SENSOR_OX05C1S_FEATURES     ((uint32_t)ISS_SENSOR_FEATURE_MANUAL_EXPOSURE     | \
                                         (uint32_t)ISS_SENSOR_FEATURE_MANUAL_GAIN         | \
                                         (uint32_t)ISS_SENSOR_FEATURE_LINEAR_MODE         | \
                                         (uint32_t)ISS_SENSOR_FEATURE_DCC_SUPPORTED)

/* AE Registers */
/* Exposure Time */
#define AEC_HCG_CTRL_01 0x3501U
#define AEC_HCG_CTRL_02 0x3502U

/* Analog Gain 8 bits (4.4 bits format)*/
/* High Conversion Gain (HCG) */
#define AEC_HCG_CTRL_08 0x3508U /* [3:0] Integer */
#define AEC_HCG_CTRL_09 0x3509U /* [7:4] Fraction */

/* Digital Gain is (4.10 bits format)*/
/* High Conversion Gain (HCG) */
#define AEC_HCG_CTRL_0A 0x350AU /* [3:0] Integer */
#define AEC_HCG_CTRL_0B 0x350BU /* [7:0] Fraction MSB */
#define AEC_HCG_CTRL_0C 0x350CU /* [7:6] Fraction LSB */

/* AWB Registers are in 5.10b format */
/*
     The format is 5.10b, where the integer part is contained in the higher bits
     [6:2] of the first register, while the fractional part is stored in the
     lower bits [1:0] of the first register and continues through all the bits
     [7:0] in the second register
*/
/* B Gain */
/* High Conversion Gain (HCG) */
#define AWB_GAIN_HCG_0 0x5280U
#define AWB_GAIN_HCG_1 0x5281U

/* G Gain */
/* High Conversion Gain (HCG) */
/* Gb */
#define AWB_GAIN_HCG_2 0x5282U
#define AWB_GAIN_HCG_3 0x5283U

/* Gr */
#define AWB_GAIN_HCG_4 0x5284U
#define AWB_GAIN_HCG_5 0x5285U

/* R Gain */
/* High Conversion Gain (HCG) */
#define AWB_GAIN_HCG_6 0x5286U
#define AWB_GAIN_HCG_7 0x5287U

/*
 *******************************************************************************
 *  Local Functions Declarations
 *******************************************************************************
*/
static int32_t OX05C1S_Probe(uint32_t chId, void *pSensorHdl);
static int32_t OX05C1S_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested);
static int32_t OX05C1S_StreamOn(uint32_t chId, void *pSensorHdl);
static int32_t OX05C1S_StreamOff(uint32_t chId, void *pSensorHdl);
static int32_t OX05C1S_PowerOn(uint32_t chId, void *pSensorHdl);
static int32_t OX05C1S_PowerOff(uint32_t chId, void *pSensorHdl);
static int32_t OX05C1S_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms);
static int32_t OX05C1S_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms);
static int32_t OX05C1S_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms);
static void OX05C1S_InitAewbConfig(uint32_t chId, void *pSensorHdl);
static void OX05C1S_GetIspConfig (uint32_t chId, void *pSensorHdl);
static void OX05C1S_deinit (uint32_t chId, void *pSensorHdl);
static int32_t OX05C1S_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg);
static int32_t OX05C1S_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms);
static int32_t OX05C1S_SetAwbParams(void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms);
static int32_t OX05C1S_GetWBPrgFxn(uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms);
static void OX05C1S_print_frame_count(void *pSensorHdl);

//Gang_TODO: I2C bus bit depth may vary
static int32_t OX05C1S_WriteReg(uint8_t         i2cAddr,
                            uint16_t        regAddr,
                            uint8_t         regVal,
                            uint32_t        numRegs);

static int32_t OX05C1S_ReadReg(uint8_t         i2cAddr,
                            uint16_t        regAddr,
                            uint8_t         *regVal,
                            uint32_t        numRegs);

#endif /* End of ISS_SENSORS_OX05C1S_H_*/