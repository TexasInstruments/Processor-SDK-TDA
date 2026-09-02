/*
*
* Copyright (c) 2021-2026 Texas Instruments Incorporated
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



#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_target_utils.h>
#include <utils/mem/include/app_mem.h>

#include <tivx_dl_color_blend_host.h>

typedef struct {
    tivxDLColorBlendParams prms;
    
    /** Color map for each output, number of colors not to exceed TIVX_DL_COLOR_BLEND_MAX_COLORS */
    vx_uint8 color_map_yuv[TIVX_DL_COLOR_BLEND_MAX_CLASSES][TIVX_DL_COLOR_BLEND_MAX_COLORS];

}tivxDLColorBlendObject;

static tivx_target_kernel vx_dlColorBlend_kernel = NULL;

static inline void convertRGBToYCbCr(int32_t R, int32_t G, int32_t B, int32_t *Y, int32_t *Cb, int32_t *Cr, int32_t min_val, int32_t max_val)
{
    int32_t Y_  = (( 66*R) + (129*G) +  (25*B));
    int32_t Cb_ = ((-38*R) + (-74*G) + (112*B));
    int32_t Cr_ = ((112*R) + (-94*G) + (-18*B));

    Y_  = (Y_  + 128) >> 8;
    Cb_ = (Cb_ + 128) >> 8;
    Cr_ = (Cr_ + 128) >> 8;

    Y_  = Y_  + 16;
    Cb_ = Cb_ + 128;
    Cr_ = Cr_ + 128;

    if(Y_ < min_val) { Y_ = min_val; }
    if(Y_ > max_val) { Y_ = max_val; }

    if(Cb_ < min_val) { Cb_ = min_val; }
    if(Cb_ > max_val) { Cb_ = max_val; }

    if(Cr_ < min_val) { Cr_ = min_val; }
    if(Cr_ > max_val) { Cr_ = max_val; }

    *Y  = Y_;
    *Cb = Cb_;
    *Cr = Cr_;
}

static vx_status VX_CALLBACK tivxKernelDLColorBlendCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    tivxDLColorBlendObject * obj = NULL;
    tivxDLColorBlendParams * prms = NULL;

    obj = tivxMemAlloc(sizeof(tivxDLColorBlendObject), TIVX_MEM_EXTERNAL);

    if (NULL == obj)
    {
        status = VX_FAILURE;
    }
    else
    {
        int32_t n;

        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_DL_COLOR_BLEND_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        tivxDLColorBlendParams *params = (tivxDLColorBlendParams *)config_target_ptr;

        prms = (tivxDLColorBlendParams *)&obj->prms;

        memcpy(prms, params, sizeof(tivxDLColorBlendParams));

        if(prms->use_color_map == 0)
        {
            /* Randomly assign RGB colors for N classes */
            for(n = 0; n < prms->num_classes; n++)
            {
                prms->color_map[n][0] = rand() % 256;
                prms->color_map[n][1] = rand() % 256;
                prms->color_map[n][2] = rand() % 256;
            }
        }

        /* Convert RGB color map to YUV */
        for(n = 0; n < prms->num_classes; n++)
        {
            int32_t R = prms->color_map[n][0];
            int32_t G = prms->color_map[n][1];
            int32_t B = prms->color_map[n][2];

            int32_t Y, Cb, Cr;

            convertRGBToYCbCr(R, G, B, &Y, &Cb, &Cr, 0, 255);

            obj->color_map_yuv[n][0] = (uint8_t)Y;
            obj->color_map_yuv[n][1] = (uint8_t)Cb;
            obj->color_map_yuv[n][2] = (uint8_t)Cr;

        }

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        status = tivxSetTargetKernelInstanceContext(kernel, obj,  sizeof(tivxDLColorBlendObject));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLColorBlendDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxDLColorBlendObject *obj = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&obj, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(obj, sizeof(tivxDLColorBlendObject), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static inline int32_t get_class_id(void *tensor_data, vx_enum data_type, int32_t offset)
{
    int32_t class_id = 0;

    if(data_type == VX_TYPE_INT8)
    {
        class_id = *((int8_t *)tensor_data + offset);
    }
    else if(data_type == VX_TYPE_UINT8)
    {
        class_id = *((uint8_t *)tensor_data + offset);
    }
    else if(data_type == VX_TYPE_INT16)
    {
        class_id = *((int16_t *)tensor_data + offset);
    }
    else if(data_type == VX_TYPE_UINT16)
    {
        class_id = *((uint16_t *)tensor_data + offset);
    }
    else if(data_type == VX_TYPE_INT32)
    {
        class_id = *((int32_t *)tensor_data + offset);
    }
    else if(data_type == VX_TYPE_UINT32)
    {
        class_id = *((uint32_t *)tensor_data + offset);
    }
    else if(data_type == VX_TYPE_FLOAT32)
    {
        class_id = *((float *)tensor_data + offset);
    }

    return class_id;

}
static vx_status VX_CALLBACK tivxKernelDLColorBlendProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxDLColorBlendObject *obj = NULL;
    vx_int32 i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor %d is NULL!\n", i);
            status = VX_FAILURE;
            break;
        }
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&obj, &size);
        if ((VX_SUCCESS != status) || (NULL == obj) ||
            (sizeof(tivxDLColorBlendObject) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "incorrect kernel context params in dl-color-blend process function!\n");
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        tivx_obj_desc_tensor_t *in_tensor_desc;
        void* in_tensor_target_ptr;

        tivx_obj_desc_image_t* input_image_desc;
        void * input_image_target_ptr[2];

        input_image_target_ptr[0] = NULL;
        input_image_target_ptr[1] = NULL;

        tivx_obj_desc_image_t *output_image_desc;
        void * output_image_target_ptr[2];

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        in_tensor_desc  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_DL_COLOR_BLEND_INPUT_TENSOR_IDX];
        in_tensor_target_ptr  = tivxMemShared2TargetPtr(&in_tensor_desc->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_BLEND_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        if(input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_BLEND_OUTPUT_IMAGE_IDX];
        output_image_target_ptr[0] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[0]);
        tivxMemBufferMap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        if(output_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            output_image_target_ptr[1] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[1]);
            tivxMemBufferMap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        }

        tivxDLColorBlendParams *prms = (tivxDLColorBlendParams *)&obj->prms;
        if((input_image_desc->format == VX_DF_IMAGE_NV12) && 
            (output_image_desc->format == VX_DF_IMAGE_NV12))
        {
            int32_t w, h;

            /* Copy luma plane, swap CbCr plane */
            memcpy(output_image_target_ptr[0], input_image_target_ptr[0], input_image_desc->mem_size[0]);

            /* Tensor output resolution might be different from image output resolution */
            /* User nearest neighbor interpolation method select tensor class-ids to appropriate output color-map */

            float tensor_width  = in_tensor_desc->dimensions[0];
            float tensor_height = in_tensor_desc->dimensions[1];

            float output_width  = output_image_desc->width;
            float output_height = output_image_desc->height >> 1;

            float w_ratio = tensor_width / (1.0 * output_width);
            float h_ratio = tensor_height / (1.0 * output_height);

            int32_t w_loc, h_loc, w_idx, h_idx;

            uint8_t *pOut = output_image_target_ptr[1];

            for (h = 0; h < output_height; h++)
            {
                h_loc = h_ratio * (h + 0.5) - 0.5;
                h_loc = (h_loc < 0) ? 0 : h_loc;
                h_idx = h_loc;
                for (w = 0; w < output_width; w+=2)
                {
                    w_loc = w_ratio * (w + 0.5) - 0.5;
                    w_loc = (w_loc < 0) ? 0 : w_loc;
                    w_idx = w_loc;
                    int32_t tensor_offset = (h_idx * tensor_width) + w_idx;
                    int32_t output_offset = (h * output_width) + w;
                    int32_t class_id = get_class_id(in_tensor_target_ptr, in_tensor_desc->data_type, tensor_offset);

                    if((class_id >= 0) && (class_id < prms->num_classes))
                    {
                        pOut[output_offset + 0] = obj->color_map_yuv[class_id][1];
                        pOut[output_offset + 1] = obj->color_map_yuv[class_id][2];
                    }
                }
            }
        }
        else if((input_image_desc->format == VX_DF_IMAGE_RGB) &&
                (output_image_desc->format == VX_DF_IMAGE_RGB))
        {
            int32_t w, h;

            /* Tensor output resolution might be different from image output resolution */
            /* User nearest neighbor interpolation method select tensor class-ids to appropriate output color-map */

            float tensor_width  = in_tensor_desc->dimensions[0];
            float tensor_height = in_tensor_desc->dimensions[1];

            float output_width  = output_image_desc->width;
            float output_height = output_image_desc->height;

            float w_ratio = tensor_width / (1.0 * output_width);
            float h_ratio = tensor_height / (1.0 * output_height);

            int32_t w_loc, h_loc, w_idx, h_idx;

            uint8_t *pIn = input_image_target_ptr[0];
            uint8_t *pOut = output_image_target_ptr[0];

            for (h = 0; h < output_height; h++)
            {
                h_loc = h_ratio * (h + 0.5) - 0.5;
                h_loc = (h_loc < 0) ? 0 : h_loc;
                h_idx = h_loc;
                
                int32_t k = 0;
                for (w = 0; w < output_width; w++)
                {
                    w_loc = w_ratio * (w + 0.5) - 0.5;
                    w_loc = (w_loc < 0) ? 0 : w_loc;
                    w_idx = w_loc;
                    int32_t tensor_offset = (h_idx * tensor_width) + w_idx;
                    int32_t class_id = get_class_id(in_tensor_target_ptr, in_tensor_desc->data_type, tensor_offset);
                    int32_t output_offset = (h * output_image_desc->imagepatch_addr[0].stride_y);

                    if((class_id >= 0) && (class_id < prms->num_classes))
                    {
                        float R = pIn[output_offset + k + 0];
                        float G = pIn[output_offset + k + 1];
                        float B = pIn[output_offset + k + 2];
                        
                        float R_ = obj->prms.color_map[class_id][0];
                        float G_ = obj->prms.color_map[class_id][1];
                        float B_ = obj->prms.color_map[class_id][2];

                        pOut[output_offset + k + 0] = (uint8_t)((R * 0.6f) + (R_ * 0.4f));
                        pOut[output_offset + k + 1] = (uint8_t)((G * 0.6f) + (G_ * 0.4f));
                        pOut[output_offset + k + 2] = (uint8_t)((B * 0.6f) + (B_ * 0.4f));

                        k+=3;
                    }
                }
            }
        }

        tivxMemBufferUnmap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if(input_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        tivxMemBufferUnmap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if(output_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
    }

    return (status);
}

void tivxAddTargetKernelDLColorBlend()
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_dlColorBlend_kernel = tivxAddTargetKernelByName
                              (
                                TIVX_KERNEL_DL_COLOR_BLEND_NAME,
                                target_name,
                                tivxKernelDLColorBlendProcess,
                                tivxKernelDLColorBlendCreate,
                                tivxKernelDLColorBlendDelete,
                                NULL,
                                NULL
                              );
    }
}

void tivxRemoveTargetKernelDLColorBlend()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dlColorBlend_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dlColorBlend_kernel = NULL;
    }
}
