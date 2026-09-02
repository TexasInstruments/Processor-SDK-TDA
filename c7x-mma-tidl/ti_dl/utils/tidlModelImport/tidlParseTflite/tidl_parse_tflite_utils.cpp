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

#include <string>
#include "tidl_parse_tflite.h"
#include "tidl_import_common.h"
#include "tidl_import_config.h"
#include "tensorflow/lite/context_util.h"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/minimal_logging.h"


void TidlParseTflite::fillActParams(sTIDL_ActParams_t & actParams, int32_t tfLiteActType)
{
  actParams.actType = TIDL_NoAct;
  if (tfLiteActType == kTfLiteActRelu)
  {
    actParams.actType = TIDL_RelU;
  }
  if (tfLiteActType == kTfLiteActRelu6)
  {
    actParams.actType = TIDL_RelU6;
  }
  if(tfLiteActType == kTfLiteActSigmoid)
  {
    actParams.actType = TIDL_Sigmoid;
  }
  if(tfLiteActType == kTfLiteActTanh)
  {
    actParams.actType = TIDL_HardSigmoid;
  }
  if(tfLiteActType == kTfLiteActTanh)
  {
    actParams.actType = TIDL_ELU;
  }
  if(tfLiteActType == kTfLiteActTanh)
  {
    actParams.actType = TIDL_Tanh;
  }

}


/*
 * This function is used to copy constant input tensors e.g. weights/biases in conv layer to TIDL layer buffers
 * Also saves corresponding scale and zero point values
 */
int32_t TidlParseTflite::copyInputConstTensor(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context,
                                            int32_t inIdx, sBuffer_t &buf, sBuffer_t &scaleBuf, sBuffer_t &zpBuf)
{

  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[inIdx]];

  auto * ptr = reinterpret_cast<float *>(input->data.raw);

  if(input->type == kTfLiteFloat32)
  {
    buf.dataType = TIDL_SinglePrecFloat;
    buf.bufSize = input->bytes / sizeof(float);
    buf.ptr = (float *)my_malloc(buf.bufSize*sizeof(float));
    memcpy(buf.ptr, ptr, buf.bufSize*sizeof(float));
    scaleBuf.ptr = NULL;
    zpBuf.ptr = NULL;
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  else if (input->type == kTfLiteInt32)
  {
    buf.dataType = TIDL_SignedWord;
    buf.bufSize = input->bytes / sizeof(int);
    buf.ptr = (float *)my_malloc(buf.bufSize*sizeof(float));
    memcpy(buf.ptr, ptr, buf.bufSize*sizeof(int));

    const auto* quantization =
        reinterpret_cast<TfLiteAffineQuantization*>(input->quantization.params);
    auto *scale = quantization->scale;

    if (scale)
    {
      scaleBuf.bufSize = scale->size;
      scaleBuf.ptr = (float *)my_malloc(scaleBuf.bufSize*sizeof(float));
      scaleBuf.dataType = TIDL_SinglePrecFloat;
      zpBuf.bufSize = scaleBuf.bufSize;
      zpBuf.ptr = (int *)my_malloc(zpBuf.bufSize * sizeof(int));
      zpBuf.dataType = TIDL_SignedWord;

      float * qscale        = (float *)scaleBuf.ptr;
      int32_t * qzero_point = (int32_t*)zpBuf.ptr;
      for (int i = 0; i < scaleBuf.bufSize; i++)
      {
        qscale[i] = scale->data[i];
        qzero_point[i] = 0;
      }
    }
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  else if ((input->type == kTfLiteUInt8) || (input->type == kTfLiteInt8))
  {
    buf.bufSize = input->bytes;
    buf.ptr = (float *)my_malloc(buf.bufSize*sizeof(float));

    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
      buf.ptr = (float *)my_malloc(buf.bufSize*sizeof(float));
    }
    else
    {
      buf.ptr = (int8_t *)my_malloc(buf.bufSize*sizeof(int8_t));
    }

    if(input->type == kTfLiteUInt8)
    {
      if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
      {
        int* dst = (int*)buf.ptr;
        buf.dataType = TIDL_SignedWord;
        uint8_t * src = (uint8_t *)ptr;
        for (int i = 0; i < buf.bufSize; i++)
        {
          dst[i] = src[i];
        }
      }
      else
      {
        uint8_t * dst = (uint8_t *)buf.ptr;
        buf.dataType = TIDL_UnsignedChar;
        uint8_t * src = (uint8_t *)ptr;
        for (int i = 0; i < buf.bufSize; i++)
        {
          dst[i] = src[i];
        }
      }
    }
    else // (input->type == kTfLiteInt8)
    {
      if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
      {
        int* dst = (int*)buf.ptr;
        buf.dataType = TIDL_SignedWord;
        int8_t * src = (int8_t *)ptr;
        for (int i = 0; i < buf.bufSize; i++)
        {
          dst[i] = src[i];
        }
      }
      else
      {
        int8_t * dst = (int8_t *)buf.ptr;
        buf.dataType = TIDL_SignedChar;
        int8_t * src = (int8_t *)ptr;
        for (int i = 0; i < buf.bufSize; i++)
        {
          dst[i] = src[i];
        }
      }
    }

    const auto* quantization =
        reinterpret_cast<TfLiteAffineQuantization*>(input->quantization.params);
    auto *scale = quantization->scale;
    auto *zero_point = quantization->zero_point;

    if (scale && zero_point)
    {
      if(scale->size != zero_point->size)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Size of scale vector and zero_point shall match. It is not matching for tensor %s, Aborting", input->name);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      scaleBuf.bufSize = scale->size;
      scaleBuf.ptr = (float *)my_malloc(scaleBuf.bufSize*sizeof(float));
      scaleBuf.dataType = TIDL_SinglePrecFloat;
      zpBuf.bufSize = zero_point->size;
      zpBuf.ptr = (int *)my_malloc(zpBuf.bufSize*sizeof(int));
      zpBuf.dataType = TIDL_SignedWord;

      float * qscale        = (float *)scaleBuf.ptr;
      int32_t * qzero_point = (int32_t*)zpBuf.ptr;
      for (int i = 0; i < scaleBuf.bufSize; i++)
      {
        qscale[i] = scale->data[i];
        qzero_point[i] = zero_point->data[i];
      }
    }
    else
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Scale vector or zero_point not found for tensor %s", input->name);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Only float, DT_INT32, DT_UINT8 and DT_INT8 tensor is supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
}


/** Dequantize tensor and save as float */
void TidlParseTflite::dequantTensor(sBuffer_t &buf, sBuffer_t &scaleBuf, sBuffer_t &zpBuf, int32_t size)
{
  int   * src      = (int *)buf.ptr;
  float * dst      = (float *)buf.ptr;
  float * qscale   = (float *)scaleBuf.ptr;
  int * qzero_point = (int *)zpBuf.ptr;

  if((qscale != NULL) && (qzero_point != NULL))
  {
    int co = buf.bufSize / size;
    int numScales = scaleBuf.bufSize;
    for (int i = 0; i < co; i++)
    {
      for (int j = 0; j < size; j++)
      {
        int param = src[i*size+j] - qzero_point[i % numScales];
        dst[i*size + j] = param * qscale[i % numScales];
        if (param == qzero_point[i % numScales])
        {
          dst[i*size + j] = 0;
        }
      }
    }
    my_free(qscale);
    my_free(qzero_point);
  }
}


/** Find number of valid inputs of a tensor and populate corresponding indices
 * This function is needed since tflite layers can have some inputs with tensor location -1 rendering them invalid
 * e.g fully connected layer without bias has the bias tensor input with location -1 and hence is not valid
 */
int32_t TidlParseTflite::numValidInputs(const TfLiteNode* node, std::vector<int32_t> &validInputIndices)
{
  int32_t numValidInputs = 0;
  validInputIndices = {};
  int32_t tensorIdx = 0;
  for(auto & tensorLocation : TfLiteIntArrayView(node->inputs))
  {
    if(tensorLocation != -1)
    {
      validInputIndices.push_back(tensorIdx);
      numValidInputs++;
    }
    tensorIdx++;
  }
  return numValidInputs;
}


/** Function to compute number of variable inputs of layer
 * Also populates corresponding indices of variable inputs
 * Checks for variable tensors among the valid tensors if valid tensors list provided, else checks all tensors
 */
int32_t TidlParseTflite::numVariableLayerInputs(TfLiteContext* context, const TfLiteNode* node, std::vector<int32_t> &varIdx, const std::vector<int32_t> validInputIndices = {})
{
  int32_t numVarInputs = 0;
  int32_t numInputs = 0;
  if(! validInputIndices.empty())
  {
    for(int j = 0; j < validInputIndices.size(); j++)
    {
      if(isTensorVariable(&context->tensors[node->inputs->data[validInputIndices[j]]]))
      {
        numVarInputs++;
        varIdx.push_back(validInputIndices[j]);
      }
    }
  }
  else  // default argument {} passed for validInputIndices. In this case, iterate over all node inputs
  {
    for(int j = 0; j < node->inputs->size; j++)
    {
      if(isTensorVariable(&context->tensors[node->inputs->data[j]]))
      {
        numVarInputs++;
        varIdx.push_back(j);
      }
    }
  }
  return numVarInputs;
}


/** This function checks if a given tensor is variable */
bool TidlParseTflite::isTensorVariable(TfLiteTensor * tensor)
{
  // Only kTfLiteArenaRw tensors are variable
  return (tensor->allocation_type == kTfLiteArenaRw);
}


/** Broadcast vector buffer to have a modified dimension */
void TidlParseTflite::broadcastFloatTensor(sBuffer_t &buf, int32_t bufSize)
{
  float * ptr = (float *)buf.ptr;
  float val = ptr[0]; //only one const val for computation
  void * newPtr = realloc(ptr, bufSize * sizeof(float)); //ptr is void* so it should be reallocated to a void * ptr
  float *newPtrCast = (float *)newPtr;
  for(int i = 0; i < bufSize; i++)
  {
    newPtrCast[i] = val;  //broadcast the constant along number of channels
  }
  buf.ptr = newPtr;
  buf.bufSize = bufSize;
}

