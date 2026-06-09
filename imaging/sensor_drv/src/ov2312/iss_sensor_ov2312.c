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
#include "iss_sensor_ov2312.h"
#include "ov2312_serdes_config.h"

static IssSensor_CreateParams   ov2312CreatePrms = {
    SENSOR_OV2312_UB953_LI,                         /*sensor name*/
    0x6,                                            /*i2cInstId*/
    I2C_ADDR_SENSOR,                /*i2cAddrSensor*/
    I2C_ADDR_SER,                   /*i2cAddrSer*/
    /*IssSensor_Info*/
    {
        {
            OV2312_OUT_WIDTH,                       /*width*/
            OV2312_OUT_HEIGHT,                      /*height*/
            1,                                      /*num_exposures*/
            (vx_bool)vx_false_e,                             /*line_interleaved*/
            {
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 9},         /*dataFormat and MSB [0]*/
            },
            0,                                      /*meta_height_before*/
            0,                                      /*meta_height_after*/
        },
        ISS_SENSOR_OV2312_FEATURES,                 /*features*/
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_AE,                /*aeMode*/
        30,                                         /*fps*/
        4,                                          /*numDataLanes*/
        {1, 2, 3, 4},                               /*dataLanesMap*/
        {0, 0, 0, 0},                               /*dataLanesPolarity*/
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS,    /*csi_laneBandSpeed*/
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,           /* fpdLink */
        OV2312_I2C_ADDR,                /* senI2cAddr */
        0x30,                           /* serI2cAddr */
    },
#endif
    4,                                              /*numChan*/
    2312,                                           /*dccId*/
};

static IssSensorFxns            ov2312SensorFxns = {
    OV2312_Probe,
    OV2312_Config,
    OV2312_StreamOn,
    OV2312_StreamOff,
    OV2312_PowerOn,
    OV2312_PowerOff,
    OV2312_GetExpParams,
    OV2312_SetAeParams,
    OV2312_GetDccParams,
    OV2312_InitAewbConfig,
    OV2312_GetIspConfig,
    OV2312_ReadWriteReg,
    OV2312_GetExpPrgFxn,
    OV2312_deinit,
    OV2312_GetWBPrgFxn,
    OV2312_SetAwbParams
};

static IssSensorIntfParams      ov2312SensorIntfPrms = {
    0,             /*sensorBroadcast*/
    0,             /*enableFsin*/
    0,			   /*numCamerasStreaming*/
};

IssSensorConfig     ov2312SensorRegConfigLinear30fps = {
    NULL,                       /*desCfgPreScript*/
    ub953SerCfg_D3OV2312,       /*serCfgPreScript*/
    ov2312LinearConfig30fps,    /*sensorCfgPreScript*/
    NULL,                       /*desCfgPostScript*/
    NULL,                       /*serCfgPostScript*/
    NULL,                       /*sensorCfgPostScript*/
};

IssSensors_Handle ov2312SensorHandle = {
    1,                          /*isUsed*/
    &ov2312CreatePrms,          /*CreatePrms*/
    &ov2312SensorFxns,          /*SensorFxns*/
    &ov2312SensorIntfPrms,      /*SensorIntfPrms*/
};

/*
 * \brief DCC Parameters of ov2312
 */

int32_t IssSensor_OV2312_Init(void)
{
    int32_t status;
    status = IssSensor_Register(&ov2312SensorHandle);
    if(0 != status)
    {
        appLogPrintf("IssSensor_OV2312_Init failed \n");
    }
    #if defined(QNX)
    UNUSED(ub960DesCfg_Common);
    #endif

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t OV2312_Probe(uint32_t chId, void *pSensorHdl)
{
    int32_t status = -1;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
    uint16_t chipIdRegAddr = OV2312_CHIP_ID_REG_ADDR;
    uint8_t chipIdRegValueRead = 0xAB;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    I2cParams    * serCfg = NULL;
    uint8_t count=0;
    uint8_t max_retries = 1;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    status = UB960_SetSensorAlias(chId, OV2312_I2C_ADDR >> 1, pCreatePrms->i2cAddrSer[chId]);
    if (0 != status)
    {
        appLogPrintf("OV2312_Probe Error: UB960_SetSensorAlias for chId %d returned %d \n", chId, status);
        return status;
    }

    serCfg = ov2312SensorRegConfigLinear30fps.serCfgPreScript;
    /*The code assumes that I2C instance is the same for sensor and serializer*/
    if (NULL != serCfg)
    {
        status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], serCfg);
    }

    /*Read chip ID to detect if the sensor can be detected*/
    while ((chipIdRegValueRead != OV2312_CHIP_ID_REG_VAL) && (count < max_retries))
    {
        status = OV2312_ReadReg((uint8_t)i2cInstId, sensorI2cAddr, chipIdRegAddr, &chipIdRegValueRead, 1U);
        if (status == 0)
        {
            if (chipIdRegValueRead == OV2312_CHIP_ID_REG_VAL)
            {
                status = 0;
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("OV2312_Probe SUCCESS : Read expected value 0x%x at chip ID register 0x%x \n", OV2312_CHIP_ID_REG_VAL, chipIdRegAddr);
#endif
            }
            else
            {
                status = -1;
                appLogPrintf("OV2312_Probe : 0x%x read at chip ID register 0x%x. Expected 0x%x \n", chipIdRegValueRead, chipIdRegAddr, OV2312_CHIP_ID_REG_VAL);
                appLogPrintf("OV2312 Probe Failed.. Retrying \n");
                appLogWaitMsecs(100);
            }
        }
        else
        {
            appLogPrintf("OV2312 Probe : Failed to read CHIP_ID register 0x%x \n", chipIdRegAddr);
        }
        count++;
    }

    return (status);
}

static int32_t OV2312_Sensor_RegConfig(uint32_t i2cInstId, uint8_t sensorI2cAddr, I2cParams *sensorCfg, uint16_t sensor_cfg_script_len)
{
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

#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf(" Configuring OV2312 imager 0x%x.. Please wait till it finishes \n", sensorI2cAddr);
#endif
        while(regCnt<sensor_cfg_script_len)
        {
            status |= OV2312_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);

            if (0 != status)
            {
                appLogPrintf(" \n \n OV2312: Sensor Reg Write Failed for regAddr 0x%x \n \n", regAddr);
            }

            if(delayMilliSec > 0)
            {
               appLogWaitMsecs(delayMilliSec);
            }

            regCnt++;
            regAddr  = sensorCfg[regCnt].nRegAddr;
            regValue = (uint8_t)sensorCfg[regCnt].nRegValue;
            delayMilliSec = (uint16_t)sensorCfg[regCnt].nDelay;
        }
        /*Wait 100ms after the init is done*/
        appLogWaitMsecs(100);
    }
    else
    {
        appLogPrintf(" OV2312 config script is NULL \n");
    }
    return status;
}

static uint32_t ov2312FeaturesEnabled;
static int32_t OV2312_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    int32_t status = 0;
    uint32_t i2cInstId;
    uint16_t sensor_cfg_script_len = 0;
    I2cParams *sensorCfg = NULL;
    I2cParams *deserCfg = NULL;
    I2cParams *serCfg = NULL;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    int8_t ub960InstanceId = getDeserializerInstIdFromChId(chId);

    if (ub960InstanceId < 0)
    {
        appLogPrintf("Invalid ub960InstanceId \n");
        return -1;
    }

    pCreatePrms = pSenHandle->createPrms;

    if (sensor_features_requested != (sensor_features_requested & ISS_SENSOR_OV2312_FEATURES))
    {
        appLogPrintf("OV2312_Config : Error. feature set 0x%x is not supported \n", sensor_features_requested);
        return -1;
    }

    ov2312FeaturesEnabled = sensor_features_requested;

    i2cInstId = pCreatePrms->i2cInstId;
    if (sensor_features_requested & ISS_SENSOR_FEATURE_LINEAR_MODE)
    {
        deserCfg = ov2312SensorRegConfigLinear30fps.desCfgPreScript;
        serCfg = ov2312SensorRegConfigLinear30fps.serCfgPreScript;
        sensorCfg = ov2312SensorRegConfigLinear30fps.sensorCfgPreScript;
        sensor_cfg_script_len = OV2312_LINEAR_CONFIG_SIZE;
    }

    if (NULL != deserCfg)
    {
        status |= IssSensor_cfgDesScript(deserCfg, ub960InstanceId);
    }

    /*Deserializer config is done in IssSensor_PowerOn, Need to set sensor alias*/
    status = UB960_SetSensorAlias(chId, OV2312_I2C_ADDR >> 1, pCreatePrms->i2cAddrSer[chId]);
    if (0 != status)
    {
        appLogPrintf("OV2312_Config Error : UB960_SetSensorAlias for chId %d returned %d \n", chId, status);
    }
    else
    {
        if (NULL != serCfg)
        {
            status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], serCfg);
        }
        if (0 != status)
        {
            appLogPrintf("OV2312_Config Error : UB953 config failed for camera # %d \n", chId);
        }
        else
        {
            appLogWaitMsecs(50);
            status = OV2312_Sensor_RegConfig((uint8_t)i2cInstId, pCreatePrms->i2cAddrSensor[chId], sensorCfg, sensor_cfg_script_len);
        }
    }

    return (status);
}

static int32_t OV2312_set_groupB(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = -1;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    uint32_t exposure = (60 * 1300 * pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG]/1000000);
    uint32_t analogGain = pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG];

    I2cParams update_groupB[13] = {
        {0x3208, 0x00}, /* Group B (RGB Dominant VC0) */
        {OV2312_AEC_PK_EXPO_HI, (exposure >> 8) & 0xff},
        {OV2312_AEC_PK_EXPO_LO, exposure & 0xff},
        {OV2312_AEC_PK_AGAIN_HI, (analogGain >> 4) & 0xff},
        {OV2312_AEC_PK_AGAIN_LO, (analogGain & 0x0f) << 4},
        {OV2312_AEC_PK_DGAIN_HI, 0x01},
        {OV2312_AEC_PK_DGAIN_LO, 0x00},
        {0x3920, 0x00},
        {0x4813, 0x01}, /* VC=1. This register takes effect from next frame */
        {0x3208, 0x10},
        {0x320D, 0x00}, /* Auto mode switch between group0 and group1 ;setting to switch */
        {0x320D, 0x31},
        {0x3208, 0xA0},
    };

    for (int32_t i = 0; i < 13; i++)
    {
        status = OV2312_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, update_groupB[i].nRegAddr, (uint8_t)update_groupB[i].nRegValue, 1u);
    }

    return status;
}

static int32_t OV2312_set_groupA(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = -1;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    uint32_t exposure = (60 * 1300 * pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG]/1000000);
    uint32_t analogGain = pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG];

    I2cParams update_groupA[13] = {
        {0x3208, 0x01}, /* Group A (RGB Dominant VC0) */
        {OV2312_AEC_PK_EXPO_HI, (exposure >> 8) & 0xff},
        {OV2312_AEC_PK_EXPO_LO, exposure & 0xff},
        {OV2312_AEC_PK_AGAIN_HI, (analogGain >> 4) & 0xff},
        {OV2312_AEC_PK_AGAIN_LO, (analogGain & 0x0f) << 4},
        {OV2312_AEC_PK_DGAIN_HI, 0x01},
        {OV2312_AEC_PK_DGAIN_LO, 0x00},
        {0x3920, 0x00},
        {0x4813, 0x00}, /* VC=0. This register takes effect from next frame */
        {0x3208, 0x11},
        {0x320D, 0x00}, /* Auto mode switch between group0 and group1 ;setting to switch */
        {0x320D, 0x31},
        {0x3208, 0xA0},
    };

    for (int32_t i = 0; i < 13; i++)
    {
        status = OV2312_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, update_groupA[i].nRegAddr, (uint8_t)update_groupA[i].nRegValue, 1u);
    }
    
    return status;
}

static int32_t OV2312_StreamOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    int8_t ub960InstanceId = getDeserializerInstIdFromChId(chId);

    if (ub960InstanceId < 0)
    {
        appLogPrintf("Invalid ub960InstanceId \n");
        return -1;
    }

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    status |= OV2312_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, 0x0100, 0x01, 1u); /*ACTIVE*/
    appLogWaitMsecs(10);
    status |= enableUB960Streaming(chId);

    return (status);
}

static int32_t OV2312_StreamOff(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    status |= OV2312_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, 0x0100, 0x00, 1u); /*STANDBY*/
    appLogWaitMsecs(10);
    status |= disableUB960Streaming(chId);
    return status;
}

static int32_t OV2312_PowerOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    return status;
}

static int32_t OV2312_PowerOff(uint32_t chId, void *pSensorHdl)
{
    return (0);
}

static int32_t OV2312_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = -1;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;

    if (chId%2 == 0)
    {
        /* RGB dominant channel */
        status = OV2312_set_groupB(chId, pSenHandle, pExpPrms);
    }
    else
    {
         /* IR dominant channel */
        status = OV2312_set_groupA(chId, pSenHandle, pExpPrms);
    }
    return status;
}

static int32_t OV2312_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    int32_t status = 0;
    return (status);
}

static int32_t OV2312_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = 0;

    pExpPrms->expRatio = ISS_SENSOR_OV2312_DEFAULT_EXP_RATIO;

    return (status);
}

static void OV2312_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    return;
}

static void OV2312_GetIspConfig (uint32_t chId, void *pSensorHdl)
{
    return;
}

static void OV2312_deinit (uint32_t chId, void *pSensorHdl)
{
    return;
}

static int32_t OV2312_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg)
{
    int32_t status = 0;

    uint8_t regValue = 0;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if (1u == readWriteFlag)
    {
        /*write*/
        regValue = (uint8_t)pReg->nRegValue;
        status = OV2312_WriteReg((uint8_t)pCreatePrms->i2cInstId,
            pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, regValue, 1u);
    }
    else
    {
        /*read*/
        status = OV2312_ReadReg((uint8_t)pCreatePrms->i2cInstId,
            pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, &regValue, 1u);

        if (0 == status)
        {
            pReg->nRegValue = regValue;
        }
    }
    return (status);
}

static int32_t OV2312_ReadReg(uint8_t     i2cInstId,
                            uint8_t         i2cAddr,
                            uint16_t        regAddr,
                            uint8_t         *regVal,
                            uint32_t        numRegs)
{
    int32_t  status = -1;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;
    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
        return -1;
    }
    status = Board_i2c16BitRegRd(sensorI2cHandle, i2cAddr, regAddr, regVal, (uint8_t)numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        appLogPrintf("Error : I2C Timeout while reading from OV2312 register 0x%x \n", regAddr);
    }
    return (status);
}

static int32_t OV2312_WriteReg(uint8_t    i2cInstId,
                             uint8_t       i2cAddr,
                             uint16_t         regAddr,
                             uint8_t          regVal,
                             uint32_t      numRegs)
{
    int32_t  status = -1;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;
    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
        return -1;
    }
    status = Board_i2c16BitRegWr(sensorI2cHandle, i2cAddr, regAddr, &regVal, (uint8_t)numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        appLogPrintf("Error : I2C Timeout while writing 0x%x to OV2312 register 0x%x \n", regVal, regAddr);
    }

    return (status);
}

static int32_t OV2312_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms)
{
    int32_t  status = 0;
    uint8_t count = 0;

    p_ae_dynPrms->targetBrightnessRange.min = 40;
    p_ae_dynPrms->targetBrightnessRange.max = 50;
    p_ae_dynPrms->targetBrightness = 45;
    p_ae_dynPrms->threshold = 5;
    p_ae_dynPrms->enableBlc = 0;
    p_ae_dynPrms->exposureTimeStepSize = 1;

    p_ae_dynPrms->exposureTimeRange[count].min = 1000;
    p_ae_dynPrms->exposureTimeRange[count].max = 14450;
    p_ae_dynPrms->analogGainRange[count].min = 16;
    p_ae_dynPrms->analogGainRange[count].max = 16;
    p_ae_dynPrms->digitalGainRange[count].min = 1024;
    p_ae_dynPrms->digitalGainRange[count].max = 1024;
    count++;

    p_ae_dynPrms->exposureTimeRange[count].min = 14450;
    p_ae_dynPrms->exposureTimeRange[count].max = 14450;
    p_ae_dynPrms->analogGainRange[count].min = 16;
    p_ae_dynPrms->analogGainRange[count].max = 42;
    p_ae_dynPrms->digitalGainRange[count].min = 1024;
    p_ae_dynPrms->digitalGainRange[count].max = 1024;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    status = 0;
    return (status);
}

static int32_t OV2312_GetWBPrgFxn(uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms)
{
    int32_t  status = 0;

    return (status);
}

static int32_t OV2312_SetAwbParams(void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms)
{
    int32_t status = 0;

    return (status);
}
