/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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

#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_dl_post_proc_host.h>
#include <vx_nv12_drawing_utils.h>
#include <string.h>

static tivx_target_kernel vx_dlPostProc_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDLPostProcCreate
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

static vx_status VX_CALLBACK tivxKernelDLPostProcDelete
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
static void overlayBoundingBox(Image                        *img,
                               int                          *box,
                               char                         *objectname,
                               YUVColor                     *boxColor,
                               YUVColor                     *textColor,
                               YUVColor                     *textBGColor,
                               FontProperty                 *textFont
                               )
{
    // Draw bounding box for the detected object
    drawRect(img,
             box[0],
             box[1],
             box[2] - box[0],
             box[3] - box[1],
             boxColor,
             2);

    drawRect(img,
             (box[0] + box[2])/2 - 5,
             (box[1] + box[3])/2 - 5,
             strlen(objectname) * textFont->width + 10,
             textFont->height + 10,
             textBGColor,
             -1);

    drawText(img,
             objectname,
             (box[0] + box[2])/2,
             (box[1] + box[3])/2,
             textFont,
             textColor);
}

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

static vx_float32 getVal(vx_uint32 firstDims[], vx_uint32 num_input_tensors, vx_float32 *output_buffer[], vx_int32 iter, vx_int32 pos)
{
    vx_int32 cumuDims = 0;

    for (vx_uint32 i=0; i < num_input_tensors; i++)
    {
        cumuDims += firstDims[i];

        VX_PRINT(VX_ZONE_INFO, " iter = %d, firstDims[%d] = %d, pos = %d, cumuDims = %d \n", iter, i, firstDims[i], pos, cumuDims);
        vx_int32 offset = iter * firstDims[i] + pos - cumuDims + firstDims[i];

        VX_PRINT(VX_ZONE_INFO, " i = %d, offset = %d \n", i, offset);

        if(pos < cumuDims)
        {
            return output_buffer[i][offset];
        }
    }

    return 0;
}

static vx_status VX_CALLBACK tivxKernelDLPostProcProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    // tivx_set_debug_zone(VX_ZONE_INFO);

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
        tivx_obj_desc_tensor_t *in_tensor_desc[TIVX_DL_POST_PROC_MAX_PARAMS];
        void* in_tensor_target_ptr[TIVX_DL_POST_PROC_MAX_PARAMS];

        for(i = 0; i < TIVX_DL_POST_PROC_MAX_PARAMS; i++)
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

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_DL_POST_PROC_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxDLPostProcParams *prms = (tivxDLPostProcParams *)config_target_ptr;

        for(i = 0; i < prms->num_input_tensors; i++)
        {   
            in_tensor_desc[i]  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_DL_POST_PROC_INPUT_TENSOR_START_IDX + i];
            in_tensor_target_ptr[i]  = tivxMemShared2TargetPtr(&in_tensor_desc[i]->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr[i], in_tensor_desc[i]->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_POST_PROC_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if(input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_POST_PROC_OUTPUT_IMAGE_IDX];
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

        if(output_image_desc->mem_size[1] == input_image_desc->mem_size[1])
        {
            memcpy(output_image_target_ptr[1], input_image_target_ptr[1], output_image_desc->mem_size[1]);
        }

        if(prms->task_type == TIVX_DL_POST_PROC_CLASSIFICATION_TASK_TYPE)
        {
            sTIDL_IOBufDesc_t *ioBufDesc = (sTIDL_IOBufDesc_t *)prms->oc_prms.ioBufDesc;

            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[0]);
            vx_uint32 classid[TIVX_DL_POST_PROC_OC_MAX_CLASSES];
            vx_int32 i, j;

            for(i = 0; i < TIVX_DL_POST_PROC_OC_MAX_CLASSES; i++)
            {
                classid[i] = (vx_uint32)-1;
            }

            if(data_type == VX_TYPE_FLOAT32)
            {
                vx_float32 score[TIVX_DL_POST_PROC_OC_MAX_CLASSES];
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
                vx_int16 score[TIVX_DL_POST_PROC_OC_MAX_CLASSES];
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
                vx_int8 score[TIVX_DL_POST_PROC_OC_MAX_CLASSES];
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
                vx_uint8 score[TIVX_DL_POST_PROC_OC_MAX_CLASSES];
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
        else if(prms->task_type == TIVX_DL_POST_PROC_DETECTION_TASK_TYPE)
        {
            vx_float32              *output_buffer[4] = {NULL};
            vx_uint32               firstDims[4] = {0};
            vx_int32                i, j;

            /* NV12 drawing utils */
            Image           img;
            YUVColor        boxColor;
            YUVColor        textColor;
            YUVColor        textBGColor;
            FontProperty    textFont;

            getColor(&boxColor,20,220,20);
            getColor(&textColor,0,0,0);
            getColor(&textBGColor,0,255,0);
            getFont(&textFont,12);

            img.yRowAddr    = output_image_target_ptr[0];
            img.uvRowAddr   = output_image_target_ptr[1];
            img.width       = output_image_desc->width;
            img.height      = output_image_desc->height;

            for(i = 0; i < prms->num_input_tensors; i++)
            {
                output_buffer[i] = (vx_float32 *)in_tensor_target_ptr[i];

                uint32_t nDims = in_tensor_desc[i]->number_of_dimensions;

                for (j = 0; j < in_tensor_desc[i]->number_of_dimensions; j++)
                {
                    if(in_tensor_desc[i]->dimensions[j] == 1)
                    {
                        nDims--;
                    }
                }

                if(nDims == 1)
                {
                    firstDims[i] = 1;
                }
                else
                {
                    firstDims[i] = in_tensor_desc[i]->dimensions[0];
                }
                VX_PRINT(VX_ZONE_INFO, "firstDims[%d] = %d \n", i, firstDims[i]); 
            }

            int32_t numEntries = (in_tensor_desc[prms->num_input_tensors-1]->dimensions[0] *
                                    in_tensor_desc[prms->num_input_tensors-1]->dimensions[1] *
                                    in_tensor_desc[prms->num_input_tensors-1]->dimensions[2]) / firstDims[prms->num_input_tensors-1];

            VX_PRINT(VX_ZONE_INFO, "numEntries = %d \n", numEntries);

            for ( i = 0; i < numEntries; i++)
            {
                vx_float32      score;
                vx_int32        label;
                vx_int32        box[4];

                if(prms->num_input_tensors > (sizeof(firstDims) / sizeof(firstDims[0]))
                    || prms->num_input_tensors > (sizeof(output_buffer) / sizeof(output_buffer[0])))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Number of input tensors can not be greater than size of 4. \n");
                    return VX_FAILURE;
                }

                score = getVal(firstDims, prms->num_input_tensors, output_buffer, i, prms->od_prms.formatter[5]);

                VX_PRINT(VX_ZONE_INFO, "score = %f \n", score);

                if (score < prms->od_prms.viz_th)
                {
                    continue;
                }
                
                box[0] = getVal(firstDims, prms->num_input_tensors, output_buffer, i, prms->od_prms.formatter[0]) * prms->od_prms.scaleX;
                box[1] = getVal(firstDims, prms->num_input_tensors, output_buffer, i, prms->od_prms.formatter[1]) * prms->od_prms.scaleY;
                box[2] = getVal(firstDims, prms->num_input_tensors, output_buffer, i, prms->od_prms.formatter[2]) * prms->od_prms.scaleX;
                box[3] = getVal(firstDims, prms->num_input_tensors, output_buffer, i, prms->od_prms.formatter[3]) * prms->od_prms.scaleY;

                label = (vx_int32)getVal(firstDims, prms->num_input_tensors, output_buffer, i, prms->od_prms.formatter[4]);

                VX_PRINT(VX_ZONE_INFO, "box[0] = %d, box[1] = %d, box[2] =%d, box[3] = %d, label = %d, score = %f\n",
                                            box[0], box[1], box[2], box[3], label, score);

                VX_PRINT(VX_ZONE_INFO, " label = %d, prms->od_prms.labelIndexOffset = %d \n", label, prms->od_prms.labelIndexOffset);

                int32_t index = prms->od_prms.labelOffset[label - prms->od_prms.labelIndexOffset];
                char *classname = prms->od_prms.classnames[index];

                overlayBoundingBox(&img, box, classname, &boxColor, 
                                    &textColor, &textBGColor, &textFont);
            }
        }
        else if(prms->task_type == TIVX_DL_POST_PROC_SEGMENTATION_TASK_TYPE)
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

    // tivx_clr_debug_zone(VX_ZONE_INFO);

    return (status);
}

void tivxAddTargetKernelDLPostProc()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];

    strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
    status = (vx_status)VX_SUCCESS;

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_dlPostProc_kernel = tivxAddTargetKernelByName
                                (
                                    TIVX_KERNEL_DL_POST_PROC_NAME,
                                    target_name,
                                    tivxKernelDLPostProcProcess,
                                    tivxKernelDLPostProcCreate,
                                    tivxKernelDLPostProcDelete,
                                    NULL,
                                    NULL
                                );
    }
}

void tivxRemoveTargetKernelDLPostProc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dlPostProc_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dlPostProc_kernel = NULL;
    }
}
