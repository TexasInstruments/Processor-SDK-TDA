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

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/context_util.h"

#include "itidl_ti.h"
#include "itidl_rt.h"
#include "stdio.h"
#include "tidl_runtimes_infer_common.h"

namespace tflite{

extern "C"{

/** Get TIDL data type, element size and the runtime data pointer based on the runtime data type */
int32_t TIDL_tfliteRtGetTypeAndPtr(TfLiteTensor *tensor, int32_t * type, void ** ptr, int32_t * size)
{
  int32_t status = 0;
  if(tensor->type == kTfLiteUInt8)
  {
    *type =  TIDL_UnsignedChar;
    *ptr = (tensor->data.uint8);
    *size = sizeof(uint8_t);
  }
  else if(tensor->type == kTfLiteInt8)
  {
    *type =  TIDL_SignedChar;
    *ptr = (tensor->data.int8);
    *size = sizeof(int8_t);
  }
  else if(tensor->type == kTfLiteInt16)
  {
    *type =  TIDL_SignedShort;
    *ptr = (tensor->data.i16);
    *size = sizeof(int16_t);
  }
  else if(tensor->type == kTfLiteFloat32)
  {
    *type =  TIDL_SinglePrecFloat;
    *ptr = (tensor->data.f);
    *size = sizeof(float);
  }
  else if(tensor->type == kTfLiteInt32)
  {
    *type =  TIDLRT_Int32;
    *ptr = (tensor->data.i32);
    *size = sizeof(int32_t);
  }
  else if(tensor->type == kTfLiteInt64)
  {
    *type =  TIDLRT_Int64;
    *ptr = (tensor->data.i64);
    *size = sizeof(int64_t);
  }
  else
  {
    printf("ERROR : Tflite data type : %d not supported by TIDL\n", (int32_t)(tensor->type));
    status = -1;
  }
  return status;
}

void TIDL_tfliteRtGetScaleAndZeroPoint(TfLiteTensor *tensor , float * scale, int32_t * zp)
{
    if ((tensor->type == kTfLiteUInt8) || (tensor->type == kTfLiteInt8))
    {
        TfLiteAffineQuantization *quantization = (TfLiteAffineQuantization *)tensor->quantization.params;
        *zp = quantization->zero_point->data[0];
        *scale = 1 / quantization->scale->data[0];
    }
    else 
    {
      *zp    = 0;
      *scale = 1.0;
    }
}

/** We need some id to identify a subgraph with, so get the tensor index of one of the output nodes of the subgraph
 * from tflite runtime, and use that as subgraph id */
int32_t TIDL_getOneOutputTensorIdx(TfLiteContext* context, const TfLiteDelegateParams* params)
{
    int32_t outTensorIdx = 0;
    // Return the index of the first output encountered - this is used to identify subgraph, so one is enough
    for (auto tensor_index : TfLiteIntArrayView(params->output_tensors)) 
    {
        if (tensor_index == kTfLiteOptionalTensor) 
        {
            continue;
        }
        TfLiteTensor* tensor = &context->tensors[tensor_index];
        // Const tensors should be added as const nodes during graph construction.
        if(tensor->allocation_type == kTfLiteArenaRw)
        {
            outTensorIdx = tensor_index;
        }
    }
    return outTensorIdx;
}

/** Set TIDL RT input and output tensor properties required for calling invoke */
int32_t TIDL_setRtTensorParameters(TfLiteTensor * tfliteTensor, sTIDLRT_Tensor_t * allocatedPtrs, sTIDLRT_Tensor_t * rtPtrs[], 
                                          int32_t currTensorNum, sTIDL_IOBufDesc_t * ioBufDesc, 
                                          sTIDL_tidlRtDynamicLoading_t * infer_ops, int32_t isInfer, int32_t isInput)
{
  int32_t status = 0;

  float32_tidl scale;
  int32_t zeroPoint, elementType, elementSize, memType = 0, bufferSize = -1;
  uint8_t * ptr;

  int32_t numVirtualCores = ioBufDesc->numVirtualCores;
  int32_t numSuperBatches = ioBufDesc->numSuperBatches;
  int32_t numBatches = numVirtualCores * numSuperBatches;
  int32_t numIosPerCore;
  if(isInput == 1)
  {
    numIosPerCore = ioBufDesc->numInputBuf / numBatches;
    /* Actual buffer size coming from OSRT*/
    bufferSize = 1;
    for (int32_t k = 0; k < tfliteTensor->dims->size; k++)
    {
      bufferSize *= tfliteTensor->dims->data[k];
    }
  }
  else
  {
    numIosPerCore = ioBufDesc->numOutputBuf / numBatches;
  }

  int32_t numElementsPerBatch = tfliteTensor->dims->data[tfliteTensor->dims->size - 1];
  for(int i = tfliteTensor->dims->size - 2; i > 0; i--)
  {
    numElementsPerBatch *= tfliteTensor->dims->data[i]; /* Multiply all dimensions other than batch (dimension 0) */
  }

  TIDL_tfliteRtGetScaleAndZeroPoint(tfliteTensor, &scale, &zeroPoint);
  status = TIDL_tfliteRtGetTypeAndPtr(tfliteTensor, &elementType, (void **)&ptr, &elementSize);
  if((tfliteTensor->allocation_type == kTfLiteCustom) && (infer_ops->TIDLRT_isSharedMem(ptr)) && isInfer)
  {
    memType = TIDLRT_MEM_SHARED;
  }

  for(int l = 0; l < numSuperBatches; l++)
  {
    for(int k = 0; k < numVirtualCores; k++)
    {
      int32_t idx = l * numVirtualCores * numIosPerCore + k * numIosPerCore + currTensorNum;
      rtPtrs[idx] = &(allocatedPtrs[idx]);
      status = infer_ops->TIDLRT_setTensorDefault(rtPtrs[idx]);
      rtPtrs[idx]->layout = TIDLRT_LT_NHWC;
      rtPtrs[idx]->bufferSize = bufferSize; /* This is set only for input to handle TIDL-4466, output buffer size is kept -1 by default */
      strcpy((char *)rtPtrs[idx]->name, tfliteTensor->name);
      rtPtrs[idx]->scale = scale;
      rtPtrs[idx]->zeroPoint = zeroPoint;
      rtPtrs[idx]->elementType = elementType;
      rtPtrs[idx]->ptr = ptr + l * numVirtualCores * numElementsPerBatch * elementSize + k * numElementsPerBatch * elementSize; /* Ptr must be start of each batch */
      rtPtrs[idx]->padValues[0] = ioBufDesc->inPadL[currTensorNum];
      rtPtrs[idx]->padValues[1] = ioBufDesc->inPadR[currTensorNum];
      rtPtrs[idx]->padValues[2] = ioBufDesc->inPadT[currTensorNum];
      rtPtrs[idx]->padValues[3] = ioBufDesc->inPadB[currTensorNum];
      if(memType == TIDLRT_MEM_SHARED)
      {
        rtPtrs[idx]->memType = TIDLRT_MEM_SHARED;
      }
    }
  }
  return status;
}

TfLiteStatus TIDL_subgraphRtInvoke(int32_t osrtDebugPrintLevel, TfLiteContext *context, TfLiteNode *node, sTIDL_tidlRtParams_t *tidlRtParams, sTIDL_tidlRtDynamicLoading_t * infer_ops, int32_t isInfer)
{
  TIDL_osrtDebugPrint(osrtDebugPrintLevel, "*******   In TIDL_subgraphRtInvoke  ******** \n");
  int status = 0;
  int currTensorNum = 0;

  void *handle = tidlRtParams->rtHandle;
  sTIDLRT_PerfStats_t *stats = (sTIDLRT_PerfStats_t *)tidlRtParams->stats;
  sTIDL_IOBufDesc_t * ioBufDesc = (sTIDL_IOBufDesc_t *)tidlRtParams->ioBufDesc;

  sTIDLRT_Tensor_t *in[128];
  sTIDLRT_Tensor_t *out[128];
  sTIDLRT_Tensor_t *ins;
  sTIDLRT_Tensor_t *outs;

  ins = (sTIDLRT_Tensor_t *)tidlRtParams->rtInList;
  outs = (sTIDLRT_Tensor_t *)tidlRtParams->rtOutList;

  if ((ins == NULL) || (outs == NULL))
  {
    printf("Invoke  : ERROR: Unable to allocate memory for TIDL RT in[] out [] tensor struct\n");
    return kTfLiteError;
  }
  else
  {
    /* Input tesnsors property set up */
    currTensorNum = 0;
    for (auto tensor_index : TfLiteIntArrayView(node->inputs))
    {
        if (tensor_index == kTfLiteOptionalTensor)
        {
            continue;
        }
        TfLiteTensor *tensor = &context->tensors[tensor_index];
        if ((tensor->allocation_type == kTfLiteArenaRw) ||
            (tensor->allocation_type == kTfLiteCustom))
        {
          status = TIDL_setRtTensorParameters(tensor, ins, in, currTensorNum, ioBufDesc, infer_ops, isInfer, 1);
          
          if(status == -1)
          {
              return kTfLiteError;
          }
          currTensorNum++;
      }
    }

    /* Output tensors property set up */
    currTensorNum = 0;
    for (auto tensor_index : TfLiteIntArrayView(node->outputs))
    {
        if (tensor_index == kTfLiteOptionalTensor)
        {
            continue;
        }
        TfLiteTensor *tensor = &context->tensors[tensor_index];
        if ((tensor->allocation_type == kTfLiteArenaRw ) ||
              (tensor->allocation_type == kTfLiteCustom) )
        {
          status = TIDL_setRtTensorParameters(tensor, outs, out, currTensorNum, ioBufDesc, infer_ops, isInfer, 0);
          if(status == -1)
          {
              return kTfLiteError;
          }
          currTensorNum++;
        }
    }
  }
  status = infer_ops->TIDLRT_invoke(handle, in, out);

  if(osrtDebugPrintLevel)
  {
    TIDL_printSubgraphStats(stats);
  }
  TIDL_osrtDebugPrint(osrtDebugPrintLevel, "*******  TIDL_subgraphRtInvoke done  ******** \n");
  
  return kTfLiteOk;
}

} //extern "C"
} //namespace tflite

