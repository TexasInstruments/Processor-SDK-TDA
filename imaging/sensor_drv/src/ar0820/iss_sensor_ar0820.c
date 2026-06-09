/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#include "iss_sensor_ar0820.h"
#include "ar0820_serdes_config.h"

#define AR0820_MAX_COARSE_AGAIN (8U)
#define AR0820_MAX_FINE_AGAIN (2U)

static IssSensor_CreateParams  ar0820CreatePrms = {
    SENSOR_ONSEMI_AR0820_UB953_LI,     /*sensor name*/
    0x6,                             /*i2cInstId*/
    I2C_ADDR_SENSOR,                /*i2cAddrSensor*/
    I2C_ADDR_SER,                   /*i2cAddrSer*/
    /*IssSensor_Info*/
    {
        {
            AR0820_OUT_WIDTH,               /*width*/
            AR0820_OUT_HEIGHT,              /*height*/
            1,                              /*num_exposures*/
            (vx_bool)vx_false_e,                     /*line_interleaved*/
            {
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0]*/
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0]*/
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0]*/
            },
            0,                              /*meta_height_before*/
            0,                              /*meta_height_after*/
        },
        (uint32_t)ISS_SENSOR_AR0820_FEATURES,     /*features*/
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_AEWB,  /*aewbMode*/
        30,                             /*fps*/
        4,                              /*numDataLanes*/
        {1, 2, 3, 4},                   /*dataLanesMap*/
        {0, 0, 0, 0},                   /*dataLanesPolarity*/
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS, /*csi_laneBandSpeed*/
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,           /* fpdLink */
        AR0820_I2C_ADDR,                /* senI2cAddr */
        0x30,                           /* serI2cAddr */
    },
#endif
    1,                                  /*numChan*/
    820,                                /*dccId*/
};

static IssSensorFxns           ar0820SensorFxns = {
    AR0820_Probe,
    AR0820_Config,
    AR0820_StreamOn,
    AR0820_StreamOff,
    AR0820_PowerOn,
    AR0820_PowerOff,
    AR0820_GetExpParams,
    AR0820_SetAeParams,
    AR0820_GetDccParams,
    AR0820_InitAewbConfig,
    AR0820_GetIspConfig,
    AR0820_ReadWriteReg,
    AR0820_GetExpPrgFxn,
    AR0820_deinit,
    NULL,
    NULL
};

static IssSensorIntfParams     ar0820SensorIntfPrms = {
    0,             /*sensorBroadcast*/
    0,             /*enableFsin*/
    0,               /*numCamerasStreaming*/
};

IssSensorConfig     ar0820SensorRegConfigLinear = {
    NULL,     /*desCfgPreScript*/
    ub953SerCfg_AR0820,     /*serCfgPreScript*/
    ar0820LinearConfig,        /*sensorCfgPreScript*/
    ub960AR0820DesCSI2Enable,        /*desCfgPostScript*/
    NULL,                    /*serCfgPostScript*/
    NULL,                    /*sensorCfgPostScript*/
};

IssSensorConfig     ar0820SensorRegConfigWdr = {
    NULL,     /*desCfgPreScript*/
    ub953SerCfg_AR0820,     /*serCfgPreScript*/
    ar0820WdrConfig,        /*sensorCfgPreScript*/
    ub960AR0820DesCSI2Enable,        /*desCfgPostScript*/
    NULL,                    /*serCfgPostScript*/
    NULL,                    /*sensorCfgPostScript*/
};

IssSensors_Handle ar0820SensorHandle = {
    1,                                 /*isUsed*/
    &ar0820CreatePrms,                /*CreatePrms*/
    &ar0820SensorFxns,                /*SensorFxns*/
    &ar0820SensorIntfPrms,            /*SensorIntfPrms*/
};

/*
 * \brief DCC Parameters of AR0820
 */
IssCapture_CmplxIoLaneCfg           ar0820Csi2CmplxIoLaneCfg;

int32_t IssSensor_AR0820_Init(void)
{
    int32_t status;
    status = IssSensor_Register(&ar0820SensorHandle);
    if(0 != status)
    {
        appLogPrintf("IssSensor_AR0820_Init failed \n");
    }

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t AR0820_Probe(uint32_t chId, void *pSensorHdl)
{
    int32_t status = -1;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
    uint16_t chipIdRegAddr = AR0820_CHIP_ID_REG_ADDR;
    uint16_t chipIdRegValueRead = 0xABCD;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint8_t count=0;
    uint8_t max_retries = 5;
    I2cParams    * serCfg = NULL;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    status = UB960_SetSensorAlias(chId, AR0820_I2C_ADDR, pCreatePrms->i2cAddrSer[chId]);
    if(0 != status)
    {
        appLogPrintf("AR0820_Probe : UB960_SetSensorAlias for chId %d returned %d \n", chId, status);
    }
    else
    {
        appLogWaitMsecs(100);

        serCfg = ar0820SensorRegConfigWdr.serCfgPreScript;
        /*The code assumes that I2C instance is the same for sensor and serializer*/
        if(NULL != serCfg)
        {
            (void)ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], serCfg);
        }

        appLogWaitMsecs(100);

        /*Read chip ID to detect if the sensor can be detected*/

        while( (chipIdRegValueRead != (uint16_t)AR0820_CHIP_ID_REG_VAL) && (count < max_retries))
        {
            status = AR0820_ReadReg((uint8_t)i2cInstId, sensorI2cAddr, chipIdRegAddr, &chipIdRegValueRead, 1U);
            if(status == 0)
            {
                if(chipIdRegValueRead == (uint16_t)AR0820_CHIP_ID_REG_VAL)
                {
#ifdef ENABLE_DEBUG_IMAGING
                    appLogPrintf("AR0820_Probe SUCCESS : Read expected value 0x%x at chip ID register 0x%x \n", AR0820_CHIP_ID_REG_VAL, chipIdRegAddr);
#endif
                }
                else
                {
                    status = -1;
                    appLogPrintf("AR0820_Probe : 0x%x read at chip ID register 0x%x. Expected 0x%x \n", chipIdRegValueRead, chipIdRegAddr, AR0820_CHIP_ID_REG_VAL);
                    appLogPrintf("AR0820 Probe Failed \n");
                    appLogWaitMsecs(100);
                }
            }
            else
            {
                appLogPrintf("AR0820_Probe Error : Failed reading CHIP_ID register 0x%x \n", chipIdRegAddr);
            }
            count++;
        }

        if(0 == status)
        {
            chipIdRegAddr = 0x31FE;/*Sensor Revision*/
            status = AR0820_ReadReg((uint8_t)i2cInstId, sensorI2cAddr, chipIdRegAddr, &chipIdRegValueRead, 1U);
            if(status == 0)
            {
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("AR0820 Sensor Revision ID = 0x%x \n", chipIdRegValueRead);
#endif
            }
            else
            {
                appLogPrintf("AR0820_Probe Error : Failed reading Rev ID register 0x%x \n", chipIdRegAddr);
            }
        }
    }
    return (status);
}

/*Flag indicating operating mode 0:Linear, 1:WDR*/
static uint8_t ar0820_op_mode = 0;

static int32_t AR0820_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    int32_t status = 0;
    uint32_t regCnt, i2cInstId;
    uint8_t sensorI2cAddr;
    uint16_t regAddr;
    uint16_t regValue;
    uint16_t delayMilliSec;
    uint16_t sensor_cfg_script_len = 0;
    I2cParams *sensorCfg = NULL;
    I2cParams *deserCfg = NULL;
    I2cParams *serCfg = NULL;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    int8_t ub960InstanceId = getDeserializerInstIdFromChId(chId);

    if(ub960InstanceId < 0)
    {
        appLogPrintf("Invalid ub960InstanceId \n");
        status = 0xFF;
    }
    else
    {
        pCreatePrms = pSenHandle->createPrms;

        if(sensor_features_requested != (sensor_features_requested & (uint32_t)ISS_SENSOR_AR0820_FEATURES))
        {
            appLogPrintf("AR0820_Config : Error. feature set 0x%x is not supported \n", sensor_features_requested);
            status = -1;
        }
        else
        {
            i2cInstId = pCreatePrms->i2cInstId;
            sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

            if((sensor_features_requested & (uint32_t)ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE) != 0U)
            {
                deserCfg = ar0820SensorRegConfigWdr.desCfgPreScript;
                serCfg = ar0820SensorRegConfigWdr.serCfgPreScript;
                sensorCfg = ar0820SensorRegConfigWdr.sensorCfgPreScript;
                sensor_cfg_script_len = AR0820_WDR_CONFIG_SIZE;
                ar0820_op_mode = 1U;
            }else
            {
                deserCfg = ar0820SensorRegConfigLinear.desCfgPreScript;
                serCfg = ar0820SensorRegConfigLinear.serCfgPreScript;
                sensorCfg = ar0820SensorRegConfigLinear.sensorCfgPreScript;
                sensor_cfg_script_len = AR0820_LINEAR_CONFIG_SIZE;
                ar0820_op_mode = 0U;
            }

            if(NULL != deserCfg)
            {
                (void)IssSensor_cfgDesScript(deserCfg, ub960InstanceId);
            }

            /*Deserializer config is done in IssSensor_PowerOn, Need to set sensor alias*/
            status = UB960_SetSensorAlias(chId, AR0820_I2C_ADDR, pCreatePrms->i2cAddrSer[chId]);
            if(0 != status)
            {
                appLogPrintf("AR0820_Config : UB960_SetSensorAlias for chId %d returned %d \n", chId, status);
            }

            if(0 == status)
            {
                appLogWaitMsecs(50);

            /*The code assumes that I2C instance is the same for sensor and serializer*/
                if(NULL != serCfg)
                {
                    status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], serCfg);
                }
                if(0 == status)
                {
                    appLogWaitMsecs(50);

#ifndef _TEST_PATTERN_ENABLE_
                    if(NULL != sensorCfg)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf(" Configuring AR0820 imager .. Please wait till it finishes \n");
#endif
                        for(regCnt=0; regCnt<sensor_cfg_script_len; regCnt++)
                        {
                            regAddr  = sensorCfg[regCnt].nRegAddr;
                            regValue = sensorCfg[regCnt].nRegValue;
                            delayMilliSec = (uint16_t)sensorCfg[regCnt].nDelay;

                            status = AR0820_WriteReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, regValue, 1u);
                            if (0 != status)
                            {
                                appLogPrintf(" \n \n AR0820: Sensor Reg Write Failed for regAddr 0x%x \n \n", regAddr);
                            }

                            if (delayMilliSec > 0U)
                            {
                                appLogWaitMsecs(delayMilliSec);
                            }
                        }
                        /*Wait 10ms after the init is done*/
                        appLogWaitMsecs(10);
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf(" AR0820 config done\n");
#endif
                    }
                    else
                    {
                        appLogPrintf(" AR0820 config script is NULL \n");
                    }
#endif

                }

            }
        }
    }

    return (status);
}

static int32_t AR0820_StreamOn(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    int8_t ub960InstanceId = getDeserializerInstIdFromChId(chId);

    if(ub960InstanceId < 0)
    {
        appLogPrintf("Invalid ub960InstanceId \n");
        status = 0xFF;
    }
    else
    {
        pCreatePrms = pSenHandle->createPrms;

        /*Start Streaming from sensor*/
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], 0x301A, 0x005C, 1u);
        if(status == 0 )
        {
            appLogWaitMsecs(10);
            status = IssSensor_cfgDesScript(ub960AR0820DesCSI2Enable, ub960InstanceId);
        }
    }
    return (status);
}

static int32_t AR0820_StreamOff(uint32_t chId, void *pSensorHdl)
{
    int32_t status = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    int8_t ub960InstanceId = getDeserializerInstIdFromChId(chId);

    if(ub960InstanceId < 0)
    {
        appLogPrintf("Invalid ub960InstanceId \n");
        status = 0xFF;
    }
    else
    {
        pCreatePrms = pSenHandle->createPrms;

        /*Stop Streaming from sensor*/
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], 0x301A, 0x0058, 1u);
        if(status == 0)
        {
            appLogWaitMsecs(10);
            status = IssSensor_cfgDesScript(ub960AR0820DesCSI2Disable, ub960InstanceId);
        }
    }
    return (status);
}

static int32_t AR0820_PowerOn(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return 0;
}

static int32_t AR0820_PowerOff(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return (0);
}

static uint16_t exp_time_to_rows(uint32_t integration_time, uint32_t wdr_mode)
{
    uint16_t num_rows = 0;
	uint16_t llpck = LLP;
	uint16_t pclk_mhz = PCLK_MHz;
	uint16_t num_exp = 0U;
	uint16_t num_adc = NUM_ADC;
	uint16_t fit_numerator;
	uint16_t fit_denominator;

	/* Use integer arithmetic with scaling factor to avoid floating point */
	/* Scaling factor: 1000000 (1 million) for precision */
	int64_t mit_scaled;
	int64_t fit_scaled;
    int64_t calculated_rows;
	int64_t temp1_scaled, temp2_scaled;
	int64_t integration_time_scaled;

	/* Calculate MIT scaled: (MIT * 1000000) / pclk_mhz */
	mit_scaled = ((int64_t)MIT * 1000000) / (int64_t)pclk_mhz;

	/* Calculate FIT scaled: ((164 - FIT) * 1000000) / (2 * pclk_mhz) */
	fit_numerator = 164U - FIT;
	fit_denominator = 2U * pclk_mhz;
	fit_scaled = ((int64_t)fit_numerator * 1000000) / (int64_t)fit_denominator;

    if(wdr_mode == 0U)
    {
    	num_exp = 1U;
    }
	else
    {
    	num_exp = NUM_EXP_WDR;
    }

	/* Scale integration_time for precision */
	integration_time_scaled = (int64_t)integration_time * 1000000;

	/* temp1 = (integration_time - fit) / llpck */
	temp1_scaled = (integration_time_scaled - fit_scaled) / (int64_t)llpck;

	/* temp2 = (temp1 - mit) / (num_exp * num_adc) */
	temp2_scaled = (temp1_scaled - mit_scaled) / ((int64_t)num_exp * (int64_t)num_adc);

	/* temp3 = temp2 * pclk_mhz, then descale */
    calculated_rows = ((temp2_scaled * (int64_t)pclk_mhz) + 500000) / 1000000;
    num_rows = (uint16_t)calculated_rows;

    return num_rows;
}

static uint16_t expTimePrev[ISS_SENSORS_MAX_CHANNEL];
static uint16_t analogGainPrev[ISS_SENSORS_MAX_CHANNEL];
static int32_t AR0820_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = 0;
    uint16_t regAddr;
    uint16_t regValue;
    uint32_t analogGain = 0;
    uint8_t coarseGain = 0;
    uint8_t tmp8 = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if (expTimePrev[chId] != pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG])
    {
        expTimePrev[chId] = (uint16_t)pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG];
        regAddr = AR0820_GROUP_HOLD_REG_ADDR;
        regValue = 0x1;
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, regValue, 1u);
        if(status < 0)
        {
            appLogPrintf("AR0820_SetAeParams : Failed writing 0x%x to register 0x%x\n", regValue, regAddr);
        }

        if(0U == ar0820_op_mode)
        {
            /*Linear Mode*/
            regAddr = SENSOR_AR0820_EXP_T1;
            regValue = exp_time_to_rows(pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG], 0U);
            status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, regValue, 1u);
            if(status < 0)
            {
                appLogPrintf("AR0820_SetAeParams : Failed writing 0x%x to register 0x%x\n", regValue, regAddr);
            }
        }
        else
        {
            /*WDR mode*/
            uint32_t t1_exp_time = pExpPrms->exposureTime[ISS_SENSOR_EXPOSURE_LONG];

            regAddr = SENSOR_AR0820_EXP_T1;
            regValue = exp_time_to_rows(t1_exp_time, 1U);
            status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, regValue, 1u);
            if(status < 0)
            {
               appLogPrintf("AR0820_SetAeParams : Failed writing 0x%x to register 0x%x\n", regValue, regAddr);
            }
        }
    }

    if (analogGainPrev[chId] != pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG])
    {
        analogGainPrev[chId] = (uint16_t)pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG];
        analogGain = pExpPrms->analogGain[ISS_SENSOR_EXPOSURE_LONG];

        /*Calculate coarse gain*/
        if(analogGain < 2048U)
        {
            coarseGain = 1U;
            tmp8 = 0;
        }else if(analogGain < 4096U)
        {
            coarseGain = 2U;
            tmp8 = 1;
        }else if(analogGain < 8192U)
        {
            coarseGain = 4U;
            tmp8 = 2U;
        }else
        {
            coarseGain = 8U;
            tmp8 = 3U;
        }

        regAddr = SENSOR_AR0820_COARSE_ANALOG_GAIN;
        regValue = (((uint16_t)tmp8)<<12U) | (((uint16_t)tmp8)<<8U) | (((uint16_t)tmp8)<<4U) | ((uint16_t)tmp8);
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, regValue, 1u);
        if(status < 0)
        {
           appLogPrintf("AR0820_SetAeParams : Failed writing 0x%x to register 0x%x\n", regValue, regAddr);
        }

        /*Calculate fine gain*/
        regAddr = SENSOR_AR0820_FINE_ANALOG_GAIN;
        tmp8 = (uint8_t)((16U*((analogGain/coarseGain)-1024U))/1024U);
        regValue = (((uint16_t)tmp8)<<12U) | (((uint16_t)tmp8)<<8U) | (((uint16_t)tmp8)<<4U) | ((uint16_t)tmp8);
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, regValue, 1u);
        if(status < 0)
        {
           appLogPrintf("AR0820_SetAeParams : Failed writing 0x%x to register 0x%x\n", regValue, regAddr);
        }

        regAddr = AR0820_GROUP_HOLD_REG_ADDR;
        regValue = 0x0;
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId, pCreatePrms->i2cAddrSensor[chId], regAddr, regValue, 1u);
        if(status < 0)
        {
            appLogPrintf("AR0820_SetAeParams : Failed writing 0x%x to register 0x%x\n", regValue, regAddr);
        }
    }

    return (status);
}

static int32_t AR0820_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    (void)chId;
    (void)pSensorHdl;
    (void)pDccPrms;
    int32_t status = 0;
    return (status);
}

static int32_t AR0820_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;

    pExpPrms->expRatio = 256;

    return (status);
}

static void AR0820_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void AR0820_GetIspConfig (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void AR0820_deinit (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static int32_t AR0820_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg)
{
    int32_t status = 0;

    uint16_t regValue = 0;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    if (1u == readWriteFlag)
    {
        /*write*/
        regValue = pReg->nRegValue;
        status = AR0820_WriteReg((uint8_t)pCreatePrms->i2cInstId,
            pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, regValue, 1u);
    }
    else
    {
        /*read*/
        status = AR0820_ReadReg((uint8_t)pCreatePrms->i2cInstId,
            pCreatePrms->i2cAddrSensor[chId], pReg->nRegAddr, &regValue, 1u);

        if (0 == status)
        {
            pReg->nRegValue = regValue;
        }
    }
    return (status);
}

static int32_t AR0820_ReadReg(uint8_t     i2cInstId,
                            uint8_t         i2cAddr,
                            uint16_t        regAddr,
                            uint16_t         *regVal,
                            uint32_t        numRegs)
{
    (void)i2cInstId;
    int32_t  status = -1;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = BOARD_I2C_REG_ADDR_MSB_FIRST;
    uint8_t   readReg8_High = 0xAB;
    uint8_t   readReg8_Low = 0xCD;
    uint32_t count;

    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
        status = -1;
    }
    else
    {
        for(count = 0;count<numRegs;count++)
        {
            /*Read a 16-bit value as two 8-byte values*/
            status = Board_i2c16BitRegRd(sensorI2cHandle, i2cAddr, regAddr, &readReg8_High, 1U, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
            if(status == 0)
            {
                status = Board_i2c16BitRegRd(sensorI2cHandle, i2cAddr, (uint16_t)(regAddr+1U), &readReg8_Low, 1U, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
            }
        }
        *regVal = (uint16_t)(((((uint16_t)readReg8_High) << 8) & 0xFF00U) | readReg8_Low);
    }

    return (status);
}

static int32_t AR0820_WriteReg(uint8_t    i2cInstId,
                             uint8_t      i2cAddr,
                             uint16_t     regAddr,
                             uint16_t     regVal,
                             uint32_t     numRegs)
{
    (void)i2cInstId;
    (void)numRegs;
    int32_t  status = 0;
    int16_t  ret = 0;
    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = 255u;
    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    uint8_t   rawRegVal[4];
    I2C_Transaction transaction;

    #if defined (MCU_PLUS_SDK)
    I2C_Transaction_init(&transaction);
    #else
    I2C_transactionInit(&transaction);
    #endif

    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("Sensor I2C Handle is NULL \n");
        status = -1;
    }
    else
    {
        #if defined(MCU_PLUS_SDK)
        transaction.targetAddress = i2cAddr;
        #else
        transaction.slaveAddress = i2cAddr;
        #endif
        transaction.writeBuf     = rawRegVal;
        transaction.writeCount   = 4;
        transaction.readBuf      = NULL;
        transaction.readCount    = 0;

        /*Assume numRegs = 1 */
        {
              /* Convert Registers address and value into 8bit array */
              rawRegVal[0U] = (uint8_t) ((regAddr >> 8U) & (uint8_t) 0xFF);
              rawRegVal[1U] = (uint8_t) ((regAddr >> 0U) & (uint8_t) 0xFF);
              rawRegVal[2U] = (uint8_t) ((regVal >> 8U) & (uint8_t) 0xFF);
              rawRegVal[3U] = (uint8_t) ((regVal >> 0U) & (uint8_t) 0xFF);
              ret = (int16_t)I2C_transfer(sensorI2cHandle, &transaction);
              if(ret != I2C_STS_SUCCESS)
              {
                    appLogPrintf("AR0820_WriteReg : Error writing to register 0x%x \n ", regAddr);
                    status = -1;
              }
        }
    }

    return (status);
}

static int32_t AR0820_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    uint8_t count = 0;
    /*Max 38ms Exposure time since sensor is configured at 26 fps*/
    p_ae_dynPrms->targetBrightnessRange.min = DEFAULT_TARGET_BRIGHTNESS_MIN;
    p_ae_dynPrms->targetBrightnessRange.max = DEFAULT_TARGET_BRIGHTNESS_MAX;
    p_ae_dynPrms->targetBrightness = DEFAULT_TARGET_BRIGHTNESS;
    p_ae_dynPrms->threshold = DEFAULT_TARGET_THRESHOLD;
    p_ae_dynPrms->exposureTimeStepSize = DEFAULT_EXPOSURE_TIME;
    p_ae_dynPrms->enableBlc = DEFAULT_ENABLE_BLC;

    p_ae_dynPrms->exposureTimeRange[count].min = 100;
    p_ae_dynPrms->exposureTimeRange[count].max = 38000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 1024;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->exposureTimeRange[count].min = 38000;
    p_ae_dynPrms->exposureTimeRange[count].max = 38000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 15872;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}

#ifdef _AR0820_DEBUG_
typedef struct 
{
    uint32_t regAddr;
    char regDesc[32];
}sensorDebug;

int32_t AR0820_Debug(uint32_t chId, void *pSensorHdl)
{
    int32_t status = -1;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
    uint16_t regAddr;
    uint16_t regValueRead = 0xABCD;
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint16_t count=0;
    uint16_t  numDbgReg = 0;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];

    const sensorDebug ar0820RegReadList[] = {
            {0x2000, "FRAME_COUNT2"},
            {0x2002, "FRAME_COUNT"},
            {0x2008, "FRAME_STATUS"},
        };
    numDbgReg = sizeof(ar0820RegReadList)/sizeof(ar0820RegReadList[0]);

    for(count = 0; count < numDbgReg; count++ )
    {
        regAddr = ar0820RegReadList[count].regAddr;
        AR0820_ReadReg((uint8_t)i2cInstId, sensorI2cAddr, regAddr, &regValueRead, 1u);
        appLogPrintf("AR0820 Debug Reg %d : %s, Addr 0x%x, Value 0x%x \n", count, ar0820RegReadList[count].regDesc, regAddr, regValueRead);
    }

    return (status);
}
#endif //_AR0820_DEBUG_
