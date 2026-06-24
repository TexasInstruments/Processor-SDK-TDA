/*
 *
 * Copyright (c) {2015 - 2026} Texas Instruments Incorporated
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
 *  \file tidl_nonZero_ref.c
 *
 *  \brief This file defines kernel functions for Element Wise layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <limits>
#include <math.h>
#include "tidl_nonZero_ref.h"


template <class Tin>
int32_t TIDL_fillOutBufAndUpdateOutShape(sTIDL_DataParams_t * inDataParams,
                            sTIDL_DataParams_t * outDataParams,
                            Tin *inPtr,
                            int32_t *outPtr,
                            sTIDL_NonZeroParams_t  *params)
{
    int32_t status = IALG_EOK;
    uint32_t numNonZeroElements = 0;
    for ( int32_t batchIdx = 0; batchIdx < inDataParams->dimValues[TIDL_DIM_BATCH] ; batchIdx++ )
    {
        for ( int32_t dim1Idx = 0; dim1Idx < inDataParams->dimValues[TIDL_DIM_DIM1] ; dim1Idx++ )
        {
            for ( int32_t dim2Idx = 0; dim2Idx < inDataParams->dimValues[TIDL_DIM_DIM2]; dim2Idx++ )
            {
                for ( int32_t chIdx = 0; chIdx < inDataParams->dimValues[TIDL_DIM_NUMCH]; chIdx++ )
                {
                    for ( int32_t heightIdx = 0; heightIdx < inDataParams->dimValues[TIDL_DIM_HEIGHT]; heightIdx++)
                    {
                        for ( int32_t widthIdx = 0; widthIdx < inDataParams->dimValues[TIDL_DIM_WIDTH]; widthIdx++)
                        {
                            int32_t inOffset = (batchIdx * inDataParams->pitch[TIDL_ROI_PITCH]) + (dim1Idx * inDataParams->pitch[TIDL_DIM1_PITCH]) + (dim2Idx * inDataParams->pitch[TIDL_DIM2_PITCH]) + (chIdx * inDataParams->pitch[TIDL_CHANNEL_PITCH]) + (heightIdx * inDataParams->pitch[TIDL_LINE_PITCH]) + (widthIdx * 1);
                            if ( inPtr[inOffset] != 0)
                            {
                                uint32_t nonZeroElementIdx[TIDL_DIM_MAX] = {batchIdx, dim1Idx, dim2Idx, chIdx, heightIdx, widthIdx};
                                for ( int dim = 0 ; dim < params->numValidInputDims ; dim++)
                                {
                                    int32_t outOffset = (outDataParams->pitch[TIDL_LINE_PITCH] * dim) + (numNonZeroElements * 1);
                                    outPtr[outOffset] = nonZeroElementIdx[TIDL_DIM_MAX - outDataParams->dimValues[TIDL_DIM_HEIGHT] + dim ]; 
                                }
                                numNonZeroElements += 1;
                            }
                            
                        }
                    }
                }
            }
        }
    }

    outDataParams->dimValues[TIDL_DIM_WIDTH] = numNonZeroElements;
    return status;
}


int32_t TIDL_NonZeroLayerRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
    int32_t status = IALG_EOK;

    sTIDL_DataParams_t *inDataParams  = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;

    sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;

    sTIDL_NonZeroParams_t  *params    = &tidlLayer->layerParams.nonZeroParams;

    if(inDataParams->elementType == TIDL_UnsignedChar)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (uint8_t *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_SignedChar)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (int8_t *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_UnsignedShort)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (uint16_t *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_SignedShort)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (int16_t *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_UnsignedWord)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (uint32_t *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_SignedWord)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (int32_t *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_SinglePrecFloat)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (float32_tidl *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else if(inDataParams->elementType == TIDL_Bool)
    {
        status = TIDL_fillOutBufAndUpdateOutShape(inDataParams,
                                                        outDataParams,
                                                        (bool *)inPtrs[0],
                                                        (int32_t *)outPtrs[0],
                                                        params);
    }
    else
    {
        tidl_printf(0,"TIDL_NonZero inElementType is  not supported !!!\n ");
        status = TIDL_ERR_FAILURE;
    }
    
    return status;

}