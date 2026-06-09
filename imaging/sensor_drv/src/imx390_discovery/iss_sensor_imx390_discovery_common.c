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
 * @file        iss_sensor_imx390_discovery_common.c
 * @brief       Common methods shared by different types of cameras
 * @details     ***************************** 
 * @copyright   Copyright (C) 2024 Define Design Deploy Corp. All Rights Reserved.
 */
#include "iss_sensor_imx390_discovery.h"
#include "imx390_discovery_serdes_config.h"

/*
 * \brief DCC Parameters of IMX390
 */
//IssCapture_CmplxIoLaneCfg           imx390Csi2CmplxIoLaneCfg;

static uint16_t sp1hGainRegValueOld[ISS_SENSORS_MAX_CHANNEL];
static uint16_t redGain_prev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t greenGain_prev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t blueGain_prev[ISS_SENSORS_MAX_CHANNEL];

static uint8_t i2cAddrMisc[ISS_SENSORS_MAX_CHANNEL] = {
    MISC_0_I2C_ALIAS, MISC_1_I2C_ALIAS, MISC_2_I2C_ALIAS, MISC_3_I2C_ALIAS,
    MISC_4_I2C_ALIAS, MISC_5_I2C_ALIAS, MISC_6_I2C_ALIAS, MISC_7_I2C_ALIAS,
#if defined(SOC_J784S4) || defined(SOC_J742S2)
    MISC_8_I2C_ALIAS, MISC_9_I2C_ALIAS, MISC_10_I2C_ALIAS, MISC_11_I2C_ALIAS
#endif
    };

IssSensorConfig     imx390DiscoverySensorRegConfigLinear = {
    NULL,                               /*desCfgPreScript*/
    ub953SerCfg_D3IMX390_DISCOVERY,     /*serCfgPreScript*/
    iMX390LinearConfig,                 /*sensorCfgPreScript*/
    NULL,                               /*desCfgPostScript*/
    NULL,                               /*serCfgPostScript*/
    NULL,                               /*sensorCfgPostScript*/
};

IssSensorConfig     imx390DiscoverySensorCfgWdr = {
    NULL,                               /*desCfgPreScript*/
    ub953SerCfg_D3IMX390_DISCOVERY,     /*serCfgPreScript*/
    iMX390WdrConfig,                    /*sensorCfgPreScript*/
    NULL,                               /*desCfgPostScript*/
    NULL,                               /*serCfgPostScript*/
    NULL,                               /*sensorCfgPostScript*/
};

IssSensorConfig     imx390DiscoverySensorCfgWdr60fps = {
    NULL,                               /*desCfgPreScript*/
    ub953SerCfg_D3IMX390_DISCOVERY,     /*serCfgPreScript*/
    iMX390Wdr60fpsConfig,               /*sensorCfgPreScript*/
    NULL,                               /*desCfgPostScript*/
    NULL,                               /*serCfgPostScript*/
    NULL,                               /*sensorCfgPostScript*/
};

IssSensorFxns           imx390DiscoSensorFxns = {
    NULL,
    IMX390_Disco_Config,
    IMX390_Disco_StreamOn,
    IMX390_Disco_StreamOff,
    IMX390_Disco_PowerOn,
    IMX390_Disco_PowerOff,
    IMX390_Disco_GetExpParams,
    IMX390_Disco_SetAeParams,
    IMX390_Disco_GetDccParams,
    IMX390_Disco_InitAewbConfig,
    IMX390_Disco_GetIspConfig,
    IMX390_Disco_ReadWriteReg,
    IMX390_Disco_GetExpPrgFxn,
    IMX390_Disco_Deinit,
    IMX390_Disco_GetWBPrgFxn,
    IMX390_Disco_SetAwbParams
};

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */
static int32_t IMX390_Sensor_RegConfig(uint32_t i2cInstId, uint8_t sensorI2cAddr, I2cParams *sensorCfg, uint16_t sensor_cfg_script_len)
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

        appLogPrintf(" Configuring IMX390 imager 0x%x.. Please wait till it finishes \n", sensorI2cAddr);
        while(regCnt<sensor_cfg_script_len)
        {
            {
                int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);
                if(temp_status != 0) {
                    status = temp_status;
                }
            }

            if (0 != status)
            {
                appLogPrintf(" \n \n IMX390: Sensor Reg Write Failed for regAddr 0x%x \n \n", regAddr);
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
        /*Wait 100ms after the init is done*/
        appLogWaitMsecs(100);
        appLogPrintf(" IMX390 config done\n");
    }
    else
    {
        appLogPrintf(" IMX390 config script is NULL \n");
    }
    return status;
}

static uint32_t imx390FeaturesEnabled;
int32_t IMX390_Disco_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    int32_t status = 0;
    uint32_t i2cInstId;
    uint16_t sensor_cfg_script_len = 0;
    I2cParams *sensorCfg = NULL;
    I2cParams *serCfg = NULL;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint8_t sensorI2Caddr = IMX390_DISCOVERY_I2C_ADDR;

    pCreatePrms = pSenHandle->createPrms;

    if(sensor_features_requested == (sensor_features_requested & (uint32_t)ISS_SENSOR_IMX390_DISCOVERY_FEATURES))
    {
        imx390FeaturesEnabled= sensor_features_requested;
        i2cInstId = pCreatePrms->i2cInstId;

        if((uint32_t)ISS_SENSOR_FEATURE_CFG_UC1 == (sensor_features_requested & (uint32_t)ISS_SENSOR_FEATURE_CFG_UC1))
        {
            serCfg = imx390DiscoverySensorCfgWdr60fps.serCfgPreScript;
            sensorCfg = imx390DiscoverySensorCfgWdr60fps.sensorCfgPreScript;
            sensor_cfg_script_len = IMX390_WDR_CONFIG_60FPS_SIZE;
        }
        else
        {
            if((sensor_features_requested & (uint32_t)ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE) != 0U)
            {
                serCfg = imx390DiscoverySensorCfgWdr.serCfgPreScript;
                sensorCfg = imx390DiscoverySensorCfgWdr.sensorCfgPreScript;
                sensor_cfg_script_len = IMX390_WDR_CONFIG_SIZE;
            }else
            {
                serCfg = imx390DiscoverySensorRegConfigLinear.serCfgPreScript;
                sensorCfg = imx390DiscoverySensorRegConfigLinear.sensorCfgPreScript;
                sensor_cfg_script_len = IMX390_LINEAR_CONFIG_SIZE;
            }
        }

        /*Deserializer config is done in IssSensor_PowerOn, Need to set sensor alias*/
        (void)UB960_SetAlias(chId, 0, 0x18, pCreatePrms->i2cAddrSer[chId]);
        status = UB960_SetAlias(chId, 1, sensorI2Caddr >> 1, pCreatePrms->i2cAddrSensor[chId]);
        //status |= UB960_SetSerAlias(chId, pCreatePrms->i2cAddrSer[chId]);

        //status = UB960_SetSensorAlias(chId, sensorI2Caddr >> 1, pCreatePrms->i2cAddrSer[chId]);
        if(0 != status)
        {
            appLogPrintf("IMX390_Disco_Config Error : UB960_SetSensorAlias for chId %d returned %d \n", chId, status);
        }else
        {
            status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], serCfg);
            if(0 != status)
            {
                appLogPrintf("IMX390_Disco_Config Error : UB953 config failed for camera # %d \n", chId);
            }else
            {
                status = IMX390_Sensor_RegConfig(i2cInstId, pCreatePrms->i2cAddrSensor[chId], sensorCfg, sensor_cfg_script_len);
             }
        }
    }
    else
    {
        appLogPrintf("IMX390_Disco_Config : Error. feature set 0x%x is not supported \n", sensor_features_requested);
        status = -1;
    }
    return (status);
}

int32_t IMX390_Disco_StreamOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
    int8_t ub960InstanceId = getDeserializerInstIdFromChId(chId);

    if(ub960InstanceId >= 0)
    {
        pCreatePrms = pSenHandle->createPrms;

        if((uint32_t)ISS_SENSOR_FEATURE_CFG_UC1 == (imx390FeaturesEnabled& (uint32_t)ISS_SENSOR_FEATURE_CFG_UC1))
        {
            if(pSenHandle->sensorIntfPrms->numCamerasStreaming >= 3U)
            {
                appLogPrintf("IMX390_Disco_StreamOn Error : %d cameras streaming already \n", pSenHandle->sensorIntfPrms->numCamerasStreaming);
                appLogPrintf("IMX390_Disco_StreamOn Error : 60fps mode can support upto 3 cameras because of UB960 b/w limitation \n");
                status = -1;
            }
            else
            {
                i2cInstId = pCreatePrms->i2cInstId;
                sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

                {
                    int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, 0x0, 0x0, 1u);/*ACTIVE*/
                    if(temp_status != 0) {
                        status = temp_status;
                    }
                }
                appLogWaitMsecs(10);
                {
                    int32_t temp_status = enableUB960Streaming(chId);
                    if(temp_status != 0) {
                        status = temp_status;
                    }
                }
            }
        }
        else
        {
            i2cInstId = pCreatePrms->i2cInstId;
            sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

            {
                int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, 0x0, 0x0, 1u);/*ACTIVE*/
                if(temp_status != 0) {
                    status = temp_status;
                }
            }
            appLogWaitMsecs(10);
            {
                int32_t temp_status = enableUB960Streaming(chId);
                if(temp_status != 0) {
                    status = temp_status;
                }
            }
        }
    }
    else
    {
        appLogPrintf("Invalid ub960InstanceId \n");
        status = -1;
    }
    return (status);
}

int32_t IMX390_Disco_StreamOff(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    {
        int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, 0x0, 0x1, 1u);/*STANDBY*/
        if(temp_status != 0) {
            status = temp_status;
        }
    }
    appLogWaitMsecs(10);
    {
        int32_t temp_status = disableUB960Streaming(chId);
        if(temp_status != 0) {
            status = temp_status;
        }
    }
    return status;
}

int32_t IMX390_Disco_PowerOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;

    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
    uint8_t pcalI2cAddr;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;

    sp1hGainRegValueOld[chId] = 0;
    redGain_prev[chId] = 512;
    greenGain_prev[chId] = 512;
    blueGain_prev[chId] = 512;

    appLogPrintf("IMX390_Disco_PowerOn : chId = 0x%x \n", chId);

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];
    pcalI2cAddr = i2cAddrMisc[chId];

    /* Set sensor and PCAL alias */
    (void)UB960_SetAlias(chId, 1, IMX390_DISCOVERY_I2C_ADDR >> 1, sensorI2cAddr);
    (void)UB960_SetAlias(chId, 2, IMX390_PCAL_ADDRESS >> 1, pcalI2cAddr);

    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;
    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);

    /* Reset PCAL */
    uint8_t regVal = 0xFE;
    (void)Board_i2c8BitRegWr(sensorI2cHandle, pcalI2cAddr, 0x03, &regVal, 1, SENSOR_I2C_TIMEOUT);
    regVal = 0x01;
    status = Board_i2c8BitRegWr(sensorI2cHandle, pcalI2cAddr, 0x01, &regVal, 1, SENSOR_I2C_TIMEOUT);

    if(status == 0)
    {
        appLogWaitMsecs(200);

        /* Set serializer alias */
        status = UB960_SetAlias(chId, 0, 0x18, pCreatePrms->i2cAddrSer[chId]);
        if(0 == status)
        {
            status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], ub953SerCfg_D3IMX390_DISCOVERY);
            if (status != 0)
            {
                appLogPrintf("IMX390_Disco_PowerOn : UB953 config failed for camera # %d \n", chId);
            }
        }
        else
        {
            appLogPrintf("IMX390_Probe_Narrow Error: UB960_SetSensorAlias for chId %d returned %d \n", chId, status);
        }
    }
    else
    {
        appLogPrintf("IMX390 Error: Configuring PCAL for Reset line of ch %d returned %d \n", chId, status);
    }

    return status;
}

int32_t IMX390_Disco_PowerOff(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return (0);
}

static uint16_t sp1hGainRegValueOld[ISS_SENSORS_MAX_CHANNEL];
int32_t IMX390_Disco_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    uint16_t regAddr;
    uint16_t cnt;
    uint8_t regValue;
    int32_t status = -1;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
    uint32_t sp1h_again = 0U;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    /* Exp time is fixed to 11ms for LFM. Set Analog Gain Only */

    for (cnt = 0; cnt < ISS_IMX390_GAIN_TBL_SIZE; cnt ++)
    {
        if (pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG] <= gIMX390GainsTable[cnt][0])
        {
            sp1h_again = gIMX390GainsTable[cnt][1];
            break;
        }
    }

    if(sp1hGainRegValueOld[chId] != sp1h_again)
    {
        sp1hGainRegValueOld[chId] = (uint16_t)sp1h_again;

        regAddr = 0x0008;
        regValue = 1;
        status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);
        if(status != 0)
        {
            appLogPrintf("Error writing 0x%x to IMX390 register 0x%x \n", regValue, regAddr);
        }

        regAddr = IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR;
        regValue = (uint8_t)(sp1h_again & 0xFFU);
        status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);
        if(status != 0)
        {
            appLogPrintf("Error writing 0x%x to IMX390 register 0x%x \n", regValue, regAddr);
        }

        regAddr = IMX390_SP1H_ANALOG_GAIN_CONTROL_REG_ADDR_HIGH;
        regValue = (uint8_t)(sp1h_again >> 8);
        status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);
        if(status != 0)
        {
            appLogPrintf("Error writing 0x%x to IMX390 register 0x%x \n", regValue, regAddr);
        }

        regAddr = 0x0008;
        regValue = 0;
        status = IMX390_Disco_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);
        if(status != 0)
        {
            appLogPrintf("Error writing 0x%x to IMX390 register 0x%x \n", regValue, regAddr);
        }
    }
    else
    {
        /*Reduce I2C transactions.
        Do not write to the sensor if register value does not change */
        status = 0;
    }

    return (status);
}

int32_t IMX390_Disco_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    (void)chId;
    (void)pSensorHdl;
    (void)pDccPrms;
    int32_t status = 0;
    return (status);
}

int32_t IMX390_Disco_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;

    pExpPrms->expRatio = ISS_SENSOR_IMX390_DEFAULT_EXP_RATIO;

    return (status);
}

void IMX390_Disco_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

void IMX390_Disco_GetIspConfig (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

void IMX390_Disco_Deinit (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

int32_t IMX390_Disco_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg)
{
    int32_t status = 0;

    uint8_t regValue = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if (1u == readWriteFlag)
    {
        /*write*/
        regValue = (uint8_t)pReg->nRegValue;
        status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId,
            pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, regValue, 1u);
    }
    else
    {
        /*read*/
        status = IMX390_Disco_ReadReg((uint8_t)pCreatePrms->i2cInstId,
            pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, &regValue, 1u);

        if (0 == status)
        {
            pReg->nRegValue = regValue;
        }
    }
    return (status);
}

int32_t IMX390_Disco_ReadReg(uint8_t     i2cInstId,
                            uint8_t         i2cAddr,
                            uint16_t        regAddr,
                            uint8_t         *regVal,
                            uint32_t        numRegs)
{
    (void)i2cInstId;
    int32_t  status = -1;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;
    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL != sensorI2cHandle)
    {
        status = Board_i2c16BitRegRd(sensorI2cHandle, i2cAddr, regAddr, regVal, (uint8_t)numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
        if(0 != status)
        {
            appLogPrintf("Error : I2C Timeout while reading from IMX390 register 0x%x \n", regAddr);
        }
    }
    else
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
    }
    return (status);
}

int32_t IMX390_Disco_WriteReg(uint8_t    i2cInstId,
                               uint8_t    i2cAddr,
                               uint16_t   regAddr,
                               uint8_t    regVal,
                               uint32_t   numRegs)
{
    (void)i2cInstId;
    int32_t  status = -1;
    uint8_t  regValue;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255U;
    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL != sensorI2cHandle)
    {
        regValue = regVal;
        status = Board_i2c16BitRegWr(sensorI2cHandle, i2cAddr, regAddr, &regValue, (uint8_t)numRegs, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
        if(0 != status)
        {
            appLogPrintf("Error : I2C Timeout while writing 0x%x to IMX390 register 0x%x \n", regVal, regAddr);
        }
    }
    else
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
    }

    return (status);
}

int32_t IMX390_Disco_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t  status = 0;
    uint8_t count = 0;

    p_ae_dynPrms->targetBrightnessRange.min = 40;
    p_ae_dynPrms->targetBrightnessRange.max = 50;
    p_ae_dynPrms->targetBrightness = 45;
    p_ae_dynPrms->threshold = 1;
    p_ae_dynPrms->enableBlc = 1;
    p_ae_dynPrms->exposureTimeStepSize = 1;

    p_ae_dynPrms->exposureTimeRange[count].min = 11000;
    p_ae_dynPrms->exposureTimeRange[count].max = 11000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 8192;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}

int32_t IMX390_Disco_GetWBPrgFxn(uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms)
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

    p_awb_dynPrms->sensor_pre_gain = 0;

    appLogPrintf("IMX390_Disco_GetWBPrgFxn: sensor_pre_gain = %d \n", p_awb_dynPrms->sensor_pre_gain);
    return (status);
}

/* PENDING - METRICS.E.Number_of_paths_PATH - Warning - Number of paths PATH: IMX390_Disco_SetAwbParams 424.99993>400 */
int32_t IMX390_Disco_SetAwbParams(void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms)
{
    int32_t status = 0;
    uint16_t regAddr;
    uint16_t regValue;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if(redGain_prev[chId] != pWbPrms->rGain[0])
    {
        redGain_prev[chId] = (uint16_t)pWbPrms->rGain[0];
        regAddr = IMX390_RED_GAIN_REG_L;
        regValue = (uint16_t)((pWbPrms->rGain[0]>>IMX390_ISP_GAIN_OFFSET) & 0xffU);/*Sensor gain is Q8, ISP gain is Q10*/
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
        regAddr = IMX390_RED_GAIN_REG_H;
        regValue = (uint16_t)((pWbPrms->rGain[0]>>IMX390_ISP_GAIN_OFFSET) >> 8);/*Sensor gain is Q8, ISP gain is Q10*/
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }

    if(greenGain_prev[chId] != pWbPrms->gGain[0])
    {
        greenGain_prev[chId] = (uint16_t)pWbPrms->gGain[0];
        regAddr = IMX390_GREEN1_GAIN_REG_L;
        regValue = (uint16_t)((pWbPrms->gGain[0]>>IMX390_ISP_GAIN_OFFSET) & 0xffU);/*Sensor gain is Q8, ISP gain is Q10*/
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
        regAddr = IMX390_GREEN2_GAIN_REG_L;
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }

        regAddr = IMX390_GREEN1_GAIN_REG_H;
        regValue = (uint16_t)((pWbPrms->gGain[0]>>IMX390_ISP_GAIN_OFFSET) >> 8);/*Sensor gain is Q8, ISP gain is Q10*/
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
        regAddr = IMX390_GREEN2_GAIN_REG_H;
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }

    if(blueGain_prev[chId] != pWbPrms->bGain[0])
    {
        blueGain_prev[chId] = (uint16_t)pWbPrms->bGain[0];
        regAddr = IMX390_BLUE_GAIN_REG_L;
        regValue = (uint16_t)((pWbPrms->bGain[0]>>IMX390_ISP_GAIN_OFFSET) & 0xffU);/*Sensor gain is Q8, ISP gain is Q10*/
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
        regAddr = IMX390_BLUE_GAIN_REG_H;
        regValue = (uint16_t)((pWbPrms->bGain[0]>>IMX390_ISP_GAIN_OFFSET) >> 8);/*Sensor gain is Q8, ISP gain is Q10*/
        {
            int32_t temp_status = IMX390_Disco_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, (uint8_t)regValue, 1u);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }

    return (status);
}
