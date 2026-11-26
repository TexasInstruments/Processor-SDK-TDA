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

#include "app_aewb_module.hpp"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "app_sensor_module.hpp"
#include "app_viss_module.hpp"

AEWBObj::AEWBObj() { setDefaults(); }

void AEWBObj::setDefaults()
{
    /* Initialize AEWB parameters */
    memset(&this->params, 0, sizeof(tivx_aewb_config_t));

    /* Initialize arrays and objects */
    this->node = NULL;
    this->config_arr = NULL;
    this->dcc_config = NULL;
    this->histogram_arr = NULL;
    this->aewb_output_arr = NULL;
    memset(this->objName, 0, sizeof(this->objName));

    /* Initialize queued mode arrays */
    for (int i = 0; i < (int)APP_MODULES_MAX_BUFQ_DEPTH; i++) {
        this->aewb_output_arr_q[i] = NULL;
        this->aewb_output[i] = NULL;
        this->h3a_stats[i] = NULL;
    }

    this->graph_parameter_in_index = 0;
    this->graph_parameter_out_index = 0;
    this->bufq_depth = 0;
    this->starting_channel = 0;
    this->num_cameras_enabled = 0;
}

vx_status AEWBObj::initModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_context context = graph->context;

    /* Get the sensor object from the graph */
    AppModuleObj* sensor_module_obj = graph->getModuleByType(AppModuleType::Sensor);
    if (!sensor_module_obj || !sensor_module_obj->module_obj) {
        printf("[AEWB_MODULE] Sensor object not found\n");
        return VX_FAILURE;
    }

    SensorObj *sensorObj = static_cast<SensorObj*>(sensor_module_obj->module_obj);

    return this->initAewb(context, sensorObj);
}

vx_status AEWBObj::deinitModule(AppGraphObj *graph, vx_int32 module_index)
{
    this->deinitAewb();
    return VX_SUCCESS;
}

vx_status AEWBObj::createNode(AppGraphObj *graph, vx_int32 module_index)
{
    /* Get the VISS object from the graph */
    AppModuleObj* viss_module_obj = graph->getModuleByType(AppModuleType::Viss);
    if (!viss_module_obj || !viss_module_obj->module_obj) {
        printf("[AEWB_MODULE] VISS object not found\n");
        return VX_FAILURE;
    }

    VISSObj *vissObj = static_cast<VISSObj*>(viss_module_obj->module_obj);

    return this->createGraphAewb(graph->graph, vissObj->h3a_stats_arr);
}

vx_status AEWBObj::deleteNode(AppGraphObj *graph, vx_int32 module_index)
{
    if (this->node != NULL) {
        vxReleaseNode(&this->node);
        this->node = NULL;
    }
    return VX_SUCCESS;
}

/*==========================================================================*/
/* AEWB MODULE PRIVATE HELPER FUNCTIONS                                     */
/*==========================================================================*/

vx_status AEWBObj::configureDcc(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    if(sensorObj->sensor_dcc_enabled)
    {
        int32_t dcc_buff_size;
        uint8_t * dcc_buf;
        vx_map_id dcc_buf_map_id;

        dcc_buff_size = appIssGetDCCSize2A(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled);
        if(dcc_buff_size < 0)
        {
            printf("[AEWB-MODULE] Invalid DCC size for 2A! \n");
            return VX_FAILURE;
        }
        this->dcc_config = vxCreateUserDataObject(context, "dcc_2a", dcc_buff_size, NULL);
        status = vxGetStatus((vx_reference)this->dcc_config);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->dcc_config, "aewb_node_dcc_config");

            vxMapUserDataObject(
                this->dcc_config,
                0,
                dcc_buff_size,
                &dcc_buf_map_id,
                (void **)&dcc_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );
            status = appIssGetDCCBuff2A(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled,  dcc_buf, dcc_buff_size);
            if(status != VX_SUCCESS)
            {
                printf("[AEWB-MODULE] Error getting 2A DCC buffer! \n");
                return VX_FAILURE;
            }
            vxUnmapUserDataObject(this->dcc_config, dcc_buf_map_id);
        }
        else
        {
            printf("[AEWB-MODULE] Unable to create DCC config object!\n");
        }
    }
    else
    {
        this->dcc_config = NULL;
        printf("[AEWB-MODULE] Sensor DCC is disabled \n");
    }

    return status;
}

vx_status AEWBObj::configureAewb(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 ch;
    vx_int32 ch_en;
    vx_int32 ch_mask;
    uint32_t starting_channel   = this->starting_channel;
    uint32_t num_cameras_enabled = this->num_cameras_enabled;

    this->params.sensor_dcc_id       = sensorObj->sensorParams.dccId;
    this->params.sensor_img_format   = 0;
    this->params.sensor_img_phase    = 3;

    if(sensorObj->sensor_exp_control_enabled || sensorObj->sensor_gain_control_enabled )
    {
        this->params.ae_mode = ALGORITHMS_ISS_AE_AUTO;
    }
    else
    {
        this->params.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
    }
    this->params.awb_mode = ALGORITHMS_ISS_AWB_AUTO;

    this->params.awb_num_skip_frames = 9;
    this->params.ae_num_skip_frames  = 9;
    this->params.channel_id          = starting_channel;

    vx_user_data_object config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &this->params);
    status = vxGetStatus((vx_reference)config);
    if(status == VX_SUCCESS)
    {
        this->config_arr = vxCreateObjectArray(context, (vx_reference)config, num_cameras_enabled);
        status = vxGetStatus((vx_reference)this->config_arr);
        if(status != VX_SUCCESS)
        {
            printf("[AEWB_MODULE] Unable to create AEWB config object array! \n");
        }
        vxReleaseUserDataObject(&config);

        if(status == VX_SUCCESS)
        {
            vx_uint32 array_obj_index = 0;
            vxSetReferenceName((vx_reference)this->config_arr, "aewb_node_config_arr");

            ch = starting_channel;
            ch_mask = sensorObj->ch_mask >> starting_channel;
            ch_en = 0;
            while(ch_mask > 0)
            {
                if(ch_mask & 0x1)
                {
                    vx_user_data_object config = (vx_user_data_object)vxGetObjectArrayItem(this->config_arr, array_obj_index);
                    array_obj_index++;
                    this->params.channel_id = ch;
                    vxCopyUserDataObject(config, 0, sizeof(tivx_aewb_config_t), &this->params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
                    vxReleaseUserDataObject(&config);
                    ch_en++;
                }
                ch++;
                ch_mask = ch_mask >> 1;
                if (ch_en == (vx_int32)num_cameras_enabled)
                {
                    break;
                }
            }

            if(ch_en < (vx_int32)num_cameras_enabled)
            {
                printf("[AEWB_MODULE] Warning: %d camera(s) were enabled, %d expected\n", ch_en, num_cameras_enabled);
            }
        }
    }
    else
    {
        printf("[AEWB_MODULE] Unable to create AEWB config object! \n");
    }

    return status;
}

vx_status AEWBObj::createHistogram(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_distribution histogram = vxCreateDistribution(context, 256, 0, 256);
    status = vxGetStatus((vx_reference)histogram);
    if(status == VX_SUCCESS)
    {
        this->histogram_arr = vxCreateObjectArray(context, (vx_reference)histogram, this->num_cameras_enabled);
        status = vxGetStatus((vx_reference)this->histogram_arr);
        if(status != VX_SUCCESS)
        {
            printf("[AEWB_MODULE] Unable to create Histogram object array! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)this->histogram_arr, "aewb_node_histogram_arr");
        }
        vxReleaseDistribution(&histogram);
    }
    else
    {
        printf("[AEWB_MODULE] Unable to create Histogram object! \n");
    }

    return status;
}

vx_status AEWBObj::createAewbOutput(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object aewb_output =  vxCreateUserDataObject(context, "tivx_ae_awb_params_t", sizeof(tivx_ae_awb_params_t), NULL);
    status = vxGetStatus((vx_reference)aewb_output);
    if(status == VX_SUCCESS)
    {
        this->aewb_output_arr = vxCreateObjectArray(context, (vx_reference)aewb_output, this->num_cameras_enabled);
        status = vxGetStatus((vx_reference)this->aewb_output_arr);
        if(status != VX_SUCCESS)
        {
            printf("[AEWB_MODULE] Unable to create AEWB output object array! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)this->aewb_output_arr, "aewb_node_aewb_output_arr");
        }
        vxReleaseUserDataObject(&aewb_output);
    }
    else
    {
        printf("[AEWB_MODULE] Unable to create AEWB output object! \n");
    }

    return status;
}

vx_status AEWBObj::createAewbOutputQ(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 q = 0;

    vx_user_data_object aewb_output = NULL;

    (void)sensorObj;

    aewb_output = vxCreateUserDataObject(context,
                                        "tivx_ae_awb_params_t",
                                        sizeof(tivx_ae_awb_params_t),
                                        NULL);

    status = vxGetStatus((vx_reference)aewb_output);

    if (status != VX_SUCCESS)
    {
        printf("[AEWB_MODULE] Unable to create AEWB output object!\n");
    }

    if (status == VX_SUCCESS)
    {
        static char name[APP_MODULES_MAX_OBJ_NAME_SIZE];

        for (q = 0; q < (vx_int32)this->bufq_depth; q++)
        {
            this->aewb_output_arr_q[q] =
                vxCreateObjectArray(context,
                                    (vx_reference)aewb_output,
                                    this->num_cameras_enabled);

            status = vxGetStatus((vx_reference)this->aewb_output_arr_q[q]);

            if (status == VX_SUCCESS)
            {
                snprintf(name, sizeof(name), "aewb_output_arr_q%d", q);
                vxSetReferenceName((vx_reference)this->aewb_output_arr_q[q], name);

                this->aewb_output[q] =
                    (vx_user_data_object)vxGetObjectArrayItem(
                        (vx_object_array)this->aewb_output_arr_q[q],
                        0);

            status = vxGetStatus((vx_reference)this->aewb_output[q]);
            }

            if (status == VX_SUCCESS)
            {
                snprintf(name, sizeof(name), "aewb_output_q%d", q);
                vxSetReferenceName((vx_reference)this->aewb_output[q], name);
            }

            if (status != VX_SUCCESS)
            {
                printf("[AEWB_MODULE] Unable to create AEWB output object array at depth %d\n", q);
                break;
            }
        }
    }

    if (aewb_output != NULL)
    {
        (void)vxReleaseUserDataObject(&aewb_output);
    }

    return status;
}

vx_status AEWBObj::initQueued(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    if (sensorObj == NULL)
    {
        return VX_FAILURE;
    }

    if (this->bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        printf("[AEWB_MODULE] Warning: bufq_depth %u exceeds maximum %u, limiting\n",
               (uint32_t)this->bufq_depth, (uint32_t)APP_MODULES_MAX_BUFQ_DEPTH);
        this->bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    if (status == VX_SUCCESS)
    {
        status = this->configureDcc(context, sensorObj);
    }

    if (status == VX_SUCCESS)
    {
        status = this->configureAewb(context, sensorObj);
    }

    if (status == VX_SUCCESS)
    {
        status = this->createHistogram(context, sensorObj);
    }

    if (status == VX_SUCCESS)
    {
        status = this->createAewbOutputQ(context, sensorObj);
    }

    return status;
}

void AEWBObj::deinitQueued()
{
    vx_int32 q;
    vx_int32 bufq_depth;

    bufq_depth = this->bufq_depth;

    if (bufq_depth > (vx_int32)APP_MODULES_MAX_BUFQ_DEPTH)
    {
        bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    if (this->histogram_arr != NULL)
    {
        (void)vxReleaseObjectArray(&this->histogram_arr);
    }

    if (this->config_arr != NULL)
    {
        (void)vxReleaseObjectArray(&this->config_arr);
    }

    for (q = 0; q < bufq_depth; q++)
    {
        if (this->aewb_output_arr_q[q] != NULL)
        {
            (void)vxReleaseObjectArray(&this->aewb_output_arr_q[q]);
        }

        if (this->aewb_output[q] != NULL)
        {
            (void)vxReleaseUserDataObject(&this->aewb_output[q]);
        }

        if (this->h3a_stats[q] != NULL)
        {
            (void)vxReleaseUserDataObject(&this->h3a_stats[q]);
        }
    }

    if (this->dcc_config != NULL)
    {
        (void)vxReleaseUserDataObject(&this->dcc_config);
    }
}

vx_status AEWBObj::createGraphQueued(vx_graph graph,
                                      vx_object_array h3a_stats_arr[],
                                      const char *target_string)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object aewb_output = NULL;
    vx_user_data_object config = NULL;
    vx_distribution histogram = NULL;

    static char name[APP_MODULES_MAX_OBJ_NAME_SIZE];
    vx_int32 q = 0;
    vx_int32 bufq_depth = this->bufq_depth;

    if (bufq_depth > (vx_int32)APP_MODULES_MAX_BUFQ_DEPTH)
    {
        printf("[AEWB_MODULE] Warning: bufq_depth %d exceeds maximum %d, limiting\n",
               bufq_depth, APP_MODULES_MAX_BUFQ_DEPTH);
        bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    aewb_output =
        (vx_user_data_object)vxGetObjectArrayItem(this->aewb_output_arr_q[0], 0);

    status = vxGetStatus((vx_reference)aewb_output);

    if (status == VX_SUCCESS)
    {
        for (q = 0; q < bufq_depth; q++)
        {
            this->h3a_stats[q] =
                (vx_user_data_object)vxGetObjectArrayItem(h3a_stats_arr[q], 0);

            status = vxGetStatus((vx_reference)this->h3a_stats[q]);

            if (status == VX_SUCCESS)
            {
                snprintf(name, sizeof(name), "aewb_h3a_stats_q%d", q);
                vxSetReferenceName((vx_reference)this->h3a_stats[q], name);
            }

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        config = (vx_user_data_object)vxGetObjectArrayItem(this->config_arr, 0);
        status = vxGetStatus((vx_reference)config);
    }

    if (status == VX_SUCCESS)
    {
        histogram = (vx_distribution)vxGetObjectArrayItem(this->histogram_arr, 0);
        status = vxGetStatus((vx_reference)histogram);
    }

    if (status == VX_SUCCESS)
    {
        this->node = tivxAewbNode(graph,
                                     config,
                                     histogram,
                                     this->h3a_stats[0],
                                     NULL,
                                     aewb_output,
                                     this->dcc_config);

        status = vxGetStatus((vx_reference)this->node);

        if (status != VX_SUCCESS)
        {
            printf("[AEWB_MODULE] Unable to create AEWB node!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->node, "aewb_node");

        status = vxSetNodeTarget(this->node,
                                 VX_TARGET_STRING,
                                 target_string);
    }

    if (status == VX_SUCCESS)
    {
        vx_bool replicate[] =
        {
            vx_true_e,
            vx_true_e,
            vx_true_e,
            vx_false_e,
            vx_true_e,
            vx_false_e
        };

        status = vxReplicateNode(graph, this->node, replicate, 6);
    }

    if (aewb_output != NULL)
    {
        vxReleaseUserDataObject(&aewb_output);
    }

    if (config != NULL)
    {
        vxReleaseUserDataObject(&config);
    }

    if (histogram != NULL)
    {
        vxReleaseDistribution(&histogram);
    }

    return status;
}

vx_status AEWBObj::initAewb(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    status = this->configureDcc(context, sensorObj);

    if(status == VX_SUCCESS)
    {
        status = this->configureAewb(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = this->createHistogram(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = this->createAewbOutput(context, sensorObj);
    }

    return (status);
}

void AEWBObj::deinitAewb()
{
    vxReleaseObjectArray(&this->histogram_arr);
    vxReleaseObjectArray(&this->config_arr);
    vxReleaseObjectArray(&this->aewb_output_arr);

    if(this->dcc_config != NULL)
    {
        vxReleaseUserDataObject(&this->dcc_config);
    }
}

void AEWBObj::deleteAewb()
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
}

vx_status AEWBObj::createGraphAewb(vx_graph graph, vx_object_array h3a_stats_arr)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object aewb_output = (vx_user_data_object)vxGetObjectArrayItem(this->aewb_output_arr, 0);
    vx_user_data_object h3a_stats   = (vx_user_data_object)vxGetObjectArrayItem(h3a_stats_arr, 0);

    vx_user_data_object config = (vx_user_data_object)vxGetObjectArrayItem(this->config_arr, 0);
    vx_distribution histogram  = (vx_distribution)vxGetObjectArrayItem(this->histogram_arr, 0);

    this->node = tivxAewbNode(graph,
                                 config,
                                 histogram,
                                 h3a_stats,
                                 NULL,
                                 aewb_output,
                                 this->dcc_config);

    vxReleaseUserDataObject(&aewb_output);
    vxReleaseUserDataObject(&h3a_stats);
    vxReleaseUserDataObject(&config);
    vxReleaseDistribution(&histogram);

    status = vxGetStatus((vx_reference)this->node);
    if(status != VX_SUCCESS)
    {
        printf("[AEWB_MODULE] Unable to create AEWB node! \n");
        return status;
    }

    vxSetReferenceName((vx_reference)this->node, "aewb_node");

#if defined(SOC_TDA54)
    vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_MCU2);
#else
    vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_MCU2_0);
#endif

    vx_bool replicate[] = { vx_true_e, vx_true_e, vx_true_e, vx_false_e, vx_true_e, vx_false_e};
    vxReplicateNode(graph, this->node, replicate, 6);


    return status;
}