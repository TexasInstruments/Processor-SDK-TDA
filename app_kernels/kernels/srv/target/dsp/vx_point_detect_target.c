/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/tivx_srv.h"
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_point_detect.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "ti/vxlib/vxlib.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"
#include "core_point_detect.h"
#include <math.h>
#include <stdio.h>
#include <utils/mem/include/app_mem.h>


#define MAX_FP_ALL                   80
#define MAX_INPUT_CAMERAS            4
#define FP_TO_DETECT                 2      


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


//#define DEBUG_1
//#define DEBUG_2
//#define DEBUG_3





static tivx_target_kernel vx_point_detect_target_kernel = NULL;

static vx_status VX_CALLBACK tivxPointDetectProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPointDetectCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPointDetectDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status tivxPointDetectAllocPrms(tivxPointDetectParams *prms, vx_int16 SVInCamFrmHeight, vx_int16 SVInCamFrmWidth);

static vx_status tivxPointDetectAllocPrms(tivxPointDetectParams *prms, vx_int16 SVInCamFrmHeight, vx_int16 SVInCamFrmWidth)
{
    vx_status status = VX_SUCCESS;
    int i, j;

    prms->buf_IntegralImg_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint32_t);
    prms->buf_IntegralImg_ptr = tivxMemAlloc(prms->buf_IntegralImg_size, TIVX_MEM_EXTERNAL_SCRATCH);

    if (NULL == prms->buf_IntegralImg_ptr)
    {
        status = VX_ERROR_NO_MEMORY;
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
    }
    else
    {
        memset(prms->buf_IntegralImg_ptr, 0, prms->buf_IntegralImg_size);
    }
    if (VX_SUCCESS == status)
    {
        prms->buf_IntegralRows_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint32_t);
        prms->buf_IntegralRows_ptr = tivxMemAlloc(prms->buf_IntegralRows_size, TIVX_MEM_EXTERNAL_SCRATCH);

        if (NULL == prms->buf_IntegralRows_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
        else
        {
            memset(prms->buf_IntegralRows_ptr, 0, prms->buf_IntegralRows_size);
        }
    }
    if (VX_SUCCESS == status)
    {
        prms->buf_grayLumaFrame_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint8_t);
        prms->buf_grayLumaFrame_ptr = tivxMemAlloc(prms->buf_grayLumaFrame_size, TIVX_MEM_EXTERNAL_SCRATCH);

        if (NULL == prms->buf_grayLumaFrame_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
        else
        {
            memset(prms->buf_grayLumaFrame_ptr, 0, prms->buf_grayLumaFrame_size);
        }
    }

    if (VX_SUCCESS == status)
    {
        prms->buf_bwLumaFrame_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint8_t);
        prms->buf_bwLumaFrame_ptr = tivxMemAlloc(prms->buf_bwLumaFrame_size, TIVX_MEM_EXTERNAL_SCRATCH);

        if (NULL == prms->buf_bwLumaFrame_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
        else
        {
            memset(prms->buf_bwLumaFrame_ptr, 0, prms->buf_bwLumaFrame_size);
        }
    }

    if (VX_SUCCESS == status)
    {
        prms->buf_candFPId_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint16_t);
        prms->buf_candFPId_ptr = tivxMemAlloc(prms->buf_candFPId_size, TIVX_MEM_EXTERNAL_SCRATCH);

        if (NULL == prms->buf_candFPId_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
        else
        {
            memset(prms->buf_candFPId_ptr, 0, prms->buf_candFPId_size);
        }
    }

            if (VX_SUCCESS == status)
            {
                prms->buf_candFPx_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint16_t);
                prms->buf_candFPx_ptr = tivxMemAlloc(prms->buf_candFPId_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_candFPx_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_candFPx_ptr, 0, prms->buf_candFPx_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_candFPy_size = SVInCamFrmHeight * SVInCamFrmWidth * sizeof(uint16_t);
                prms->buf_candFPy_ptr = tivxMemAlloc(prms->buf_candFPId_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_candFPy_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_candFPy_ptr, 0, prms->buf_candFPy_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_svFPBoundaryPosStruct_size = NUM_PT_BOUNDARY* sizeof (SV_ACDetect_FPBoundaryPos);
                prms->buf_svFPBoundaryPosStruct_ptr = tivxMemAlloc(prms->buf_svFPBoundaryPosStruct_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_svFPBoundaryPosStruct_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_svFPBoundaryPosStruct_ptr, 0, prms->buf_svFPBoundaryPosStruct_size);
                }
            }     
     
            if (VX_SUCCESS == status)
            {
                prms->buf_intOutCenter_size = MAX_FP_ALL*2* sizeof (int16_t);
                prms->buf_intOutCenter_ptr = tivxMemAlloc(prms->buf_intOutCenter_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_intOutCenter_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_intOutCenter_ptr, 0, prms->buf_intOutCenter_size);
                }
            }     

            if (VX_SUCCESS == status)
            {
                prms->buf_outCenterNum_size = MAX_FP_ALL* sizeof (int16_t);
                prms->buf_outCenterNum_ptr = tivxMemAlloc(prms->buf_outCenterNum_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_outCenterNum_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_outCenterNum_ptr, 0, prms->buf_outCenterNum_size);
                }
            }     

            if (VX_SUCCESS == status)
            {
                prms->buf_outCenter_size = MAX_FP_ALL*3* sizeof (double);
                prms->buf_outCenter_ptr = tivxMemAlloc(prms->buf_outCenter_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_outCenter_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_outCenter_ptr, 0, prms->buf_outCenter_size);
                }
            }     

            if (VX_SUCCESS == status)
            {
                prms->buf_candidCenter_size = MAX_FP_ALL*2* sizeof (int16_t);
                prms->buf_candidCenter_ptr = tivxMemAlloc(prms->buf_candidCenter_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_candidCenter_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_candidCenter_ptr, 0, prms->buf_candidCenter_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_boundX_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_boundX_ptr = tivxMemAlloc(prms->buf_boundX_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_boundX_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_boundX_ptr, 0, prms->buf_boundX_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_boundY_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_boundY_ptr = tivxMemAlloc(prms->buf_boundY_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_boundY_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_boundY_ptr, 0, prms->buf_boundY_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_tempBoundX_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_tempBoundX_ptr = tivxMemAlloc(prms->buf_tempBoundX_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_tempBoundX_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_tempBoundX_ptr, 0, prms->buf_tempBoundX_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_tempBoundY_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_tempBoundY_ptr = tivxMemAlloc(prms->buf_tempBoundY_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_tempBoundY_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_tempBoundY_ptr, 0, prms->buf_tempBoundY_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_fcBoundX_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_fcBoundX_ptr = tivxMemAlloc(prms->buf_fcBoundX_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_fcBoundX_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_fcBoundX_ptr, 0, prms->buf_fcBoundX_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_fcBoundY_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_fcBoundY_ptr = tivxMemAlloc(prms->buf_fcBoundY_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_fcBoundY_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_fcBoundY_ptr, 0, prms->buf_fcBoundY_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_tempCorner_size = 8* sizeof (double);
                prms->buf_tempCorner_ptr = tivxMemAlloc(prms->buf_tempCorner_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_tempCorner_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_tempCorner_ptr, 0, prms->buf_tempCorner_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_tempCorner1_size = 8* sizeof (double);
                prms->buf_tempCorner1_ptr = tivxMemAlloc(prms->buf_tempCorner1_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_tempCorner1_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_tempCorner1_ptr, 0, prms->buf_tempCorner1_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_tempCorner2_size = 8* sizeof (double);
                prms->buf_tempCorner2_ptr = tivxMemAlloc(prms->buf_tempCorner2_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_tempCorner2_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_tempCorner2_ptr, 0, prms->buf_tempCorner2_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_line1PtsX_size = NUM_PT_BOUNDARY* sizeof (double);
                prms->buf_line1PtsX_ptr = tivxMemAlloc(prms->buf_line1PtsX_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_line1PtsX_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_line1PtsX_ptr, 0, prms->buf_line1PtsX_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_line1PtsY_size = NUM_PT_BOUNDARY * sizeof (double);
                prms->buf_line1PtsY_ptr = tivxMemAlloc(prms->buf_line1PtsY_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_line1PtsY_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_line1PtsY_ptr, 0, prms->buf_line1PtsY_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_line2PtsX_size = NUM_PT_BOUNDARY * sizeof (double);
                prms->buf_line2PtsX_ptr = tivxMemAlloc(prms->buf_line2PtsX_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_line2PtsX_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_line2PtsX_ptr, 0, prms->buf_line2PtsX_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_line2PtsY_size = NUM_PT_BOUNDARY * sizeof (double);
                prms->buf_line2PtsY_ptr = tivxMemAlloc(prms->buf_line2PtsY_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_line2PtsY_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_line2PtsY_ptr, 0, prms->buf_line2PtsY_size);
                }
            }

            for (i=0;i<MAX_INPUT_CAMERAS;i++) {
                for (j=0; j<FP_TO_DETECT;j++ ){
                    if (VX_SUCCESS == status)
                    {
                        prms->buf_finalCorners_size = 8 * sizeof (int32_t) ;
                        prms->buf_finalCorners_ptr[i][j] = tivxMemAlloc(prms->buf_finalCorners_size, TIVX_MEM_EXTERNAL_SCRATCH);

                        if (NULL == prms->buf_finalCorners_ptr[i][j])
                         {
                             status = VX_ERROR_NO_MEMORY;
                             VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                         }
                         else
                         {
                             memset(prms->buf_finalCorners_ptr[i][j], 0, prms->buf_finalCorners_size );
                         }
                   } 
               }  //j
            } //i

            for (i=0;i<MAX_FP_ALL;i++) {

                if (VX_SUCCESS == status)
                {
                    prms->buf_candidCorners_size = 8 * sizeof (int32_t) ;
                    prms->buf_candidCorners_ptr[i] = tivxMemAlloc(prms->buf_candidCorners_size, TIVX_MEM_EXTERNAL_SCRATCH);

                    if (NULL == prms->buf_candidCorners_ptr[i])
                    {
                        status = VX_ERROR_NO_MEMORY;
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                    }
                    else
                    {
                        memset(prms->buf_candidCorners_ptr[i], 0, prms->buf_candidCorners_size);
                    }
               } 
           } // i

           if (VX_SUCCESS == status)
           {
                prms->buf_matchScore_size = MAX_FP_ALL * sizeof (double) ;
                prms->buf_matchScore_ptr = tivxMemAlloc(prms->buf_matchScore_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_matchScore_ptr)
                 {
                     status = VX_ERROR_NO_MEMORY;
                     VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                 }
                 else
                 {
                     memset(prms->buf_matchScore_ptr, 0, prms->buf_matchScore_size);
                 }
           } 

    return status;
}

static vx_status VX_CALLBACK tivxPointDetectProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    /* Shashank-1.5 */
    svPointDetect_t       *in_params; 
    svLdcLut_t            *in_ldclut; 
    svACDetectStructFinalCorner_t *out_params; 

    vx_status status = VX_SUCCESS;
    tivxPointDetectParams *prms = NULL;
    tivx_obj_desc_user_data_object_t *in_configuration_desc;
    tivx_obj_desc_user_data_object_t *in_ldclut_desc;
    tivx_obj_desc_image_t *in_desc;
    tivx_obj_desc_user_data_object_t *out_configuration_desc;
    tivx_obj_desc_image_t *buf_bwluma_frame_desc;

    if ( (num_params != TIVX_KERNEL_POINT_DETECT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        uint32_t size;
        in_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX];
        in_ldclut_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX];
        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_IN_IDX];
        out_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX];
        buf_bwluma_frame_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_BUF_BWLUMA_FRAME_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxPointDetectParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {

        void *in_configuration_target_ptr;
        void *in_ldclut_target_ptr;
        void *in_target_ptr;
        void *out_configuration_target_ptr;
        void *buf_bwluma_frame_target_ptr;

        in_configuration_target_ptr = tivxMemShared2TargetPtr(&in_configuration_desc->mem_ptr);
        tivxMemBufferMap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_ldclut_target_ptr = tivxMemShared2TargetPtr(&in_ldclut_desc->mem_ptr);
        tivxMemBufferMap(in_ldclut_target_ptr,
           in_ldclut_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_target_ptr = tivxMemShared2TargetPtr(&in_desc->mem_ptr[0]);
        tivxMemBufferMap(in_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        out_configuration_target_ptr = tivxMemShared2TargetPtr(&out_configuration_desc->mem_ptr);
        tivxMemBufferMap(out_configuration_target_ptr,
           out_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY);

        if( buf_bwluma_frame_desc != NULL)
        {
            buf_bwluma_frame_target_ptr = tivxMemShared2TargetPtr(&buf_bwluma_frame_desc->mem_ptr[0]);
            tivxMemBufferMap(buf_bwluma_frame_target_ptr,
               buf_bwluma_frame_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
               VX_WRITE_ONLY);
        }

        status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);

        if (VX_SUCCESS == status)
        {  /* Core Process block */
            uint8_t *in_addr = NULL;
            VXLIB_bufParams2D_t vxlib_buf_bwluma_frame;
            uint8_t *buf_bwluma_frame_addr = NULL;
            vx_int16 retval = -1;
            #ifdef DEBUG_1
            uint32_t width, height, stride;
            #endif

            tivxSetPointerLocation(in_desc, &in_target_ptr, &in_addr); // Primary input is in_addr

            if( buf_bwluma_frame_desc != NULL)
            {
                tivxInitBufParams(buf_bwluma_frame_desc, &vxlib_buf_bwluma_frame);
                tivxSetPointerLocation(buf_bwluma_frame_desc, &buf_bwluma_frame_target_ptr, &buf_bwluma_frame_addr);
            }

            /* call kernel processing function */

            in_params  = (svPointDetect_t *)in_configuration_target_ptr;
            in_ldclut  = (svLdcLut_t      *)in_ldclut_target_ptr;
            out_params = (svACDetectStructFinalCorner_t *)out_configuration_target_ptr;
            
            prms->img_prms.dim_x = in_desc->imagepatch_addr[0].dim_x;
            prms->img_prms.dim_y = in_desc->imagepatch_addr[0].dim_y;
            prms->img_prms.stride_y = in_desc->imagepatch_addr[0].stride_y;

            if (NULL != buf_bwluma_frame_addr)
            {
                retval =svGetFinderPatterns(in_params,in_ldclut, prms, in_addr, buf_bwluma_frame_addr,out_params);
            }

            if (0 == retval) 
            {
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "InCorrect Corners found \n");
            }
            
            #ifdef DEBUG_1
            width = prms->img_prms.dim_x;
            height = prms->img_prms.dim_y;
            stride = prms->img_prms.stride_y;
            printf("width =%d height = %d stride = %d \n",width,height,stride);
            printf("Final Corners Point1 = %d %d %d %d  %d %d %d %d \n",out_params->finalCorners[0][0]>>4,
            out_params->finalCorners[0][1]>>4,out_params->finalCorners[0][2]>>4,out_params->finalCorners[0][3]>>4,out_params->finalCorners[0][4]>>4,
            out_params->finalCorners[0][5]>>4,out_params->finalCorners[0][6]>>4,out_params->finalCorners[0][7]>>4); 

            printf("final Corners Point2 = %d %d %d %d  %d %d %d %d \n",out_params->finalCorners[1][0]>>4,
            out_params->finalCorners[1][1]>>4,out_params->finalCorners[1][2]>>4,out_params->finalCorners[1][3]>>4,out_params->finalCorners[1][4]>>4,
            out_params->finalCorners[1][5]>>4,out_params->finalCorners[1][6]>>4,out_params->finalCorners[1][7]>>4); 

            #endif


            /* The corners are generated in prms->buf_finalCorners_ptr[0][i][0..7] */
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to reset scratch heap\n");
        }

        tivxMemBufferUnmap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_ldclut_target_ptr,
           in_ldclut_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(out_configuration_target_ptr,
           out_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        if( buf_bwluma_frame_desc != NULL)
        {
            tivxMemBufferUnmap(buf_bwluma_frame_target_ptr,
               buf_bwluma_frame_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }



    }

    return status;
}

static vx_status VX_CALLBACK tivxPointDetectCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPointDetectParams *prms = NULL;
    void *in_configuration_desc_target_ptr;
    void *in_target_ptr;
    tivx_obj_desc_image_t *in_desc;
    vx_int16   SVInCamFrmHeight;
    vx_int16   SVInCamFrmWidth;

    if ( (num_params != TIVX_KERNEL_POINT_DETECT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        tivx_obj_desc_user_data_object_t *in_configuration_desc;
        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_IN_IDX];
        in_target_ptr = tivxMemShared2TargetPtr(&in_desc->mem_ptr[0]);
        tivxMemBufferMap(in_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        SVInCamFrmHeight   = in_desc->imagepatch_addr[0].dim_y; 
        SVInCamFrmWidth    = in_desc->imagepatch_addr[0].dim_x; 

        in_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX];
        in_configuration_desc_target_ptr = tivxMemShared2TargetPtr(&in_configuration_desc->mem_ptr); 
        tivxMemBufferMap(in_configuration_desc_target_ptr, in_configuration_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY); 

        status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to reset scratch heap\n");
        }
        else
        {
            prms = tivxMemAlloc(sizeof(tivxPointDetectParams), TIVX_MEM_EXTERNAL);
        }

        if (NULL != prms)
        {            
            status = tivxPointDetectAllocPrms(prms, SVInCamFrmHeight, SVInCamFrmWidth);
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxPointDetectParams));
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
                /* Shashank-5: Unmap buffers, only struct */
        tivxMemBufferUnmap(in_configuration_desc_target_ptr, in_configuration_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(in_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);


    }

    return status;
}

static vx_status VX_CALLBACK tivxPointDetectDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPointDetectParams *prms = NULL;
    uint32_t size;

    if ( (num_params != TIVX_KERNEL_POINT_DETECT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((NULL != prms) &&
            (sizeof(tivxPointDetectParams) == size))
        {
            status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to reset scratch heap\n");
            }
            else
            {
                /* Note: Freeing this in order to allow for total allocated memory to reset */
                tivxMemFree(prms->buf_IntegralImg_ptr, prms->buf_IntegralImg_size, TIVX_MEM_EXTERNAL_SCRATCH);
                tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

void tivxAddTargetKernelPointDetect(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_point_detect_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_POINT_DETECT_NAME,
                            target_name,
                            tivxPointDetectProcess,
                            tivxPointDetectCreate,
                            tivxPointDetectDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelPointDetect(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_point_detect_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_point_detect_target_kernel = NULL;
    }
}


