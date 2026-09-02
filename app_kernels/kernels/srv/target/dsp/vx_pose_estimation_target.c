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
#include "tivx_kernel_pose_estimation.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "DSPF_sp_svd_cn.h"
#include "DSPF_dp_svd_cn.h"
#include "DSPF_dp_qrd_cn.h"
#include "srv_common.h"
#include "core_pose_estimation.h"
#include <utils/mem/include/app_mem.h>

#include <math.h>
#include <float.h>


#include <stdio.h>

static tivx_target_kernel vx_pose_estimation_target_kernel = NULL;

static vx_status VX_CALLBACK tivxPoseEstimationProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPoseEstimationCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPoseEstimationDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status tivxPoseEstimationAllocPrms(tivxPoseEstimationParams *prms);

static vx_status tivxPoseEstimationAllocPrms(tivxPoseEstimationParams *prms)
{
    vx_status status = VX_SUCCESS;
    int i;
            
            for (i=0;i<MAX_INPUT_CAMERAS;i++)
            {
                if (VX_SUCCESS == status)
                {
                    prms->buf_cip_size = sizeof(CameraIntrinsicParams) ;
                    prms->buf_cip_ptr[i] = tivxMemAlloc(prms->buf_cip_size, TIVX_MEM_EXTERNAL_SCRATCH);

                    if (NULL == prms->buf_cip_ptr[i])
                    {
                        status = VX_ERROR_NO_MEMORY;
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                    }
                    else
                    {
                        memset(prms->buf_cip_ptr[i], 0, sizeof(CameraIntrinsicParams));
                    }
                }
            } //i

            for (i=0;i<MAX_INPUT_CAMERAS;i++)
            {
                if (VX_SUCCESS == status)
                {
                    prms->buf_chartPoints_size = sizeof(Point2D_f) * NUM_CHART_CORNERS ;
                    prms->buf_chartPoints_ptr[i] = tivxMemAlloc(prms->buf_chartPoints_size, TIVX_MEM_EXTERNAL_SCRATCH);

                    if (NULL == prms->buf_chartPoints_ptr[i])
                    {
                        status = VX_ERROR_NO_MEMORY;
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                    }
                    else
                    {
                        memset(prms->buf_chartPoints_ptr[i], 0, sizeof(Point2D_f) * NUM_CHART_CORNERS);
                    }
                }
            } //int i

            for (i=0;i<MAX_INPUT_CAMERAS;i++)
            {
                if (VX_SUCCESS == status)
                {
                    prms->buf_cornerPoints_size = sizeof(Point2D_f) * NUM_CHART_CORNERS ;
                    prms->buf_cornerPoints_ptr[i] = tivxMemAlloc(prms->buf_cornerPoints_size, TIVX_MEM_EXTERNAL_SCRATCH);

                    if (NULL == prms->buf_cornerPoints_ptr[i])
                    {
                        status = VX_ERROR_NO_MEMORY;
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                    }
                    else
                    {
                        memset(prms->buf_cornerPoints_ptr[i], 0, sizeof(Point2D_f) * NUM_CHART_CORNERS);
                    }
                }
            }

            for (i=0;i<4;i++)
            {
                if (VX_SUCCESS == status)
                {
                    prms->buf_baseChartPoints_size = sizeof(Point2D_f) * NUM_CORNERS_PER_CHART ;
                    prms->buf_baseChartPoints_ptr[i] = tivxMemAlloc(prms->buf_baseChartPoints_size, TIVX_MEM_EXTERNAL_SCRATCH);

                    if (NULL == prms->buf_baseChartPoints_ptr[i])
                    {
                        status = VX_ERROR_NO_MEMORY;
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                    }
                    else 
                    {
                        memset(prms->buf_baseChartPoints_ptr[i], 0, sizeof(Point2D_f) * NUM_CORNERS_PER_CHART);
                    }
                }
            }


            if (VX_SUCCESS == status)
            {
                prms->buf_inCornerPoints_size = sizeof(vx_int32) * ((MAX_INPUT_CAMERAS*8*FP_TO_DETECT) +1) ;
                prms->buf_inCornerPoints_ptr  = tivxMemAlloc(prms->buf_inCornerPoints_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_inCornerPoints_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    memset(prms->buf_inCornerPoints_ptr, 0, sizeof(vx_int32) * ((MAX_INPUT_CAMERAS*8*FP_TO_DETECT) +1));
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->buf_H_cg_size = sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS ;
                prms->buf_H_cg_ptr = tivxMemAlloc(prms->buf_H_cg_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_H_cg_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_H_cg_ptr, 0, sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_R_cg_size = sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS ;
                prms->buf_R_cg_ptr = tivxMemAlloc(prms->buf_R_cg_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_R_cg_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_R_cg_ptr, 0, sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_R_gc_size = sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS ;
                prms->buf_R_gc_ptr = tivxMemAlloc(prms->buf_R_gc_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_R_gc_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_R_gc_ptr, 0, sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_t_cg_size = sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS ;
                prms->buf_t_cg_ptr = tivxMemAlloc(prms->buf_t_cg_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_t_cg_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_t_cg_ptr, 0, sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_t_gc_size = sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS ;
                prms->buf_t_gc_ptr = tivxMemAlloc(prms->buf_t_gc_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_t_gc_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_t_gc_ptr, 0, sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_normCornerPoint_size = sizeof(Point2D_f) * NUM_CHART_CORNERS ;
                prms->buf_normCornerPoint_ptr = tivxMemAlloc(prms->buf_normCornerPoint_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_normCornerPoint_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_normCornerPoint_ptr, 0, sizeof(Point2D_f) * NUM_CHART_CORNERS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_points1norm_size = sizeof(Point2D_f) * NUM_CHART_CORNERS*SCALING_2;
                prms->buf_points1norm_ptr = tivxMemAlloc(prms->buf_points1norm_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_points1norm_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_points1norm_ptr, 0, sizeof(Point2D_f) * NUM_CHART_CORNERS*SCALING_2);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_points2norm_size = sizeof(Point2D_f) * NUM_CHART_CORNERS * SCALING_2;
                prms->buf_points2norm_ptr = tivxMemAlloc(prms->buf_points2norm_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_points2norm_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_points2norm_ptr, 0, sizeof(Point2D_f) * NUM_CHART_CORNERS * SCALING_2);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_xvec_size = sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) * SCALING;
                prms->buf_xvec_ptr = tivxMemAlloc(prms->buf_xvec_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_xvec_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_xvec_ptr, 0, sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) * SCALING);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_fvec_size = sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) ;
                prms->buf_fvec_ptr = tivxMemAlloc(prms->buf_fvec_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_fvec_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_fvec_ptr, 0, sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3));
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_bvec_size = sizeof(Flouble) * A_NCOLS ;
                prms->buf_bvec_ptr = tivxMemAlloc(prms->buf_bvec_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_bvec_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_bvec_ptr, 0, sizeof(Flouble) * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_tempvec_size = sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) ;
                prms->buf_tempvec_ptr = tivxMemAlloc(prms->buf_tempvec_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_tempvec_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_tempvec_ptr, 0, sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3));
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_deltavec_size = sizeof(Flouble) * A_NCOLS ;
                prms->buf_deltavec_ptr = tivxMemAlloc(prms->buf_deltavec_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_deltavec_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_deltavec_ptr, 0, sizeof(Flouble) * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_Jacob_size = sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) * A_NCOLS ;
                prms->buf_Jacob_ptr = tivxMemAlloc(prms->buf_Jacob_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_Jacob_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_Jacob_ptr, 0, sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3)* A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_JacobT_size = sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) *A_NCOLS ;
                prms->buf_JacobT_ptr = tivxMemAlloc(prms->buf_JacobT_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_JacobT_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_JacobT_ptr, 0, sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3)* A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_gammaIdentity_size = sizeof(Flouble) * A_NCOLS *A_NCOLS ;
                prms->buf_gammaIdentity_ptr = tivxMemAlloc(prms->buf_gammaIdentity_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_gammaIdentity_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_gammaIdentity_ptr, 0, sizeof(Flouble) * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_A_size = sizeof(Flouble) * A_SIZE ;
                prms->buf_A_ptr = tivxMemAlloc(prms->buf_A_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_A_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_A_ptr, 0, sizeof(Flouble) * A_SIZE);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_U_size = sizeof(Flouble) * U_SIZE ;
                prms->buf_U_ptr = tivxMemAlloc(prms->buf_U_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_U_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_U_ptr, 0, sizeof(Flouble) * U_SIZE);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_U1_size = sizeof(Flouble) * U_SIZE ;
                prms->buf_U1_ptr = tivxMemAlloc(prms->buf_U1_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_U1_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_U1_ptr, 0, sizeof(Flouble) * U_SIZE);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_V_size = sizeof(Flouble) * V_SIZE ;
                prms->buf_V_ptr = tivxMemAlloc(prms->buf_V_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_V_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_V_ptr, 0, sizeof(Flouble) * V_SIZE);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_Diag_size = sizeof(Flouble) * A_NROWS ;
                prms->buf_Diag_ptr = tivxMemAlloc(prms->buf_Diag_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_Diag_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_Diag_ptr, 0, sizeof(Flouble) * A_NROWS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_superDiag_size = sizeof(Flouble) * A_NROWS ;
                prms->buf_superDiag_ptr = tivxMemAlloc(prms->buf_superDiag_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_superDiag_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_superDiag_ptr, 0, sizeof(Flouble) * A_NROWS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_AInv_size = sizeof(Flouble) * A_NCOLS * A_NCOLS ;
                prms->buf_AInv_ptr = tivxMemAlloc(prms->buf_AInv_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_AInv_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_AInv_ptr, 0, sizeof(Flouble) * A_NCOLS * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_Q_size = sizeof(Flouble) * A_NCOLS * A_NCOLS ;
                prms->buf_Q_ptr = tivxMemAlloc(prms->buf_Q_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_Q_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_Q_ptr, 0, sizeof(Flouble) * A_NCOLS * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_R_size = sizeof(Flouble) * A_NCOLS * A_NCOLS ;
                prms->buf_R_ptr = tivxMemAlloc(prms->buf_R_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_R_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_R_ptr, 0, sizeof(Flouble) * A_NCOLS * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_yvec_size = sizeof(Flouble) * A_NCOLS ;
                prms->buf_yvec_ptr = tivxMemAlloc(prms->buf_yvec_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_yvec_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_yvec_ptr, 0, sizeof(Flouble) * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_h_size = sizeof(Flouble) * A_NCOLS ;
                prms->buf_h_ptr = tivxMemAlloc(prms->buf_h_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_h_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_h_ptr, 0, sizeof(Flouble) * A_NCOLS);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_R_DLT_size = sizeof(Flouble) * 9 ;
                prms->buf_R_DLT_ptr = tivxMemAlloc(prms->buf_R_DLT_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_R_DLT_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_R_DLT_ptr, 0, sizeof(Flouble) * 9);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_t_DLT_size = sizeof(Flouble) * 3 ;
                prms->buf_t_DLT_ptr = tivxMemAlloc(prms->buf_t_DLT_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_t_DLT_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_t_DLT_ptr, 0, sizeof(Flouble) * 3);
                }
            }
            if (VX_SUCCESS == status)
            {
                prms->buf_in_corners_size = sizeof(svACDetectStructFourCameraCorner_t);
                prms->buf_in_corners_ptr = tivxMemAlloc(prms->buf_in_corners_size, TIVX_MEM_EXTERNAL_SCRATCH);

                if (NULL == prms->buf_in_corners_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
                }
                else
                {
                    /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                    memset(prms->buf_in_corners_ptr, 0, sizeof(svACDetectStructFourCameraCorner_t));
                }
            }

    return status;
}

static vx_status VX_CALLBACK tivxPoseEstimationProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPoseEstimationParams *prms = NULL;
    tivx_obj_desc_object_array_t *in_corner_points_desc;
    tivx_obj_desc_user_data_object_t *user_data_in_corner_points_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS]; //SD , update type
    vx_uint32 i;
    tivx_obj_desc_user_data_object_t *in_configuration_desc;
    tivx_obj_desc_user_data_object_t *in_ldclut_desc;
    tivx_obj_desc_user_data_object_t *out_calmat_desc;


    svPoseEstimation_t                 *in_params_sv;
    svLdcLut_t                         *in_ldclut; 

    svACDetectStructFinalCorner_t      *in_single_camera_corners;
//    svACDetectStructFourCameraCorner_t in_params_corners;
    svACCalmatStruct_t                 *out_params_calmat;


    if ( (num_params != TIVX_KERNEL_POSE_ESTIMATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CORNER_POINTS_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_OUT_CALMAT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        uint32_t size;
        in_corner_points_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CORNER_POINTS_IDX];
        in_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CONFIGURATION_IDX];
        in_ldclut_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_LDCLUT_IDX];
        out_calmat_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_POSE_ESTIMATION_OUT_CALMAT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxPoseEstimationParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        /* Do not merge some of the code below from python generator */
        /* Py generator always generates img instead of user_data    */
        void *in_corner_points_target_ptr[4];
        void *in_configuration_target_ptr;
        void *in_ldclut_target_ptr;
        void *out_calmat_target_ptr;
        vx_uint8 bLeftChart[4];
        int x, y;
        
        tivxGetObjDescList(in_corner_points_desc->obj_desc_id, (tivx_obj_desc_t**)user_data_in_corner_points_desc, in_corner_points_desc->num_items);

        status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);

        if (VX_SUCCESS == status)
        {
            for(i=0; i<in_corner_points_desc->num_items; i++) //SD This would be 4 for 4 cameras
            {
                in_corner_points_target_ptr[i] = tivxMemShared2TargetPtr(&user_data_in_corner_points_desc[i]->mem_ptr);
                tivxMemBufferMap(in_corner_points_target_ptr[i],
                   user_data_in_corner_points_desc[i]->mem_size, VX_MEMORY_TYPE_HOST,
                   VX_READ_ONLY);
                in_single_camera_corners = (svACDetectStructFinalCorner_t *) in_corner_points_target_ptr[i];
                /* Populate the master corner buffer */
//                prms->buf_in_corners_ptr->finalCameraCorners[i] = in_single_camera_corners;
                for (y=0;y<FP_TO_DETECT;y++) 
                {
                    for (x=0;x<8;x++) 
                    {
                        prms->buf_in_corners_ptr->finalCameraCorners[i].finalCorners[y][x] = in_single_camera_corners->finalCorners[y][x];
                    }
                }
                prms->buf_in_corners_ptr->finalCameraCorners[i].numFPDetected = in_single_camera_corners->numFPDetected;
            }

            in_configuration_target_ptr = tivxMemShared2TargetPtr(&in_configuration_desc->mem_ptr);
            tivxMemBufferMap(in_configuration_target_ptr,
               in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
               VX_READ_ONLY);

            in_ldclut_target_ptr = tivxMemShared2TargetPtr(&in_ldclut_desc->mem_ptr);
            tivxMemBufferMap(in_ldclut_target_ptr,
               in_ldclut_desc->mem_size, VX_MEMORY_TYPE_HOST,
               VX_READ_ONLY);

            out_calmat_target_ptr = tivxMemShared2TargetPtr(&out_calmat_desc->mem_ptr);
            tivxMemBufferMap(out_calmat_target_ptr,
               out_calmat_desc->mem_size, VX_MEMORY_TYPE_HOST,
               VX_WRITE_ONLY);

            /* ADD Core function */
            in_params_sv      = (svPoseEstimation_t *) in_configuration_target_ptr;
            in_ldclut  = (svLdcLut_t      *)in_ldclut_target_ptr;
            out_params_calmat = (svACCalmatStruct_t *) out_calmat_target_ptr;

            /* pointNum has to be used from the in_params_corners struct */
            /* sv needs to be upgraded to included inChartPos            */
            svPoseEstimate(in_params_sv,in_ldclut,prms, prms->buf_in_corners_ptr, in_params_sv->inChartPos, out_params_calmat->outcalmat, bLeftChart,1);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to reset scratch heap\n");
        }

        for(i=0; i<in_corner_points_desc->num_items; i++)
        {
            tivxMemBufferUnmap(in_corner_points_target_ptr,
               user_data_in_corner_points_desc[i]->mem_size, VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }

        if (VX_SUCCESS == status)
        {
            tivxMemBufferUnmap(in_configuration_target_ptr,
               in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxMemBufferUnmap(in_ldclut_target_ptr,
               in_ldclut_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxMemBufferUnmap(out_calmat_target_ptr,
               out_calmat_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxPoseEstimationCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPoseEstimationParams *prms = NULL;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */
    if ( (num_params != TIVX_KERNEL_POSE_ESTIMATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CORNER_POINTS_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_OUT_CALMAT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to reset scratch heap\n");
        }
        else
        {
            prms = tivxMemAlloc(sizeof(tivxPoseEstimationParams), TIVX_MEM_EXTERNAL);
        }

        if (NULL != prms)
        {
            status = tivxPoseEstimationAllocPrms(prms);
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxPoseEstimationParams));
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxPoseEstimationDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxPoseEstimationParams *prms = NULL;
    uint32_t size;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */
    if ( (num_params != TIVX_KERNEL_POSE_ESTIMATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_IN_CORNER_POINTS_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_POSE_ESTIMATION_OUT_CALMAT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((NULL != prms) &&
            (sizeof(tivxPoseEstimationParams) == size))
        {
            status = appMemResetScratchHeap(APP_MEM_HEAP_DDR_SCRATCH);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to reset scratch heap\n");
            }
            else
            {
                tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

void tivxAddTargetKernelPoseEstimation(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_pose_estimation_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_POSE_ESTIMATION_NAME,
                            target_name,
                            tivxPoseEstimationProcess,
                            tivxPoseEstimationCreate,
                            tivxPoseEstimationDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelPoseEstimation(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_pose_estimation_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_pose_estimation_target_kernel = NULL;
    }
}


