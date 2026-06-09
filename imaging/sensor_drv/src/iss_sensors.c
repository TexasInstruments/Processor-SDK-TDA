/*
 *
 * Copyright (c) 2020-2024 Texas Instruments Incorporated
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

#ifdef QNX
#include <errno.h>
#include <hw/i2c.h>
#include <fcntl.h>
#endif

#include <iss_sensors.h>
#include <iss_sensor_if.h>
#include <iss_sensor_priv.h>
#include <app_remote_service.h>
#include <app_ipc.h>
#include "TI/tivx_mutex.h"

#if defined (MCU_PLUS_SDK)
#include <board/utils/board_utils.h>
#if (defined(SOC_AM62A) && defined(adas))
#include <board/ioexp/ioexp_tca6424.h>
#endif
#endif

#if defined (SOC_J721E)
#include <ti/board/src/j721e_evm/include/board_cfg.h>
#include <ti/board/src/j721e_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_cfg.h>
#include <ti/board/src/j721s2_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_cfg.h>
#include <ti/board/src/j784s4_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J742S2)
#include <ti/board/src/j742s2_evm/include/board_cfg.h>
#include <ti/board/src/j742s2_evm/include/board_i2c_io_exp.h>
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
#include <ti/board/src/devices/fpd/ds90ub9702.h>
#endif

#if defined(SOC_AM62A)
#define IO_EXPANDER_ADDR 0x23
#endif

// Note: this is a hack for now
#if defined(SOC_AM62A)
#define CONFIG_I2C2 (0U)
#define CONFIG_I2C1 (1U)
#else
#define CONFIG_I2C2 (1U)
#endif

#if defined(BUILD_QNX)
/* Reset I2C1 hardware to ensure clean state */
/* They are called so MCU1_0 uses TCA6424 with CONFIG_I2C1*/
/* This is because for AM62A QNX HDMI is congigured using I2C1 which is handled by A53 during boot*/
/* Reset the hardware after HDMI initialisation*/
    #define AM62A_I2C1_BASE       0x20010000UL
    #define AM62A_I2C_CON_EN      (1U << 15)  /* I2C Enable bit */
    #define AM62A_I2C_SYSS_RDONE  (1U << 0)   /* Reset Done bit */
    #define AM62A_I2C_INT_ALL     0x7FFF      /* All interrupt bits */
#endif

/*******************************************************************************
 *  Globals
 *******************************************************************************
 */

/* Array to store sensor handles registered to the framework. Populated during
*  runtime by sensor drivers calling IssSensor_Register. */
IssSensors_Handle *gIssSensorTable[ISS_SENSORS_MAX_SENSOR_HANDLES];

/* Fusion2 board detect flag */
vx_bool gFusion2Det = (vx_bool)vx_false_e;

/* Used to call functions depending on deserializer hubs */
uint8_t gDeserializerHubs = 0;

static I2C_Handle gISS_Sensor_I2cHandle = NULL;
static uint8_t gISS_Sensor_ByteOrder = BOARD_I2C_REG_ADDR_MSB_FIRST;
static uint8_t num_sensors_open = 0U;
static IssSensors_Handle *g_pSenHndl[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
static uint8_t g_detectedSensors[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
I2cParams *desCfgCommon_ptr;

typedef struct
{
    uint8_t nPort;
    uint8_t nVC;
} ub960MapParams;


/*******************************************************************************
 *  Local Functions Declarations
 *******************************************************************************
 */
static void IssSensor_getDeserializerI2cAddr(uint8_t desHubInst,
    uint8_t *i2cBus, uint8_t *i2cAddr);
static int32_t checkForHandle(const IssSensors_Handle* handle);
static int32_t setup_io_expander(void);
static int32_t setupI2CInst(uint8_t i2cInst);
static int32_t Serialzier_WriteReg(uint8_t slaveI2cAddr,
    uint16_t regAddr, uint8_t regVal);
static int32_t Deserializer_ReadReg(uint8_t slaveI2cAddr,
    uint16_t regAddr, uint8_t *regVal);
static int32_t Deserializer_WriteReg(uint8_t   slaveI2cAddr,
    uint16_t  regAddr, uint8_t   regVal);
static int32_t IssSensor_detect_serializer(int8_t desHubInstId, uint16_t *cameras_detected);
static int32_t IssSensor_detect_sensor(uint8_t chId);
static int32_t IssSensor_DeserializerInit(void);
static int32_t powerOnDeserializers(void);

#if defined(B7_IMPLEMENTATION)
static int32_t Serialzier_ReadReg(uint8_t slaveI2cAddr,
    uint16_t regAddr, uint8_t *regVal);
static int32_t IssSensor_DeserializerInit_B7(void);
static int32_t IssSensor_cfgDesScriptCustom(uint8_t  slaveAddr, I2cParams *script, uint8_t  bitOperation);
#endif

/*******************************************************************************
 *  Function Definition
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Powers off deserailzer hubs on Fusion1/2 boards
 *
 * \return  0 if success
 * \return -1 otherwise
 *
 *******************************************************************************
*/
int32_t powerOffDeserializers(void)
{
    int32_t status = -1;
    uint32_t tca6408SlaveAddr = TCA6408_I2C_ADDR_1;
    uint8_t regAddr = TCA6408_CONFIG_REG;
    uint8_t regVal = 0xFF;
    static uint8_t fusionBrdInitDone = 0;

    if(fusionBrdInitDone == 0U)
    {
        status = Board_i2c8BitRegWr(gISS_Sensor_I2cHandle, tca6408SlaveAddr, regAddr, &regVal, 1U, SENSOR_I2C_TIMEOUT);
        if(0 == status)
        {
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("Write 0x%.2x to TCA6408(0x%.2x) register 0x%.2x \n", regVal,
                     tca6408SlaveAddr, regAddr);
#endif
        }
        else
        {
            appLogPrintf(
            "[powerOffDeserializers]: Board_i2c8BitRegWr returned %d while "
            "writing 0x%.2x to TCA6408(0x%.2x) register 0x%.2x \n",
            status, regVal, tca6408SlaveAddr, regAddr);
        }

        appLogWaitMsecs(200);

        fusionBrdInitDone = 0U;
    }
    return status;
}

void getIssSensorI2cInfo(uint8_t *byteOrder, I2C_Handle *i2cHndl)
{
    *byteOrder = gISS_Sensor_ByteOrder;
    *i2cHndl = gISS_Sensor_I2cHandle;
}

int32_t IssSensor_Init(void)
{
    int32_t status = -1;
    uint32_t cnt;

    for(cnt = 0; cnt < ISS_SENSORS_MAX_SUPPORTED_SENSOR; cnt++)
    {
        g_pSenHndl[cnt] = NULL;
        g_detectedSensors[cnt] = 0xFF;
    }

    status = appRemoteServiceRegister(
        IMAGE_SENSOR_REMOTE_SERVICE_NAME,
        ImageSensor_RemoteServiceHandler
    );

    if(status != 0)
    {
        appLogPrintf(" REMOTE_SERVICE_SENSOR: ERROR: Unable to register remote service sensor handler\n");
    }
    else
    {
        for (cnt = 0U; cnt < ISS_SENSORS_MAX_SENSOR_HANDLES; cnt ++)
        {
            gIssSensorTable[cnt] = NULL;
        }

        /*  Initialization function call of all supported sensors. Sensor
        initialization functions shall register the driver to the framework using
        IssSensor_Register(...) */
        status = IssSensor_IMX390_Init();
        if (status == 0) {
            status = IssSensor_OX03F10_Init();
        }
        if (status == 0) {
            status = IssSensor_OX05C1S_Init();
        }
        if (status == 0) {
            status = IssSensor_IMX390DISCOVERY_NARROW_Init();
        }
        if (status == 0) {
            status = IssSensor_IMX390DISCOVERY_MID_Init();
        }
        if (status == 0) {
            status = IssSensor_IMX390DISCOVERY_WIDE_Init();
        }
        if (status == 0) {
            status = IssSensor_IMX728_Init();
        }
        if (status == 0) {
            status = IssSensor_rawtestpat_Init();
        }
        if (status == 0) {
            status = IssSensor_testpat_Init();
        }
        if (status == 0) {
            status = IssSensor_gw_ar0233_Init();
        }
#if defined(SOC_AM62A)
        if (status == 0) {
            status = IssSensor_OV2312_Init();
        }
#else /* QNX: I2C reads need update for other sensors */
        if (status == 0) {
            status = IssSensor_AR0233_Init();
        }
        if (status == 0) {
            status = IssSensor_AR0820_Init();
        }
#endif
    }
    return status;
}

int32_t IssSensor_Register(IssSensors_Handle *pSensorHandle)
{
    int32_t status = 0;
    uint32_t cnt = 0U;

    if (NULL == pSensorHandle)
    {
        status = -1;
    }
    else
    {
        /* Find a free entry in the sensor table */
        for (cnt = 0U; cnt < ISS_SENSORS_MAX_SENSOR_HANDLES; cnt++)
        {
            if (NULL == gIssSensorTable[cnt])
            {
                gIssSensorTable[cnt] = pSensorHandle;
                appLogPrintf("Found sensor %s at location %d \n", pSensorHandle->createPrms->name, cnt);
                break;
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (cnt == ISS_SENSORS_MAX_SENSOR_HANDLES)
        {
            appLogPrintf(" ISS_SENSOR: Could not register sensor \n");
            status = -1;
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}

IssSensors_Handle *IssSensor_getSensorHandleFromName(const char *name)
{
    uint32_t cnt;
    IssSensors_Handle *pSensorHandle = NULL;
    vx_bool shouldBreak = (vx_bool)vx_false_e;

    /* Check For Errors */
    if (NULL == name)
    {
        pSensorHandle = NULL;
    }
    else
    {
        for (cnt = 0U; cnt < ISS_SENSORS_MAX_SENSOR_HANDLES; cnt++)
        {
            pSensorHandle = gIssSensorTable[cnt];
            if(NULL == pSensorHandle)
            {
                appLogPrintf("pSensorHandle is NULL \n");
                pSensorHandle = NULL;
                shouldBreak = (vx_bool)vx_true_e;
            }
            else if(NULL == pSensorHandle->createPrms)
            {
                appLogPrintf("createPrms is NULL \n");
                pSensorHandle = NULL;
                shouldBreak = (vx_bool)vx_true_e;
            }
            else
            {
                if (0 == strncmp(pSensorHandle->createPrms->name, name, ISS_SENSORS_MAX_NAME))
                {
                    shouldBreak = (vx_bool)vx_true_e;
                }
            }

            if((vx_bool)vx_true_e == shouldBreak)
            {
                break;
            }
        }
    }

    return pSensorHandle;
}

int32_t IssSensor_GetSensorInfo(const char *name, IssSensor_CreateParams *sensor_prms)
{
    int32_t status = 0;
    IssSensors_Handle *pSensorHandle = NULL;

    pSensorHandle = IssSensor_getSensorHandleFromName(name);
    if(pSensorHandle == NULL)
    {
        appLogPrintf(
            "[IssSensor_GetSensorInfo] Error: "
            "IssSensor_getSensorHandleFromName returned NULL\n");
        status = -1;
    }
    else
    {
        (void)memcpy(sensor_prms, pSensorHandle->createPrms,
                       sizeof(IssSensor_CreateParams));
    }

    return status;
}

int32_t IssSensor_Delete(IssSensors_Handle* handle)
{
    int32_t status = -1;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (0 == status)
    {
        (void)memset(handle->createPrms, 0, sizeof(IssSensor_CreateParams));
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

int32_t IssSensor_Config(IssSensors_Handle* handle, uint32_t chId, uint32_t feat)
{
    int32_t status = -1;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    if (0 == status)
    {
        if((chId < handle->createPrms->num_channels) && (chId < ISS_SENSORS_MAX_CHANNEL))
        {
            if (NULL != handle->sensorFxns->config)
            {
                status = handle->sensorFxns->config(chId, (void*)handle, feat);
            }
        }
        else
        {
            appLogPrintf("IssSensor_Config Error : Incorrect channel ID %d \n", chId);
            status = -1;
        }
    }

    return (status);
}

int32_t IssSensor_PowerOn(IssSensors_Handle* handle, uint32_t chMask)
{
    int32_t status = -1;

    if(num_sensors_open < ISS_SENSORS_MAX_CHANNEL)
    {
        num_sensors_open = num_sensors_open + 1U;
    }
    else
    {
        num_sensors_open = ISS_SENSORS_MAX_CHANNEL;
    }

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    if (0 == status)
    {
        uint32_t chId = 0;
        uint32_t chMaskLocal = chMask;
        handle->sensorIntfPrms->numCamerasStreaming = 0;
        while ((chMaskLocal > 0U) && (chId < ISS_SENSORS_MAX_CHANNEL))
        {
            if((chMaskLocal & 0x1U) != 0U)
            {
                if (NULL != handle->sensorFxns->powerOn)
                {
                    status = handle->sensorFxns->powerOn(chId, (void*)handle);
                }
            }
            chMaskLocal = chMaskLocal >> 1;
            chId++;
        }
    }

    return (status);

}

int32_t IssSensor_PowerOff(IssSensors_Handle* handle, uint32_t chMask)
{
    int32_t status;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    if (status != 0)
    {
        status = status;
    }
    else
    {
        uint32_t chId = 0;
        uint32_t chIter = chMask;

        while ((chIter > 0U) && (chId < ISS_SENSORS_MAX_SUPPORTED_SENSOR))
        {
            if ((chIter & 0x1U) != 0U)
            {
                if (handle->sensorFxns->powerOff != NULL)
                {
                    status = handle->sensorFxns->powerOff(chId, (void*)handle);
                }
                if(num_sensors_open > 0U)
                {
                    num_sensors_open = num_sensors_open - 1U;
                }
                else
                {
                    num_sensors_open = 0U;
                }
            }
            chIter = chIter >> 1;
            chId++;
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (num_sensors_open == 0U)
        /* LDRA_JUSTIFY_END */
        {
            #if !defined (MCU_PLUS_SDK)
            I2C_close(gISS_Sensor_I2cHandle);
            #endif
            gISS_Sensor_I2cHandle = NULL;
        }
    }

    return (status);
}

int32_t IssSensor_Start(IssSensors_Handle* handle, uint32_t chId)
{
    int32_t status = -1;
    uint8_t numCamerasEnabled = 1U;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    if (0 == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if(chId < ISS_SENSORS_MAX_CHANNEL)
        /* LDRA_JUSTIFY_END */
        {
            if(NULL != handle->sensorFxns->streamOn)
            {
                status = handle->sensorFxns->streamOn(chId, (void*)handle);
            }
        }

        if(status == 0)
        {
            uint16_t tmpSum = (uint16_t)handle->sensorIntfPrms->numCamerasStreaming + (uint16_t)numCamerasEnabled;
            handle->sensorIntfPrms->numCamerasStreaming = (uint8_t)tmpSum;
        }
    }

    return (status);
}

int32_t IssSensor_Stop(IssSensors_Handle* handle, uint32_t chId)
{
    int32_t status = -1;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        if(chId < ISS_SENSORS_MAX_CHANNEL)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != handle->sensorFxns->streamOff)
            /* LDRA_JUSTIFY_END */
            {
                status = handle->sensorFxns->streamOff(chId, (void*)handle);
            }

            uint8_t result = 0;
            if(handle->sensorIntfPrms->numCamerasStreaming > 0U)
            {
                result = handle->sensorIntfPrms->numCamerasStreaming - 1U;
            }
            else
            {
                result = 0U;
            }

            handle->sensorIntfPrms->numCamerasStreaming = result;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            appLogPrintf("IssSensor_Stop Error: Incorrect channel ID %d \n", chId);
            status = -1;
        }
        /* LDRA_JUSTIFY_END */
    }

    g_pSenHndl[chId] = NULL;

    return (status);
}

int32_t IssSensor_GetAeParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = 0;
 
    /* Check if the handle is valid or not */
    status = checkForHandle(handle);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status != 0)
    {
        status = status;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */
    if (handle == NULL)
    {
        appLogPrintf("IssSensor_SetAeParams Error: sensor handle is NULL for channel ID %d \n", chId);
            status = -1;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            if (handle->sensorFxns->getAeParams != NULL) {
                status = handle->sensorFxns->getAeParams(chId, handle, pExpPrms);
            }
        }
    }
 
    return (status);
}

int32_t IssSensor_SetAeParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_ExposureParams *pExpPrms)
{
    int32_t status = -1;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    if (0 == status)
    {
        if(handle != NULL)
        {
            if(NULL != handle->sensorFxns->setAeParams)
            {
                status = handle->sensorFxns->setAeParams(handle, chId, pExpPrms);
            }
            else
            {
                appLogPrintf("IssSensor_SetAeParams Error : Sensor setAeParams callback is NULL \n");
                (void)memset(pExpPrms, 0x0, sizeof(IssSensor_ExposureParams));
            }
        }
        else
        {
            appLogPrintf("IssSensor_SetAeParams Error: sensor handle is NULL for channel ID %d \n", chId);
            status = -1;
        }
    }

    return (status);
}

int32_t IssSensor_GetAwbParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms)
{
    int32_t status = 0;
 
    /* Check if the handle is valid or not */
    status = checkForHandle(handle);
    if (status != 0)
    {
        status = status;
    }
    else
    {
        if (handle == NULL)
        {
            appLogPrintf("IssSensor_SetAwbParams Error : sensor handle is NULL for channel ID %d\n", chId);
            status = -1;
        }
        else
        {
            if (handle->sensorFxns->getAwbParams != NULL)
            {
                status = handle->sensorFxns->getAwbParams(chId, handle, pWbPrms);
            }
        }
    }
 
    return (status);
}

int32_t IssSensor_SetAwbParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms)
{
    int32_t status = -1;

    /* Check if the handle is valid or not */
    status = checkForHandle(handle);

    if (0 == status)
    {
        //if(chId < handle->createPrms->num_channels)
        if(handle != NULL)
        {
            if(NULL != handle->sensorFxns->setAwbParams)
            {
                status = handle->sensorFxns->setAwbParams(handle, chId, pWbPrms);
            }
            else
            {
                appLogPrintf("IssSensor_SetAwbParams Error : Sensor setAwbParams callback is NULL \n");
                (void)memset(pWbPrms, 0x0, sizeof(IssSensor_WhiteBalanceParams));
            }
        }
        else
        {
            appLogPrintf("IssSensor_SetAwbParams Error : sensor handle is NULL for  channel ID %d \n", chId);
            status = -1;
        }
    }

    return (status);
}

int32_t IssSensor_Control(const IssSensors_Handle* handle, uint32_t cmd, uint8_t *cmdArgs, const void* cmdRetArgs)
{
    (void)handle;
    (void)cmd;
    (void)cmdRetArgs;
    /* TODO: Remove dependency to vx_status */
    int32_t status = (int32_t)VX_FAILURE;
    uint32_t chId = 0xFF;
    IMAGE_SENSOR_CTRLCMD ctrlCmd;
    uint8_t *cmd_ptr = cmdArgs;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    cmp_ptr cannot be NULL as it is pre-validated by ImageSensor_RemoteServiceHandler()
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL == cmd_ptr)
    {
        appLogPrintf("Error : cmd_ptr is NULL \n");
        status = (int32_t)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
    (void)memcpy((void*)&chId, (const void*)&cmd_ptr[ISS_SENSORS_MAX_NAME], sizeof(uint32_t));

    (void)memcpy((void*)&ctrlCmd, (const void*)&cmd_ptr[ISS_SENSORS_MAX_NAME + sizeof(uint32_t)], sizeof(IMAGE_SENSOR_CTRLCMD));

    cmd_ptr = &cmd_ptr[ISS_SENSORS_MAX_NAME + sizeof(uint32_t) + sizeof(IMAGE_SENSOR_CTRLCMD)];

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
    <justification end> */
    switch(ctrlCmd)
    {
    /* LDRA_JUSTIFY_END */
        case IMAGE_SENSOR_CTRLCMD_GETEXPPRG:
            {
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                if(NULL != pSenHndl)
                {
                    IssAeDynamicParams tmpAeParms;

                    (void)memcpy((void *)&tmpAeParms, (const void *)cmd_ptr, sizeof(IssAeDynamicParams));
                    status = pSenHndl->sensorFxns->getExpPrg(chId, (void*)pSenHndl, &tmpAeParms);
                    (void)memcpy((void *)cmd_ptr, (const void *)&tmpAeParms, sizeof(IssAeDynamicParams));
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    appLogPrintf("Error : sensor handle is NULL for channel %d \n", chId);
                    status = (int32_t)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
            }
            break;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        case IMAGE_SENSOR_CTRLCMD_INIT_AEWB_CFG:
            {
                IssSensors_Handle *pSenHndl = g_pSenHndl[chId];
                if (pSenHndl == NULL)
                {
                    appLogPrintf("Error : sensor handle is NULL for channel %d.\n",chId);
                    status = (int32_t)VX_FAILURE;
                    break;
                }

                if (pSenHndl->sensorFxns->initAewbConfig != NULL)
                {
                    pSenHndl->sensorFxns->initAewbConfig(chId, (void*)pSenHndl);
                    status = (int32_t)VX_SUCCESS;
                }
            }
            break;
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        case IMAGE_SENSOR_CTRLCMD_GETWBCFG:
            {
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                if(NULL != pSenHndl)
                {
                    if(NULL != pSenHndl->sensorFxns)
                    {
                        if(NULL != pSenHndl->sensorFxns->getWbCfg)
                        {
                            IssAwbDynamicParams tmpAwbParms;

                            (void)memcpy((void *)&tmpAwbParms, (const void *)cmd_ptr, sizeof(IssAwbDynamicParams));
                            status = pSenHndl->sensorFxns->getWbCfg(chId, (void*)pSenHndl, &tmpAwbParms);
                            (void)memcpy((void *)cmd_ptr, (const void *)&tmpAwbParms, sizeof(IssAwbDynamicParams));
                        }else
                        {
                            /*Sensor driver does not support WB update API*/
                            (void)memset(cmd_ptr, 0, sizeof(IssAwbDynamicParams));
                        }
                    }
                }
                else
                {
                    (void)memset(cmd_ptr, -1, sizeof(IssAwbDynamicParams));
                    status = (int32_t)VX_FAILURE;
                }
            }
            break;
        /* LDRA_JUSTIFY_END */
        case IMAGE_SENSOR_CTRLCMD_SETEXPGAIN:
            {
                IssSensor_ExposureParams aePrms;
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL != pSenHndl)
                /* LDRA_JUSTIFY_END */
                {
                    (void)memcpy((void*)&aePrms, (const void*)cmd_ptr, sizeof(IssSensor_ExposureParams));
                    status = IssSensor_SetAeParams(pSenHndl, chId, &aePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    appLogPrintf("Error : sensor handle is NULL for channel %d \n", chId);
                    status = (int32_t)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_GETEXPGAIN:
            {
                IssSensor_ExposureParams aePrms;
                (void)memset(&aePrms, 0, sizeof(IssSensor_ExposureParams));

                IssSensors_Handle *pSenHndl = g_pSenHndl[chId];
                if (pSenHndl != NULL)
                {
                    status = IssSensor_GetAeParams(pSenHndl, chId, &aePrms);
                    (void)memcpy((void*)cmd_ptr, (const void*)&aePrms, sizeof(IssSensor_ExposureParams));
                }
                else
                {
                    appLogPrintf("Error : sensor handle is NULL for channel %d \n", chId);
                    status = (int32_t)VX_FAILURE;
                }
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_SETWBGAIN:
            {
                IssSensor_WhiteBalanceParams awbPrms;
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                if(NULL != pSenHndl)
                {
                    (void)memcpy((void*)&awbPrms, (const void*)cmd_ptr, sizeof(IssSensor_WhiteBalanceParams));
                    status = IssSensor_SetAwbParams(pSenHndl, chId, &awbPrms);
                }
                else
                {
                    appLogPrintf("Error : sensor handle is NULL for channel %d \n", chId);
                    status = (int32_t)VX_FAILURE;
                }
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_GETWBGAIN:
            {
                IssSensor_WhiteBalanceParams awbPrms;
                (void)memset(&awbPrms, 0, sizeof(IssSensor_WhiteBalanceParams));

                IssSensors_Handle *pSenHndl = g_pSenHndl[chId];
                if (pSenHndl != NULL)
                {
                    status = IssSensor_GetAwbParams(pSenHndl, chId, &awbPrms);
                    (void)memcpy((void*)cmd_ptr, (const void*)&awbPrms, sizeof(IssSensor_WhiteBalanceParams));
                }
                else
                {
                    appLogPrintf("Error : sensor handle is NULL for channel %d \n", chId);
                    status = (int32_t)VX_FAILURE;
                }
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_DEBUG:
            /*Reserved for future use*/
            status = 0;
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG \n");
#endif
            {
                /* MISRA.CAST.OBJ_PTR_TO_OBJ_PTR.2012 - must be waived */
                uint32_t * ptr32 = (uint32_t * )cmd_ptr;
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                uint32_t rw_flag = *ptr32++; /*ReadWrite Flag = 1 for Write*/
                uint32_t devType = *ptr32++; /*Device Type*/
                uint32_t regAddr32 = *ptr32++; /*Register Address*/
                uint32_t regVal32 = *ptr32++; /*Register Value*/
                uint32_t slaveI2cAddr = 0x0;
                uint8_t regAddr = 0xBC;
                uint8_t regVal = 0xDE;

                if(0U == devType)
                {
                    uint8_t  ch_num;
                    uint8_t ub960InstanceId = (uint8_t)chId;
                    uint8_t  i2cSlaveAddr8;
                    IssSensor_getDeserializerI2cAddr(ub960InstanceId, &ch_num, &i2cSlaveAddr8);
                    slaveI2cAddr = i2cSlaveAddr8;
                }
                else if(1U == devType)
                {
                    slaveI2cAddr = pSenHndl->createPrms->i2cAddrSer[chId];
                }
                else if(2U == devType)
                {
                    slaveI2cAddr = pSenHndl->createPrms->i2cAddrSensor[chId];
                }
                else if (255U == devType)
                {
                    slaveI2cAddr = *ptr32;
                }
                else
                {
                    appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG Error : Unsupported devType %d \n", devType);
                    status = -1;
                }

                if((status == 0) && (2U == devType))
                {
                    I2cParams sensorI2cParams;
                    sensorI2cParams.nDelay = 1U;
                    sensorI2cParams.nRegAddr = (uint16_t)(regAddr32);
                    sensorI2cParams.nRegValue = (uint16_t)(regVal32);
                    status = pSenHndl->sensorFxns->readWriteReg(chId, pSenHndl, rw_flag, &sensorI2cParams);
                    if(0 != status)
                    {
                        appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG : Error reading from register 0x%.2x on the image sensor %s \n", regAddr, pSenHndl->createPrms->name);
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start> 
                    Rationale: The test framework and test apps cannot reach this portion.
                    The test framework does not support the configuration required to trigger this scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to read the register values and continue execution as expected.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        ptr32 = (uint32_t * )cmdArgs;
                        *ptr32 = (uint32_t)sensorI2cParams.nRegValue;
                    }
                    /* LDRA_JUSTIFY_END */
                }
                else if(status == 0)
                {
                    if(0U == rw_flag)
                    {
                        regAddr = (uint8_t)(regAddr32);
                        status = Board_i2c8BitRegRd(gISS_Sensor_I2cHandle, slaveI2cAddr, regAddr, &regVal, 1U, SENSOR_I2C_TIMEOUT);
                        if(0 != status)
                        {
                            appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG : Error reading from register 0x%.2x on slave device 0x%.2x \n", regAddr, slaveI2cAddr);
                        }
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG : Read 0x%.2x from register 0x%.2x on slave device 0x%.2x \n", regVal, regAddr, slaveI2cAddr);
#endif
                    }
                    else if (1U == rw_flag)
                    {
                        regAddr = (uint8_t)(regAddr32);
                        regVal =  (uint8_t)(regVal32);
                        status = Board_i2c8BitRegWr(gISS_Sensor_I2cHandle, slaveI2cAddr, regAddr, &regVal, 1U, SENSOR_I2C_TIMEOUT);
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start> 
                        Rationale: The test framework and test apps cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(0 != status)
                        /* LDRA_JUSTIFY_END */
                        {
                            appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG : Error writing 0x%.2x to register 0x%.2x on slave device 0x%.2x \n", regVal, regAddr, slaveI2cAddr);
                        }
                    }
                    else
                    {
                        status = -1;
                        appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG Error : Invalid rwflag = %d \n", rw_flag);
                    }

                    if(0 == status)
                    {
                        ptr32 = (uint32_t * )cmdArgs;
                        *ptr32 = (uint32_t)regVal;
                    }
                }
                else
                {
                    appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG Error : invalid command params\n");
                }
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_READ_SENSOR_REG:
            {
                /* MISRA.CAST.OBJ_PTR_TO_OBJ_PTR.2012 - must be waived */
                uint32_t * ptr32 = (uint32_t * )cmd_ptr;
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                I2cParams reg_read;
                reg_read.nDelay = 0;
                reg_read.nRegAddr = (uint16_t)(*ptr32);
                ptr32++;
                reg_read.nRegValue = 0xFF;
                status = pSenHndl->sensorFxns->readWriteReg(chId, pSenHndl, 0, &reg_read);
                *ptr32 = reg_read.nRegValue;
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_WRITE_SENSOR_REG:
            {
                /* MISRA.CAST.OBJ_PTR_TO_OBJ_PTR.2012 - must be waived */
                uint32_t * ptr32 = (uint32_t * )cmd_ptr;
                IssSensors_Handle * pSenHndl = g_pSenHndl[chId];
                I2cParams reg_write;
                reg_write.nDelay = 0;
                reg_write.nRegAddr = (uint16_t)(*ptr32);
                ptr32++;
                reg_write.nRegValue = (uint16_t)(*ptr32);
                status = pSenHndl->sensorFxns->readWriteReg(chId, pSenHndl, 1, &reg_write);
                *ptr32 = reg_write.nRegValue;
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_READ_SENSOR_REG_IMMEDIATE_I2C:
            {
                /* MISRA.CAST.OBJ_PTR_TO_OBJ_PTR.2012 - must be waived */
                uint32_t *ptr32       = (uint32_t *)cmd_ptr;
                uint32_t i2cSlaveAddr = *ptr32++;
                uint16_t regAddr      = (uint16_t)(*ptr32++);
                uint8_t  regVal       = 0;
 
                I2C_Handle     sensorI2cHandle    = NULL;
                static uint8_t sensorI2cByteOrder = 255U;
                getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
                if (sensorI2cHandle == NULL)
                {
                    appLogPrintf("Sensor I2C Handle is NULL \n");
                    break;
                }
                status = Board_i2c16BitRegRd(sensorI2cHandle, i2cSlaveAddr, regAddr, &regVal, 1U, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
                if (status != 0)
                {
                    appLogPrintf("Error : I2C Timeout while reading from register 0x%x \n", regAddr);
                }
                *ptr32 = regVal;
            }
            break;
        case IMAGE_SENSOR_CTRLCMD_WRITE_SENSOR_REG_IMMEDIATE_I2C:
            {
                /* MISRA.CAST.OBJ_PTR_TO_OBJ_PTR.2012 - must be waived */
                uint32_t *ptr32       = (uint32_t *)cmd_ptr;
                uint32_t i2cSlaveAddr = *ptr32++;
                uint16_t regAddr      = (uint16_t)*ptr32++;
                uint8_t  regVal       = (uint8_t)*ptr32;
 
                I2C_Handle     sensorI2cHandle    = NULL;
                static uint8_t sensorI2cByteOrder = 255U;
                getIssSensorI2cInfo(&sensorI2cByteOrder, &sensorI2cHandle);
                if (sensorI2cHandle == NULL)
                {
                    appLogPrintf("Sensor I2C Handle is NULL \n");
                    break;
                }
                status = Board_i2c16BitRegWr(sensorI2cHandle, i2cSlaveAddr, regAddr, &regVal, 1U, sensorI2cByteOrder, SENSOR_I2C_TIMEOUT);
                if (status != 0)
                {
                    appLogPrintf("Error : I2C Timeout while writing 0x%x to register 0x%x \n", regVal, regAddr);
                }
            }
            break;
        default:
            status = -1;
            appLogPrintf("IMAGE_SENSOR_CTRLCMD_DEBUG Error : Unknown control command %d \n", ctrlCmd);
            break;
    }
    }

    return (status);
}

int32_t IssSensor_cfgDesScript(const I2cParams *script, int8_t desHubInstId)
{
    uint16_t regAddr;
    uint8_t  regValue;
    uint16_t delayMilliSec;
    uint32_t cnt;
    int32_t status = 0;
    uint8_t  desI2cBusNum = 0;
    uint8_t  desI2cAddr;

    /*Assumption for Fusion1 board - maximum two UB960s*/
    if((desHubInstId < (int8_t)BOARD_CSI_INST_0) || ((int8_t)gDeserializerHubs < desHubInstId))
    {
        appLogPrintf("Error : Invalid desHubInstId %d \n", desHubInstId);
        status = -1;
    }
    else
    {
        IssSensor_getDeserializerI2cAddr((uint8_t)desHubInstId, &desI2cBusNum,
            &desI2cAddr);

        if(NULL != script)
        {
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("Deserializer config start \n");
#endif
            cnt = 0;
            regAddr  = script[cnt].nRegAddr;
            regValue = (uint8_t)script[cnt].nRegValue;
            delayMilliSec = (uint16_t)script[cnt].nDelay;

            while(regAddr != 0xFFFFU)
            {
                status = Deserializer_WriteReg(desI2cAddr, regAddr, regValue);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0 != status)
                {
                    appLogPrintf(" Deserializer Error: Reg Write Failed for regAddr 0x%.2x, cnt = %d\n", regAddr, cnt);
                    break;
                }
                /* LDRA_JUSTIFY_END */

                if(delayMilliSec > 0U)
                {
                    appLogWaitMsecs(delayMilliSec);
                }

                cnt++;
                regAddr  = script[cnt].nRegAddr;
                regValue = (uint8_t)script[cnt].nRegValue;
                delayMilliSec = (uint16_t)script[cnt].nDelay;
            }
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("End of deserializer config \n");
#endif
        }
    }
    return (status);
}

int32_t ub953_cfgScript(uint8_t  i2cInstId, uint8_t  i2cAddr, const I2cParams *script)
{
    (void)i2cInstId;
    int32_t status = 0;

    uint16_t regAddr;
    uint8_t  regValue;
    uint16_t delayMilliSec;
    uint32_t cnt;

    if(NULL != script)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("ub953 config start : i2cAddr = 0x%.2x \n", i2cAddr);
#endif
        cnt = 0;
        regAddr  = script[cnt].nRegAddr;
        regValue = (uint8_t)script[cnt].nRegValue;
        delayMilliSec = (uint16_t)script[cnt].nDelay;

        while(regAddr != 0xFFFFU)
        {
            status = Serialzier_WriteReg(i2cAddr, regAddr, regValue);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != status)
            {
                appLogPrintf(" Serializer Error: Reg Write Failed for regAddr 0x%.2x\n", regAddr);
                break;
            }
            /* LDRA_JUSTIFY_END */

            if(delayMilliSec > 0U)
            {
                appLogWaitMsecs(delayMilliSec);
            }

            cnt++;
            regAddr  = script[cnt].nRegAddr;
            regValue = (uint8_t)script[cnt].nRegValue;
            delayMilliSec = (uint16_t)script[cnt].nDelay;
        }
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("End of serialzier config \n");
#endif
    }

    return (status);
}

#if defined(B7_IMPLEMENTATION)
/* To do masked write, set delay to 0x00FFFFFF where the 2 MSB is the delay, the value is the masking value
 bitOperation: 0 = OR; 1 = AND
*/
static int32_t IssSensor_cfgDesScriptCustom(uint8_t  desI2cAddr, I2cParams *script, uint8_t  bitOperation)
{
    int32_t status = 0;

    uint16_t regAddr;
    uint8_t regVal;
    uint32_t delayMilliSec;
    uint32_t cnt;

    if(NULL != script)
    {
        cnt = 0;
        regAddr  = script[cnt].nRegAddr;
        regVal = script[cnt].nRegValue;
        delayMilliSec = script[cnt].nDelay;

        while(regAddr != 0xFFFF)
        {
            if((delayMilliSec & 0x00FFFFFF) == 0x00FFFFFF)
            {
                /* masked write */
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("%d , masking with 0x%02x, writing back with 0x%02x\n", cnt+1, script[cnt].nRegValue, regVal);
#endif

                regVal = 0x00;
                status = Deserializer_ReadReg(desI2cAddr, regAddr, &regVal);
                if (status == 0) {
                    if(bitOperation == 0)
                    {
                        regVal = regVal | script[cnt].nRegValue;
                    }
                    else if(bitOperation == 1)
                    {
                        regVal = regVal & script[cnt].nRegValue;
                    }
                    else
                    {
                        appLogPrintf("IssSensor_cfgDesScriptCustom: Invalid bit operation: %d. Defaulting to OR \n", bitOperation);
                        bitOperation = 0;
                    }
                    status = Deserializer_WriteReg(desI2cAddr, regAddr, regVal);
                }

                delayMilliSec = delayMilliSec >> 24;
            }
            else
            {
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("%d Writing 0x%02x to 0x%02x\n", cnt+1, regVal, regAddr);
#endif
                Deserializer_WriteReg(desI2cAddr, regAddr, regVal);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if(delayMilliSec > 0)
            /* LDRA_JUSTIFY_END */
            {
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("\tdelaying for 0x%02x\n", delayMilliSec);
#endif
                appLogWaitMsecs(delayMilliSec);
            }

            cnt++;
            regAddr  = script[cnt].nRegAddr;
            regVal = script[cnt].nRegValue;
            delayMilliSec = script[cnt].nDelay;
        }
    }
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf(" End of deserializer config \n");
#endif
    return (status);
}
#endif

int32_t enableUB960Broadcast(int8_t desHubInstId)
{
    int32_t status = 0;
    I2cParams enableUB960BroadcastScript[] =
    {
        {0x4C, 0x0F, 0x10},
        {0xFFFF,0x00, 0x00}
    };

    status = IssSensor_cfgDesScript(enableUB960BroadcastScript, desHubInstId);

    return status;
}

int32_t disableUB960Broadcast(int8_t desHubInstId)
{
    int32_t status = 0;
    I2cParams disableUB960BroadcastScript[] =
    {
        {0x4C, DES_PORT_SEL_0, 0x10},
        {0x5C, (SER_0_I2C_ALIAS<< 1U), 0x10},
        {0x66, (SENSOR_0_I2C_ALIAS << 1U), 0x10},

        {0x4C, DES_PORT_SEL_1, 0x10},
        {0x5C, (SER_1_I2C_ALIAS<< 1U), 0x10},
        {0x66, (SENSOR_1_I2C_ALIAS << 1U), 0x10},

        {0x4C, DES_PORT_SEL_2, 0x10},
        {0x5C, (SER_2_I2C_ALIAS<< 1U), 0x10},
        {0x66, (SENSOR_2_I2C_ALIAS << 1U), 0x10},

        {0x4C, DES_PORT_SEL_3, 0x10},
        {0x5C, (SER_3_I2C_ALIAS<< 1U), 0x10},
        {0x66, (SENSOR_3_I2C_ALIAS << 1U), 0x10},

        {0xFFFF,0x00, 0x00}
    };

    if(0 == desHubInstId)
    {
        disableUB960BroadcastScript[1].nRegValue = (SER_0_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[2].nRegValue = (SENSOR_0_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[4].nRegValue = (SER_1_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[5].nRegValue = (SENSOR_1_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[7].nRegValue = (SER_2_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[8].nRegValue = (SENSOR_2_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[10].nRegValue = (SER_3_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[11].nRegValue = (SENSOR_3_I2C_ALIAS<<1U);
    } else if(1 == desHubInstId)
    {
        disableUB960BroadcastScript[1].nRegValue = (SER_4_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[2].nRegValue = (SENSOR_4_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[4].nRegValue = (SER_5_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[5].nRegValue = (SENSOR_5_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[7].nRegValue = (SER_6_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[8].nRegValue = (SENSOR_6_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[10].nRegValue = (SER_7_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[11].nRegValue = (SENSOR_7_I2C_ALIAS<<1U);
    } 
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
    <justification end> */
    else if(2 == desHubInstId)
    /* LDRA_JUSTIFY_END */
    {
        disableUB960BroadcastScript[1].nRegValue = (SER_8_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[2].nRegValue = (SENSOR_8_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[4].nRegValue = (SER_9_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[5].nRegValue = (SENSOR_9_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[7].nRegValue = (SER_10_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[8].nRegValue = (SENSOR_10_I2C_ALIAS<<1U);

        disableUB960BroadcastScript[10].nRegValue = (SER_11_I2C_ALIAS<<1U);
        disableUB960BroadcastScript[11].nRegValue = (SENSOR_11_I2C_ALIAS<<1U);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        appLogPrintf("enableUB960Broadcast: Error: Invalid instance ID \n");
        status = -1;
    }
    /* LDRA_JUSTIFY_END */

    if(status == 0)
    {
        status = IssSensor_cfgDesScript(disableUB960BroadcastScript, desHubInstId);
    }

    return status;
}

int8_t getDeserializerInstIdFromChId(uint32_t chId)
{
    int8_t desHubInstId = -1;
    /* Assumptions :
    *  max 2 UB960  instances (Fusion1 Board)
    *  max 3 UB9702 instances (Fusion2 Board)
    *  up to 4 cameras per instance of UB960/UB9702
    */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this scenario.
    Effect on this unit: If the control reaches here, our code base is expected to continue execution as expected.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(chId < 4U)
    /* LDRA_JUSTIFY_END */
    {
        desHubInstId = (int8_t)BOARD_CSI_INST_0;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    else if(chId < 8U)
    {
        desHubInstId = (int8_t)BOARD_CSI_INST_1;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this scenario.
    Effect on this unit: If the control reaches here, our code base is expected to continue execution as expected.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if((gFusion2Det == (vx_bool)vx_true_e) && (chId < 12U))
    /* LDRA_JUSTIFY_END */
    {
        desHubInstId = (int8_t)BOARD_CSI_INST_2;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        appLogPrintf("Error : Invalid chId 0x%.2x \n", chId);
        desHubInstId = -1;
    }
    /* LDRA_JUSTIFY_END */

    return desHubInstId;
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
int32_t enableUB960Streaming(uint32_t chId)
{
    int32_t status = -1;
    int8_t desHubInstId = getDeserializerInstIdFromChId(chId);

    I2cParams ub960DesCSI2Enable[5U] = {
        /* CSI-TX Port Config */
        {0x32, 0x03, 0x20}, /* Read CSI-TX Port 0, Write CSI-TX Ports 0 and 1 */
        {0x20, 0xF0, 0x10}, /* Disable FPD-Link RX forwarding */
        {0x33, 0x03, 0x20}, /* 4 CSI-2 Lanes; Continous clock mode, CSI-2 output enabled */
        {0x20, 0x00, 0x10}, /* Enable FPD-Link RX forwarding (all ports) to CSI-TX port 0 */

        {0xFFFF,0x00, 0x00} /* End script */
    };

    status = IssSensor_cfgDesScript(ub960DesCSI2Enable, desHubInstId);

    return status;
}
/* LDRA_JUSTIFY_END */

int32_t disableUB960Streaming(uint32_t chId)
{
    int32_t status = -1;
    int8_t desHubInstId = getDeserializerInstIdFromChId(chId);

    I2cParams ub960DesCSI2Disable[3u] = {
        {0x20, 0xF0, 0x10}, /* Disable FPD-Link RX forwarding */
        {0x33, 0x02, 0x10}, /* Disable CSI-TX output */
        {0xFFFF, 0x00, 0x0} /* End of script */
    };

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(desHubInstId < 0)
    {
        appLogPrintf("Error : Invalid desHubInstId \n");
    }
    else
    /* LDRA_JUSTIFY_END */
    {
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
    <justification end> */
    switch(chId)
        {
    /* LDRA_JUSTIFY_END */
        case 0:
        case 4:
        case 8:
            ub960DesCSI2Disable[0].nRegValue = DES_PORT_SEL_0;
            break;
        case 1:
        case 5:
        case 9:
            ub960DesCSI2Disable[0].nRegValue = DES_PORT_SEL_1;
            break;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */
        case 2:
        case 6:
        /* LDRA_JUSTIFY_END */
        case 10:
            ub960DesCSI2Disable[0].nRegValue = DES_PORT_SEL_2;
            break;
        case 3:
        case 7:
        case 11:
            ub960DesCSI2Disable[0].nRegValue = DES_PORT_SEL_3;
            break;
        default:
            appLogPrintf("Error : disableUB960Streaming: Invalid channel ID 0x%.2x \n", chId);
            status = -1;
            break;
        }
        status = IssSensor_cfgDesScript(ub960DesCSI2Disable, desHubInstId);
    }

    return status;
}

// Selects a specific port on the UB960.
// This function is unaware of broadcast mode, check for broadcast mode
// before calling, otherwise broadcast mode will be clobbered.
// Returns -1 on errors, 0 on success.
int32_t UB960_SelectPort(uint32_t chId)
{
    int32_t status = -1;
    uint8_t desHubInstId = (uint8_t)getDeserializerInstIdFromChId(chId);
    uint8_t desI2cAddr;
    uint8_t desI2cBusNum = 0;
    uint8_t portSelReg = 0x4C;
    uint8_t chIdMod = (uint8_t)(chId % 4U);
    uint8_t portSelVal = (uint8_t)((uint8_t)(1U << chIdMod) | (uint8_t)((uint8_t)chIdMod << 4U)); // Select the proper port.

    IssSensor_getDeserializerI2cAddr(desHubInstId, &desI2cBusNum,
        &desI2cAddr);

    if(desI2cAddr != 0U) 
    {
        status = 0;
    }
    if(status == 0) 
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    {
        status = Deserializer_WriteReg(desI2cAddr, portSelReg, portSelVal); 
    }
    /* LDRA_JUSTIFY_END */
    return status;
}

int32_t UB960_SetSerAlias(uint32_t chId, uint8_t ser_alias_i2c_addr_7bit)
{
    int32_t status = -1;
    int8_t desHubInstId = getDeserializerInstIdFromChId(chId);
    uint8_t desI2cAddr;
    uint8_t  desI2cBusNum = 0;
    uint8_t portSelReg = 0x4C;
    uint8_t portSelVal;

    I2cParams ub960setSerAlias[2u] = {
        {0x5C, 0x0, 0x1},
        {0xFFFF, 0x0, 0x0}
    };

    IssSensor_getDeserializerI2cAddr((uint8_t)desHubInstId, &desI2cBusNum,
        &desI2cAddr);

    status = Deserializer_ReadReg(desI2cAddr, portSelReg, &portSelVal);
    if(status != 0)
    {
        appLogPrintf("%s:%d - Deserializer_ReadReg returned status: %d\n", __func__, __LINE__, status);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
    <justification end> */
    if(0x0FU != portSelVal)
    /* LDRA_JUSTIFY_END */
    {
        status = UB960_SelectPort(chId);
    }

    if(status == 0) {
        ub960setSerAlias[0].nRegValue = (uint16_t)((uint16_t)ser_alias_i2c_addr_7bit << 1);
        status = IssSensor_cfgDesScript(ub960setSerAlias, desHubInstId);
    }

    return status;
}

int32_t UB960_SetAlias(uint32_t chId, uint8_t slot, uint8_t phy_i2c_addr_7bit, uint8_t alias_i2c_addr_7bit)
{
    int32_t status = -1;
    int8_t desHubInstId = getDeserializerInstIdFromChId(chId);
    uint8_t desI2cAddr;
    uint8_t  desI2cBusNum = 0;
    uint8_t portSelReg = 0x4C;
    uint8_t portSelVal;

    uint8_t alias_register_pairs[8][2] = {
        {0x5D, 0x65}, 
        {0x5E, 0x66},
        {0x5F, 0x67},
        {0x60, 0x68},
        {0x61, 0x69},
        {0x62, 0x70},
        {0x63, 0x71},
        {0x64, 0x72}
    };

    I2cParams ub960setAlias[3u] = {
        {0x0, 0x0, 0x1},
        {0x0, 0x0, 0x1},
        {0xFFFF, 0x0, 0x0}
    };

    ub960setAlias[0].nRegAddr = alias_register_pairs[slot][0];
    ub960setAlias[1].nRegAddr = alias_register_pairs[slot][1]; // Select which alias we are setting.

    appLogPrintf("Configuring Alias on UB960 CHID: %i, aliasing 0x%.2x to 0x%.2x\n", chId, phy_i2c_addr_7bit, alias_i2c_addr_7bit);

    IssSensor_getDeserializerI2cAddr((uint8_t)desHubInstId, &desI2cBusNum,
        &desI2cAddr);

    status = Deserializer_ReadReg(desI2cAddr, portSelReg, &portSelVal);
    if(status != 0)
    {
        appLogPrintf("%s:%d - Deserializer_ReadReg returned status: %d\n", __func__, __LINE__, status);
    }

    if(0x0FU != portSelVal)
    {
        status = UB960_SelectPort(chId);
    }

    if(status == 0) {
        ub960setAlias[0].nRegValue = (uint16_t)((uint16_t)phy_i2c_addr_7bit << 1U);
        ub960setAlias[1].nRegValue = (uint16_t)((uint16_t)alias_i2c_addr_7bit << 1U);
        status = IssSensor_cfgDesScript(ub960setAlias, desHubInstId);
    }

    return status;
}

int32_t UB960_SetSensorAlias(uint32_t chId, uint8_t sensor_phy_i2c_addr_7bit, uint8_t ser_alias_i2c_addr_7bit)
{
    int32_t status = -1;
    int8_t desHubInstId = getDeserializerInstIdFromChId(chId);
    uint8_t pageSelReg = 0x4C;
    uint8_t pageSelVal = 0xFF;
    uint8_t desI2cAddr;
    uint8_t  desI2cBusNum;
    uint8_t  sensor_alias[ISS_SENSORS_MAX_SUPPORTED_SENSOR] =
        {SENSOR_0_I2C_ALIAS, SENSOR_1_I2C_ALIAS, SENSOR_2_I2C_ALIAS, SENSOR_3_I2C_ALIAS,
        SENSOR_4_I2C_ALIAS, SENSOR_5_I2C_ALIAS, SENSOR_6_I2C_ALIAS, SENSOR_7_I2C_ALIAS,
        SENSOR_8_I2C_ALIAS, SENSOR_9_I2C_ALIAS, SENSOR_10_I2C_ALIAS, SENSOR_11_I2C_ALIAS};

    I2cParams ub960setSensorAlias[5u] = {
        {0x4C, 0x00, 0x1},
        {0x5E, 0x00, 0x1},
        {0x66, 0x00, 0x1},
        {0x5C, 0x00, 0x1},
        {0xFFFF, 0x00, 0x0} //End of script
    };

    if(desHubInstId < 0)
    {
        appLogPrintf("Error : Invalid desHubInstId %d\n", desHubInstId);
    }
    else
    {
        IssSensor_getDeserializerI2cAddr((uint8_t)desHubInstId, &desI2cBusNum,
            &desI2cAddr);

        status = Deserializer_ReadReg(desI2cAddr, pageSelReg, &pageSelVal);
        if(status != 0)
        {
            appLogPrintf("%s:%d - Deserializer_ReadReg returned status: %d\n", __func__, __LINE__, status);
        }

        if(0x0FU == pageSelVal)
        {
            /*Broadcast Enabled. No need to set page select register*/
            ub960setSensorAlias[0].nRegValue = 0x0F;
        }
        else
        {
            /*Broadcast not enabled. Must set page select as per channel ID*/
            switch(chId)
            {
                case 0:
                case 4:
                case 8:
                    ub960setSensorAlias[0].nRegValue = DES_PORT_SEL_0;
                    break;
                case 1:
                case 5:
                case 9:
                    ub960setSensorAlias[0].nRegValue = DES_PORT_SEL_1;
                    break;
                case 2:
                case 6:
                case 10:
                    ub960setSensorAlias[0].nRegValue = DES_PORT_SEL_2;
                    break;
                case 3:
                case 7:
                case 11:
                    ub960setSensorAlias[0].nRegValue = DES_PORT_SEL_3;
                    break;
                default:
                    appLogPrintf("Error : UB960_SetSensorAlias: Invalid channel ID 0x%.2x \n", chId);
                    status = -1;
                    break;
            }
        }

        ub960setSensorAlias[1].nRegValue = (uint16_t)((uint16_t)sensor_phy_i2c_addr_7bit << 1U);
        ub960setSensorAlias[2].nRegValue = (uint16_t)((uint16_t)sensor_alias[chId] << 1U);
        ub960setSensorAlias[3].nRegValue = (uint16_t)((uint16_t)ser_alias_i2c_addr_7bit << 1U);
        status = IssSensor_cfgDesScript(ub960setSensorAlias, desHubInstId);
    }

    return status;
}

void IssSensor_CloseHandle(void)
{
    #if !defined(MCU_PLUS_SDK)
    I2C_close(gISS_Sensor_I2cHandle);
    #endif

    gISS_Sensor_I2cHandle = NULL;
}

static int32_t IssSensor_registerHandle(IssSensors_Handle *handle, uint32_t chMask)
{
    int32_t status = -1;
    uint32_t chId = 0;
    uint32_t chMaskLocal = chMask;

    while ((chMaskLocal > 0U) && (chId < ISS_SENSORS_MAX_CHANNEL))
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */
        if((chMaskLocal & 0x1U) == 0x1U)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if(g_pSenHndl[chId] != NULL)
            {
                appLogPrintf("Warning : channel ID %d already has a registered sensor handle \n", chId);
            }
            /* LDRA_JUSTIFY_END */
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("Registered handle at g_pSenHndl[%d] for chId = %d \n", chId, chId);
#endif
            g_pSenHndl[chId] = handle;
            status = 0;
        }
        chMaskLocal = chMaskLocal >> 1;
        chId++;
    }

    return status;
}

/* MISRA.PPARAM.NEEDS.CONST - must be waived */
int32_t ImageSensor_RemoteServiceHandler(char *service_name, uint32_t cmd,
    void *prm, uint32_t prm_size, uint32_t flags)
{
    (void)service_name;
    (void)flags;
    int32_t status = -1;
    uint16_t cmdMemoryNeeded = 0;
    uint8_t * cmd_param = (uint8_t * )prm;
    uint8_t chId = 0;
    uint32_t sensor_features_requested = 0;
    uint32_t channel_mask = 0;
    //uint32_t channel_mask_supported = 0;
    char * sensor_name = NULL;
    IssSensors_Handle * pSenHndl = NULL;
    IssSensor_CreateParams tmpSenParams;
    uint32_t bit_count = 0;
    uint32_t tmpchMask = 0;

    switch(cmd)
    {
        case (uint32_t)IM_SENSOR_CMD_ENUMERATE:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_ENUMERATE \n");
#endif
            cmdMemoryNeeded = ISS_SENSORS_MAX_NAME * ISS_SENSORS_MAX_SENSOR_HANDLES;
            if(prm_size < cmdMemoryNeeded)
            {
                appLogPrintf("Error : Insufficient prm size %d, need at least %d \n", prm_size, cmdMemoryNeeded);
                status = -1;
            }
            else
            {
#if !defined(B7_IMPLEMENTATION)
                status = IssSensor_DeserializerInit();
                if(status != 0)
                {
                    appLogPrintf("%s:%d - IssSensor_DeserializerInit returned status: %d\n", __func__, __LINE__, status);
                }
#endif
                {
                    uint8_t count;
                    for(count = 0; count < ISS_SENSORS_MAX_SENSOR_HANDLES; count++)
                    {
                        pSenHndl = gIssSensorTable[count];
                        if(NULL !=pSenHndl)
                        {
                            (void)memcpy((void*)&cmd_param[count*ISS_SENSORS_MAX_NAME], (const void*)pSenHndl->createPrms->name, ISS_SENSORS_MAX_NAME);
                        }
                    }
                }
                status = 0;
            }

            break;
        case (uint32_t)IM_SENSOR_CMD_QUERY:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_QUERY \n");
#endif
            sensor_name = (char*)(cmd_param);
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("Received Query for %s \n", sensor_name);
#endif

            status = IssSensor_GetSensorInfo(sensor_name, &tmpSenParams);
            (void)memcpy((void *)&cmd_param[ISS_SENSORS_MAX_NAME], (const void *)&tmpSenParams, sizeof(IssSensor_CreateParams));
            break;
        case (uint32_t)IM_SENSOR_CMD_PWRON:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_PWRON \n");
#endif
            sensor_name = (char*)(cmd_param);
            (void)memcpy((void*)&channel_mask, (const void*)&cmd_param[ISS_SENSORS_MAX_NAME], sizeof(uint32_t));
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("IM_SENSOR_CMD_PWRON : channel_mask = 0x%.2x \n", channel_mask);
#endif

            pSenHndl = IssSensor_getSensorHandleFromName(sensor_name);
            if(NULL == pSenHndl)
            {
                status = -1;
                appLogPrintf("ERROR : NULL handle returned for sensor %s \n", sensor_name);
            }
            else
            {
                status = IssSensor_registerHandle(pSenHndl, channel_mask);
                if(status == -1)
                {
                    appLogPrintf("ERROR : IssSensor_registerHandle returned -1");
                }
                else
                {
                    #if defined(B7_IMPLEMENTATION)
                    IssSensor_DeserializerInit();
                    #endif
                    status = IssSensor_PowerOn(pSenHndl, channel_mask);
                }
            }
            break;
        case (uint32_t)IM_SENSOR_CMD_CONFIG:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_CONFIG \n");
#endif
            sensor_name = (char*)(cmd_param);
            (void)memcpy((void*)&sensor_features_requested, (const void*)&cmd_param[ISS_SENSORS_MAX_NAME], sizeof(uint32_t));
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("Application requested features = 0x%.2x \n ", sensor_features_requested);
#endif
            (void)memcpy((void*)&channel_mask, (const void*)&cmd_param[ISS_SENSORS_MAX_NAME+sizeof(uint32_t)], sizeof(uint32_t));
            pSenHndl = IssSensor_getSensorHandleFromName(sensor_name);
            if(NULL == pSenHndl)
            {
                status = -1;
                appLogPrintf("ERROR : NULL handle returned for sensor %s \n", sensor_name);
            }
            else
            {
                status = 0;

                if(1U == pSenHndl->sensorIntfPrms->sensorBroadcast)
                {
                    /* enable broadcast mode only if more than one camera connected to des hub */
                    tmpchMask = (channel_mask & 0x00FU);
                    bit_count = (tmpchMask & 1U) + ((tmpchMask>>1) & 1U) + ((tmpchMask>>2) & 1U) + ((tmpchMask>>3) & 1U);
                    if(bit_count > 1U)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("Configuring all cameras on UB9xx_0 Des in broadcast mode \n");
#endif
                        if (status == 0) {
                            status = enableUB960Broadcast(0);
                        }
                        if (status == 0) {
                            status = IssSensor_Config(pSenHndl, 0, sensor_features_requested);
                        }
                        channel_mask &= 0xFF0U;
                    }

                    tmpchMask = ((channel_mask >> 4) & 0x00FU);
                    bit_count = (tmpchMask & 1U) + ((tmpchMask>>1) & 1U) + ((tmpchMask>>2) & 1U) + ((tmpchMask>>3) & 1U);
                    if(bit_count > 1U)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("Configuring all cameras on UB9xx_1 Des in broadcast mode \n");
#endif
                        if (status == 0) {
                            status = enableUB960Broadcast(1);
                        }
                        if (status == 0) {
                            status = IssSensor_Config(pSenHndl, 4, sensor_features_requested);
                        }
                        channel_mask &= 0xF0FU;
                    }

                    tmpchMask = ((channel_mask >> 8) & 0x00FU);
                    bit_count = (tmpchMask & 1U) + ((tmpchMask>>1) & 1U) + ((tmpchMask>>2) & 1U) + ((tmpchMask>>3) & 1U);
                    if(bit_count > 1U)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("Configuring all cameras on UB9xx_2 Des in broadcast mode \n");
#endif
                        if (status == 0) {
                            status = enableUB960Broadcast(2);
                        }
                        if (status == 0) {
                            status = IssSensor_Config(pSenHndl, 8, sensor_features_requested);
                        }
                        channel_mask &= 0x0FFU;
                    }
                }

#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("Disabling broadcast mode \n");
#endif
                for(uint8_t i = 0; i < gDeserializerHubs; i++)
                {
                    status = disableUB960Broadcast((int8_t)i);
                    if(status != 0)
                    {
                        appLogPrintf("%s:%d - disableUB960Broadcast returned status: %d\n", __func__, __LINE__, status);
                    }
                }

#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("IM_SENSOR_CMD_CONFIG channel_mask = %.2x\n", channel_mask);
#endif

                if(0U != channel_mask)
                {
                    chId = 0;
                    while( (channel_mask > 0U) && (chId < ISS_SENSORS_MAX_CHANNEL) )
                    {
                        if((channel_mask & 0x1U) == 0x1U)
                        {
#ifdef ENABLE_DEBUG_IMAGING
                            appLogPrintf("Configuring camera # %d \n", chId);
#endif
                            if (status == 0) {
                                status = IssSensor_Config(pSenHndl, chId, sensor_features_requested);
                            }
                        }
                        chId++;

                        channel_mask = channel_mask >> 1U;
                    }
                }
            }
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("IM_SENSOR_CMD_CONFIG returning status = %d\n", status);
#endif

            break;
        case (uint32_t)IM_SENSOR_CMD_STREAM_ON:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_STREAM_ON \n");
#endif
            sensor_name = (char*)(cmd_param);
            (void)memcpy((void*)&channel_mask, (const void*)&cmd_param[ISS_SENSORS_MAX_NAME], sizeof(uint32_t));
            pSenHndl = IssSensor_getSensorHandleFromName(sensor_name);
            if(NULL == pSenHndl)
            {
                status = -1;
                appLogPrintf("ERROR : NULL handle returned for sensor %s \n", sensor_name);
            }
            else
            {
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("IM_SENSOR_CMD_STREAM_ON:  channel_mask = 0x%.2x\n", channel_mask);
#endif
                status = 0;

                //channel_mask_supported = (1<<pSenHndl->createPrms->num_channels) - 1;
                //channel_mask &= channel_mask_supported;
                /* Broadcast stream on to sensor is unstable. Setting this to 2 so it never goes in, but leaving the code to be fixed */
                if(2U == pSenHndl->sensorIntfPrms->sensorBroadcast)
                {
                    if((channel_mask & 0x00FU) != 0U)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("Starting all cameras on UB9xx_0 Des in broadcast mode \n");
#endif
                        if (status == 0) {
                            status = enableUB960Broadcast(0);
                        }
                        if (status == 0) {
                            status = IssSensor_Start(pSenHndl, 0);
                        }
                        channel_mask &= 0xFF0U;
                    }

                    if((channel_mask & 0x0F0U) != 0U)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("Starting all cameras on UB9xx_1 Des in broadcast mode \n");
#endif
                        if (status == 0) {
                            status = enableUB960Broadcast(1);
                        }
                        if (status == 0) {
                            status = IssSensor_Start(pSenHndl, 4);
                        }
                        channel_mask &= 0xF0FU;
                    }

                    if((channel_mask & 0xF00U) != 0U)
                    {
#ifdef ENABLE_DEBUG_IMAGING
                        appLogPrintf("Starting all cameras on UB9xx_2 Des in broadcast mode \n");
#endif
                        if (status == 0) {
                            status = enableUB960Broadcast(2);
                        }
                        if (status == 0) {
                            status = IssSensor_Start(pSenHndl, 8);
                        }
                        channel_mask &= 0x0FFU;
                    }
                }

                /*Disable broadcast after enabling streaming
                so that every camera can have independent 2A control */
                for(uint8_t i = 0; i < gDeserializerHubs; i++)
                {
                    status = disableUB960Broadcast((int8_t)i);
                    if(status != 0)
                    {
                        appLogPrintf("%s:%d - disableUB960Broadcast returned status: %d\n", __func__, __LINE__, status);
                    }
                }

                chId = 0;

                while( (channel_mask > 0U) && (chId < ISS_SENSORS_MAX_CHANNEL) )
                {
                    if((channel_mask & 0x1U) != 0U)
                    {
                        if (status == 0) {
                            status = IssSensor_Start(pSenHndl, chId);
                        }
                        if(status < 0)
                        {
                            appLogPrintf("Error : Failed to start sensor at channel Id %d \n", chId);
                        }
                        if( NULL ==g_pSenHndl[chId])
                        {
                            appLogPrintf("Error : sensor handle at channel Id %d = NULL \n", chId);
                        }
                    }

                    chId++;

                    channel_mask = channel_mask >> 1U;
                }
            }

            break;
        case (uint32_t)IM_SENSOR_CMD_STREAM_OFF:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_STREAM_OFF \n");
#endif

            sensor_name = (char*)(cmd_param);
            (void)memcpy((void*)&channel_mask, (const void*)&cmd_param[ISS_SENSORS_MAX_NAME], sizeof(uint32_t));
            pSenHndl = IssSensor_getSensorHandleFromName(sensor_name);
            if(NULL == pSenHndl)
            {
                status = -1;
                appLogPrintf("ERROR : NULL handle returned for sensor %s \n", sensor_name);
            }
            else
            {
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("IM_SENSOR_CMD_STREAM_ON:  channel_mask = 0x%.2x\n", channel_mask);
#endif
                status = 0;

                chId = 0;

                while( (channel_mask > 0U) && (chId < ISS_SENSORS_MAX_CHANNEL) )
                {
                    if((channel_mask & 0x1U) != 0U)
                    {
                        status = IssSensor_Stop(pSenHndl, chId);
                        if(status < 0)
                        {
                            appLogPrintf("Warning : Failed to stop sensor at channel Id %d \n", chId);
                        }
                        if( NULL !=g_pSenHndl[chId])
                        {
                            appLogPrintf("Warning : sensor handle at channel Id %d is not NULL \n", chId);
                        }
                    }

                    chId++;

                    channel_mask = channel_mask >> 1U;
                }
            }
            break;
        case (uint32_t)IM_SENSOR_CMD_PWROFF:
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("ImageSensor_RemoteServiceHandler: IM_SENSOR_CMD_PWROFF \n");
#endif
            sensor_name = (char*)(cmd_param);
            (void)memcpy((void*)&channel_mask, (const void*)&cmd_param[ISS_SENSORS_MAX_NAME], sizeof(uint32_t));
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("IM_SENSOR_CMD_PWROFF : channel_mask = 0x%x \n", channel_mask);
#endif
            pSenHndl = IssSensor_getSensorHandleFromName(sensor_name);
            if (pSenHndl != NULL)
            {
                status = IssSensor_PowerOff(pSenHndl, channel_mask);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                appLogPrintf("ERROR : NULL handle returned for sensor %s \n", sensor_name);
            }
            /* LDRA_JUSTIFY_END */
            IssSensor_CloseHandle();
            break;
        case (uint32_t)IM_SENSOR_CMD_CTL:
            status = IssSensor_Control(NULL, 0, (void*)cmd_param, NULL);
            break;
        case (uint32_t)IM_SENSOR_CMD_DETECT:
            {
                uint8_t channel_id = 0;
                int32_t probeStatus;
                uint16_t serializers_detected = 0;
                (void)memcpy((void*)&channel_mask, (const void*)cmd_param, sizeof(uint32_t));

                probeStatus = IssSensor_detect_serializer((int8_t)BOARD_CSI_INST_0, &serializers_detected);
                if(probeStatus != 0)
                {
                    appLogPrintf("IssSensor_detect_serializer returned 0x%.2x \n", probeStatus);
                }

                probeStatus = IssSensor_detect_serializer((int8_t)BOARD_CSI_INST_1, &serializers_detected);
                if(probeStatus != 0)
                {
                    appLogPrintf("IssSensor_detect_serializer returned 0x%.2x \n", probeStatus);
                }

                if(gFusion2Det != 0)
                {
                    probeStatus = IssSensor_detect_serializer((int8_t)BOARD_CSI_INST_2, &serializers_detected);
                    if(probeStatus != 0)
                    {
                        appLogPrintf("IssSensor_detect_serializer returned 0x%.2x \n", probeStatus);
                    }
                }

                while(channel_mask > 0U)
                {
                    if((channel_mask & 0x1U) != 0U)
                    {
                        int32_t detectedSensor;
                        uint32_t shiftResult = (uint32_t)1U << channel_id;
                        int32_t tmp = (int32_t)shiftResult;
                        if(((uint16_t)tmp & serializers_detected) != 0U)
                        {
                            if(g_detectedSensors[channel_id] == 0xFFU)
                            {
                                /*Sensor has not been detected at this port yet. Try to detect */
                                detectedSensor = IssSensor_detect_sensor(channel_id);
                                if(detectedSensor < 0)
                                {
                                    appLogPrintf("IM_SENSOR_CMD_DETECT found serializer but no sensor at chId %d \n", chId);
                                    detectedSensor = 0xFF;
                                }
                                else
                                {
#ifdef ENABLE_DEBUG_IMAGING
                                    appLogPrintf("IM_SENSOR_CMD_DETECT : Found sensor %d at port %d \n", detectedSensor, channel_id);
#endif
                                }
                                g_detectedSensors[channel_id] = (uint8_t)detectedSensor;
                                cmd_param[channel_id] = (uint8_t)detectedSensor;
                            }
                            else
                            {
                                /* Sensor has previously been detected at this port. It might be getting used.
                                *  Trying to probe can overwrite the configuration
                                *  Return the previously detected ID instead of probing again */
#ifdef ENABLE_DEBUG_IMAGING
                                appLogPrintf("IM_SENSOR_CMD_DETECT : Previously found sensor %d at port %d \n", g_detectedSensors[chId], chId);
#endif
                                cmd_param[channel_id] = (uint8_t)g_detectedSensors[channel_id];
                            }
                        }else
                        {
                            appLogPrintf("IM_SENSOR_CMD_DETECT No serializer found at port %d \n", channel_id);
                            cmd_param[channel_id] = 0xFF;
                        }
                    }
                    else
                    {
                        cmd_param[channel_id] = 0xFF;
                    }
                    channel_mask = channel_mask >> 1;
                    if(channel_id < (ISS_SENSORS_MAX_CHANNEL-1U))
                    {
                        channel_id = channel_id + 1U;
                    }
                    else
                    {
                        channel_id = (uint8_t)(ISS_SENSORS_MAX_CHANNEL-1U);
                        appLogPrintf("IM_SENSOR_CMD_DETECT: Max channel_id reached!!\n");
                    }
                }
                status = 0;
            }
            break;
        default:
            appLogPrintf("ImageSensor_RemoteServiceHandler : Unsupported command : %d\n", cmd);
            status = -1;
            break;
    }

    return status;
}

int32_t IssSensor_DeInit(void)
{
    int32_t status = -1;
    status = appRemoteServiceUnRegister(IMAGE_SENSOR_REMOTE_SERVICE_NAME);
    if(status!=0)
    {
        appLogPrintf(" REMOTE_SERVICE_SENSOR: ERROR: Unable to unregister remote service sensor handler\n");
    }
    return status;
}

/*******************************************************************************
 *  Local Functions Definition
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Get deserializer I2C address and channel/bus number
 * 
 * \param[in]   desHubInst    Deserializer hub instance. (Fusion1: 0, 1)
 *                              (Fusion2: 0, 1, 2)
 * \param[out]  i2cBus           Channel/bus number of desHubInstId
 * \param[out]  i2cAddr         I2C address of desHubInstId
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
static void IssSensor_getDeserializerI2cAddr(uint8_t desHubInst,
    uint8_t *i2cBus, uint8_t *i2cAddr)
{
    #if defined (MCU_PLUS_SDK)
    *i2cBus = 0;
    if(gFusion2Det != 0)
    {
        Board_fpdUb9702GetI2CAddr(i2cAddr, (uint32_t)desHubInst);
    }
    else{
        Board_fpdUb960GetI2CAddr(i2cAddr, (uint32_t)desHubInst);
    }
    #else
    if(gFusion2Det != 0)
    {
        Board_fpdUb9702GetI2CAddr(desHubInst, NULL, i2cBus, i2cAddr);
    }
    else
    {
        Board_fpdU960GetI2CAddr(i2cBus, i2cAddr, (uint32_t)desHubInst);
    }
    #endif
}

static int32_t checkForHandle(const IssSensors_Handle* handle)
{
    int32_t found = -1;
    uint32_t cnt;

    /* Find a free entry in the sensor table */
    for (cnt = 0U; cnt < ISS_SENSORS_MAX_SENSOR_HANDLES; cnt ++)
    {
        if (handle == gIssSensorTable[cnt])
        {
            found = 0;
            break;
        }
    }

    return (found);
}

/**
 *******************************************************************************
 *
 * \brief Initialize I2C on bus with Fusion board
 * 
 * \param[in]  i2cInst  I2C instance/bus Fusion board is on
 *
 * \return  0 if success
 * \return -1 otherwise
 *
 *******************************************************************************
*/
static int32_t setupI2CInst(uint8_t i2cInst)
{
    int32_t status = -1;

#if !defined(MCU_PLUS_SDK)
    I2C_Params i2cParams;

    /* Initializes the I2C Parameters */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;

    /* Configures the I2C instance with the passed parameters*/
    if(NULL == gISS_Sensor_I2cHandle)
    {
       gISS_Sensor_I2cHandle = I2C_open(i2cInst, &i2cParams);
    }
#else
    gISS_Sensor_I2cHandle = I2C_getHandle(CONFIG_I2C2);
#endif /* !defined(MCU_PLUS_SDK) */

    if(gISS_Sensor_I2cHandle == NULL)
    {
        appLogPrintf("[setupI2CInst]: Failed to initialze I2C bus %d!!!\n", i2cInst);
        status = -1;
    }
    else
    {
        status = 0;
    }

    return status;
}

static int32_t Serialzier_WriteReg(uint8_t slaveI2cAddr,
                                   uint16_t regAddr,
                                   uint8_t regVal)
{
    int32_t status = -1;
    uint8_t regValLocal = regVal;
    status = Board_i2c8BitRegWr(gISS_Sensor_I2cHandle, slaveI2cAddr, (uint8_t)regAddr,
                                &regValLocal, 1U, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        appLogPrintf(
            "Error writing 0x%.2x to serializer (slaveAddr: 0x%.2x) register 0x%.2x!\n",
            regVal, slaveI2cAddr, regAddr);
    }
    return status;
}

#if defined(B7_IMPLEMENTATION)
static int32_t Serialzier_ReadReg(uint8_t slaveI2cAddr,
                                uint16_t regAddr,
                                uint8_t *regVal)
{
    int32_t status = -1;
    status = Board_i2c8BitRegRd(gISS_Sensor_I2cHandle, slaveI2cAddr, regAddr,
                                regVal, 1U, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        appLogPrintf(
            "Error reading from serializer(0x%.2x) register 0x%.2x!\n",
            slaveI2cAddr, regAddr);
    }
    return status;
}
#endif

static int32_t Deserializer_ReadReg(uint8_t slaveI2cAddr,
                                    uint16_t regAddr,
                                    uint8_t *regVal)
{
    int32_t status = -1;
    status = Board_i2c8BitRegRd(gISS_Sensor_I2cHandle, slaveI2cAddr, (uint8_t)regAddr,
                                regVal, 1U, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        appLogPrintf(
            "Error reading from de-serializer(0x%.2x) register 0x%.2x!\n",
            slaveI2cAddr, regAddr);
    }
    return status;
}

static int32_t Deserializer_WriteReg(uint8_t   slaveI2cAddr,
                                     uint16_t  regAddr,
                                     uint8_t   regVal)
{
    int32_t status = -1;
    uint8_t regValLocal = regVal;
    status = Board_i2c8BitRegWr(gISS_Sensor_I2cHandle, slaveI2cAddr, (uint8_t)regAddr,
                                &regValLocal, 1U, SENSOR_I2C_TIMEOUT);
    if(0 != status)
    {
        appLogPrintf(
            "Error writing 0x%.2x to de-serializer(0x%.2x) register 0x%.2x!\n",
            regVal, slaveI2cAddr, regAddr);
    }
    return status;
}

/* Detects which ports of a de-serializer hub instance have a connected serializer
by checking LOCK_STS and PORT_PASS registers for each de-serializer port.
Returns a 4-bit mask - 0 = Ser Not Detected, 1 = Ser Detected */
static int32_t IssSensor_detect_serializer(int8_t desHubInstId, uint16_t *cameras_detected)
{
    uint8_t mask = 0x0;
    uint8_t pageSelectOrig = 0x0;
    uint8_t regVal = 0x0;
    uint8_t found = 0x0;
    int32_t status = -1;
    uint8_t  desI2cInstId = 0;
    uint8_t  desI2cAddr;

    uint8_t portSelCfg[] = {DES_PORT_SEL_0, DES_PORT_SEL_1,
                            DES_PORT_SEL_2, DES_PORT_SEL_3};

    IssSensor_getDeserializerI2cAddr((uint8_t)desHubInstId, &desI2cInstId,
        &desI2cAddr);

    status = Deserializer_ReadReg(desI2cAddr, 0x4C, &pageSelectOrig);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status != 0)
    /* LDRA_JUSTIFY_END */
    {
        status = status;
    }
    else
    {
        /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
    <justification end> */
    /* TODO: MISRA.BUILTIN_NUMERIC - Builtin numeric type 'int' is used */
    for(uint8_t i = 0; i < (sizeof(portSelCfg) / sizeof(portSelCfg[0])); i++)
    {
        vx_bool shouldBreak = (vx_bool)vx_false_e;

        /* Select port on deserializer */
        status = Deserializer_WriteReg(desI2cAddr, 0x4C, portSelCfg[i]);
        if(status != 0)
        {
            shouldBreak = (vx_bool)vx_true_e;
        }
        else
        {
            /* Read RX_PORT_STS1(0x4D) */
            status = Deserializer_ReadReg(desI2cAddr, 0x4D, &regVal);
            if(status != 0)
            {
                shouldBreak = (vx_bool)vx_true_e;
            }
            else
            {
                /* Extract PORT_PASS(0x4D[1]) and LOCK_STS(0x4D[0]) */
                found = (uint8_t)(regVal & 0x3U);
                if(found == 0x3U)
                {
                    mask |= (uint8_t)(portSelCfg[i] & 0x0FU);
                }
            }
        }

        if((vx_bool)vx_true_e == shouldBreak)
        {
            break;
        }
    }
    /* LDRA_JUSTIFY_END */

        status = Deserializer_WriteReg(desI2cAddr, 0x4C, pageSelectOrig);

        *cameras_detected |= (uint16_t)((uint16_t)mask << ((uint8_t)desHubInstId * 4U));
    }

    return status;
}

/* Probes all the sensors in gIssSensorTable */
/* If a sensor is detected, the index of the sensor in gIssSensorTable is copied to sensor_id_found */
static int32_t IssSensor_detect_sensor(uint8_t chId)
{
    int32_t probeStatus = -1;
    int32_t i = 0;
    IssSensors_Handle *pSenHndl;
    int32_t sensor_id_found = -1;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    while ((sensor_id_found < 0) && (i < (int32_t)ISS_SENSORS_MAX_SENSOR_HANDLES))
    /* LDRA_JUSTIFY_END */
    {
        pSenHndl = gIssSensorTable[i];
        if(NULL !=pSenHndl)
        {
            probeStatus = pSenHndl->sensorFxns->probe(chId, pSenHndl);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            if(probeStatus == 0 )
            {
                /*Sensor found at index i*/
#ifdef ENABLE_DEBUG_IMAGING
                appLogPrintf("Found sensor %s at port # %d\n", pSenHndl->createPrms->name, chId);
#endif
                sensor_id_found = i;
            }
            /* LDRA_JUSTIFY_END */
        }
        i++;
    }

    return sensor_id_found;
}

#if defined(B7_IMPLEMENTATION)
static void IssSensor_GetPortSelectRegisterValue(uint32_t chId, uint8_t *portSel, uint8_t *portSelInd)
{
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    doubt
    Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
    <justification end> */
    switch(chId)
    {
    /* LDRA_JUSTIFY_END */
        case 0:
        case 4:
        case 8:
            *portSel = DES_PORT_SEL_0;
            *portSelInd = 0x04;
            break;
        case 1:
        case 5:
        case 9:
            *portSel = DES_PORT_SEL_1;
            *portSelInd = 0x08;
            break;
        case 2:
        case 6:
        case 10:
            *portSel = DES_PORT_SEL_2;
            *portSelInd = 0x0C;
            break;
        case 3:
        case 7:
        case 11:
            *portSel = DES_PORT_SEL_3;
            *portSelInd = 0x10;
            break;
        case 0xFF:
            /* Read port 0; Write all ports (broadcast mode) */
            *portSel = 0x0F;

            /*  Simultaneous Writes To FPD-Link RX Ports 0-3 Analog (FPD_RX_PORT_ANALOG) Registers */
            *portSelInd = 0x18;
            break;
        default:
            appLogPrintf("Error : Invalid channel ID 0x%x \n", chId);
            return;
    }
}

static int32_t IssSensor_DesSelPort(uint8_t chId)
{
    /* write code to select port based on chID */
    int32_t status = -1;
    
    uint8_t  portSel = 0;
    uint8_t  portSelInd = 0;
    int8_t desHubInstId = -1;

    static I2cParams portSelScript[] = {
        {0x4C, 0x00, 0x00},  // read/write port registers
        {0xB0, 0x00, 0x00},   // read/write port indirect registers

        {0xFFFF,0x00, 0x00} /* End script */
    };


    IssSensor_GetPortSelectRegisterValue(chId, &portSel, &portSelInd);
    if(portSel == 0)
    {
        appLogPrintf("IssSensor_DesSelPort : IssSensor_GetPortSelectRegisterValue incorrect channel ID = %d\n", chId);
        status = -1;
        return status;
    }

    /* Set read/write for port registers */
    portSelScript[0].nRegValue = portSel;

    /* Set read/write for port indirect registers */
    portSelScript[1].nRegValue = portSelInd;

    desHubInstId = getDeserializerInstIdFromChId(chId);
    IssSensor_cfgDesScript(portSelScript, desHubInstId);

    // appLogPrintf("Port select: 0x4C = 0x%02x \t 0xB0 = 0x%02x for chID = %d\n", portSel, portSelInd, chId);

    return status;
}

static int32_t configure_serializer(uint8_t chId, IssSensor_ModuleParams moduleInfo, uint8_t serAddr)
{
    int32_t status = -1;

    static I2cParams ub971InitScript[] = {
        {0x4B, 0x02, 0x00}, /* disable BC alternate mode auto detect */
        {0x49, 0x06, 0x00}, /* decrease link detect timer on 971 BC-RX */
        {0x0A, 0x12, 0x00}, /* Speed up I2C bus watchdog timer to ~50us */
        {0xFFFF,0x00, 0x00} /* End script */
    };

    static I2cParams ub953InitScript[] = {
        {0x0A, 0x12, 0x00},  /* Speed up I2C bus watchdog timer to ~50us */
        {0xFFFF,0x00, 0x00} /* End script */
    };

    
    if(moduleInfo.fpdLink == IMAGE_SERDES_FPD_LINK_3)
    {
        status = ub953_cfgScript(0, serAddr, ub953InitScript);
        if(status != 0)
        {
            appLogPrintf("configure_serializer : [FPD3] Failed to configure serializer for port %d\n", chId);
        }
    }
    else if(moduleInfo.fpdLink == IMAGE_SERDES_FPD_LINK_4)
    {
        if(gFusion2Det == (vx_bool)vx_true_e)
        {
            status = ub953_cfgScript(0, serAddr, ub971InitScript);
            if(status != 0)
            {
                appLogPrintf("configure_serializer : [FPD4] Failed to configure serializer for port %d\n", chId);
            }
        }
    }
    else
    {
        appLogPrintf("configure_serializer: Incorrect FPD version: %d\n", moduleInfo.fpdLink);
    }

    return status;
}

static int32_t configure_deserializer_port(uint8_t  desI2cAddr, IMAGE_SERDES_FPD_LINK fpdLink, uint32_t chId)
{
    int32_t status = -1;

    chId = chId % 4;

    I2cParams ub9702_ub953portModeScript[] = {
        {0xE4, 0x02, 0x00},// Set FPD functional mode to FPD3 Async CSI Mode
        {0x58, 0x46, 0x00FFFFFF},// BC_FREQ_SELECT=(PLL_FREQ/3200) Mbps  
        {0xB1, 0xA8, 0x00},// write to indirect registers
        {0xB2, 0x80, 0x00},// set aeq_lock_mode = 1

        /* FPD4 BC CONFIG START */
        {0xB1, 0x04, 0x00},
        {0xB2, 0x40, 0x00},// remove HiZ of NMOS drv of spare driver
        {0xB1, 0x1B, 0x00},
        {0xB2, 0x28, 0x00},// remove HiZ of PMOS drv of spare driver & disable 1 CMR ladder
        {0xB1, 0x0D, 0x00},
        {0xB2, 0x7F, 0x01},// enable the FPD3 spare driver time.sleep(.1)
        /* FPD4 BC CONFIG END */

        {0x0C, 0x1 << chId, 0x00FFFFFF},// readback already enabled ports
        {0xA0, 0x10 | chId, 0x00FFFFFF},// Set VC channel

        {0xFFFF,0x00, 0x00} /* End script */
    };

    I2cParams ub9702_ub971portModeScript[] = {
        {0xE4, 0x00, 0x00}, // Set FPD functional mode to FPD4 Sync Mode
        {0x58, 0x06, 0x00FFFFFF}, /* Masked write */
        
        /* FPD4 BC CONFIG START */
        {0xB1, 0x04, 0x00},
        {0xB2, 0x00, 0x00},// set FPD PBC drv into FPD IV mode
        {0xB1, 0x1B, 0x00},
        {0xB2, 0x00, 0x00},// set FPD PBC drv into FPD IV mode
        /* FPD4 BC CONFIG END */

        {0xB1, 0x21, 0x00},
        {0xB2, 0x2F, 0x00},// set 960 AEQ timer to 400us/step
        {0x3C, 0x0F, 0x00},// disable lock lost feature

        {0x0C, 0x1 << chId, 0x00FFFFFF}, /* Masked write */
        {0xA0, 0x10 | chId, 0x00FFFFFF}, /* Set VC channel */
        {0x3C, 0x9F, 0x00}, // enable lock lost feature and enable Exclusive Forwarding
        {0xC7, 0x10, 0x00}, // CSI Transmit Port2 Enable
        
        {0xFFFF,0x00, 0x00} /* End script */
    };

    IssSensor_DesSelPort(chId);
    
    if(fpdLink == IMAGE_SERDES_FPD_LINK_3)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("\t Configuring port %d to FPD3 Mode\n ", chId);
#endif
        status = IssSensor_cfgDesScriptCustom(desI2cAddr, ub9702_ub953portModeScript, 0);
    }
    else if(fpdLink == IMAGE_SERDES_FPD_LINK_4)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("\t Configuring port %d to FPD4 Mode\n ", chId);
#endif
        status = IssSensor_cfgDesScriptCustom(desI2cAddr, ub9702_ub971portModeScript, 0);
    }
    else
    {
        appLogPrintf(" error %s: %d\n", __func__, __LINE__);
    }

    return status;
}

static int32_t disable_deserializer_port(uint8_t  desI2cAddr, IMAGE_SERDES_FPD_LINK fpdLink, uint32_t chId)
{
    int32_t status = -1;

    uint8_t rxPortEn[4] = {0xFE, 0xFD, 0xFB, 0xF7};

    static I2cParams ub9702portDisableScript[] = {
        {0x0C, 0x00, 0x00FFFFFF},  // reg_0x0C_mask = {0: 0xFE, 1: 0xFD, 2: 0xFB, 3: 0xF7} (read_0x0C & reg_0x0C_mask[1])
        {0x87, 0x1B, 0x00},  // disable FPD Rx and FPD BC CMR
        {0x58, 0xEF, 0x00FFFFFF},  // reg_0x58 & 0xEF  disable FPD BC Tx
        {0x80, 0x15, 0x00}, // disable PI
        {0xB1, 0x4D, 0x00}, // 
        {0xB2, 0x03, 0x00}, // disable AEQ
        {0xB1, 0x85, 0x00}, // 
        {0xB2, 0x09, 0x00}, // PI disabled and oDAC disabled
        {0xB1, 0x34, 0x00}, // 
        {0xB2, 0x20, 0x00}, // AEQ configured for disabled link
        {0xB1, 0x79, 0x00}, // 
        {0xB2, 0x45, 0x00}, // disable AEQ clock and DFE 
        {0xB1, 0xA5, 0x00}, // 
        {0xB2, 0x82, 0x00}, // pwrdown FPD3 CDR

        {0xFFFF,0x00, 0x00} /* End script */
    };

    ub9702portDisableScript[0].nRegValue = rxPortEn[chId % 4];

    // appLogPrintf(" \t\tdesI2cAddr: 0x%02x rxPortEn[chId %% 4] = 0x%02x\n", desI2cAddr, rxPortEn[chId % 4]);

    status = IssSensor_cfgDesScriptCustom(desI2cAddr, ub9702portDisableScript, 1);

    return status;
}

static int32_t checkerlockrecover_des(uint8_t  desI2cAddr, uint32_t chId)
{
    int32_t status = -1;

    uint8_t rx_AEQ_limit = 0x10; // Assumption that max AEQ should be under 16 
    uint8_t prev_AEQ = 0xFF;

    uint8_t rx_AEQ, rx_lock;

    for(uint8_t retry = 0; retry < ISS_SENSORS_MAX_SUPPORTED_SENSOR; retry++)
    {
        /* lock status */
        Deserializer_ReadReg(desI2cAddr, 0x4D, &rx_lock);
        rx_lock = rx_lock & 0x01;

        /* read/write indirect registers */
        Deserializer_WriteReg(desI2cAddr, 0xB1, 0x2C);

        Deserializer_ReadReg(desI2cAddr, 0xB2, &rx_AEQ);

        if(rx_lock == 0)
        {
            static I2cParams ub9702AeqPart1[] = {
                {0xE4, 0x10, 0x01}, // time.sleep(0.0001)
                {0xE4, 0x00, 0x00}, // Toggle Recovery Bit
                
                {0xB1, 0x28, 0x00}, // Restart AEQ by changing max to 0 --> 0x23
                {0xB2, 0x00, 0x02}, // time.sleep(0.02)
                {0xB2, 0x23, 0x02}, // AEQ Restart

                {0xFFFF,0x00, 0x00} /* End script */
            };

            IssSensor_cfgDesScriptCustom(desI2cAddr, ub9702AeqPart1, 0);

            appLogPrintf("Port not locked at 1st check recovery loop\n\n");
            appLogPrintf("Port Lock = 0x%02x Retry=%d\n\n", rx_lock, retry);

            if(retry==3)
            {
                appLogPrintf("!!ERROR!! Max number of retry for FPD-Link IV lock!!ERROR!!\n\n");
                appLogPrintf("Please check connections and hardware\n\n");

                return -1;
            }
            else if( (rx_lock == 1) && (rx_AEQ >= rx_AEQ_limit) )
            {
                if(rx_AEQ != prev_AEQ)
                {
                    static I2cParams ub9702AeqPart2[] = {
                        {0xE4, 0x10, 0x01}, /* time.sleep(0.0001) */
                        {0xE4, 0x00, 0x00}, // Toggle Recovery Bit
                        {0xB1, 0x28, 0x00}, // Restart AEQ by changing max to 0 --> 0x23
                        {0xB2, 0x00, 0x02}, /* time.sleep(0.02) */
                        {0xB2, 0x23, 0x00}, // AEQ Restart time.sleep(0.02)

                        {0xFFFF,0x00, 0x00} /* End script */
                    };

                    IssSensor_cfgDesScriptCustom(desI2cAddr, ub9702AeqPart2, 0);
                    appLogPrintf("Port high AEQ at 1st check recovery loop\n\n");
                    appLogPrintf("Port Lock = 0x%02x RX_AEQ=02%02x Retry=%d\n\n", rx_lock, rx_AEQ, retry);
                    prev_AEQ = rx_AEQ;

                    if (retry==3)
                    {
                        appLogPrintf("!!ERROR!! Max number of retry for FPD-Link IV unstable AEQ !!ERROR!!\n\n");
                        appLogPrintf("Please check connections and hardware\n\n");
                        return -1;
                    }
                }
                else
                {
                    appLogPrintf("Lossy Cable Detected Lock =0x%02x,   RX_AEQ (0x%02x) > RX_AEQ_Limit(0x%02x),   Retry=%d \n\n", rx_lock, rx_AEQ, rx_AEQ_limit, retry);
                    appLogPrintf("// Port Will continue with initiation sequence but high AEQ \n\n");
                    break;
                }

            }
            else
            {
                appLogPrintf("Port FPD-Link IV Locked and AEQ Normal before setting AEQ window in en_AEQ_LMS \n\n");
            }
        }
    }

    return status;
}

static int32_t deserializer_aeq_optimization(uint8_t  desI2cAddr, uint32_t chId)
{
    int32_t status = -1;

    uint8_t read_aeq_init = 0x00;

    I2cParams UB9702aeqOptimizationScript[] = {
        {0xB1, 0x27, 0x00},  // read/write indirect registers
        {0xB2, 0x00, 0x00},  // read/write indirect registers
        {0xB1, 0x28, 0x00},  // Set AEQ Min
        {0xB2, 0x00, 0x00},  // Set AEQ Max
        {0xB1, 0x2B, 0x00},  // Set AEQ offset
        {0xB2, 0x00, 0x00},  //Set Offset to 0
        {0xB1, 0x9E, 0x00},  // read/write indirect registers
        {0xB2, 0x00, 0x00},  // enable AEQ tap2
        {0xB1, 0xF1, 0x00},  // read/write indirect registers
        {0xB2, 0x00, 0x00},  // Set VGA Gain 1 Gain 2 override to 0
        {0xB1, 0x77, 0x00},  // read/write indirect registers
        {0xB2, 0x80, 0x00},  // Set VGA Initial Sweep Gain to 0
        {0xB1, 0x74, 0x00},  // read/write indirect registers
        {0xB2, 0x00, 0x00},  // Set VGA_Adapt (VGA Gain) override to 0 (thermometer encoded)
        {0xB1, 0x2E, 0x00},  // read/write indirect registers
        {0xB2, 0x40, 0x00},  // Enable VGA_SWEEP
        {0xB1, 0xF0, 0x00},  // read/write indirect registers
        {0xB2, 0x00, 0x00},  // Disable VGA_SWEEP_GAIN_OV, disable VGA_TUNE_OV

        {0xB1, 0x72, 0x00},  // read/write indirect registers
        {0xB2, 0x2B, 0x00},  // Set VGA HIGH Threshold to 43
        {0xB1, 0x73, 0x00},  // read/write indirect registers
        {0xB2, 0x12, 0x00},  // Set VGA LOW Threshold to 18
        {0xB1, 0x87, 0x00},  // read/write indirect registers
        {0xB2, 0x20, 0x00},  // set vga_sweep_th to 32
        {0xB1, 0x21, 0x00},  // read/write indirect registers
        {0xB2, 0xEF, 0x00},  // set AEQ timer to 400us/step and parity threshold to 7

        // {0x4C, 0x01, 0x00},  // read/write port0 registers
        // {0xB0, 0x04, 0x00},  // read/write port0 indirect registers

        {0xD4,0x10, 0x00}, // Hold State Machine in reset
        {0x01,0x21, 0x01},  // Soft reset and release GPIO hold time.sleep(0.01)
        {0xD4,0x00, 0x02}, // Release SM in reset time.sleep(0.015) //wait time for stable lock

        // {0x4C, 0x01, 0x00},  // read/write port0 registers
        // {0xB0, 0x04, 0x00},  // read/write port0 indirect registers

        {0xB1, 0x90, 0x00},  // read/write indirect registers
        {0xB2, 0x40, 0x00},  // enable DFE LMS
        {0xB1, 0x71, 0x00},  // read/write indirect registers
        {0xB2, 0x20, 0x01},  // Disable VGA Gain1 override time.sleep(0.0001)
        {0xB2, 0x00, 0x00},  // Disable VGA Gain2 override

        {0xFFFF,0x00, 0x00} /* End script */
    };

    /* read/write indirect registers */
    Deserializer_WriteReg(desI2cAddr, 0xB1, 0x2C);

    /* initial FPD-Link IV AEQ value */
    Deserializer_ReadReg(desI2cAddr, 0xB2, &read_aeq_init);

    /* read/write indirect registers */
    UB9702aeqOptimizationScript[1].nRegValue = read_aeq_init;

    /* Set AEQ Max */
    UB9702aeqOptimizationScript[3].nRegValue = read_aeq_init+1;

    IssSensor_cfgDesScriptCustom(desI2cAddr, UB9702aeqOptimizationScript, 0);

    status = 0;
    return status;
}

static int32_t ub953_temp_ramp(uint8_t  desI2cAddr, uint8_t  serI2cAddr, uint32_t chId)
{
    int32_t status = -1;

    uint8_t read_aeq_init = 0x00;
    uint8_t reg_0x58 = 0x00;
    uint8_t temp_code_95x = 0x00;
    uint8_t vdac_ori = 0x00;
    uint8_t vdac = 0x00;
    uint8_t vco_tweak = 0x00;

    Deserializer_ReadReg(desI2cAddr, 0x58, &reg_0x58);
    reg_0x58 = reg_0x58 | 0x40; // Enable I2C Passthrough
    Deserializer_WriteReg(desI2cAddr, 0x58, reg_0x58);

    Deserializer_ReadReg(desI2cAddr, 0x53, &temp_code_95x);

    Serialzier_WriteReg(serI2cAddr, 0xB0, 0x04);
    Serialzier_WriteReg(serI2cAddr, 0xB1, 0x4B);
    Serialzier_ReadReg(serI2cAddr, 0xB2, &vdac_ori);
    vdac = vdac_ori | 0x20;

    Serialzier_WriteReg(serI2cAddr, 0xB1, 0x4C);
    Serialzier_ReadReg(serI2cAddr, 0xB2, &vco_tweak);
    vco_tweak = (vco_tweak & 0x8F) | 0x70;
    Deserializer_WriteReg(serI2cAddr,0xB2, vco_tweak);

    Serialzier_WriteReg(serI2cAddr, 0xB1, 0x4B);

    switch (temp_code_95x)
    {
        case 0:
        case 1:
            vdac = vdac - 1;
            break;
        case 2:
        case 3:
            /* vdac does not change */
            break;
        case 4:
        case 5:
        case 6:
            vdac = vdac + 1;
            break;
        case 7:
            vdac = vdac + 3;
            break;
        default:
            appLogPrintf(" Warning %s: %d\n", __func__, __LINE__);
            break;
    }

    Serialzier_WriteReg(serI2cAddr, 0xB2, vdac);

    reg_0x58 = reg_0x58 | 0x20; // Enable all auto ACK I2C Passthrough
    Deserializer_WriteReg(desI2cAddr, 0x58, reg_0x58);

    Serialzier_WriteReg(serI2cAddr, 0x01, 0x01); // Soft Rest to apply PLL updates
    appLogWaitMsecs(20); /* wait 20 ms */

    reg_0x58 = reg_0x58 & 0x1F; // Disable auto ACK and I2C passthrough
    Deserializer_WriteReg(desI2cAddr, 0x58, reg_0x58);

    return status;
}

static int32_t deserializer_enable_csitx(uint8_t  desI2cAddr)
{
    int32_t status = -1;

    uint8_t regVal = 0x00;

    I2cParams UB9702csiTxEnableScript[] = {
        /* CSI-TX Port Config */
        {0x32, 0x03, 0x20}, /* Read CSI-TX Port 0, Write CSI-TX Ports 0 and 1 */
        {0x20, 0xF0, 0x20}, /* Disable FPD-Link RX forwarding */
        {0x33, 0x03, 0x20}, /* 4 CSI-2 Lanes; Continous clock mode, CSI-2 output enabled */
        {0x20, 0x00, 0x10}, /* Enable FPD-Link RX forwarding (all ports) to CSI-TX port 0 */

        {0xFFFF,0x00, 0x00} /* End script */
    };

    
    status = IssSensor_cfgDesScriptCustom(desI2cAddr, UB9702csiTxEnableScript, 0);
    
    Deserializer_ReadReg(desI2cAddr, 0x20, &regVal);

    regVal = 0x00;
    Deserializer_ReadReg(desI2cAddr, 0x33, &regVal);

    return status;
}

static int32_t deserializer_setBackChannel50mbps(uint8_t  desI2cAddr)
{
    int32_t status = -1;

    I2cParams UB9702setBackChannel50mbpsScript[] = {
        /* Set back channel to 50Mbps */
        {0xB0, 0x16, 0x20}, // PLL_CTRL indirect registers
        {0xB1, 0x8A, 0x20},
        {0xB2, 0x00, 0x20}, // Set RX PLL MASH divider order for 8GHz
        {0xB1, 0x87, 0x20},
        {0xB2, 0xA0, 0x20}, // Set RX PLL NCOUNT setting for 8GHz
        {0xB1, 0x83, 0x20},
        {0xB2, 0x00, 0x20}, // Set RX PLL MASH numerator setting [23:16] for 8GHz
        {0xB1, 0x84, 0x20},
        {0xB2, 0x00, 0x20}, // Set RX PLL MASH numerator setting [15:8] for 8GHz
        {0xB1, 0x85, 0x20},
        {0xB2, 0x00, 0x20}, // Set RX PLL MASH numerator setting [7:0] for 8GHz

        {0xFFFF,0x00, 0x00} /* End script */
    };

    status = IssSensor_cfgDesScriptCustom(desI2cAddr, UB9702setBackChannel50mbpsScript, 0);

    return status;
}

static int32_t deserializer_setCsiSpeed(uint8_t  desI2cAddr, IMAGE_SERDES_FPD_LINK fpdLink)
{
    int32_t status = -1;

    I2cParams UB9702setCsiSpeed1500mbpsScript[] = {
        /* Set CSI Transmitter to 1500Mbps */
        {0x1F, 0x00, 0x20},
        {0xC9, 0x0F, 0x20},
        {0xB0, 0x1C, 0x20}, // CSI-2_ANALOG indirect registers
        {0xB1, 0x92, 0x20},
        {0xB2, 0x80, 0x20},

        {0xFFFF,0x00, 0x00} /* End script */
    };

    I2cParams UB9702setCsiSpeed2500mbpsScript[] = {
        /* Set CSI Transmitter to 2500Mbps */
        {0x1F, 0x10, 0x20},
        {0xC9, 0x19, 0x20},
        {0xB0, 0x1C, 0x20}, // CSI-2_ANALOG indirect registers
        {0xB1, 0x92, 0x20},
        {0xB2, 0x80, 0x20},

        {0xFFFF,0x00, 0x00} /* End script */
    };

    if (fpdLink == IMAGE_SERDES_FPD_LINK_3)
    {
        appLogPrintf(" setting CSI speed to 1500Mbps\n");
        status = IssSensor_cfgDesScriptCustom(desI2cAddr, UB9702setCsiSpeed1500mbpsScript, 0);
    }
    else
    {
        appLogPrintf(" setting CSI speed to 2500Mbps\n");
        status = IssSensor_cfgDesScriptCustom(desI2cAddr, UB9702setCsiSpeed2500mbpsScript, 0);
    }

    return status;
}

/* chMask will be bit mask of cameras being programmed */
static int32_t IssSensor_DeserializerInit_B7(void)
{
    int32_t status = -1;

    uint8_t  desI2cBusNum = 0;
    uint8_t  desI2cAddr;
    int8_t desHubInstId = -1;
    uint8_t  regVal;
    uint32_t chId = 0;
    IssSensors_Handle *pSenHndl;
    IssSensor_CreateParams *sensorInfo;
    IssSensor_ModuleParams *moduleInfo;

    uint8_t portSel[4] = {DES_PORT_SEL_0, DES_PORT_SEL_1, DES_PORT_SEL_2, 
        DES_PORT_SEL_3};

    uint8_t i = 0;
    uint8_t tmpRd = 0x00;
    uint8_t reg_0x4D = 0x00;

    /* g_pSenHndl should have already been populated in IM_SENSOR_CMD_PWRON
    (this function gets called in config which is after power on) */
    chId = 0;
    while(chId < ISS_SENSORS_MAX_SUPPORTED_SENSOR)
    {
        desHubInstId = getDeserializerInstIdFromChId(chId);
        IssSensor_getDeserializerI2cAddr(desHubInstId, &desI2cBusNum, &desI2cAddr);

        /* Select port on deserializer (direct and indirect registers) */
        IssSensor_DesSelPort(chId);

        if(g_pSenHndl[chId] != NULL)
        {
            pSenHndl = g_pSenHndl[chId];
            sensorInfo = pSenHndl->createPrms;
            moduleInfo = &(sensorInfo->moduleInfo);

            /* --------------- BEGIN 971 serializer programming --------------- */
            /* Configure serializer alias with freeze and auto ACK in deserializer */
            Deserializer_WriteReg(desI2cAddr, 0x5B, moduleInfo->serI2cAddr+1);
            Deserializer_WriteReg(desI2cAddr, 0x5C, (sensorInfo->i2cAddrSer[chId] << 1)+1);

            Deserializer_ReadReg(desI2cAddr, 0x58, &regVal);
            
            /* Enable I2C Passthrough with auto ACK */
            regVal = 0x00;
            Deserializer_ReadReg(desI2cAddr, 0x58, &regVal);
            regVal = regVal | 0x60;
            Deserializer_WriteReg(desI2cAddr, 0x58, regVal);

            configure_serializer(chId, *moduleInfo, sensorInfo->i2cAddrSer[chId]);
            
            /* Disable auto ACK and I2C passthrough */
            regVal = regVal & 0x1F;
            Deserializer_WriteReg(desI2cAddr, 0x58, regVal);
            /* --------------- END 971 serializer programming --------------- */
            
            if(moduleInfo->fpdLink == IMAGE_SERDES_FPD_LINK_4)
            {
                Deserializer_WriteReg(desI2cAddr, 0xD4, 0x10); /* Hold State Machine in reset */
                Deserializer_WriteReg(desI2cAddr, 0xB1, 0x28);
                Deserializer_WriteReg(desI2cAddr, 0xB2, 0x00); /* set AEQ max to 0 */
            }

            configure_deserializer_port(desI2cAddr, moduleInfo->fpdLink, chId);
        }
        else
        {
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("\t Disabling port %d\n", chId);
#endif
            disable_deserializer_port(desI2cAddr, moduleInfo->fpdLink, chId);
        }
        chId++;
    }

    for(uint8_t hubInst = 0; hubInst < gDeserializerHubs; hubInst++)
    {
        desI2cAddr = 0x00;
        IssSensor_getDeserializerI2cAddr(hubInst, &desI2cBusNum, &desI2cAddr);

        Deserializer_WriteReg(desI2cAddr, 0x01, 0x21); /* Soft reset and release GPIO hold */
        appLogWaitMsecs(10); /* delay 10ms */
        if(moduleInfo->fpdLink == IMAGE_SERDES_FPD_LINK_3)
        {
            appLogWaitMsecs(105); /* delay 105ms for stable lock */
        }
    }

    chId = 0;
    while(chId < ISS_SENSORS_MAX_SUPPORTED_SENSOR)
    {
        desHubInstId = getDeserializerInstIdFromChId(chId);
        IssSensor_getDeserializerI2cAddr(desHubInstId, &desI2cBusNum, &desI2cAddr);

        /* Select port on deserializer (direct and indirect registers) */
        IssSensor_DesSelPort(chId);

        if(g_pSenHndl[chId] != NULL)
        {
            pSenHndl = g_pSenHndl[chId];
            sensorInfo = pSenHndl->createPrms;
            moduleInfo = &(sensorInfo->moduleInfo);

            if(moduleInfo->fpdLink == IMAGE_SERDES_FPD_LINK_4)
            {
                Deserializer_WriteReg(desI2cAddr, 0xD4, 0x00); /* Release State Machine in reset */
                appLogWaitMsecs(5); /* delay 5ms */

                Deserializer_WriteReg(desI2cAddr, 0xB1, 0x28); /* AEQ restart */
                Deserializer_WriteReg(desI2cAddr, 0xB2, 0x23); /* AEQ restart */
                appLogWaitMsecs(20); /* delay 20ms for stable lock FPD-Link IV */

                checkerlockrecover_des(desI2cAddr, chId);
            }
        }
        chId++;
    }

    chId = 0;
    while(chId < ISS_SENSORS_MAX_SUPPORTED_SENSOR)
    {
        desHubInstId = getDeserializerInstIdFromChId(chId);
        IssSensor_getDeserializerI2cAddr(desHubInstId, &desI2cBusNum, &desI2cAddr);

        /* Select port on deserializer (direct and indirect registers) */
        IssSensor_DesSelPort(chId);

        if(g_pSenHndl[chId] != NULL)
        {
            pSenHndl = g_pSenHndl[chId];
            sensorInfo = pSenHndl->createPrms;
            moduleInfo = &(sensorInfo->moduleInfo);

            if(moduleInfo->fpdLink == IMAGE_SERDES_FPD_LINK_4)
            {
                deserializer_aeq_optimization(desI2cAddr, chId);
            }
            else if(moduleInfo->fpdLink == IMAGE_SERDES_FPD_LINK_3)
            {
                ub953_temp_ramp(desI2cAddr, sensorInfo->i2cAddrSer[chId], chId);
            }
            else
            {
                appLogPrintf(" error %s: %d\n", __func__, __LINE__);
            }
        }
        chId++;
    }

    chId = 0;
    while(chId < ISS_SENSORS_MAX_SUPPORTED_SENSOR)
    {
        desHubInstId = getDeserializerInstIdFromChId(chId);
        IssSensor_getDeserializerI2cAddr(desHubInstId, &desI2cBusNum, &desI2cAddr);

        /* Select port on deserializer (direct and indirect registers) */
        IssSensor_DesSelPort(chId);

        if(g_pSenHndl[chId] != NULL)
        {
            pSenHndl = g_pSenHndl[chId];
            sensorInfo = pSenHndl->createPrms;
            moduleInfo = &(sensorInfo->moduleInfo);

            /* Configure serializer alias without freeze and remove auto ACK in deserializer */
            Deserializer_WriteReg(desI2cAddr, 0x5B, moduleInfo->serI2cAddr);
            Deserializer_WriteReg(desI2cAddr, 0x5C, (sensorInfo->i2cAddrSer[chId] << 1));

            /* Enable I2C Passthrough */
            regVal = 0x00;
            Deserializer_ReadReg(desI2cAddr, 0x58, &regVal);
            regVal = regVal | 0x40;
            Deserializer_WriteReg(desI2cAddr, 0x58, regVal);

            Deserializer_ReadReg(desI2cAddr, 0x4D, 0x00); // Clear status register of errors.
            Deserializer_ReadReg(desI2cAddr, 0x4E, 0x00); // Clear status register of errors.
            Deserializer_ReadReg(desI2cAddr, 0x55, 0x00); // Clear status register of errors.
            Deserializer_ReadReg(desI2cAddr, 0x56, 0x00); // Clear status register of errors. 
            Deserializer_ReadReg(desI2cAddr, 0x7A, 0x00); // Clear status register of errors.  
            Deserializer_ReadReg(desI2cAddr, 0x7B, 0x00); // Clear status register of errors.
        }
        else
        {
            // appLogPrintf("\tskipping 4 for chid: %d \n", chId);
        }
        chId++;
    }
    appLogWaitMsecs(5);

    for(uint8_t deshubInst = 0; deshubInst < gDeserializerHubs; deshubInst++)
    {
        IssSensor_getDeserializerI2cAddr(deshubInst, &desI2cBusNum, &desI2cAddr);
        deserializer_setBackChannel50mbps(desI2cAddr);
        deserializer_setCsiSpeed(desI2cAddr, moduleInfo->fpdLink);
    }

    status = 0;
    return status;
}
#endif

/* Checks which cameras are connected to the specified Deserializer */
/* Returns a 4-bit mask - 0 = Camera Not Detected, 1 = Camera Detected */
static int32_t IssSensor_DeserializerInit(void)
{
    int32_t status = -1;
    uint8_t  desI2cBusNum = 0;
    uint8_t  desI2cAddr;
    tivx_mutex desCfgLock;
    vx_bool bDet = (vx_bool)vx_false_e; /* default to Fusion1 configuration */

    (void)tivxMutexCreate(&desCfgLock);
    (void)tivxMutexLock(desCfgLock);

    IssSensor_getDeserializerI2cAddr((uint8_t)BOARD_CSI_INST_0, &desI2cBusNum,
        &desI2cAddr);

    /* Check for the Fusion2 board */
    #if defined (MCU_PLUS_SDK)
    bDet = (Board_detectBoard(BOARD_ID_FUSION2, desI2cBusNum) == BTRUE) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    #if defined(SOC_AM62A)
    /* AM62A AutoSDK for 5MP out of box support */
    bDet = (vx_bool)vx_true_e;
    #endif
    #else
    bDet = (Board_detectBoard(BOARD_ID_FUSION2) == BTRUE) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    #endif

    if(bDet == (vx_bool)vx_true_e)
    {
        appLogPrintf("Fusion2 Board Detected, using Fusion2 configuration\n");
        gFusion2Det = (vx_bool)vx_true_e;
        desCfgCommon_ptr = ub9702DesCfg_Common;
        gDeserializerHubs = 3U;
    }
    else
    {
        appLogPrintf("Fusion1 Board configuration\n");
        gFusion2Det = (vx_bool)vx_false_e;
        desCfgCommon_ptr = ub960DesCfg_Common;
        gDeserializerHubs = 2U;
    }

    status = setupI2CInst(desI2cBusNum);
    if(status!=0)
    {
        appLogPrintf(" I2C ERROR \n");
        (void)tivxMutexUnlock(desCfgLock);
        (void)tivxMutexDelete(&desCfgLock);
        return status;
    }

    status = powerOnDeserializers();
    if(status != 0)
    {
        appLogPrintf(
            "IssSensor_DeserializerInit Error: powerOnDeserializers "
            "returned %d \n", status);
    }

    /* Deserializer non-port specific configuration */
    for(uint8_t hubInst = 0; hubInst < gDeserializerHubs; hubInst++)
    {
#if defined(B7_IMPLEMENTATION)

        Deserializer_WriteReg(desI2cAddr, 0x01, 0x02); /* hard reset */

        if(gFusion2Det == vx_true_e)
        {
            IssSensor_getDeserializerI2cAddr(hubInst, &desI2cBusNum, &desI2cAddr);
    
            /* Disable lock lost feature */
            Deserializer_WriteReg(desI2cAddr, 0x3C, 0x0F);
        }
        else
        {
            status = IssSensor_cfgDesScript(desCfgCommon_ptr, (int8_t)hubInst);
        }
#else
        status = IssSensor_cfgDesScript(desCfgCommon_ptr, (int8_t)hubInst);
#endif /* defined(B7_IMPLEMENTATION) */
    }

#if defined(B7_IMPLEMENTATION)
    if(gFusion2Det == vx_true_e)
    {
        status = IssSensor_DeserializerInit_B7();
        if(status != 0)
        {
            appLogPrintf("Error : IssSensor_DeserializerInit_B7 returned %d while configuring DES \n", status);
        }
    }
#endif /* defined(B7_IMPLEMENTATION) */

    (void)tivxMutexUnlock(desCfgLock);
    (void)tivxMutexDelete(&desCfgLock);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Powers on deserailzer hubs on Fusion1/2 boards
 *
 * \return  0 if success
 * \return -1 otherwise
 
 *******************************************************************************
*/
static int32_t powerOnDeserializers(void)
{
    int32_t status = -1;

    #if defined(SOC_AM62A)
    uint8_t regVal = 0x00;
    uint8_t regAddr = 0x00;
    /* Commented out old TCA6424 configuration - replaced with TCA6424 driver API below */
    // I2C_Handle i2c1Handle;
    // static const I2cParams tca6424Config[] = {
    //   /* addr, val, delay */
    //     // {0x08, 0x00, 0x00},
    //     // {0x09, 0x00, 0x00},
    //     // {0x0a, 0x00, 0x00},
    //     {0x0D, 0xFD, 0x32}, /* (CSI_RSTz) Set P11 as Output  */
    //     {0x05, 0x02, 0x32}, /* (CSI_RSTz) Set P11 HIGH */
    //     {0x0e, 0xe7, 0x32}, /* (CSI_EN, CSI_SEL2) Set P24, P23 as Outputs */
    //     {0x06, 0xf7, 0x32}  /* (CSI_EN) Set P24 HIGH, (CSI_SEL2) Set P23 LOW */
    // };
    //
    // uint32_t tca6424SlaveAddr = TCA6424_I2C_ADDR;
    //
    // i2c1Handle = I2C_getHandle(CONFIG_I2C1);
    //
    // for(int i = 0; i < (sizeof(tca6424Config)/sizeof(tca6424Config[0])); i++)
    // {
    //     regAddr = tca6424Config[i].nRegAddr;
    //     regVal = tca6424Config[i].nRegValue;
    //     status = Board_i2c8BitRegWr(i2c1Handle, tca6424SlaveAddr, regAddr, &regVal, 1U, SENSOR_I2C_TIMEOUT);
    //     if(0 == status)
    //     {
    //         appLogPrintf("write 0x%.2x to TCA6424 register 0x%.2x \n", regVal, regAddr);
    //     }
    //     else
    //     {
    //     appLogPrintf("Error writing to TCA6424 register 0x%.2x \n", regAddr);
    //     }
    //     appLogWaitMsecs(200);
    // }

#if defined(BUILD_QNX)
    printf("[I2C1] Performing hardware reset to ensure clean state...\n");

    volatile uint32_t *i2c1_iec = (volatile uint32_t *)(AM62A_I2C1_BASE + 0x30);    /* IEC offset */
    volatile uint32_t *i2c1_buf = (volatile uint32_t *)(AM62A_I2C1_BASE + 0x94);    /* BUF offset */
    volatile uint32_t *i2c1_systest = (volatile uint32_t *)(AM62A_I2C1_BASE + 0xBC); /* SYSTEST offset */
    volatile uint32_t *i2c1_con = (volatile uint32_t *)(AM62A_I2C1_BASE + 0xA4);    /* CON offset */
    volatile uint32_t *i2c1_syss = (volatile uint32_t *)(AM62A_I2C1_BASE + 0x90);   /* SYSS offset */

    /* Disable interrupts */
    *i2c1_iec = AM62A_I2C_INT_ALL;

    /* Disable DMA */
    *i2c1_buf = 0;

    /* Disable test mode */
    *i2c1_systest = 0;

    /* Disable I2C controller if enabled */
    if (*i2c1_con & AM62A_I2C_CON_EN) {
        *i2c1_con = 0;
    }

    /* Trigger soft reset by enabling controller */
    *i2c1_con = AM62A_I2C_CON_EN;

    /* Wait for reset to complete (poll SYSS.RDONE bit) */
    int timeout = 2000;
    while ((*i2c1_syss & AM62A_I2C_SYSS_RDONE) == 0) {
        if (--timeout <= 0) {
            printf("[I2C1] ERROR: Reset timeout! SYSS=0x%x\n", *i2c1_syss);
            return -1;
        }
        appLogWaitMsecs(1);
    }

    /* Disable controller after reset */
    *i2c1_con = 0;

    printf("[I2C1] Hardware reset completed successfully\n");
#endif
    /* New TCA6424 configuration using driver API (ported from csirx_fusion2_test_main.c) */
    TCA6424_Config TCA6424_IOexp_config;
    TCA6424_Params TCA6424_IOexp_params = {
        .i2cInstance = CONFIG_I2C1,
        .i2cAddress = IO_EXPANDER_ADDR
    };

#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Opening TCA6424@0x%02x on CONFIG_I2C1...\n", IO_EXPANDER_ADDR);
#endif
    status = TCA6424_open(&TCA6424_IOexp_config, &TCA6424_IOexp_params);
    if (status != 0)
    {
        appLogPrintf("Error: TCA6424_open failed (status=%d)\n", status);
        return status;
    }
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] TCA6424_open - SUCCESS\n");
#endif

    /* Configure Pin 20 (CSI_EN) as output and set HIGH */
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Configuring Pin 20 (CSI_EN) as OUTPUT...\n");
#endif
    status = TCA6424_config(&TCA6424_IOexp_config, 20, TCA6424_MODE_OUTPUT);
    if (status != 0)
    {
        appLogPrintf("Error: TCA6424_config Pin 20 failed (status=%d)\n", status);
        TCA6424_close(&TCA6424_IOexp_config);
        return status;
    }
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Pin 20 (CSI_EN) configured as OUTPUT - SUCCESS\n");
#endif

#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Setting Pin 20 (CSI_EN) to HIGH...\n");
#endif
    status = TCA6424_setOutput(&TCA6424_IOexp_config, 20, TCA6424_OUT_STATE_HIGH);
    if (status == 0)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("[TCA6424] Pin 20 (CSI_EN) set HIGH - SUCCESS (Camera power enabled)\n");
#endif
    }
    else
    {
        appLogPrintf("Error: Pin 20 (CSI_EN) set HIGH failed (status=%d)\n", status);
        TCA6424_close(&TCA6424_IOexp_config);
        return status;
    }

#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Waiting 50ms for power-up...\n");
#endif
    appLogWaitMsecs(50);  /* 50ms for power-up */

    /* Configure Pin 9 (CSI_RSTz) as output and set HIGH */
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Configuring Pin 9 (CSI_RSTz) as OUTPUT...\n");
#endif
    status = TCA6424_config(&TCA6424_IOexp_config, 9, TCA6424_MODE_OUTPUT);
    if (status != 0)
    {
        appLogPrintf("Error: TCA6424_config Pin 9 failed (status=%d)\n", status);
        TCA6424_close(&TCA6424_IOexp_config);
        return status;
    }
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Pin 9 (CSI_RSTz) configured as OUTPUT - SUCCESS\n");
#endif

#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Setting Pin 9 (CSI_RSTz) to HIGH (releasing reset)...\n");
#endif
    status = TCA6424_setOutput(&TCA6424_IOexp_config, 9, TCA6424_OUT_STATE_HIGH);
    if (status == 0)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("[TCA6424] Pin 9 (CSI_RSTz) set HIGH - SUCCESS (Reset released)\n");
#endif
    }
    else
    {
        appLogPrintf("Error: Pin 9 (CSI_RSTz) set HIGH failed (status=%d)\n", status);
        TCA6424_close(&TCA6424_IOexp_config);
        return status;
    }

    /* Configure Pin 19 (CSI_SEL2) as output and set LOW */
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Configuring Pin 19 (CSI_SEL2) as OUTPUT...\n");
#endif
    status = TCA6424_config(&TCA6424_IOexp_config, 19, TCA6424_MODE_OUTPUT);
    if (status != 0)
    {
        appLogPrintf("Error: TCA6424_config Pin 19 failed (status=%d)\n", status);
        TCA6424_close(&TCA6424_IOexp_config);
        return status;
    }
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Pin 19 (CSI_SEL2) configured as OUTPUT - SUCCESS\n");
#endif

#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Setting Pin 19 (CSI_SEL2) to LOW...\n");
#endif
    status = TCA6424_setOutput(&TCA6424_IOexp_config, 19, TCA6424_OUT_STATE_LOW);
    if (status == 0)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("[TCA6424] Pin 19 (CSI_SEL2) set LOW - SUCCESS (CSI channel selected)\n");
#endif
    }
    else
    {
        appLogPrintf("Error: Pin 19 (CSI_SEL2) set LOW failed (status=%d)\n", status);
        TCA6424_close(&TCA6424_IOexp_config);
        return status;
    }

    /* Close handle after use */
    TCA6424_close(&TCA6424_IOexp_config);
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("[TCA6424] Closed TCA6424@0x%02x on CONFIG_I2C1...\n", IO_EXPANDER_ADDR);
#endif

    if (status == 0)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("[TCA6424] ========== All pins configured successfully! ==========\n");
        appLogPrintf("[TCA6424] Summary: CSI_EN=HIGH, CSI_RSTz=HIGH, CSI_SEL2=LOW\n");
        appLogPrintf("[TCA6424] Waiting 200ms for UB9702 power-up and I2C to become ready...\n");
        appLogWaitMsecs(200);  /* Wait 200ms after reset release */
        appLogPrintf("[TCA6424] Power-up delay complete - proceeding with I2C communication\n");
#endif
    }
    else
    {
        appLogPrintf("Error: TCA6424@0x%02x configuration FAILED (status=%d)\n", IO_EXPANDER_ADDR, status);
        return status;
    }

    /* TCA9543A is a 2-channel I2C switch. It must be configured to channel 1 (out of channel 0 and 1) for I2C communication
        to CSI direct link and fusion board. Both are the same channel */
#ifdef ENABLE_DEBUG_IMAGING
    appLogPrintf("AM62A Board Init: Configuring TCA9543A mux on CONFIG_I2C2\n");
#endif
    regVal = 0x02;  /* Enable channel 1 */
    status = Board_i2c8BitRegWrSingle(
        gISS_Sensor_I2cHandle,
        0x71,
        &regVal,
        0x20);

    if (status != 0)
    {
        appLogPrintf("AM62A Board Init: Failed to configure TCA9543A mux (status=%d)\n", status);
        return status;
    }
    else{
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf("\nsuccesfully configured I2C switch to channel 1.\n");
#endif
    }

    #elif !defined(MCU_PLUS_SDK)
    uint8_t regVals[] = {0xFF, 0xFE};
    uint32_t tca6408SlaveAddr = TCA6408_I2C_ADDR_1;
    uint8_t regAddr = TCA6408_CONFIG_REG;

    for(uint8_t i = 0; i < (sizeof(regVals)/sizeof(regVals[0])); i++)
    {
        status = Board_i2c8BitRegWr(gISS_Sensor_I2cHandle, tca6408SlaveAddr, regAddr, &regVals[i], 1U, SENSOR_I2C_TIMEOUT);
        if(0 == status)
        {
#ifdef ENABLE_DEBUG_IMAGING
            appLogPrintf("write 0x%.2x to TCA6408 register 0x%.2x \n", regVals[i], regAddr);
#endif
        }
        else
        {
            appLogPrintf("Error writing to TCA6408 register 0x%.2x \n", regAddr);
        }
        appLogWaitMsecs(200);
    }
    #else
    uint32_t tca9543I2cSlaveAddr = 0x70;
    uint8_t regVal = 0x01; /* Enable Channel 0 */

    /* Main I2C2 interacts with CSI0_I2C2 via Channel 0 of the TCA9543APWR,
    *  a 2-Channel I2C Bus Switch. A value of 0x01 has to be written to the
    *  TCA9543APWR to enable I2C communication through Channel 0. The address
    *  of TCA9543APWR is 0x70 on J7AEN (defined by hardware).
    */

    status = Board_i2c8BitRegWrSingle(
        gISS_Sensor_I2cHandle,
        tca9543I2cSlaveAddr,
        &regVal,
        0x20);

    if(status == 0)
    {
#ifdef ENABLE_DEBUG_IMAGING
        appLogPrintf(
            "Write 0x%.2x to TCA9543APWR (0x%.2x) successful!!!\n",
            regVal,
            tca9543I2cSlaveAddr);
#endif
    }
    else
    {
        appLogPrintf(
            "Error writing 0x%.2x to TCA9543APWR (0x%.2x)!!!\n",
            regVal,
            tca9543I2cSlaveAddr);
    }
    #endif

    return status;
}
