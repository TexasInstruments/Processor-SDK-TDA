/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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
#include "app_scaler_module.hpp"
#include <string.h>

ScalerObj::ScalerObj() { setDefaults(); }

void ScalerObj::setDefaults()
{
    for (int i = 0; i < APP_MODULES_MAX_SCALER_BUFFERS; i++)
    {
        this->input_arr[i]    = NULL;
        this->input_images[i] = NULL;
    }
    this->input_width         = 0;
    this->input_height        = 0;
    this->bufq_depth          = 0;
    this->node                = NULL;
    /* output[] elements are zero-initialised by ImgObj's own constructor */
    for (int i = 0; i < APP_MODULES_MAX_SCALER_OUTPUTS; i++)
    {
        this->output[i] = ImgObj();
        this->file_prefix[i] = NULL;
        this->write_node[i]  = NULL;
        this->write_cmd[i]   = NULL;
    }
    this->coeff_obj           = NULL;
    this->graph_parameter_index = 0;
    this->en_out_scaler_write = 0;
    this->file_path           = NULL;
    memset(this->output_file_path, 0, sizeof(this->output_file_path));
    memset(this->objName, 0, sizeof(this->objName));
    this->num_ch              = 1;
    this->num_outputs         = 2;   /* output[0]=DL input, output[1]=viz */
    this->color_format        = VX_DF_IMAGE_NV12;
    this->external_input      = vx_false_e;  /* Default: create/manage input buffers */
}


vx_status ScalerObj::configureScalerCoeffs(vx_context context)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_msc_coefficients_t coeffs;

    scale_set_coeff(&coeffs, VX_INTERPOLATION_BILINEAR);

    /* Set Coefficients */
    this->coeff_obj = vxCreateUserDataObject(context,
                                "tivx_vpac_msc_coefficients_t",
                                sizeof(tivx_vpac_msc_coefficients_t),
                                NULL);
    status = vxGetStatus((vx_reference)this->coeff_obj);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->coeff_obj, "scaler_node_coeff_obj");

        status = vxCopyUserDataObject(this->coeff_obj, 0,
                                    sizeof(tivx_vpac_msc_coefficients_t),
                                    &coeffs,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST);
    }
    else
    {
        printf("[SCALER-MODULE] Unable to create scaler coeffs object! \n");
    }

    return status;
}

vx_status ScalerObj::createScalerOutputs(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_int32 num_ch, num_outputs;
    vx_int32 idx;

    num_ch = this->num_ch;
    num_outputs = this->num_outputs;

    if(num_outputs > APP_MODULES_MAX_SCALER_OUTPUTS)
    {
        printf("[SCALER-MODULE] Number of outputs %d greater than max supported %d!\n", num_outputs, APP_MODULES_MAX_SCALER_OUTPUTS);
        return VX_FAILURE;
    }

    for(idx = 0; idx < APP_MODULES_MAX_SCALER_OUTPUTS; idx++)
    {
        this->output[idx].arr  = NULL;
    }

    for(idx = 0; idx < num_outputs; idx++)
    {
        vx_image out_img;

        if(this->color_format == VX_DF_IMAGE_U8)
        {
            out_img  = vxCreateImage(context, this->output[idx].width, this->output[idx].height, VX_DF_IMAGE_U8);
            APP_PRINTF("Creating VX_DF_IMAGE_U8 image!\n");
        }
        else
        {
            out_img  = vxCreateImage(context, this->output[idx].width, this->output[idx].height, VX_DF_IMAGE_NV12);
            APP_PRINTF("Creating VX_DF_IMAGE_NV12 image!\n");
        }

        status = vxGetStatus((vx_reference)out_img);

        if(status == VX_SUCCESS)
        {
            this->output[idx].arr  = vxCreateObjectArray(context, (vx_reference)out_img, num_ch);
            vxReleaseImage(&out_img);

            status = vxGetStatus((vx_reference)this->output[idx].arr);
            if(status != VX_SUCCESS)
            {
                printf("[SCALER-MODULE] Unable to create output array! \n");
                break;
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_node_output_arr_%d", idx);

                vxSetReferenceName((vx_reference)this->output[idx].arr, name);
            }
        }
        else
        {
            printf("[SCALER-MODULE] Unable to create output image template! \n");
        }
    }

    if(this->en_out_scaler_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

        strcpy(file_path, this->output_file_path);
        this->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)this->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->file_path, "scaler_write_node_file_path");

            vxAddArrayItems(this->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[SCALER-MODULE] Unable to create file path object for fileio!\n");
        }

        for(idx = 0; idx < num_outputs; idx++)
        {
            char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

            sprintf(file_prefix, "scaler_output_%d", idx);
            this->file_prefix[idx] = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
            status = vxGetStatus((vx_reference)this->file_prefix[idx]);
            if(status == VX_SUCCESS)
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_file_prefix_%d", idx);

                vxSetReferenceName((vx_reference)this->file_prefix[idx], name);

                vxAddArrayItems(this->file_prefix[idx], TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
            }
            else
            {
                printf("[SCALER-MODULE] Unable to create file prefix object for output %d!\n", idx);
            }

            this->write_cmd[idx] = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
            status = vxGetStatus((vx_reference)this->write_cmd[idx]);
            if(status != VX_SUCCESS)
            {
                printf("[SCALER-MODULE] Unable to create write cmd object for output %d!\n", idx);
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_write_cmd_%d", idx);

                vxSetReferenceName((vx_reference)this->write_cmd[idx], name);
            }
        }

    }
    else
    {
        this->file_path   = NULL;
        for(idx = 0; idx < APP_MODULES_MAX_SCALER_OUTPUTS; idx++)
        {
            this->file_prefix[idx] = NULL;
            this->write_node[idx]  = NULL;
            this->write_cmd[idx]   = NULL;
        }
    }

    return status;
}

vx_status ScalerObj::initModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    vx_context context = graph->context;

    /* num_ch, num_outputs, input_width, input_height, bufq_depth must be set before calling */

    /* Validate required parameters */
    if(this->bufq_depth <= 0)
    {
        printf("[SCALER-MODULE] Error: bufq_depth must be > 0!\n");
        return VX_FAILURE;
    }

    if(this->input_width == 0 || this->input_height == 0)
    {
        printf("[SCALER-MODULE] Error: input_width and input_height must be set!\n");
        return VX_FAILURE;
    }

    /* Create internal input buffers (skip if external_input mode) */
    if(!this->external_input)
    {
        vx_image input_template;
        if(this->color_format == VX_DF_IMAGE_U8)
        {
            input_template = vxCreateImage(context, this->input_width, this->input_height, VX_DF_IMAGE_U8);
        }
        else
        {
            input_template = vxCreateImage(context, this->input_width, this->input_height, VX_DF_IMAGE_NV12);
        }

        status = vxGetStatus((vx_reference)input_template);
        if(status != VX_SUCCESS)
        {
            printf("[SCALER-MODULE] Failed to create input template image %dx%d\n", this->input_width, this->input_height);
            return status;
        }

        for(vx_int32 q = 0; q < this->bufq_depth; q++)
        {
            this->input_arr[q] = vxCreateObjectArray(context, (vx_reference)input_template, this->num_ch);
            status = vxGetStatus((vx_reference)this->input_arr[q]);
            if(status != VX_SUCCESS)
            {
                printf("[SCALER-MODULE] Failed to create input object array at index %d\n", q);
                vxReleaseImage(&input_template);
                return status;
            }
            this->input_images[q] = (vx_image)vxGetObjectArrayItem(this->input_arr[q], 0);
        }

        vxReleaseImage(&input_template);
        APP_PRINTF("[SCALER-MODULE] Created internal input buffers: %d x (%dx%d) x %d channels\n",
                   this->bufq_depth, this->input_width, this->input_height, this->num_ch);
    }
    else
    {
        APP_PRINTF("[SCALER-MODULE] External input mode: input_arr will be provided externally\n");
    }

    status = configureScalerCoeffs(context);

    if(status == VX_SUCCESS)
    {
        status = createScalerOutputs(context);
    }

    return status;
}

vx_status ScalerObj::deinitModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_int32 idx;

    /* Release internal input buffers (skip if external_input mode) */
    if(!this->external_input)
    {
        for(vx_int32 q = 0; q < this->bufq_depth; q++)
        {
            vxReleaseImage(&this->input_images[q]);
            vxReleaseObjectArray(&this->input_arr[q]);
        }
    }

    vxReleaseUserDataObject(&this->coeff_obj);

    for(idx = 0; idx < this->num_outputs; idx++)
    {
        vxReleaseObjectArray(&this->output[idx].arr);
    }

    if(this->en_out_scaler_write == 1)
    {
        vxReleaseArray(&this->file_path);
        for(idx = 0; idx < this->num_outputs; idx++)
        {
            vxReleaseArray(&this->file_prefix[idx]);
            vxReleaseUserDataObject(&this->write_cmd[idx]);
        }
    }

    return VX_SUCCESS;
}

vx_status ScalerObj::sendMscCoeff(vx_int32 module_index)
{
    vx_reference refs[1];
    refs[0] = (vx_reference)this->coeff_obj;

    vx_status status = tivxNodeSendCommand(this->node, 0u,
                                           TIVX_VPAC_MSC_CMD_SET_COEFF,
                                           refs, 1u);
    if (status != VX_SUCCESS)
    {
        printf("MSC: Node send command failed for scaler index %d!\n", module_index);
    }
    else
    {
        APP_PRINTF("App Send MSC Command Done for scaler index %d!\n", module_index);
    }

    return status;
}

vx_status ScalerObj::deleteNode(AppGraphObj *graph, vx_int32 module_index)
{
    vx_int32 idx;

    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
    for(idx = 0; idx < this->num_outputs; idx++)
    {
        if(this->write_node[idx] != NULL)
        {
            vxReleaseNode(&this->write_node[idx]);
        }
    }

    return VX_SUCCESS;
}

vx_status ScalerObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph = appGraph->graph;
    vx_status status = VX_SUCCESS;

    vx_image input   = (vx_image)vxGetObjectArrayItem((vx_object_array)this->input_arr[0], 0);

    vx_image output1, output2, output3, output4, output5;

    output1 = (this->output[0].arr != NULL) ? (vx_image)vxGetObjectArrayItem((vx_object_array)this->output[0].arr, 0) : NULL;
    output2 = (this->output[1].arr != NULL) ? (vx_image)vxGetObjectArrayItem((vx_object_array)this->output[1].arr, 0) : NULL;
    output3 = (this->output[2].arr != NULL) ? (vx_image)vxGetObjectArrayItem((vx_object_array)this->output[2].arr, 0) : NULL;
    output4 = (this->output[3].arr != NULL) ? (vx_image)vxGetObjectArrayItem((vx_object_array)this->output[3].arr, 0) : NULL;
    output5 = (this->output[4].arr != NULL) ? (vx_image)vxGetObjectArrayItem((vx_object_array)this->output[4].arr, 0) : NULL;

    this->node = tivxVpacMscScaleNode(graph, input, output1, output2, output3, output4, output5);

    status = vxGetStatus((vx_reference)this->node);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->node, "scaler_node");
        vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);

        vx_bool replicate[] = { vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_true_e};

        if(output1 == NULL)
            replicate[1] = vx_false_e;
        if(output1 == NULL)
            replicate[2] = vx_false_e;
        if(output2 == NULL)
            replicate[3] = vx_false_e;
        if(output3 == NULL)
            replicate[4] = vx_false_e;
        if(output4 == NULL)
            replicate[5] = vx_false_e;

        vxReplicateNode(graph, this->node, replicate, 6);

        if(this->en_out_scaler_write == 1)
        {
            if(output1 != NULL)
            {
                status = createGraphScalerWriteOutput(graph, 0);
                if(status != VX_SUCCESS)
                    printf("[SCALER-MODULE] Unable to create write node for output1!\n");
            }
            if(output2 != NULL)
            {
                status = createGraphScalerWriteOutput(graph, 1);
                if(status != VX_SUCCESS)
                    printf("[SCALER-MODULE] Unable to create write node for output2!\n");
            }
            if(output3 != NULL)
            {
                status = createGraphScalerWriteOutput(graph, 2);
                if(status != VX_SUCCESS)
                    printf("[SCALER-MODULE] Unable to create write node for output3!\n");
            }
            if(output4 != NULL)
            {
                status = createGraphScalerWriteOutput(graph, 3);
                if(status != VX_SUCCESS)
                    printf("[SCALER-MODULE] Unable to create write node for output4!\n");
            }
            if(output5 != NULL)
            {
                status = createGraphScalerWriteOutput(graph, 4);
                if(status != VX_SUCCESS)
                    printf("[SCALER-MODULE] Unable to create write node for output5!\n");
            }
        }
    }
    else
    {
        printf("[SCALER-MODULE] Unable to create scaler node! \n");
    }

    vxReleaseImage(&input);

    if(output1 != NULL) vxReleaseImage(&output1);
    if(output2 != NULL) vxReleaseImage(&output2);
    if(output3 != NULL) vxReleaseImage(&output3);
    if(output4 != NULL) vxReleaseImage(&output4);
    if(output5 != NULL) vxReleaseImage(&output5);

    return status;
}


vx_status ScalerObj::createGraphScalerWriteOutput(vx_graph graph, vx_int32 idx)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_image output_img = (vx_image)vxGetObjectArrayItem(this->output[idx].arr, 0);
    this->write_node[idx] = tivxWriteImageNode(graph, output_img, this->file_path, this->file_prefix[idx]);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)this->write_node[idx]);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->write_node[idx], "scaler_write_node");
        vxSetNodeTarget(this->write_node[idx], VX_TARGET_STRING, TIVX_TARGET_MPU_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, this->write_node[idx], replicate, 3);
    }
    else
    {
        printf("[SCALER-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
}

vx_status ScalerObj::sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;
    vx_int32 idx;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    for(idx = 0; idx < this->num_outputs; idx++)
    {
        status = vxCopyUserDataObject(this->write_cmd[idx], 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if(status == VX_SUCCESS)
        {
            vx_reference refs[2];

            refs[0] = (vx_reference)this->write_cmd[idx];

            status = tivxNodeSendCommand(this->write_node[idx], TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                    TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                    refs, 1u);

            if(VX_SUCCESS != status)
            {
                printf("Scaler write node send command failed!\n");
            }

            APP_PRINTF("Scaler write node send command success!\n");
        }
    }

    return (status);
}

vx_status readScalerInput(char* file_name, vx_object_array img_arr, vx_int32 read_mode, vx_int32 ch_num)
{
    vx_status status;

    status = vxGetStatus((vx_reference)img_arr);

    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_size arr_len;
        vx_int32 i, j;

        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        vxQueryObjectArray(img_arr, VX_OBJECT_ARRAY_NUMITEMS, &arr_len, sizeof(vx_size));

        /* For READ_CHANNEL mode, only read into the specified channel */
        vx_int32 start_idx = (read_mode == APP_MODULES_READ_CHANNEL) ? ch_num : 0;
        vx_int32 end_idx = (read_mode == APP_MODULES_READ_CHANNEL) ? (ch_num + 1) : arr_len;

        for (i = start_idx; i < end_idx; i++)
        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32 img_format;
            vx_image   in_img;
            vx_uint32  num_bytes = 0;

            in_img = (vx_image)vxGetObjectArrayItem(img_arr, i);

            vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;
            status = vxMapImagePatch(in_img,
                                    &rect,
                                    0,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);

            vx_int32 one_frame = img_width * img_height * 1.5; // in file YCbCr is present, but only luma is in use.

            if(read_mode == APP_MODULES_READ_FILE)
            {
                fseek(fp, one_frame * i, SEEK_SET);
            }
            else  /* APP_MODULES_READ_CHANNEL */
            {
                /* For READ_CHANNEL mode, each file contains one frame, seek to beginning */
                fseek(fp, 0, SEEK_SET);
            }

            /* Copy Luma */
            for (j = 0; j < (vx_int32)img_height; j++)
            {
                num_bytes += fread(data_ptr, 1, img_width, fp);
                data_ptr = (uint8_t *)data_ptr + image_addr.stride_y;
            }

            if(num_bytes != (img_width*img_height))
                printf("Luma bytes read = %d, expected = %d\n", num_bytes, img_width*img_height);

            vxUnmapImagePatch(in_img, map_id);

            if(img_format == VX_DF_IMAGE_NV12)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height / 2;
                status = vxMapImagePatch(in_img,
                                        &rect,
                                        1,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);


                /* Copy CbCr */
                num_bytes = 0;
                for (j = 0; j < (vx_int32)img_height/2; j++)
                {
                    num_bytes += fread(data_ptr, 1, img_width, fp);
                    data_ptr = (uint8_t *)data_ptr + image_addr.stride_y;
                }

                if(num_bytes != (img_width*img_height/2))
                    printf("CbCr bytes read = %d, expected = %d\n", num_bytes, img_width*img_height/2);

                vxUnmapImagePatch(in_img, map_id);
            }

            vxReleaseImage(&in_img);
        }

        fclose(fp);
    }

    return(status);
}

vx_status writeScalerOutput(char* file_name, vx_object_array img_arr)
{
    vx_status status;

    status = vxGetStatus((vx_reference)img_arr);

    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"wb");
        vx_size arr_len;
        vx_int32 i, j;

        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        vxQueryObjectArray(img_arr, VX_OBJECT_ARRAY_NUMITEMS, &arr_len, sizeof(vx_size));

        for (i = 0; i < (vx_int32)arr_len; i++)
        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_image   out_img;
            vx_uint32  num_bytes = 0;

            out_img = (vx_image)vxGetObjectArrayItem(img_arr, i);

            vxQueryImage(out_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(out_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;
            status = vxMapImagePatch(out_img,
                                    &rect,
                                    0,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);

            /* Copy Luma */
            for (j = 0; j < (vx_int32)img_height; j++)
            {
                num_bytes += fwrite(data_ptr, 1, img_width, fp);
                data_ptr = (uint8_t *)data_ptr + image_addr.stride_y;
            }

            if(num_bytes != (img_width*img_height))
                printf("Luma bytes written = %d, expected = %d\n", num_bytes, img_width*img_height);

            vxUnmapImagePatch(out_img, map_id);

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height / 2;
            status = vxMapImagePatch(out_img,
                                    &rect,
                                    1,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);


            /* Copy CbCr */
            num_bytes = 0;
            for (j = 0; j < (vx_int32)img_height/2; j++)
            {
                num_bytes += fwrite(data_ptr, 1, img_width, fp);
                data_ptr = (uint8_t *)data_ptr + image_addr.stride_y;
            }

            if(num_bytes != (img_width*img_height/2))
                printf("CbCr bytes written = %d, expected = %d\n", num_bytes, img_width*img_height/2);

            vxUnmapImagePatch(out_img, map_id);

            vxReleaseImage(&out_img);
        }

        fclose(fp);
    }

    return(status);
}

void scale_set_coeff(tivx_vpac_msc_coefficients_t *coeff, uint32_t interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    idx = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 256;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    idx = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 256;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256-weight;
            coeff->multi_phase[0][idx ++] = weight;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256-weight;
            coeff->multi_phase[1][idx ++] = weight;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256-weight;
            coeff->multi_phase[2][idx ++] = weight;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256-weight;
            coeff->multi_phase[3][idx ++] = weight;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
    else /* STR_VX_INTERPOLATION_NEAREST_NEIGHBOR */
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
}