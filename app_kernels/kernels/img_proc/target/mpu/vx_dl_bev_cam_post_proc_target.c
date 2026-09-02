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

// include <app_tiovx_img_proc.h>
#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_target_utils.h>
#include <vx_nv12_drawing_utils.h>
#include <string.h>
#include <tivx_dl_bev_cam_post_proc_host.h>

// #define BEV_DEBUG_LOGS

#define Lidar_to_Camera_matrix_read

#ifdef Lidar_to_Camera_matrix_read
// Define a structure for a 4x1 matrix (vector)
typedef struct
{
    vx_float32 data[4];
} Vector4;

// Define a structure for a 3x3 matrix
typedef struct
{
    double data[3][3];
} Matrix3x3;

// Define a structure for a 4x4 matrix
typedef struct
{
    double data[4][4];
} Matrix4x4;

// Define a structure for a 3D box (corners in LiDAR space)
typedef struct
{
    Vector4 corners[8]; // 8 corners of the box
} Box3D;

// Define a structure for a 2D point (corner)
typedef struct
{
    double x;
    double y;
} Point;

// Function to perform matrix-vector multiplication (4x4 * 4x1)
Vector4 matrixVectorMultiply(Matrix4x4 matrix, Vector4 vector)
{
    Vector4 result;
    for (int i = 0; i < 4; i++)
    {
        result.data[i] = 0.0;
        for (int j = 0; j < 4; j++)
        {
            result.data[i] += matrix.data[i][j] * vector.data[j];
        }
    }
    return result;
}

// Function to perform matrix-vector multiplication (3x3 * 3x1)
Vector4 matrixVectorMultiply3x3(Matrix3x3 matrix, Vector4 vector)
{
    Vector4 result;
    for (int i = 0; i < 3; i++)
    {
        result.data[i] = 0.0;
        for (int j = 0; j < 3; j++)
        {
            result.data[i] += matrix.data[i][j] * vector.data[j];
        }
    }
    result.data[3] = vector.data[3]; // Keep the original w value
    return result;
}

Point *adjust_edge_in_the_img(Image *img, Vector4 image_corners[], vx_int32 index_edge1, vx_int32 index_edge2)
{
    if (image_corners[index_edge1].data[2] < 1e-5 && image_corners[index_edge2].data[2] < 1e-5)
    {
        return NULL;
    }
    if (image_corners[index_edge1].data[2] > 1e5 && image_corners[index_edge2].data[2] > 1e5)
    {
        return NULL;
    }

    if ((image_corners[index_edge1].data[0] < 0 && image_corners[index_edge2].data[0] < 0) || (image_corners[index_edge1].data[0] > img->width && image_corners[index_edge2].data[0] > img->width) || (image_corners[index_edge1].data[1] < 0 && image_corners[index_edge2].data[1] < 0) || (image_corners[index_edge1].data[1] > img->height && image_corners[index_edge2].data[1] > img->height))
    {
        return NULL;
    }

    if ((image_corners[index_edge1].data[0] >= 0 && image_corners[index_edge2].data[0] >= 0) || (image_corners[index_edge1].data[0] <= img->width && image_corners[index_edge2].data[0] <= img->width) || (image_corners[index_edge1].data[1] >= 0 && image_corners[index_edge2].data[1] >= 0) || (image_corners[index_edge1].data[1] <= img->height && image_corners[index_edge2].data[1] <= img->height))
    {
        Point *result = (Point *)malloc(2 * sizeof(Point));
        if (result == NULL)
        {
            printf("malloc failed");
            return NULL;
        }
        result[0].x = (int)image_corners[index_edge1].data[0];
        result[0].y = (int)image_corners[index_edge1].data[1];
        result[1].x = (int)image_corners[index_edge2].data[0];
        result[1].y = (int)image_corners[index_edge2].data[1];
        return result;
    }

    double slope = (image_corners[index_edge2].data[1] - image_corners[index_edge1].data[1]) / (image_corners[index_edge2].data[0] - image_corners[index_edge1].data[0]);

    if (image_corners[index_edge2].data[0] < 0)
    {
        image_corners[index_edge2].data[1] = slope * (0 - image_corners[index_edge2].data[0]) + image_corners[index_edge2].data[1];
        image_corners[index_edge2].data[0] = 0;
    }
    if (image_corners[index_edge2].data[0] > img->width)
    {
        image_corners[index_edge2].data[1] = slope * (img->width - image_corners[index_edge2].data[0]) + image_corners[index_edge2].data[1];
        image_corners[index_edge2].data[0] = img->width;
    }
    if (image_corners[index_edge2].data[1] < 0)
    {
        image_corners[index_edge2].data[0] = (0 - image_corners[index_edge2].data[1]) / slope + image_corners[index_edge2].data[0];
        image_corners[index_edge2].data[1] = 0;
    }
    if (image_corners[index_edge2].data[1] > img->height)
    {
        image_corners[index_edge2].data[0] = (img->height - image_corners[index_edge2].data[1]) / slope + image_corners[index_edge2].data[0];
        image_corners[index_edge2].data[1] = img->height;
    }
    if (image_corners[index_edge1].data[0] < 0)
    {
        image_corners[index_edge1].data[1] = slope * (0 - image_corners[index_edge1].data[0]) + image_corners[index_edge1].data[1];
        image_corners[index_edge1].data[0] = 0;
    }
    if (image_corners[index_edge1].data[0] > img->width)
    {
        image_corners[index_edge1].data[1] = slope * (img->width - image_corners[index_edge1].data[0]) + image_corners[index_edge1].data[1];
        image_corners[index_edge1].data[0] = img->width;
    }
    if (image_corners[index_edge1].data[1] < 0)
    {
        image_corners[index_edge1].data[0] = (0 - image_corners[index_edge1].data[1]) / slope + image_corners[index_edge1].data[0];
        image_corners[index_edge1].data[1] = 0;
    }
    if (image_corners[index_edge1].data[1] > img->height)
    {
        image_corners[index_edge1].data[0] = (img->height - image_corners[index_edge1].data[1]) / slope + image_corners[index_edge1].data[0];
        image_corners[index_edge1].data[1] = img->height;
    }

    Point *result = (Point *)malloc(2 * sizeof(Point));
    if (result == NULL)
    {
        perror("malloc failed");
        return NULL;
    }

    result[0].x = (int)image_corners[index_edge1].data[0];
    result[0].y = (int)image_corners[index_edge1].data[1];
    result[1].x = (int)image_corners[index_edge2].data[0];
    result[1].y = (int)image_corners[index_edge2].data[1];

    return result;
}

#endif

static tivx_target_kernel vx_DrawBevCamBoxDetections_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDrawBevCamBoxDetectionsCreate(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDrawBevCamBoxDetectionsDelete(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i++)
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
    else if (tidl_type == TIDL_SignedChar)
    {
        openvx_type = VX_TYPE_INT8;
    }
    else if (tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    }
    else if (tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if (tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if (tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if (tidl_type == TIDL_SinglePrecFloat)
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


static void blendSegMask(vx_uint8 *frame_uv_ptr,
                         void *classes,
                         int32_t inDataWidth,
                         int32_t inDataHeight,
                         int32_t outDataWidth,
                         int32_t outDataHeight,
                         float alpha,
                         uint8_t **colorMap,
                         uint8_t maxClass,
                         vx_size data_type)
{
    uint8_t a;
    uint8_t sa;
    uint8_t u_m;
    uint8_t v_m;
    int32_t w;
    int32_t h;
    int32_t sw;
    int32_t sh;
    int32_t class_id;
    int32_t rowOffset;

    a = alpha * 256;
    sa = (1 - alpha) * 256;

    // Here, (w, h) iterate over frame and (sw, sh) iterate over classes
    for (h = 0; h < outDataHeight / 2; h++)
    {
        sh = (int32_t)((h << 1) * inDataHeight / outDataHeight);
        rowOffset = sh * inDataWidth;

        for (w = 0; w < outDataWidth; w += 2)
        {
            int32_t index;

            sw = (int32_t)(w * inDataWidth / outDataWidth);
            index = (int32_t)(rowOffset + sw);

            if (data_type == VX_TYPE_UINT8)
            {
                class_id = *((uint8_t *)classes + index);
            }
            else if (data_type == VX_TYPE_INT8)
            {
                class_id = *((int8_t *)classes + index);
            }
            else if (data_type == VX_TYPE_UINT16)
            {
                class_id = *((uint16_t *)classes + index);
            }
            else if (data_type == VX_TYPE_INT16)
            {
                class_id = *((int16_t *)classes + index);
            }
            else if (data_type == VX_TYPE_UINT32)
            {
                class_id = *((uint32_t *)classes + index);
            }
            else if (data_type == VX_TYPE_INT32)
            {
                class_id = *((int32_t *)classes + index);
            }
            else if (data_type == VX_TYPE_FLOAT32)
            {
                class_id = *((float *)classes + index);
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
            u_m = ((*(frame_uv_ptr)*a) + (u_m * sa)) >> 8;
            v_m = ((*(frame_uv_ptr + 1) * a) + (v_m * sa)) >> 8;
            *((uint16_t *)frame_uv_ptr) = (v_m << 8) | u_m;
            frame_uv_ptr += 2;
        }
    }
}

static vx_status VX_CALLBACK tivxKernelDrawBevCamBoxDetectionsProcess(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    VX_PRINT(VX_ZONE_INFO, "Started Process Kernel\n");
    // tivx_set_debug_zone(VX_ZONE_INFO);

    for (i = 0U; i < num_params; i++)
    {
        if (NULL == obj_desc[i])
        {
            VX_PRINT(VX_ZONE_ERROR, "Object descriptor %d is NULL!\n", i);
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_tensor_t *in_tensor_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_MAX_INPUTS];
        void *in_tensor_target_ptr[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_MAX_INPUTS];

        for (i = 0; i < TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_MAX_INPUTS; i++)
        {
            in_tensor_target_ptr[i] = NULL;
            in_tensor_desc[i] = NULL;
        }

        tivx_obj_desc_image_t *input_image_desc;
        void *input_image_target_ptr[2];

        input_image_target_ptr[0] = NULL;
        input_image_target_ptr[1] = NULL;

        tivx_obj_desc_image_t *output_image_desc;
        void *output_image_target_ptr[2];

        tivx_obj_desc_tensor_t *Lidar_2Camera_tensor_desc;
        void *Lidar_2Camera_tensor_target_ptr = NULL;

        tivx_obj_desc_tensor_t *Cam_2Img_tensor_desc;
        void *Cam_2Img_tensor_target_ptr = NULL;

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        tivx_obj_desc_user_data_object_t *config_desc;
        void *config_target_ptr = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_CONFIGURATION_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxDLBEVPostProcParams *prms = (tivxDLBEVPostProcParams *)config_target_ptr;

        for (i = 0; i < prms->num_input_tensors; i++)
        {
            in_tensor_desc[i] = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_INPUT_TENSOR_START_IDX + i];
            in_tensor_target_ptr[i] = tivxMemShared2TargetPtr(&in_tensor_desc[i]->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr[i], in_tensor_desc[i]->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        Lidar_2Camera_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_LIDAR_2_IMG_TENSOR_IDX];
        Lidar_2Camera_tensor_target_ptr = tivxMemShared2TargetPtr(&Lidar_2Camera_tensor_desc->mem_ptr);
        tivxMemBufferMap(Lidar_2Camera_tensor_target_ptr, Lidar_2Camera_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        Cam_2Img_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_CAM_2_IMG_TENSOR_IDX];
        Cam_2Img_tensor_target_ptr = tivxMemShared2TargetPtr(&Cam_2Img_tensor_desc->mem_ptr);
        tivxMemBufferMap(Cam_2Img_tensor_target_ptr, Cam_2Img_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_OUTPUT_IMAGE_IDX];
        output_image_target_ptr[0] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[0]);
        tivxMemBufferMap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if (output_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            output_image_target_ptr[1] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[1]);
            tivxMemBufferMap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }

        if (output_image_desc->mem_size[0] == input_image_desc->mem_size[0])
        {
            memcpy(output_image_target_ptr[0], input_image_target_ptr[0], output_image_desc->mem_size[0]);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "OUtput_mem_size != input_mem_size, \n");
        }

        if (output_image_desc->mem_size[1] == input_image_desc->mem_size[1])
        {
            memcpy(output_image_target_ptr[1], input_image_target_ptr[1], output_image_desc->mem_size[1]);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "OUtput_mem_size != input_mem_size, \n");
        }

        if (prms->task_type == TIVX_DL_BEV_POST_PROC_CLASSIFICATION_TASK_TYPE)
        {
            sTIDL_IOBufDesc_t *ioBufDesc = (sTIDL_IOBufDesc_t *)prms->oc_prms.ioBufDesc;

            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[0]);
            vx_uint32 classid[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
            vx_int32 i, j;

            for (i = 0; i < TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES; i++)
            {
                classid[i] = (vx_uint32)-1;
            }

            if (data_type == VX_TYPE_FLOAT32)
            {
                vx_float32 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_float32 *pIn;

                pIn = (vx_float32 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for (i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = -FLT_MAX;
                    classid[i] = (vx_uint32)-1;

                    if (prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for (; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if (pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if (classid[i] < ioBufDesc->outWidth[0])
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
            else if (data_type == VX_TYPE_INT16)
            {
                vx_int16 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_int16 *pIn;

                pIn = (vx_int16 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for (i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = INT16_MIN;
                    classid[i] = (vx_uint32)-1;

                    if (prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for (j = 0; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if (pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if (classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = INT16_MIN;
                    }
                    else
                    {
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }
                }
            }
            else if (data_type == VX_TYPE_INT8)
            {
                vx_int8 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_int8 *pIn;

                pIn = (vx_int8 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for (i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = INT8_MIN;
                    classid[i] = (vx_uint32)-1;

                    if (prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for (j = 0; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if (pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if (classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = INT8_MIN;
                    }
                    else
                    {
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }
                }
            }
            else if (data_type == VX_TYPE_UINT8)
            {
                vx_uint8 score[TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES];
                vx_uint8 *pIn;

                pIn = (vx_uint8 *)in_tensor_target_ptr[0] + (ioBufDesc->outPadT[0] * in_tensor_desc[0]->dimensions[0]) + ioBufDesc->outPadL[0];

                for (i = 0; i < prms->oc_prms.num_top_results; i++)
                {
                    score[i] = 0;
                    classid[i] = (vx_uint32)-1;

                    if (prms->oc_prms.labelOffset == 0)
                    {
                        j = 0;
                    }
                    else
                    {
                        j = 1;
                    }

                    for (j = 0; j < ioBufDesc->outWidth[0]; j++)
                    {
                        if (pIn[j] > score[i])
                        {
                            score[i] = pIn[j];
                            classid[i] = j;
                        }
                    }
                    if (classid[i] < ioBufDesc->outWidth[0])
                    {
                        pIn[classid[i]] = 0;
                    }
                    else
                    {
                        classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                    }
                }
            }

            Image imgHolder;
            YUVColor titleColor;
            FontProperty titleFont;
            YUVColor textColor;
            FontProperty textFont;
            YUVColor textBgColor;
            char title[40];

            /** Get YUV value for green color. */
            getColor(&titleColor, 0, 255, 0);

            /** Get YUV value for yellow color. */
            getColor(&textColor, 255, 255, 0);

            /** Get YUV value for dark blue color. */
            getColor(&textBgColor, 5, 11, 120);

            /** Get Monospace font from available font sizes
             *  where width of character is closest to 1.5%
             *  of the total width image width
             */
            int titleSize = (int)(0.015 * output_image_desc->width);
            getFont(&titleFont, titleSize);

            /** Get Monospace font from available font sizes
             *  where width of character is closest to 1.5%
             *  of the total width image width
             */
            int textSize = (int)(0.015 * output_image_desc->width);
            getFont(&textFont, textSize);

            imgHolder.yRowAddr = (uint8_t *)output_image_target_ptr[0];
            imgHolder.uvRowAddr = (uint8_t *)output_image_target_ptr[1];
            imgHolder.width = output_image_desc->width;
            imgHolder.height = output_image_desc->height;

            snprintf(title, sizeof(title), "Recognized Classes (Top %d) : \n", prms->oc_prms.num_top_results);

            int yOffset = (titleFont.height) + 12;

            drawRect(&imgHolder,
                     0,
                     10,
                     (titleFont.width * strlen(title)) + 10,
                     yOffset,
                     &textBgColor,
                     -1);

            drawText(&imgHolder, title, 5, 10, &titleFont, &titleColor);

            for (int i = 0; i < prms->oc_prms.num_top_results; i++)
            {
                VX_PRINT(VX_ZONE_INFO, "classid[i] = %d, prms->oc_prms.labelOffset = %d \n", classid[i], prms->oc_prms.labelOffset);
                int32_t index = classid[i] + prms->oc_prms.labelOffset;

                if (index >= 0)
                {
                    char *str = prms->oc_prms.classnames[index];
                    int32_t row = (i * textFont.height) + yOffset;
                    drawRect(&imgHolder,
                             0,
                             10 + row,
                             (textFont.width * strlen(str)) + 10,
                             textFont.height,
                             &textBgColor,
                             -1);
                    drawText(&imgHolder, str, 5, 10 + row, &textFont, &textColor);
                }
            }
        }
        else if (prms->task_type == TIVX_DL_BEV_POST_PROC_DETECTION_TASK_TYPE)
        {
#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "Post Proc BEV OD Kernel executing\n");
#endif
            vx_float32 *output_buffer[4] = {NULL};
            // vx_float32              *lidar_buffer =NULL;
            // vx_uint32               firstDims[4] = {0};
            vx_int32 i;

            // NV12 drawing utils
            Image img;
            YUVColor boxColor;
            YUVColor textColor;
            YUVColor textBGColor;
            FontProperty textFont;

            TIDL_ODLayerHeaderInfo *pHeader;
            TIDL_3DODLayerObjInfo *pObjInfo;
            vx_uint32 numObjs;
#ifdef BEV_DEBUG_LOGS
            vx_uint32 objInfoSize;
            vx_uint32 objInfoOffset;
#endif
            getColor(&boxColor, 20, 220, 20);
            getColor(&textColor, 0, 0, 0);
            getColor(&textBGColor, 0, 255, 0);
            getFont(&textFont, 12);

            img.yRowAddr = output_image_target_ptr[0];
            img.uvRowAddr = output_image_target_ptr[1];
            img.width = output_image_desc->width;
            img.height = output_image_desc->height;
#ifdef BEV_DEBUG_LOGS
            uint32_t lidar_tensor_Dims = Lidar_2Camera_tensor_desc->number_of_dimensions;
            VX_PRINT(VX_ZONE_ERROR, "lidar_tensor_Dims = %d \n", lidar_tensor_Dims);
#endif

#ifdef Lidar_to_Camera_matrix_read
            vx_float32 *lidar_2Cam_buffer = NULL;
            vx_float32 *Cam_2Img_buffer = NULL;
            Matrix4x4 T_lidar_to_camera = {
                {{0.0, 0.00, 0.00, 0.0},
                 {0.00, 0.00, 0.00, 0.0},
                 {0.00, 0.00, 0.00, 0.0},
                 {0.0, 0.0, 0.0, 0.0}}};
            // This should be read from application level to kernel.
            Matrix3x3 Camera_intrinsics = {
                {{0.0, 0.0, 0.0},
                 {0.0, 0.0, 0.0},
                 {0.0, 0.0, 0.0}}};
            vx_int32 j;
            lidar_2Cam_buffer = (vx_float32 *)Lidar_2Camera_tensor_target_ptr;

            vx_uint32 lidar_to_camera_t_points = 16; // MAtrix will be 4*4
            for (i = 0; i < (lidar_to_camera_t_points / 4); i++)
            {
                for (j = 0; j < (lidar_to_camera_t_points / 4); j++)
                {
                    T_lidar_to_camera.data[i][j] = *(vx_float32 *)lidar_2Cam_buffer;
                    lidar_2Cam_buffer++;
                }
            }
            Cam_2Img_buffer = (vx_float32 *)Cam_2Img_tensor_target_ptr;
            vx_uint32 camera_to_image_t_points = 9; // MAtrix will be 3*3
            for (i = 0; i < (camera_to_image_t_points / 3); i++)
            {
                for (j = 0; j < (camera_to_image_t_points / 3); j++)
                {
                    Camera_intrinsics.data[i][j] = *(vx_float32 *)Cam_2Img_buffer;
                    // printf("Camera_intrinsics.data[%d][%d]-%f\n",i,j,Camera_intrinsics.data[i][j]);
                    Cam_2Img_buffer++;
                }
            }

#endif

            for (i = 0; i < prms->num_input_tensors; i++)
            {
                output_buffer[i] = (vx_float32 *)in_tensor_target_ptr[i];
#ifdef BEV_DEBUG_LOGS
                uint32_t nDims = in_tensor_desc[i]->number_of_dimensions;
                VX_PRINT(VX_ZONE_ERROR, "nDims = %d \n", nDims);
#endif
            }
#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "BEV Additions \n");
#endif
            pHeader = (TIDL_ODLayerHeaderInfo *)output_buffer[0];
            pObjInfo = (TIDL_3DODLayerObjInfo *)((uint8_t *)output_buffer[0] + (vx_uint32)pHeader->objInfoOffset);
            numObjs = (vx_uint32)pHeader->numDetObjects;
#ifdef BEV_DEBUG_LOGS
            objInfoSize = (vx_uint32)pHeader->objInfoSize;
            objInfoOffset = (vx_uint32)pHeader->objInfoOffset;
#endif
            Box3D box;

#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "numEntries = %d \n", numEntries);
            VX_PRINT(VX_ZONE_ERROR, "num of detected Objs = %d \n", numObjs);
            VX_PRINT(VX_ZONE_ERROR, "objInfoSize = %d \n", objInfoSize);
            VX_PRINT(VX_ZONE_ERROR, "objInfoOffset = %d \n", objInfoOffset);
#endif
            for (vx_uint32 i = 0; i < numObjs; i++)
            {

                TIDL_3DODLayerObjInfo *pDet = (TIDL_3DODLayerObjInfo *)((uint8_t *)pObjInfo + (i * ((vx_uint32)pHeader->objInfoSize)));

                if ((pDet->score >= prms->od_prms.viz_th))
                {
#ifdef BEV_DEBUG_LOGS
                    VX_PRINT(VX_ZONE_ERROR, "Detection- %d----  x = %f, y = %f, z= %f, w =%f, l = %f\n",
                             i, pDet->x, pDet->y, pDet->z, pDet->w, pDet->l);

                    VX_PRINT(VX_ZONE_ERROR, "Detection- %d----   h = %f, yaw = %f, label = %f, score = %f\n",
                             i, pDet->h, pDet->yaw, pDet->label, pDet->score);
#endif
                    box.corners[0].data[0] = pDet->x + (((pDet->w) / 2) * cos(pDet->yaw)) + (((pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[0].data[1] = pDet->y + (((pDet->w) / 2) * sin(pDet->yaw)) + (((pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[0].data[2] = pDet->z + (pDet->h);
                    box.corners[0].data[3] = 1;

                    box.corners[1].data[0] = pDet->x + ((-(pDet->w) / 2) * cos(pDet->yaw)) + (((pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[1].data[1] = pDet->y + ((-(pDet->w) / 2) * sin(pDet->yaw)) + (((pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[1].data[2] = pDet->z + (pDet->h);
                    box.corners[1].data[3] = 1;

                    box.corners[2].data[0] = pDet->x + ((-(pDet->w) / 2) * cos(pDet->yaw)) + ((-(pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[2].data[1] = pDet->y + ((-(pDet->w) / 2) * sin(pDet->yaw)) + ((-(pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[2].data[2] = pDet->z + (pDet->h);
                    box.corners[2].data[3] = 1;

                    box.corners[3].data[0] = pDet->x + (((pDet->w) / 2) * cos(pDet->yaw)) + ((-(pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[3].data[1] = pDet->y + (((pDet->w) / 2) * sin(pDet->yaw)) + ((-(pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[3].data[2] = pDet->z + (pDet->h);
                    box.corners[3].data[3] = 1;

                    box.corners[4].data[0] = pDet->x + (((pDet->w) / 2) * cos(pDet->yaw)) + (((pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[4].data[1] = pDet->y + (((pDet->w) / 2) * sin(pDet->yaw)) + (((pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[4].data[2] = pDet->z ;
                    box.corners[4].data[3] = 1;

                    box.corners[5].data[0] = pDet->x + ((-(pDet->w) / 2) * cos(pDet->yaw)) + (((pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[5].data[1] = pDet->y + ((-(pDet->w) / 2) * sin(pDet->yaw)) + (((pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[5].data[2] = pDet->z ;
                    box.corners[5].data[3] = 1;

                    box.corners[6].data[0] = pDet->x + ((-(pDet->w) / 2) * cos(pDet->yaw)) + ((-(pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[6].data[1] = pDet->y + ((-(pDet->w) / 2) * sin(pDet->yaw)) + ((-(pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[6].data[2] = pDet->z ;
                    box.corners[6].data[3] = 1;

                    box.corners[7].data[0] = pDet->x + (((pDet->w) / 2) * cos(pDet->yaw)) + ((-(pDet->l) / 2) * -sin(pDet->yaw));
                    box.corners[7].data[1] = pDet->y + (((pDet->w) / 2) * sin(pDet->yaw)) + ((-(pDet->l) / 2) * cos(pDet->yaw));
                    box.corners[7].data[2] = pDet->z ;
                    box.corners[7].data[3] = 1;

#ifdef Lidar_to_Camera_matrix_read
                    Vector4 image_corners[8];
                    vx_int32 count = 0;
                    for (vx_int32 j = 0; j < 8; j++)
                    {
                        Vector4 camera_corner = matrixVectorMultiply(T_lidar_to_camera, box.corners[j]);
                        image_corners[j] = matrixVectorMultiply3x3(Camera_intrinsics, camera_corner);
                        image_corners[j].data[0] = image_corners[j].data[0] / camera_corner.data[2];
                        image_corners[j].data[1] = image_corners[j].data[1] / camera_corner.data[2];

                        if (((image_corners[j].data[0] > 0) && (image_corners[j].data[0] < img.width)))
                        {
                            if(((image_corners[j].data[1] > 0) && (image_corners[j].data[1] < img.height)))
                            {
                                if((image_corners[j].data[2] >= 0))
                                    {
                                        count++;
                                    VX_PRINT(VX_ZONE_INFO, "Detection_Box - %d--Cord-%d  box_corners[j].data[0] = %f,  box_corners[j].data[1]= %f, box_corners[j].data[1] = %f,  \n",
                                        i, j, box.corners[j].data[0], box.corners[j].data[1], box.corners[j].data[2]);

                                    VX_PRINT(VX_ZONE_INFO, "Detection_Box - %d--Cord-%d  image_corners[j].data[0] = %f,  image_corners[j].data[1]= %f, Depth = %f, \n",
                                        i, j, image_corners[j].data[0], image_corners[j].data[1], image_corners[j].data[2]);
                                    
                                    }
                            }
                        }
                    }

                    if (count >= 4)
                    {
                        vx_int32 edges[12][2] = {
                            {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
                        for (vx_int32 m = 0; m < 12; m++)
                        {
                            vx_int32 index_edge1 = edges[m][0];
                            vx_int32 index_edge2 = edges[m][1];

                            /* If want to project truncated line use below function*/
                            /*Point* adjusted_corners = adjust_edge_in_the_img(&img, image_corners,index_edge1,index_edge2);
                            if (adjusted_corners != NULL) {
                                drawLine(&img, adjusted_corners[index_edge1].x, adjusted_corners[index_edge1].y, adjusted_corners[index_edge2].x,adjusted_corners[index_edge2].y, &boxColor, 2);
                                free(adjusted_corners);
                            }*/
                            if((image_corners[index_edge1].data[2] >= 0) && (image_corners[index_edge2].data[2] >= 0))
                            {
                                if ((image_corners[index_edge1].data[0] >= 0 || image_corners[index_edge2].data[0] >= 0) && (image_corners[index_edge1].data[0] <= img.width || image_corners[index_edge2].data[0] > img.width) && (image_corners[index_edge1].data[1] >= 0 || image_corners[index_edge2].data[1] >= 0) && (image_corners[index_edge1].data[1] <= img.height || image_corners[index_edge2].data[1] <= img.height))
                                {
                                    if ((image_corners[index_edge1].data[0] >= 0 && image_corners[index_edge2].data[0] >= 0) && (image_corners[index_edge1].data[0] <= img.width && image_corners[index_edge2].data[0] <= img.width) && (image_corners[index_edge1].data[1] >= 0 && image_corners[index_edge2].data[1] >= 0) && (image_corners[index_edge1].data[1] <= img.height && image_corners[index_edge2].data[1] <= img.height))
                                    {
                                        drawLine(&img, image_corners[index_edge1].data[0], image_corners[index_edge1].data[1], image_corners[index_edge2].data[0], image_corners[index_edge2].data[1], &boxColor, 2);
                                    }
                                }
                            }
                        }
                    }
#endif

#ifdef BEV_DEBUG_LOGS
                    VX_PRINT(VX_ZONE_ERROR, "Detection- %d----   box[0] = %d, box[1] = %d, box[2]-box[0] = %d, box[3]-box[1] = %d\n",
                             i, box[0], box[1], box[2] - box[0], box[3] - box[1]);
#endif
                }
            }
#ifdef BEV_DEBUG_LOGS
            VX_PRINT(VX_ZONE_ERROR, "custom BEV Additions finished \n");
#endif
        }
        else if (prms->task_type == TIVX_DL_BEV_POST_PROC_SEGMENTATION_TASK_TYPE)
        {
            void *detPtr = NULL;
            sTIDL_IOBufDesc_t *ioBufDesc;

            ioBufDesc = (sTIDL_IOBufDesc_t *)prms->ss_prms.ioBufDesc;

            vx_int32 output_buffer_pitch = ioBufDesc->outWidth[0] + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
            vx_int32 output_buffer_offset = output_buffer_pitch * ioBufDesc->outPadT[0] + ioBufDesc->outPadL[0];

            detPtr = (vx_uint8 *)in_tensor_target_ptr[0] + output_buffer_offset;

            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[0]);

            blendSegMask((vx_uint8 *)output_image_target_ptr[1], detPtr, prms->ss_prms.inDataWidth, prms->ss_prms.inDataHeight, output_image_desc->width,
                         output_image_desc->height, prms->ss_prms.alpha, prms->ss_prms.YUVColorMap, prms->ss_prms.MaxColorClass, data_type);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid task type for post process\n");
        }

        for (i = 0; i < prms->num_input_tensors; i++)
        {
            tivxMemBufferUnmap(in_tensor_target_ptr[i], in_tensor_desc[i]->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(Lidar_2Camera_tensor_target_ptr, Lidar_2Camera_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(Cam_2Img_tensor_target_ptr, Cam_2Img_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (input_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        tivxMemBufferUnmap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if (output_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    // tivx_clr_debug_zone(VX_ZONE_INFO);
    VX_PRINT(VX_ZONE_INFO, "Completed Process Kernel\n");
    return (status);
}

void tivxAddTargetKernelDrawBevCamBoxDetections()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];

    strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
    status = (vx_status)VX_SUCCESS;

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_DrawBevCamBoxDetections_kernel = tivxAddTargetKernelByName(
            TIVX_KERNEL_DRAW_BEV_CAM_BOX_DETECTIONS_NAME,
            target_name,
            tivxKernelDrawBevCamBoxDetectionsProcess,
            tivxKernelDrawBevCamBoxDetectionsCreate,
            tivxKernelDrawBevCamBoxDetectionsDelete,
            NULL,
            NULL);
    }
}

void tivxRemoveTargetKernelDrawBevCamBoxDetections()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_DrawBevCamBoxDetections_kernel);
    if (status == VX_SUCCESS)
    {
        vx_DrawBevCamBoxDetections_kernel = NULL;
    }
}
