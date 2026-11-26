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

#include "app_img_mosaic_module.hpp"
#include <string.h>

ImgMosaicObj::ImgMosaicObj() { setDefaults(); }

void ImgMosaicObj::setDefaults()
{
    this->background  = NULL;
    this->node        = NULL;
    this->kernel      = NULL;
    this->config      = NULL;
    memset(&this->params, 0, sizeof(this->params));
    for (int i = 0; i < (int)TIVX_IMG_MOSAIC_MAX_INPUTS; i++)
    {
        this->input_arr[i] = NULL;
    }
    for (int i = 0; i < (int)APP_MODULES_MAX_BUFQ_DEPTH; i++)
    {
        this->output_image[i] = NULL;
    }
    this->graph_parameter_index = 0;
    this->out_width   = 1920;
    this->out_height  = 1080;
    this->num_inputs  = 1;
    this->bufq_depth  = 1;  /* Default 1 for backward compatibility with OD apps */

    tivxImgMosaicParamsSetDefaults(&this->params);
    this->params.windows[0].startX = 500;
    this->params.windows[0].startY = 200;
    this->params.num_windows       = 1;
    this->params.clear_count       = 4;
}

vx_status ImgMosaicObj::configureParams(vx_context context)
{
    vx_status status = VX_SUCCESS;

    this->config = vxCreateUserDataObject(context, "ImgMosaicConfig", sizeof(tivxImgMosaicParams), NULL);
    status = vxGetStatus((vx_reference)this->config);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->config, "mosaic_node_config");

        status = vxCopyUserDataObject(this->config, 0, sizeof(tivxImgMosaicParams),\
                    &this->params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        if(status != VX_SUCCESS)
        {
            printf("[SW-MOSAIC-MODULE] Unable to copy mosaic params! \n");
        }
    }
    else
    {
        printf("[SW-MOSAIC-MODULE] Unable to create config object! \n");
    }

    return status;
}

vx_status ImgMosaicObj::createSwMosaicKernel(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    for(i = 0; i < (vx_int32)TIVX_IMG_MOSAIC_MAX_INPUTS; i++)
    {
        this->input_arr[i] = NULL;
    }

    this->kernel = tivxAddKernelImgMosaic(context, this->num_inputs);
    status = vxGetStatus((vx_reference)this->kernel);

    if(status != VX_SUCCESS)
    {
        printf("[SW-MOSAIC-MODULE] Unable to create kernel with %d inputs!\n", this->num_inputs);
    }

    return status;
}

vx_status ImgMosaicObj::createOutputImage(vx_context context, vx_int32 bufq_depth)
{
    vx_status status = VX_SUCCESS;
    vx_int32 q;

    for(q = 0; q < bufq_depth; q++)
    {
        this->output_image[q] = vxCreateImage(context, this->out_width, this->out_height, VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)this->output_image[q]);
        if(status != VX_SUCCESS)
        {
            printf("[SW-MOSAIC-MODULE] Unable to create output image of size %d x %d!\n", this->out_width, this->out_height);
            break;
        }
        else
        {
            vx_char name[VX_MAX_REFERENCE_NAME];

            snprintf(name, VX_MAX_REFERENCE_NAME, "mosaic_node_output_image_%d", q);

            vxSetReferenceName((vx_reference)this->output_image[q], name);
        }
    }

    return status;
}

/* ---- Virtual dispatch methods ---- */

vx_status ImgMosaicObj::initModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_context context = appGraph->context;
    vx_int32 bufq_depth = this->bufq_depth;  /* Use configured depth (defaults to 1) */
    vx_status status = VX_SUCCESS;

    status = this->configureParams(context);

    if(status == VX_SUCCESS)
    {
        status = this->createOutputImage(context, bufq_depth);
    }

    if(status == VX_SUCCESS)
    {
        status = this->createSwMosaicKernel(context);
    }

    return status;
}

vx_status ImgMosaicObj::deinitModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_int32 q;
    vx_int32 bufq_depth = this->bufq_depth;  /* Use configured depth (defaults to 1) */

    vxReleaseUserDataObject(&this->config);
    for(q = 0; q < bufq_depth; q++)
    {
        vxReleaseImage(&this->output_image[q]);
    }

    return VX_SUCCESS;
}

vx_status ImgMosaicObj::deleteNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }

    if(this->kernel != NULL)
    {
        vxRemoveKernel(this->kernel);
    }
    return VX_SUCCESS;
}

vx_status ImgMosaicObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph_vx = appGraph->graph;
    vx_status status = VX_SUCCESS;

    this->node = tivxImgMosaicNode(graph_vx,
                             this->kernel,
                             this->config,
                             this->output_image[0],
                             this->background,
                             this->input_arr,
                             this->num_inputs);

    status = vxGetStatus((vx_reference)this->node);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->node, "mosaic_node");
        vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);
    }
    else
    {
        printf("[SW-MOSAIC-MODULE] Unable to create mosaic node! \n");
    }

    return (status);
}

vx_status ImgMosaicObj::writeMosaicOutput(char* file_name, vx_image out_img)
{
    vx_status status;
    vx_int32 j;

    status = vxGetStatus((vx_reference)out_img);

    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"wb");

        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32  num_bytes = 0;

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
            {
                printf("Luma bytes written = %d, expected = %d", num_bytes, img_width*img_height);
            }

            vxUnmapImagePatch(out_img, map_id);

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
            {
                printf("CbCr bytes written = %d, expected = %d", num_bytes, img_width*img_height/2);
            }

            vxUnmapImagePatch(out_img, map_id);

        }

        fclose(fp);
    }

    return(status);
}
