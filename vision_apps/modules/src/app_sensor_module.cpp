/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
 *
 * All rights not granted herein.
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
 *       permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 *       licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 *       works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 *       and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
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
 */

#include "app_sensor_module.hpp"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern "C" {
#include <utils/iss/include/app_iss.h>
}

SensorObj::SensorObj() { setDefaults(); }

void SensorObj::setDefaults()
{
    /* Initialize sensor parameters structure */
    memset(&this->sensorParams, 0, sizeof(IssSensor_CreateParams));

    /* Initialize available sensor names array */
    memset(this->availableSensorNames, 0, sizeof(this->availableSensorNames));

    /* Initialize sensor name */
    memset(this->sensor_name, 0, sizeof(this->sensor_name));

    /* Initialize counters and flags */
    this->num_sensors_found = 0;
    this->sensor_features_enabled = 0;
    this->sensor_features_supported = 0;
    this->sensor_dcc_enabled = 0;
    this->sensor_wdr_enabled = 0;
    this->sensor_exp_control_enabled = 0;
    this->sensor_gain_control_enabled = 0;
    this->num_cameras_enabled = 0;
    this->ch_mask = 0;
    this->is_interactive = 0;
    this->enable_ldc = 0;
    this->sensor_index = 0;
    this->usecase_option = 0;
    this->image_width = 0;
    this->image_height = 0;
    this->sensor_out_format = 0;
}

vx_status SensorObj::initModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    int32_t sensor_init_status = -1;
    int32_t ch_mask = this->ch_mask;

    /* Initialize the image sensor */
    sensor_init_status = appInitImageSensor(this->sensor_name, this->sensor_features_enabled, ch_mask);
    if(0 != sensor_init_status)
    {
        printf("Error initializing sensor %s \n", this->sensor_name);
        status = VX_FAILURE;
    }

    return status;
}

vx_status SensorObj::deinitModule(AppGraphObj *graph, vx_int32 module_index)
{
    /* Deinitialize the image sensor */
    appDeInitImageSensor(this->sensor_name);

    return VX_SUCCESS;
}

vx_status SensorObj::createNode(AppGraphObj *graph, vx_int32 module_index)
{
    /* Sensor module doesn't create OpenVX nodes - it's a configuration module */
    /* This method is kept for API consistency with other modules */
    return VX_SUCCESS;
}

vx_status SensorObj::deleteNode(AppGraphObj *graph, vx_int32 module_index)
{
    /* Sensor module doesn't have OpenVX nodes to delete */
    /* This method is kept for API consistency with other modules */
    return VX_SUCCESS;
}

/*==========================================================================*/
/* SENSOR MODULE HELPER FUNCTION                                            */
/*==========================================================================*/

vx_status SensorObj::querrySensor()
{
    vx_status status = VX_SUCCESS;
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint16 selectedSensor = 0xFFF;
    vx_uint8 sensors_detected[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_bool sensorSelected = vx_false_e;
    vx_bool ldcSelected = vx_false_e;
    int32_t i;

    memset(this->availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
    for(i = 0; i < ISS_SENSORS_MAX_SUPPORTED_SENSOR; i++)
    {
        sensor_list[i] = this->availableSensorNames[i];
    }

    memset(&this->sensorParams, 0, sizeof(IssSensor_CreateParams));
    status = appEnumerateImageSensor(sensor_list, &this->num_sensors_found);
    if(VX_SUCCESS != status)
    {
        printf("appCreateImageSensor returned %d\n", status);
        return status;
    }

    if(this->is_interactive == 1)
    {
        vx_char ch = 0;
        int ch_id;

        while(sensorSelected != vx_true_e)
        {
            printf("%d sensor(s) found \n", this->num_sensors_found);
            printf("Supported sensor list: \n");
            for(i = 0; i < this->num_sensors_found; i++)
            {
                printf("%c : %s \n", i+'a', sensor_list[i]);
            }

            printf("Select a sensor above or press '0' to autodetect the sensor \n");
            ch = getchar();
            if(ch == '0')
            {
                uint8_t num_sensors_found= 0;
                uint16_t channel_mask = 0xFFF;
                /*AutoDetect*/
                memset(sensors_detected, 0xFFF, ISS_SENSORS_MAX_SUPPORTED_SENSOR);
                status = appDetectImageSensor(sensors_detected, &num_sensors_found, channel_mask);
                if(0 == status)
                {
                    int detected_sensor_index;
                    char * detected_sensor_name;
                    for(ch_id=0;ch_id<ISS_SENSORS_MAX_SUPPORTED_SENSOR;ch_id++)
                    {
                        detected_sensor_index = sensors_detected[ch_id];
                        if(detected_sensor_index < ISS_SENSORS_MAX_SUPPORTED_SENSOR)
                        {
                            detected_sensor_name = sensor_list[detected_sensor_index];
                            printf("sensor detected at channel %d = %s \n", ch_id, detected_sensor_name);
                        }
                        else
                        {
                            printf("sensor detected at channel %d = None \n", ch_id);
                        }
                    }
                }
                else
                {
                    printf("appDetectImageSensor failed with error = 0x%x \n", status);
                }
            }
            else
            {
                selectedSensor = ch - 'a';
                /*Assume all cameras are identical*/
                for(ch_id=0;ch_id<ISS_SENSORS_MAX_SUPPORTED_SENSOR;ch_id++)
                {
                    sensors_detected[ch_id] = selectedSensor;
                }
            }

            //TODO : sensor properties need to be queried.
            //Currently this is supported for cam0 only
            selectedSensor = sensors_detected[0];
            if(selectedSensor > (this->num_sensors_found-1))
            {
                printf("Invalid selection %d. Try again \n", selectedSensor);
            }
            else
            {
                snprintf(this->sensor_name, ISS_SENSORS_MAX_NAME, "%s", sensor_list[selectedSensor]);

                printf("Sensor selected : %s\n", this->sensor_name);

                printf("Querying %s \n", this->sensor_name);
                status = appQueryImageSensor(this->sensor_name, &this->sensorParams);
                if(VX_SUCCESS != status)
                {
                    printf("appQueryImageSensor returned %d\n", status);
                    return status;
                }

                if(this->sensorParams.sensorInfo.raw_params.format[0].pixel_container == VX_DF_IMAGE_UYVY)
                {
                    this->sensor_out_format = 1;
                }
                else
                {
                    this->sensor_out_format = 0;
                }

                sensorSelected = vx_true_e;
            }
        }

        while (ldcSelected != vx_true_e)
        {
            fflush (stdin);
            printf ("LDC Selection Yes(1)/No(0)\n");
            ch = getchar();
            this->enable_ldc = ch - '0';

            if((this->enable_ldc > 1) || (this->enable_ldc < 0))
            {
                printf("Invalid selection %c. Try again \n", ch);
            }
            else
            {
                ldcSelected = vx_true_e;
            }
        }

        this->num_cameras_enabled = 0;
        while(this->num_cameras_enabled == 0)
        {
            int c;
            int ret;
            int num_cameras;
            fflush(stdin);
            printf("Max number of cameras supported by sensor %s = %d \n", this->sensor_name, this->sensorParams.num_channels);
            printf("Please enter number of cameras to be enabled \n");
            ret = scanf("%d", &num_cameras);
            while ((c = getchar()) != '\n' && c != EOF);
            this->num_cameras_enabled = num_cameras;
            if( (1==ret) && ((this->num_cameras_enabled > this->sensorParams.num_channels) || (this->num_cameras_enabled <= 0)) )
            {
                this->num_cameras_enabled = 0;
                printf("Invalid selection %d. Try again \n", num_cameras);
            }
        }
        this->ch_mask = (1<<this->num_cameras_enabled) - 1;
    }
    else
    {
        selectedSensor = this->sensor_index;
        if(selectedSensor > (this->num_sensors_found-1))
        {
            printf("Invalid selection found in config file: %d. Exiting. \n", selectedSensor);
            return -1;
        }
        else
        {
            snprintf(this->sensor_name, ISS_SENSORS_MAX_NAME, "%s", sensor_list[selectedSensor]);
            printf("Sensor selected : %s\n", this->sensor_name);

            printf("Querying %s \n", this->sensor_name);
            status = appQueryImageSensor(this->sensor_name, &this->sensorParams);
            if(VX_SUCCESS != status)
            {
                printf("appQueryImageSensor returned %d\n", status);
                return status;
            }

            if(this->sensorParams.sensorInfo.raw_params.format[0].pixel_container == VX_DF_IMAGE_UYVY)
            {
                this->sensor_out_format = 1;
            }
            else
            {
                this->sensor_out_format = 0;
            }

            sensorSelected = vx_true_e;
        }

        if(this->ch_mask > 0)
        {
            vx_uint32 mask = this->ch_mask;
            this->num_cameras_enabled = 0;
            while(mask > 0)
            {
                if(mask & 0x1)
                {
                    this->num_cameras_enabled++;
                }
                mask = mask >> 1;
            }
        }
    }

    /*
    Check for supported sensor features.
    It is upto the application to decide which features should be enabled.
    This demo app enables WDR, DCC and 2A if the sensor supports it.
    */

    this->sensor_features_supported = this->sensorParams.sensorInfo.features;

    if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (this->sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
    {
        APP_PRINTF("WDR mode is supported \n");
        this->sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
        this->sensor_wdr_enabled = 1;
    }else
    {
        APP_PRINTF("WDR mode is not supported. Defaulting to linear \n");
        this->sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
        this->sensor_wdr_enabled = 0;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (this->sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
    {
        APP_PRINTF("Expsoure control is supported \n");
        this->sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
        this->sensor_exp_control_enabled = 1;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (this->sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
    {
        APP_PRINTF("Gain control is supported \n");
        this->sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
        this->sensor_gain_control_enabled = 1;
    }

    if(ISS_SENSOR_FEATURE_CFG_UC1 == (this->sensor_features_supported & ISS_SENSOR_FEATURE_CFG_UC1))
    {
        if(this->usecase_option == APP_SENSOR_FEATURE_CFG_UC1)
        {
            APP_PRINTF("CMS Usecase is supported \n");
            this->sensor_features_enabled |= ISS_SENSOR_FEATURE_CFG_UC1;
        }
    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (this->sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        this->sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
        this->sensor_dcc_enabled = 1;
        APP_PRINTF("Sensor DCC is enabled \n");
    }else
    {
        this->sensor_dcc_enabled = 0;
        APP_PRINTF("Sensor DCC is disabled \n");
    }

    this->image_width   = this->sensorParams.sensorInfo.raw_params.width;
    this->image_height  = this->sensorParams.sensorInfo.raw_params.height;

    APP_PRINTF("Sensor width = %d\n", this->sensorParams.sensorInfo.raw_params.width);
    APP_PRINTF("Sensor height = %d\n", this->sensorParams.sensorInfo.raw_params.height);
    APP_PRINTF("Sensor DCC ID = %d\n", this->sensorParams.dccId);
    APP_PRINTF("Sensor Supported Features = 0x%08X\n", this->sensor_features_supported);
    APP_PRINTF("Sensor Enabled Features = 0x%08X\n", this->sensor_features_enabled);

    return (status);
}
vx_status SensorObj::startSensor()
{
    int32_t status = appStartImageSensor(this->sensor_name, this->ch_mask);
    if (status != 0)
    {
        printf("[SENSOR] ERROR: appStartImageSensor failed (%d)\n", status);
        return VX_FAILURE;
    }
    return VX_SUCCESS;
}

vx_status SensorObj::stopSensor()
{
    int32_t status = appStopImageSensor(this->sensor_name, this->ch_mask);
    if (status != 0)
    {
        printf("[SENSOR] ERROR: appStopImageSensor failed (%d)\n", status);
        return VX_FAILURE;
    }
    return VX_SUCCESS;
}
