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
#include "iss_sensor_SENSOR_NAME_LOWERCASE.h"
#include "SENSOR_NAME_LOWERCASE_serdes_config.h"

static IssSensor_CreateParams  SENSOR_NAME_LOWERCASECreatePrms = {

    //TODO: must define this macro in "iss_sensors.h" for integrating the new sensor driver
    SENSOR_MACRO_IN_HEADER,             /* sensor name */
    0x6,                                /* i2cInstId: DON'T CARE */
    I2C_ADDR_SENSOR,                    /* i2cAddrSensor */
    I2C_ADDR_SER,                       /* i2cAddrSer */
    {                                   /* sensorInfo */
        {                               /* raw_params */
            SENSOR_NAME_UPPERCASE_OUT_WIDTH,          /* width */
            SENSOR_NAME_UPPERCASE_OUT_HEIGHT,         /* height */
            1,                          /* num_exposures */
            vx_false_e,                 /* line_interleaved */
            {                           /* tivx_raw_image_format_t */
                {TIVX_RAW_IMAGE_16_BIT, SENSOR_RAW_MSB_LOCATION},    /*dataFormat and MSB */
            },
            SENSOR_NAME_UPPERCASE_META_HEIGHT_BEFORE,  /* meta_height_before */
            SENSOR_NAME_UPPERCASE_META_HEIGHT_AFTER,  /* meta_height_after */
        },
        ISS_SENSOR_SENSOR_NAME_UPPERCASE_FEATURES,    /* features */
        ALGORITHMS_ISS_AEWB_MODE_NONE,  /* aewbMode */
        30,                             /* fps: DON'T CARE */
        4,                              /* numDataLanes */
        {1, 2, 3, 4},                   /* dataLanesMap */
        {0, 0, 0, 0},                   /* dataLanesPolarity */
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS, /* csi_laneBandSpeed */
    },
    1,                                  /* numChan */
    SENSOR_DCC_ID,                      /* dccId   */
};

static IssSensorFxns           SENSOR_NAME_LOWERCASESensorFxns = {
    SENSOR_NAME_UPPERCASE_Probe,
    SENSOR_NAME_UPPERCASE_Config,
    SENSOR_NAME_UPPERCASE_StreamOn,
    SENSOR_NAME_UPPERCASE_StreamOff,
    SENSOR_NAME_UPPERCASE_PowerOn,
    SENSOR_NAME_UPPERCASE_PowerOff,
    SENSOR_NAME_UPPERCASE_GetExpParams,
    SENSOR_NAME_UPPERCASE_SetAeParams,
    SENSOR_NAME_UPPERCASE_GetDccParams,
    SENSOR_NAME_UPPERCASE_InitAewbConfig,
    SENSOR_NAME_UPPERCASE_GetIspConfig,
    SENSOR_NAME_UPPERCASE_ReadWriteReg,
    SENSOR_NAME_UPPERCASE_GetExpPrgFxn,
    SENSOR_NAME_UPPERCASE_deinit,
    SENSOR_NAME_UPPERCASE_GetWBPrgFxn,
    SENSOR_NAME_UPPERCASE_SetAwbParams
};

static IssSensorIntfParams     SENSOR_NAME_LOWERCASESensorIntfPrms = {
    0,                      /* sensorBroadcast */
    0,                      /* enableFsin */
    0,                      /* numCamerasStreaming */
};

IssSensors_Handle SENSOR_NAME_LOWERCASESensorHandle = {
    1,                      /* isUsed */
    &SENSOR_NAME_LOWERCASECreatePrms,     /* CreatePrms */
    &SENSOR_NAME_LOWERCASESensorFxns,     /* SensorFxns */
    &SENSOR_NAME_LOWERCASESensorIntfPrms, /* SensorIntfPrms */
};

/*
 * \brief DCC Parameters of SENSOR_NAME_UPPERCASE
 */
static uint16_t redGain_prev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t greenGain_prev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t blueGain_prev[ISS_SENSORS_MAX_CHANNEL];

int32_t IssSensor_SENSOR_NAME_UPPERCASE_Init()
{
    int32_t status;
    int32_t chId;
    status = IssSensor_Register(&SENSOR_NAME_LOWERCASESensorHandle);
    if(0 != status)
    {
        printf("IssSensor_SENSOR_NAME_UPPERCASE_Init failed \n");
    }
	for(chId=0;chId<ISS_SENSORS_MAX_CHANNEL;chId++)
	{
        redGain_prev[chId] = greenGain_prev[chId] = blueGain_prev[chId] = 512;
    }

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t SENSOR_NAME_UPPERCASE_Probe(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    /* TODO: add probe code here if necessary */
    status = -1;
    
    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_Sensor_RegConfig(uint32_t i2cInstId, uint8_t sensorI2cAddr, I2cParams *sensorCfg, uint16_t sensor_cfg_script_len)
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
        regValue = sensorCfg[regCnt].nRegValue;
        delayMilliSec = sensorCfg[regCnt].nDelay;

        printf(" Configuring SENSOR_NAME_UPPERCASE imager 0x%.2x... Please wait till it finishes \n", sensorI2cAddr);
        while(regCnt<sensor_cfg_script_len)
        {
            status |= SENSOR_NAME_UPPERCASE_WriteReg(sensorI2cAddr, regAddr, regValue, 1u);
            if (0 != status)
            {
                printf("\nSENSOR_NAME_UPPERCASE: Sensor Reg Write Failed for regAddr 0x%.2x\n", regAddr);
            }

            if(delayMilliSec > 0)
            {
               appLogWaitMsecs(delayMilliSec);
            }

            regCnt++;
            regAddr  = sensorCfg[regCnt].nRegAddr;
            regValue = sensorCfg[regCnt].nRegValue;
            delayMilliSec = sensorCfg[regCnt].nDelay;
        }
        /* Wait 100ms after the init is done */
        appLogWaitMsecs(100);
    }
    else
    {
        printf(" SENSOR_NAME_UPPERCASE config script is NULL \n");
    }
    return status;
}

static uint32_t SENSOR_NAME_LOWERCASEFeaturesEnabled;
static int32_t SENSOR_NAME_UPPERCASE_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    int32_t status = 0;
    uint32_t i2cInstId;
    uint16_t sensor_cfg_script_len = 0;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    assert(NULL != pSenHandle);
    pCreatePrms = pSenHandle->createPrms;
    assert(NULL != pCreatePrms);

    if(sensor_features_requested != (sensor_features_requested & ISS_SENSOR_SENSOR_NAME_UPPERCASE_FEATURES))
    {
        printf("SENSOR_NAME_UPPERCASE_Config : Error. feature set 0x%x is not supported \n", sensor_features_requested);
        return -1;
    }

    SENSOR_NAME_LOWERCASEFeaturesEnabled = sensor_features_requested;
    i2cInstId = pCreatePrms->i2cInstId;

    /* Configure serializer */
    status = ub953_cfgScript(i2cInstId, pCreatePrms->i2cAddrSer[chId], SerCfg_SENSOR_NAME_UPPERCASE);
    if(0 != status)
    {
        printf("SENSOR_NAME_UPPERCASE_Config Error : UB953 config failed for camera # %d \n", chId);
    }
    else
    {
        sensor_cfg_script_len = sizeof(SENSOR_NAME_LOWERCASEConfig) / sizeof(SENSOR_NAME_LOWERCASEConfig[0]);
        status = SENSOR_NAME_UPPERCASE_Sensor_RegConfig(i2cInstId, pCreatePrms->i2cAddrSensor[chId], SENSOR_NAME_LOWERCASEConfig, sensor_cfg_script_len);
        if(0 != status)
        {
            printf("SENSOR_NAME_UPPERCASE_Config Error : SENSOR_NAME_UPPERCASE_Sensor_RegConfig returned status = %d \n", status);
        }
    }

    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_StreamOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    assert(NULL != pSenHandle);
    pCreatePrms = pSenHandle->createPrms;
    assert(NULL != pCreatePrms);

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    /* Stream on */
    status |= SENSOR_NAME_UPPERCASE_WriteReg(sensorI2cAddr, SENSOR_STREAM_REG_ADDR, SENSOR_STREAM_ON_VALUE, 1u);
#if SENSOR_STREAM_ON_REG2
    status |= SENSOR_NAME_UPPERCASE_WriteReg(sensorI2cAddr, SENSOR_STREAM_REG2_ADDR, SENSOR_STREAM_ON_VAL2, 1u);
#endif
    appLogWaitMsecs(10);

    /* Enable deserializer CSI-TX output */
    status |= enableUB960Streaming(chId);
    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_StreamOff(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    assert(NULL != pSenHandle);
    pCreatePrms = pSenHandle->createPrms;
    assert(NULL != pCreatePrms);

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    /* Stream off */
    status |= SENSOR_NAME_UPPERCASE_WriteReg(sensorI2cAddr, SENSOR_STREAM_REG_ADDR, SENSOR_STREAM_OFF_VALUE, 1u);
#if SENSOR_STREAM_OFF_REG2
    status |= SENSOR_NAME_UPPERCASE_WriteReg(sensorI2cAddr, SENSOR_STREAM_REG2_ADDR, SENSOR_STREAM_OFF_VAL2, 1u);
#endif
    appLogWaitMsecs(10);

    /* Disable deserializer CSI-TX output */
    status |= disableUB960Streaming(chId);
    return status;
}

#if SENSOR_USE_PCAL_D3
static uint16_t GetPortSelectRegisterValue(uint32_t chId)
{
    switch (chId)
    {
        case 0:
        case 4:
            return 0x01;
        case 1:
        case 5:
            return 0x12;
        case 2:
        case 6:
            return 0x24;
        case 3:
        case 7:
            return 0x38;
        case 0xFF:
            return 0x0F;
        default:
            printf("Error : Invalid channel ID 0x%x \n", chId);
            return 0;
    }
}
#endif

static int32_t SENSOR_NAME_UPPERCASE_PowerOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    uint8_t serI2Caddr_7bit = SENSOR_NAME_UPPERCASE_SER_I2C_ADDR >> 1;
    uint8_t senI2cAddr_7bit = SENSOR_NAME_UPPERCASE_SEN_I2C_ADDR >> 1;

    IssSensors_Handle *pSenHandle = (IssSensors_Handle*)pSensorHdl;
    assert(pSenHandle != NULL);
    IssSensor_CreateParams *pCreatePrms = pSenHandle->createPrms;
    assert(pCreatePrms != NULL);

    uint32_t i2cInstId = pCreatePrms->i2cInstId;

#if SERDES_USE_FPDLINK4
    /* Configure deserializer to FPD4 mode */
    for (int i = 0; i < 2; i++)
    {
        status = IssSensor_cfgDesScript(ub9702_971_DesCfg_Common, i);
        if(status!=0)
        {
            printf("Error : IssSensor_cfgDesScript returned %d while configuring DES %d \n", status, i);
        }
    }
#endif

#if SENSOR_USE_PCAL_D3      // configure PCAL for D3 cameras
#define PCAL_7BIT_ADDR 0x70
    {
        uint8_t i2cAddrMisc[ISS_SENSORS_MAX_CHANNEL] = {
            MISC_0_I2C_ALIAS, MISC_1_I2C_ALIAS, MISC_2_I2C_ALIAS, MISC_3_I2C_ALIAS,
            MISC_4_I2C_ALIAS, MISC_5_I2C_ALIAS, MISC_6_I2C_ALIAS, MISC_7_I2C_ALIAS,
#if defined(SOC_J784S4) || defined(SOC_J742S2)
            MISC_8_I2C_ALIAS, MISC_9_I2C_ALIAS, MISC_10_I2C_ALIAS, MISC_11_I2C_ALIAS
#endif
        };

        uint8_t pcalI2cAliasAddr = i2cAddrMisc[chId];

        /* PCAL9538A @ 7-bit address 0x70 */
        I2cParams script[4u] = {
            {0x4C, 0x01, 0xFF},     /* port select register */
            {0x5F, PCAL_7BIT_ADDR << 1, 0xFF},
            {0x67, pcalI2cAliasAddr << 1, 0xFF},
            {0xFFFF, 0x00, 0x0} // End of script
        };

        script[0].nRegValue = GetPortSelectRegisterValue(chId);
        if (script[0].nRegValue == 0)
        {
            return -1;
        }

        int8_t  desInstanceId = getDeserializerInstIdFromChId(chId);
        if(desInstanceId < 0)
        {
            printf("Error : Invalid desInstanceId %d\n", desInstanceId);
            return -1;
        }

        status = IssSensor_cfgDesScript(script, desInstanceId);
        if (status != 0)
        {
            printf("\n\n %s: IssSensor_cfgDesScript FAILED \n\n", __func__);
            return status;
        }

        I2C_Handle sensorI2cHandle = NULL;
        static uint8_t sensorI2cByteOrder = 255U;
        getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
        uint8_t regVal = 0xFE;
        status = Board_i2c8BitRegWr(sensorI2cHandle, pcalI2cAliasAddr, 0x03, &regVal, 1, SENSOR_I2C_TIMEOUT);
        regVal = 0x01;
        status = Board_i2c8BitRegWr(sensorI2cHandle, pcalI2cAliasAddr, 0x01, &regVal, 1, SENSOR_I2C_TIMEOUT);

        if (status != 0)
        {
            printf("Error: Configuring PCAL for Reset line of ch %d returned %d \n", chId, status);
            return status;
        }

        printf("\n %s: PCAL RESET SUCCESSFUL DEBUG 1 \n\n", __func__);

        appLogWaitMsecs(200);
    }
#endif

    /* Set serializer alias address */
    //Gang_TODO: is this refactored?
    status = UB960_SetAlias(chId, 0, serI2Caddr_7bit, pCreatePrms->i2cAddrSer[chId]);
    if(0 != status)
    {
        printf("SENSOR_NAME_UPPERCASE_PowerOn Error : UB960_SetAlias returned %d when setting serializer (0x%.2x 7-bit) to alias 0x%.2x 7-bit \n",
            status, serI2Caddr_7bit, pCreatePrms->i2cAddrSer[chId]);
    }

    /* Set sensor alias address */
    //Gang_TODO: is this refactored?
    status = UB960_SetAlias(chId, 1, senI2cAddr_7bit, pCreatePrms->i2cAddrSensor[chId]);
    if(0 != status)
    {
        printf("SENSOR_NAME_UPPERCASE_PowerOn Error : UB960_SetAlias returned %d when setting sensor (0x%.2x 7-bit) to alias 0x%.2x 7-bit \n",
            status, senI2cAddr_7bit, pCreatePrms->i2cAddrSer[chId]);
    }

    appLogWaitMsecs(20);

    return status;
}

static int32_t SENSOR_NAME_UPPERCASE_PowerOff(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = 0;

    /* TODO: Set exposure in terms of row time */

    /* TODO: Set analog gain */
    
    /* TODO: Set digital gain */
    status = -1;

    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    int32_t status = 0;
    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = 0;
    return (status);
}

static void SENSOR_NAME_UPPERCASE_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    return;
}

static void SENSOR_NAME_UPPERCASE_GetIspConfig(uint32_t chId, void *pSensorHdl)
{
    return;
}

static void SENSOR_NAME_UPPERCASE_deinit(uint32_t chId, void *pSensorHdl)
{
    return;
}

static int32_t SENSOR_NAME_UPPERCASE_ReadWriteReg(uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg)
{
    int32_t status = 0;

    uint8_t regValue = 0;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    assert(NULL != pSenHandle);
    pCreatePrms = pSenHandle->createPrms;
    assert(NULL != pReg);

    if (1u == readWriteFlag)
    {
        /* write */
        regValue = pReg->nRegValue;
        status = SENSOR_NAME_UPPERCASE_WriteReg(pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, regValue, 1u);
    }
    else
    {
        /* read */
        status = SENSOR_NAME_UPPERCASE_ReadReg(pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, &regValue, 1u);
        if (0 == status)
        {
            pReg->nRegValue = regValue;
        }
    }
    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_ReadReg(uint8_t i2cAddr,
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
        printf("Sensor I2C Handle is NULL \n");
        return -1;
    }

    // 16-bit address is used by most ADAS sensors from OVT/Sony/Onsemi
    status = Board_i2c16BitRegRd(sensorI2cHandle, i2cAddr, regAddr, regVal, numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        issLogPrintf("Error : I2C Timeout while reading from SENSOR_NAME_UPPERCASE register 0x%x \n", regAddr);
    }

    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_WriteReg(uint8_t i2cAddr,
                                uint16_t regAddr,
                                uint8_t regVal,
                                uint32_t numRegs)
{
    int32_t  status = -1;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;

    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        printf("Sensor I2C Handle is NULL \n");
        return -1;
    }

    // 16-bit address is used by most ADAS sensors from OVT/Sony/Onsemi
    status = Board_i2c16BitRegWr(sensorI2cHandle, i2cAddr, regAddr, &regVal, numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        printf("Error : I2C Timeout while writing 0x%x to SENSOR_NAME_UPPERCASE register 0x%x \n", regVal, regAddr);
    }

    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms)
{
    int32_t status = 0;
    uint8_t count = 0;

    p_ae_dynPrms->targetBrightnessRange.min = 45;
    p_ae_dynPrms->targetBrightnessRange.max = 55;
    p_ae_dynPrms->targetBrightness = 50;
    p_ae_dynPrms->threshold = 5;
    p_ae_dynPrms->exposureTimeStepSize = 1;
    p_ae_dynPrms->enableBlc = 0;

    /* 16.666ms */
    p_ae_dynPrms->exposureTimeRange[count].min = 16666;
    p_ae_dynPrms->exposureTimeRange[count].max = 16666;

    /* sensor gain: 1x gain (1024) */
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 1024;

    /* digital 1x gain (256) */
    p_ae_dynPrms->digitalGainRange[count].min = 256;    /* not used */
    p_ae_dynPrms->digitalGainRange[count].max = 256;    /* not used */
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_GetWBPrgFxn(uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms)
{
    int32_t  status = 0;

    p_awb_dynPrms->redGainRange.min = 512;
    p_awb_dynPrms->redGainRange.max = 512;

    p_awb_dynPrms->greenGainRange.min = 512;
    p_awb_dynPrms->greenGainRange.max = 512;

    p_awb_dynPrms->blueGainRange.min = 512;
    p_awb_dynPrms->blueGainRange.max = 512;

    p_awb_dynPrms->sensor_pre_gain = 0;

    printf("SENSOR_NAME_UPPERCASE_GetWBPrgFxn: sensor_pre_gain = %d \n", p_awb_dynPrms->sensor_pre_gain);
    return (status);
}

static int32_t SENSOR_NAME_UPPERCASE_SetAwbParams(void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms)
{
    int32_t status = 0;

    /* TODO */

    return (status);
}
