/*
*
* Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#include <tivx_draw_keypoint_detections_host.h>
#include "itidl_ti.h"
#include <math.h>

static tivx_target_kernel vx_DrawKeypointDetections_kernel = NULL;
static void drawPoints(tivxDrawKeypointDetectionsParams *params, vx_uint8 *data_ptr_1, vx_uint8 *data_ptr_2, vx_uint16* kp_ptr, vx_int32 num_points, vx_int32 label);
static void drawJoinedPoints(tivxDrawKeypointDetectionsParams *params, vx_uint8 *data_ptr_1, vx_uint8 *data_ptr_2, vx_uint16* kp_ptr, vx_int32 num_points, vx_int32 label);
static vx_int32 lineInterp(vx_uint16 startPoint[2], vx_uint16 endPoint[2], vx_uint16 *outPoints, vx_int32 total_points);

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

static vx_status VX_CALLBACK tivxKernelDrawKeypointDetectionsCreate
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

static vx_status VX_CALLBACK tivxKernelDrawKeypointDetectionsDelete(
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

static vx_status VX_CALLBACK tivxKernelDrawKeypointDetectionsProcess
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

        tivx_obj_desc_tensor_t* kp_tensor_desc;
        void * kp_tensor_target_ptr = NULL;

        tivx_obj_desc_tensor_t* kp_valid_desc;
        void * kp_valid_target_ptr = NULL;

        tivx_obj_desc_tensor_t* input_tensor_desc;
        void * input_tensor_target_ptr = NULL;

        tivx_obj_desc_image_t* input_image_desc;
        void * input_image_target_ptr[2];

        tivx_obj_desc_image_t* output_image_desc;
        void * output_image_target_ptr[2];


        tivxDrawKeypointDetectionsParams *params;
        sTIDL_IOBufDesc_t *ioBufDesc;

        TIDL_ODLayerHeaderInfo *pHeader;
        TIDL_ODLayerObjInfo *pObjInfo;
        vx_float32 *pOut;
        vx_uint32 numObjs;
        TIDL_ODLayerObjInfo * pPSpots;
        vx_int32 total_points_per_box;
        vx_uint16 *kp_ptr;
        vx_uint8 *kp_valid_ptr;
        vx_uint8 *data_ptr_1;
        vx_uint8 *data_ptr_2;
        vx_float32 *output_buffer;
        vx_int32 draw_joined_points = 1;

        vx_size output_sizes[4];

        vx_int32 i;

        input_image_target_ptr[0] = NULL;
        input_image_target_ptr[1] = NULL;

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_CONFIGURATION_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        kp_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_KEYPOINT_TENSOR_IDX];
        kp_tensor_target_ptr = tivxMemShared2TargetPtr(&kp_tensor_desc->mem_ptr);
        tivxMemBufferMap(kp_tensor_target_ptr, kp_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        kp_valid_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_KEYPOINT_VALID_IDX];
        kp_valid_target_ptr = tivxMemShared2TargetPtr(&kp_valid_desc->mem_ptr);
        tivxMemBufferMap(kp_valid_target_ptr, kp_valid_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        input_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_INPUT_TENSOR_IDX];
        input_tensor_target_ptr = tivxMemShared2TargetPtr(&input_tensor_desc->mem_ptr);
        tivxMemBufferMap(input_tensor_target_ptr, input_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        input_image_target_ptr[1] = NULL;
        if(input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_OUTPUT_IMAGE_IDX];
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

        params = (tivxDrawKeypointDetectionsParams *)config_target_ptr;
        ioBufDesc = (sTIDL_IOBufDesc_t *)&params->ioBufDesc;

        output_buffer = (vx_float32 *)input_tensor_target_ptr;

        output_sizes[0] = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
        output_sizes[1] = ioBufDesc->outHeight[0] + ioBufDesc->outPadT[0] + ioBufDesc->outPadB[0];
        output_sizes[2] = ioBufDesc->outNumChannels[0];

        pOut   = (vx_float32*)output_buffer + (ioBufDesc->outPadT[0] * output_sizes[0]) + ioBufDesc->outPadL[0];
        pHeader  = (TIDL_ODLayerHeaderInfo *)pOut;
        pObjInfo = (TIDL_ODLayerObjInfo *)((uint8_t *)pOut + (vx_uint32)pHeader->objInfoOffset);
        numObjs  = (vx_uint32)pHeader->numDetObjects;

        total_points_per_box = params->num_keypoints * params->points_per_line;

        kp_ptr = (vx_uint16 *)kp_tensor_target_ptr;
        kp_valid_ptr = (vx_uint8 *)kp_valid_target_ptr;

        data_ptr_1 = (vx_uint8 *)output_image_target_ptr[0];
        data_ptr_2 = (vx_uint8 *)output_image_target_ptr[1];

        for (i = 0; i < numObjs; i++)
        {
            pPSpots = (TIDL_ODLayerObjInfo *) ((uint8_t *)pObjInfo + (i * ((vx_uint32)pHeader->objInfoSize)));

            /*Drawing of object for display purpose should be done only when score is high and key point valid flag is 1*/
            /*For drawing purpose interpolated and post processed key points are used*/
            if((pPSpots->score >= params->viz_th) && (kp_valid_ptr[i] == 1) && (data_ptr_1 != 0x0) && (data_ptr_2 != 0x0))
            {
                if(draw_joined_points == 0x0){
                    drawPoints(params, data_ptr_1, data_ptr_2, &kp_ptr[i*total_points_per_box*2],total_points_per_box, pPSpots->label);
                }else{
                    drawJoinedPoints(params, data_ptr_1, data_ptr_2, &kp_ptr[i*total_points_per_box*2],total_points_per_box, pPSpots->label);
                }
            }
        }

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(kp_tensor_target_ptr, kp_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(kp_valid_target_ptr, kp_valid_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(input_tensor_target_ptr, input_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if(input_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        tivxMemBufferUnmap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if(output_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
    }

    return (status);
}
void tivxAddTargetKernelDrawKeypointDetections()
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
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name);
    }

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_DrawKeypointDetections_kernel = tivxAddTargetKernelByName
                                        (
                                            TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_NAME,
                                            target_name,
                                            tivxKernelDrawKeypointDetectionsProcess,
                                            tivxKernelDrawKeypointDetectionsCreate,
                                            tivxKernelDrawKeypointDetectionsDelete,
                                            NULL,
                                            NULL
                                        );
    }
}

void tivxRemoveTargetKernelDrawKeypointDetections()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_DrawKeypointDetections_kernel);
    if (status == VX_SUCCESS)
    {
        vx_DrawKeypointDetections_kernel = NULL;
    }
}

static void drawPoints(tivxDrawKeypointDetectionsParams *params, vx_uint8 *data_ptr_1, vx_uint8 *data_ptr_2, vx_uint16* kp_ptr, vx_int32 num_points, vx_int32 label)
{
    vx_uint8 *pData1;
    vx_uint8 *pData2;

    vx_int32 img_width;
    vx_int32 img_height;
    vx_int32 img_stride;
    vx_int32 kp_x,kp_y;
    vx_int32 i,j,k;
    vx_uint8 color[3];
    vx_int32 kp_disp_dim = 2;
    vx_int32 draw_lines = 0;
    vx_int32 cbcr_offset;

    /*Currently key points are generated in original image resolution 1280x720, hence
      to display it back on 512x512 imge it has to be rescaled.
     */
    vx_float32 scale_fact_x = (vx_float32)params->dl_width/params->width;
    vx_float32 scale_fact_y = (vx_float32)params->dl_height/params->height;

    img_width  = params->dl_width;
    img_height = params->dl_height;
    img_stride = params->dl_width;

    for(i = 0; i < params->num_classes; i++)
    {
        if(params->class_id[i] == label)
        {
            draw_lines = 1;
            break;
        }
    }

    if(draw_lines == 1)
    {
        if(label == 1)
        {
            color[0] = 128; //Y
            color[1] = 55;  //Cb
            color[2] = 35;  //Cr
        }
        else if(label == 2)
        {
            color[0] = 66;  //Y
            color[1] = 91;  //Cb
            color[2] = 240; //Cr
        }
        else if(label == 3)
        {
            color[0] = 128; //Y
            color[1] = 55;  //Cb
            color[2] = 35;  //Cr
        }
        else
        {
            color[0] = 0;   //Y
            color[1] = 128; //Cb
            color[2] = 128; //Cr
        }

        for(i = 0; i < num_points; i++)
        {

            kp_x = (vx_int32)((vx_float32)kp_ptr[2*i]*scale_fact_x);
            kp_y = (vx_int32)((vx_float32)kp_ptr[2*i + 1]*scale_fact_y);

            if((kp_x >= kp_disp_dim/2) &&
            (kp_x < (img_width - kp_disp_dim/2)) &&
            (kp_y > kp_disp_dim/2) &&
            (kp_y < (img_height - kp_disp_dim/2)))
            {

                pData1 = (vx_uint8 *)data_ptr_1 + (kp_y * img_stride) + kp_x;
                for(j = -kp_disp_dim/2; j < kp_disp_dim/2; j++)
                {
                    for(k = -kp_disp_dim/2; k < kp_disp_dim/2; k++)
                    {
                        pData1[(j*img_stride) + k] = color[0];
                    }
                }

                pData2 = (vx_uint8 *)data_ptr_2;
                for(j = -kp_disp_dim/2; j < kp_disp_dim/2; j++)
                {
                    for(k = -kp_disp_dim/2; k < kp_disp_dim/2; k++)
                    {
                        cbcr_offset = (((kp_y + j)>> 1) * img_stride) + (((kp_x + k) >> 1)<< 1);
                        pData2[cbcr_offset] = color[1];
                        pData2[cbcr_offset + 1] = color[2];
                    }
                }
            }
        }
    }
}

static void drawJoinedPoints(tivxDrawKeypointDetectionsParams *params, vx_uint8 *data_ptr_1, vx_uint8 *data_ptr_2, vx_uint16* kp_ptr, vx_int32 num_points, vx_int32 label)
{
    vx_uint8 *pData1;
    vx_uint8 *pData2;

    vx_int32 img_width;
    vx_int32 img_height;
    vx_int32 img_stride;
    vx_int32 kp_x,kp_y;
    vx_int32 i,j,k;
    vx_uint8 color[3];
    vx_int32 kp_disp_dim = 2;
    vx_int32 draw_lines = 0;

    vx_uint16 cur_kp[2],next_kp[2];
    vx_uint16 local_line[1024*2];
    vx_int32  local_line_num_points = 256;
    vx_int32  cur_points;
    vx_int32  l = 0;
    vx_int32  cbcr_offset;

    /*Currently key points are generated in original image resolution 1280x720, hence
      to display it back on 512x512 imge it has to be rescaled.
     */
    vx_float32 scale_fact_x = (vx_float32)params->dl_width/params->width;
    vx_float32 scale_fact_y = (vx_float32)params->dl_height/params->height;

    img_width  = params->dl_width;
    img_height = params->dl_height;
    img_stride = params->dl_width;

    for(i = 0; i < params->num_classes; i++)
    {
        if(params->class_id[i] == label)
        {
            draw_lines = 1;
            break;
        }
    }

    if(draw_lines == 1)
    {
        if(label == 1)
        {
            color[0] = 128; //Y
            color[1] = 55;  //Cb
            color[2] = 35;  //Cr
        }
        else if(label == 2)
        {
            color[0] = 66;  //Y
            color[1] = 91;  //Cb
            color[2] = 240; //Cr
        }
        else if(label == 3)
        {
            color[0] = 128; //Y
            color[1] = 55;  //Cb
            color[2] = 35;  //Cr
        }
        else
        {
            color[0] = 0;   //Y
            color[1] = 128; //Cb
            color[2] = 128; //Cr
        }

        for(i = 0; i < num_points; i++)
        {

            cur_kp[0] = (vx_uint16)((vx_float32)kp_ptr[2*i]*scale_fact_x);
            cur_kp[1] = (vx_uint16)((vx_float32)kp_ptr[2*i + 1]*scale_fact_y);

            next_kp[0] = (vx_uint16)((vx_float32)kp_ptr[2*((i+1)%num_points)]*scale_fact_x);
            next_kp[1] = (vx_uint16)((vx_float32)kp_ptr[2*((i+1)%num_points) + 1]*scale_fact_y);

            cur_points = lineInterp(cur_kp, next_kp, local_line, local_line_num_points);

            for(l = 0; l < cur_points; l++)
            {

                kp_x = local_line[2*l];
                kp_y = local_line[2*l + 1];

                if((kp_x >= kp_disp_dim/2) &&
                (kp_x < (img_width - kp_disp_dim/2)) &&
                (kp_y > kp_disp_dim/2) &&
                (kp_y < (img_height - kp_disp_dim/2)))
                {

                    pData1 = (vx_uint8 *)data_ptr_1 + (kp_y * img_stride) + kp_x;
                    for(j = -kp_disp_dim/2; j < kp_disp_dim/2; j++)
                    {
                        for(k = -kp_disp_dim/2; k < kp_disp_dim/2; k++)
                        {
                            pData1[(j*img_stride) + k] = color[0];
                        }
                    }

                    pData2 = (vx_uint8 *)data_ptr_2;
                    for(j = -kp_disp_dim/2; j < kp_disp_dim/2; j++)
                    {
                        for(k = -kp_disp_dim/2; k < kp_disp_dim/2; k++)
                        {
                            cbcr_offset = (((kp_y + j)>> 1) * img_stride) + (((kp_x + k) >> 1)<< 1);
                            pData2[cbcr_offset] = color[1];
                            pData2[cbcr_offset + 1] = color[2];
                        }
                    }
                }
            }
        }
    }
}

static vx_int32 lineInterp(vx_uint16 startPoint[2], vx_uint16 endPoint[2], vx_uint16 *outPoints, vx_int32 total_points)
{
    int32_t x_0 = startPoint[0];
    int32_t y_0 = startPoint[1];

    int32_t x_1 = endPoint[0];
    int32_t y_1 = endPoint[1];
    int32_t i;
    float y_inc,x_inc,x,y;
    int32_t dx,dy;
    int32_t steps;

    dx = x_1 - x_0;
    dy = y_1 - y_0;

    if(abs(dx) > abs(dy))
    {
        /*increment in x direction and have all the x axis pixels filled up*/
        steps = abs(dx);
    }
    else
    {
        steps = abs(dy);
    }

    if(steps > total_points)
    {
        steps = total_points;
    }

    x_inc = ((float)dx)/(float)(steps);
    y_inc = ((float)dy)/(float)(steps);

    x = x_0;
    y = y_0;

    for(i = 0; i < steps; i++)
    {
        outPoints[2*i + 0] = (uint16_t)x;
        outPoints[2*i + 1] = (uint16_t)y;

        x = x + x_inc;
        y = y + y_inc;
    }

    for(; i < total_points; i++)
    {
        outPoints[2*i + 0] = outPoints[2*(i-1) + 0];
        outPoints[2*i + 1] = outPoints[2*(i-1) + 1];
    }
    /*returns real number of unique points*/
    return(steps);
}
