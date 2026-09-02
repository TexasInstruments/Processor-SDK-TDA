/*
 *
 * Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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
 * ----------------------------------------------------------------------------
 * @file    tidl_shapeInference.c
 * @brief   Shape Inference Framework — implementation
 *
 * @version 1.0 (Mar 2026) : Initial version
 * ----------------------------------------------------------------------------
 */

#include <string.h>
#include "tidl_shapeInference.h"
#include "tidl_common_utils_infer_import.h"
#ifdef HOST_EMULATION
#include "ti_dl.h"
#include "tidl_import_config.h"
#endif

/* =========================================================================
 * Per-layer shape inference implementations
 * ========================================================================= */

/* -------------------------------------------------------------------------
 * PassThrough - Output shape == first input shape.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_PassThrough(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)layerParams;
    (void)context;


    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Reshape (dynamic shape support)
 *
 * Recomputes output dims that were -1 or 0 (passthrough) in the original
 * shape spec, using TIDL 6D params baked in at import time:
 *   reshapeParams.passthroughMask  — bits set → copy in[d] to out[d]
 *   reshapeParams.minusOneDimIdx   — index of the inferred dim; -1 if none
 *
 * All other output dims keep their import-time values (from the temp copy
 * provided by TIDL_inferShapeGeneric).
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Reshape(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)numInBufs;
    (void)context;

    int32_t minusOneDimIdx  = layerParams->reshapeParams.minusOneDimIdx;
    int32_t passthroughMask = layerParams->reshapeParams.passthroughMask;
    int32_t d;

    /* Step 1: apply passthrough dims */
    for (d = 0; d < TIDL_DIM_MAX; d++)
    {
        if ((passthroughMask >> d) & 1)
        {
            outDataParam->dimValues[d] = inDataParams[0]->dimValues[d];
        }
    }

    /* Step 2: recompute the inferred (-1) dim */
    if (minusOneDimIdx >= 0)
    {
        int32_t totalInputVol    = 1;
        int32_t partialOutputVol = 1;

        for (d = 0; d < TIDL_DIM_MAX; d++)
        {
            totalInputVol *= inDataParams[0]->dimValues[d];
        }
        for (d = 0; d < TIDL_DIM_MAX; d++)
        {
            if (d != minusOneDimIdx)
            {
                partialOutputVol *= outDataParam->dimValues[d];
            }
        }
        if (partialOutputVol > 0)
        {
            outDataParam->dimValues[minusOneDimIdx] = totalInputVol / partialOutputVol;
        }
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

// Remove this if the operator supports dynamic shape during inference
#ifdef HOST_EMULATION
/* -------------------------------------------------------------------------
 * EltWise (broadcast-aware)
 *
 * For each dimension d:
 *     out.dimValues[d] = max(in[0].dimValues[d], in[1].dimValues[d], ...)
 *
 * This correctly handles NumPy-style broadcasting: when one input has
 * size 1 along a dimension and another has size N, the output takes N.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_EltWise(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t d;
    int32_t i;

    (void)layerParams;
    (void)context;


    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    /* For each additional input, take the element-wise maximum per dimension.
     * This implements broadcast semantics: max(1, N) == N. */
    for (i = 1; i < numInBufs; i++)
    {
        for (d = 0; d < TIDL_DIM_MAX; d++)
        {
            int32_t v = inDataParams[i]->dimValues[d];
            if (v > outDataParam->dimValues[d])
            {
                outDataParam->dimValues[d] = v;
            }
        }
    }

    return TIDL_SHAPE_INFERENCE_OK;
}
#endif /* HOST_EMULATION */

/* -------------------------------------------------------------------------
 * Transpose
 *
 * RUNTIME path:
 *   perm[] is already normalized to TIDL_DIM_MAX entries in layerParams.
 *   Applies: out.dimValues[d] = in.dimValues[perm[d]] for d in [0, TIDL_DIM_MAX).
 *
 * COMPILE-TIME path (context != NULL && context->isCompileTime != 0):
 *   The raw permutation is stored in the layer's weights buffer as an
 *   int32_t array of length weights.bufSize (the original model's ndim).
 *   This function:
 *     1. Reads the raw permutation from context->pcParams (sTIDL_LayerPC_t*).
 *     2. Pads it to TIDL_DIM_MAX by prepending identity dims at the front
 *        (matching the logic in TIDL_tfOutReshapeTransposeLayer).
 *     3. Applies the permutation to compute outDataParam->dimValues[].
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Transpose(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t        *perm;
    int32_t        i;


    perm = layerParams->transposeParams.perm;

#ifdef HOST_EMULATION
    if ((context != NULL) && (context->isCompileTime != 0) && (context->pcLayer != NULL))
    {
        // COMPILE-TIME PATH
        const sTIDL_LayerPC_t    *pcLayer       = (const sTIDL_LayerPC_t *)context->pcLayer;
        const tidl_import_config *configParams  = (const tidl_import_config *)context->configParams;
        int32_t                  bufSize        = pcLayer->weights.bufSize;
        const int32_t            *rawPerm       = (const int32_t *)pcLayer->weights.ptr;
        int32_t                  k              = TIDL_DIM_MAX - bufSize;
        int32_t                  dim            = 0;

        if ((rawPerm == NULL) || (bufSize <= 0) || (bufSize > TIDL_DIM_MAX))
        {
            return TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS;
        }

        /* Prepend identity dims for the leading dimensions not covered by
         * the original model permutation (e.g. a 4D perm padded to 6D). */
        for (i = 0; i < k; i++)
        {
            perm[i] = dim++;
            outDataParam->dimValues[i] = 1;
        }

        /* Append the original permutation, offset by k. */
        for (i = 0; i < bufSize; i++)
        {
            perm[k + i] = rawPerm[i] + k;
            outDataParam->dimValues[k + i] = inDataParams[0]->dimValues[perm[k + i]];
        }

        if ((configParams->inferenceMode == TIDL_inferenceModeHighThroughput) ||
            (pcLayer->isBatchGroupLayer == 1))
        {
            outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
        }
    }
    else
#endif /* HOST_EMULATION */
    {
        // RUNTIME PATH
        for (i = 0; i < TIDL_DIM_MAX; i++)
        {
            outDataParam->dimValues[i] = inDataParams[0]->dimValues[perm[i]];
        }
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * NonZero
 *
 * Infers the output shape for the NonZero operator.
 *
 * The output is a 2-D tensor of shape [TIDL_DIM_MAX, N], where N is the
 * total number of elements in the input tensor (product of all input
 * dimensions).  The actual non-zero count is not known at compile time, so
 * width dimension is marked as dynamic (dynDimMask = 0x20).
 *
 * HOST_EMULATION compile-time path:
 *   When running under host emulation with a resolved layer context, the
 *   output dimensions are set conservatively: HEIGHT is fixed to TIDL_DIM_MAX
 *   (the number of input dimensions) and WIDTH is set to the flat element
 *   count of the input tensor, representing the worst-case number of
 *   non-zero indices.
 * ------------------------------------------------------------------------- */

int32_t TIDL_shapeInfer_NonZero(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context
)
{

    #ifdef HOST_EMULATION
    if ((context != NULL) && (context->isCompileTime != 0) && (context->pcLayer != NULL))
    {
        const sTIDL_LayerPC_t *pcLayer = (const sTIDL_LayerPC_t *)context->pcLayer;
        outDataParam->dimValues[TIDL_DIM_BATCH]  = 1;
        outDataParam->dimValues[TIDL_DIM_DIM1]   = 1;
        outDataParam->dimValues[TIDL_DIM_DIM2]   = 1;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = 1;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = pcLayer->layerParams.nonZeroParams.numValidInputDims;
        outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH] *
                                                    inDataParams[0]->dimValues[TIDL_DIM_DIM1] *
                                                    inDataParams[0]->dimValues[TIDL_DIM_DIM2] *
                                                    inDataParams[0]->dimValues[TIDL_DIM_NUMCH] *
                                                    inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] *
                                                    inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
    }
    #endif /* HOST_EMULATION */

    outDataParam->dynDimMask = 0x20;
    
    return TIDL_SHAPE_INFERENCE_OK;

}

// Remove this if the operator supports dynamic shape during inference
#ifdef HOST_EMULATION
/* -------------------------------------------------------------------------
 * Resize
 *
 * Scales the spatial dimensions (HEIGHT, WIDTH) of the input tensor by the
 * resize ratios stored in resizeParams.resizeRatio[].
 *
 * Negative ratio convention (compile-time only):
 *   A negative ratio encodes an absolute output size as:
 *       ratio = -(outputSize)
 *   At compile-time this function normalises it in-place to a true ratio:
 *       ratio = outputSize / inputDim
 *   so that the stored ratio is correct for runtime use.
 *
 * All other dimensions (BATCH, DIM1, DIM2, NUMCH) are copied from input[0].
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Resize(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)context;


    /* Negative ratios encode absolute output sizes — compile-time import convention only.
     * At runtime ratios are always positive */
#ifdef HOST_EMULATION
    if (layerParams->resizeParams.resizeRatio[TIDL_DIM_HEIGHT] < 0.0f ||
        layerParams->resizeParams.resizeRatio[TIDL_DIM_WIDTH]  < 0.0f)
    {
        layerParams->resizeParams.resizeRatio[TIDL_DIM_HEIGHT] =
            -layerParams->resizeParams.resizeRatio[TIDL_DIM_HEIGHT]
            / (float)inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        layerParams->resizeParams.resizeRatio[TIDL_DIM_WIDTH] =
            -layerParams->resizeParams.resizeRatio[TIDL_DIM_WIDTH]
            / (float)inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
    }
#endif /* HOST_EMULATION */

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = (int32_t)((float)inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] * layerParams->resizeParams.resizeRatio[TIDL_DIM_HEIGHT]);
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = (int32_t)((float)inDataParams[0]->dimValues[TIDL_DIM_WIDTH]  * layerParams->resizeParams.resizeRatio[TIDL_DIM_WIDTH]);

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * DeformConv (Deformable Convolution)
 *
 * Computes the output spatial dimensions of a deformable convolution:
 *   BATCH, DIM1, DIM2 : copied from input[0]
 *   NUMCH             : deformConvParams.numOutChannels
 *   HEIGHT            : ((inDim[NUMCH]  + 2*padH - ((kernelH-1)*dilH+1)) / strideH) + 1
 *   WIDTH             : ((inDim[HEIGHT] + 2*padW - ((kernelW-1)*dilW+1)) / strideW) + 1
 *
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_DeformConv(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)context;


    const sTIDL_DeformConvParams_t *deformConvParams = &layerParams->deformConvParams;

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = deformConvParams->numOutChannels;
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = ((inDataParams[0]->dimValues[TIDL_DIM_NUMCH] + (deformConvParams->padH * 2) - ((deformConvParams->kernelH - 1) * deformConvParams->dilationH + 1)) / deformConvParams->strideH) + 1;
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = ((inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] + (deformConvParams->padW * 2) - ((deformConvParams->kernelW - 1) * deformConvParams->dilationW + 1)) / deformConvParams->strideW) + 1;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * GridSample
 *
 * Computes the output shape of a grid_sample operation:
 *   BATCH  : from input[0] (the data tensor)
 *   DIM1   : from input[0]
 *   DIM2   : from input[0]
 *   NUMCH  : from input[1] (the grid tensor — encodes output H)
 *   HEIGHT : from input[1] (the grid tensor — encodes output W)
 *   WIDTH  : from input[0] (the data tensor — number of channels)
 *
 * input[0] = data tensor,  input[1] = grid tensor.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_GridSample(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)layerParams;
    (void)context;

    if (numInBufs < 2)
    {
        return TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS;
    }

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[1]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[1]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * GatherElements
 *
 * Computes the output shape of a GatherElements operation:
 * For GatherElements operator Output shape should be equal to the indices shape
 * input[0] = data tensor,  input[1] = indices tensor.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_GatherElements(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)layerParams;
    (void)context;


    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Gather
 *
 * Computes the output shape of a Gather operation along gatherParams.axis:
 *
 *   Case 1 — scalar index (gatherParams.isIdxScalar == 1):
 *     The axis dimension is removed by shifting all dims from [axis] down
 *     by one position (right-shift), and setting dimValues[0] = 1.
 *
 *   Case 2 — tensor index (gatherParams.isIdxScalar == 0):
 *     dimValues[axis] is replaced by inDataParams[1]->dimValues[TIDL_DIM_WIDTH],
 *     which holds the number of indices in the index tensor.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Gather(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t i;

    (void)context;


    const sTIDL_GatherLayerParams_t *gatherParams = &layerParams->gatherParams;

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    if (gatherParams->isIdxScalar == 1)
    {
        for (i = gatherParams->axis; i > 0; i--)
        {
            outDataParam->dimValues[i] = outDataParam->dimValues[i-1];
        }
        outDataParam->dimValues[0] = 1;
    }
    else
    {
        /* Tensor-index case: inDataParams[1] must be valid. */
        if ((numInBufs < 2) || (inDataParams[1] == NULL))
        {
            return TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS;
        }
        outDataParam->dimValues[gatherParams->axis] = inDataParams[1]->dimValues[TIDL_DIM_WIDTH];
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

#endif /* HOST_EMULATION */

/* -------------------------------------------------------------------------
 * GatherND
 *
 * Computes the output shape of a GatherND operation:
 *
 *   Case 1 — Without batchDims (gatherNDParams.batchDims == 0):
 *   output_shape = indices_shape[:-1] + data_shape[indices_shape[-1]:]
 *
 *   Case 1 — With batchDims (gatherNDParams.batchDims > 0):
 *   output_shape = data_shape[:batch_dims] + indices_shape[batch_dims:-1] + data_shape[batch_dims + indices_shape[-1]:]
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_GatherND(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{

    (void)context;

    if (inDataParams[1] == NULL)
    {
        return TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS;
    }

    int32_t batchDims = layerParams->gatherNDParams.batchDims;
    int32_t indicesDimCount = layerParams->gatherNDParams.indicesDimCount;
    int32_t dataDimCount = layerParams->gatherNDParams.dataDimCount;
    int32_t indicesLastDim = inDataParams[1]->dimValues[TIDL_DIM_WIDTH];

    // Calculate total output dimensions
    int32_t totalOutDims = batchDims + (indicesDimCount - 1 - batchDims) + (dataDimCount - batchDims - indicesLastDim);
    
    // Initialize output dimensions to 1
    for (int32_t i = 0; i < TIDL_DIM_MAX; i++)
    {
        outDataParam->dimValues[i] = 1;
    }
    
    // Calculate starting index for output dimensions
    int32_t outputStartIdx = TIDL_DIM_MAX - totalOutDims;
    int32_t currentOutputIdx = outputStartIdx;
    
    // Step 1: Copy batch dimensions from data tensor
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - FUTURE_USE: GatherND with batch_dims > 0 requires dynamic reshape of NonZero output which is not currently supported; this loop is retained for future support when batch_dims > 0 models become available.
    Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
    This does not impact functional correctness or safety.
    <justification end> */
    for (int32_t i = 0; i < batchDims && currentOutputIdx < TIDL_DIM_MAX; i++)
    {
        int32_t dataIdx = TIDL_DIM_MAX - dataDimCount + i;
        outDataParam->dimValues[currentOutputIdx] = inDataParams[0]->dimValues[dataIdx];
        currentOutputIdx++;
    }
    /* LDRA_JUSTIFY_END */
    
    // Step 2: Copy indices dimensions (excluding last dimension, starting from batch_dims)
    for (int32_t i = batchDims; i < indicesDimCount - 1 && currentOutputIdx < TIDL_DIM_MAX; i++)
    {
        int32_t indicesIdx = TIDL_DIM_MAX - indicesDimCount + i;
        outDataParam->dimValues[currentOutputIdx] = inDataParams[1]->dimValues[indicesIdx];
        currentOutputIdx++;
    }
    
    // Step 3: Copy remaining data dimensions
    int32_t remainingDataStart = batchDims + indicesLastDim;
    for (int32_t i = remainingDataStart; i < dataDimCount && currentOutputIdx < TIDL_DIM_MAX; i++)
    {
        int32_t dataIdx = TIDL_DIM_MAX - dataDimCount + i;
        outDataParam->dimValues[currentOutputIdx] = inDataParams[0]->dimValues[dataIdx];
        currentOutputIdx++;
    }
    return TIDL_SHAPE_INFERENCE_OK;
}

// Remove this if the operator supports dynamic shape during inference
#ifdef HOST_EMULATION
/* -------------------------------------------------------------------------
 * Cast
 *
 * Cast only changes the element type of the tensor, not the shape.
 * Output dimensions are identical to the input dimensions.
 * The element type is set by the compile-time wrapper
 * (TIDL_tfOutReshapeCastLayer) and is not updated here.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Cast(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)layerParams;
    (void)context;


    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Attention
 *
 * Attention dimensions are identical to the input dimensions, 
 * other than height and width which is decided by the input Key 
 * and Value dimensions.
 * The element type is set by the compile-time wrapper
 * (TIDL_tfOutReshapeAttentionLayer) and is not updated here.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Attention(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)layerParams;
    (void)context;

    if (numInBufs < 3)
    {
        return TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS;
    }

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[2]->dimValues[TIDL_DIM_WIDTH];

    return TIDL_SHAPE_INFERENCE_OK;
}

#endif /* HOST_EMULATION */

/* -------------------------------------------------------------------------
 * Shape
 *
 * Computes the output shape of an ONNX Shape operation:
 *   Output is a 1-D tensor of width (end - start), where start and end
 *   are taken from shapeParams (already normalised to TIDL-dimension
 *   indices by the importer).
 *
 *   All output dimensions are set to 1 except TIDL_DIM_WIDTH which holds
 *   the count of selected input dimensions: end - start.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Shape(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t i;

    (void)inDataParams;
    (void)numInBufs;
    (void)context;


    for (i = 0; i < (int32_t)TIDL_DIM_MAX; i++)
    {
        outDataParam->dimValues[i] = 1;
    }
    outDataParam->dimValues[TIDL_DIM_WIDTH] =
        layerParams->shapeParams.end - layerParams->shapeParams.start;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Size
 *
 * Computes the output shape of an ONNX Size operation:
 *   The output is a scalar (single element), so all output dimensions are
 *   set to 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Size(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t i;

    (void)layerParams;
    (void)inDataParams;
    (void)numInBufs;
    (void)context;


    for (i = 0; i < (int32_t)TIDL_DIM_MAX; i++)
    {
        outDataParam->dimValues[i] = 1;
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

// Remove this if the operator supports dynamic shape during inference
#ifdef HOST_EMULATION
/* -------------------------------------------------------------------------
 * Tile
 *
 * Computes the output shape of a Tile operation:
 *
 *   For each dimension d in [0, TIDL_DIM_MAX):
 *     out.dimValues[d] = in.dimValues[d] * tileParams.repeats[d]
 *
 *   tileParams.repeats[d] is the integer repetition count along dimension d.
 *   A repeat value of 1 leaves that dimension unchanged.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Tile(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t i;

    (void)context;


    for (i = 0; i < TIDL_DIM_MAX; i++)
    {
        outDataParam->dimValues[i] = inDataParams[0]->dimValues[i] * layerParams->tileParams.repeats[i];
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * LogicalOp (broadcast-aware)
 *
 * Computes the output shape of a logical/comparison operation (And, Or, Not,
 * Where, etc.) across all inputs using broadcast semantics:
 *
 *   For each dimension d in [0, TIDL_DIM_MAX):
 *     out.dimValues[d] = max(in[0].dimValues[d], in[1].dimValues[d], ...)
 *
 * This is identical to EltWise broadcast semantics: when one input has
 * size 1 along a dimension and another has size N, the output takes N.
 *
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_LogicalOp(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    /* LogicalOp shape is identical to EltWise broadcast semantics. */
    return TIDL_shapeInfer_EltWise(layerParams, inDataParams, numInBufs, outDataParam, context);
}

/* -------------------------------------------------------------------------
 * Squeeze
 *
 * Removes dimensions of size 1 from the tensor shape.
 *
 * COMPILE-TIME path (context != NULL && context->isCompileTime != 0):
 *   The axis mask is stored in pcLayer->layerPCParams.squeezeParams.axis[].
 *   axis[i] != 0 means dimension i is squeezed (removed).
 *   Algorithm:
 *     1. Iterate from TIDL_DIM_MAX-1 down to TIDL_DIM_BATCH+1; for each
 *        dimension i where axis[i] == 0 (not squeezed), pack it into the
 *        output from the high end (TIDL_DIM_MAX-1 downward).
 *     2. Fill any remaining leading output slots (below the packed dims)
 *        with 1.
 *     3. Always preserve TIDL_DIM_BATCH from input[0].
 *
 * RUNTIME path (context == NULL or context->isCompileTime == 0):
 *   The axis mask is a PC-only field not available at runtime.
 *   The output shape is fully baked in at import time, so this function
 *   returns TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER to signal that
 *   the static import-time dimValues should be preserved as-is.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Squeeze(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)layerParams;


#ifdef HOST_EMULATION
    /* Non-compile-time (runtime): axis mask is PC-only; output shape is
     * baked in at import. Return UNSUPPORTED so static dims are preserved. */
    if ((context == NULL) || (context->isCompileTime == 0) || (context->pcLayer == NULL))
    {
        return TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER;
    }

    /* Compile-time: use layerPCParams.squeezeParams.axis[] */
    {
        const sTIDL_LayerPC_t *pcLayer = (const sTIDL_LayerPC_t *)context->pcLayer;
        int32_t i;
        int32_t ii = 0;

        for (i = TIDL_DIM_MAX - 1; i > TIDL_DIM_BATCH; i--)
        {
            if (pcLayer->layerPCParams.squeezeParams.axis[i] == 0)
            {
                outDataParam->dimValues[TIDL_DIM_MAX - 1 - ii] = inDataParams[0]->dimValues[i];
                ii++;
            }
        }

        for (; ii < TIDL_DIM_MAX - 1; ii++)
        {
            outDataParam->dimValues[TIDL_DIM_MAX - 1 - ii] = 1;
        }

        outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    }

    return TIDL_SHAPE_INFERENCE_OK;
#else
    return TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER;
#endif /* HOST_EMULATION */
}

/* -------------------------------------------------------------------------
 * SoftMax
 *
 * Output shape is identical to the input shape for all dimensions, with one
 * optional exception: when softMaxParams.outTranspose == 1, the HEIGHT and
 * WIDTH dimensions are swapped in the output.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_SoftMax(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t tmp;

    (void)context;


    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    /* If outTranspose is set, swap HEIGHT and WIDTH in the output. */
    if (layerParams->softMaxParams.outTranspose == 1)
    {
        tmp = outDataParam->dimValues[TIDL_DIM_HEIGHT];
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = outDataParam->dimValues[TIDL_DIM_WIDTH];
        outDataParam->dimValues[TIDL_DIM_WIDTH]  = tmp;
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Concat
 *
 * Concatenates numInBufs inputs along concatParams.axis:
 *   All dims are copied from inDataParams[0], except:
 *     out.dimValues[axis] = sum of inDataParams[j]->dimValues[axis]
 *                           for j in [0, numInBufs)
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Concat(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t totDim;
    int32_t i;

    (void)context;



    for (i = 0; i < TIDL_DIM_MAX; i++)
    {
        outDataParam->dimValues[i] = inDataParams[0]->dimValues[i];
    }

    /* Sum the axis dimension across all inputs. */
    totDim = 0;
    for (i = 0; i < numInBufs; i++)
    {
        totDim += inDataParams[i]->dimValues[layerParams->concatParams.axis];
    }
    outDataParam->dimValues[layerParams->concatParams.axis] = totDim;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * DepthToSpace
 *
 * Rearranges data from depth (channel) into spatial blocks:
 *   BATCH  : copied from input[0]
 *   DIM1   : copied from input[0]
 *   DIM2   : copied from input[0]
 *   NUMCH  : inC / (blockSize * blockSize)
 *   HEIGHT : inH * blockSize
 *   WIDTH  : inW * blockSize
 *
 * blockSize is stored in layerParams.depthToSpaceParams.blockSize.
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_DepthToSpace(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t blockSize;

    (void)context;


    blockSize = layerParams->depthToSpaceParams.blockSize;

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH] / (blockSize * blockSize);
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] * blockSize;
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]  * blockSize;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Pad
 *
 * Adds constant padding to the spatial dimensions:
 *   BATCH  : copied from input[0]
 *   DIM1   : copied from input[0]
 *   DIM2   : copied from input[0]
 *   NUMCH  : copied from input[0]
 *   HEIGHT : inH + padT + padB
 *   WIDTH  : inW + padL + padR
 *
 * Pad amounts are stored in layerParams.padLayerParams.{padT,padB,padL,padR}.
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Pad(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)context;


    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
                                               + layerParams->padLayerParams.padT
                                               + layerParams->padLayerParams.padB;
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
                                               + layerParams->padLayerParams.padL
                                               + layerParams->padLayerParams.padR;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * ArgOp (ArgMax / ArgMin)
 *
 * Reduces the channel dimension to 1; all other spatial dims are preserved:
 *   BATCH  : copied from input[0]
 *   DIM1   : copied from input[0]
 *   DIM2   : copied from input[0]
 *   NUMCH  : 1  (channel dimension is collapsed)
 *   HEIGHT : copied from input[0]
 *   WIDTH  : copied from input[0]
 *
 * The number of input channels is stored back into
 * layerParams.argOpParams.numChannels so the kernel knows the reduction size.
 * This update is performed at both compile-time and runtime so that the kernel
 * always sees the correct channel count even when the input shape changes
 * dynamically.
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_ArgOp(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    (void)context;


    /* Update numChannels so the argop kernel knows the reduction dimension size.
     * This must be kept in sync with the live input shape at runtime. */
    layerParams->argOpParams.numChannels = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = 1;
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * TopK
 *
 *  Computes the output shape of a TopK operation:
 *   All dims are copied from input[0], except:
 *     out.dimValues[topKParams.axis] = topKParams.K
 *
 *          Output Buffer-merging:
 *            TopK provides two outputs (values + indices). In TIDL both the
 *            outputs are merged into on output buffer which is then sliced
 *            using slice operator. This is done as part of optimizer
 *            tidl_handleTopKLayers. This implies the output buffer of TopK
 *            after this optimization (i.e during runtime) is a combination
 *            of values and indices packed into single enlarged buffer.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_TopK(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t i;
    int32_t elementSizeInBits;
    int32_t increment = 1;


    for (i = 0; i < TIDL_DIM_MAX; i++)
    {
        outDataParam->dimValues[i] = inDataParams[0]->dimValues[i];
    }

    outDataParam->dimValues[layerParams->topKParams.axis] = layerParams->topKParams.K;

    /* Determine the buffer-merging increment factor based on element size.
     * Both outputs (values + indices) are packed into one enlarged buffer;
     * the axis dimension is multiplied by this factor to account for both. */
    elementSizeInBits = TIDL_getDatElementSize(outDataParam->elementType) * 8;

    if (elementSizeInBits == 8)
    {
        increment = TOPK_BUFFER_INCREMENT_8BIT;
#ifdef HOST_EMULATION
        if ((context != NULL) && (context->isCompileTime != 0) && (context->configParams != NULL))
        {
            /* At compile-time, use16BitForTopK forces 16-bit index storage
             * even when the data element type is 8-bit. */
            const tidl_import_config *configParams = (const tidl_import_config *)context->configParams;
            if (configParams->use16BitForTopK)
            {
                increment = TOPK_BUFFER_INCREMENT_16BIT;
            }
        }
#endif /* HOST_EMULATION */
    }
    else if (elementSizeInBits == 16)
    {
        increment = TOPK_BUFFER_INCREMENT_16BIT;
    }
    else if (elementSizeInBits == 32)
    {
        increment = TOPK_BUFFER_INCREMENT_32BIT;
    }
    outDataParam->dimValues[layerParams->topKParams.incrementAxis] *= increment;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * LSTM — combined (runtime) output shape
 *
 * This function computes the FINAL shape of the LSTM output buffer as it
 * exists at runtime, AFTER the optimizer tidl_handleRecurrentLayers has
 * combined all per-buffer outputs (full-sequence Y, hidden state Y_h, cell
 * state Y_c) into a single enlarged buffer.
 *
 * The initial per-buffer shapes (before combining) are computed by
 * TIDL_tfOutReshapeLSTMLayer() in tidl_import_common.cpp and are NOT the
 * concern of this function.
 *
 * Combined output shape (layout == 0, seq-first):
 *   BATCH  : from input[0]
 *   DIM1   : from input[0]
 *   DIM2   = seq_length + 2   (full-sequence slot + 2 state slots packed in)
 *   NUMCH  = num_directions
 *   HEIGHT = batch_size
 *   WIDTH  = hidden_size
 *
 * Combined output shape (layout == 1, batch-first):
 *   BATCH  : from input[0]
 *   DIM1   : from input[0]
 *   DIM2   = batch_size
 *   NUMCH  = seq_length + 2   (full-sequence slot + 2 state slots packed in)
 *   HEIGHT = num_directions
 *   WIDTH  = hidden_size
 *
 * num_directions = 2 when lstmParams.direction == TIDL_RecurrentBidirectional,
 * otherwise 1.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_LSTM(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t seq_length, batch_size, hidden_size, num_directions;

    (void)context;


    const sTIDL_LSTMParams_t *lstmParams = &layerParams->lstmParams;

    /* Extract seq_length and batch_size from input shape based on layout.
     * layout == 0: seq-first  [seq_length, batch_size, input_size]
     * layout == 1: batch-first [batch_size, seq_length, input_size] */
    if (lstmParams->layout == 0)
    {
        seq_length = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
        batch_size = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    }
    else
    {
        seq_length = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        batch_size = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    }

    num_directions = (lstmParams->direction == TIDL_RecurrentBidirectional) ? 2 : 1;
    hidden_size    = lstmParams->hidden_size;

    /* BATCH and DIM1 are always inherited from the input. */
    outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]  = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_WIDTH] = hidden_size;

    /* Assign the remaining dims directly from the already-computed locals —
     * no need to re-check layout a second time. */
    if (lstmParams->layout == 0)
    {
        /* Seq-first: DIM2=seq+2, NUMCH=dirs, HEIGHT=batch */
        outDataParam->dimValues[TIDL_DIM_DIM2]   = seq_length + 2;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = num_directions;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = batch_size;
    }
    else
    {
        /* Batch-first: DIM2=batch, NUMCH=seq+2, HEIGHT=dirs */
        outDataParam->dimValues[TIDL_DIM_DIM2]   = batch_size;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = seq_length + 2;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = num_directions;
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * GRU — combined (runtime) output shape
 *
 * This function computes the FINAL shape of the GRU output buffer as it
 * exists at runtime, AFTER the optimizer tidl_handleRecurrentLayers has
 * combined the per-buffer outputs (full-sequence Y and hidden state Y_h)
 * into a single enlarged buffer.
 *
 * The initial per-buffer shapes (before combining) are computed by
 * TIDL_tfOutReshapeGRULayer() in tidl_import_common.cpp and are NOT the
 * concern of this function.
 *
 * Combined output shape (layout == 0, seq-first):
 *   BATCH  : from input[0]
 *   DIM1   : from input[0]
 *   DIM2   = seq_length + 1   (full-sequence slot + 1 state slot packed in)
 *   NUMCH  = num_directions
 *   HEIGHT = batch_size
 *   WIDTH  = hidden_size
 *
 * Combined output shape (layout == 1, batch-first):
 *   BATCH  : from input[0]
 *   DIM1   : from input[0]
 *   DIM2   = batch_size
 *   NUMCH  = seq_length + 1   (full-sequence slot + 1 state slot packed in)
 *   HEIGHT = num_directions
 *   WIDTH  = hidden_size
 *
 * num_directions = 2 when gruParams.direction == TIDL_RecurrentBidirectional,
 * otherwise 1.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_GRU(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t seq_length, batch_size, hidden_size, num_directions;

    (void)context;


    const sTIDL_GRUParams_t *gruParams = &layerParams->gruParams;

    /* Extract seq_length and batch_size from input shape based on layout.
     * layout == 0: seq-first  [seq_length, batch_size, input_size]
     * layout == 1: batch-first [batch_size, seq_length, input_size] */
    if (gruParams->layout == 0)
    {
        seq_length = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
        batch_size = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    }
    else
    {
        seq_length = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        batch_size = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    }

    num_directions = (gruParams->direction == TIDL_RecurrentBidirectional) ? 2 : 1;
    hidden_size    = gruParams->hidden_size;

    /* BATCH and DIM1 are always inherited from the input. */
    outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]  = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_WIDTH] = hidden_size;

    if (gruParams->layout == 0)
    {
        /* Seq-first: DIM2=seq+1, NUMCH=dirs, HEIGHT=batch */
        outDataParam->dimValues[TIDL_DIM_DIM2]   = seq_length + 1;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = num_directions;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = batch_size;
    }
    else
    {
        /* Batch-first: DIM2=batch, NUMCH=seq+1, HEIGHT=dirs */
        outDataParam->dimValues[TIDL_DIM_DIM2]   = batch_size;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = seq_length + 1;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = num_directions;
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * RNN — combined (runtime) output shape
 *
 * This function computes the FINAL shape of the RNN output buffer as it
 * exists at runtime, AFTER the optimizer tidl_handleRecurrentLayers has
 * combined the per-buffer outputs (full-sequence Y and hidden state Y_h)
 * into a single enlarged buffer.
 *
 * The initial per-buffer shapes (before combining) are computed by
 * TIDL_tfOutReshapeRNNLayer() in tidl_import_common.cpp and are NOT the
 * concern of this function.
 *
 * Combined output shape (layout == 0, seq-first):
 *   BATCH  : from input[0]
 *   DIM1   : from input[0]
 *   DIM2   = seq_length + 1   (full-sequence slot + 1 state slot packed in)
 *   NUMCH  = num_directions
 *   HEIGHT = batch_size
 *   WIDTH  = hidden_size
 *
 * Combined output shape (layout == 1, batch-first):
 *   BATCH  : from input[0]
 *   DIM1   : from input[0]
 *   DIM2   = batch_size
 *   NUMCH  = seq_length + 1   (full-sequence slot + 1 state slot packed in)
 *   HEIGHT = num_directions
 *   WIDTH  = hidden_size
 *
 * num_directions = 2 when rnnParams.direction == TIDL_RecurrentBidirectional,
 * otherwise 1.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_RNN(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t seq_length, batch_size, hidden_size, num_directions;

    (void)context;


    const sTIDL_RNNParams_t *rnnParams = &layerParams->rnnParams;

    /* Extract seq_length and batch_size from input shape based on layout.
     * layout == 0: seq-first  [seq_length, batch_size, input_size]
     * layout == 1: batch-first [batch_size, seq_length, input_size] */
    if (rnnParams->layout == 0)
    {
        seq_length = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
        batch_size = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    }
    else
    {
        seq_length = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        batch_size = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    }

    num_directions = (rnnParams->direction == TIDL_RecurrentBidirectional) ? 2 : 1;
    hidden_size    = rnnParams->hidden_size;

    /* BATCH and DIM1 are always inherited from the input. */
    outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]  = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_WIDTH] = hidden_size;

    if (rnnParams->layout == 0)
    {
        /* Seq-first: DIM2=seq+1, NUMCH=dirs, HEIGHT=batch */
        outDataParam->dimValues[TIDL_DIM_DIM2]   = seq_length + 1;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = num_directions;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = batch_size;
    }
    else
    {
        /* Batch-first: DIM2=batch, NUMCH=seq+1, HEIGHT=dirs */
        outDataParam->dimValues[TIDL_DIM_DIM2]   = batch_size;
        outDataParam->dimValues[TIDL_DIM_NUMCH]  = seq_length + 1;
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = num_directions;
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Reduce (ReduceSum / ReduceMean / ReduceMax / ReduceMin / etc.)
 *
 * Reduces the input tensor along a single axis:
 *   All dims are copied from input[0].
 *   dimValues[axis] is set to 1.
 *   If keepDims == 0: dims from [axis] down to [1] are right-shifted
 *     (dimValues[i] = dimValues[i-1] for i from axis down to 1),
 *     and numDim is decremented by 1.
 *
 * COMPILE-TIME path (HOST_EMULATION, context->isCompileTime != 0):
 *   axis and keepDims are read from:
 *     - pcLayer->layerParams.reduceParams        (TIDL_ReduceLayer)
 *     - pcLayer->layerPCParams.reduceSumParams   (TIDL_ReduceSumLayer)
 *     - pcLayer->layerPCParams.reduceMeanParams  (all other reduce variants)
 *   Returns TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER if context is
 *   NULL or not compile-time.
 *
 * RUNTIME path (non-HOST_EMULATION):
 *   axis and keepDims are read from layerParams->reduceParams.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Reduce(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t i;
    int32_t keepDims;
    int32_t axis;


#ifdef HOST_EMULATION
    if ((context != NULL) && (context->isCompileTime != 0))
    {
        const sTIDL_LayerPC_t *pcLayer = (const sTIDL_LayerPC_t *)context->pcLayer;
        if (pcLayer->layerType == TIDL_ReduceLayer)
        {
            keepDims = pcLayer->layerParams.reduceParams.keepDims;
            axis = pcLayer->layerParams.reduceParams.axis;
        }
        else if (pcLayer->layerType == TIDL_ReduceSumLayer)
        {
            keepDims = pcLayer->layerPCParams.reduceSumParams.reduceDims;
            axis = pcLayer->layerPCParams.reduceSumParams.axis;
        }
        else
        {
            keepDims = pcLayer->layerPCParams.reduceMeanParams.reduceDims;
            axis = pcLayer->layerPCParams.reduceMeanParams.axis;
        }
    }
    else
#endif /* HOST_EMULATION */
    {
        keepDims = layerParams->reduceParams.keepDims;
        axis = layerParams->reduceParams.axis;
    }

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    outDataParam->dimValues[axis] = 1;
    outDataParam->numDim =  inDataParams[0]->numDim;

    if(keepDims == 0)
    {
        for(i = axis; i > 0 ; i--)
        {
            outDataParam->dimValues[i] = outDataParam->dimValues[i-1];
        }
        outDataParam->numDim -= 1;
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * InnerProduct
 *
 * Computes the output shape of an InnerProduct layer.
 *
 * COMPILE-TIME path (HOST_EMULATION):
 *   Handled explicitly by TIDL_tfOutReshapeIPLayer in tidl_import_common.cpp.
 *   This function only updates numInRows (always) and numOutCols / numInCols
 *   when they are currently -1 or 0 (not yet set by the importer).
 *
 * RUNTIME path (non-HOST_EMULATION):
 *   numInRows, numInCols, and numOutCols are derived from the live input
 *   tensor shapes, respecting the inputATranspose / inputBTranspose flags:
 *
 *   inputATranspose == 0 (default): A stored as [numOutRows × numInCols]
 *     → numInRows  = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
 *     → numInCols  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
 *   inputATranspose != 0: A stored transposed as [numInCols × numOutRows]
 *     → numInRows  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
 *     → numInCols  = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
 *
 *   inputBTranspose == 0 (default): B stored as [numInCols × numOutCols]
 *     → numOutCols = inDataParams[1]->dimValues[TIDL_DIM_WIDTH]
 *   inputBTranspose != 0: B stored transposed as [numOutCols × numInCols]
 *     → numOutCols = inDataParams[1]->dimValues[TIDL_DIM_HEIGHT]
 *
 * Output shape (both paths):
 *   BATCH, DIM1, DIM2, NUMCH = element-wise max of input[0] and input[1]
 *   HEIGHT = numInRows
 *   WIDTH  = numOutCols
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_InnerProduct(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t d;

    (void)context;


    sTIDL_InnerProductParams_t *innerProductParams = &layerParams->innerProductParams;

#ifdef HOST_EMULATION
    if ((context != NULL) && (context->isCompileTime != 0))
    {
        /* Compile-time path: importer handles shape explicitly; only fill in
         * fields that have not been set yet (sentinel value -1 or 0). */
        innerProductParams->numInRows = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        if (innerProductParams->numOutCols == -1 || innerProductParams->numOutCols == 0)
        {
            if ((numInBufs >= 2) && (inDataParams[1] != NULL))
            {
                innerProductParams->numOutCols = inDataParams[1]->dimValues[TIDL_DIM_WIDTH];
            }
        }
        if (innerProductParams->numInCols == -1 || innerProductParams->numInCols == 0)
        {
            innerProductParams->numInCols = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
        }
    }
    else
#endif /* HOST_EMULATION */
    {
        /* Runtime path (DSP or host-emulation with context == NULL):
         * derive all three fields from live input shapes, respecting
         * the transpose flags stored in innerProductParams. */
        if (innerProductParams->inputATranspose != 0)
        {
            /* A is stored transposed: [numInCols × numOutRows] */
            innerProductParams->numInRows = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
            innerProductParams->numInCols = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        }
        else
        {
            /* A is stored normally: [numOutRows × numInCols] */
            innerProductParams->numInRows = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
            innerProductParams->numInCols = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
        }
        if ((numInBufs >= 2) && (inDataParams[1] != NULL))
        {
            if (innerProductParams->inputBTranspose != 0)
            {
                /* B is stored transposed: [numOutCols × numInCols] */
                innerProductParams->numOutCols = inDataParams[1]->dimValues[TIDL_DIM_HEIGHT];
            }
            else
            {
                /* B is stored normally: [numInCols × numOutCols] */
                innerProductParams->numOutCols = inDataParams[1]->dimValues[TIDL_DIM_WIDTH];
            }
        }
    }

    if ((numInBufs >= 2) && (inDataParams[1] != NULL))
    {
        for (d = 0; d < TIDL_DIM_HEIGHT; d++)
        {
            int32_t v0 = inDataParams[0]->dimValues[d];
            int32_t v1 = inDataParams[1]->dimValues[d];
            outDataParam->dimValues[d] = (v1 > v0) ? v1 : v0;
        }
    }
    else
    {
        for (d = 0; d < TIDL_DIM_HEIGHT; d++)
        {
            outDataParam->dimValues[d] =  inDataParams[0]->dimValues[d];
        }
    }

    /* Override HEIGHT and WIDTH with the computed FC dimensions. */
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = innerProductParams->numInRows;
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = innerProductParams->numOutCols;

    innerProductParams->numBatches = outDataParam->dimValues[TIDL_DIM_BATCH];

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Deconv2D (Transposed Convolution)
 *
 * Computes the output spatial dimensions of a transposed convolution:
 *   BATCH, DIM1, DIM2 : copied from input[0]
 *   NUMCH             : convParams.numOutChannels
 *   HEIGHT            : strideH * (inH - 1) + outPadH + (dilationH*(kernelH-1)+1) - (padT+padB)
 *   WIDTH             : strideW * (inW - 1) + outPadW + (dilationW*(kernelW-1)+1) - (padL+padR)
 *
 * outPadH / outPadW are PC-only fields (sTIDL_ConvPCParams_t.outPadH/outPadW).
 * At runtime these are always 0 (the output size is fully determined by the
 * other parameters).
 *
 * Negative dimValues convention (compile-time only):
 *   When the importer already knows the exact output size it stores it as
 *   -(absoluteOutputSize) in outDataParam->dimValues[HEIGHT/WIDTH].
 *   This function detects the negative sentinel and flips the sign to recover
 *   the actual size, bypassing the formula.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Deconv2D(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    int32_t outPadH = 0;
    int32_t outPadW = 0;
    const sTIDL_ConvParams_t *convParams;


    convParams = &layerParams->convParams;

#ifdef HOST_EMULATION
    if ((context != NULL) && (context->isCompileTime != 0) && (context->pcLayer != NULL))
    {
        const sTIDL_LayerPC_t *pcLayer = (const sTIDL_LayerPC_t *)context->pcLayer;
        outPadH = pcLayer->layerPCParams.convParams.outPadH;
        outPadW = pcLayer->layerPCParams.convParams.outPadW;
    }

    // TODO: Validate if outPadH and outPadW is 0 at runtime

#endif /* HOST_EMULATION */

    outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]  = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]  = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH] = convParams->numOutChannels;

    /* Negative sentinel: importer pre-set the output size as -(absoluteSize).
     * Flip sign to recover the actual output size and skip the formula. */
    if ((outDataParam->dimValues[TIDL_DIM_HEIGHT] < 0) ||
        (outDataParam->dimValues[TIDL_DIM_WIDTH]  < 0))
    {
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = -outDataParam->dimValues[TIDL_DIM_HEIGHT];
        outDataParam->dimValues[TIDL_DIM_WIDTH]  = -outDataParam->dimValues[TIDL_DIM_WIDTH];
    }
    else
    {
        /* Standard transposed-convolution output size formula:
         *   stride * (in - 1) + outPad + (dilation*(kernel-1)+1) - (padStart+padEnd) */
        outDataParam->dimValues[TIDL_DIM_HEIGHT] =
            (convParams->strideH * (inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] - 1) + outPadH +
             (convParams->dilationH * (convParams->kernelH - 1) + 1) -
             (convParams->padT + convParams->padB));

        outDataParam->dimValues[TIDL_DIM_WIDTH] =
            (convParams->strideW * (inDataParams[0]->dimValues[TIDL_DIM_WIDTH] - 1) + outPadW +
             (convParams->dilationW * (convParams->kernelW - 1) + 1) -
             (convParams->padL + convParams->padR));
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Conv (Convolution)
 *
 * Computes the output spatial dimensions of a standard convolution:
 *   BATCH, DIM1, DIM2 : copied from input[0]
 *   NUMCH             : convParams.numOutChannels
 *   HEIGHT            : ((inH + padT + padB - ((kernelH-1)*dilH + 1)) / strideH) + 1
 *   WIDTH             : ((inW + padL + padR - ((kernelW-1)*dilW + 1)) / strideW) + 1
 *
 * NOTE on fixed weights / channels:
 *   TIDL only supports convolutions with fixed (compiled-in) weights and biases.
 *   This means kernelH, kernelW, numOutChannels, and numInChannels are all
 *   compile-time constants baked into the network binary — they NEVER change
 *   at runtime.  Consequently:
 *     - numOutChannels is always valid directly from convParams (no dynamic time change).
 *     - numInChannels does NOT need to be updated from the live input shape here;
 *       it is already correct from import time and is a parameter field, not an
 *       output shape field.  Any numInChannels update belongs in the compile-time
 *       wrapper (TIDL_tfOutReshapeConvLayer) only.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Conv(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    const sTIDL_ConvParams_t *convParams;

    (void)context;


    convParams = &layerParams->convParams;

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = convParams->numOutChannels;
    outDataParam->dimValues[TIDL_DIM_HEIGHT] =
        ((inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
          + (convParams->padT + convParams->padB)
          - ((convParams->kernelH - 1) * convParams->dilationH + 1))
         / convParams->strideH) + 1;
    outDataParam->dimValues[TIDL_DIM_WIDTH]  =
        ((inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
          + (convParams->padL + convParams->padR)
          - ((convParams->kernelW - 1) * convParams->dilationW + 1))
         / convParams->strideW) + 1;

    return TIDL_SHAPE_INFERENCE_OK;
}

/* -------------------------------------------------------------------------
 * Pooling
 *
 * BATCH, DIM1, DIM2 : copied from input[0]
 * NUMCH             : copied from input[0]; also stored in poolParams.numChannels
 *
 * Global Average Pooling (kernelH == 0 && kernelW == 0):
 *   HEIGHT = 1,  WIDTH = 1
 *
 * Regular Pooling:
 *   RUNTIME path (context == NULL or context->isCompileTime == 0):
 *     Uses the final LRTB pads stored in poolParams.padT/B/L/R.
 *     When poolParams.useCeil == 1, applies ceiling division with LRTB pads:
 *       outH = ceil((inH + padT + padB - kernelH) / strideH) + 1
 *       outW = ceil((inW + padL + padR - kernelW) / strideW) + 1
 *     When poolParams.useCeil == 0, applies floor division with LRTB pads:
 *       outH = (inH + padT + padB - kernelH) / strideH + 1
 *       outW = (inW + padL + padR - kernelW) / strideW + 1
 *
 *   COMPILE-TIME path (HOST_EMULATION, context->isCompileTime != 0):
 *     Uses the original (pre-LRTB) pads from
 *     pcLayer->layerPCParams.poolParams.originalPadT/B/L/R.
 *     When poolParams.useCeil == 1, applies ceiling division:
 *       outH = ceil((inH + origPadT + origPadB - kernelH) / strideH) + 1
 *       outW = ceil((inW + origPadL + origPadR - kernelW) / strideW) + 1
 *     When poolParams.useCeil == 0, applies floor division with originalPads.
 *
 * poolParams.numChannels is updated from the live input channel count at
 * both compile-time and runtime.
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Pooling(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    sTIDL_PoolingParams_t *poolParams;
    int32_t inH, inW;


    poolParams = &layerParams->poolParams;

    outDataParam->dimValues[TIDL_DIM_BATCH] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]  = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]  = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH] = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];

    /* Update numChannels for the kernel at both compile-time and runtime. */
    poolParams->numChannels = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];

    if ((poolParams->kernelH == 0) && (poolParams->kernelW == 0))
    {
        /* Global Average Pooling: output spatial size is always 1×1. */
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = 1;
        outDataParam->dimValues[TIDL_DIM_WIDTH]  = 1;
    }
    else
    {
        inH = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
        inW = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

#ifdef HOST_EMULATION
        if ((context != NULL) && (context->isCompileTime != 0) && (context->pcLayer != NULL))
        {
            /* Compile-time: use original (pre-LRTB) pads and ceil or floor. */
            const sTIDL_LayerPC_t      *pcLayer      = (const sTIDL_LayerPC_t *)context->pcLayer;
            const sTIDL_PoolPCParams_t *poolPCParams = &pcLayer->layerPCParams.poolParams;
            int32_t numerH = inH + poolPCParams->originalPadT + poolPCParams->originalPadB
                             - poolParams->kernelH;
            int32_t numerW = inW + poolPCParams->originalPadL + poolPCParams->originalPadR
                             - poolParams->kernelW;

            if (poolParams->useCeil)
            {
                /* Integer ceiling: ceil(a/b) = (a + b - 1) / b for a >= 0, b > 0 */
                outDataParam->dimValues[TIDL_DIM_HEIGHT] =
                    (numerH + poolParams->strideH - 1) / poolParams->strideH + 1;
                outDataParam->dimValues[TIDL_DIM_WIDTH]  =
                    (numerW + poolParams->strideW - 1) / poolParams->strideW + 1;

                if((outDataParam->dimValues[TIDL_DIM_HEIGHT] - 1) * poolParams->strideH >= inH + poolPCParams->originalPadT)
                {
                    outDataParam->dimValues[TIDL_DIM_HEIGHT] -=1;
                }
                if((outDataParam->dimValues[TIDL_DIM_WIDTH] - 1) * poolParams->strideW >= inW + poolPCParams->originalPadL)
                {
                    outDataParam->dimValues[TIDL_DIM_WIDTH] -=1;
                }
            }
            else
            {
                outDataParam->dimValues[TIDL_DIM_HEIGHT] = numerH / poolParams->strideH + 1;
                outDataParam->dimValues[TIDL_DIM_WIDTH]  = numerW / poolParams->strideW + 1;
            }
        }
        else
#endif /* HOST_EMULATION */
        {
            int32_t numerH = inH + poolParams->padT + poolParams->padB - poolParams->kernelH;
            int32_t numerW = inW + poolParams->padL + poolParams->padR - poolParams->kernelW;
            if (poolParams->useCeil)
            {
                /* Integer ceiling: ceil(a/b) = (a + b - 1) / b for a >= 0, b > 0 */
                outDataParam->dimValues[TIDL_DIM_HEIGHT] =
                    (numerH + poolParams->strideH - 1) / poolParams->strideH + 1;
                outDataParam->dimValues[TIDL_DIM_WIDTH]  =
                    (numerW + poolParams->strideW - 1) / poolParams->strideW + 1;
                
                if((outDataParam->dimValues[TIDL_DIM_HEIGHT] - 1) * poolParams->strideH >= inH + poolParams->padT)
                {
                    outDataParam->dimValues[TIDL_DIM_HEIGHT] -=1;
                }
                if((outDataParam->dimValues[TIDL_DIM_WIDTH] - 1) * poolParams->strideW >= inW + poolParams->padL)
                {
                    outDataParam->dimValues[TIDL_DIM_WIDTH] -=1;
                }
            }
            else
            {
                outDataParam->dimValues[TIDL_DIM_HEIGHT] = numerH / poolParams->strideH + 1;
                outDataParam->dimValues[TIDL_DIM_WIDTH]  = numerW / poolParams->strideW + 1;
            }
        }
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* =========================================================================
 * BatchNorm / PReLU
 *
 * Output shape is identical to the input shape (PassThrough semantics).
 * Additionally, at RUNTIME (non-HOST_EMULATION or context == NULL),
 * batchNormParams.numChannels is updated from the live input channel count
 * so that the BN/PReLU kernel always sees the correct value even when the
 * input shape changes dynamically.
 *
 * At compile-time (HOST_EMULATION with context->isCompileTime != 0), the
 * numChannels update is skipped here because the compile-time wrapper
 * (TIDL_tfOutReshapeBN / TIDL_tfOutReshapePRelu) handles it with its own
 * logic, including the allowlisting metadata (varTensorsDims) path.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_BatchNorm(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{

    /* PassThrough: output shape == input shape */
    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

    /* At compile-time the wrapper handles numChannels (may use metadata).
     * Skip the update here to avoid overwriting the wrapper's decision. */
    if ((context == NULL) || (context->isCompileTime == 0))
    {
        layerParams->batchNormParams.numChannels = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
    }

    return TIDL_SHAPE_INFERENCE_OK;
}

/* =========================================================================
 * Crop
 *
 * Computes the output shape of a Crop layer:
 *   BATCH  : copied from input[0]
 *   DIM1   : copied from input[0]
 *   DIM2   : copied from input[0]
 *   NUMCH  : copied from input[0]
 *
 * Spatial dimensions (HEIGHT, WIDTH) are determined as follows:
 *
 *   COMPILE-TIME (numInBufs == 2, context->isCompileTime != 0):
 *     HEIGHT : from input[1] (reference tensor provides target spatial size)
 *     WIDTH  : from input[1]
 *
 *   RUNTIME (numInBufs == 1, single-input):
 *     HEIGHT : inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] - cropParams.offsetH
 *     WIDTH  : inDataParams[0]->dimValues[TIDL_DIM_WIDTH]  - cropParams.offsetW
 *
 *     offsetH and offsetW encode the number of rows/columns cropped from the
 *     top/left of the input tensor. The output is the remaining spatial extent.
 *
 * At RUNTIME (context == NULL or context->isCompileTime == 0),
 * cropParams.numChannels is updated from the live input channel count so the
 * crop kernel always sees the correct value. At compile-time the wrapper
 * (TIDL_tfOutReshapeCropLayer) handles numChannels with its own logic.
 *
 * Preconditions : nIn >= 1.
 * ------------------------------------------------------------------------- */
int32_t TIDL_shapeInfer_Crop(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{

    outDataParam->dimValues[TIDL_DIM_BATCH]  = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
    outDataParam->dimValues[TIDL_DIM_DIM1]   = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
    outDataParam->dimValues[TIDL_DIM_DIM2]   = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
    outDataParam->dimValues[TIDL_DIM_NUMCH]  = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];

    if ((context != NULL) && (context->isCompileTime == 1))
    {
        if ((numInBufs >= 2) && (inDataParams[1] != NULL))
        {
            /* Compile-time 2-input crop: second input provides the target spatial
            * dimensions (HEIGHT, WIDTH) for the crop output. */
            outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[1]->dimValues[TIDL_DIM_HEIGHT];
            outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[1]->dimValues[TIDL_DIM_WIDTH];
        }

    }
    else
    {
        /* Runtime single-input crop: output spatial size = input size minus
         * the crop offset.  offsetH rows are removed from the top; offsetW
         * columns are removed from the left. */
        outDataParam->dimValues[TIDL_DIM_HEIGHT] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
                                                   - layerParams->cropParams.offsetH;
        outDataParam->dimValues[TIDL_DIM_WIDTH]  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
                                                   - layerParams->cropParams.offsetW;

        layerParams->cropParams.numChannels = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];

    }

    return TIDL_SHAPE_INFERENCE_OK;
}

#endif /* HOST_EMULATION */

/* =========================================================================
 * Dispatch table
 * ========================================================================= */
const TIDL_ShapeInferEntry_t
    gTIDL_ShapeInferDispatch[TIDL_SHAPE_INFERENCE_DISPATCH_TABLE_SIZE] =
{
     /* [0]  TIDL_DataLayer — identity */
    { TIDL_DataLayer,               TIDL_shapeInfer_PassThrough },

    /* [1]  TIDL_ConvolutionLayer */
    { TIDL_ConvolutionLayer,        TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Conv) },

    /* [2]  TIDL_PoolingLayer */
    { TIDL_PoolingLayer,            TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Pooling) },

    /* [3]  TIDL_ReLULayer — identity */
    { TIDL_ReLULayer,               TIDL_shapeInfer_PassThrough },

    /* [4]  TIDL_PReLULayer */
    { TIDL_PReLULayer,              TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_BatchNorm) },

    /* [5]  TIDL_EltWiseLayer — broadcast-aware max */
    { TIDL_EltWiseLayer,            TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_EltWise) },

    /* [6]  TIDL_InnerProductLayer */
    { TIDL_InnerProductLayer,       TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_InnerProduct) },

    /* [7]  TIDL_SoftMaxLayer — identity with optional HEIGHT/WIDTH swap */
    { TIDL_SoftMaxLayer,            TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_SoftMax) },

    /* [8]  TIDL_BatchNormLayer */
    { TIDL_BatchNormLayer,          TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_BatchNorm) },

    /* [9]  TIDL_BiasLayer — identity */
    { TIDL_BiasLayer,               TIDL_shapeInfer_PassThrough },

    /* [10] TIDL_ScaleLayer — identity */
    { TIDL_ScaleLayer,              TIDL_shapeInfer_PassThrough },

    /* [11] TIDL_Deconv2DLayer */
    { TIDL_Deconv2DLayer,           TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Deconv2D) },

    /* [12] TIDL_ConcatLayer */
    { TIDL_ConcatLayer,             TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Concat) },

    /* [13] TIDL_SplitLayer — static (split sizes baked in at import) */
    { TIDL_SplitLayer,              NULL                        },

    /* [14] TIDL_SliceLayer - static (slice sizes baked in at import)*/
    { TIDL_SliceLayer,              NULL                        },

    /* [15] TIDL_CropLayer — 2-input: H/W from inData[1]; runtime numChannels update */
    { TIDL_CropLayer,               TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Crop) },

    /* [16] TIDL_FlattenLayer — static (output shape baked in at import) */
    { TIDL_FlattenLayer,            NULL                        },

    /* [17] TIDL_DropOutLayer — identity */
    { TIDL_DropOutLayer,            TIDL_shapeInfer_PassThrough },

    /* [18] TIDL_ArgOpLayer */
    { TIDL_ArgOpLayer,              TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_ArgOp) },

    /* [19] TIDL_DetectionOutputLayer — static output shape */
    { TIDL_DetectionOutputLayer,    NULL                        },

    /* [20] TIDL_ShuffleChannelLayer — identity (same shape, reordered channels) */
    { TIDL_ShuffleChannelLayer,     TIDL_shapeInfer_PassThrough },

    /* [21] TIDL_ResizeLayer */
    { TIDL_ResizeLayer,             TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Resize) },

    /* [22] TIDL_RoiPoolingLayer — static output shape */
    { TIDL_RoiPoolingLayer,         NULL                        },

    /* [23] TIDL_OdPostProcessingLayer — static output shape */
    { TIDL_OdPostProcessingLayer,   NULL                        },

    /* [24] TIDL_DepthToSpaceLayer */
    { TIDL_DepthToSpaceLayer,       TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_DepthToSpace) },

    /* [25] TIDL_SigmoidLayer — identity */
    { TIDL_SigmoidLayer,            TIDL_shapeInfer_PassThrough },

    /* [26] TIDL_PadLayer */
    { TIDL_PadLayer,                TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Pad) },

    /* [27] TIDL_ColorConversionLayer — identity (same spatial, may change C) */
    { TIDL_ColorConversionLayer,    TIDL_shapeInfer_PassThrough },

    /* [28] TIDL_OdOutputReformatLayer — static output shape */
    { TIDL_OdOutputReformatLayer,   NULL                        },

    /* [29] TIDL_DataConvertLayer — identity */
    { TIDL_DataConvertLayer,        TIDL_shapeInfer_PassThrough },

    /* [30] TIDL_CustomLayer — caller-defined; cannot infer generically */
    { TIDL_CustomLayer,             NULL                        },

    /* [31] TIDL_BatchReshapeLayer — static (output shape baked in at import) */
    { TIDL_BatchReshapeLayer,       NULL                        },

    /* [32] TIDL_ReduceLayer */
    { TIDL_ReduceLayer,             TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Reduce) },

    /* [33] TIDL_ScatterElementsLayer — identity */
    { TIDL_ScatterElementsLayer,    TIDL_shapeInfer_PassThrough },

    /* [34] TIDL_SqueezeLayer — */
    { TIDL_SqueezeLayer,            TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Squeeze) },

    /* [35] TIDL_TanhLayer — identity */
    { TIDL_TanhLayer,               TIDL_shapeInfer_PassThrough },

    /* [36] TIDL_HardSigmoidLayer — identity */
    { TIDL_HardSigmoidLayer,        TIDL_shapeInfer_PassThrough },

    /* [37] TIDL_ELULayer — identity */
    { TIDL_ELULayer,                TIDL_shapeInfer_PassThrough },

    /* [38] TIDL_ReshapeLayer — dynamic shape support */
    { TIDL_ReshapeLayer,            TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Reshape)  },

    /* [39] TIDL_ConstDataLayer — identity */
    { TIDL_ConstDataLayer,          TIDL_shapeInfer_PassThrough },

    /* [40] TIDL_GatherLayer */
    { TIDL_GatherLayer,             TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Gather) },

    /* [41] TIDL_TransposeLayer — permute dimensions */
    { TIDL_TransposeLayer,          TIDL_shapeInfer_Transpose   },

    /* [42] TIDL_LayerNormLayer — identity */
    { TIDL_LayerNormLayer,          TIDL_shapeInfer_PassThrough },

    /* [43] TIDL_GridSampleLayer */
    { TIDL_GridSampleLayer,         TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_GridSample) },

    /* [44] TIDL_TopKLayer — copies all dims, replaces axis dim with K */
    { TIDL_TopKLayer,               TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_TopK) },

    /* [45] TIDL_DeformableConvLayer */
    { TIDL_DeformableConvLayer,     TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_DeformConv) },

    /* [46] TIDL_TileLayer */
    { TIDL_TileLayer,               TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Tile) },

    /* [47] TIDL_LogicalOpLayer — broadcast-aware max (same as EltWise) */
    { TIDL_LogicalOpLayer,          TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_LogicalOp) },

    /* [48] TIDL_RMSNormalizationLayer */
    { TIDL_RMSNormalizationLayer,   TIDL_shapeInfer_PassThrough },

    /* [49] TIDL_LSTMLayer */
    { TIDL_LSTMLayer,               TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_LSTM) },

    /* [50] TIDL_GRULayer */
    { TIDL_GRULayer,                TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_GRU) },

    /* [51] TIDL_RNNLayer */
    { TIDL_RNNLayer,                TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_RNN) },

    /* [52] TIDL_GatherNDLayer */
    { TIDL_GatherNDLayer,           TIDL_shapeInfer_GatherND    },

    /* [53] TIDL_CastLayer */
    { TIDL_CastLayer,               TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Cast) },

    /* [54] TIDL_GatherElementsLayer */
    { TIDL_GatherElementsLayer,     TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_GatherElements) },

    /* [55] TIDL_ShapeLayer */
    { TIDL_ShapeLayer,              TIDL_shapeInfer_Shape      },

    /* [56] TIDL_SizeLayer */
    { TIDL_SizeLayer,               TIDL_shapeInfer_Size       },

    /* [57] TIDL_AttentionLayer */
    { TIDL_AttentionLayer,          TIDL_SHAPE_FN_IMPORT_ONLY(TIDL_shapeInfer_Attention) },

    /* [58] TIDL_NonZeroLayer */
    { TIDL_NonZeroLayer,            TIDL_shapeInfer_NonZero     },

    /* [59] TIDL_UnsupportedLayer */
    { TIDL_UnsupportedLayer,        NULL                        },
};

/* =========================================================================
 * TIDL_inferShapeGeneric — generic parameter-based shape inference
 *
 * dynDimMask filtering (runtime only):
 *   outDataParam->dynDimMask is a bitmask where bit d (1 << d) indicates
 *   that dimValues[d] is allowed to change at runtime.  When the mask is
 *   non-zero, the per-layer inferFn writes into a temporary copy of
 *   outDataParam; only the dimensions whose bits are set in dynDimMask are
 *   then propagated back to the real outDataParam.  Dimensions whose bits
 *   are clear keep their static import-time values unchanged.
 *
 * ========================================================================= */
int32_t TIDL_inferShapeGeneric(
    int32_t                    layerType,
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context)
{
    TIDL_ShapeInferFn inferFn   = NULL;
    int32_t           ret       = TIDL_SHAPE_INFERENCE_OK;

    if ((layerParams == NULL) || (outDataParam == NULL) ||
        (inDataParams == NULL) || (numInBufs < 1) || (inDataParams[0] == NULL))
    {
        ret = TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS;
    }

    if (ret == TIDL_SHAPE_INFERENCE_OK)
    {
        if ((layerType >= 0) &&
            ((uint32_t)layerType < TIDL_SHAPE_INFERENCE_DISPATCH_TABLE_SIZE))
        {
            inferFn = gTIDL_ShapeInferDispatch[layerType].inferFn;
        }

        if (inferFn == NULL)
        {
            /* Layer type out of bounds or no shape function registered. */
            ret = TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER;
        }
    }

    if (ret == TIDL_SHAPE_INFERENCE_OK && outDataParam->dynDimMask != 0U)
    {
        /* dynDimMask is set: run inferFn on a temporary copy of outDataParam
         * so that all fields (including elementType needed by e.g. TopK) are
         * available to the inference function.  After the call, propagate back
         * only the dimensions whose bits are set in dynDimMask; static dims
         * keep their import-time values. */
        sTIDL_DataParams_t tempOut;
        int32_t            d;

        memcpy(&tempOut, outDataParam, sizeof(sTIDL_DataParams_t));

        ret = inferFn(layerParams, inDataParams, numInBufs, &tempOut, context);

        if (ret == TIDL_SHAPE_INFERENCE_OK)
        {
            /* Copy back only the dynamic dimensions. */
            for (d = 0; d < (int32_t)TIDL_DIM_MAX; d++)
            {
                if (((uint32_t)outDataParam->dynDimMask >> (uint32_t)d) & 1U)
                {
                    outDataParam->dimValues[d] = tempOut.dimValues[d];
                }
            }
            /* numDim may also be updated by inferFn */
            outDataParam->numDim = tempOut.numDim;
        }
    }

    return ret;
}

/* =========================================================================
 * TIDL_recalcDataParamsPitch — generic parameter-based pitch recalculation
 * ========================================================================= */
void TIDL_recalcDataParamsPitch(sTIDL_DataParams_t *dataParams)
{
    int32_t isPadW;

    if (dataParams == NULL)
    {
        return;
    }

    isPadW = (dataParams->padW != 0) ? 1 : 0;

    dataParams->pitch[TIDL_LINE_PITCH]    = dataParams->dimValues[TIDL_DIM_WIDTH] + dataParams->padW;
    dataParams->pitch[TIDL_CHANNEL_PITCH] = (dataParams->dimValues[TIDL_DIM_HEIGHT] + (2 * dataParams->padH) + isPadW)
                                            * dataParams->pitch[TIDL_LINE_PITCH];
    dataParams->pitch[TIDL_DIM2_PITCH]   = dataParams->dimValues[TIDL_DIM_NUMCH] * dataParams->pitch[TIDL_CHANNEL_PITCH];
    dataParams->pitch[TIDL_DIM1_PITCH]   = dataParams->dimValues[TIDL_DIM_DIM2]  * dataParams->pitch[TIDL_DIM2_PITCH];
    dataParams->pitch[TIDL_ROI_PITCH]    = dataParams->dimValues[TIDL_DIM_DIM1]  * dataParams->pitch[TIDL_DIM1_PITCH];

    /*
     * TODO: Take care of NC pitch manipulation here :)
     */
}
