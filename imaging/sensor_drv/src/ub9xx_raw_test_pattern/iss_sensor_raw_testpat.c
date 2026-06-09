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
#include "iss_sensor_raw_testpat.h"
#include "raw_testpat_serdes_config.h"

static IssSensor_CreateParams  raw_testpat_CreatePrms = {
    UB9XX_RAW_TESTPAT,     /*sensor name*/
    0x6,                             /*i2cInstId*/
    {
        SENSOR_0_I2C_ALIAS, 0, 0, 0, 0, 0, 0, 0
        #if defined(SOC_J784S4) || defined(SOC_J742S2)
        , 0, 0, 0, 0
        #endif
    },/*i2cAddrSensor*/
    {
        SER_0_I2C_ALIAS, 0, 0, 0, 0, 0, 0, 0
        #if defined(SOC_J784S4) || defined(SOC_J742S2)
        , 0, 0, 0, 0
        #endif
    },/*i2cAddrSer*/
    /*IssSensor_Info*/
    {
        {
            RAWTESTPAT_OUT_WIDTH,               /*width*/
            RAWTESTPAT_OUT_HEIGHT,              /*height*/
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
        (uint32_t)ISS_SENSOR_RAWTESTPAT_FEATURES,     /*features*/
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_NONE,  /*aewbMode*/
        30,                             /*fps*/
        4,                              /*numDataLanes*/
        {1, 2, 3, 4},                   /*dataLanesMap*/
        {0, 0, 0, 0},                   /*dataLanesPolarity*/
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS, /*csi_laneBandSpeed*/
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,       /* fpdLink */
        0x00,                       /* senI2cAddr */
        0x00,                       /* serI2cAddr */
    },
#endif
    1,                                  /*numChan*/
    9702,                                /*dccId*/
};

static IssSensorFxns           raw_testpat_SensorFxns = {
    rawtestpat_Probe,
    rawtestpat_Config,
    rawtestpat_StreamOn,
    rawtestpat_StreamOff,
    rawtestpat_PowerOn,
    rawtestpat_PowerOff,
    rawtestpat_GetExpParams,
    rawtestpat_SetAeParams,
    rawtestpat_GetDccParams,
    rawtestpat_InitAewbConfig,
    rawtestpat_GetIspConfig,
    rawtestpat_ReadWriteReg,
    rawtestpat_GetExpPrgFxn,
    rawtestpat_deinit,
    NULL,
    NULL
};

static IssSensorIntfParams     raw_testpat_SensorIntfPrms = {
    0,             /*sensorBroadcast*/
    0,             /*enableFsin*/
    0,               /*numCamerasStreaming*/
};

IssSensorConfig     raw_testpat_SensorConfig = {
    ub9702DesCfg_RAWTESTPAT,     /*desCfgPreScript*/
    NULL,      /*serCfgPreScript*/
    NULL,      /*sensorCfgPreScript*/
    ub9702RAWTESTPATDesCSI2Enable,        /*desCfgPostScript*/
    NULL,                    /*serCfgPostScript*/
    NULL,                    /*sensorCfgPostScript*/
};

IssSensors_Handle raw_testpat_SensorHandle = {
    1,                                 /*isUsed*/
    &raw_testpat_CreatePrms,                /*CreatePrms*/
    &raw_testpat_SensorFxns,                /*SensorFxns*/
    &raw_testpat_SensorIntfPrms,            /*SensorIntfPrms*/
};

/*
 * \brief DCC Parameters of RAWTESTPAT
 */
IssCapture_CmplxIoLaneCfg           raw_testpat_Csi2CmplxIoLaneCfg;

int32_t IssSensor_rawtestpat_Init(void)
{
    int32_t status;
    status = IssSensor_Register(&raw_testpat_SensorHandle);
    if(0 != status)
    {
        appLogPrintf("IssSensor_rawtestpat_Init failed \n");
    }

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t rawtestpat_Probe(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    /*
        Probe is used only for auto-detecting cameras connected to an FPD Link port.
        UB97x test pattern is not a camera therefore it must not be auto-detected.
        To generate test pattern, users can manually select this source from the
        application menu.
    */

    return (status);
}

static int32_t rawtestpat_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    (void)pSensorHdl;
    int32_t status = 0;
    I2cParams *deserCfg = NULL;
    uint8_t  ub97x_I2cAddr;
    int8_t ub97xInstanceId = getDeserializerInstIdFromChId(chId);

#if !defined(MCU_PLUS_SDK)
    uint8_t  ub97xI2CInstanceId;
    uint8_t  domain;
#endif

    if(ub97xInstanceId < 0)
    {
        appLogPrintf("Invalid ub97xInstanceId \n");
        status = 0xFF;
    }
    else
    {
        if(chId < 4U)
        {
            #if defined (MCU_PLUS_SDK)
            Board_fpdUb960GetI2CAddr(&ub97x_I2cAddr, BOARD_FPD_9702_CSI2_DES_HUB1);
            #else
            Board_fpdUb9702GetI2CAddr(BOARD_FPD_9702_CSI2_DES_HUB1,
                                  &domain,
                                  &ub97xI2CInstanceId,
                                  &ub97x_I2cAddr);
            #endif
            status = 0;
        }
        else
        {
            appLogPrintf("Invalid channel ID %d \n", chId);
            status = -1;
        }

        if(0 == status)
        {
            if(sensor_features_requested != (sensor_features_requested & (uint32_t)ISS_SENSOR_RAWTESTPAT_FEATURES))
            {
                appLogPrintf("rawtestpat_Config : Error. feature set 0x%x is not supported \n", sensor_features_requested);
                status = -1;
            }
            else
            {
                deserCfg = raw_testpat_SensorConfig.desCfgPreScript;

                if(NULL != deserCfg)
                {
                    status = IssSensor_cfgDesScript(deserCfg, ub97xInstanceId);
                }
            }
        }
    }

#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf(" rawtestpat_Config : status = 0x%x \n", status);
#endif
    return (status);
}

static int32_t rawtestpat_StreamOn(uint32_t chId, void *pSensorHdl)
{
    (void)pSensorHdl;
    int32_t status = 0;
    int8_t ub97xInstanceId = getDeserializerInstIdFromChId(chId);

    if(ub97xInstanceId < 0)
    {
        appLogPrintf("Invalid ub97xInstanceId \n");
        status = 0xFF;
    }
    else
    {
        {
            int32_t temp_status = IssSensor_cfgDesScript(ub9702RAWTESTPATDesCSI2Enable, ub97xInstanceId);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }
    return (status);
}

static int32_t rawtestpat_StreamOff(uint32_t chId, void *pSensorHdl)
{
    (void)pSensorHdl;
    int32_t status = 0;
    int8_t ub97xInstanceId = getDeserializerInstIdFromChId(chId);

    if(ub97xInstanceId < 0)
    {
        appLogPrintf("Invalid ub97xInstanceId \n");
        status = 0xFF;
    }
    else
    {
        {
            int32_t temp_status = IssSensor_cfgDesScript(ub9702RAWTESTPATDesCSI2Disable, ub97xInstanceId);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }
    return (status);
}

static int32_t rawtestpat_PowerOn(uint32_t chId, void *pSensorHdl)
{
    (void)pSensorHdl;
    int32_t status;
    uint8_t  ub97x_I2cAddr;
    uint8_t  ub97x_devId_regAddr = 0x0;
    uint8_t  ub97x_devId_regVal = 0xAB;

#if !defined(MCU_PLUS_SDK)
    uint8_t  domain;
    uint8_t  ub97x_I2cInstId;
#endif

    I2C_Handle sensorI2cHandle = NULL;
    static uint8_t sensorI2cByteOrder = BOARD_I2C_REG_ADDR_MSB_FIRST;

    getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
    if(NULL == sensorI2cHandle)
    {
        appLogPrintf("rawtestpat_PowerOn : Sensor I2C Handle is NULL \n");
        status = -1;
    }
    else
    {
        /*
            Driver implementation for Fusion2 board with dual UB97x
            Cameras 0-3 are mapped to UB97_0
            Cameras 4-7 are mapped to UB97_1
            Other boards or deserializers may need a change
        */

        if(chId < 4U)
        {
            #if defined (MCU_PLUS_SDK)
            Board_fpdUb960GetI2CAddr(&ub97x_I2cAddr, BOARD_FPD_9702_CSI2_DES_HUB1);
            #else
            Board_fpdUb9702GetI2CAddr(BOARD_FPD_9702_CSI2_DES_HUB1,
                                  &domain,
                                  &ub97x_I2cInstId,
                                  &ub97x_I2cAddr);
            #endif
            status = 0;
        }
        else
        {
            appLogPrintf("rawtestpat_PowerOn : Invalid channel ID %d \n", chId);
            ub97x_I2cAddr = 0xFF;
            status = -1;
        }

        if(0 == status)
        {
            status = Board_i2c8BitRegRd(sensorI2cHandle, ub97x_I2cAddr, ub97x_devId_regAddr, &ub97x_devId_regVal, 1U, SENSOR_I2C_TIMEOUT);
            if(0 != status)
            {
                appLogPrintf("rawtestpat_PowerOn : Error reading from register 0x%x on slave device 0x%x \n", ub97x_devId_regAddr, ub97x_I2cAddr);
            }
            else
            {
                appLogPrintf("rawtestpat_PowerOn : Read 0x%x from register 0x%x on slave device 0x%x \n", ub97x_devId_regVal, ub97x_devId_regAddr, ub97x_I2cAddr);
            }
        }
    }

    return status;
}

static int32_t rawtestpat_PowerOff(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status;
    status = powerOffDeserializers();
    if(status != 0)
    {
        appLogPrintf("rawtestpat_PowerOn : powerOffDeserializers returned 0x%x \n", status);
    }
    return status;
}

static int32_t rawtestpat_SetAeParams(void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    (void)pSensorHdl;
    (void)chId;
    (void)pExpPrms;
    int32_t status = 0;
    return (status);
}

static int32_t rawtestpat_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    (void)chId;
    (void)pSensorHdl;
    (void)pDccPrms;
    int32_t status = 0;
    return (status);
}

static int32_t rawtestpat_GetExpParams(uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;

    pExpPrms->expRatio = 256;

    return (status);
}

static void rawtestpat_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void rawtestpat_GetIspConfig (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void rawtestpat_deinit (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static int32_t rawtestpat_ReadWriteReg (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg)
{
    (void)chId;
    (void)pSensorHdl;

    /* return no error status */
    int32_t status = 0;


    if (0u == readWriteFlag)
    {
        /* read - always return 0 as read value */
        pReg->nRegValue = 0;
    }

    return (status);
}

static int32_t rawtestpat_ReadReg(uint8_t     i2cInstId,
                            uint8_t         i2cAddr,
                            uint16_t        regAddr,
                            uint16_t         *regVal,
                            uint32_t        numRegs)
{
    (void)i2cInstId;
    (void)i2cAddr;
    (void)regAddr;
    (void)regVal;
    (void)numRegs;
    int32_t  status = 0;
    return (status);
}

static int32_t rawtestpat_WriteReg(uint8_t    i2cInstId,
                             uint8_t      i2cAddr,
                             uint16_t     regAddr,
                             uint16_t     regVal,
                             uint32_t     numRegs)
{
    (void)i2cInstId;
    (void)i2cAddr;
    (void)regAddr;
    (void)regVal;
    (void)numRegs;
    int32_t  status = 0;
    return (status);
}

static int32_t rawtestpat_GetExpPrgFxn(uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t  status = -1;
    uint8_t count = 0;
    /*Max 40ms Exposure time since sensor is configured at 25 fps*/
    p_ae_dynPrms->targetBrightnessRange.min = 30;
    p_ae_dynPrms->targetBrightnessRange.max = 45;
    p_ae_dynPrms->targetBrightness = 35;
    p_ae_dynPrms->threshold = 1;
    p_ae_dynPrms->exposureTimeStepSize = 1;
    p_ae_dynPrms->enableBlc = 0;

    p_ae_dynPrms->exposureTimeRange[count].min = 100;
    p_ae_dynPrms->exposureTimeRange[count].max = 40000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 1024;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->exposureTimeRange[count].min = 40000;
    p_ae_dynPrms->exposureTimeRange[count].max = 40000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 8192;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}

