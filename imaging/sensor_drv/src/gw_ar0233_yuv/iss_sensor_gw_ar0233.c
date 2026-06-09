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
#include "iss_sensor_gw_ar0233.h"
#include "ub9xx_gw_ar0233_serdes_config.h"

static IssSensor_CreateParams  gw_ar0233CreatePrms = {
    GW_AR0233_UYVY,     /*sensor name*/
    0x6,                             /*i2cInstId*/
    {
        0, 0, 0, 0, 0, 0, 0, 0
        #if defined(SOC_J784S4) || defined(SOC_J742S2)
        , 0, 0, 0, 0
        #endif
    },/*i2cAddrSensor*/
    {
        0, 0, 0, 0, 0, 0, 0, 0
        #if defined(SOC_J784S4) || defined(SOC_J742S2)
        , 0, 0, 0, 0
        #endif
    },/*i2cAddrSer*/
    /*IssSensor_Info*/
    {
        {
            GW_AR0233_OUT_WIDTH,               /*width*/
            GW_AR0233_OUT_HEIGHT,              /*height*/
            1,                              /*num_exposures*/
            (vx_bool)vx_false_e,                     /*line_interleaved*/
            {
                {(uint32_t)VX_DF_IMAGE_UYVY, 7},    /*dataFormat and MSB [0]*/
                {(uint32_t)VX_DF_IMAGE_UYVY, 7},    /*dataFormat and MSB [0]*/
                {(uint32_t)VX_DF_IMAGE_UYVY, 7},    /*dataFormat and MSB [0]*/
            },
            0,                              /*meta_height_before*/
            0,                              /*meta_height_after*/
        },
        (uint32_t)ISS_SENSOR_GW_AR0233_FEATURES,     /*features*/
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_NONE,  /*aewbMode*/
        60,                             /*fps*/
        4,                              /*numDataLanes*/
        {1, 2, 3, 4},                   /*dataLanesMap*/
        {0, 0, 0, 0},                   /*dataLanesPolarity*/
        CSIRX_LANE_BAND_SPEED_720_TO_800_MBPS, /*csi_laneBandSpeed*/
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,           /* fpdLink */
        0x00,                           /* senI2cAddr */
        0x30,                           /* serI2cAddr */
    },
#endif
    1,                                  /*numChan*/
    5233,                                /*dccId*/
};

static IssSensorFxns           gw_ar0233SensorFxns = {
    gw_ar0233_Probe,
    gw_ar0233_Config,
    gw_ar0233_StreamOn,
    gw_ar0233_StreamOff,
    gw_ar0233_PowerOn,
    gw_ar0233_PowerOff,
    NULL,
    NULL,
    gw_ar0233_GetDccParams,
    gw_ar0233_InitAewbConfig,
    NULL,
    NULL,
    NULL,
    gw_ar0233_deinit,
    NULL,
    NULL
};

static IssSensorIntfParams     gw_ar0233SensorIntfPrms = {
    0,             /*sensorBroadcast*/
    0,             /*enableFsin*/
    0,			   /*numCamerasStreaming*/
};

IssSensorConfig     gw_ar0233SensorRegConfig = {
    NULL,     /*desCfgPreScript*/
    ub9xxSerCfg_GW_AR0233,      /*serCfgPreScript*/
    NULL,      /*sensorCfgPreScript*/
    ub9xxGW_AR0233DesCSI2Enable,        /*desCfgPostScript*/
    NULL,                    /*serCfgPostScript*/
    NULL,                    /*sensorCfgPostScript*/
};


IssSensors_Handle gw_ar0233SensorHandle = {
    1,                                 /*isUsed*/
    &gw_ar0233CreatePrms,                /*CreatePrms*/
    &gw_ar0233SensorFxns,                /*SensorFxns*/
    &gw_ar0233SensorIntfPrms,            /*SensorIntfPrms*/
};

/*
 * \brief DCC Parameters of gw_ar0233
 */
IssCapture_CmplxIoLaneCfg           gw_ar0233Csi2CmplxIoLaneCfg;

int32_t IssSensor_gw_ar0233_Init(void)
{
    int32_t status;
    status = IssSensor_Register(&gw_ar0233SensorHandle);
    if(0 != status)
    {
        appLogPrintf("IssSensor_gw_ar0233_Init failed \n");
    }

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t gw_ar0233_Probe(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
	/*
		Probe is used only for detecting cameras connected to an FPD Link port.
		There is no reliable way of detecting this camera.
		Always return -1 to indicate autodetect failed.
		If needed, user can manually select this sensor from application menu.
	*/
    int32_t status = -1;
    return (status);
}

static int32_t gw_ar0233_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    (void)sensor_features_requested;
    int32_t status = 0;
    I2cParams *deserCfg = NULL;
    I2cParams *serCfg = NULL;
    int8_t ub9xxInstanceId = getDeserializerInstIdFromChId(chId);
    uint32_t i2cInstId;
    /* MISRA.CAST.VOID_PTR_TO_OBJ_PTR.2012 - must be waived */
    IssSensors_Handle * pSenHandle = (IssSensors_Handle*)pSensorHdl;
    IssSensor_CreateParams * pCreatePrms;

    pCreatePrms = pSenHandle->createPrms;

    deserCfg = gw_ar0233SensorRegConfig.desCfgPreScript;
    serCfg = gw_ar0233SensorRegConfig.serCfgPreScript;
    i2cInstId = pCreatePrms->i2cInstId;

    if(NULL != deserCfg)
    {
        status = IssSensor_cfgDesScript(deserCfg, ub9xxInstanceId);
    }

    if(0 == status)
    {
        if(NULL != serCfg)
        {
            status = ub953_cfgScript((uint8_t)i2cInstId, pCreatePrms->i2cAddrSer[chId], serCfg);
        }
    }

    return (status);
}

static int32_t gw_ar0233_StreamOn(uint32_t chId, void *pSensorHdl)
{
    (void)pSensorHdl;
    int32_t status = 0;
    int8_t ub9xxInstanceId = getDeserializerInstIdFromChId(chId);

    if(ub9xxInstanceId < 0)
    {
        appLogPrintf("Invalid ub9xxInstanceId \n");
        status = 0xFF;
    }
    else
    {
        /*Start Streaming*/
        {
            int32_t temp_status = IssSensor_cfgDesScript(ub9xxGW_AR0233DesCSI2Enable, ub9xxInstanceId);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }
    return (status);
}

static int32_t gw_ar0233_StreamOff(uint32_t chId, void *pSensorHdl)
{
    (void)pSensorHdl;
    int32_t status = 0;
    int8_t ub9xxInstanceId = getDeserializerInstIdFromChId(chId);

    if(ub9xxInstanceId < 0)
    {
        appLogPrintf("Invalid ub9xxInstanceId \n");
        status = 0xFF;
    }
    else
    {
        /*Stop Streaming*/
        {
            int32_t temp_status = IssSensor_cfgDesScript(ub9xxGW_AR0233DesCSI2Disable, ub9xxInstanceId);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }
    return (status);
}

static int32_t gw_ar0233_PowerOn(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    return status;
}

static int32_t gw_ar0233_PowerOff(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    return (status);
}

static int32_t gw_ar0233_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    (void)chId;
    (void)pSensorHdl;
    (void)pDccPrms;
    int32_t status = 0;
    return (status);
}

static void gw_ar0233_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}

static void gw_ar0233_deinit (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}


