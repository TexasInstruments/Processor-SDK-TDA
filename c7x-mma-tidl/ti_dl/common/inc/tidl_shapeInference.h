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
 * @file    tidl_shapeInference.h
 * @brief   Generic Shape Inference Framework for TIDL
 *
 *          It is designed to be used:
 *
 *          (A) Inside TIDL_process(), layers have dynamic shape
 *              to propagate input shapes through the network and update each
 *              layer's outData.dimValues before execution.
 *
 *          (B) At COMPILE TIME (PC/host) inside the TIDL import tool
 *              (tidl_import_common.cpp / tidl_import_core.cpp) to validate
 *              and pre-compute shapes during model import, replacing the
 *              existing ad-hoc shape logic that uses sTIDL_OrgNetwork_t.
 *
 *          Adding a new operator
 *          ---------------------
 *          1. Write a function matching TIDL_ShapeInferFn.
 *          2. Declare it here (or in a separate header).
 *          3. Register it in gTIDL_ShapeInferDispatch[] in tidl_shapeInference.c
 *             at the index equal to the layer's TIDL_*Layer constant.
 *
 * ----------------------------------------------------------------------------
 */

#ifndef TIDL_SHAPE_INFERENCE_H_
#define TIDL_SHAPE_INFERENCE_H_

/* Helper macro: resolves to the shape-inference function pointer on
 * HOST_EMULATION (import tool) builds, or NULL on DSP/device builds.
 * Used in gTIDL_ShapeInferDispatch[] for operators whose shape inference
 * is only needed at import time and not exercised at DSP runtime due to
 * dynamic not yet supported for those operators.
 */
#ifdef HOST_EMULATION
#define TIDL_SHAPE_FN_IMPORT_ONLY(fn) (fn)
#else
#define TIDL_SHAPE_FN_IMPORT_ONLY(fn) NULL
#endif

#include <stdint.h>
#include "itidl_ti.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Return codes
 * ========================================================================= */

/** Shape inference completed successfully. */
#define TIDL_SHAPE_INFERENCE_OK                    ( 0)

/** A required input data parameter (by dataId) was not found in the
 *  network's data buffer table. Indicates a corrupt network. */
#define TIDL_SHAPE_INFERENCE_ERR_DATA_NOT_FOUND    (-1)

/** The layer type has no registered shape function (NULL entry in the
 *  dispatch table). The layer's static import-time dimValues are preserved.
 *  This is NOT fatal — fully-static layers (Reshape, Flatten, etc.) are
 *  expected to return this code. */
#define TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER (-2)

/** numInBufs passed to TIDL_inferNetworkShapes() is out of range, or a
 *  required pointer argument is NULL. */
#define TIDL_SHAPE_INFERENCE_ERR_INVALID_ARGS      (-3)

/** TopK buffer increment macros. */
#define TOPK_BUFFER_INCREMENT_8BIT                 ( 5)
#define TOPK_BUFFER_INCREMENT_16BIT                ( 3)
#define TOPK_BUFFER_INCREMENT_32BIT                ( 2)

/* =========================================================================
 * Shape context for optional compile-time parameters
 * ========================================================================= */

/**
 * @brief Optional context providing compile-time PC parameters.
 *
 *        This structure enables access to PC-specific parameters that are
 *        not available at runtime
 *
 *        For runtime usage, pass NULL as the context parameter.
 *        For compile-time (import tool) usage, set isCompileTime = 1 and
 *        provide pcLayer and configParams pointers.
 *
 *        The PC-only headers (ti_dl.h, tidl_import_config.h) that define
 *        sTIDL_LayerPC_t and tidl_import_config are included in
 *        tidl_shapeInference.c under #ifdef HOST_EMULATION, which covers
 *        all PC builds (both algo host-emulation and import tool).
 *        On DSP/TI_DEVICE builds these headers are excluded and context
 *        is always NULL
 */
typedef struct {
    /** Flag indicating if PC parameters are available: 0 = runtime, 1 = compile-time */
    int32_t isCompileTime;

    /** Pointer to compile-time PC parameters (NULL for runtime).
     *
     *  At runtime: Always NULL.
     *  At compile-time: Cast to appropriate PC layer structure to access
     *                   fields in PC layer structure
     *
     */
    void *pcLayer;

    /** Pointer to compile-time config parameters (NULL for runtime).
     *
     *  At runtime: Always NULL.
     *  At compile-time: Cast to appropriate tidl_import_config to access
     *                   fields in tidl_import_config
     *
     */
    void *configParams;

} TIDL_ShapeContext_t;

/* =========================================================================
 * Per-layer shape inference function pointer type
 *
 * Each operator registers one function of this signature in the dispatch
 * table gTIDL_ShapeInferDispatch[].
 *
 * Parameters
 * ----------
 * layerParams  : Pointer to the layer parameters union (read-only).
 *                Contains operator-specific parameters (convParams, poolParams, etc.)
 * inDataParams : Array of pointers to the input sTIDL_DataParams_t for this
 *                layer. All pointers are guaranteed non-NULL by the caller, and
 *                dimValues have already been updated (forward-pass order).
 * numInBufs    : Number of valid entries in inDataParams[].
 * outDataParam : Pointer to the output sTIDL_DataParams_t being inferred.
 *
 * context      : Optional context providing compile-time PC parameters.
 *                Can be NULL for runtime (most operators don't need it).
 *                Non-NULL at compile-time for operators needing PC params.
 *
 * Return value
 * ------------
 * TIDL_SHAPE_INFERENCE_OK on success, or a negative TIDL_SHAPE_INFERENCE_ERR_* code.
 * ========================================================================= */
typedef int32_t (*TIDL_ShapeInferFn)(
    sTIDL_LayerParams_t           *layerParams,
    sTIDL_DataParams_t            *inDataParams[],
    int32_t                        numInBufs,
    sTIDL_DataParams_t            *outDataParam,
    TIDL_ShapeContext_t           *context);

/* =========================================================================
 * Dispatch table entry
 *
 * gTIDL_ShapeInferDispatch[] is indexed directly by layerType value.
 * ========================================================================= */
typedef struct {
    int32_t            layerType;
    TIDL_ShapeInferFn  inferFn;
} TIDL_ShapeInferEntry_t;


#define TIDL_SHAPE_INFERENCE_DISPATCH_TABLE_SIZE  (TIDL_UnsupportedLayer + 1)

extern const TIDL_ShapeInferEntry_t
    gTIDL_ShapeInferDispatch[TIDL_SHAPE_INFERENCE_DISPATCH_TABLE_SIZE];

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief   Generic shape inference function (runtime only).
 *
 *          This is the core runtime API that depends ONLY on:
 *          - Layer type
 *          - Layer parameters (from layerParams union)
 *          - Input tensor shapes
 *          - Output tensor shape (to be computed)
 *
 *          NO dependency on network structures (sTIDL_Network_t or sTIDL_OrgNetwork_t).
 *
 *          dynDimMask filtering:
 *            outDataParam->dynDimMask is a bitmask where bit d (1 << d) indicates
 *            that dimValues[d] is allowed to change at runtime.
 *
 *            - dynDimMask == 0: All output dimensions are static (baked in at
 *              import time).  Returns TIDL_SHAPE_INFERENCE_OK immediately without
 *              calling the per-layer inferFn — no inference needed.
 *
 *            - dynDimMask != 0: Runs the per-layer inferFn on a temporary copy of
 *              outDataParam (so all fields including elementType are available).
 *              After the call, only the dimensions whose bits are set in dynDimMask
 *              are propagated back to outDataParam; static dimensions (bits clear)
 *              keep their import-time values unchanged.  numDim is always propagated
 *              (needed by Reduce with keepDims == 0).
 *
 *          NOTE: Compile-time wrappers (TIDL_tfOutReshape*) call the per-layer
 *          inferFn directly and bypass this function entirely.
 *
 * @param   layerType     Layer type constant (TIDL_ConvolutionLayer, etc.)
 * @param   layerParams   Pointer to layer parameters union
 * @param   inDataParams  Array of input data parameter pointers
 * @param   numInBufs     Number of inputs
 * @param   outDataParam  Output data parameter to update (dynDimMask must be set)
 * @param   context       Optional context for special layers (NULL at runtime)
 *
 * @return  TIDL_SHAPE_INFERENCE_OK on success (including dynDimMask == 0 fast-path).
 *          TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER if no shape function is
 *          registered for this layer type (non-fatal; static dims preserved).
 *          Other negative codes on error.
 */
int32_t TIDL_inferShapeGeneric(
    int32_t                    layerType,
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Recomputes all five pitch[] fields of a sTIDL_DataParams_t from
 *          its current dimValues[] and padW/padH fields.
 *
 *          Must be called after any operation that updates dimValues[]
 *          (e.g. TIDL_inferShapeGeneric), because pitch[] fields are NOT
 *          automatically derived from dimValues[].
 *
 *          Pitch hierarchy (innermost → outermost):
 *            pitch[TIDL_LINE_PITCH]    = dimValues[WIDTH]  + padW
 *            pitch[TIDL_CHANNEL_PITCH] = (dimValues[HEIGHT] + 2*padH + isPadW)
 *                                        * LINE_PITCH
 *            pitch[TIDL_DIM2_PITCH]    = dimValues[NUMCH]  * CHANNEL_PITCH
 *            pitch[TIDL_DIM1_PITCH]    = dimValues[DIM2]   * DIM2_PITCH
 *            pitch[TIDL_ROI_PITCH]     = dimValues[DIM1]   * DIM1_PITCH
 *
 * @param   dataParams  Pointer to the data buffer descriptor to update in-place.
 *                      No-op if NULL.
 */
void TIDL_recalcDataParamsPitch(sTIDL_DataParams_t *dataParams);

/**
 * @brief   PassThrough shape inference.
 *
 *          Output shape is identical to the first input shape.  Used for
 *          all identity-like operators: ReLU, PReLU, BatchNorm, LayerNorm,
 *          SoftMax, Sigmoid, Tanh, HardSigmoid, ELU, DataConvert,
 *          LogicalOp, ShuffleChannel, DropOut, Crop (same-size), etc.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_PassThrough(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Reshape shape inference (dynamic shape support).
 *
 *          Recomputes the output dims that were -1 or 0 (passthrough) in the
 *          original shape spec, using the TIDL 6D params baked in at import:
 *            reshapeParams.passthroughMask  — bitmask of output dim indices
 *              that copy from the corresponding input dim at runtime.
 *            reshapeParams.minusOneDimIdx   — TIDL 6D index of the inferred
 *              (-1) output dim; -1 if none.
 *
 *          Steps:
 *            1. For each bit set in passthroughMask:
 *                 outDataParam->dimValues[d] = inDataParams[0]->dimValues[d]
 *            2. If minusOneDimIdx >= 0:
 *                 outDataParam->dimValues[minusOneDimIdx] =
 *                   totalInputVol / (product of all other output dims)
 *
 *          Output dims not covered by either step keep their import-time
 *          values (from the temp copy passed by TIDL_inferShapeGeneric).
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Reshape(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   EltWise shape inference (broadcast-aware).
 *
 *          For each dimension d in [0, TIDL_DIM_MAX):
 *              out.dimValues[d] = max(in[0].dimValues[d],
 *                                    in[1].dimValues[d], ...)
 *
 *          This correctly handles NumPy-style broadcasting where one input
 *          has size 1 along a dimension and the other has size N — the
 *          output takes the larger value.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_EltWise(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Transpose shape inference.
 *
 *          Applies the permutation stored in transposeParams.perm[] to the
 *          input dimValues:
 *              out.dimValues[d] = in.dimValues[perm[d]]  for d in [0, TIDL_DIM_MAX)
 *
 *          Preconditions : nIn >= 1.
 *          perm[] must be a valid permutation of [0, TIDL_DIM_MAX).
 */
int32_t TIDL_shapeInfer_Transpose(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);


/**
 * @brief   NonZero shape inference.
 *
 *          Infers the output shape for the NonZero operator.  The output is a
 *          2-D tensor of shape [TIDL_DIM_MAX, N], where N is the flat element
 *          count of the input tensor (product of all input dimensions).
 *          Because the actual number of non-zero elements is unknown at
 *          compile time, width dimension is marked as dynamic
 *          (dynDimMask = 0x20).
 *
 *          HOST_EMULATION compile-time path (context->isCompileTime != 0):
 *            Sets conservative worst-case output dimensions:
 *              HEIGHT = TIDL_DIM_MAX   (one row per input dimension)
 *              WIDTH  = product of all input dimValues   (max non-zero count)
 *
 */
int32_t TIDL_shapeInfer_NonZero(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context
);

/**
 * @brief   Resize shape inference.
 *
 *          Scales the spatial dimensions (HEIGHT, WIDTH) of the input tensor
 *          by the ratios stored in resizeParams.resizeRatio[].
 *
 *          Negative ratio convention (compile-time only):
 *            A negative ratio encodes an absolute output size as -(outputSize).
 *
 *          All other dimensions (BATCH, DIM1, DIM2, NUMCH) are copied from
 *          input[0].
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Resize(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Attention shape inference.
 *
 *          Computed the spatial dimensions (HEIGHT, WIDTH) of the input tensor
 *          by the key and value input dimensions.

 *          All other dimensions (BATCH, DIM1, DIM2, NUMCH) are copied from
 *          input[0].
 *
 */
int32_t TIDL_shapeInfer_Attention(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   DeformConv (Deformable Convolution) shape inference.
 *
 *          Computes the output spatial dimensions:
 *            BATCH, DIM1, DIM2 : copied from input[0]
 *            NUMCH             : deformConvParams.numOutChannels
 *            HEIGHT            : ((inDim[NUMCH]  + 2*padH - ((kH-1)*dilH+1)) / strideH) + 1
 *            WIDTH             : ((inDim[HEIGHT] + 2*padW - ((kW-1)*dilW+1)) / strideW) + 1
 *
 *          Note: input spatial dims are stored in NUMCH and HEIGHT slots due
 *          to the internal TIDL data layout for this operator.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_DeformConv(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   GridSample shape inference.
 *
 *          Computes the output shape of a grid_sample operation:
 *            BATCH  : from input[0] (data tensor)
 *            DIM1   : from input[0]
 *            DIM2   : from input[0]
 *            NUMCH  : from input[1] (grid tensor — encodes output H)
 *            HEIGHT : from input[1] (grid tensor — encodes output W)
 *            WIDTH  : from input[0] (data tensor — number of channels)
 *
 *          Preconditions : nIn >= 2.
 */
int32_t TIDL_shapeInfer_GridSample(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   GatherElements
 *
 *          Computes the output shape of a GatherElements operation:
 *          For GatherElements operator Output shape should be equal to the indices shape
 *          input[0] = data tensor,  input[1] = indices tensor.
 */
int32_t TIDL_shapeInfer_GatherElements(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Gather shape inference.
 *
 *          Computes the output shape of a Gather operation along
 *          gatherParams.axis:
 *
 *          Case 1 — scalar index (gatherParams.isIdxScalar == 1):
 *            The axis dimension is removed by right-shifting all dims from
 *            [axis] down by one, and setting dimValues[0] = 1.
 *
 *          Case 2 — tensor index (gatherParams.isIdxScalar == 0):
 *            dimValues[axis] is replaced by
 *            inDataParams[1]->dimValues[TIDL_DIM_WIDTH].
 *
 *          All other dimensions are copied from input[0].
 *          Preconditions : nIn >= 1; nIn >= 2 when isIdxScalar == 0.
 */
int32_t TIDL_shapeInfer_Gather(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 *@brief  GatherND shape inference.
 *
 *        Computes the output shape of a GatherND operation:
 *
 *        Case 1 — Without batchDims (gatherNDParams.batchDims == 0):
 *             output_shape = indices_shape[:-1] + data_shape[indices_shape[-1]:]
 *
 *        Case 1 — With batchDims (gatherNDParams.batchDims > 0):
 *             output_shape = data_shape[:batch_dims] + indices_shape[batch_dims:-1] + data_shape[batch_dims + indices_shape[-1]:]
 */
int32_t TIDL_shapeInfer_GatherND(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Tile shape inference.
 *
 *          For each dimension d in [0, TIDL_DIM_MAX):
 *              out.dimValues[d] = in.dimValues[d] * tileParams.repeats[d]
 *
 *          tileParams.repeats[d] is the integer repetition count along
 *          dimension d.  A repeat value of 1 leaves that dimension unchanged.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Tile(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   LogicalOp shape inference (broadcast-aware).
 *
 *          Computes the output shape of a logical/comparison operation
 *          (And, Or, Not, Where, etc.) using broadcast semantics:
 *
 *          For each dimension d in [0, TIDL_DIM_MAX):
 *              out.dimValues[d] = max(in[0].dimValues[d],
 *                                    in[1].dimValues[d], ...)
 *
 *          This is identical to EltWise broadcast semantics.  The output
 *          element type (Bool, or the data type for Where) is set by the
 *          caller after this function returns.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_LogicalOp(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   SoftMax shape inference.
 *
 *          Output shape is identical to the input shape for all dimensions,
 *          with one optional exception: when softMaxParams.outTranspose == 1,
 *          the HEIGHT and WIDTH dimensions are swapped in the output.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_SoftMax(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Squeeze shape inference.
 *
 *          Removes dimensions of size 1 from the tensor shape using the
 *          axis mask stored in pcLayer->layerPCParams.squeezeParams.axis[].
 *
 *          COMPILE-TIME path (context != NULL && context->isCompileTime != 0):
 *            axis[i] != 0 means dimension i is squeezed (removed).
 *            Non-squeezed dims are packed from the high end of dimValues[]
 *            downward; remaining leading slots are set to 1.
 *            TIDL_DIM_BATCH is always preserved from input[0].
 *            Only available when HOST_EMULATION is defined (PC builds).
 *
 *          RUNTIME path (context == NULL or context->isCompileTime == 0):
 *            The axis mask is a PC-only field not available at runtime.
 *            Returns TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER so that the
 *            static import-time dimValues are preserved as-is (non-fatal).
 *            Also returned on DSP/TI_DEVICE builds (no HOST_EMULATION).
 *
 *          Preconditions : nIn >= 1.
 *          At compile-time: context->pcLayer must point to sTIDL_LayerPC_t.
 */
int32_t TIDL_shapeInfer_Squeeze(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Concat shape inference.
 *
 *          Concatenates numInBufs inputs along concatParams.axis:
 *            All dims are copied from inDataParams[0], except:
 *              out.dimValues[axis] = sum of inDataParams[j]->dimValues[axis]
 *                                    for j in [0, numInBufs)
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Concat(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   DepthToSpace shape inference.
 *
 *          Rearranges data from the channel dimension into spatial blocks:
 *            BATCH, DIM1, DIM2 : copied from input[0]
 *            NUMCH  : inDataParams[0]->dimValues[TIDL_DIM_NUMCH] / (blockSize * blockSize)
 *            HEIGHT : inDataParams[0]->dimValues[TIDL_DIM_HEIGHT] * blockSize
 *            WIDTH  : inDataParams[0]->dimValues[TIDL_DIM_WIDTH]  * blockSize
 *
 *          blockSize is taken from depthToSpaceParams.blockSize.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_DepthToSpace(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Pad shape inference.
 *
 *          Adds symmetric padding to the HEIGHT and WIDTH dimensions:
 *            BATCH, DIM1, DIM2, NUMCH : copied from input[0]
 *            HEIGHT : inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
 *                     + padParams.padT + padParams.padB
 *            WIDTH  : inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
 *                     + padParams.padL + padParams.padR
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Pad(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   ArgOp (ArgMax / ArgMin) shape inference.
 *
 *          Reduces the input tensor along argOpParams.axis:
 *            All dims are copied from input[0], except:
 *              out.dimValues[axis] = 1
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_ArgOp(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   TopK shape inference.
 *
 *          Computes the output shape of a TopK operation:
 *            All dims are copied from input[0], except:
 *              out.dimValues[topKParams.axis] = topKParams.K
 *
 *          Output Buffer-merging:
 *            TopK provides two outputs (values + indices). In TIDL both the
 *            outputs are merged into on output buffer which is then sliced
 *            using slice operator. This is done as part of optimizer
 *            tidl_handleTopKLayers. This implies the output buffer of TopK
 *            after this optimization (i.e during runtime) is a combination
 *            of values and indices packed into single enlarged buffer.
 *
 *          Preconditions : nIn >= 1.
 */

int32_t TIDL_shapeInfer_TopK(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);
/**
 * @brief   LSTM shape inference — combined (runtime) output shape.
 *
 *          This function computes the FINAL shape of the LSTM output buffer
 *          as it exists at runtime, AFTER the optimizer tidl_handleRecurrentLayers
 *          has combined all per-buffer outputs (full-sequence Y, hidden state Y_h,
 *          cell state Y_c) into a single enlarged buffer.
 *
 *          The initial per-buffer shapes (before combining) are computed by
 *          TIDL_tfOutReshapeLSTMLayer() in tidl_import_common.cpp and are NOT
 *          the concern of this function.
 *
 *          Combined output shape (layout == 0, seq-first):
 *            DIM2   = seq_length + 2   (seq output + 2 state slots packed in)
 *            NUMCH  = num_directions
 *            HEIGHT = batch_size
 *            WIDTH  = hidden_size
 *
 *          Combined output shape (layout == 1, batch-first):
 *            DIM2   = batch_size
 *            NUMCH  = seq_length + 2   (seq output + 2 state slots packed in)
 *            HEIGHT = num_directions
 *            WIDTH  = hidden_size
 *
 *          num_directions = 2 when lstmParams.direction == TIDL_RecurrentBidirectional,
 *          otherwise 1.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_LSTM(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   GRU shape inference — combined (runtime) output shape.
 *
 *          This function computes the FINAL shape of the GRU output buffer
 *          as it exists at runtime, AFTER the optimizer tidl_handleRecurrentLayers
 *          has combined the per-buffer outputs (full-sequence Y and hidden state
 *          Y_h) into a single enlarged buffer.
 *
 *          The initial per-buffer shapes (before combining) are computed by
 *          TIDL_tfOutReshapeGRULayer() in tidl_import_common.cpp and are NOT
 *          the concern of this function.
 *
 *          Combined output shape (layout == 0, seq-first):
 *            DIM2   = seq_length + 1   (seq output + 1 state slot packed in)
 *            NUMCH  = num_directions
 *            HEIGHT = batch_size
 *            WIDTH  = hidden_size
 *
 *          Combined output shape (layout == 1, batch-first):
 *            DIM2   = batch_size
 *            NUMCH  = seq_length + 1   (seq output + 1 state slot packed in)
 *            HEIGHT = num_directions
 *            WIDTH  = hidden_size
 *
 *          num_directions = 2 when gruParams.direction == TIDL_RecurrentBidirectional,
 *          otherwise 1.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_GRU(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   RNN shape inference — combined (runtime) output shape.
 *
 *          This function computes the FINAL shape of the RNN output buffer
 *          as it exists at runtime, AFTER the optimizer tidl_handleRecurrentLayers
 *          has combined the per-buffer outputs (full-sequence Y and hidden state
 *          Y_h) into a single enlarged buffer.
 *
 *          The initial per-buffer shapes (before combining) are computed by
 *          TIDL_tfOutReshapeRNNLayer() in tidl_import_common.cpp and are NOT
 *          the concern of this function.
 *
 *          Combined output shape (layout == 0, seq-first):
 *            DIM2   = seq_length + 1   (seq output + 1 state slot packed in)
 *            NUMCH  = num_directions
 *            HEIGHT = batch_size
 *            WIDTH  = hidden_size
 *
 *          Combined output shape (layout == 1, batch-first):
 *            DIM2   = batch_size
 *            NUMCH  = seq_length + 1   (seq output + 1 state slot packed in)
 *            HEIGHT = num_directions
 *            WIDTH  = hidden_size
 *
 *          num_directions = 2 when rnnParams.direction == TIDL_RecurrentBidirectional,
 *          otherwise 1.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_RNN(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Deconv2D (Transposed Convolution) shape inference.
 *
 *          Computes the output spatial dimensions of a transposed convolution:
 *            BATCH, DIM1, DIM2 : copied from input[0]
 *            NUMCH             : convParams.numOutChannels
 *            HEIGHT            : strideH*(inH-1) + outPadH + (dilH*(kH-1)+1) - (padT+padB)
 *            WIDTH             : strideW*(inW-1) + outPadW + (dilW*(kW-1)+1) - (padL+padR)
 *
 *          outPadH / outPadW are PC-only fields (sTIDL_ConvPCParams_t).
 *          At runtime these are always 0.
 *
 *          Negative dimValues convention (compile-time only):
 *            When the importer pre-sets the output size as -(absoluteSize) in
 *            outDataParam->dimValues[HEIGHT/WIDTH], this function detects the
 *            negative sentinel and flips the sign, bypassing the formula.
 *
 *          Preconditions : nIn >= 1.
 *          At compile-time: context->pcLayer must point to sTIDL_LayerPC_t.
 */
int32_t TIDL_shapeInfer_Deconv2D(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   InnerProduct (Fully-Connected / MatMul) shape inference.
 *
 *          Computes the output shape of an InnerProduct layer, accounting for
 *          the transpose flags on both input matrices.
 *
 *          COMPILE-TIME path (HOST_EMULATION, context != NULL && isCompileTime != 0):
 *            The importer handles shape explicitly; this function only fills in
 *            fields that have not been set yet (sentinel value -1 or 0):
 *              numInRows  = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]  (always)
 *              numOutCols = inDataParams[1]->dimValues[TIDL_DIM_WIDTH]   (if -1 or 0)
 *              numInCols  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]   (if -1 or 0)
 *              numBatches = outDataParam->dimValues[TIDL_DIM_BATCH]      (always)
 *
 *          RUNTIME path (DSP, or HOST_EMULATION with context == NULL):
 *            numInRows, numInCols, numOutCols are derived from live input shapes,
 *            respecting the transpose flags:
 *
 *            inputATranspose == 0: A stored as [numOutRows × numInCols]
 *              → numInRows  = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
 *              → numInCols  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
 *            inputATranspose != 0: A stored transposed as [numInCols × numOutRows]
 *              → numInRows  = inDataParams[0]->dimValues[TIDL_DIM_WIDTH]
 *              → numInCols  = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT]
 *
 *            inputBTranspose == 0: B stored as [numInCols × numOutCols]
 *              → numOutCols = inDataParams[1]->dimValues[TIDL_DIM_WIDTH]
 *            inputBTranspose != 0: B stored transposed as [numOutCols × numInCols]
 *              → numOutCols = inDataParams[1]->dimValues[TIDL_DIM_HEIGHT]
 *
 *          Output shape (both paths):
 *            BATCH, DIM1, DIM2, NUMCH = element-wise max of input[0] and input[1]
 *            HEIGHT = numInRows
 *            WIDTH  = numOutCols
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_InnerProduct(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Reduce (ReduceSum / ReduceMean / ReduceMax / ReduceMin / etc.) shape inference.
 *
 *          Reduces the input tensor along a single axis:
 *            All dims are copied from input[0].
 *            dimValues[axis] is set to 1.
 *            If keepDims == 0: dims from [axis] down to [1] are right-shifted
 *              (dimValues[i] = dimValues[i-1] for i from axis down to 1),
 *              and numDim is decremented by 1.
 *
 *          COMPILE-TIME path (HOST_EMULATION, context->isCompileTime != 0):
 *            axis and keepDims are read from:
 *              - pcLayer->layerParams.reduceParams        (TIDL_ReduceLayer)
 *              - pcLayer->layerPCParams.reduceSumParams   (TIDL_ReduceSumLayer)
 *              - pcLayer->layerPCParams.reduceMeanParams  (all other reduce variants)
 *            Returns TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER if context is
 *            NULL or not compile-time.
 *
 *          RUNTIME path (non-HOST_EMULATION):
 *            axis and keepDims are read from layerParams->reduceParams.
 *
 *          Preconditions : nIn >= 1.
 *          At compile-time: context->pcLayer must point to sTIDL_LayerPC_t.
 */
int32_t TIDL_shapeInfer_Reduce(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Conv (Convolution) shape inference.
 *
 *          Computes the output spatial dimensions of a standard convolution:
 *            BATCH, DIM1, DIM2 : copied from input[0]
 *            NUMCH             : convParams.numOutChannels
 *            HEIGHT            : ((inH + padT + padB - ((kernelH-1)*dilH + 1)) / strideH) + 1
 *            WIDTH             : ((inW + padL + padR - ((kernelW-1)*dilW + 1)) / strideW) + 1
 *
 *          NOTE on fixed weights / channels:
 *            TIDL only supports convolutions with fixed (compiled-in) weights and biases.
 *            This means kernelH, kernelW, numOutChannels, and numInChannels are all
 *            compile-time constants baked into the network binary — they NEVER change
 *            at runtime.  Consequently:
 *              - numOutChannels is always valid directly from convParams (no dynamic lookup).
 *              - numInChannels does NOT need to be updated from the live input shape here;
 *                it is already correct from import time and is a parameter field, not an
 *                output shape field.  Any numInChannels update belongs in the compile-time
 *                wrapper (TIDL_tfOutReshapeConvLayer) only.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Conv(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Pooling shape inference.
 *
 *          Computes the output shape of a pooling layer:
 *            BATCH, DIM1, DIM2 : copied from input[0]
 *            NUMCH             : copied from input[0]; also stored in
 *                                poolParams.numChannels for the kernel
 *
 *          Global Average Pooling (kernelH == 0 && kernelW == 0):
 *            HEIGHT = 1,  WIDTH = 1
 *
 *          Regular Pooling:
 *              RUNTIME path (context == NULL or context->isCompileTime == 0):
 *              Uses the final LRTB pads stored in poolParams.padT/B/L/R.
 *              When poolParams.useCeil == 1, applies ceiling division with LRTB pads:
 *                  outH = ceil((inH + padT + padB - kernelH) / strideH) + 1
 *                  outW = ceil((inW + padL + padR - kernelW) / strideW) + 1
 *              When poolParams.useCeil == 0, applies floor division with LRTB pads:
 *                  outH = (inH + padT + padB - kernelH) / strideH + 1
 *                  outW = (inW + padL + padR - kernelW) / strideW + 1
 *
 *              COMPILE-TIME path (HOST_EMULATION, context->isCompileTime != 0):
 *                  Uses the original (pre-LRTB) pads from
 *                  pcLayer->layerPCParams.poolParams.originalPadT/B/L/R.
 *                  When poolParams.useCeil == 1, applies ceiling division:
 *                      outH = ceil((inH + origPadT + origPadB - kernelH) / strideH) + 1
 *                      outW = ceil((inW + origPadL + origPadR - kernelW) / strideW) + 1
 *                  When poolParams.useCeil == 0, applies floor division with originalPads.
 *
 *          poolParams.numChannels is updated from the live input channel count at
 *          both compile-time and runtime.
 *
 *          Preconditions : nIn >= 1.
 *          At compile-time: context->pcLayer must point to sTIDL_LayerPC_t.
 */
int32_t TIDL_shapeInfer_Pooling(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   BatchNorm / PReLU shape inference.
 *
 *          Output shape is identical to the input shape (PassThrough semantics).
 *          Additionally, at runtime (non-HOST_EMULATION or context == NULL),
 *          batchNormParams.numChannels is updated from the live input channel
 *          count so that the BN/PReLU kernel always sees the correct value even
 *          when the input shape changes dynamically.
 *
 *          At compile-time (HOST_EMULATION with context->isCompileTime != 0),
 *          the numChannels update is skipped here because the compile-time
 *          wrapper handles it (including the allowlisting metadata path).
 *
 *          Used for: TIDL_BatchNormLayer, TIDL_PReLULayer.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_BatchNorm(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Crop layer shape inference.
 *
 *          Computes the output shape of a Crop layer:
 *            BATCH, DIM1, DIM2, NUMCH : copied from input[0]
 *            HEIGHT, WIDTH            : from input[1] if numInBufs >= 2,
 *                                       else from input[0]
 *
 *          When numInBufs == 2, the second input (reference tensor) provides
 *          the target spatial dimensions (HEIGHT, WIDTH) for the crop output.
 *
 *          At runtime (non-HOST_EMULATION or context == NULL),
 *          cropParams.numChannels is updated from the live input channel count
 *          so the crop kernel always sees the correct value. At compile-time
 *          the wrapper handles this.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Crop(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Cast shape inference.
 *
 *          Cast does not change the output shape — it only changes the
 *          element type.  This function copies all input dimension values
 *          to the output unchanged.
 *
 *          The output element type is set by the compile-time wrapper
 *          (TIDL_tfOutReshapeCastLayer) from layerPCParams.castParams.castTo
 *          and is NOT updated here.
 *
 *          Preconditions : nIn >= 1.
 */
int32_t TIDL_shapeInfer_Cast(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Shape operator shape inference.
 *
 *          Computes the output shape of an ONNX/TIDL Shape operation:
 *            All output dimensions are set to 1 except TIDL_DIM_WIDTH,
 *            which is set to (shapeParams.end - shapeParams.start).
 *
 *          shapeParams.start and .end are already normalised to
 *          TIDL-dimension indices by the importer (tidl_parse_onnx_shape.cpp).
 *
 *          Preconditions : layerParams != NULL.
 */
int32_t TIDL_shapeInfer_Shape(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

/**
 * @brief   Size operator shape inference.
 *
 *          The ONNX Size operator returns the total number of elements in
 *          the input tensor as a scalar.  The output is always a single
 *          element, so all output dimensions are set to 1.
 *
 *          Preconditions : outDataParam != NULL.
 */
int32_t TIDL_shapeInfer_Size(
    sTIDL_LayerParams_t       *layerParams,
    sTIDL_DataParams_t        *inDataParams[],
    int32_t                    numInBufs,
    sTIDL_DataParams_t        *outDataParam,
    TIDL_ShapeContext_t       *context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TIDL_SHAPE_INFERENCE_H_ */
