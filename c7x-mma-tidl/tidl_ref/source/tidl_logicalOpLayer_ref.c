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
 *  \file tidl_logicalOpLayer_ref.c
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
#include "tidl_logicalOpLayer_ref.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

static void TIDL_ComputeBroadcastPitches(sTIDL_DataParams_t *dataParams,
    int32_t *widthPitch, int32_t *heightPitch, int32_t *chPitch,
    int32_t *dim1Pitch, int32_t *dim2Pitch, int32_t *batchPitch)
{
    *widthPitch  = (dataParams->dimValues[TIDL_DIM_WIDTH]  == 1) ? 0 : 1;
    *heightPitch = (dataParams->dimValues[TIDL_DIM_HEIGHT] == 1) ? 0 : dataParams->pitch[TIDL_LINE_PITCH];
    *chPitch     = (dataParams->dimValues[TIDL_DIM_NUMCH]  == 1) ? 0 : dataParams->pitch[TIDL_CHANNEL_PITCH];
    *dim1Pitch   = (dataParams->dimValues[TIDL_DIM_DIM1]   == 1) ? 0 : dataParams->pitch[TIDL_DIM1_PITCH];
    *dim2Pitch   = (dataParams->dimValues[TIDL_DIM_DIM2]   == 1) ? 0 : dataParams->pitch[TIDL_DIM2_PITCH];
    *batchPitch  = (dataParams->dimValues[TIDL_DIM_BATCH]  == 1) ? 0 : dataParams->pitch[TIDL_ROI_PITCH];
}

int32_t TIDL_LogicalOperatorRefProcess(sTIDL_DataParams_t * inDataParamsA,
                            sTIDL_DataParams_t * inDataParamsB,
                            sTIDL_DataParams_t * outDataParams,
                            bool *inPtrA,
                            bool *inPtrB,
                            bool *outPtr, int32_t logicalOperatorType)
{
    int32_t inbatchPitchA,inDIM1PitchA,inDIM2PitchA,inchPitchA,inheightPitchA,inwidthPitchA;
    int32_t inbatchPitchB,inDIM1PitchB,inDIM2PitchB,inchPitchB,inheightPitchB,inwidthPitchB;
    int32_t outbatchPitch,outDIM1Pitch,outDIM2Pitch,outchPitch,outheightPitch,outwidthPitch;

    TIDL_ComputeBroadcastPitches(inDataParamsA, &inwidthPitchA, &inheightPitchA, &inchPitchA, &inDIM1PitchA, &inDIM2PitchA, &inbatchPitchA);
    TIDL_ComputeBroadcastPitches(inDataParamsB, &inwidthPitchB, &inheightPitchB, &inchPitchB, &inDIM1PitchB, &inDIM2PitchB, &inbatchPitchB);
    TIDL_ComputeBroadcastPitches(outDataParams, &outwidthPitch, &outheightPitch, &outchPitch, &outDIM1Pitch, &outDIM2Pitch, &outbatchPitch);


    for ( int32_t batchIdx = 0; batchIdx < outDataParams->dimValues[TIDL_DIM_BATCH] ; batchIdx++ )
    {
        for ( int32_t dim1Idx = 0; dim1Idx < outDataParams->dimValues[TIDL_DIM_DIM1] ; dim1Idx++ )
        {
            for ( int32_t dim2Idx = 0; dim2Idx < outDataParams->dimValues[TIDL_DIM_DIM2]; dim2Idx++ )
            {
                for ( int32_t chIdx = 0; chIdx < outDataParams->dimValues[TIDL_DIM_NUMCH]; chIdx++ )
                {
                    for ( int32_t heightIdx = 0; heightIdx < outDataParams->dimValues[TIDL_DIM_HEIGHT]; heightIdx++)
                    {
                        for ( int32_t widthIdx = 0; widthIdx < outDataParams->dimValues[TIDL_DIM_WIDTH]; widthIdx++)
                        {
                            int32_t inOffsetA = (batchIdx * inbatchPitchA) + (dim1Idx * inDIM1PitchA) + (dim2Idx * inDIM2PitchA) + (chIdx * inchPitchA) + (heightIdx * inheightPitchA) + (widthIdx * inwidthPitchA);
                            int32_t inOffsetB = (batchIdx * inbatchPitchB) + (dim1Idx * inDIM1PitchB) + (dim2Idx * inDIM2PitchB) + (chIdx * inchPitchB) + (heightIdx * inheightPitchB) + (widthIdx * inwidthPitchB);
                            int32_t outOffset = (batchIdx * outbatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outchPitch) + (heightIdx * outheightPitch) + (widthIdx * outwidthPitch);
                            if ( logicalOperatorType == TIDL_And)
                            {
                                outPtr[outOffset] = inPtrA[inOffsetA] && inPtrB[inOffsetB];
                            }
                            else if ( logicalOperatorType == TIDL_Or)
                            {   
                                outPtr[outOffset] = inPtrA[inOffsetA] || inPtrB[inOffsetB];
                            }
                            else if ( logicalOperatorType == TIDL_Xor)
                            {
                                outPtr[outOffset] = inPtrA[inOffsetA] ^ inPtrB[inOffsetB];
                            }
                        }
                    }
                }
            }
        }
    } 
    return 0;
}

template <class Tin>
int32_t TIDL_Not_IsInf_IsNan_RefProcess(sTIDL_DataParams_t * inDataParamsA,
                            sTIDL_DataParams_t * outDataParams,
                            Tin *inPtrA,
                            bool *outPtr, sTIDL_LogicalOpLayerParams_t  * params)
{
    int32_t inbatchPitchA,inDIM1PitchA,inDIM2PitchA,inchPitchA,inheightPitchA,inwidthPitchA;
    int32_t outbatchPitch,outDIM1Pitch,outDIM2Pitch,outchPitch,outheightPitch,outwidthPitch;
    int32_t booleanOperatorType = params->operatorType;

    TIDL_ComputeBroadcastPitches(inDataParamsA, &inwidthPitchA, &inheightPitchA, &inchPitchA, &inDIM1PitchA, &inDIM2PitchA, &inbatchPitchA);
    TIDL_ComputeBroadcastPitches(outDataParams, &outwidthPitch, &outheightPitch, &outchPitch, &outDIM1Pitch, &outDIM2Pitch, &outbatchPitch);


    for ( int32_t batchIdx = 0; batchIdx < outDataParams->dimValues[TIDL_DIM_BATCH] ; batchIdx++ )
    {
        for ( int32_t dim1Idx = 0; dim1Idx < outDataParams->dimValues[TIDL_DIM_DIM1] ; dim1Idx++ )
        {
            for ( int32_t dim2Idx = 0; dim2Idx < outDataParams->dimValues[TIDL_DIM_DIM2]; dim2Idx++ )
            {
                for ( int32_t chIdx = 0; chIdx < outDataParams->dimValues[TIDL_DIM_NUMCH]; chIdx++ )
                {
                    for ( int32_t heightIdx = 0; heightIdx < outDataParams->dimValues[TIDL_DIM_HEIGHT]; heightIdx++)
                    {
                        for ( int32_t widthIdx = 0; widthIdx < outDataParams->dimValues[TIDL_DIM_WIDTH]; widthIdx++)
                        {
                            int32_t inOffsetA = (batchIdx * inbatchPitchA) + (dim1Idx * inDIM1PitchA) + (dim2Idx * inDIM2PitchA) + (chIdx * inchPitchA) + (heightIdx * inheightPitchA) + (widthIdx * inwidthPitchA);
                            int32_t outOffset = (batchIdx * outbatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outchPitch) + (heightIdx * outheightPitch) + (widthIdx * outwidthPitch);
                            if (booleanOperatorType == TIDL_Not)
                            {
                                outPtr[outOffset] = !inPtrA[inOffsetA];
                            }
                            else if (booleanOperatorType == TIDL_IsInf)
                            {
                                if ( (params->detect_negative == 1) && (params->detect_positive == 1) )
                                {
                                    outPtr[outOffset] = std::isinf(inPtrA[inOffsetA]);
                                }
                                else if ((params->detect_negative == 0) && (params->detect_positive == 1))
                                {
                                    outPtr[outOffset] = std::isinf(inPtrA[inOffsetA]) && inPtrA[inOffsetA] > 0.0f;
                                }
                                else if ((params->detect_negative == 1) && (params->detect_positive == 0))
                                {
                                    outPtr[outOffset] = std::isinf(inPtrA[inOffsetA]) && inPtrA[inOffsetA] < 0.0f;
                                }
                                else
                                {
                                    outPtr[outOffset] = false;
                                }
                            }
                            else if (booleanOperatorType == TIDL_IsNaN)
                            {
                                outPtr[outOffset] = std::isnan(inPtrA[inOffsetA]);
                            }
                        }
                    }
                }
            }
        }
    } 
    return 0;
}

template <class Tin>
int32_t TIDL_Where_RefProcess(sTIDL_DataParams_t * inDataParamsCondition,
                            sTIDL_DataParams_t * inDataParamsX,
                            sTIDL_DataParams_t * inDataParamsY,
                            sTIDL_DataParams_t * outDataParams,
                            bool *inPtrCondition,
                            Tin *inPtrX,
                            Tin *inPtrY,
                            Tin *outPtr, sTIDL_LogicalOpLayerParams_t  * params)
{
    int32_t inbatchPitchCondition,inDIM1PitchCondition,inDIM2PitchCondition,inchPitchCondition,inheightPitchCondition,inwidthPitchCondition;
    int32_t inbatchPitchX,inDIM1PitchX,inDIM2PitchX,inchPitchX,inheightPitchX,inwidthPitchX;
    int32_t inbatchPitchY,inDIM1PitchY,inDIM2PitchY,inchPitchY,inheightPitchY,inwidthPitchY;
    int32_t outbatchPitch,outDIM1Pitch,outDIM2Pitch,outchPitch,outheightPitch,outwidthPitch;
    int32_t booleanOperatorType = params->operatorType;

    TIDL_ComputeBroadcastPitches(inDataParamsCondition, &inwidthPitchCondition, &inheightPitchCondition, &inchPitchCondition, &inDIM1PitchCondition, &inDIM2PitchCondition, &inbatchPitchCondition);
    TIDL_ComputeBroadcastPitches(inDataParamsX, &inwidthPitchX, &inheightPitchX, &inchPitchX, &inDIM1PitchX, &inDIM2PitchX, &inbatchPitchX);
    TIDL_ComputeBroadcastPitches(inDataParamsY, &inwidthPitchY, &inheightPitchY, &inchPitchY, &inDIM1PitchY, &inDIM2PitchY, &inbatchPitchY);
    TIDL_ComputeBroadcastPitches(outDataParams, &outwidthPitch, &outheightPitch, &outchPitch, &outDIM1Pitch, &outDIM2Pitch, &outbatchPitch);


    for ( int32_t batchIdx = 0; batchIdx < outDataParams->dimValues[TIDL_DIM_BATCH] ; batchIdx++ )
    {
        for ( int32_t dim1Idx = 0; dim1Idx < outDataParams->dimValues[TIDL_DIM_DIM1] ; dim1Idx++ )
        {
            for ( int32_t dim2Idx = 0; dim2Idx < outDataParams->dimValues[TIDL_DIM_DIM2]; dim2Idx++ )
            {
                for ( int32_t chIdx = 0; chIdx < outDataParams->dimValues[TIDL_DIM_NUMCH]; chIdx++ )
                {
                    for ( int32_t heightIdx = 0; heightIdx < outDataParams->dimValues[TIDL_DIM_HEIGHT]; heightIdx++)
                    {
                        for ( int32_t widthIdx = 0; widthIdx < outDataParams->dimValues[TIDL_DIM_WIDTH]; widthIdx++)
                        {
                            int32_t inOffsetCondition = (batchIdx * inbatchPitchCondition) + (dim1Idx * inDIM1PitchCondition) + (dim2Idx * inDIM2PitchCondition) + (chIdx * inchPitchCondition) + (heightIdx * inheightPitchCondition) + (widthIdx * inwidthPitchCondition);
                            int32_t inOffsetX = (batchIdx * inbatchPitchX) + (dim1Idx * inDIM1PitchX) + (dim2Idx * inDIM2PitchX) + (chIdx * inchPitchX) + (heightIdx * inheightPitchX) + (widthIdx * inwidthPitchX);
                            int32_t inOffsetY = (batchIdx * inbatchPitchY) + (dim1Idx * inDIM1PitchY) + (dim2Idx * inDIM2PitchY) + (chIdx * inchPitchY) + (heightIdx * inheightPitchY) + (widthIdx * inwidthPitchY);
                            int32_t outOffset = (batchIdx * outbatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outchPitch) + (heightIdx * outheightPitch) + (widthIdx * outwidthPitch);
                            
                            outPtr[outOffset] = (inPtrCondition[inOffsetCondition]) ? inPtrX[inOffsetX] : inPtrY[inOffsetY];
                            
                        }
                    }
                }
            }
        }
    } 
    return 0;

}

template <class Tin>
int32_t TIDL_ComparisonOperatorRefProcess(sTIDL_DataParams_t * inDataParamsA,
                            sTIDL_DataParams_t * inDataParamsB,
                            sTIDL_DataParams_t * outDataParams,
                            Tin *inPtrA,
                            Tin *inPtrB,
                            bool *outPtr, sTIDL_LogicalOpLayerParams_t  * params)
{
    int32_t inbatchPitchA,inDIM1PitchA,inDIM2PitchA,inchPitchA,inheightPitchA,inwidthPitchA;
    int32_t inbatchPitchB,inDIM1PitchB,inDIM2PitchB,inchPitchB,inheightPitchB,inwidthPitchB;
    int32_t outbatchPitch,outDIM1Pitch,outDIM2Pitch,outchPitch,outheightPitch,outwidthPitch;
    int32_t booleanOperatorType = params->operatorType;

    TIDL_ComputeBroadcastPitches(inDataParamsA, &inwidthPitchA, &inheightPitchA, &inchPitchA, &inDIM1PitchA, &inDIM2PitchA, &inbatchPitchA);
    TIDL_ComputeBroadcastPitches(inDataParamsB, &inwidthPitchB, &inheightPitchB, &inchPitchB, &inDIM1PitchB, &inDIM2PitchB, &inbatchPitchB);
    TIDL_ComputeBroadcastPitches(outDataParams, &outwidthPitch, &outheightPitch, &outchPitch, &outDIM1Pitch, &outDIM2Pitch, &outbatchPitch);


    for ( int32_t batchIdx = 0; batchIdx < outDataParams->dimValues[TIDL_DIM_BATCH] ; batchIdx++ )
    {
        for ( int32_t dim1Idx = 0; dim1Idx < outDataParams->dimValues[TIDL_DIM_DIM1] ; dim1Idx++ )
        {
            for ( int32_t dim2Idx = 0; dim2Idx < outDataParams->dimValues[TIDL_DIM_DIM2]; dim2Idx++ )
            {
                for ( int32_t chIdx = 0; chIdx < outDataParams->dimValues[TIDL_DIM_NUMCH]; chIdx++ )
                {
                    for ( int32_t heightIdx = 0; heightIdx < outDataParams->dimValues[TIDL_DIM_HEIGHT]; heightIdx++)
                    {
                        for ( int32_t widthIdx = 0; widthIdx < outDataParams->dimValues[TIDL_DIM_WIDTH]; widthIdx++)
                        {
                            int32_t inOffsetA = (batchIdx * inbatchPitchA) + (dim1Idx * inDIM1PitchA) + (dim2Idx * inDIM2PitchA) + (chIdx * inchPitchA) + (heightIdx * inheightPitchA) + (widthIdx * inwidthPitchA);
                            int32_t inOffsetB = (batchIdx * inbatchPitchB) + (dim1Idx * inDIM1PitchB) + (dim2Idx * inDIM2PitchB) + (chIdx * inchPitchB) + (heightIdx * inheightPitchB) + (widthIdx * inwidthPitchB);
                            int32_t outOffset = (batchIdx * outbatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outchPitch) + (heightIdx * outheightPitch) + (widthIdx * outwidthPitch);

                            if ( booleanOperatorType == TIDL_Equal)
                            {
                                outPtr[outOffset] = inPtrA[inOffsetA] == inPtrB[inOffsetB];
                            }
                            else if ( booleanOperatorType == TIDL_Greater)
                            {   
                                outPtr[outOffset] = inPtrA[inOffsetA] > inPtrB[inOffsetB];
                            }
                            else if ( booleanOperatorType == TIDL_GreaterOrEqual)
                            {
                                outPtr[outOffset] = inPtrA[inOffsetA] >= inPtrB[inOffsetB];
                            }
                            else if ( booleanOperatorType == TIDL_Less)
                            {
                                outPtr[outOffset] = inPtrA[inOffsetA] < inPtrB[inOffsetB];
                            }
                            else if ( booleanOperatorType == TIDL_LessOrEqual)
                            {
                                outPtr[outOffset] = inPtrA[inOffsetA] <= inPtrB[inOffsetB];
                            }
                            
                        }
                    }
                }
            }
        }
    } 
    return 0;

}

int32_t TIDL_LogicalOpLayerRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
    int32_t status = IALG_EOK;

    sTIDL_LogicalOpLayerParams_t  * params    = &tidlLayer->layerParams.logicalOpLayerParams;

    sTIDL_DataParams_t *inDataParamsA  = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;

    sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;

    int32_t booleanOperatorType   = params->operatorType;

    if (booleanOperatorType == TIDL_Not)
    {
        status = TIDL_Not_IsInf_IsNan_RefProcess(
                                            inDataParamsA,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (bool *) outPtrs[0],
                                            params
                                            );
    }

    else if (booleanOperatorType == TIDL_IsInf ||  booleanOperatorType == TIDL_IsNaN)
    {
        if(inDataParamsA->elementType == TIDL_SinglePrecFloat)
        {
            status = TIDL_Not_IsInf_IsNan_RefProcess(
                                                inDataParamsA,
                                                outDataParams,
                                                (float32_tidl *)inPtrs[0],
                                                (bool *) outPtrs[0],
                                                params
                                                );
        }
        else
        {
            tidl_printf(0,"TIDL_LogicalOpLayer inElementType is  not supported !!!\n ");
            status = TIDL_ERR_FAILURE;
        }
    }

    else if (booleanOperatorType == TIDL_Where)
    {
        sTIDL_DataParams_t *inDataParamsB  = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[1]].outData;
        sTIDL_DataParams_t *inDataParamsC  = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[2]].outData;
        if (outDataParams->elementType == TIDL_UnsignedChar)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (uint8_t *)inPtrs[1],
                                            (uint8_t *)inPtrs[2],
                                            (uint8_t *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_SignedChar)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (int8_t *)inPtrs[1],
                                            (int8_t *)inPtrs[2],
                                            (int8_t *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_UnsignedShort)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (uint16_t *)inPtrs[1],
                                            (uint16_t *)inPtrs[2],
                                            (uint16_t *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_SignedShort)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (int16_t *)inPtrs[1],
                                            (int16_t *)inPtrs[2],
                                            (int16_t *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_UnsignedWord)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (uint32_t *)inPtrs[1],
                                            (uint32_t *)inPtrs[2],
                                            (uint32_t *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_SignedWord)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (int32_t *)inPtrs[1],
                                            (int32_t *)inPtrs[2],
                                            (int32_t *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_SinglePrecFloat)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (float32_tidl *)inPtrs[1],
                                            (float32_tidl *)inPtrs[2],
                                            (float32_tidl *) outPtrs[0],
                                            params
                                            );
    
        }
        else if (outDataParams->elementType == TIDL_Bool)
        {
            status = TIDL_Where_RefProcess( inDataParamsA,
                                            inDataParamsB,
                                            inDataParamsC,
                                            outDataParams,
                                            (bool *)inPtrs[0],
                                            (bool *)inPtrs[1],
                                            (bool *)inPtrs[2],
                                            (bool *) outPtrs[0],
                                            params
                                            );
    
        }
        else
        {
            tidl_printf(0,"TIDL_Where inElementType is  not supported !!!\n ");
            status = TIDL_ERR_FAILURE;
        }
    }

    else if (booleanOperatorType == TIDL_And || booleanOperatorType == TIDL_Or || booleanOperatorType == TIDL_Xor)
    {
        sTIDL_DataParams_t *inDataParamsB  = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[1]].outData;
        status = TIDL_LogicalOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (bool *)inPtrs[0],
                                                (bool *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                booleanOperatorType
                                                );
    }
    
    else if (booleanOperatorType == TIDL_Equal || booleanOperatorType == TIDL_Greater || booleanOperatorType == TIDL_GreaterOrEqual || booleanOperatorType == TIDL_Less || booleanOperatorType == TIDL_LessOrEqual)
    {
        sTIDL_DataParams_t *inDataParamsB  = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[1]].outData;
        if (inDataParamsA->elementType == TIDL_UnsignedChar)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (uint8_t *)inPtrs[0],
                                                (uint8_t *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_SignedChar)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (int8_t *)inPtrs[0],
                                                (int8_t *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_UnsignedShort)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (uint16_t *)inPtrs[0],
                                                (uint16_t *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_SignedShort)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (int16_t *)inPtrs[0],
                                                (int16_t *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_UnsignedWord)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (uint32_t *)inPtrs[0],
                                                (uint32_t *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_SignedWord)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (int32_t *)inPtrs[0],
                                                (int32_t *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_SinglePrecFloat)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (float32_tidl *)inPtrs[0],
                                                (float32_tidl *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else if (inDataParamsA->elementType == TIDL_Bool)
        {
            status = TIDL_ComparisonOperatorRefProcess(
                                                inDataParamsA,
                                                inDataParamsB,
                                                outDataParams,
                                                (bool *)inPtrs[0],
                                                (bool *)inPtrs[1],
                                                (bool *) outPtrs[0],
                                                params
                                                );
    
        }
        else
        {
            tidl_printf(0,"TIDL_LogicalOpLayer inElementType is  not supported !!!\n ");
            status = TIDL_ERR_FAILURE;
        }

    }
    else
    {
        tidl_printf(0,"Invalid LogicalOpLayer operator type !!!\n ");
        status = TIDL_ERR_FAILURE;
    }


    return status;
    
    

  
}
