/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include "app_scaler_module.h"

static vx_status configure_scaler_coeffs(vx_context context, ScalerObj *scalerObj)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_msc_coefficients_t coeffs;

    scale_set_coeff(&coeffs, VX_INTERPOLATION_BILINEAR);

    /* Set Coefficients */
    scalerObj->coeff_obj = vxCreateUserDataObject(context,
                                "tivx_vpac_msc_coefficients_t",
                                sizeof(tivx_vpac_msc_coefficients_t),
                                NULL);
    status = vxGetStatus((vx_reference)scalerObj->coeff_obj);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)scalerObj->coeff_obj, "scaler_node_coeff_obj");

        status = vxCopyUserDataObject(scalerObj->coeff_obj, 0,
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

static vx_status create_scaler_outputs(vx_context context, ScalerObj *scalerObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 num_ch, num_outputs;
    vx_int32 idx;

    num_ch = scalerObj->num_ch;
    num_outputs = scalerObj->num_outputs;

    if(num_outputs > APP_MODULES_MAX_SCALER_OUTPUTS)
    {
        printf("[SCALER-MODULE] Number of outputs %d greater than max supported %d!\n", num_outputs, APP_MODULES_MAX_SCALER_OUTPUTS);
        return VX_FAILURE;
    }

    for(idx = 0; idx < APP_MODULES_MAX_SCALER_OUTPUTS; idx++)
    {
        scalerObj->output[idx].arr  = NULL;
    }

    for(idx = 0; idx < num_outputs; idx++)
    {
        vx_image out_img;

        if(scalerObj->color_format == VX_DF_IMAGE_U8)
        {
            out_img  = vxCreateImage(context, scalerObj->output[idx].width, scalerObj->output[idx].height, VX_DF_IMAGE_U8);
            APP_PRINTF("Creating VX_DF_IMAGE_U8 image!\n");
        }
        else
        {
            out_img  = vxCreateImage(context, scalerObj->output[idx].width, scalerObj->output[idx].height, VX_DF_IMAGE_NV12);
            APP_PRINTF("Creating VX_DF_IMAGE_NV12 image!\n");
        }

        status = vxGetStatus((vx_reference)out_img);

        if(status == VX_SUCCESS)
        {
            scalerObj->output[idx].arr  = vxCreateObjectArray(context, (vx_reference)out_img, num_ch);
            vxReleaseImage(&out_img);

            status = vxGetStatus((vx_reference)scalerObj->output[idx].arr);
            if(status != VX_SUCCESS)
            {
                printf("[SCALER-MODULE] Unable to create output array! \n");
                break;
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_node_output_arr_%d", idx);

                vxSetReferenceName((vx_reference)scalerObj->output[idx].arr, name);
            }
        }
        else
        {
            printf("[SCALER-MODULE] Unable to create output image template! \n");
        }
    }

    if(scalerObj->en_out_scaler_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

        strcpy(file_path, scalerObj->output_file_path);
        scalerObj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)scalerObj->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)scalerObj->file_path, "scaler_write_node_file_path");

            vxAddArrayItems(scalerObj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[SCALER-MODULE] Unable to create file path object for fileio!\n");
        }

        for(idx = 0; idx < num_outputs; idx++)
        {
            char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

            sprintf(file_prefix, "scaler_output_%d", idx);
            scalerObj->file_prefix[idx] = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
            status = vxGetStatus((vx_reference)scalerObj->file_prefix[idx]);
            if(status == VX_SUCCESS)
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_file_prefix_%d", idx);

                vxSetReferenceName((vx_reference)scalerObj->file_prefix[idx], name);

                vxAddArrayItems(scalerObj->file_prefix[idx], TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
            }
            else
            {
                printf("[SCALER-MODULE] Unable to create file prefix object for output %d!\n", idx);
            }

            scalerObj->write_cmd[idx] = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
            status = vxGetStatus((vx_reference)scalerObj->write_cmd[idx]);
            if(status != VX_SUCCESS)
            {
                printf("[SCALER-MODULE] Unable to create write cmd object for output %d!\n", idx);
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_write_cmd_%d", idx);

                vxSetReferenceName((vx_reference)scalerObj->write_cmd[idx], name);
            }
        }

    }
    else
    {
        scalerObj->file_path   = NULL;
        for(idx = 0; idx < APP_MODULES_MAX_SCALER_OUTPUTS; idx++)
        {
            scalerObj->file_prefix[idx] = NULL;
            scalerObj->write_node[idx]  = NULL;
            scalerObj->write_cmd[idx]   = NULL;
        }
    }

    return status;
}

vx_status app_init_scaler(vx_context context, ScalerObj *scalerObj, char *objName, vx_int32 num_ch, vx_int32 num_outputs)
{
    vx_status status = VX_SUCCESS;

    scalerObj->num_outputs = num_outputs;
    scalerObj->num_ch = num_ch;

    status = configure_scaler_coeffs(context, scalerObj);

    if(status == VX_SUCCESS)
    {
        status = create_scaler_outputs(context, scalerObj);
    }

    return status;
}

void app_deinit_scaler(ScalerObj *scalerObj)
{
    vx_int32 num_outputs = scalerObj->num_outputs;
    vx_int32 idx;

    vxReleaseUserDataObject(&scalerObj->coeff_obj);

    for(idx = 0; idx < num_outputs; idx++)
    {
        vxReleaseObjectArray(&scalerObj->output[idx].arr);
    }

    if(scalerObj->en_out_scaler_write == 1)
    {
        vxReleaseArray(&scalerObj->file_path);
        for(idx = 0; idx < num_outputs; idx++)
        {
            vxReleaseArray(&scalerObj->file_prefix[idx]);
            vxReleaseUserDataObject(&scalerObj->write_cmd[idx]);
        }
    }
}

void app_delete_scaler(ScalerObj *scalerObj)
{
    vx_int32 num_outputs = scalerObj->num_outputs;
    vx_int32 idx;

    if(scalerObj->node != NULL)
    {
        vxReleaseNode(&scalerObj->node);
    }
    for(idx = 0; idx < num_outputs; idx++)
    {
        if(scalerObj->write_node[idx] != NULL)
        {
            vxReleaseNode(&scalerObj->write_node[idx]);
        }
    }
}

vx_status app_create_graph_scaler(vx_context context, vx_graph graph, ScalerObj *scalerObj, vx_object_array input_img_arr)
{
    vx_status status = VX_SUCCESS;

    vx_image input   = (vx_image)vxGetObjectArrayItem((vx_object_array)input_img_arr, 0);

    vx_image output1, output2, output3, output4, output5;

    if(scalerObj->output[0].arr != NULL)
    {
        output1 = (vx_image)vxGetObjectArrayItem((vx_object_array)scalerObj->output[0].arr, 0);
    }
    else
    {
        output1 = NULL;
    }
    if(scalerObj->output[1].arr != NULL)
    {
        output2 = (vx_image)vxGetObjectArrayItem((vx_object_array)scalerObj->output[1].arr, 0);
    }
    else
    {
        output2 = NULL;
    }
    if(scalerObj->output[2].arr != NULL)
    {
        output3 = (vx_image)vxGetObjectArrayItem((vx_object_array)scalerObj->output[2].arr, 0);
    }
    else
    {
        output3 = NULL;
    }
    if(scalerObj->output[3].arr != NULL)
    {
        output4 = (vx_image)vxGetObjectArrayItem((vx_object_array)scalerObj->output[3].arr, 0);
    }
    else
    {
        output4 = NULL;
    }
    if(scalerObj->output[4].arr != NULL)
    {
        output5 = (vx_image)vxGetObjectArrayItem((vx_object_array)scalerObj->output[4].arr, 0);
    }
    else
    {
        output5 = NULL;
    }

    scalerObj->node = tivxVpacMscScaleNode(graph, input, output1, output2, output3, output4, output5);

    status = vxGetStatus((vx_reference)scalerObj->node);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)scalerObj->node, "scaler_node");
        vxSetNodeTarget(scalerObj->node, VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);

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

        vxReplicateNode(graph, scalerObj->node, replicate, 6);

        if(scalerObj->en_out_scaler_write == 1)
        {
            if(output1 != NULL)
            {
                status = app_create_graph_scaler_write_output(graph, scalerObj, 0);
                if(status != VX_SUCCESS)
                {
                    printf("[SCALER-MODULE] Unable to create write node for output1!\n");
                }
            }
            if(output2 != NULL)
            {
                status = app_create_graph_scaler_write_output(graph, scalerObj, 1);
                if(status != VX_SUCCESS)
                {
                    printf("[SCALER-MODULE] Unable to create write node for output2!\n");
                }
            }
            if(output3 != NULL)
            {
                status = app_create_graph_scaler_write_output(graph, scalerObj, 2);
                if(status != VX_SUCCESS)
                {
                    printf("[SCALER-MODULE] Unable to create write node for output3!\n");
                }
            }
            if(output4 != NULL)
            {
                status = app_create_graph_scaler_write_output(graph, scalerObj, 3);
                if(status != VX_SUCCESS)
                {
                    printf("[SCALER-MODULE] Unable to create write node for output4!\n");
                }
            }
            if(output5 != NULL)
            {
                status = app_create_graph_scaler_write_output(graph, scalerObj, 4);
                if(status != VX_SUCCESS)
                {
                    printf("[SCALER-MODULE] Unable to create write node for output5!\n");
                }
            }

        }
    }
    else
    {
        printf("[SCALER-MODULE] Unable to create scaler node! \n");
    }

    vxReleaseImage(&input);

    if(output1 != NULL)
        vxReleaseImage(&output1);
    if(output2 != NULL)
        vxReleaseImage(&output2);
    if(output3 != NULL)
        vxReleaseImage(&output3);
    if(output4 != NULL)
        vxReleaseImage(&output4);
    if(output5 != NULL)
        vxReleaseImage(&output5);

    return status;
}


vx_status app_create_graph_scaler_write_output(vx_graph graph, ScalerObj *scalerObj, vx_int32 idx)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_image output_img = (vx_image)vxGetObjectArrayItem(scalerObj->output[idx].arr, 0);
    scalerObj->write_node[idx] = tivxWriteImageNode(graph, output_img, scalerObj->file_path, scalerObj->file_prefix[idx]);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)scalerObj->write_node[idx]);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)scalerObj->write_node[idx], "scaler_write_node");
        vxSetNodeTarget(scalerObj->write_node[idx], VX_TARGET_STRING, TIVX_TARGET_MPU_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, scalerObj->write_node[idx], replicate, 3);
    }
    else
    {
        printf("[SCALER-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
}

vx_status app_send_cmd_scaler_write_node(ScalerObj *scalerObj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;
    vx_int32 idx;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    for(idx = 0; idx < scalerObj->num_outputs; idx++)
    {
        status = vxCopyUserDataObject(scalerObj->write_cmd[idx], 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if(status == VX_SUCCESS)
        {
            vx_reference refs[2];

            refs[0] = (vx_reference)scalerObj->write_cmd[idx];

            status = tivxNodeSendCommand(scalerObj->write_node[idx], TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
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

        for (i = 0; i < arr_len; i++)
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
            else
            {
                fseek(fp, one_frame * ch_num, SEEK_SET);
            }

            /* Copy Luma */
            for (j = 0; j < img_height; j++)
            {
                num_bytes += fread(data_ptr, 1, img_width, fp);
                data_ptr += image_addr.stride_y;
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
                for (j = 0; j < img_height/2; j++)
                {
                    num_bytes += fread(data_ptr, 1, img_width, fp);
                    data_ptr += image_addr.stride_y;
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

        for (i = 0; i < arr_len; i++)
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
            for (j = 0; j < img_height; j++)
            {
                num_bytes += fwrite(data_ptr, 1, img_width, fp);
                data_ptr += image_addr.stride_y;
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
            for (j = 0; j < img_height/2; j++)
            {
                num_bytes += fwrite(data_ptr, 1, img_width, fp);
                data_ptr += image_addr.stride_y;
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

static vx_status create_scaler_outputs_q(vx_context context,
                                         ScalerObj *scalerObj)
{
    vx_status status = VX_SUCCESS;

    vx_int32  num_ch      = 0;
    vx_int32  num_outputs = 0;
    vx_uint32 bufq_depth  = 0U;

    vx_int32  out_idx = 0;
    vx_uint32 buf_idx = 0U;

    if ((context == NULL) || (scalerObj == NULL))
    {
        status = VX_FAILURE;
    }
    else
    {
        num_ch      = scalerObj->num_ch;
        num_outputs = scalerObj->num_outputs;
        bufq_depth  = scalerObj->bufq_depth;

        if (num_outputs > APP_MODULES_MAX_SCALER_OUTPUTS)
        {
            printf("[SCALER-MODULE] Number of outputs %d greater than max supported %d!\n",
                   num_outputs, APP_MODULES_MAX_SCALER_OUTPUTS);
            status = VX_FAILURE;
        }
        else if ((bufq_depth == 0U) || (bufq_depth > (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH))
        {
            printf("[SCALER-MODULE] Invalid bufq_depth=%u (max=%d)\n",
                   (unsigned)bufq_depth, APP_MODULES_MAX_BUFQ_DEPTH);
            status = VX_FAILURE;
        }
        else
        {
            for (out_idx = 0; out_idx < APP_MODULES_MAX_SCALER_OUTPUTS; out_idx++)
            {
                for (buf_idx = 0U; buf_idx < (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH; buf_idx++)
                {
                    scalerObj->output_q[out_idx].arr[buf_idx] = NULL;
                    scalerObj->output_q[out_idx].img[buf_idx] = NULL;
                }
            }

            for (out_idx = 0; (out_idx < num_outputs) && (status == VX_SUCCESS); out_idx++)
            {
                vx_image out_img_tmpl = NULL;

                if (scalerObj->color_format == VX_DF_IMAGE_U8)
                {
                    out_img_tmpl = vxCreateImage(context,
                                                 scalerObj->output_q[out_idx].width,
                                                 scalerObj->output_q[out_idx].height,
                                                 VX_DF_IMAGE_U8);
                }
                else
                {
                    out_img_tmpl = vxCreateImage(context,
                                                 scalerObj->output_q[out_idx].width,
                                                 scalerObj->output_q[out_idx].height,
                                                 VX_DF_IMAGE_NV12);
                }

                status = vxGetStatus((vx_reference)out_img_tmpl);
                if (status != VX_SUCCESS)
                {
                    printf("[SCALER-MODULE] Unable to create output image template!\n");
                }

                for (buf_idx = 0U; (buf_idx < bufq_depth) && (status == VX_SUCCESS); buf_idx++)
                {
                    scalerObj->output_q[out_idx].arr[buf_idx] =
                        vxCreateObjectArray(context, (vx_reference)out_img_tmpl, num_ch);

                    status = vxGetStatus((vx_reference)scalerObj->output_q[out_idx].arr[buf_idx]);
                    if (status != VX_SUCCESS)
                    {
                        printf("[SCALER-MODULE] Unable to create output array!\n");
                    }

                    if (status == VX_SUCCESS)
                    {
                        vx_char name[VX_MAX_REFERENCE_NAME];

                        snprintf(name,
                                 VX_MAX_REFERENCE_NAME,
                                 "scaler_node_output_arr_%d_buf_%u",
                                 out_idx,
                                 (unsigned)buf_idx);

                        (void)vxSetReferenceName(
                            (vx_reference)scalerObj->output_q[out_idx].arr[buf_idx],
                            name);

                        scalerObj->output_q[out_idx].img[buf_idx] =
                            (vx_image)vxGetObjectArrayItem(
                                scalerObj->output_q[out_idx].arr[buf_idx],
                                0);

                        snprintf(name,
                                 sizeof(name),
                                 "scaler_out%d_img_q%u",
                                 out_idx,
                                 (unsigned)buf_idx);
                        (void)vxSetReferenceName(
                            (vx_reference)scalerObj->output_q[out_idx].img[buf_idx],
                            name);

                        status = vxGetStatus(
                            (vx_reference)scalerObj->output_q[out_idx].img[buf_idx]);
                        if (status != VX_SUCCESS)
                        {
                            printf("[SCALER-MODULE] Unable to get output image!\n");
                        }
                    }
                }

                if (out_img_tmpl != NULL)
                {
                    (void)vxReleaseImage(&out_img_tmpl);
                    out_img_tmpl = NULL;
                }
            }
        }
    }

    if (status != VX_SUCCESS)
    {
        /* cleanup partial allocations */
        for (out_idx = 0; out_idx < num_outputs; out_idx++)
        {
            for (buf_idx = 0U; buf_idx < bufq_depth; buf_idx++)
            {
                if (scalerObj->output_q[out_idx].img[buf_idx] != NULL)
                {
                    (void)vxReleaseImage(&scalerObj->output_q[out_idx].img[buf_idx]);
                    scalerObj->output_q[out_idx].img[buf_idx] = NULL;
                }

                if (scalerObj->output_q[out_idx].arr[buf_idx] != NULL)
                {
                    (void)vxReleaseObjectArray(&scalerObj->output_q[out_idx].arr[buf_idx]);
                    scalerObj->output_q[out_idx].arr[buf_idx] = NULL;
                }
            }
        }
    }

    return status;
}

vx_status app_init_scaler_queued(vx_context context,
                                ScalerObj *scalerObj,
                                char *objName,
                                vx_int32 num_ch,
                                vx_int32 num_outputs,
                                vx_uint32 bufq_depth)
{
    vx_status status = VX_SUCCESS;

    if (scalerObj == NULL)
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        scalerObj->num_outputs = num_outputs;
        scalerObj->num_ch = num_ch;
        scalerObj->bufq_depth = bufq_depth;

        if (objName != NULL)
        {
            strncpy(scalerObj->objName,
                    objName,
                    APP_MODULES_MAX_OBJ_NAME_SIZE - 1);
            scalerObj->objName[APP_MODULES_MAX_OBJ_NAME_SIZE - 1] = 0;
        }
        else
        {
            scalerObj->objName[0] = 0;
        }
    }

    if (status == VX_SUCCESS)
    {
        status = configure_scaler_coeffs(context, scalerObj);
    }

    if (status == VX_SUCCESS)
    {
        status = create_scaler_outputs_q(context, scalerObj);
    }

    return status;
}

void app_deinit_scaler_queued(ScalerObj *scalerObj)
{
    vx_int32 out_idx;
    vx_uint32 buf_idx;

    if (scalerObj == NULL)
    {
        return;
    }

    if (scalerObj->coeff_obj != NULL)
    {
        (void)vxReleaseUserDataObject(&scalerObj->coeff_obj);
        scalerObj->coeff_obj = NULL;
    }

    for (out_idx = 0; out_idx < scalerObj->num_outputs; out_idx++)
    {
        for (buf_idx = 0; buf_idx < scalerObj->bufq_depth; buf_idx++)
        {
            if (scalerObj->output_q[out_idx].img[buf_idx] != NULL)
            {
                (void)vxReleaseImage(&scalerObj->output_q[out_idx].img[buf_idx]);
                scalerObj->output_q[out_idx].img[buf_idx] = NULL;
            }
        }
    }

    for (out_idx = 0; out_idx < scalerObj->num_outputs; out_idx++)
    {
        for (buf_idx = 0; buf_idx < scalerObj->bufq_depth; buf_idx++)
        {
            if (scalerObj->output_q[out_idx].arr[buf_idx] != NULL)
            {
                (void)vxReleaseObjectArray(&scalerObj->output_q[out_idx].arr[buf_idx]);
                scalerObj->output_q[out_idx].arr[buf_idx] = NULL;
            }
        }
    }

    scalerObj->bufq_depth = 0;
    scalerObj->num_outputs = 0;
    scalerObj->num_ch = 0;
}


vx_status app_create_graph_scaler_queued(vx_context context,
                                        vx_graph graph,
                                        ScalerObj *scalerObj,
                                        vx_object_array input_img_arr)
{
    vx_status status = VX_SUCCESS;

    vx_image input = NULL;

    vx_image output1 = NULL;
    vx_image output2 = NULL;
    vx_image output3 = NULL;
    vx_image output4 = NULL;
    vx_image output5 = NULL;

    (void)context;

    input = (vx_image)vxGetObjectArrayItem(input_img_arr, 0);
    status = vxGetStatus((vx_reference)input);

    if (status == VX_SUCCESS && scalerObj->output_q[0].img[0] != NULL)
    {
        output1 = scalerObj->output_q[0].img[0];
        status = vxGetStatus((vx_reference)output1);
    }

    if (status == VX_SUCCESS && scalerObj->output_q[1].img[0] != NULL)
    {
        output2 = scalerObj->output_q[1].img[0];
        status = vxGetStatus((vx_reference)output2);
    }

    if (status == VX_SUCCESS && scalerObj->output_q[2].img[0] != NULL)
    {
        output3 = scalerObj->output_q[2].img[0];
        status = vxGetStatus((vx_reference)output3);
    }

    if (status == VX_SUCCESS && scalerObj->output_q[3].img[0] != NULL)
    {
        output4 = scalerObj->output_q[3].img[0];
        status = vxGetStatus((vx_reference)output4);
    }

    if (status == VX_SUCCESS && scalerObj->output_q[4].img[0] != NULL)
    {
        output5 = scalerObj->output_q[4].img[0];
        status = vxGetStatus((vx_reference)output5);
    }

    if (status == VX_SUCCESS)
    {
        scalerObj->node = tivxVpacMscScaleNode(graph,
                                              input,
                                              output1,
                                              output2,
                                              output3,
                                              output4,
                                              output5);

        status = vxGetStatus((vx_reference)scalerObj->node);

        if (status != VX_SUCCESS)
        {
            printf("[SCALER-MODULE] Unable to create scaler node!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        vx_bool replicate[6];

        replicate[0] = vx_true_e;
        replicate[1] = (output1 != NULL) ? vx_true_e : vx_false_e;
        replicate[2] = (output2 != NULL) ? vx_true_e : vx_false_e;
        replicate[3] = (output3 != NULL) ? vx_true_e : vx_false_e;
        replicate[4] = (output4 != NULL) ? vx_true_e : vx_false_e;
        replicate[5] = (output5 != NULL) ? vx_true_e : vx_false_e;

        vxSetReferenceName((vx_reference)scalerObj->node, "scaler_node");

        status = vxSetNodeTarget(scalerObj->node, VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);

        if (status == VX_SUCCESS)
        {
            status = vxReplicateNode(graph, scalerObj->node, replicate, 6);
        }
    }

    if (input != NULL)
    {
        (void)vxReleaseImage(&input);
    }

    return status;
}
