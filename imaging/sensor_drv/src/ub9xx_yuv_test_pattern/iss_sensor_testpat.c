#include "iss_sensor_testpat.h"
#include "ub9xx_testpat_serdes_config.h"

static IssSensor_CreateParams  testpatCreatePrms = {
    UB96X_TESTPATTERN_UYVY,     /*sensor name*/
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
            TESTPAT_OUT_WIDTH,               /*width*/
            TESTPAT_OUT_HEIGHT,              /*height*/
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
        (uint32_t)ISS_SENSOR_TESTPAT_FEATURES,     /*features*/
        (uint32_t)ALGORITHMS_ISS_AEWB_MODE_NONE,  /*aewbMode*/
        30,                             /*fps*/
        4,                              /*numDataLanes*/
        {1, 2, 3, 4},                   /*dataLanesMap*/
        {0, 0, 0, 0},                   /*dataLanesPolarity*/
        CSIRX_LANE_BAND_SPEED_1350_TO_1500_MBPS, /*csi_laneBandSpeed*/
    },
#if defined(B7_IMPLEMENTATION)
    {   /* moduleInfo */
        IMAGE_SERDES_FPD_LINK_3,           /* fpdLink */
        0x00,                           /* senI2cAddr */
        0x00,                           /* serI2cAddr */
    },
#endif
    1,                                  /*numChan*/
    960,                                /*dccId*/
};

static IssSensorFxns           testpatSensorFxns = {
    testpat_Probe,
    testpat_Config,
    testpat_StreamOn,
    testpat_StreamOff,
    testpat_PowerOn,
    testpat_PowerOff,
    NULL,
    NULL,
    testpat_GetDccParams,
    testpat_InitAewbConfig,
    NULL,
    NULL,
    NULL,
    testpat_deinit,
    NULL,
    NULL
};

static IssSensorIntfParams     testpatSensorIntfPrms = {
    0,             /*sensorBroadcast*/
    0,             /*enableFsin*/
    0,               /*numCamerasStreaming*/
};

IssSensorConfig     testpatSensorRegConfig = {
    ub9xxDesCfg_testpat,     /*desCfgPreScript*/
    NULL,      /*serCfgPreScript*/
    NULL,      /*sensorCfgPreScript*/
    ub9xxtestpatDesCSI2Enable,        /*desCfgPostScript*/
    NULL,                    /*serCfgPostScript*/
    NULL,                    /*sensorCfgPostScript*/
};


IssSensors_Handle testpatSensorHandle = {
    1,                                 /*isUsed*/
    &testpatCreatePrms,                /*CreatePrms*/
    &testpatSensorFxns,                /*SensorFxns*/
    &testpatSensorIntfPrms,            /*SensorIntfPrms*/
};

/*
 * \brief DCC Parameters of testpat
 */
IssCapture_CmplxIoLaneCfg           testpatCsi2CmplxIoLaneCfg;

int32_t IssSensor_testpat_Init(void)
{
    int32_t status;
    status = IssSensor_Register(&testpatSensorHandle);
    if(0 != status)
    {
        appLogPrintf("IssSensor_testpat_Init failed \n");
    }

    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

static int32_t testpat_Probe(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    /*
        Probe is used only for detecting cameras connected to an FPD Link port.
        UB97x test pattern is not a camera therefore it must not be detected.
        For test purpose, users can manually select this source from application menu.
    */
    return (status);
}

static int32_t testpat_Config(uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested)
{
    (void)pSensorHdl;
    (void)sensor_features_requested;
    int32_t status = 0;
    I2cParams *deserCfg = NULL;
    int8_t ub9xxInstanceId = getDeserializerInstIdFromChId(chId);

    deserCfg = testpatSensorRegConfig.desCfgPreScript;

    if(NULL != deserCfg)
    {
        int32_t temp_status = IssSensor_cfgDesScript(deserCfg, ub9xxInstanceId);
        if(temp_status != 0) {
            status = temp_status;
        }
    }

    return (status);
}

static int32_t testpat_StreamOn(uint32_t chId, void *pSensorHdl)
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
            int32_t temp_status = IssSensor_cfgDesScript(ub9xxtestpatDesCSI2Enable, ub9xxInstanceId);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }
    return (status);
}

static int32_t testpat_StreamOff(uint32_t chId, void *pSensorHdl)
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
            int32_t temp_status = IssSensor_cfgDesScript(ub9xxtestpatDesCSI2Disable, ub9xxInstanceId);
            if(temp_status != 0) {
                status = temp_status;
            }
        }
    }
    return (status);
}

static int32_t testpat_PowerOn(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status = 0;
    return status;
}

static int32_t testpat_PowerOff(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    int32_t status;
    status = powerOffDeserializers();
    if(status != 0)
    {
        appLogPrintf("testpat_PowerOn : powerOffDeserializers returned 0x%x \n", status);
    }
    return status;
}

static int32_t testpat_GetDccParams(uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms)
{
    (void)chId;
    (void)pSensorHdl;
    (void)pDccPrms;
    int32_t status = 0;
    return (status);
}

static void testpat_InitAewbConfig(uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}


static void testpat_deinit (uint32_t chId, void *pSensorHdl)
{
    (void)chId;
    (void)pSensorHdl;
    return;
}


