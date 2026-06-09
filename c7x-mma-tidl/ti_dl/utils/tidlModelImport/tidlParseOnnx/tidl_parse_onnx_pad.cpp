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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Pad")> ()
{
  int32_t idx, j;
  layer.layerType = TIDL_PadLayer;

  NodeProto node = graph.node(index);
  int32_t status;
  int32_t padT = 0, padB= 0, padL= 0, padR= 0;
  int32_t tot_axis, atrIdx, axes_status, numDims;
  char padMode[50];
  float32_tidl constant_value;
  sBuffer_t buf, axes;
  int32_t* padValStore;

  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDims = (numDims == 0)? 4:numDims;
  }
  else
  {
    numDims = 4;
  }

  int32_t padModeStatus = getStringAttr(node, "mode", padMode, 0);
  if (padModeStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    // Default pad mode
    strcpy(padMode, "constant");
  }

  // Only constant pad mode is supported
  if ((strcmp(padMode, "constant") != 0))
  {
    layer.layerParams.padLayerParams.padType = TIDL_PadModeUnsupported;
  }
  else
  {
    constant_value = 0;
    /* Pad v1 & v2 can have value in Attributes */
    atrIdx = getFloatAttr(node, "value", &constant_value, 0);
    if(atrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      layer.layerParams.padLayerParams.padConstValue = (int32_t)constant_value;
    }
    else
    {
      /* In other cases pad value can be in 3rd input */
      if (node.input_size() > 2)
      {
        atrIdx = copyFloatConst(graph, index, 2, buf, INPUT_NOT_REQUIRED);
        if (atrIdx != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          float32_tidl *padPtr = (float32_tidl *)buf.ptr;
          constant_value = padPtr[0];
          layer.layerParams.padLayerParams.padConstValue = (int32_t)padPtr[0];
          free(buf.ptr);
          buf.bufSize = 0;
          buf.ptr = NULL;
        }
        /* If still not found, set default value as 0 */
        else
        {
          layer.layerParams.padLayerParams.padConstValue = 0;
        }
      }
      /* If still not found, set default value as 0 */
      else
      {
        layer.layerParams.padLayerParams.padConstValue = 0;
      }
    }

    /* Only zero pad is supported */
    if(constant_value != 0)
    {
      layer.layerParams.padLayerParams.padType = TIDL_PadModeUnsupported;
    }
    else
    {
      layer.layerParams.padLayerParams.padType = TIDL_PadZero;
    }

    /* Axes can be input in v18 */
    axes_status = copyFloatConst(graph, index, 3, axes, INPUT_NOT_REQUIRED);
    int64_t *axesPtr = (int64_t *)axes.ptr; /* The axesPtr points to the pointer in the axes buffer.*/

    /* If the optional input axes is populated.*/
    if(axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      /** Iterating through the axes buffer.
       * Ignore repetition as described as undefined behavior in onnx
      */
      for(int j = 0; j < axes.bufSize; j++)
      {
        /* Adjust to TIDL dimensions */
        if(axesPtr[j] >= 0)
        {
          /*+ve Axis*/
          axesPtr[j] += (TIDL_DIM_MAX - numDims);
        }
        else
        {
          /*-ve Axis*/
          axesPtr[j] += TIDL_DIM_MAX;
        }

        /* If axes are not height or width, do not support */
        if ((axesPtr[j] != TIDL_DIM_HEIGHT) && (axesPtr[j] != TIDL_DIM_WIDTH))
        {
          layer.layerParams.padLayerParams.padType = TIDL_PadNonWidthHeight;
        }
      }
    }

    /**
     * Pad can be in attribute
     * in v1 named as padding
     * in v2 named as pads
    */
    atrIdx = getAttrIdx(node, "pads");
    if (atrIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      atrIdx = getAttrIdx(node, "paddings");
    }
    /* If found in attribute */
    if (atrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      tot_axis = node.attribute(atrIdx).ints_size() >> 1;
      padValStore = (int32_t *)my_malloc(tot_axis * 2 * sizeof(int32_t));

      for (j = 0; j < tot_axis*2; j++)
      {
        getIntAttr(node, "pads", &padValStore[j], j);
      }

      if(tot_axis > 2) /* more than width and height */
      {
        for (j = 0; j < (tot_axis-2); j++)
        {
          /* Cannot be non-zero in these dimensions */
          if(( padValStore[j] != 0 ) || ( padValStore[tot_axis + j] != 0 ))
          {
            layer.layerParams.padLayerParams.padType = TIDL_PadNonWidthHeight;
          }
        }
      }

      padT = (tot_axis >= 2)    ?  padValStore[tot_axis   - 2] : 0;
      padB = (tot_axis*2 >= 2)  ?  padValStore[tot_axis*2 - 2] : 0;
      padL = (tot_axis >= 1)    ?  padValStore[tot_axis   - 1] : 0;
      padR = (tot_axis*2 >= 1)  ?  padValStore[tot_axis*2 - 1] : 0;
      my_free(padValStore);
    }
    /* Pad value can be in 2nd input */
    else
    {
      status = copyFloatConst(graph, index, 1, buf, INPUT_REQUIRED);
      if(status != 0)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to find pad input intializer tensor");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      int64_t *padPtr = (int64_t *)buf.ptr;
      tot_axis = buf.bufSize >> 1;

      /* Axes is present */
      if(axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        if (tot_axis != axes.bufSize) /*The size of the axes tensor must be half the pad input size if axes exists*/
        {
          TIDL_LOG_ERROR(gDiags.gDiagList, "Pad layer input tensor axes and pads dimensions are not matching");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        else
        {
          for (int j = 0; j < tot_axis; j++)
          {
            if (axesPtr[j] == TIDL_DIM_WIDTH)
            {
              padL = padPtr[j];
              padR = padPtr[tot_axis+j];
            }
            else if (axesPtr[j] == TIDL_DIM_HEIGHT)
            {
              padT = padPtr[j];
              padB = padPtr[tot_axis+j];
            }
            else
            {
              layer.layerParams.padLayerParams.padType = TIDL_PadNonWidthHeight;
            }
          }
        }
      }
      /* Axes is not given */
      else
      {
        if(tot_axis > 2) /*more than width and height*/
        {
          for (j = 0; j < (tot_axis-2); j++)
          {
            /* Cannot be non-zero in these dimensions */
            if(( padPtr[j] != 0 ) || ( padPtr[tot_axis + j] != 0 ))
            {
              layer.layerParams.padLayerParams.padType = TIDL_PadNonWidthHeight;
            }
          }
        }

        padT = (tot_axis >= 2)    ? padPtr[tot_axis   - 2] : 0;
        padB = (tot_axis*2 >= 2)  ? padPtr[tot_axis*2 - 2] : 0;
        padL = (tot_axis >= 1)    ? padPtr[tot_axis   - 1] : 0;
        padR = (tot_axis*2 >= 1)  ? padPtr[tot_axis*2 - 1] : 0;
      }
      my_free(buf.ptr);
    }

    /* if axis was being used, free axis ptr */
    if (axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      my_free(axes.ptr);
      axes.bufSize = 0;
      axes.ptr = NULL;
    }

    layer.layerParams.padLayerParams.padT = padT;
    layer.layerParams.padLayerParams.padB = padB;
    layer.layerParams.padLayerParams.padL = padL;
    layer.layerParams.padLayerParams.padR = padR;
  }
  return 0;
}

