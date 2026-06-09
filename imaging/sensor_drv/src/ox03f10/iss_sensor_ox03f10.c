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
#include "iss_sensor_ox03f10.h"
#include "ox03f10_serdes_config.h"

static IssSensor_CreateParams  ox03f10CreatePrms = {
    SENSOR_OX03F10_UB953_OV,            /* sensor name */
    0x6,                                /* i2cInstId */
    I2C_ADDR_SENSOR,                    /*i2cAddrSensor*/
    I2C_ADDR_SER,                       /*i2cAddrSer*/
    {                                   /* sensorInfo */
        {                               /* raw_params */
            OX03F10_OUT_WIDTH,          /* width */
            OX03F10_OUT_HEIGHT-OX03F10_META_HEIGHT_AFTER,            /* height */
            1,                          /* num_exposures */
            (vx_bool)vx_false_e,                 /* line_interleaved */
            {                           /* tivx_raw_image_format_t */
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0] */
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0] */
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0] */
            },
            OX03F10_META_HEIGHT_BEFORE, /* meta_height_before */
            OX03F10_META_HEIGHT_AFTER,  /* meta_height_after */
        },
        (uint32_t)ISS_SENSOR_OX03F10_FEATURES,    /* features */
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_AEWB,  /* aewbMode */
        30,                             /* fps */
        4,                              /* numDataLanes */
        {1, 2, 3, 4},                   /* dataLanesMap */
        {0, 0, 0, 0},                   /* dataLanesPolarity */
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS, /* csi_laneBandSpeed */
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,        /* fpdLink */
        OX03F10_SEN_I2C_ADDR,           /* senI2cAddr */
        OX03F10_SER_I2C_ADDR,           /* serI2cAddr */
    },
#endif
    12,                                 /* numChan */
    3,                                  /* dccId */
};

static IssSensorFxns           ox03f10SensorFxns = {
    OX03F10_Probe,
    OX03F10_Config,
    OX03F10_StreamOn,
    OX03F10_StreamOff,
    OX03F10_PowerOn,
    OX03F10_PowerOff,
    OX03F10_GetExpParams,
    OX03F10_SetAeParams,
    OX03F10_GetDccParams,
    OX03F10_InitAewbConfig,
    OX03F10_GetIspConfig,
    OX03F10_ReadWriteReg,
    OX03F10_GetExpPrgFxn,
    OX03F10_deinit,
    OX03F10_GetWBPrgFxn,
    OX03F10_SetAwbParams
};

static IssSensorIntfParams     ox03f10SensorIntfPrms = {
    1,                      /* sensorBroadcast */
    0,                      /* enableFsin */
    0,                      /* numCamerasStreaming */
};

IssSensors_Handle ox03f10SensorHandle = {
    1,                      /* isUsed */
    &ox03f10CreatePrms,     /* CreatePrms */
    &ox03f10SensorFxns,     /* SensorFxns */
    &ox03f10SensorIntfPrms, /* SensorIntfPrms */
};

/*
 * \brief DCC Parameters of OX03F10
 */
static uint16_t redGain_prev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t greenGain_prev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t blueGain_prev[ISS_SENSORS_MAX_CHANNEL];

int32_t IssSensor_OX03F10_Init(void)
{
    int32_t status;
    int32_t chId;
    status = IssSensor_Register(&ox03f10SensorHandle);
    if(0 != status)
    {
        appLogPrintf("IssSensor_OX03F10_Init failed \n");
    }
	for(chId=0;chId<(int32_t)ISS_SENSORS_MAX_CHANNEL;chId++)
	{
        redGain_prev[chId] = 512;
        greenGain_prev[chId] = 512;
        blueGain_prev[chId] = 512;
    }

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t OX03F10_Probe(uint32_t chId, void *pSensorHdl)
{
    int32_t status = -1;

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint16_t regAddr;
    uint8_t sensorI2cAddr, regVal, regValExp;

    uint16_t regAddrValMap[][2] = {
        {OX03F10_SEN_ID_REG_A, OX03F10_SEN_ID_VAL_A},
        {OX03F10_SEN_ID_REG_B, OX03F10_SEN_ID_VAL_B},
        {OX03F10_SEN_ID_REG_C, OX03F10_SEN_ID_VAL_C}
    };

    regVal = 0xFF;

    pCreatePrms = pSenHandle->createPrms;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    status = OX03F10_PowerOn(chId, pSensorHdl);
    if(status != 0)
    {
        appLogPrintf("%s: OX03F10_PowerOn returned status: %d\n", __func__, status);
    }
    else
    {
        for(uint8_t i = 0; i < ((sizeof(regAddrValMap))/(sizeof(regAddrValMap[0]))); i++)
        {
            vx_bool shouldBreak = (vx_bool)vx_false_e;

            regAddr = regAddrValMap[i][0];
            regValExp = (uint8_t)regAddrValMap[i][1];
            status = OX03F10_ReadReg(sensorI2cAddr, regAddr, &regVal, 1u);
            if(status != 0)
            {
                appLogPrintf("%s: OX03F10_ReadReg returned status: %d\n", __func__, status);
                status = -1;
                shouldBreak = (vx_bool)vx_true_e;
            }
            else if(regVal != regValExp)
            {
                appLogPrintf("%s: Read chip revision (register: 0x%.4x) 0x%.2x, expected: 0x%.2x\n",
                    __func__, regAddr, regVal, regValExp);
                status = -1;
                shouldBreak = (vx_bool)vx_true_e;
            }
            else
            {
                /* Continue to next register */
            }

            if((vx_bool)vx_true_e == shouldBreak)
            {
                break;
            }
        }

        /* status can be set to 0 indicating probe has determined camera attached is X3F */
        if(status == 0)
        {
            status = 0;
        }
    }
    
    return (status);
}

static int32_t OX03F10_Sensor_RegConfig(uint32_t i2cInstId, uint8_t sensorI2cAddr, I2cParams *sensorCfg, uint16_t sensor_cfg_script_len)
{
    (void)i2cInstId;
    int32_t status = 0;
    uint16_t regAddr;
    uint8_t regValue;
    uint16_t delayMilliSec;
    uint32_t regCnt;

    if(NULL != sensorCfg)
    {
        regCnt = 0;
        regAddr  = sensorCfg[regCnt].nRegAddr;
        regValue = (uint8_t)sensorCfg[regCnt].nRegValue;
        delayMilliSec = (uint16_t)sensorCfg[regCnt].nDelay;

        appLogPrintf(" Configuring OX03F10 imager 0x%.2x... Please wait till it finishes \n", sensorI2cAddr);
        while(regCnt<sensor_cfg_script_len)
        {
            int32_t write_status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regValue, 1u);
            if (0 != write_status)
            {
                appLogPrintf("\nOX03F10: Sensor Reg Write Failed for regAddr 0x%.2x\n", regAddr);
                status = write_status;
            }

            if(delayMilliSec > 0U)
            {
               appLogWaitMsecs(delayMilliSec);
            }

            regCnt++;
            regAddr  = sensorCfg[regCnt].nRegAddr;
            regValue = (uint8_t)sensorCfg[regCnt].nRegValue;
            delayMilliSec = (uint16_t)sensorCfg[regCnt].nDelay;
        }
        /* Wait 100ms after the init is done */
        appLogWaitMsecs(100);
    }
    else
    {
        appLogPrintf(" OX03F10 config script is NULL \n");
    }
    return status;
}

static uint32_t ox03f10FeaturesEnabled;
static int32_t OX03F10_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    int32_t status = 0;
    uint32_t i2cInstId;
    uint16_t sensor_cfg_script_len = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if(sensor_features_requested != (sensor_features_requested & (uint32_t)ISS_SENSOR_OX03F10_FEATURES))
    {
        appLogPrintf("OX03F10_Config : Error. feature set 0x%x is not supported \n", sensor_features_requested);
        status = -1;
    }
    else
    {
        ox03f10FeaturesEnabled = sensor_features_requested;
        i2cInstId = pCreatePrms->i2cInstId;

        /* Configure serializer */
        status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], ub953SerCfg_OX03F10);
        if(0 != status)
        {
            appLogPrintf("OX03F10_Config Error : UB953 config failed for camera # %d \n", chId);
        }
        else
        {
            sensor_cfg_script_len = sizeof(ox03f10UB953Config) / sizeof(ox03f10UB953Config[0]);
            status = OX03F10_Sensor_RegConfig(i2cInstId, pCreatePrms->i2cAddrSensor[chId], ox03f10UB953Config, sensor_cfg_script_len);
            if(0 != status)
            {
                appLogPrintf("OX03F10_Config Error : OX03F10_Sensor_RegConfig returned status = %d \n", status);
            }
        }
    }

    return (status);
}

static int32_t OX03F10_StreamOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint8_t sensorI2cAddr;

    pCreatePrms = pSenHandle->createPrms;

    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    /* OX03F Stream on, 0x0100 = 0x01 */
    status = OX03F10_WriteReg(sensorI2cAddr, 0x0100, 0x01, 1u);
    if (0 != status)
    {
        status = status;
    }
    else
    {
        appLogWaitMsecs(10);

        /* Enable deserializer CSI-TX output */
        status = enableUB960Streaming(chId);
    }
    return (status);
}

static int32_t OX03F10_StreamOff(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint8_t sensorI2cAddr;

    pCreatePrms = pSenHandle->createPrms;

    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    /* OX03F Stream off, 0x0100 = 0x00 */
    status = OX03F10_WriteReg(sensorI2cAddr, 0x0100, 0x00, 1u);
    if (0 != status)
    {
        status = status;
    }
    else
    {
        appLogWaitMsecs(10);

        /* Disable deserializer CSI-TX output */
        status = disableUB960Streaming(chId);
    }
    return status;
}

static int32_t OX03F10_PowerOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    uint8_t serI2Caddr_7bit = (uint8_t)(OX03F10_SER_I2C_ADDR >> 1);
    uint8_t senI2cAddr_7bit = (uint8_t)(OX03F10_SEN_I2C_ADDR >> 1);

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle *pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams *pCreatePrms = pSenHandle->createPrms;

    /* Set serializer alias address */
    status = UB960_SetAlias(chId, 0, serI2Caddr_7bit, pCreatePrms->i2cAddrSer[chId]);
    if(0 != status)
    {
        appLogPrintf("OX03F10_PowerOn Error : UB960_SetAlias returned %d when setting serializer (0x%.2x 7-bit) to alias 0x%.2x 7-bit \n",
            status, serI2Caddr_7bit, pCreatePrms->i2cAddrSer[chId]);
    }

    /* Set sensor alias address */
    status = UB960_SetAlias(chId, 1, senI2cAddr_7bit, pCreatePrms->i2cAddrSensor[chId]);
    if(0 != status)
    {
        appLogPrintf("OX03F10_PowerOn Error : UB960_SetAlias returned %d when setting sensor (0x%.2x 7-bit) to alias 0x%.2x 7-bit \n",
            status, senI2cAddr_7bit, pCreatePrms->i2cAddrSer[chId]);
    }

    appLogWaitMsecs(20);

    return status;
}

static int32_t OX03F10_PowerOff(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;

    return (status);
}

static int32_t OX03F10_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = -1;

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint8_t sensorI2cAddr;

#if defined(ENABLE_DIGITAL_GAIN)
    uint16_t regAddr;
    uint8_t regVal;

    uint32_t digGain = 0;
#endif

    uint16_t expTime;
    uint8_t intVal = 0U;
    uint8_t decVal = 0U;

    pCreatePrms = pSenHandle->createPrms;

    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];
    
    /* set exposure in terms of row time
    *  row_time = HTS/SCLK = 2025/100MHz = 0.02025 ms */
    expTime = (uint16_t)((pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG] * 100U) / 2025U);
    (void)OX03F10_WriteReg(sensorI2cAddr, AEC_HCG_CTRL_01, (uint8_t)(((uint32_t)expTime >> 8) & 0xFFU), 1U);
    (void)OX03F10_WriteReg(sensorI2cAddr, AEC_HCG_CTRL_02, (uint8_t)((uint32_t)expTime & 0xFFU), 1U);

    /* set analog gain */
    /*
        Analog Gain Range       Gain Step       Active Bits
        1.0000  - 1.9375    |   0.0625      |   0001.xxxx
        2.000   - 3.875     |   0.125       |   001x.xxx0
        4.00    - 7.75      |   0.25        |   01xx.xx00
        8.0     - 15.5      |   0.5         |   1xxx.x000
    */
    intVal = (uint8_t)((pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] >> 10U) & 0xFFU); /* gain / 1024*/
    if(intVal <= 1U)
    {
        /* Equivalent to (uint8_t)(((gain % 1024) / 64) + 0.5)*/
        decVal = (uint8_t)(((pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] & 1023U) + 32U) >> 6);
    }
    else if (intVal <= 3U) /* intVal range: 2 - 3 */
    {
        /* Equivalent to (uint8_t)(((gain % 1024) / 128) + 0.5)*/
        decVal = (uint8_t)(((pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] & 1023U) + 64U) >> 6);
        decVal = (uint8_t)((uint32_t)decVal & 0x1EU);
    }
    else if (intVal <= 7U) /* intVal range: 4 - 7 */
    {
        /* Equivalent to (uint8_t)(((gain % 1024) / 256) + 0.5)*/
        decVal = (uint8_t)(((pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] & 1023U) + 128U) >> 6);
        decVal = (uint8_t)((uint32_t)decVal & 0x1CU);
    }
    else if (intVal <= 15U) /* intVal range: 8 - 15 */
    {
        /* Equivalent to (uint8_t)(((gain % 1024) / 512) + 0.5)*/
        decVal = (uint8_t)(((pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] & 1023U) + 256U) >> 6);
        decVal = (uint8_t)((uint32_t)decVal & 0x18U);
    }
    else
    {
#if !defined(ENABLE_DIGITAL_GAIN)
        appLogPrintf("Invalid analog gain: %d\n", pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG]);
#endif
    }

    if(decVal > 0xFU)
    {
        /* round */
        decVal = 0U;
        if(intVal < 15U)
        {
            intVal = intVal + 1U;
        }
        else
        {
            intVal = 15U;
        }
    }

#if defined(ENABLE_DIGITAL_GAIN)
    if(intVal > 0xF)
    {
        /* (analog_gain - 16127) + 1024
        *  15.5x gain (15872) to 16127 rounds down to 15.5, so use 16128
        *  and above as surpllus value for digital gain. Add 1024 for base
        *  1x value of digital gain. */
        digGain = pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] - 15103;

        if(digGain > 0){
            /* set digital gain */

            /* Write integer to AEC_HCG_CTRL_0A[3:0] */
            regAddr = AEC_HCG_CTRL_0A;
            regVal = (digGain >> 10);
            status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regVal, 1U);
            
            /* Write decimal to AEC_HCG_CTRL_0B[7:0] */
            regAddr = AEC_HCG_CTRL_0B;
            regVal = ((digGain >> 2) & 0xFF);
            status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regVal, 1U);
            
            /* Write decimal to AEC_HCG_CTRL_0C[7:6] */
            regAddr = AEC_HCG_CTRL_0C;
            regVal = (digGain & 0x03) << 6;
            status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regVal, 1U);
        }    

        /* set analog gain to max (15.5) */
        intVal = 0x0F;
        decVal = 0x08;
    }
    else
    {
        /* set digital gain to base 1x gain */
        regAddr = AEC_HCG_CTRL_0A;
        regVal = 0x01;
        status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regVal, 1U);
        
        regAddr = AEC_HCG_CTRL_0B;
        regVal = 0x00;
        status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regVal, 1U);
        
        regAddr = AEC_HCG_CTRL_0C;
        regVal = 0x00;
        status = OX03F10_WriteReg(sensorI2cAddr, regAddr, regVal, 1U);
    }
#endif

    /* Write integer value (4 bits) to AEC_HCG_CTRL_08[3:0] */
    (void)OX03F10_WriteReg(sensorI2cAddr, AEC_HCG_CTRL_08, intVal, 1U);

    /* Write decimal value (4 bits) to AEC_HCG_CTRL_09[7:4] */
    (void)OX03F10_WriteReg(sensorI2cAddr, AEC_HCG_CTRL_09, (decVal << 4), 1U);

    status = 0;
    return (status);
}

static int32_t OX03F10_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    (void)chId;
    (void)pSensorHdl;
    (void)pDccPrms;
    int32_t status = 0;
    return (status);
}

static int32_t OX03F10_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;

    pExpPrms->expRatio = ISS_SENSOR_OX03F_DEFAULT_EXP_RATIO;

    return (status);
}

static void OX03F10_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void OX03F10_GetIspConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void OX03F10_deinit(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static int32_t OX03F10_ReadWriteReg(uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg)
{
    int32_t status = 0;

    uint8_t regValue = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if (1u == readWriteFlag)
    {
        /* write */
        regValue = (uint8_t)pReg->nRegValue;
        status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, regValue, 1u);
    }
    else
    {
        /* read */
        status = OX03F10_ReadReg(pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, &regValue, 1u);
        if (0 == status)
        {
            pReg->nRegValue = regValue;
        }
    }
    return (status);
}

static int32_t OX03F10_ReadReg(uint8_t i2cAddr,
                                uint16_t regAddr,
                                uint8_t *regVal,
                                uint32_t numRegs)
{
    int32_t  status = -1;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;

    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
        status = -1;
    }
    else
    {
        status = Board_i2c16BitRegRd(sensorI2cHandle, i2cAddr, regAddr, regVal, (uint8_t)numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
        if(0 != status)
        {
            appLogPrintf("Error : I2C Timeout while reading from OX03F10 register 0x%x \n", regAddr);
        }
    }

    return (status);
}

static int32_t OX03F10_WriteReg(uint8_t i2cAddr,
                                uint16_t regAddr,
                                uint8_t regVal,
                                uint32_t numRegs)
{
    int32_t  status = -1;
    uint8_t  regValue;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;

    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
        status = -1;
    }
    else
    {
        regValue = regVal;
        status = Board_i2c16BitRegWr(sensorI2cHandle, i2cAddr, regAddr, &regValue, (uint8_t)numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
        if(0 != status)
        {
            appLogPrintf("Error : I2C Timeout while writing 0x%x to OX03F10 register 0x%x \n", regVal, regAddr);
        }
    }

    return (status);
}

static int32_t OX03F10_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    uint8_t count = 0;

#if defined(ENABLE_DIGITAL_GAIN)
    /* 8x digital gain (using analog DCC 1x gain 1024) */
    uint16_t digitalGainMax = (uint16_t)(DIGITAL_GAIN_SURPLUS * 1024U);

    if(digitalGainMax > 15872U) {
        /* set to max x3f digital gain 15.5x */
        digitalGainMax = 15872U;
    }
#endif

    p_ae_dynPrms->targetBrightnessRange.min = 100;
    p_ae_dynPrms->targetBrightnessRange.max = 110;
    p_ae_dynPrms->targetBrightness = 105;
    p_ae_dynPrms->threshold = DEFAULT_TARGET_THRESHOLD;
    p_ae_dynPrms->exposureTimeStepSize = DEFAULT_EXPOSURE_TIME;
    p_ae_dynPrms->enableBlc = DEFAULT_ENABLE_BLC;

    /* 11.0 * 1000 ns = 11000 ns = 11.0 ms */
    p_ae_dynPrms->exposureTimeRange[count].min = 11000;

    /* 32.5 * 1000 ns = 32500 ns = 32.5 ms */
    p_ae_dynPrms->exposureTimeRange[count].max = 33000;

    /* 1x gain (1024) */
    p_ae_dynPrms->analogGainRange[count].min = 1024;

#if defined(ENABLE_DIGITAL_GAIN)
    /* 15.5x Analog Gain + Digital Gain */
    p_ae_dynPrms->analogGainRange[count].max = 15872 + digitalGainMax;
#else
    /* 15.5x Analog Gain (15.5 x 1024 = 15872) */
    p_ae_dynPrms->analogGainRange[count].max = 15872;
#endif
    p_ae_dynPrms->digitalGainRange[count].min = 256;    /* not used */
    p_ae_dynPrms->digitalGainRange[count].max = 256;    /* not used */
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}

static int32_t OX03F10_GetWBPrgFxn(uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t  status = 0;

    p_awb_dynPrms->redGainRange.min = 512;
    p_awb_dynPrms->redGainRange.max = 2048;

    p_awb_dynPrms->greenGainRange.min = 512;
    p_awb_dynPrms->greenGainRange.max = 2048;

    p_awb_dynPrms->blueGainRange.min = 512;
    p_awb_dynPrms->blueGainRange.max = 2048;

    p_awb_dynPrms->sensor_pre_gain = 1;

    appLogPrintf("OX03F10_GetWBPrgFxn: sensor_pre_gain = %d \n", p_awb_dynPrms->sensor_pre_gain);
    return (status);
}

static int32_t OX03F10_SetAwbParams(void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms)
{
    int32_t status = 0;
    uint16_t regVal, decVal;
    uint8_t intVal;

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if(redGain_prev[chId] != pWbPrms->rGain[0])
    {
        redGain_prev[chId] = (uint16_t)pWbPrms->rGain[0];

        intVal = (uint8_t)(pWbPrms->rGain[ISS_SENSOR_EXPOSURE_LONG] / 512U);
        decVal = (uint16_t)(((pWbPrms->rGain[ISS_SENSOR_EXPOSURE_LONG] % 512U) * 2U));

        regVal = (uint16_t)((((uint32_t)intVal & 0x1FU) << 2) | (((uint32_t)decVal >> 8) & 0x3U));
        int32_t write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_6, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }

        regVal = (uint16_t)((uint32_t)decVal & 0xFFU);
        write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_7, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }
    }

    if(greenGain_prev[chId] != pWbPrms->gGain[0])
    {
        greenGain_prev[chId] = (uint16_t)pWbPrms->gGain[0];
        /* HCG */
        intVal = (uint8_t)(pWbPrms->gGain[ISS_SENSOR_EXPOSURE_LONG] / 512U);
        decVal = (uint16_t)(((pWbPrms->gGain[ISS_SENSOR_EXPOSURE_LONG] % 512U) * 2U));

        regVal = (uint16_t)((((uint32_t)intVal & 0x1FU) << 2) | (((uint32_t)decVal >> 8) & 0x3U));
        int32_t write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_2, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }
        write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_4, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }

        regVal = (uint16_t)((uint32_t)decVal & 0xFFU);
        write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_3, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }
        write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_5, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }
    }

    if(blueGain_prev[chId] != pWbPrms->bGain[0])
    {
        blueGain_prev[chId] = (uint16_t)pWbPrms->bGain[0];

        intVal = (uint8_t)(pWbPrms->bGain[ISS_SENSOR_EXPOSURE_LONG] / 512U);
        decVal = (uint16_t)(((pWbPrms->bGain[ISS_SENSOR_EXPOSURE_LONG] % 512U) * 2U));

        regVal = (uint16_t)((((uint32_t)intVal & 0x1FU) << 2) | (((uint32_t)decVal >> 8) & 0x3U));
        int32_t write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_0, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }

        regVal = (uint16_t)((uint32_t)decVal & 0xFFU);
        write_status = OX03F10_WriteReg(pCreatePrms->i2cAddrSensor[chId], AWB_GAIN_HCG_1, (uint8_t)regVal, 1u);
        if (0 != write_status)
        {
            status = write_status;
        }
    }


    return (status);
}
