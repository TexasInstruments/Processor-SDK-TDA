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



#include "tidl_parse_onnx.h"
using namespace std;
using namespace onnx;

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Resize")> ()
{
  int32_t idx, j, axes_idx = -1;
  int32_t status = 0;
  char upsampleMode[50];
  layer.layerType = TIDL_ResizeLayer;
  int32_t axis;
  layer.numInBufs = 1;
  NodeProto node = graph.node(index);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  int32_t attrRetVal = getStringAttr(node, "mode", upsampleMode, 0);
  layer.layerParams.resizeParams.mode = TIDL_ResizeNearest; /* default value of attribute */
  if ((strcmp(upsampleMode, "nearest") == 0))
  {
    layer.layerParams.resizeParams.mode = TIDL_ResizeNearest;
  }
  else if((strcmp(upsampleMode, "linear") == 0))
  {
    layer.layerParams.resizeParams.mode = TIDL_ResizeBilinear;
  }
  else if(attrRetVal != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) /* Deny only if attribute(mode) actually present in the model but none of above */
  {
    layer.layerParams.resizeParams.mode = -1;
  }

  sBuffer_t scales;
  scales.bufSize = 0;
  /* Copy scales from input */
  /* For versions lower than 10, it is 2nd input */
  int32_t scaleStatus = 0;
  if (onnxOpSetVersion <= 10)
  {
    status = copyFloatConst(graph, index, 1, scales, INPUT_NOT_REQUIRED);
  }
  /* For any version upper, it is 3rd input */
  else
  {
    if ( node.input_size() <= 4 )
    {
      status = copyFloatConst(graph, index, 2, scales, INPUT_NOT_REQUIRED);
    }
  }

  /* roi and sizes introduced in >10 version as inputs */
  sBuffer_t roi;
  sBuffer_t sizes;
  sizes.bufSize = 0;
  roi.bufSize = 0;
  if(onnxOpSetVersion > 10)
  {
    /* update its usage when we support roi */
    /* Only takes effect when coordinate_transformation_mode=tf_crop_and_resize */
    status = copyFloatConst(graph, index, 1, roi, INPUT_NOT_REQUIRED);

    /* sizes */
    if(graph.node(index).input_size() > 3)
    {
      status = copyFloatConst(graph, index, 3, sizes, INPUT_NOT_REQUIRED);
    }
  }
  if(scales.bufSize == 0 && sizes.bufSize == 0)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to find initializer for scales/sizes. One of the scales or sizes needs to be specified");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  if(scales.bufSize != 0 && sizes.bufSize != 0)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Both scales and sizes can not be specified. Only one of them can be specified");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if (getIntAttr(node, "antialias", &layer.layerPCParams.resizeParams.antialias, 0) == -1)
  {
    layer.layerPCParams.resizeParams.antialias = 0;
  }

  layer.layerPCParams.resizeParams.coordinate_transformation_mode = 0;  /* 0 for default - 'half_pixel' */
  char coordinate_transformation_mode[50];
  status = getStringAttr(node, "coordinate_transformation_mode", coordinate_transformation_mode, 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if(strcmp(coordinate_transformation_mode, "half_pixel") == 0)
    {
      layer.layerPCParams.resizeParams.coordinate_transformation_mode = TIDL_Resize_half_pixel;
    }
    else if(strcmp(coordinate_transformation_mode, "pytorch_half_pixel") == 0)
    {
      layer.layerPCParams.resizeParams.coordinate_transformation_mode = TIDL_Resize_pytorch_half_pixel;
    }
    else if(strcmp(coordinate_transformation_mode, "align_corners") == 0)
    {
      layer.layerPCParams.resizeParams.coordinate_transformation_mode = TIDL_Resize_align_corners;
    }
    else if(strcmp(coordinate_transformation_mode, "asymmetric") == 0)
    {
      layer.layerPCParams.resizeParams.coordinate_transformation_mode = TIDL_Resize_asymmetric;
    }
    else if(strcmp(coordinate_transformation_mode, "tf_crop_and_resize") == 0)
    {
      layer.layerPCParams.resizeParams.coordinate_transformation_mode = TIDL_Resize_tf_crop_and_resize;
    }
    else
    {
      layer.layerPCParams.resizeParams.coordinate_transformation_mode = -1;
    }
  }

  /* TODO: add check for this in constraints(for values which we don't support) when we support cubic resize mode, added field just for consistency*/
  if(getFloatAttr(node, "cubic_coeff_a",  &layer.layerPCParams.resizeParams.cubic_coeff_a, 0) == -1)
  {
    layer.layerPCParams.resizeParams.cubic_coeff_a = -0.75;
  }

  if(getIntAttr(node, "exclude_outside", &layer.layerPCParams.resizeParams.exclude_outside, 0) == -1)
  {
    layer.layerPCParams.resizeParams.exclude_outside = 0;
  }

  /* TODO: add check for this in constraints(for values which we don't support) when we support coordinate_transformation_mode=tf_crop_and_resize, added field just for consistency*/
  if(getFloatAttr(node, "extrapolation_value", &layer.layerPCParams.resizeParams.extrapolation_value, 0) == -1)
  {
    layer.layerPCParams.resizeParams.extrapolation_value = 0.0;
  }
 
  layer.layerPCParams.resizeParams.nearest_mode = 0;  /* 0 for default - 'round_prefer_floor' */
  char nearest_mode[50];
  status = getStringAttr(node, "nearest_mode", nearest_mode, 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if(strcmp(nearest_mode, "round_prefer_floor") == 0)
    {
      layer.layerPCParams.resizeParams.nearest_mode = TIDL_Resize_round_prefer_floor;
    }
    else if(strcmp(nearest_mode, "round_prefer_ceil") == 0)
    {
      layer.layerPCParams.resizeParams.nearest_mode = TIDL_Resize_round_prefer_ceil;
    }
    else if(strcmp(nearest_mode, "floor") == 0)
    {
      layer.layerPCParams.resizeParams.nearest_mode = TIDL_Resize_floor;
    }
    else if(strcmp(nearest_mode, "ceil") == 0)
    {
      layer.layerPCParams.resizeParams.nearest_mode = TIDL_Resize_ceil;
    }
    else
    {
      layer.layerPCParams.resizeParams.nearest_mode = -1;
    }
  }

  layer.layerPCParams.resizeParams.keep_aspect_ratio_policy = 0;
  char keep_aspect_ratio[50];
  status = getStringAttr(node, "keep_aspect_ratio_policy", keep_aspect_ratio, 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if (strcmp(keep_aspect_ratio,"stretch") != 0)
    {
      layer.layerPCParams.resizeParams.keep_aspect_ratio_policy = -1;
    }
  }


  vector<int32_t> inputDims = {};
  int32_t numDims;
  bool isInputConst = false;

  /* If data input (input 0) is a constant initializer, store it in layer.weights
   * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
  for (int32_t i = 0; i < (int32_t)layer.allowlistingMetaData.numConstInputs; i++)
  {
    if (layer.allowlistingMetaData.constTensorIndices[i] == 0)
    {
      isInputConst = true;
      status = copyFloatConst(graph, index, 0, layer.weights, INPUT_REQUIRED);
      if (status != 0)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : Resize layer : Unable to read constant data input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      inputDims = layer.allowlistingMetaData.constTensorsDims[i];
      break;
    }
  }

  if (!isInputConst)
  {
    layer.weights.ptr = NULL;
    layer.weights.bufSize = 0;
    if (layer.allowlistingMetaData.varTensorsDims.size() > 0)
    {
      inputDims = layer.allowlistingMetaData.varTensorsDims[0];
    }
  }

  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  numDims = (inputDims.empty()) ? 4 : inputDims.size();

  int32_t dimOffset = TIDL_DIM_MAX - numDims;
  /* initialize resizeRatio by 1, later will be changed acc to scales/sizes */
  for(int32_t i = 0; i < TIDL_DIM_MAX; i++)
  {
    layer.layerParams.resizeParams.resizeRatio[i] = 1.0;
  }

  int32_t *axes = NULL;
  axes_idx = getAttrIdx(node, "axes");
  if (axes_idx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    /* Read the axes values only if length of scales/sizes and axes are same*/
    if (node.attribute(axes_idx).ints_size() == scales.bufSize)
    {
      /* Read axes values */
      axes = (int32_t*)malloc (node.attribute(axes_idx).ints_size() * sizeof (int32_t));
      for (int32_t i = 0; i < node.attribute(axes_idx).ints_size(); i++)
      {
        getIntAttr(node, "axes", &axes[i], i);
        /* Adjust axes[i] to TIDL dimension */
        if(axes[i] >= 0)
        {
          /*+ve Axis*/
          axes[i] += dimOffset;
        }
        else
        {
          /*-ve Axis*/
          axes[i] += TIDL_DIM_MAX;
        }
      }

      /* Read resize Ratios from scales according to axes provided */
      float32_tidl* resizeRatios = (float32_tidl*)scales.ptr;
      for (int32_t i = 0; i < node.attribute(axes_idx).ints_size(); i++)
      {
        layer.layerParams.resizeParams.resizeRatio[axes[i]] = resizeRatios[i];
      }
      my_free(resizeRatios);
    }
    else if(node.attribute(axes_idx).ints_size() == sizes.bufSize)
    {
      /* Read axes values */
      axes = (int32_t*)malloc (node.attribute(axes_idx).ints_size() * sizeof (int32_t));
      for (int32_t i = 0; i < node.attribute(axes_idx).ints_size(); i++)
      {
        getIntAttr(node, "axes", &axes[i], i);
        /* Adjust axes[i] to TIDL dimension */
        if(axes[i] >= 0)
        {
          /*+ve Axis*/
          axes[i] += dimOffset;
        }
        else
        {
          /*-ve Axis*/
          axes[i] += TIDL_DIM_MAX;
        }
      }
      /* calculate resize ratio using (output_shape / input_shape) */
      float32_tidl resizeRatios[sizes.bufSize];
      int64_t *sizesPtr = (int64_t *)sizes.ptr;
      for (int32_t i = 0; i < node.attribute(axes_idx).ints_size(); i++)
      {
        layer.layerParams.resizeParams.resizeRatio[axes[i]] = (sizesPtr[i] * 1.0) / (inputDims[axes[i]-dimOffset]);
      }
      my_free(sizesPtr);
    }
    else
    {
      TIDL_LOG_ERROR(gDiags.gDiagList, "Axes length = %d should match with sizes/scales length (whichever is specified)", node.attribute(axes_idx).ints_size());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  /* No axes, fill resize ration acc to scales or sizes */
  else
  {
    if(scales.bufSize > 0)
    {
      if(scales.bufSize != numDims)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Scales size = %d should match with input rank = %d", scales.bufSize, numDims);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      float *scalesPtr = (float *)scales.ptr;
      for (int i = 0; i < scales.bufSize; i++)
      {
        layer.layerParams.resizeParams.resizeRatio[i+dimOffset] = scalesPtr[i];
      }
      my_free(scalesPtr);
    }
    else if(sizes.bufSize > 0)
    {
      if(sizes.bufSize != numDims)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Sizes length = %d should match with input rank = %d", sizes.bufSize, numDims);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      int64_t *sizesPtr = (int64_t *)sizes.ptr;
      for(int32_t i = 0; i < inputDims.size(); i++)
      {
        layer.layerParams.resizeParams.resizeRatio[i+dimOffset] = (sizesPtr[i]*1.0) / inputDims[i];
      }
      my_free(sizesPtr);
    }
  }

  return 0;
}

