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
#ifndef ISS_SENSORS_H_
#define ISS_SENSORS_H_

#include "TI/tivx_ext_raw_image.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#if defined(SOC_AM62A)
/* FPD-Link initialization using B7 implementation */
#define B7_IMPLEMENTATION
#endif

/**
 *  \brief RPC Commands application can send to sensor driver
 * \ingroup group_vision_function_imaging_sensordrv
 */
#define SENSOR_I2C_TIMEOUT    (5000U)

typedef enum
{
    IM_SENSOR_CMD_ENUMERATE = 0,
    IM_SENSOR_CMD_QUERY,
    IM_SENSOR_CMD_PWRON,
    IM_SENSOR_CMD_CONFIG,
    IM_SENSOR_CMD_STREAM_ON,
    IM_SENSOR_CMD_STREAM_OFF,
    IM_SENSOR_CMD_PWROFF,
    IM_SENSOR_CMD_CTL,
    IM_SENSOR_CMD_DEBUG,
    IM_SENSOR_CMD_DETECT,
    IM_SENSOR_CMD_D3_USER_I2C,
    IM_SENSOR_CMD_DETECT_PERSONALITY_BOARD,
    IM_SENSOR_CMD_CTL_MAX,
    IM_SENSOR_CMD_CTL_FORCE32BITS          = 0x7FFFFFFF
}IMAGE_SENSOR_COMMAND;

/**
 * \brief Enumeration for FPD-Link type
 *
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef enum
{
    IMAGE_SERDES_FPD_LINK_3 = 0,
    IMAGE_SERDES_FPD_LINK_4,
    IMAGE_SERDES_FPD_LINK_MAX,
    IMAGE_SERDES_FPD_LINK_FORCE32BITS           = 0x7FFFFFFF
}IMAGE_SERDES_FPD_LINK;


/**
 *  \brief IOCTLS supported as part of IM_SENSOR_CMD_CTL command
 * \ingroup group_vision_function_imaging_sensordrv
 */

typedef enum
{
    IMAGE_SENSOR_CTRLCMD_GETEXPPRG = 0,
    IMAGE_SENSOR_CTRLCMD_GETWBCFG,
    IMAGE_SENSOR_CTRLCMD_INIT_AEWB_CFG,
    IMAGE_SENSOR_CTRLCMD_SETEXPGAIN,
    IMAGE_SENSOR_CTRLCMD_GETEXPGAIN,
    IMAGE_SENSOR_CTRLCMD_SETWBGAIN,
    IMAGE_SENSOR_CTRLCMD_GETWBGAIN,
    IMAGE_SENSOR_CTRLCMD_DEBUG,
    IMAGE_SENSOR_CTRLCMD_READ_SENSOR_REG,
    IMAGE_SENSOR_CTRLCMD_WRITE_SENSOR_REG,
    IMAGE_SENSOR_CTRLCMD_READ_SENSOR_REG_IMMEDIATE_I2C,
    IMAGE_SENSOR_CTRLCMD_WRITE_SENSOR_REG_IMMEDIATE_I2C,
    IMAGE_SENSOR_CTRLCMD_MAX,
    IMAGE_SENSOR_CTRLCMD_FORCE32BITS          = 0x7FFFFFFF
}IMAGE_SENSOR_CTRLCMD;


#define IMAGE_SENSOR_REMOTE_SERVICE_NAME  "com.ti.image_sensor"
#define CMD_PARAM_SIZE 384

/* Maximum number of channels supported */
#if defined(SOC_J784S4) || defined(SOC_J742S2)
#define ISS_SENSORS_MAX_CHANNEL                 (12U)
#else
#define ISS_SENSORS_MAX_CHANNEL                 (8U)
#endif

/* Max characters in the sensor name */
#define ISS_SENSORS_MAX_NAME                    (32U)

/* Supports Sensor's name */
#define SENSOR_SONY_IMX390_UB953_D3                 "IMX390-UB953_D3"
#define SENSOR_X5C_UB971_OV                         "OX05C1-UB971-OV"
#define SENSOR_SONY_IMX728_UB971_SONY               "IMX728-UB971_SONY"
#define SENSOR_ONSEMI_AR0233_UB953_MARS             "AR0233-UB953_MARS"
#define SENSOR_ONSEMI_AR0820_UB953_LI               "AR0820-UB953_LI"
#define SENSOR_OX03F10_UB953_OV                     "OV0X3F-UB953_OV"
#define UB9XX_RAW_TESTPAT                           "UB9xxx_RAW12_TESTPATTERN"
#define UB96X_TESTPATTERN_UYVY                      "UB96x_UYVY_TESTPATTERN"
#define GW_AR0233_UYVY                              "GW_AR0233_UYVY"
#define SENSOR_OV2312_UB953_LI                      "OV2312-UB953_LI"
#define SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW   "IMX390-UB953-DISCOVERY-NARROW"
#define SENSOR_SONY_IMX390_UB953_DISCOVERY_MID      "IMX390-UB953-DISCOVERY-MID"
#define SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE     "IMX390-UB953-DISCOVERY-WIDE"

/* Defines ported from PDK for devices on MCU_PLUS_SDK */
#if defined (MCU_PLUS_SDK)

/** To send the MSB of a register offset address first */
#define BOARD_I2C_REG_ADDR_MSB_FIRST                        (0x0U)

#define I2C_400kHz                                          I2C_400KHZ

/**
 *  \anchor BOARD_csiInst
 *  \name   CSIRX Module instance
 *  \brief  CSIRX Module instance used to get the I2C instance.
 *
 *  @{
 */
/** \brief CSIRX Instance: 0 */
#define BOARD_CSI_INST_0             ((uint32_t) 0x0U)
/** \brief CSIRX Instance: 1 */
#define BOARD_CSI_INST_1             ((uint32_t) 0x1U)
/** \brief CSIRX Instance: 2 */
#define BOARD_CSI_INST_2             ((uint32_t) 0x2U)
/* @} */

#define BOARD_FPD_9702_CSI2_DES_HUB1                            (0U)
#define BOARD_FPD_9702_CSI2_DES_HUB2                            (1U)
#define BOARD_FPD_9702_CSI2_DES_HUB3                            (2U)

#endif /* MCU_PLUS_SDK */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *  \brief Enum for Sensor Features
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef enum
{
    ISS_SENSOR_FEATURE_HFLIP                = 0x1,
    /**< Horizontal Flip feature */
    ISS_SENSOR_FEATURE_VFLIP                = 0x2,
    /**< Vertical Flip feature */
    ISS_SENSOR_FEATURE_EMBEDDED_DATA        = 0x4,
    /**< Embedded extra data in the frame feature */
    ISS_SENSOR_FEATURE_MANUAL_EXPOSURE      = 0x8,
    /**< Support for Manual Exposure */
    ISS_SENSOR_FEATURE_MANUAL_GAIN          = 0x10,
    /**< Support for Manual Gain */
    ISS_SENSOR_FEATURE_LINEAR_MODE          = 0x20,
    /**< Support for Linear Mode output */
    ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE    = 0x40,
    /**< Support for Combined Compressed WDR Mode */
    ISS_SENSOR_FEATURE_TWO_FRAME_WDR_MODE    = 0x80,
    /**< Support for Two Pass WDR Mode */
    ISS_SENSOR_FEATURE_DCC_SUPPORTED        = 0x100,
    /**< DCC Feature */
    ISS_SENSOR_FEATURE_CFG_UC1               = 0x200,
    /**< Advanced sensor configuration for custom usecases.*/
    ISS_SENSOR_FEATURE_CFG_UC2               = 0x400,
    /**< Advanced sensor configuration for custom usecases.*/
    ISS_SENSOR_MAX_FEATURES                 = 0x800,
    /**< Max Features */
    ISS_SENSOR_FEATURE_FORCE32BITS          = 0x7FFFFFFF
    /**< Last enum to make it int32 */
} IssSensor_Features;


/**
 *  \brief Enum for different sensor exposures in WDR sensor
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef enum
{
    ISS_SENSOR_EXPOSURE_LONG,
    /**< Long Exposure output from sensor */
    ISS_SENSOR_EXPOSURE_SHORT,
    /**< Short Exposure output from sensor */
    ISS_SENSOR_EXPOSURE_VSHORT,
    /**< Very short Exposure output from sensor */
    ISS_SENSOR_MAX_EXPOSURE,
    ISS_SENSOR_EXPOSURE_FORCE32BITS = 0x7FFFFFFF
} IssSensor_Exposures;

/**
 *  \brief AEWB Algorithm Mode
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef enum {
    ALGORITHMS_ISS_AEWB_MODE_AWB        = 0,
    /**< Auto White Balance Mode only */
    ALGORITHMS_ISS_AEWB_MODE_AE         = 1,
    /**< Auto Exposure Mode only */
    ALGORITHMS_ISS_AEWB_MODE_AEWB       = 2,
    /**< Auto Exposure and Auto White Balance Mode */
    ALGORITHMS_ISS_AEWB_MODE_NONE       = 3,
    /**< None of AEWB Mode,
         Used when DCC Functionality is required, but not AEWB
         Also used when AEWB is dynamically enabled/disabled using DCC */
    ALGORITHMS_ISS_AEWB_MODE_MAX        = 4,
    /**< Max mode value, used for error checking */
    ALGORITHMS_ISS_AEWB_MODE_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgItt_IssAewbMode;

/**
 *  \brief AE Algorithm SubModes
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef enum {
    /**< Auto Exposure */
    ALGORITHMS_ISS_AE_AUTO        = 0,
    /**< Manual Exposure */
    ALGORITHMS_ISS_AE_MANUAL        = 1,
    /**< Exposure Control Disabled*/
    ALGORITHMS_ISS_AE_DISABLED        = 2
} AlgItt_IssAeMode;

/**
 *  \brief AWB Algorithm SubModes
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef enum {
    /**< Auto WhiteBalance */
    ALGORITHMS_ISS_AWB_AUTO        = 0,
    /**< Manual Exposure */
    ALGORITHMS_ISS_AWB_MANUAL      = 1,
    /**< Exposure Control Disabled*/
    ALGORITHMS_ISS_AWB_DISABLED    = 2
} AlgItt_IssAwbMode;


/**
 *  \brief For Line Interleaved WDR mode, this structure is used
 *         for specifying position of each exposure frame.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    uint32_t totalWidth, totalHeight;
    /**< Total Frame size */
    struct {
        uint32_t startx, starty;
        /**< Start position of the exposure frame */
        uint32_t width, height;
        /**< frame size of the exposure frame */
    } info[ISS_SENSOR_MAX_EXPOSURE];
} IssSensor_LineInterleavedExpFrmInfo;

/**
 *  \brief Register address and value pair, with delay.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    uint16_t nRegAddr;
    /**< Register Address */
    uint16_t nRegValue;
    /**< Slave Address */
    uint32_t nDelay;
    /**< Delay to be applied, after the register is programmed */
} I2cParams;


/**
 *  \brief AutoExposure results data structure
 * \ingroup group_vision_function_imaging_sensordrv
 */

typedef struct
{
    uint32_t chId;
    /**< Channel Id */
    uint32_t expRatio;
    /**< Exposure ratio for WDR output format, not used for linear mode */
    uint32_t exposureTime[ISS_SENSOR_MAX_EXPOSURE];
    /**< Exposure time in ms for all exposure outputs,
         For Linear mode output, only Long exposure entry is used */
    uint32_t analogGain[ISS_SENSOR_MAX_EXPOSURE];
    /**< Exposure time in ms for all exposure outputs,
         For Linear mode output, only Long exposure entry is used */
} IssSensor_ExposureParams;


/**
 *  \brief AutoWhiteBalance results data structure
 * \ingroup group_vision_function_imaging_sensordrv
 */

typedef struct
{
    uint32_t chId;
    /**< Channel Id */
    uint32_t rGain[ISS_SENSOR_MAX_EXPOSURE];
    /**< Red Gain for all exposure outputs,
         For Linear mode output, only Long exposure entry is used */
    uint32_t gGain[ISS_SENSOR_MAX_EXPOSURE];
    /**< Green Gain for all exposure outputs,
         For Linear mode output, only Long exposure entry is used */
    uint32_t bGain[ISS_SENSOR_MAX_EXPOSURE];
    /**< Blue Gain for all exposure outputs,
         For Linear mode output, only Long exposure entry is used */
    uint32_t colorTemp[ISS_SENSOR_MAX_EXPOSURE];
    /**< Color Temperature estimation for all exposure outputs,
         For Linear mode output, only Long exposure entry is used */
}IssSensor_WhiteBalanceParams;


/**
 *  \brief Structure for setting DCC params
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    uint32_t chId;
    /**< Channel Id */
    uint8_t *pDccCfg;
    /**< Pointer to DCC config */
    uint32_t dccCfgSize;
    /**< DCC Profile Size */
} IssSensor_DccParams;

/**
 *  \brief Structure for setting camera moduel information
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    /**< FPD-Link version */
    IMAGE_SERDES_FPD_LINK         fpdLink;

    /**< 8-bit physical I2C Address of the sensor */
    uint8_t                     senI2cAddr;

    /**< 8-bit physical I2C Address of the serializer */
    uint8_t                     serI2cAddr;
} IssSensor_ModuleParams;


/**
 *  \brief Sensor Information structure
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    tivx_raw_image_create_params_t        raw_params;
    /**< raw parameters of the sensor  */
    uint32_t                              features;
    /**< Bitwise list of feature supported by the sensor */
    uint32_t                              aewbMode;
    /**< AEWB mode */
    uint32_t                          fps;
    /**< frame rate */
    uint32_t numDataLanes;
    /**< Number of CSIRX data lanes */
    uint32_t dataLanesMap[4];
    /**< Data Lanes map array */
    uint32_t dataLanesPolarity[4];
    /**< Data Lanes map array */
    uint32_t csi_laneBandSpeed;      /*!< CSI2 Rx Lane Band Speed  */
    /**< CSI clock  */
} IssSensor_Info;


/**
 *  \brief Create Time parameters, ISS sensor layer uses these
 *         parameters and configures board module and sensor.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    char                                name[ISS_SENSORS_MAX_NAME];
    /**< Name of the sensor */
    uint16_t                            i2cInstId;
    /**< I2C Instance id for the sensor */
    uint8_t                             i2cAddrSensor[ISS_SENSORS_MAX_CHANNEL];
    /**< I2C alias addresses for the sensor */
    uint8_t                             i2cAddrSer[ISS_SENSORS_MAX_CHANNEL];
    /**< I2C alias addresses for the serializer */
    // uint8_t                             i2cAddrMisc[ISS_SENSORS_MAX_CHANNEL];
    /**< I2C Address of misc part on the camera board */
    IssSensor_Info                      sensorInfo;
    /**< Sensor Information e.g. width, height, format etc.*/
#if defined(B7_IMPLEMENTATION)
    IssSensor_ModuleParams              moduleInfo;
    /**< Module information, including camera I2C addresses and FPD-Link version. */
#endif
    uint32_t                            num_channels;
    /**< Number of channels supported */
    uint32_t                            dccId;
    /**< camera identifier  */
} IssSensor_CreateParams;

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* Maximum amount of sensors allowed to be connected via Fusion board
(8 on Fusion1, 12 on Fusion2) */
#define ISS_SENSORS_MAX_SENSOR_INSTANCES        (12U)
#define ISS_SENSORS_MAX_SUPPORTED_SENSOR        ISS_SENSORS_MAX_SENSOR_INSTANCES

/* Maximum amount of sensor handles allowed to be registered to the framework
   Memory can handle up to 15 handles */
#define ISS_SENSORS_MAX_SENSOR_HANDLES          (12U)


#define DEFAULT_TARGET_BRIGHTNESS_MIN                   40
#define DEFAULT_TARGET_BRIGHTNESS_MAX                   50
#define DEFAULT_TARGET_BRIGHTNESS                       45
#define DEFAULT_TARGET_THRESHOLD                        5
#define DEFAULT_EXPOSURE_TIME                           1
#define DEFAULT_ENABLE_BLC                              0

/* 8-bit GPIO Expander 5 */
#define TCA6408_I2C_ADDR_1          0x20U
#define TCA6408_I2C_ADDR_2          0x21U
#define TCA6408_CONFIG_REG          0x03U

/* 24-bit GPIO Expander for AM62A */
#define TCA6424_I2C_ADDR            0x23

/* Register values for port select register (0x4C) to read and write from
    specified port:
FPD3 RX Port 0  |   FPD3 RX Port 1  |   FPD3 RX Port 2  |   FPD3 RX Port 3
R: 0x4C[5:4]=00 |   R: 0x4C[5:4]=01 |   R: 0x4C[5:4]=10 |   R: 0x4C[5:4]=11
W: 0x4C[0]=1    |   W: 0x4C[1]=1    |   W: 0x4C[2]=1    |   W: 0x4C[3]=1  */
#define DES_PORT_SEL_0      0x01U
#define DES_PORT_SEL_1      0x12U
#define DES_PORT_SEL_2      0x24U
#define DES_PORT_SEL_3      0x38U


/*******************************************************************************
 *  Data Structures
 *******************************************************************************
 */

/**
 *  \brief CSI-2 COmplex I/O Lane Configuration
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    uint8_t pol;
    /**< TRUE configures for - / + order of differential signal.
        + / - order otherwise */
    uint8_t position;
    /**< Specify if this lane is to be used, if so, on which position.
        0x0 - Not used / disabled lane
        0x1 - Position 1
        0x2 - Position 2
        0x3 - Position 3
        0x4 - Position 4
        0x5 - Position 5 */
} IssCapture_CmplxIoLaneCfg;


/**  \brief Structure used for describing sensor interfacing.
 *
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    uint8_t                                sensorBroadcast;
    /**< Enable/Disable sensor broadcast - see \ref BROADCAST_MODE */
    uint8_t                                enableFsin;
    /**< Enable/Disable sensor Frame Sync Input (FSIN) */
    uint8_t                                numCamerasStreaming;
    /**< Number of cameras streaming simultaneously */
} IssSensorIntfParams;


/**  \brief Structure used for specifying register configurations of sensor, serializer and deserializer.
 *
 *  desCfgPreScript : Deserializer config script pre-sensor initialization
 *  serCfgPreScript : Serializer config script pre-sensor initialization
 *  sensorCfgPreScript : Sensor config script without starting streaming
 *  desCfgPostScript : Deserializer script post-sensor initialization
 *  serCfgPostScript : Serializer config script post-sensor initialization
 *  sensorCfgPostScript : Sensor config script after starting streaming
 *
 *
 * Each script is an array of type I2CParams
 * The scripts maybe of variable lengths. Scripts can also be NULL is the config is not needed.
 *
 * Last entry in serailizer and deserializer config script must be {0xFFFF, 0x00, 0x00}
 *
 *
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    I2cParams    * desCfgPreScript;
    I2cParams    * serCfgPreScript;
    I2cParams    * sensorCfgPreScript;
    I2cParams    * desCfgPostScript;
    I2cParams    * serCfgPostScript;
    I2cParams    * sensorCfgPostScript;
} IssSensorConfig;

/**
 *  \brief Min/Max Range, used in AWB calibration data
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct {
    uint32_t min;
    /**< Min Value */
    uint32_t max;
    /**< Max Value */
} IssAeRange;


/**
 *  \brief Maximum number of dynamic parameter configuration
 *  supported for Auto Exposure Algorithm tuning
 * \ingroup group_vision_function_imaging_sensordrv
 */

#define MAX_AE_DYN_PARAMS       (10U)

/**
 *  \brief Sensor Specific Auto Exposure Dynamic Parameters
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct {
    IssAeRange exposureTimeRange[MAX_AE_DYN_PARAMS];
    /**< range of exposure time in nanoseconds */
    IssAeRange analogGainRange[MAX_AE_DYN_PARAMS];
    /**< range of sensor gain, 1024 = 1x */
    IssAeRange digitalGainRange[MAX_AE_DYN_PARAMS];
    /**< range of ISP Digital gain, 256 = 1x */
    uint32_t                     numAeDynParams;
    /**< Number of Valid Entries in above arrays */
    IssAeRange targetBrightnessRange;
    /**< range of target brightness */
    uint32_t                     targetBrightness;
    /**< target brightness value */
    uint32_t                     threshold;
    /**< threshold for not using history brightness information */
    uint32_t                     exposureTimeStepSize;
    /**< step size of exposure time adjustment */
    uint32_t                     enableBlc;
    /**< TRUE enables Backlight compensation, disabled otherwise */
} IssAeDynamicParams;


/**
 *  \brief Sensor Specific Auto WhiteBalance Dynamic Parameters
 * \ingroup group_vision_function_imaging_sensordrv
 */

typedef struct {
    IssAeRange redGainRange;
    IssAeRange blueGainRange;
    IssAeRange greenGainRange;
    uint32_t   sensor_pre_gain;
    /**< Non-Zero enables applying WB gains in Sensor PreHDR Merge. In ISP otherwise */
} IssAwbDynamicParams;

/**
 *  \brief Callback for sensor probe. Detection of sensor HW using chip ID register
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_ProbeFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for sensor & SerDes configuration.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_ConfigFxn) (uint32_t chId, void *pSensorHdl, uint32_t sensor_features_requested);

/**
 *  \brief Callback for starting streaming.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_StreamOnFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for stopping streaming.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_StreamOffFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for powering ON the sensor and/or configuring the deserialser
 *  so that the sensor is visible to the SoC.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_PowerOnFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for powering OFF the sensor, if applicable.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_PowerOffFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for setting sensor exposure and gain.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_SetAeParamsFxn) (void *pSensorHdl, uint32_t chId, IssSensor_ExposureParams *pExpPrms);

/**
 *  \brief Callback for getting DCC parameters. Reserved for future use.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_GetDccParamsFxn) (uint32_t chId, void *pSensorHdl, IssSensor_DccParams *pDccPrms);

/**
 *  \brief Callback for getting current exposure settings. Reserved for future use.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_GetExpParamsFxn) (uint32_t chId, void *pSensorHdl, IssSensor_ExposureParams *pExpPrms);

/**
 *  \brief Callback for setting AEWB specific sensor settings, if applicable. Reserved for future use.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef void (*IssSensor_InitAewbConfigFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for getting ISP settings specific to the sensor, if applicable. Reserved for future use.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef void (*IssSensor_GetIspConfigFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for de-initializing the sensor, if applicable. Reserved for future use.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef void (*IssSensor_DeinitFxn) (uint32_t chId, void *pSensorHdl);

/**
 *  \brief Callback for reading/writing sensor registers from the application. Reserved for future use.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*I2cRegReadWriteFxn) (uint32_t chId, void *pSensorHdl, uint32_t readWriteFlag, I2cParams *pReg);

/**
 *  \brief Callback for getting sensor's exposure constraints. Needed for AutoExposure tuning.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_GetExpPrgFxn) (uint32_t chId, void *pSensorHdl, IssAeDynamicParams *p_ae_dynPrms);

/**
 *  \brief Callback for getting sensor's WB constraints. Needed for AutoWhiteBalance tuning.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_GetWbConfigFxn) (uint32_t chId, void *pSensorHdl, IssAwbDynamicParams *p_awb_dynPrms);


/**
 *  \brief Callback for setting WB gains in the sensor.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_SetAwbParamsFxn) (void *pSensorHdl, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms);

/**
 *  \brief Callback for getting WB gains in the sensor.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef int32_t (*IssSensor_GetAwbParamsFxn) (uint32_t chId, void *pSensorHdl, IssSensor_WhiteBalanceParams *pWbPrms);

/**
 *  \brief Structure containins sensor information, used for
 *         registering it to the sensor framework.
 * \ingroup group_vision_function_imaging_sensordrv
 */
struct IssSensorFxns_t {
    IssSensor_ProbeFxn               probe;
    IssSensor_ConfigFxn              config;
    IssSensor_StreamOnFxn            streamOn;
    IssSensor_StreamOffFxn           streamOff;
    IssSensor_PowerOnFxn             powerOn;
    IssSensor_PowerOffFxn            powerOff;
    IssSensor_GetExpParamsFxn        getAeParams;
    IssSensor_SetAeParamsFxn         setAeParams;
    IssSensor_GetDccParamsFxn        getDccParams;
    IssSensor_InitAewbConfigFxn      initAewbConfig;
    IssSensor_GetIspConfigFxn        getIspConfig;
    I2cRegReadWriteFxn                readWriteReg;
    IssSensor_GetExpPrgFxn           getExpPrg;
    IssSensor_DeinitFxn                deinit;
    IssSensor_GetWbConfigFxn         getWbCfg;
    IssSensor_SetAwbParamsFxn        setAwbParams;
    IssSensor_GetAwbParamsFxn        getAwbParams;
};
/* Forward Declaration of Sensor Params */
typedef struct IssSensorFxns_t IssSensorFxns;


// typedef int32_t (*IssSerDes_ProbeFxn)(uint32_t chId, void *pSensorHdl);

// typedef int32_t (*IssSerDes_ConfigFxn)(uint32_t chId, void *pSensorHdl);

// typedef int32_t (*IssSerDes_EnableCSIFxn)(uint32_t chId, void *pSensorHdl);

// typedef int32_t (*IssSerDes_DisableCSIFxn)(uint32_t chId, void *pSensorHdl);

// typedef int32_t (*IssSerDes_SelectTypeFxn)(uint32_t chId, void *pSensorHdl, uint32_t type);

// struct IssSerDesFxns_t {
//     IssSerDes_ProbeFxn              probe;
//     IssSerDes_ConfigFxn             config;
//     IssSerDes_EnableCSIFxn          enableCSI;
//     IssSerDes_DisableCSIFxn         disableCSI;
//     IssSerDes_SelectTypeFxn         selectType;
// };
// typedef struct IssSerDesFxns_t IssSerDesFxns;

/**
 *  \brief Handle to the sensor driver. Includes all the information about the sensor.
 * \ingroup group_vision_function_imaging_sensordrv
 */
typedef struct
{
    uint32_t                      isUsed;
    /**< Flag to indicate if given instance is free or not */
    IssSensor_CreateParams      * createPrms;
    /**< Create Parameters */
    IssSensorFxns          * sensorFxns;
    /**< Registered Sensor's parameters */
    IssSensorIntfParams      *    sensorIntfPrms;
    /**< Registered Sensor's interface parameters */
    // IssSerDesFxns           * desFxns;
    // IssSerDesFxns           * serFxns;
} IssSensors_Handle;


/*******************************************************************************
 *  Functions Declarations
 *******************************************************************************
 * \ingroup group_vision_function_imaging_sensordrv
 */


 /**
 *******************************************************************************
 *
 * \brief Function to initialize sensor driver framework.
 * Registers with remote service.
 * Includes init function of supported sensor drivers. This
 * step is critical for registring a sensor driver with the framework
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
 */
int32_t IssSensor_Init(void);

 /**
 *******************************************************************************
 *
 * \brief Function to de-initialize sensor driver framework.
 * Unregisters remote service.
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
 */

int32_t IssSensor_DeInit(void);

/**
 *******************************************************************************
 *
 * \brief Function to register a sensor driver with the driver framework
 *
 * @param pSensorHandle      All sensor properties and the APIs
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/

int32_t IssSensor_Register(IssSensors_Handle *pSensorHandle);


/**
 *******************************************************************************
 *
 * \brief Function to get Sensor information for given sensor
 *        It searches in the table of registered sensors, it returns
 *        information of first sensor, whose name matches with the
 *        given sensor.
 *
 * @param name         Name of the sensor
 * @param sensor_prms     [OUT]  Pointer to sensor create parmas structure
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_GetSensorInfo(const char *name, IssSensor_CreateParams *sensor_prms);

/**
 *******************************************************************************
 *
 * \brief Function to get sensor handle from name
 *
 * \param[in]   name    name of the sensor as specfied in IssSensor_CreateParams
 *
 * \return pointer to the sensor handle if the name was found in the registered
 *         sensor list, NULL otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
IssSensors_Handle *IssSensor_getSensorHandleFromName(const char *name);

/**
 *******************************************************************************
 *
 * \brief Creates the sensor, based on the create params, it
 *        searches for the given sensor in the registered sensors,
 *        configures the board module if sensor supports board
 *        module and creates sensor fvid2 driver.
 *        It also configures sensor in the given WDR mode and also sets
 *        the default output resolution in the sensor.
 *
 * @param pCreatePrms       Pointer to the create params
 *
 * \return handle to the created sensor
 *         NULL if there is any error
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
void* IssSensor_Create(char * name);

/**
 *******************************************************************************
 *
 * \brief Used for sending control commands, this layer
 *        supports all configuration using control commants.
 *        For the list of supported control command, see defines
 *        section of this file.
 *
 *        Must be called after Create function
 *
 * @param handle             Handle to the created sensor
 * @param cmd                Control command
 * @param cmdArgs            Pointer to the command specific arguments.
 * @param cmdRetArgs       [OUT] Pointer to the command specific return arguments
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_Control(const IssSensors_Handle* handle, uint32_t cmd, uint8_t* cmdArgs,
    const void* cmdRetArgs);

/**
 *******************************************************************************
 *
 * \brief Used Starting the sensor, which is already opened using
 *        Create function.
 *        Must be called after Create function.
 *
 * @param handle        Handle of the sensor
 * @param chId          ID of the channel to be started.
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_Start(IssSensors_Handle* handle, uint32_t chId);

/**
 *******************************************************************************
 *
 * \brief Used stopping the sensor, which is alraedy opened using
 *        Create function.
 *        Must be called after Create function.
 *
 * @param handle        Handle of the sensor
 * @param chId          ID of the channel to be stopped.
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_Stop(IssSensors_Handle* handle, uint32_t chId);

/**
 *******************************************************************************
 *
 * \brief Used for deleting the sensor, which is already opened using
 *        Create function. Internally it deletes sensor's fvid2 driver.
 *        After this call, handle is not valid.
 *
 *        Must be called after Create function.
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_Delete(IssSensors_Handle* handle);

/**
 *******************************************************************************
 *
 * \brief Function to configure UB960 deserializer
 *
 * @param script                 Array of type I2CParams with last entry being {0xFFFF, 0x00, 0x00}
 * @param desHubInstId        ID 0/1 indicating which UB960 on the board is to be configured
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_cfgDesScript(const I2cParams *script, int8_t desHubInstId);

/**
 *******************************************************************************
 *
 * \brief Function to configure UB953 serializer
 *
 * @param i2cInstId              I2C bus on which the serializer is populated
 * @param i2cAddr                7-bit I2C address, usually alias address as configured in UB960
 * @param script                 Array of type I2CParams with last entry being {0xFFFF, 0x00, 0x00}
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t ub953_cfgScript(uint8_t i2cInstId, uint8_t i2cAddr, const I2cParams *script);

/**
 *******************************************************************************
 *
 * \brief Function to get sensor handle from name
 *
 * @param name              name of the sensor as specfied in IssSensor_CreateParams
 * \return pointer to the sensor handle if the name was found in the registered sensor list
 *         NULL otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
// IssSensors_Handle * getSensorHandleFromName(char *name);

/**
 *******************************************************************************
 *
 * \brief Function to power ON the sensor
 *
 * @param handle              Pointer to sensor handle
 * @param chMask              Binary mask indicating which cameras are to be powered ON
 *                              For e.g. 0x1F means that first 5 cameras must be enabled
 *                              This would mean configuring both the UB960 deserializers
 *                              Another example - chMask = 0x1 means only the first camera should be enabled
 *                              Therefore, only first instance of UB960 should be configured.
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_PowerOn(IssSensors_Handle* handle, uint32_t chMask);


/**
 *******************************************************************************
 *
 * \brief Function to power OFF the sensor
 *
 * @param handle            Pointer to sensor handle
 * @param chId              Channel ID indicating which camera needs to be disabled
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_PowerOff(IssSensors_Handle* handle, uint32_t chMask);

/**
 *******************************************************************************
 *
 * \brief Function to configure the sensor
 *
 * @param handle            Pointer to sensor handle
 * @param chId              Channel ID of the camera to be configured
 * @param feat              Feature mask. Sensor drver can apply different settings for different features
 *                          For e.g. linear mode vs WDR mode
 *                          OR 30fps mode vs 60fps mode
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_Config(IssSensors_Handle* handle, uint32_t chId, uint32_t feat);

/**
 *******************************************************************************
 *
 * \brief Function to send Exposure parameters to the sensor
 *
 * @param handle            Pointer to sensor handle
 * @param chId              Channel ID of the camera to be configured
 * @param pExpPrms          Exposure parameters - integration time and sensor gain
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_SetAeParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_ExposureParams *pExpPrms);

/**
 *******************************************************************************
 *
 * \brief Function to send WhiteBalance gains to the sensor
 *
 * @param handle            Pointer to sensor handle
 * @param chId              Channel ID of the camera to be configured
 * @param pWbPrms          WhiteBalance parameters - RGB colors gains
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_SetAwbParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms);

int32_t IssSensor_GetAeParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_ExposureParams *pExpPrms);
int32_t IssSensor_GetAwbParams(IssSensors_Handle *handle, uint32_t chId, IssSensor_WhiteBalanceParams *pWbPrms);
void IssSensor_CloseHandle(void);

int32_t enableUB960Broadcast(int8_t desHubInstId);
int32_t disableUB960Broadcast(int8_t desHubInstId);
int32_t UB960_SelectPort(uint32_t chId);
int32_t UB960_SetAlias(uint32_t chId, uint8_t slot,  uint8_t phy_i2c_addr_7bit, uint8_t alias_i2c_addr_7bit);
int32_t UB960_SetSerAlias(uint32_t chId, uint8_t ser_alias_i2c_addr_7bit);
int32_t UB960_SetSensorAlias(uint32_t chId, uint8_t sensor_phy_i2c_addr_7bit, uint8_t ser_alias_i2c_addr_7bit);


/**
 *******************************************************************************
 *
 * \brief Function to enable streaming from UB960 by setting register 0x33 to 0x3
 *  for the given camera channel
 *
 * @param chId              Channel ID of the camera to be configured
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t enableUB960Streaming(uint32_t chId);

/**
 *******************************************************************************
 *
 * \brief Function to enable streaming from UB960 by setting register 0x33 to 0x2
 *  for the given camera channel
 *
 * @param chId              Channel ID of the camera to be configured
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t disableUB960Streaming(uint32_t chId);


/**
 *******************************************************************************
 *
 * \brief Function to map UB960 Instance to channel number
 * On Fusion board the mapping is as follows
 *  Camera Channel  0 - UB960 Instance 0
 *  Camera Channel  1 - UB960 Instance 0
 *  Camera Channel  2 - UB960 Instance 0
 *  Camera Channel  3 - UB960 Instance 0
 *  Camera Channel  4 - UB960 Instance 1
 *  Camera Channel  5 - UB960 Instance 1
 *  Camera Channel  6 - UB960 Instance 1
 *  Camera Channel  7 - UB960 Instance 1
 *
 *
 *  On customer boards, this mapping maybe redefined
 *
 * @param chId              Channel ID of the camera to be configured
 * \return 0 if success
 *         -1 otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int8_t getDeserializerInstIdFromChId(uint32_t chId);

int32_t powerOffDeserializers(void);


#ifndef DEPRECATED
/**
 *******************************************************************************
 *
 * \brief Function to get Sensor information for sensor whose DCC ID is passed.
 *        It searches in the table of registered sensors, it returns
 *        information of first sensor, whose dcc id matches with the given id.
 *        Typically used by the DCC Network handler to get the
 *        sensor information.
 *
 * @param dccId        DCC Id of the sensor
 * @param pInfo     [OUT]  Pointer to sensor information structure
 *
 * \return 0 in case of success
 *         error otherwise
 *
 * \ingroup group_vision_function_imaging_sensordrv
 *******************************************************************************
*/
int32_t IssSensor_GetSensorInfoFromDccId(uint32_t dccId, IssSensor_Info *pInfo);

#endif /* DEPRECATED */

#endif /* End of ISS_SENSORS_H_*/

