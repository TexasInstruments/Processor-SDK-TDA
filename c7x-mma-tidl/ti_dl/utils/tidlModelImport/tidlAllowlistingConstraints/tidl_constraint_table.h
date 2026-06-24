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

#include <map>
#ifndef TIDL_CONSTRAINT_TABLE_H_
#define TIDL_CONSTRAINT_TABLE_H_
#include "tidl_constraint.h"
#include "itidl_ti.h"
#include "ti_dl.h"
#define JTE(layerType, constraints) { layerType , { #layerType , constraints }}

#include "tidl_constraint_slice.h"
#include "tidl_constraint_pooling.h"
#include "tidl_constraint_transpose.h"
#include "tidl_constraint_innerProduct.h"
#include "tidl_constraint_eltwise.h"
#include "tidl_constraint_gather.h"
#include "tidl_constraint_softmax.h"
#include "tidl_constraint_concat.h"
#include "tidl_constraint_resize.h"
#include "tidl_constraint_conv.h"
#include "tidl_constraint_deconv.h"
#include "tidl_constraint_scatterElements.h"
#include "tidl_constraint_reduce.h"
#include "tidl_constraint_depthToSpace.h"
#include "tidl_constraint_spaceToDepth.h"
#include "tidl_constraint_flatten.h"
#include "tidl_constraint_batchToSpace.h"
#include "tidl_constraint_spaceToBatch.h"
#include "tidl_constraint_cast.h"
#include "tidl_constraint_shuffleChannel.h"
#include "tidl_constraint_squeeze.h"
#include "tidl_constraint_priorBox.h"
#include "tidl_constraint_split.h"
#include "tidl_constraint_batchNorm.h"
#include "tidl_constraint_gridsample.h"
#include "tidl_constraint_reshape.h"
#include "tidl_constraint_layernorm.h"
#include "tidl_constraint_elu.h"
#include "tidl_constraint_hardSigmoid.h"
#include "tidl_constraint_tanh.h"
#include "tidl_constraint_sigmoid.h"
#include "tidl_constraint_relu.h"
#include "tidl_constraint_prelu.h"
#include "tidl_constraint_argop.h"
#include "tidl_constraint_pad.h"
#include "tidl_constraint_quantizeLinear.h"
#include "tidl_constraint_dequantizeLinear.h"
#include "tidl_constraint_clip.h"
#include "tidl_constraint_dropOut.h"
#include "tidl_constraint_identity.h"
#include "tidl_constraint_leakyRelu.h"
#include "tidl_constraint_pow.h"
#include "tidl_constraint_reduceMean.h"
#include "tidl_constraint_expand.h"
#include "tidl_constraint_reduceSum.h"
#include "tidl_constraint_sqrt.h"
#include "tidl_constraint_deformConv.h"
#include "tidl_constraint_topK.h"
#include "tidl_constraint_unsqueeze.h"
#include "tidl_constraint_sin.h"
#include "tidl_constraint_acos.h"
#include "tidl_constraint_atan.h"
#include "tidl_constraint_sinh.h"
#include "tidl_constraint_cos.h"
#include "tidl_constraint_cosh.h"
#include "tidl_constraint_tan.h"
#include "tidl_constraint_neg.h"
#include "tidl_constraint_abs.h"
#include "tidl_constraint_floor.h"
#include "tidl_constraint_exp.h"
#include "tidl_constraint_erf.h"
#include "tidl_constraint_asin.h"
#include "tidl_constraint_asinh.h"
#include "tidl_constraint_hardswish.h"
#include "tidl_constraint_mish.h"
#include "tidl_constraint_log.h"
#include "tidl_constraint_swish.h"
#include "tidl_constraint_instanceNormalization.h"
#include "tidl_constraint_tile.h"
#include "tidl_constraint_logicalOpLayer.h"
#include "tidl_constraint_rmsNormalization.h"
#include "tidl_constraint_softplus.h"
#include "tidl_constraint_softsign.h"
#include "tidl_constraint_ceil.h"
#include "tidl_constraint_celu.h"
#include "tidl_constraint_selu.h"
#include "tidl_constraint_round.h"
#include "tidl_constraint_sign.h"
#include "tidl_constraint_groupnorm.h"
#include "tidl_constraint_lstm.h"
#include "tidl_constraint_gru.h"
#include "tidl_constraint_rnn.h"
#include "tidl_constraint_gatherND.h"
#include "tidl_constraint_gatherElements.h"
#include "tidl_constraint_shape.h"
#include "tidl_constraint_size.h"
#include "tidl_constraint_attention.h"
#include "tidl_constraint_nonZero.h"


/// @brief Contains the constraints and other metadata of TIDL layers
struct TidlConstraintTableEntry {
    string displayname;
    vector<TidlConstraint> constraints;
};


/// @brief Jump Table
static const map<int32_t, TidlConstraintTableEntry> tidlConstraintTable =
{
    JTE( TIDL_ConvolutionLayer         , tidlConstraintConv),
    JTE( TIDL_PoolingLayer             , tidlConstraintPooling),
    JTE( TIDL_ReLULayer                , tidlConstraintRelu),
    JTE( TIDL_PReLULayer               , tidlConstraintPRelu),
    JTE( TIDL_EltWiseLayer             , tidlConstraintEltwise),
    JTE( TIDL_InnerProductLayer        , tidlConstraintInnerProduct),
    JTE( TIDL_SoftMaxLayer             , tidlConstraintSoftmax),
    JTE( TIDL_BatchNormLayer           , tidlConstraintBatchNorm),
    JTE( TIDL_BiasLayer                , {}),
    JTE( TIDL_ScaleLayer               , {}),
    JTE( TIDL_Deconv2DLayer            , tidlConstraintDeconv),
    JTE( TIDL_ConcatLayer              , tidlConstraintConcat),
    JTE( TIDL_SplitLayer               , tidlConstraintSplit),
    JTE( TIDL_SliceLayer               , tidlConstraintSlice),
    JTE( TIDL_CropLayer                , {}),
    JTE( TIDL_FlattenLayer             , tidlConstraintFlatten),
    JTE( TIDL_DropOutLayer             , tidlConstraintDropOut),
    JTE( TIDL_ArgOpLayer               , tidlConstraintArgOp),
    JTE( TIDL_DetectionOutputLayer     , {}),
    JTE( TIDL_ShuffleChannelLayer      , tidlConstraintShuffleChannel),
    JTE( TIDL_ResizeLayer              , tidlConstraintResize),
    JTE( TIDL_RoiPoolingLayer          , {}),
    JTE( TIDL_OdPostProcessingLayer    , {}),
    JTE( TIDL_DepthToSpaceLayer        , tidlConstraintDepthToSpace),
    JTE( TIDL_SpaceToDepthLayer        , tidlConstraintSpaceToDepth),
    JTE( TIDL_SigmoidLayer             , tidlConstraintSigmoid),
    JTE( TIDL_PadLayer                 , tidlConstraintPad),
    JTE( TIDL_ColorConversionLayer     , {}),
    JTE( TIDL_OdOutputReformatLayer    , {}),
    JTE( TIDL_DataConvertLayer         , {}),
    JTE( TIDL_CustomLayer              , {}),
    JTE( TIDL_BatchReshapeLayer        , {}),
    JTE( TIDL_ReduceLayer              , tidlConstraintReduce),
    JTE( TIDL_ScatterElementsLayer     , tidlConstraintScatterElements),
    JTE( TIDL_SqueezeLayer             , tidlConstraintSqueeze),
    JTE( TIDL_TanhLayer                , tidlConstraintTanH),
    JTE( TIDL_HardSigmoidLayer         , tidlConstraintHardSigmoid),
    JTE( TIDL_ELULayer                 , tidlConstraintElu),
    JTE( TIDL_ReshapeLayer             , tidlConstraintReshape),
    JTE( TIDL_ConstDataLayer           , {}),
    JTE( TIDL_GatherLayer              , tidlConstraintGather),
    JTE( TIDL_TransposeLayer           , tidlConstraintTranspose),
    JTE( TIDL_LayerNormLayer           , tidlConstraintLayerNorm),
    JTE( TIDL_TopKLayer                , tidlConstraintTopK),
    JTE( TIDL_PriorBoxLayer            , tidlConstraintPriorBox),
    JTE( TIDL_PermuteLayer             , {}),
    JTE( TIDL_ClipLayer                , tidlConstraintClip),
    JTE( TIDL_MinimumLayer             , {}),
    JTE( TIDL_LeakyReluLayer           , tidlConstraintLeakyRelu),
    JTE( TIDL_IdentityLayer            , tidlConstraintIdentity),
    JTE( TIDL_BatchToSpaceLayer        , tidlConstraintBatchToSpace),
    JTE( TIDL_SpaceToBatchLayer        , tidlConstraintSpaceToBatch),
    JTE( TIDL_PackLayer                , {}),
    JTE( TIDL_DequantizeLayer          , tidlConstraintDequantizeLinear),
    JTE( TIDL_QuantizeLayer            , tidlConstraintQuantizeLinear),
    JTE( TIDL_SqrtLayer                , tidlConstraintSqrt),
    JTE( TIDL_ReduceMeanLayer          , tidlConstraintReduceMean),
    JTE( TIDL_ExpandLayer              , tidlConstraintExpand),
    JTE( TIDL_ReduceSumLayer           , tidlConstraintReduceSum),
    JTE( TIDL_PowLayer                 , tidlConstraintPow),
    JTE( TIDL_PatchMergeLayer          , {}),
    JTE( TIDL_CastLayer                , tidlConstraintCast),
    JTE( TIDL_GridSampleLayer          , tidlConstraintGridSample),
    JTE( TIDL_DeformableConvLayer      , tidlConstraintDeformConv),
    JTE( TIDL_AcosLayer                , tidlConstraintAcos),
    JTE( TIDL_AtanLayer                , tidlConstraintAtan),
    JTE( TIDL_SinLayer                 , tidlConstraintSin),
    JTE( TIDL_SinhLayer                , tidlConstraintSinh),
    JTE( TIDL_CosLayer                 , tidlConstraintCos),
    JTE( TIDL_CoshLayer                , tidlConstraintCosh),
    JTE( TIDL_TanLayer                 , tidlConstraintTan),
    JTE( TIDL_NegLayer                 , tidlConstraintNeg),
    JTE( TIDL_AbsLayer                 , tidlConstraintAbs),
    JTE( TIDL_FloorLayer               , tidlConstraintFloor),
    JTE( TIDL_ExpLayer                 , tidlConstraintExp),
    JTE( TIDL_InstanceNormLayer        , tidlConstraintInstanceNormalization),
    JTE( TIDL_ErfLayer                 , tidlConstraintErf),
    JTE( TIDL_UnsqueezeLayer           , tidlConstraintUnsqueeze),
    JTE( TIDL_AsinLayer                , tidlConstraintAsin),
    JTE( TIDL_AsinhLayer               , tidlConstraintAsinh),
    JTE( TIDL_HardSwishLayer           , tidlConstraintHardSwish),
    JTE( TIDL_MishLayer                , tidlConstraintMish),
    JTE( TIDL_LogLayer                 , tidlConstraintLog),
    JTE( TIDL_SwishLayer               , tidlConstraintSwish),
    JTE( TIDL_TileLayer                , tidlConstraintTile),
    JTE( TIDL_LogicalOpLayer           , tidlConstraintLogicalOpLayer),
    JTE( TIDL_RMSNormalizationLayer    , tidlConstraintRMSNormalization),
    JTE( TIDL_SoftPlusLayer            , tidlConstraintSoftPlus),
    JTE( TIDL_SoftSignLayer            , tidlConstraintSoftSign),
    JTE( TIDL_CeilLayer                , tidlConstraintCeil),
    JTE( TIDL_CeluLayer                , tidlConstraintCelu),
    JTE( TIDL_SeluLayer                , tidlConstraintSelu),
    JTE( TIDL_RoundLayer               , tidlConstraintRound),
    JTE( TIDL_SignLayer                , tidlConstraintSign),
    JTE( TIDL_GroupNormLayer           , tidlConstraintGroupNorm),
    JTE( TIDL_LSTMLayer                , tidlConstraintLSTM),
    JTE( TIDL_GRULayer                 , tidlConstraintGRU),
    JTE( TIDL_RNNLayer                 , tidlConstraintRNN),
    JTE( TIDL_GatherNDLayer            , tidlConstraintGatherND),
    JTE( TIDL_CastLikeLayer            , tidlConstraintCast),
    JTE( TIDL_GatherElementsLayer      , tidlConstraintGatherElements),
    JTE( TIDL_ShapeLayer               , tidlConstraintShape),
    JTE( TIDL_SizeLayer                , tidlConstraintSize),
    JTE( TIDL_AttentionLayer           , tidlConstraintAttention),
    JTE( TIDL_NonZeroLayer             , tidlConstraintNonZero),
};

#endif /*TIDL_CONSTRAINT_TABLE_H_ */