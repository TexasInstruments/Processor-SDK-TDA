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
 * @file        iss_sensor_imx390_discovery_narrow.c
 * @brief       Driver implementation for narrow-type cameras
 * @details    ************************************************* 
 * @copyright   Copyright (C) 2024 Define Design Deploy Corp. All Rights Reserved.
 */

#include "iss_sensor_imx390_discovery.h"
#include "imx390_discovery_serdes_config.h"
#include "d3_eeprom.h"

static int32_t IMX390_Probe_Narrow(uint32_t chId, void *pSensorHdl);

static IssSensor_CreateParams  imx390DISCOVERYCreatePrms = {
    SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW,     /*sensor name*/
    0x6,                             /*i2cInstId*/
    I2C_ADDR_SENSOR,                /*i2cAddrSensor*/
    I2C_ADDR_SER,                   /*i2cAddrSer*/
    /*IssSensor_Info*/
    {
        {
            IMX390_OUT_WIDTH,               /*width*/
            IMX390_OUT_HEIGHT-IMX390_META_HEIGHT_AFTER,            /*height*/
            1,                              /*num_exposures*/
            (vx_bool)vx_false_e,                     /*line_interleaved*/
            {
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0]*/
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0]*/
                {(uint32_t)TIVX_RAW_IMAGE_16_BIT, 11},    /*dataFormat and MSB [0]*/
            },
            0,                              /*meta_height_before*/
            IMX390_META_HEIGHT_AFTER,      /*meta_height_after*/
        },
        (uint32_t)ISS_SENSOR_IMX390_DISCOVERY_FEATURES,     /*features*/
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_AEWB,  /*aewbMode*/
        30,                             /*fps*/
        4,                              /*numDataLanes*/
        {1, 2, 3, 4},                   /*dataLanesMap*/
        {0, 0, 0, 0},                   /*dataLanesPolarity*/
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS,                            /*CSI Clock*/
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,           /* fpdLink */
        IMX390_DISCOVERY_I2C_ADDR,      /* senI2cAddr */
        0x30,                           /* serI2cAddr */
    },
#endif
    8,                                  /*numChan*/
    390,                                /*dccId*/
};

static IssSensorFxns           im390SensorFxns = {
    IMX390_Probe_Narrow,
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

static IssSensorIntfParams     imx390SensorIntfPrms = {
    0,             /*sensorBroadcast*/
    0,             /*enableFsin*/
    0,             /*numCamerasStreaming*/
};


IssSensors_Handle imx390DISCOVERYNarrowSensorHandle = {
    1,                                  /*isUsed*/
    &imx390DISCOVERYCreatePrms,         /*CreatePrms*/
    &im390SensorFxns,                   /*SensorFxns*/
    &imx390SensorIntfPrms,              /*SensorIntfPrms*/
};
/*
 * \brief Init
 */
int32_t IssSensor_IMX390DISCOVERY_NARROW_Init(void)
{    
    int32_t status;
    
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("IssSensor_IMX390DISCOVERY_NARROW_Init \n");
#endif

    status = IssSensor_Register(&imx390DISCOVERYNarrowSensorHandle);    
    if(0 != status)
    {
        appLogPrintf("%s failed \n", __func__);
    }
    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t IMX390_Probe_Narrow(uint32_t chId, void *pSensorHdl)
{
    int32_t status = -1;
    uint32_t i2cInstId;
    uint8_t sensorI2cAddr;
#ifndef SKIP_EEPROM
    uint8_t eepromI2cAddr;
#endif
    uint16_t chipIdRegAddr = IMX390_CHIP_ID_REG_ADDR;
    uint8_t chipIdRegValueRead = 0xAB;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;
    uint8_t count = 0;
    uint8_t max_retries = 1;

    pCreatePrms = pSenHandle->createPrms;

    i2cInstId = pCreatePrms->i2cInstId;
    sensorI2cAddr = pCreatePrms->i2cAddrSensor[chId];
    
    status = IMX390_Disco_PowerOn(chId, pSensorHdl);
    if(status != 0)
    {
        appLogPrintf("%s:%d - IMX390_Disco_PowerOn returned status: %d\n", __func__, __LINE__, status);
    }

    //Camera Type
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("IMX390_Probe_Narrow-> getCameraTypeFromEEPROM \n");
#endif

    /*Read chip ID to detect if the sensor can be detected*/
    while( (chipIdRegValueRead != (uint16_t)IMX390_CHIP_ID_REG_VAL) && (count < max_retries))
    {
        status = IMX390_Disco_ReadReg((uint8_t)i2cInstId, sensorI2cAddr, chipIdRegAddr, &chipIdRegValueRead, 1U);
        if(status == 0 )
        {
            if(chipIdRegValueRead == (uint16_t)IMX390_CHIP_ID_REG_VAL)
            {
                status = 0;
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("IMX390_Probe_Narrow SUCCESS : Read expected value 0x%x at chip ID register 0x%x \n", IMX390_CHIP_ID_REG_VAL, chipIdRegAddr);
#endif
            }
            else
            {
                status = -1;
                appLogPrintf("IMX390_Probe_Narrow : 0x%x read at chip ID register 0x%x. Expected 0x%x \n", chipIdRegValueRead, chipIdRegAddr, IMX390_CHIP_ID_REG_VAL);
                appLogPrintf("IMX390_Probe_Narrow Failed.. Retrying \n");
                appLogWaitMsecs(100);
            }
        }
        else
        {
            appLogPrintf("IMX390 Probe : Failed to read CHIP_ID register 0x%x \n", chipIdRegAddr);
        }
        count++;
    }

    #ifndef SKIP_EEPROM
    // Steal PCAL I2C Alias and use it for eeprom instead - we no longer need the PCAL I2C address
    status = UB960_SetAlias(chId, 2, M24C64_MEM_I2C >> 1, eepromI2cAddr);
    camera_type_t camera = getCameraTypeFromEEPROM(sensorI2cHandle, eepromI2cAddr);
    if(camera == IMX390_NARROW) {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("Success: Camera is IMX390 Discovery Narrow FOV");
#endif
        status = 0;
    } else if(camera == IMX390_MID) {
        appLogPrintf("Failure: Camera is IMX390 Discovery Mid FOV");
        status = -1;
    } else if(camera == IMX390_WIDE) {
        appLogPrintf("Failure: Camera is IMX390 Discovery Wide FOV");
        status = -1;
    } else {
        appLogPrintf("Failure: Could not query camera EEPROM");
        status = -1;
    }
    #endif

    return (status);
}
