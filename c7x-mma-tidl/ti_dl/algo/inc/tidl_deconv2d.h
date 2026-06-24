/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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

/**
 ----------------------------------------------------------------------------
 @file    tidl_deconv2d.h
 @brief   This file defines process function prototype of Deconvolution layer.
 @version 0.1 (Oct 2016) : Initial version [TVS]
 @version 0.2 (Jan 2017) : Cleaned up [EPR]
 ----------------------------------------------------------------------------
*/

#ifndef ITIDL_DECONV2D_H
#define ITIDL_DECONV2D_H

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"


/* Following is the information needed only during init call and
mostly computed based certain parameters*/
typedef struct
{
  int32_t inFeatureWidth;
  int32_t inFeatureHeight;
  int32_t inFeatureProcWidth;/* Width for processing */
  int32_t inFeatureProcHeight;/* Height for processing */
  int32_t outFeatureWidth;
  int32_t outFeatureHeight;
  int32_t inFeatureLinePitch;
  int32_t outFeatureLinePitch;
  int32_t inChPitch;
  int32_t outChPitch;
  int32_t effKerW;
  int32_t effKerH;
  int32_t NiPerG;
  int32_t NoPerG;
  int32_t kernelPad;
  int32_t featElmntSize;
  int32_t dstElmntSize;
  int32_t procElemSize;
  int32_t totalProcSize;
  int32_t groupIdx; /*To handle case when weights don't fit in L2*/
  int32_t numGroupsPerKernel;
  int32_t numGroupsPerWorkload; /* Total groups to be processed in current workload */
  int32_t inConversionFactor;
  int32_t outConversionFactor;
  int32_t strideBasedFlow;
  int32_t circularFlow;
  int32_t roundBits;
  int32_t packetizeMode;
  int32_t coeffOffsetInBlock;
  const sJoint_t *srcJoint;
  const sJoint_t *sinkJoint;
  const sJoint_t *coeffJoint;
  const sBufParams_t *srcBufParams;
  const sBufParams_t *sinkBufParams;
  const sBufParams_t *coeffBufParams;
#ifndef __C7100__
  sTIDL_virtualPad   virtualPad;
#endif
}TIDL_conv2dRowFlowSetupArgs;

/* Following functions implementations under deconv2d_device.c */
/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_DECONV2D
 @fn         TIDL_deconv2DRowFlowConvertCoeffBuffer
 @brief      Function to generate the values in the coeff buffer

 @param      coeffSrcBuf    : Ptr to weights given by NC
 @param      biasSrcBuf     : Ptr to bias buffer given by NC
 @param      coeffDstBuf    : Ptr to memory where the generated coeff are 
                            expected to be present by procesing routine
 @param      coeffDstPitch  : Expected coeff bitch in coeffDstBuf
 @param      coeffJoint     : Joint containing coeff dataflow info
 @param      gcHelperHandle     : Database containing all gc information
 @param      conv2dparams   : Convolution parameters for the layer
 @param      Ni             : Number of input channels in the tensor
 @param      No             : Number of output channels in the tensor
 @remarks    None
 @return     status  - None -
 ----------------------------------------------------------------------------
*/
int32_t TIDL_deconv2DRowFlowConvertCoeffBuffer(const void * coeffSrcBuf,
                                                  const void *biasSrcBuf,
                                                  void * coeffDstBuf,
                                                  int32_t coeffDstPitch,
                                                  TIDL_conv2dRowFlowSetupArgs  *setupArgs,
                                                  const sGCHelperHandle * gcHelperHandle,
                                                  const sTIDL_ConvParams_t * conv2dparams,
                                                  int32_t Ni,
                                                  int32_t No,
                                                  int32_t numGroupsPerCall);

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_DECONV2D
 @fn         TIDL_deconv2dXfrGetHandleSize
 @brief      Function to get handle size required for the transfer link of deconvolution
             layer (to conduct the output row interleaving)

 @param      linkInitParams    : Ptr to link init params to be given by calling fn
 @param      link              : Ptr to link
 @param      gcHelperHandle        : Ptr to database containing all gc information

 @remarks    None
 @return     Handle size for the deconvolution layer
 ----------------------------------------------------------------------------
*/
int32_t TIDL_deconv2dXfrGetHandleSize(void *linkInitParams,
                                    const sLink_t              *link,
                                    const sGCHelperHandle * gcHelperHandle);

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_DECONV2D
 @fn         TIDL_deconv2dXfrLinkInit
 @brief      Init function for transfer link of deconvolution layer

 @param      linkHandle         : Ptr to link handle for internal use
 @param      linkInitParams     : Ptr to link init params
 @param      commonParams       : Ptr to common tidl params
 @param      link               : Pointer to link
 @param      gcHelperHandle         : Ptr to database containing all gc information


 @remarks    None
 @return     status - TIDL_SUCCESS if successful
 ----------------------------------------------------------------------------
*/
int32_t TIDL_deconv2dXfrLinkInit(void                           *linkHandle,
                                    void                                *linkInitParams,
                                    const WorkloadUnitExec_CommonParams *commonParams,
                                    const sLink_t                       *link,
                                    const sGCHelperHandle *gcHelperHandle);

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_DECONV2D
 @fn         TIDL_deconv2dXfrLinkExec
 @brief      Exec function for transfer link of deconvolution layer

 @param      linkHandle     : Pointer to link handle for internal use
 @param      linkExecArgs   : ExecArgs for the link
 @param      currFlowStage  : Array containing stage of all joints of the link
 @param      currIterCount  : Iteration count of link trigger

 @remarks    None
 @return     - None -
 ----------------------------------------------------------------------------
*/
int32_t TIDL_deconv2dXfrLinkExec(void                            *linkHandle,
                                    const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                                    int32_t                          currFlowStage[],
                                    int32_t                          currIterCount);

float32_tidl TIDL_floatSat(float32_tidl outAcc, sTIDL_Layer_t  * tidlLayer);
void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);


int32_t TIDL_deconv2dGetPerfData(void* linkHandle,
                                 double *perfData);

#endif /* ITIDL_DECONV2D_H*/

