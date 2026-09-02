/*
*
* Copyright (c) {2015 - 2024} Texas Instruments Incorporated
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
 *  \file tidl_reduce.c
 *
 *  \brief This file defines kernel functions for Batch Norm layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_reduce.h"
#include <math.h>
#include <limits>
#include "tidl_reduce_ref.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

int32_t TIDL_reduceProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t  status      = TIDL_SUCCESS;
  
  const sLink_t  *link, *procLink = NULL;
  const sBufParams_t *dstBufParams;
  int32_t outWidth, outChannels, tidlChannelPitch = 0, tidlLinePitch, tidlDimNumch;
  sWorkloadUnit_t *workloadUnit = (sWorkloadUnit_t *)algLayer->workloadUnit;
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];

  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & (uint32_t)TIDL_FLOW_CTRL_REF_ONLY) != (uint32_t)0U)
  {
    uint8_t  (*inPtr)[]     = (uint8_t (*)[])(inPtrs[0]);
    int8_t   (*outPtr)[]    = (int8_t (*)[])(outPtrs[0]);
    sTIDL_ReduceParams_t   *params = &tidlLayer->layerParams.reduceParams;
    sTIDL_DataParams_t        *inDataParams;
    inDataParams   = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;

    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *) &createParams;

    status = TIDL_reduceRefProcess(&intAlgObj,
                                   algLayer,
                                   tidlLayer,
                                   params,
                                   inPtr,
                                   outPtr,
                                   inDataParams,
                                   &tidlLayer->outData);
    
    if(((uint32_t)commonParams->createParams->flowCtrl & (uint32_t)TIDL_FLOW_CTRL_REF_STAT) == (uint32_t)TIDL_FLOW_CTRL_REF_STAT)
    {
      int32_t LayerIdx;
      LayerIdx = algLayer->layerIdx;
      TIDL_UpdateScaleFactors(&intAlgObj, LayerIdx, 0, 0, 1);
    }
  }
  else /* if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY) */
  #endif
  {
    TIDL_GetTensorDimensions( commonParams->net,
                              algLayer->workloadUnit,
                              layerIdx,
                              NULL,
                              NULL,
                              NULL,
                              &outWidth,
                              NULL,
                              &outChannels);

    for (int32_t linkIdx =0 ; linkIdx < algLayer->workloadUnit->numLinks; linkIdx++)
    {
      getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
      link = linkPtrList[0];
      if(link->type == (int32_t)LINK_PROC)
      {
        procLink = link;
      }
    }

    if (procLink != NULL)
    {
      dstBufParams = (const sBufParams_t *) TIDL_getBufferParams(&(procLink->sink), commonParams->gcHelperHandle);
      tidlChannelPitch = procLink->sink.flowDim[FLOW_PIPELINE][0];
    }
    else
    {
      status = TIDL_ERR_NOT_IMPLEMENTED;
    }
    if (status == TIDL_SUCCESS)
    {
      tidlLinePitch = outWidth + dstBufParams->padParams.cols;
      tidlDimNumch = outChannels;

      status = TIDL_deviceUtilsCommonProcess(commonParams,
                                              algLayer,
                                              tidlLayer,
                                              inPtrs,
                                              outPtrs,
                                              layerIdx);

      int32_t numSplitsPerCh = getNumSplits((const sWorkloadUnit_t*)workloadUnit);
      if(numSplitsPerCh > 1)
      {
        if((dstBufParams->dataType == DATA_8_BIT) || (dstBufParams->dataType == DATA_8S_BIT))
        {
          int8_t* outLastLinePtr = ((int8_t*)outPtrs[0]) + ((tidlDimNumch * tidlChannelPitch) - tidlLinePitch);

          OPENACC(data copyout(outLastLinePtr[:algLayer->scratchSize-1]))
          OPENACC(parallel loop)
          for(int32_t i0 = 0; i0 < algLayer->scratchSize; i0++)
          {
            outLastLinePtr[i0] = ((int8_t*)algLayer->layerParams.reduceParams.ptrToScratch)[i0];
          }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
         This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
          TIDL_LDRA_TAG : TIDL_LDRA_TAG_REDUCE_PRIOR_CHECK_001, TIDL_LDRA_TAG_REDUCE_PRIOR_CHECK_002
        Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
        This does not affect runtime behavior or safety.
        <justification end> */
        else if((dstBufParams->dataType == DATA_16_BIT) || (dstBufParams->dataType == DATA_16S_BIT))
        {
          int16_t* outLastLinePtr = (int16_t*)(((int8_t*)outPtrs[0]) + (((tidlDimNumch * tidlChannelPitch) - tidlLinePitch)*2));

          OPENACC(data copyout(outLastLinePtr[:algLayer->scratchSize-1]))
          OPENACC(parallel loop)
          for(int32_t i0 = 0; i0 < algLayer->scratchSize; i0++)
          {
            outLastLinePtr[i0] = ((int16_t*)algLayer->layerParams.reduceParams.ptrToScratch)[i0];
          }
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
         This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
          TIDL_LDRA_TAG : TIDL_LDRA_TAG_REDUCE_PRIOR_CHECK_001, TIDL_LDRA_TAG_REDUCE_PRIOR_CHECK_002
        Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
        This does not affect runtime behavior or safety.
        <justification end> */
        else
        {
          status = TIDL_ERR_INVALID_TYPE;
        }
        /* LDRA_JUSTIFY_END */
      }
      else
      {
        /* do nothing */
      }
    }
  }

  return status;
}

