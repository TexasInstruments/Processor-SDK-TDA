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

#include <TI/tivx.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_stereo_kernels_priv.h"
#include "tivx_kernels_target_utils.h"

#ifdef BUILD_BAM
void tivxAddTargetKernelBamPointCloudCreation(void);
void tivxAddTargetKernelBamOccupancyGridDetection(void);
void tivxAddTargetKernelBamDisparityMerge(void);
void tivxAddTargetKernelBamMedianFilter(void);
void tivxAddTargetKernelBamHoleFilling(void);
void tivxAddTargetKernelBamExtractDisparityConfidence(void);
void tivxAddTargetKernelBamGroundEstimation(void);
void tivxAddTargetKernelBamObstacleDetection(void);
void tivxAddTargetKernelBamSdeDisparityVisualize(void);

#else
void tivxAddTargetKernelPointCloudCreation(void);
void tivxAddTargetKernelOccupancyGridDetection(void);
void tivxAddTargetKernelDisparityMerge(void);
void tivxAddTargetKernelMedianFilter(void);
void tivxAddTargetKernelHoleFilling(void);
void tivxAddTargetKernelSdeDisparityVisualize(void);
void tivxAddTargetKernelSdeHistogramVisualize(void);
void tivxAddTargetKernelSdeTriangulation(void);
void tivxAddTargetKernelExtractDisparityConfidence(void);
void tivxAddTargetKernelGroundEstimation(void);
void tivxAddTargetKernelObstacleDetection(void);

#endif

#ifdef BUILD_BAM
void tivxRemoveTargetKernelBamPointCloudCreation(void);
void tivxRemoveTargetKernelBamOccupancyGridDetection(void);
void tivxRemoveTargetKernelBamDisparityMerge(void);
void tivxRemoveTargetKernelBamMedianFilter(void);
void tivxRemoveTargetKernelBamHoleFilling(void);
void tivxRemoveTargetKernelBamExtractDisparityConfidence(void);
void tivxRemoveTargetKernelBamGroundEstimation(void);
void tivxRemoveTargetKernelBamObstacleDetection(void);
void tivxRemoveTargetKernelBamSdeDisparityVisualize(void);

#else
void tivxRemoveTargetKernelPointCloudCreation(void);
void tivxRemoveTargetKernelOccupancyGridDetection(void);
void tivxRemoveTargetKernelDisparityMerge(void);
void tivxRemoveTargetKernelMedianFilter(void);
void tivxRemoveTargetKernelHoleFilling(void);
void tivxRemoveTargetKernelSdeDisparityVisualize(void);
void tivxRemoveTargetKernelSdeHistogramVisualize(void);
void tivxRemoveTargetKernelSdeTriangulation(void);
void tivxRemoveTargetKernelExtractDisparityConfidence(void);
void tivxRemoveTargetKernelGroundEstimation(void);
void tivxRemoveTargetKernelObstacleDetection(void);

#endif

static Tivx_Target_Kernel_List  gTivx_target_kernel_list[] = {
#ifdef BUILD_BAM
    {&tivxAddTargetKernelBamPointCloudCreation, &tivxAddTargetKernelBamPointCloudCreation},
    {&tivxAddTargetKernelBamOccupancyGridDetection, &tivxRemoveTargetKernelBamOccupancyGridDetection},
    {&tivxAddTargetKernelBamDisparityMerge, &tivxRemoveTargetKernelBamDisparityMerge},
    {&tivxAddTargetKernelBamMedianFilter, &tivxRemoveTargetKernelBamMedianFilter},
    {&tivxAddTargetKernelBamHoleFilling, &tivxRemoveTargetKernelBamHoleFilling},
    {&tivxAddTargetKernelBamExtractDisparityConfidence, &tivxRemoveTargetKernelBamExtractDisparityConfidence},
    {&tivxAddTargetKernelBamGroundEstimation, &tivxRemoveTargetKernelBamGroundEstimation},
    {&tivxAddTargetKernelBamObstacleDetection, &tivxRemoveTargetKernelBamObstacleDetection},
    {&tivxAddTargetKernelBamSdeDisparityVisualize, &tivxRemoveTargetKernelBamSdeDisparityVisualize},
#else
    {&tivxAddTargetKernelPointCloudCreation, &tivxRemoveTargetKernelPointCloudCreation},
    {&tivxAddTargetKernelOccupancyGridDetection, &tivxRemoveTargetKernelOccupancyGridDetection},
    {&tivxAddTargetKernelDisparityMerge, &tivxRemoveTargetKernelDisparityMerge},
    {&tivxAddTargetKernelMedianFilter, &tivxRemoveTargetKernelMedianFilter},
    {&tivxAddTargetKernelHoleFilling, &tivxRemoveTargetKernelHoleFilling},
#if ! (defined(__C7100__) || defined(__C7120__))
    {&tivxAddTargetKernelSdeDisparityVisualize, &tivxRemoveTargetKernelSdeDisparityVisualize},
    {&tivxAddTargetKernelSdeHistogramVisualize, &tivxRemoveTargetKernelSdeHistogramVisualize},
#endif
    {&tivxAddTargetKernelSdeTriangulation, &tivxRemoveTargetKernelSdeTriangulation},
    {&tivxAddTargetKernelExtractDisparityConfidence, &tivxRemoveTargetKernelExtractDisparityConfidence},
    {&tivxAddTargetKernelGroundEstimation, &tivxRemoveTargetKernelGroundEstimation},
    {&tivxAddTargetKernelObstacleDetection, &tivxRemoveTargetKernelObstacleDetection},
    {&tivxAddTargetKernelSdeHistogramVisualize, &tivxRemoveTargetKernelSdeHistogramVisualize},
    {&tivxAddTargetKernelSdeDisparityVisualize, &tivxRemoveTargetKernelSdeDisparityVisualize},
#endif
};

void tivxRegisterStereoTargetKernels(void)
{
    tivxRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));
}

void tivxUnRegisterStereoTargetKernels(void)
{
    tivxUnRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));
}

