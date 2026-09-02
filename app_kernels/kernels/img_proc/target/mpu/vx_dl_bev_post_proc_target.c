/*
 *
 * Copyright (c) 2025-2026 Texas Instruments Incorporated
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
#include <tivx_dl_bev_post_proc_host.h>
#include <vx_nv12_drawing_utils.h>
#include <string.h>


//#define BEV_DEBUG_LOGS

#define Lidar_Drawings

static tivx_target_kernel vx_DrawBevBoxDetections_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDrawBevBoxDetectionsCreate
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

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDrawBevBoxDetectionsDelete
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

    return (status);
}

static vx_size getTensorDataType(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if(tidl_type == TIDL_SignedChar)
    {
        openvx_type = VX_TYPE_INT8;
    }
    else if(tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    }
    else if(tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if(tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if(tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if(tidl_type == TIDL_SinglePrecFloat)
    {
        openvx_type = VX_TYPE_FLOAT32;
    }

    return openvx_type;
}

/**
 * @param frame Original NV12 data buffer, where the in-place updates will happen
 * @param box bounding box co-ordinates.
 * @param outDataWidth width of the output buffer.
 * @param outDataHeight Height of the output buffer.
 *
 * @returns original frame with some in-place post processing done
 */

static void blendSegMask(vx_uint8    *frame_uv_ptr,
                         void        *classes,
                         int32_t     inDataWidth,
                         int32_t     inDataHeight,
                         int32_t     outDataWidth,
                         int32_t     outDataHeight,
                         float       alpha,
                         uint8_t     **colorMap,
                         uint8_t     maxClass,
                         vx_size     data_type)
{
    uint8_t     a;
    uint8_t     sa;
    uint8_t     u_m;
    uint8_t     v_m;
    int32_t     w;
    int32_t     h;
    int32_t     sw;
    int32_t     sh;
    int32_t     class_id;
    int32_t     rowOffset;

    a  = alpha * 256;
    sa = (1 - alpha ) * 256;

    // Here, (w, h) iterate over frame and (sw, sh) iterate over classes
    for (h = 0; h < outDataHeight/2; h++)
    {
        sh = (int32_t)((h << 1) * inDataHeight / outDataHeight);
        rowOffset = sh*inDataWidth;

        for (w = 0; w < outDataWidth; w+=2)
        {
            int32_t index;

            sw = (int32_t)(w * inDataWidth / outDataWidth);
            index = (int32_t)(rowOffset + sw);

            if(data_type == VX_TYPE_UINT8)
            {
                class_id =  *((uint8_t *)classes+index);
            }
            else if(data_type == VX_TYPE_INT8)
            {
                class_id =  *((int8_t *)classes+index);
            }
            else if(data_type == VX_TYPE_UINT16)
            {
                class_id =  *((uint16_t *)classes+index);
            }
            else if(data_type == VX_TYPE_INT16)
            {
                class_id =  *((int16_t *)classes+index);
            }
            else if(data_type == VX_TYPE_UINT32)
            {
                class_id =  *((uint32_t *)classes+index);
            }
            else if(data_type == VX_TYPE_INT32)
            {
                class_id =  *((int32_t *)classes+index);
            }
            else if(data_type == VX_TYPE_FLOAT32)
            {
                class_id =  *((float *)classes+index);
            }
            else
            {
                /* The Program should never reach here */
                class_id = -1;
            }
            
            if (class_id < maxClass)
            {
                u_m = colorMap[class_id][1];
                v_m = colorMap[class_id][2];
            }
            else
            {
                u_m = 128;
                v_m = 128;
            }
            u_m = ((*(frame_uv_ptr) * a) + (u_m * sa)) >> 8;
            v_m = ((*(frame_uv_ptr+1) * a) + (v_m * sa)) >> 8;
            *((uint16_t*)frame_uv_ptr) = (v_m << 8) | u_m;
            frame_uv_ptr += 2;
        }
    }
}

static vx_status VX_CALLBACK tivxKernelDrawBevBoxDetectionsProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;


    VX_PRINT(VX_ZONE_INFO, "Started Process Kernel\n");

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor %d is NULL!\n", i);
            status = VX_FAILURE;
            break;
        }
    }

    if(VX_SUCCESS == status)
    {
        tivx_obj_desc_tensor_t *in_tensor_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_MAX_INPUTS];
        void* in_tensor_target_ptr[TIVX_KERNEL_DRAW_BOX_DETECTIONS_MAX_INPUTS];

        for(i = 0; i < TIVX_KERNEL_DRAW_BOX_DETECTIONS_MAX_INPUTS; i++)
        {
            in_tensor_target_ptr[i] = NULL;
            in_tensor_desc[i] = NULL;
        }

        tivx_obj_desc_image_t* input_image_desc;
        void * input_image_target_ptr[2];

        input_image_target_ptr[0] = NULL;
        input_image_target_ptr[1] = NULL;

        tivx_obj_desc_image_t *output_image_desc;
        void * output_image_target_ptr[2];

         tivx_obj_desc_tensor_t* Lidar_2Camera_tensor_desc;
         void * Lidar_2Camera_tensor_target_ptr = NULL;

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_CONFIGURATION_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxDLBEVPostProcParams *prms = (tivxDLBEVPostProcParams *)config_target_ptr;

        for(i = 0; i < prms->num_input_tensors; i++)
        {   
            in_tensor_desc[i]  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_INPUT_TENSOR_START_IDX + i];
            in_tensor_target_ptr[i]  = tivxMemShared2TargetPtr(&in_tensor_desc[i]->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr[i], in_tensor_desc[i]->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

         Lidar_2Camera_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_LIDAR_2_IMG_TENSOR_IDX];
         Lidar_2Camera_tensor_target_ptr = tivxMemShared2TargetPtr(&Lidar_2Camera_tensor_desc->mem_ptr);
         tivxMemBufferMap(Lidar_2Camera_tensor_target_ptr, Lidar_2Camera_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if(input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_BOX_DETECTIONS_OUTPUT_IMAGE_IDX];
        output_image_target_ptr[0] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[0]);
        tivxMemBufferMap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if(output_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            output_image_target_ptr[1] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[1]);
            tivxMemBufferMap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        }

        if(output_image_desc->mem_size[0] == input_image_desc->mem_size[0])
        {
            memcpy(output_image_target_ptr[0], input_image_target_ptr[0], output_image_desc->mem_size[0]);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "OUtput_mem_size != input_mem_size, \n");
        }

        if(output_image_desc->mem_size[1] == input_image_desc->mem_size[1])
        {
            memcpy(output_image_target_ptr[1], input_image_target_ptr[1], output_image_desc->mem_size[1]);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "OUtput_mem_size != input_mem_size, \n");
        }

        
        if(prms->task_type == TIVX_DL_BEV_POST_PROC_CLASSIFICATION_TASK_TYPE)
        {
            sTIDL_IOBufDesc_t *ioBufDesc = (sTIDL_IOBufDesc_t *)prms->oc_prms.ioBufDesc;

            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[0]);
            vx_uint32 classid[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
            vx_int32 i, j;

            for(i = 0; i < TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES; i++)
            {
                classid[i] = (vx_uint32)-1;
            }

            if(data_type == VX_TYPE_FLOAT32)
            {
                vx_float32 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_float32 *pIn;

                pIn = (vx_float32 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for(i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = -FLT_MAX;
                    classid[i] = (vx_uint32)-1;

                    if(prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for(; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if(pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if(classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = -FLT_MAX;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "invalid class ID, ideally it should not reach here \n");
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }

                    VX_PRINT(VX_ZONE_INFO, "score[%d] = %f, classid[%d] = %d  \n", i, score[i], i, classid[i]);
                }
            }
            else if(data_type == VX_TYPE_INT16)
            {
                vx_int16 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_int16 *pIn;

                pIn = (vx_int16 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for(i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = INT16_MIN;
                    classid[i] = (vx_uint32)-1;

                    if(prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for(j = 0; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if(pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if(classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = INT16_MIN;
                    }
                    else
                    {
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }
                }
            }
            else if(data_type == VX_TYPE_INT8)
            {
                vx_int8 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_int8 *pIn;

                pIn = (vx_int8 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for(i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = INT8_MIN;
                    classid[i] = (vx_uint32)-1;

                    if(prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for(j = 0; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if(pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if(classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = INT8_MIN;
                    }
                    else
                    {
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }
                }
            }
            else if(data_type == VX_TYPE_UINT8)
            {
                vx_uint8 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_uint8 *pIn;

                pIn = (vx_uint8 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for(i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = 0;
                    classid[i] = (vx_uint32)-1;

                    if(prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for(j = 0; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if(pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if(classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = 0;
                    }
                    else
                    {
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }
                }
            }

            Image           imgHolder;
            YUVColor        titleColor;
            FontProperty    titleFont;
            YUVColor        textColor;
            FontProperty    textFont;
            YUVColor        textBgColor;
            char            title[40];

            /** Get YUV value for green color. */
            getColor(&titleColor,0,255,0);

            /** Get YUV value for yellow color. */
            getColor(&textColor,255,255,0);

            /** Get YUV value for dark blue color. */
            getColor(&textBgColor, 5, 11, 120);
            
            /** Get Monospace font from available font sizes
             *  where width of character is closest to 1.5%
             *  of the total width image width
             */
            int titleSize  = (int)(0.015*output_image_desc->width);
            getFont(&titleFont,titleSize);
            
            /** Get Monospace font from available font sizes
             *  where width of character is closest to 1.5%
             *  of the total width image width
             */
            int textSize  = (int)(0.015*output_image_desc->width);
            getFont(&textFont,textSize);

            imgHolder.yRowAddr     = (uint8_t *)output_image_target_ptr[0];
            imgHolder.uvRowAddr    = (uint8_t *)output_image_target_ptr[1];
            imgHolder.width        = output_image_desc->width;
            imgHolder.height       = output_image_desc->height;
            
            snprintf(title, sizeof(title), "Recognized Classes (Top %d) : \n", prms->oc_prms.num_top_results);

            int yOffset = (titleFont.height) + 12;

            drawRect(&imgHolder,
                     0,
                     10,
                     (titleFont.width * strlen(title)) + 10,
                     yOffset,
                     &textBgColor,
                     -1);

            drawText(&imgHolder,title,5,10,&titleFont,&titleColor);

            for (int i = 0; i < prms->oc_prms.num_top_results; i++)
            {
                VX_PRINT(VX_ZONE_INFO, "classid[i] = %d, prms->oc_prms.labelOffset = %d \n", classid[i], prms->oc_prms.labelOffset);
                int32_t index = classid[i] + prms->oc_prms.labelOffset;

                if (index >= 0)
                {
                    char *str = prms->oc_prms.classnames[index];
                    int32_t row = (i*textFont.height) + yOffset;
                    drawRect(&imgHolder,
                             0,
                             10+row,
                             (textFont.width * strlen(str)) + 10,
                             textFont.height,
                             &textBgColor,
                             -1);
                    drawText(&imgHolder,str,5,10+row,&textFont,&textColor);
                }
            }

        }
        else if(prms->task_type == TIVX_DL_BEV_POST_PROC_DETECTION_TASK_TYPE)
        {
#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR,"Post Proc BEV OD Kernel executing\n");
#endif
            vx_float32              *output_buffer[4] = {NULL};
            vx_float32              *lidar_buffer =NULL;
            vx_int32                i;

            // NV12 drawing utils 
            Image           img;
            YUVColor        boxColor;
            YUVColor        textColor;
            YUVColor        textBGColor;
            FontProperty    textFont;

            TIDL_ODLayerHeaderInfo *pHeader;
            TIDL_3DODLayerObjInfo *pObjInfo;
            vx_uint32 numObjs;
#ifdef BEV_DEBUG_LOGS
            vx_uint32 objInfoSize;
            vx_uint32 objInfoOffset;
#endif
            getColor(&boxColor,20,220,20);
            getColor(&textColor,0,0,0);
            getColor(&textBGColor,0,255,0);
            getFont(&textFont,12);

            img.yRowAddr    = output_image_target_ptr[0];
            img.uvRowAddr   = output_image_target_ptr[1];
            img.width       = output_image_desc->width;
            img.height      = output_image_desc->height;
#ifdef BEV_DEBUG_LOGS
            uint32_t lidar_tensor_Dims = Lidar_2Camera_tensor_desc->number_of_dimensions;
            VX_PRINT(VX_ZONE_ERROR, "lidar_tensor_Dims = %d \n",lidar_tensor_Dims);
#endif

#ifdef Lidar_Drawings
            YUVColor        LidarPixelColor;
            getColor(&LidarPixelColor,255,255,255);
            lidar_buffer =(vx_float32 *)Lidar_2Camera_tensor_target_ptr;
            vx_uint32 num_lidar_points = 170000;
            for(i=0;i<num_lidar_points;i++)
            {
                vx_float32 lidar_x = *(vx_float32*)lidar_buffer;
                lidar_buffer += 1;
                vx_float32 lidar_y = *(vx_float32*)lidar_buffer;
                lidar_buffer += 2;
                if (lidar_x < 55 && lidar_x > -55 && lidar_y > -50 && lidar_y < 55)
                {
                    //scaling 110meter into 1920 width- 17
                    //Scaling 65 meter into 400 length -7
                    vx_int32        lidar_point[2];
                    lidar_point[0] = (int32_t)(lidar_y*(17.45))+(img.width/2);
                    lidar_point[1] = (int32_t)(lidar_x*(5.46))+(img.height/2);

                    if(lidar_point[0] > 0 && lidar_point[0] < 1800 && lidar_point[1] >0 && lidar_point[1] <380 )
                    {
                         drawPixel(&img, lidar_point[0], lidar_point[1],&LidarPixelColor);
                    }
                }
            }
#endif
            
            for(i = 0; i < prms->num_input_tensors; i++)
            {
                output_buffer[i] = (vx_float32 *)in_tensor_target_ptr[i];
#ifdef BEV_DEBUG_LOGS
                uint32_t nDims = in_tensor_desc[i]->number_of_dimensions;
                VX_PRINT(VX_ZONE_ERROR, "nDims = %d \n",nDims); 
#endif
            }
#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "BEV Additions \n");
#endif
            pHeader  = (TIDL_ODLayerHeaderInfo *)output_buffer[0];
            pObjInfo = (TIDL_3DODLayerObjInfo *)((uint8_t *)output_buffer[0]+ (vx_uint32)pHeader->objInfoOffset);
            numObjs  = (vx_uint32)pHeader->numDetObjects;
#ifdef BEV_DEBUG_LOGS
            objInfoSize = (vx_uint32)pHeader->objInfoSize;
            objInfoOffset = (vx_uint32)pHeader->objInfoOffset;
#endif
            vx_int32        box[8];

#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "prms->od_prms.scaleX = %f \n", prms->od_prms.scaleX);
            VX_PRINT(VX_ZONE_ERROR, "prms->od_prms.scaleY = %f \n", prms->od_prms.scaleY);
            VX_PRINT(VX_ZONE_ERROR, "num of detected Objs = %d \n", numObjs);
            VX_PRINT(VX_ZONE_ERROR, "objInfoSize = %d \n", objInfoSize);
            VX_PRINT(VX_ZONE_ERROR, "objInfoOffset = %d \n", objInfoOffset);
#endif
            for(vx_uint32 i =0; i< numObjs; i++)
            {
                
                TIDL_3DODLayerObjInfo *pDet = (TIDL_3DODLayerObjInfo *) ((uint8_t *)pObjInfo + (i * ((vx_uint32)pHeader->objInfoSize)));

                if((pDet->score >= prms->od_prms.viz_th))
                {
#ifdef BEV_DEBUG_LOGS
                    VX_PRINT(VX_ZONE_ERROR, "Detection- %d----  x = %f, y = %f, z= %f, w =%f, l = %f\n",
                                       i, pDet->x, pDet->y, pDet->z, pDet->w , (pDet->l) );
                
                    VX_PRINT(VX_ZONE_ERROR, "Detection- %d----   h = %f, yaw = %f, label = %f, score = %f\n",
                                       i, pDet->h, pDet->yaw, pDet->label, pDet->score);
#endif
                    vx_int32 x0= pDet->x  + (((pDet->w)/2)*cos(pDet->yaw))+ (((pDet->l)/2)*-sin(pDet->yaw));
                    vx_int32 y0= pDet->y + (((pDet->w)/2)*sin(pDet->yaw))+ (((pDet->l)/2)*cos(pDet->yaw));
                    //vx_int32 z0= pDet->z + (pDet->h)/2;
                    vx_int32 x1= pDet->x + ((-(pDet->w)/2)*cos(pDet->yaw))+ (((pDet->l)/2)*-sin(pDet->yaw));
                    vx_int32 y1= pDet->y + ((-(pDet->w)/2)*sin(pDet->yaw))+(((pDet->l)/2)*cos(pDet->yaw));
                    //vx_int32 z1= pDet->z + (pDet->h)/2;

                    vx_int32 x2= pDet->x + ((-(pDet->w)/2)*cos(pDet->yaw))+((-(pDet->l)/2)*-sin(pDet->yaw));
                    vx_int32 y2= pDet->y + ((-(pDet->w)/2)*sin(pDet->yaw))+((-(pDet->l)/2)*cos(pDet->yaw));
                    // vx_int32 z2= pDet->z + (pDet->h)/2;

                    vx_int32 x3= pDet->x + (((pDet->w)/2)*cos(pDet->yaw))+((-(pDet->l)/2)*-sin(pDet->yaw));
                    vx_int32 y3= pDet->y + (((pDet->w)/2)*sin(pDet->yaw))+((-(pDet->l)/2)*cos(pDet->yaw));
                    // vx_int32 z3= pDet->z + (pDet->h)/2;

                    // vx_int32 x4= pDet->x + (((pDet->w)/2)*cos(pDet->yaw))+(((pDet->l)/2)*-sin(pDet->yaw));
                    // vx_int32 y4= pDet->y + (((pDet->w)/2)*sin(pDet->yaw))+(((pDet->l)/2)*cos(pDet->yaw));
                    // vx_int32 z4= pDet->z - (pDet->h)/2; 
            
                    // vx_int32 x5= pDet->x + ((-(pDet->w)/2)*cos(pDet->yaw))+(((pDet->l)/2)*-sin(pDet->yaw));
                    // vx_int32 y5= pDet->y + ((-(pDet->w)/2)*sin(pDet->yaw))+(((pDet->l)/2)*cos(pDet->yaw));
                    // vx_int32 z5= pDet->z - (pDet->h)/2; 

                    // vx_int32 x6= pDet->x + ((-(pDet->w)/2)*cos(pDet->yaw))+((-(pDet->l)/2)*-sin(pDet->yaw));
                    // vx_int32 y6= pDet->y + ((-(pDet->w)/2)*sin(pDet->yaw))+((-(pDet->l)/2)*cos(pDet->yaw));
                    // vx_int32 z6= pDet->z - (pDet->h)/2; 
             
                    // vx_int32 x7= pDet->x + (((pDet->w)/2)*cos(pDet->yaw))+((-(pDet->l)/2)*-sin(pDet->yaw));
                    // vx_int32 y7= pDet->y + (((pDet->w)/2)*sin(pDet->yaw))+((-(pDet->l)/2)*cos(pDet->yaw));
                    // vx_int32 z7= pDet->z - (pDet->h)/2;

                    box[0] =  ((y0 * prms->od_prms.scaleX) + (img.width/2));
                    box[1] =  ((x0 * prms->od_prms.scaleY) + (img.height/2));
                    box[2] =  ((y1 * prms->od_prms.scaleX) + (img.width/2));
                    box[3] =  ((x1 * prms->od_prms.scaleY) + (img.height/2));
                    box[4] =  ((y2 * prms->od_prms.scaleX) + (img.width/2));
                    box[5] =  ((x2 * prms->od_prms.scaleY) + (img.height/2));
                    box[6] =  ((y3 * prms->od_prms.scaleX) + (img.width/2));
                    box[7] =  ((x3 * prms->od_prms.scaleY) + (img.height/2));

#ifdef BEV_DEBUG_LOGS
                    VX_PRINT(VX_ZONE_ERROR, "Detection- %d----   box[0] = %d, box[1] = %d, box[2]-box[0] = %d, box[3]-box[1] = %d\n",
                                       i, box[0], box[1], box[2]-box[0], box[3]-box[1]);
#endif

                     drawLine(&img, box[0], box[1], box[2],box[3], &boxColor, 2);
                     drawLine(&img, box[2], box[3], box[4],box[5], &boxColor, 2);
                     drawLine(&img, box[4], box[5], box[6],box[7], &boxColor, 2);
                     drawLine(&img, box[6], box[7], box[0],box[1], &boxColor, 2);

                
                }
            } 
#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "custom BEV Additions finished \n");
#endif
        }
        else if(prms->task_type == TIVX_DL_BEV_POST_PROC_SEGMENTATION_TASK_TYPE)
        {
            void                *detPtr = NULL;
            sTIDL_IOBufDesc_t   *ioBufDesc;

            ioBufDesc = (sTIDL_IOBufDesc_t *)prms->ss_prms.ioBufDesc;

            vx_int32 output_buffer_pitch  = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
            vx_int32 output_buffer_offset = output_buffer_pitch*ioBufDesc->outPadT[0] + ioBufDesc->outPadL[0];

            detPtr = (vx_uint8  *)in_tensor_target_ptr[0] + output_buffer_offset;

            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[0]);

            blendSegMask((vx_uint8  *)output_image_target_ptr[1], detPtr, prms->ss_prms.inDataWidth, prms->ss_prms.inDataHeight, output_image_desc->width,
                            output_image_desc->height, prms->ss_prms.alpha, prms->ss_prms.YUVColorMap, prms->ss_prms.MaxColorClass, data_type);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid task type for post process\n");
        }

        for(i = 0; i < prms->num_input_tensors; i++)
        {
            tivxMemBufferUnmap(in_tensor_target_ptr[i], in_tensor_desc[i]->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(Lidar_2Camera_tensor_target_ptr, Lidar_2Camera_tensor_desc->mem_size , VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

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

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    
    VX_PRINT(VX_ZONE_INFO, "Completed Process Kernel\n");
    return (status);
}

void tivxAddTargetKernelDrawBevBoxDetections()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];

    strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
    status = (vx_status)VX_SUCCESS;

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_DrawBevBoxDetections_kernel = tivxAddTargetKernelByName
                                (
                                    TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_NAME,
                                    target_name,
                                    tivxKernelDrawBevBoxDetectionsProcess,
                                    tivxKernelDrawBevBoxDetectionsCreate,
                                    tivxKernelDrawBevBoxDetectionsDelete,
                                    NULL,
                                    NULL
                                );
    }
}

void tivxRemoveTargetKernelDrawBevBoxDetections()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_DrawBevBoxDetections_kernel);
    if (status == VX_SUCCESS)
    {
        vx_DrawBevBoxDetections_kernel = NULL;
    }
}
