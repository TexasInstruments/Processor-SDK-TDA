/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
 * permitted with respect to any software provided in bina
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#if defined (MCU_PLUS_SDK)
#include <drivers/i2c/v0/i2c.h>
#include <board/board_control.h>
#else
#include <ti/board/src/devices/board_devices.h>
#include <i2c/i2c.h>
#include <i2c/soc/i2c_soc.h>
#include <ti/board/src/devices/common/common.h>
#include <ti/board/board.h>

#include <ti/board/src/devices/fpd/ds90ub960.h>
#endif  /* if defined (MCU_PLUS_SDK) */

#include <utils/sensors/include/app_sensors.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/sciclient/include/app_sciclient_wrapper_api.h>

#include <utils/remote_service/include/app_remote_service.h>

#include "ov2775_cfg.h"
#include "imx390_cfg.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */



/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

static int32_t appSetupI2CInst(uint8_t i2cInst);
static int32_t appCloseI2CInst(void);
static int32_t appOv2775Config(const AppSensorCmdParams *prms);
static int32_t appImx390Config(AppSensorCmdParams *prms);

int32_t appRemoteServiceSensorHandler(char *service_name, uint32_t cmd,
    void *prm, uint32_t prm_size, uint32_t flags);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static I2C_Handle gI2cHandle = NULL;


/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t appI2cInit(void)
{
    #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    SET_DEVICE_STATE_ON(TISCI_DEV_I2C5);
    #endif

    #if !defined (MCU_PLUS_SDK)
    /* Initialize I2C Driver */
    for(uint32_t index = 0; index < I2C_HWIP_MAX_CNT; index++)
    {
        I2C_HwAttrs i2cConfig;

        (void)I2C_socGetInitCfg(index, &i2cConfig);
        i2cConfig.enableIntr = false;
        (void)I2C_socSetInitCfg(index, &i2cConfig);
    }
    #endif

    /* Initializes the I2C */
    I2C_init();

    return 0;
}

int32_t appI2cDeInit(void)
{
    /* I2C_deInit(); */

    return 0;
}

int32_t appRemoteServiceSensorInit(void)
{
    int32_t status = 0;

    status = appRemoteServiceRegister(
        APP_REMOTE_SERVICE_SENSOR_NAME, appRemoteServiceSensorHandler);
    if(status!=0)
    {
        appLogPrintf(
            " REMOTE_SERVICE_SENSOR: ERROR: Unable to register remote service sensor handler\n");
    }
    return status;
}

int32_t appRemoteServiceSensorDeInit(void)
{
    int32_t status = 0;

    status = appRemoteServiceUnRegister(APP_REMOTE_SERVICE_SENSOR_NAME);
    if(status!=0)
    {
        appLogPrintf(
            " REMOTE_SERVICE_SENSOR: ERROR: Unable to register remote service sensor handler\n");
    }

    return status;
}

int32_t appRemoteServiceSensorHandler(char *service_name, uint32_t cmd,
    void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t                 status = 0;
    AppSensorCmdParams     *cmdPrms = NULL;

    (void)service_name;
    (void)flags;

    if ((NULL != prm) && (sizeof(AppSensorCmdParams) == prm_size))
    {
        cmdPrms = (AppSensorCmdParams *)prm;

        appLogPrintf(
            " REMOTE_SERVICE_SENSOR: Received command %08x to configure %d sensor(s) !!!\n",
                cmd,
                cmdPrms->numSensors);

        switch(cmd)
        {
            case APP_REMOTE_SERVICE_SENSOR_CMD_CONFIG_OV2775:
                status = appOv2775Config(cmdPrms);
                break;
            case APP_REMOTE_SERVICE_SENSOR_CMD_CONFIG_IMX390:
                status = appImx390Config(cmdPrms);
                break;
            default:
                status = -1;
                break;
        }

        if (0 == status)
        {
            appLogPrintf(
                " REMOTE_SERVICE_SENSOR: Sensor(s) configuration done !!!\n");
        }
        else
        {
            appLogPrintf(
                " REMOTE_SERVICE_SENSOR: ERROR: Sensor(s) configuration failed !!!\n");
        }
    }
    else
    {
        appLogPrintf(
            " REMOTE_SERVICE_SENSOR: Invalid parameters passed !!!\n");
    }


    return (status);
}

static int32_t appOv2775Config(const AppSensorCmdParams *prms)
{
    int32_t status = 0;
    uint32_t cnt, cnt1;
    uint32_t numSers, numSensors;
    uint8_t serI2cAddr[APP_MAX_SENSORS] = { 0 } ;
    uint8_t sensorI2cAddr[APP_MAX_SENSORS] = { 0 } ;
    uint8_t desI2cAddr, i2cAddr;
    uint8_t regAddr8, regVal8;
#if !defined(MCU_PLUS_SDK)
    uint8_t i2cInst;
#endif
    uint16_t regAddr16;

    #if defined (MCU_PLUS_SDK)
    Board_fpdUb960GetI2CAddr(&desI2cAddr, BOARD_CSI_INST_0);
    status = 0;
    #else
    Board_fpdU960GetI2CAddr(&i2cInst, &desI2cAddr, BOARD_CSI_INST_0);
    status = appSetupI2CInst(i2cInst);
    #endif


    /* Check for maximum number of sensors & no. of ports supported */
    if ((prms->numSensors > APP_MAX_SENSORS) || (prms->portNum > APP_PORT_NUM_MAX))
    {
        appLogPrintf(" REMOTE_SERVICE_SENSOR: ERROR: Incorrect Number of sensors or ports in parameters\n");
        status = -1;
    }

    if (0 == status)
    {
        numSers = 0u;
        numSensors = 0u;
        i2cAddr = desI2cAddr;
        for (cnt = 0;
            cnt < (sizeof(ov2775_ub960_cfg)/(sizeof(ov2775_ub960_cfg[0])));
            cnt ++)
        {
            regAddr8 = (uint8_t)(ov2775_ub960_cfg[cnt][0] & 0xFFU);
            regVal8 = (uint8_t)(ov2775_ub960_cfg[cnt][1] & 0xFFU);

            if ((0x65U == regAddr8) && (numSers < APP_MAX_SENSORS))
            {
                serI2cAddr[numSers] = regVal8 >> 1u;
                numSers ++;
            }

            if ((0x66U == regAddr8) && (numSensors < APP_MAX_SENSORS))
            {
                sensorI2cAddr[numSensors] = regVal8 >> 1u;
                numSensors ++;
            }

            status = Board_i2c8BitRegWr(gI2cHandle, i2cAddr,
                regAddr8, &regVal8, 1, BOARD_I2C_TRANSACTION_TIMEOUT);
            if (0 != status)
            {
                appLogPrintf (
                    " REMOTE_SERVICE_SENSOR: Failed to Set UB960 register %x: Value:%x\n",
                    regAddr8, regVal8);
                break;
            }
            else
            {
                appLogWaitMsecs(ov2775_ub960_cfg[cnt][2]);
            }
        }
    }

    if (0 == status)
    {
        if (numSensors != numSers)
        {
            appLogPrintf(
                " REMOTE_SERVICE_SENSOR: ERROR: Incorrect UB960 configuration\n");
            status = -1;
        }
        else
        {
            if (prms->numSensors > numSensors)
            {
                appLogPrintf(
                    " REMOTE_SERVICE_SENSOR: ERROR: Incorrect Number of sensors in parameters\n");
                status = -1;
            }
        }
    }

    if (0 == status)
    {
        for (cnt1 = 0u; cnt1 < prms->numSensors; cnt1 ++)
        {
            i2cAddr = serI2cAddr[cnt1];
            for (cnt = 0;
                cnt < (sizeof(ov2775_ub953_cfg)/(sizeof(ov2775_ub953_cfg[0])));
                cnt ++)
            {
                regAddr8 = (uint8_t)(ov2775_ub953_cfg[cnt][0] & 0xFFU);
                regVal8 = (uint8_t)(ov2775_ub953_cfg[cnt][1] & 0xFFU);
                status = Board_i2c8BitRegWr(gI2cHandle, i2cAddr,
                    regAddr8, &regVal8, 1, BOARD_I2C_TRANSACTION_TIMEOUT);
                if (0 != status)
                {
                    appLogPrintf (
                        " REMOTE_SERVICE_SENSOR: Failed to Set UB953 register %x: Value:%x\n",
                        regAddr8, regVal8);
                    break;
                }
                else
                {
                    appLogWaitMsecs(ov2775_ub953_cfg[cnt][2]);
                }
            }
        }
    }

    if (0 == status)
    {
        for (cnt1 = 0u; cnt1 < prms->numSensors; cnt1 ++)
        {
            i2cAddr = sensorI2cAddr[cnt1];
            for (cnt = 0;
                cnt < (sizeof(ov2775_cfg)/sizeof(ov2775_cfg[0])); cnt ++)
            {
                regAddr16 = (uint16_t)(ov2775_cfg[cnt][0] & 0xFFFFU);
                regVal8 = (uint8_t)(ov2775_cfg[cnt][1] & 0xFFU);

                status = Board_i2c16BitRegWr(gI2cHandle, i2cAddr,
                    regAddr16, &regVal8, 1, BOARD_I2C_REG_ADDR_MSB_FIRST, BOARD_I2C_TRANSACTION_TIMEOUT);
                if (0 != status)
                {
                    appLogPrintf (
                        " REMOTE_SERVICE_SENSOR: Failed to Set IMX390 register %x: Value:0x%x\n",
                        regAddr16, regVal8);
                    break;
                }
                else
                {
                    appLogWaitMsecs(ov2775_cfg[cnt][2]);
                }
            }
        }
    }

    if (NULL != gI2cHandle)
    {
        (void)appCloseI2CInst();
    }

    return status;
}

static int32_t appImx390Config(AppSensorCmdParams *prms)
{
    int32_t status = 0;
    uint32_t cnt, cnt1;
    uint32_t numSers, numSensors;
    uint8_t serI2cAddr[APP_MAX_SENSORS] = { 0 } ;
    uint8_t sensorI2cAddr[APP_MAX_SENSORS] = { 0 } ;
    uint8_t desI2cAddr, i2cAddr;
    uint8_t regAddr8, regVal8;
#if !defined(MCU_PLUS_SDK)
    uint8_t i2cInst;
#endif
    uint16_t regAddr16;
    uint32_t portIdx;

    #if defined (MCU_PLUS_SDK)
    Board_fpdUb960GetI2CAddr(&desI2cAddr, BOARD_CSI_INST_0);
    status = 0;
    #else
    /* get I2C instance for port 0, this just to I2C instance for I2C driver Open */
    Board_fpdU960GetI2CAddr(&i2cInst, &desI2cAddr, BOARD_CSI_INST_0);
    status = appSetupI2CInst(i2cInst);
    #endif

    /* Check for maximum number of sensors & no. of ports supported */
    if ((prms->numSensors > APP_MAX_SENSORS) || (prms->portNum > APP_PORT_NUM_MAX))
    {
        appLogPrintf(" REMOTE_SERVICE_SENSOR: ERROR: Incorrect Number of sensors or ports in parameters\n");
        status = -1;
    }
    if (0 == status)
    {
        for (portIdx = 0U ; portIdx < prms->portNum ; portIdx++)
        {
            switch (prms->portIdMap[portIdx])
            {
                case 0U:
                    #if defined (MCU_PLUS_SDK)
                    Board_fpdUb960GetI2CAddr(&desI2cAddr, BOARD_CSI_INST_0);
                    #else
                    Board_fpdU960GetI2CAddr(&i2cInst, &desI2cAddr, BOARD_CSI_INST_0);
                    #endif
                break;
                case 1U:
                    #if defined (MCU_PLUS_SDK)
                    Board_fpdUb960GetI2CAddr(&desI2cAddr, BOARD_CSI_INST_1);
                    #else
                    Board_fpdU960GetI2CAddr(&i2cInst, &desI2cAddr, BOARD_CSI_INST_1);
                    #endif
                break;
                default:
                    status = -1;
                break;
            }
            if (0 == status)
            {
                appLogPrintf(" REMOTE_SERVICE_SENSOR: IMX390: Configuring UB960 ... !!!\n");
                numSers = 0u;
                numSensors = 0u;
                i2cAddr = desI2cAddr;
                for (cnt = 0;
                    cnt < (sizeof(imx390_ub960_cfg)/(sizeof(imx390_ub960_cfg[0])));
                    cnt ++)
                {
                    regAddr8 = (uint8_t)(imx390_ub960_cfg[cnt][0] & 0xFFU);
                    regVal8 = (uint8_t)(imx390_ub960_cfg[cnt][1] & 0xFFU);

                    if ((0x65U == regAddr8) && (numSers < APP_MAX_SENSORS))
                    {
                        serI2cAddr[numSers] = regVal8 >> 1u;
                        numSers ++;
                    }

                    if ((0x66U == regAddr8) && (numSensors < APP_MAX_SENSORS))
                    {
                        sensorI2cAddr[numSensors] = regVal8 >> 1u;
                        numSensors ++;
                    }

                    status = Board_i2c8BitRegWr(gI2cHandle, i2cAddr,
                        regAddr8, &regVal8, 1, BOARD_I2C_TRANSACTION_TIMEOUT);
                    if (0 != status)
                    {
                        appLogPrintf (
                            " REMOTE_SERVICE_SENSOR: Failed to Set UB960 register %x: Value:%x\n",
                            regAddr8, regVal8);
                        break;
                    }
                    else
                    {
                        appLogWaitMsecs(imx390_ub960_cfg[cnt][2]);
                    }
                }
            }
            if (0 == status)
            {
                if (numSensors != numSers)
                {
                    appLogPrintf(
                        " REMOTE_SERVICE_SENSOR: ERROR: Incorrect UB960 configuration\n");
                    status = -1;
                }
                else
                {
                    if (prms->numSensors > numSensors)
                    {
                        appLogPrintf(
                            " REMOTE_SERVICE_SENSOR: ERROR: Incorrect Number of sensors in parameters\n");
                        status = -1;
                    }
                }
            }

            if (0 == status)
            {
                for (cnt1 = 0u; cnt1 < prms->numSensors; cnt1 ++)
                {
                    appLogPrintf(" REMOTE_SERVICE_SENSOR: IMX390: Configuring UB953 for sensor %d of %d ... !!!\n", cnt1+1U, prms->numSensors);

                    i2cAddr = serI2cAddr[cnt1];
                    for (cnt = 0;
                        cnt < (sizeof(imx390_ub953_cfg)/(sizeof(imx390_ub953_cfg[0])));
                        cnt ++)
                    {
                        regAddr8 = (uint8_t)(imx390_ub953_cfg[cnt][0] & 0xFFU);
                        regVal8 = (uint8_t)(imx390_ub953_cfg[cnt][1] & 0xFFU);
                        status = Board_i2c8BitRegWr(gI2cHandle, i2cAddr,
                            regAddr8, &regVal8, 1, BOARD_I2C_TRANSACTION_TIMEOUT);
                        if (0 != status)
                        {
                            appLogPrintf (
                                " REMOTE_SERVICE_SENSOR: Failed to Set UB953 register %x: Value:%x\n",
                                regAddr8, regVal8);
                            break;
                        }
                        else
                        {
                            appLogWaitMsecs(imx390_ub953_cfg[cnt][2]);
                        }
                    }
                }
            }

            if (0 == status)
            {
                for (cnt1 = 0u; cnt1 < prms->numSensors; cnt1 ++)
                {
                    appLogPrintf(" REMOTE_SERVICE_SENSOR: IMX390: Configuring sensor %d of %d ... !!!\n", cnt1+1U, prms->numSensors);

                    i2cAddr = sensorI2cAddr[cnt1];
                    for (cnt = 0;
                        cnt < (sizeof(imx390_cfg)/sizeof(imx390_cfg[0])); cnt ++)
                    {
                        regAddr16 = (uint16_t)(imx390_cfg[cnt][0] & 0xFFFFU);
                        regVal8 = (uint8_t)(imx390_cfg[cnt][1] & 0xFFU);

                        status = Board_i2c16BitRegWr(gI2cHandle, i2cAddr,
                            regAddr16, &regVal8, 1, BOARD_I2C_REG_ADDR_MSB_FIRST, BOARD_I2C_TRANSACTION_TIMEOUT);
                        if (0 != status)
                        {
                            appLogPrintf (
                                " REMOTE_SERVICE_SENSOR: Failed to Set IMX390 register %x: Value:0x%x\n",
                                regAddr16, regVal8);
                            break;
                        }
                        else
                        {
                            appLogWaitMsecs(imx390_cfg[cnt][2]);
                        }
                    }
                }
            }
        }
    }

    if (0 == status)
    {
        for (portIdx = 0U ; portIdx < prms->portNum ; portIdx++)
        {
            switch (portIdx)
            {
                case 0U:
                    #if defined (MCU_PLUS_SDK)
                    Board_fpdUb960GetI2CAddr(&desI2cAddr, BOARD_CSI_INST_0);
                    #else
                    Board_fpdU960GetI2CAddr(&i2cInst, &desI2cAddr, BOARD_CSI_INST_0);
                    #endif
                break;
                case 1U:
                    #if defined (MCU_PLUS_SDK)
                    Board_fpdUb960GetI2CAddr(&desI2cAddr, BOARD_CSI_INST_1);
                    #else
                    Board_fpdU960GetI2CAddr(&i2cInst, &desI2cAddr, BOARD_CSI_INST_1);
                    #endif
                break;
                default:
                    status = -1;
                break;
            }
            if (0 == status)
            {
                i2cAddr = desI2cAddr;
                regAddr8 = 0x33;
                regVal8 = 0x3;
                status = Board_i2c8BitRegWr(gI2cHandle, i2cAddr,
                    regAddr8, &regVal8, 1, BOARD_I2C_TRANSACTION_TIMEOUT);
                if (0 != status)
                {
                    appLogPrintf ("Failed to enable CSI port\n");
                }
            }
        }
    }

    if (NULL != gI2cHandle)
    {
        (void)appCloseI2CInst();
    }

    return status;
}


/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static int32_t appSetupI2CInst(uint8_t i2cInst)
{
    int32_t status = 0;
    I2C_Params i2cParams;

    /* Initializes the I2C Parameters */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz; /* 400KHz */

    /* Configures the I2C instance with the passed parameters*/
    gI2cHandle = I2C_open(i2cInst, &i2cParams);
    if(gI2cHandle == NULL)
    {
        appLogPrintf(" I2C: ERROR: I2C Open Failed!!!\n");
        status = -1;
    }

    return status;
}


static int32_t appCloseI2CInst(void)
{
    I2C_close(gI2cHandle);
    gI2cHandle = NULL;

    return 0;
}

