/*
*
* Copyright (c) 2020-2026 Texas Instruments Incorporated
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
#include <utils/udma/include/app_udma_utils.h>
#include <utils/mem/include/app_mem.h>

#include <tivx_draw_box_detections_host.h>
#include "itidl_ti.h"
#include <math.h>

static tivx_target_kernel vx_DrawBoxDetections_kernel = NULL;

static vx_int32 clip_offset(vx_int32 input, vx_int32 max);
static void drawBox(vx_uint8 *y_data, vx_uint8 *cbcr_data, vx_int32 width, vx_int32 height, vx_int32 xmin, vx_int32 ymin, vx_int32 xmax, vx_int32 ymax, vx_int32 label, vx_uint8 color_map[][3], vx_int32 max_classes);

#ifndef x86_64
#if CPU_COPY    /* Not enabled - change to 1 for doing CPU copy */
static void memcpyC66(uint8_t *restrict pOut, uint8_t *restrict pIn, int32_t size)
{
    int32_t remSize = size - ((size >> 3) << 3);
    int32_t i;

    for(i = 0; i < size; i+=8)
    {
        _mem8(&pOut[i]) = _mem8(&pIn[i]);
    }
    if(remSize > 0)
    {
        i-=8;
        for(; i < size; i++)
        {
            pOut[i] = pIn[i];
        }
    }
}
#else
static void memcpyC66(uint8_t *pOut, uint8_t *pIn, int32_t size)
{
    app_udma_copy_1d_prms_t prms_1d;

    appUdmaCopy1DPrms_Init(&prms_1d);
    prms_1d.dest_addr   = appMemGetVirt2PhyBufPtr((uint64_t) pOut, APP_MEM_HEAP_DDR);
    prms_1d.src_addr    = appMemGetVirt2PhyBufPtr((uint64_t) pIn, APP_MEM_HEAP_DDR);
    prms_1d.length      = (uint32_t) size;
    appUdmaCopy1D(NULL, &prms_1d);
}
#endif
#else
#define memcpyC66 memcpy
#endif

static vx_status VX_CALLBACK tivxKernelDrawBoxDetectionsCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    return(status);
}

static vx_status VX_CALLBACK tivxKernelDrawBoxDetectionsDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDrawBoxDetectionsProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

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
        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr = NULL;

        tivx_obj_desc_tensor_t* input_tensor_desc;
        void * input_tensor_target_ptr = NULL;

        tivx_obj_desc_image_t* input_image_desc;
        void * input_image_target_ptr[2];

        tivx_obj_desc_image_t* output_image_desc;
        void * output_image_target_ptr[2];

        input_image_target_ptr[0] = NULL;
        input_image_target_ptr[1] = NULL;

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_CONFIGURATION_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        input_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_INPUT_TENSOR_IDX];
        input_tensor_target_ptr = tivxMemShared2TargetPtr(&input_tensor_desc->mem_ptr);
        tivxMemBufferMap(input_tensor_target_ptr, input_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        input_image_target_ptr[1] = NULL;
        if(input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_OUTPUT_IMAGE_IDX];
        output_image_target_ptr[0] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[0]);
        tivxMemBufferMap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        if(output_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            output_image_target_ptr[1] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[1]);
            tivxMemBufferMap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        }

        if(output_image_desc->mem_size[0] == input_image_desc->mem_size[0])
        {
            memcpyC66(output_image_target_ptr[0], input_image_target_ptr[0], output_image_desc->mem_size[0]);
        }

        if(output_image_desc->mem_size[1] == input_image_desc->mem_size[1])
        {
            memcpyC66(output_image_target_ptr[1], input_image_target_ptr[1], output_image_desc->mem_size[1]);
        }

        tivxDrawBoxDetectionsParams *params;
        sTIDL_IOBufDesc_t *ioBufDesc;

        TIDL_ODLayerHeaderInfo *pHeader;
        TIDL_ODLayerObjInfo *pObjInfo;
        vx_float32 *pOut;
        vx_uint32 numObjs;
        vx_float32 *output_buffer;
        vx_size output_sizes[4];

        vx_int32 i;

        params = (tivxDrawBoxDetectionsParams *)config_target_ptr;

        ioBufDesc = (sTIDL_IOBufDesc_t *)&params->ioBufDesc;

        output_buffer = (vx_float32 *)input_tensor_target_ptr;

        output_sizes[0] = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
        output_sizes[1] = ioBufDesc->outHeight[0] + ioBufDesc->outPadT[0] + ioBufDesc->outPadB[0];
        output_sizes[2] = ioBufDesc->outNumChannels[0];

        pOut   = (vx_float32*)output_buffer + (ioBufDesc->outPadT[0] * output_sizes[0]) + ioBufDesc->outPadL[0];
        pHeader  = (TIDL_ODLayerHeaderInfo *)pOut;
        pObjInfo = (TIDL_ODLayerObjInfo *)((uint8_t *)pOut + (vx_uint32)pHeader->objInfoOffset);
        numObjs  = (vx_uint32)pHeader->numDetObjects;

        vx_uint8 *data_ptr_1 = (vx_uint8 *)output_image_target_ptr[0];
        vx_uint8 *data_ptr_2 = (vx_uint8 *)output_image_target_ptr[1];

        vx_uint32 width  = params->width;
        vx_uint32 height = params->height;

        for (i = 0; i < numObjs; i++)
        {
            TIDL_ODLayerObjInfo *pDet = (TIDL_ODLayerObjInfo *) ((uint8_t *)pObjInfo + (i * ((vx_uint32)pHeader->objInfoSize)));

            /*Drawing of object for display purpose should be done only when score is high */
            if((pDet->score >= params->viz_th) && (data_ptr_1 != 0x0) && (data_ptr_2 != 0x0))
            {
                vx_int32   x_min = clip_offset((pDet->xmin * width), width);
                vx_int32   y_min = clip_offset((pDet->ymin * height), height);
                vx_int32   x_max = clip_offset((pDet->xmax * width), width);
                vx_int32   y_max = clip_offset((pDet->ymax * height), height);
                vx_int32   label = (vx_int32)pDet->label;

#if 0
                VX_PRINT(VX_ZONE_ERROR, "x_min = %d, y_min = %d, x_max =%d, y_max = %d, label = %d, score = %f\n",
                                        x_min, y_min, x_max, y_max, label, pDet->score);
#endif
                drawBox(data_ptr_1, data_ptr_2, width, height, x_min, y_min, x_max, y_max, label, &params->color_map[0], params->num_classes);
            }
        }

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(input_tensor_target_ptr, input_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

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
void tivxAddTargetKernelDrawBoxDetections()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
#if defined(SOC_J721S2) || defined(SOC_J784S4)
    else if (self_cpu == TIVX_CPU_ID_DSP_C7_1)
    {
        strncpy(target_name, TIVX_TARGET_DSP_C7_1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
#endif
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name);
    }

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_DrawBoxDetections_kernel = tivxAddTargetKernelByName
                                     (
                                        TIVX_KERNEL_DRAW_BOX_DETECTIONS_NAME,
                                        target_name,
                                        tivxKernelDrawBoxDetectionsProcess,
                                        tivxKernelDrawBoxDetectionsCreate,
                                        tivxKernelDrawBoxDetectionsDelete,
                                        NULL,
                                        NULL
                                     );
    }
}

void tivxRemoveTargetKernelDrawBoxDetections()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_DrawBoxDetections_kernel);
    if (status == VX_SUCCESS)
    {
        vx_DrawBoxDetections_kernel = NULL;
    }
}

static vx_int32 clip_offset(vx_int32 input, vx_int32 max)
{
    vx_int32 output;
    output = (input >= max) ? max - 1 : input;
    output = output < 0 ? 0 : output;
    return output;
}

static void drawBox(vx_uint8 *y_data, vx_uint8 *cbcr_data, vx_int32 width, vx_int32 height, vx_int32 xmin, vx_int32 ymin, vx_int32 xmax, vx_int32 ymax, vx_int32 label, vx_uint8 color_map[][3], vx_int32 max_classes)
{
    vx_uint8 y_label, cb_label, cr_label;
    vx_int32 i;

    xmin = (xmin >> 1) << 1;
    xmax = (xmax >> 1) << 1;
    ymin = (ymin >> 1) << 1;
    ymax = (ymax >> 1) << 1;

    y_label  = color_map[label][0];
    cb_label = color_map[label][1];
    cr_label = color_map[label][2];

    if(label >= max_classes)
    {
        y_label  = 255;
        cb_label = 128;
        cr_label = 128;
    }

    if(y_data != NULL)
    {
        vx_int32 box_height  = ymax - ymin;
        for(i = 0; i < box_height; i++)
        {
            y_data[(width * (ymin + i)) + xmin] = y_label;
            y_data[(width * (ymin + i)) + xmax] = y_label;
        }

        vx_int32 box_width  = xmax - xmin;
        for(i = 0; i < box_width; i++)
        {
            y_data[(width * ymin) + xmin + i] = y_label;
            y_data[(width * ymax) + xmin + i] = y_label;
        }
    }

    if(cbcr_data != NULL)
    {
        ymin = ymin >> 1;
        ymax = ymax >> 1;

        vx_int32 box_height  = ymax - ymin;
        for(i = 0; i < box_height; i++)
        {
            cbcr_data[(width * (ymin + i)) + xmin + 0] = cb_label;
            cbcr_data[(width * (ymin + i)) + xmin + 1] = cr_label;

            cbcr_data[(width * (ymin + i)) + xmax + 0] = cb_label;
            cbcr_data[(width * (ymin + i)) + xmax + 1] = cr_label;
        }

        vx_int32 box_width  = xmax - xmin;
        for(i = 0; i < box_width; i+=2)
        {
            cbcr_data[(width * ymin) + xmin + i + 0] = cb_label;
            cbcr_data[(width * ymin) + xmin + i + 1] = cr_label;

            cbcr_data[(width * ymax) + xmin + i + 0] = cb_label;
            cbcr_data[(width * ymax) + xmin + i + 1] = cr_label;
        }
    }

}
