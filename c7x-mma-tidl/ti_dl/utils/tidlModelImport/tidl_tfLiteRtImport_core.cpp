/*
*
* Copyright (c) {2015 - 2017} Texas Instruments Incorporated
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

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
using namespace std;
using ::google::protobuf::Message;
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <cmath>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <fstream>

#ifdef CREATE_METAARCH
#include "object_detection/protos/ssd.pb.h"
#include "object_detection/protos/image_resizer.pb.h"
#include "object_detection/protos/box_coder.pb.h"
#include "object_detection/protos/faster_rcnn_box_coder.pb.h"
#include "object_detection/protos/anchor_generator.pb.h"
#include "object_detection/protos/ssd_anchor_generator.pb.h"
#include "object_detection/protos/post_processing.pb.h"
#endif

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "schema_generated.h"
#include "flatbuffers/flexbuffers.h"
#include "tidl_import_config.h"

using namespace std;
using namespace tflite;

#include "object_detection/protos/pipeline.pb.h"
#include "tidl_import_common.h"
#include "tidl_import_common_model_check.h"
#include "itidl_ti.h"
#include "tidl_meta_arch.pb.h"
#include "tidl_quant_params.pb.h"
#include "itidl_rt.h"

#include "tensorflow/lite/context_util.h"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/minimal_logging.h"

#include "tidl_tfLiteRtImport.h"
#include "tidl_tflrt_common.h"
#include "tidl_import_common_tidlrt_osrt.h"
#include "tidl_import_diag.h"

#include "tidlParseTflite/tidl_parse_tflite.h"
#include "tidlAllowlistingConstraints/tidl_constraint.h"

#define __MODULE__ "[PARSER]"

using namespace object_detection;
using namespace protos;

extern sTIDL_runtimesImportState_t runtimes_import_state;
#define TIDL_MAX_ALG_IN_BUFS    ((int32_t) 32)

namespace tflite {
namespace tfl_delegate {

#ifdef __cplusplus
extern "C"
{
#endif
// Functions

extern TIDL_TFSSDConfig_t tidl_TFSSDConfigs[TIDL_MAX_TF_SSD_LAYERS];

/* This function finds graph indices corresponding to names of OD heads */
std::vector<int> TIDL_getPostProcInputIds(TfLiteContext* context, TfLiteIntArray* plan, std::vector<std::string> odPostProcIndataNames)
{
  std::vector<int> odPostProcIndataIds = {};
  TfLiteNode* node;
  TfLiteRegistration* registration;
  for(int i = 0; i < odPostProcIndataNames.size(); i++)
  {
    for (int node_index : TfLiteIntArrayView(plan))
    {
      //TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(context, node_index, &node, &registration));
      context->GetNodeAndRegistration(context, node_index, &node, &registration);
      for (auto tensor_index : TfLiteIntArrayView(node->outputs))
      {
        if (tensor_index == kTfLiteOptionalTensor)
        {
            continue;
        }
        TfLiteTensor *tensor = &context->tensors[tensor_index];

        if((strcmp(odPostProcIndataNames[i].c_str(), tensor->name)) == 0)
        {
          odPostProcIndataIds.push_back(node_index);
        }
      }
    }
  }
  return odPostProcIndataIds;
}

/** Create adjacency list of outputs of all nodes in graph
 * Returns list of form < (output node x1, output node y1...), (output node x2, output node y2...) > for nodes < node 1, node 2....>
 * */
std::vector<std::vector<int>> TIDL_createOutputAdjacencyList(TfLiteContext* context, TfLiteIntArray* plan)
{
  std::vector<int> outputAdjacentNodes = {};
  std::vector<std::vector<int>> adjacencyList;

  TfLiteNode* node1, *node2;
  TfLiteRegistration* registration1, *registration2;

  for (int node_index1 : TfLiteIntArrayView(plan))
  {
    outputAdjacentNodes.clear();
    context->GetNodeAndRegistration(context, node_index1, &node1, &registration1);
    for (auto tensor_index_outputs : TfLiteIntArrayView(node1->outputs))
    {
      TfLiteTensor *output_tensor = &context->tensors[tensor_index_outputs];
      for (int node_index2 : TfLiteIntArrayView(plan))
      {
        context->GetNodeAndRegistration(context, node_index2, &node2, &registration2);
        for (auto tensor_index_inputs : TfLiteIntArrayView(node2->inputs))
        {
          if (tensor_index_inputs == kTfLiteOptionalTensor)
          {
            continue;
          }
          TfLiteTensor *input_tensor = &context->tensors[tensor_index_inputs];
          if((input_tensor->name != NULL) && (output_tensor->name != NULL))
          {
            if(strcmp(input_tensor->name, output_tensor->name) == 0)
            {
              outputAdjacentNodes.push_back(node_index2);
            }
          }
        }
      }
    }
    adjacencyList.push_back(outputAdjacentNodes);
  }
  return adjacencyList;
}

/** Create adjacency list of inputs of all nodes in graph
 * Returns list of form < (input node x1, input node y1...), (input node x2, input node y2...) > for nodes < node 1, node 2....>
 * */
std::vector<std::vector<int>> TIDL_createInputAdjacencyList(TfLiteContext* context, TfLiteIntArray* plan)
{
  std::vector<int> inputAdjacentNodes = {};
  std::vector<std::vector<int>> adjacencyList;

  TfLiteNode* node1, *node2;
  TfLiteRegistration* registration1, *registration2;

  for (int node_index1 : TfLiteIntArrayView(plan))
  {
    inputAdjacentNodes.clear();
    context->GetNodeAndRegistration(context, node_index1, &node1, &registration1);
    for (auto tensor_index_inputs : TfLiteIntArrayView(node1->inputs))
    {
      if (tensor_index_inputs == kTfLiteOptionalTensor)
      {
        continue;
      }
      TfLiteTensor *input_tensor = &context->tensors[tensor_index_inputs];
      for (int node_index2 : TfLiteIntArrayView(plan))
      {
        context->GetNodeAndRegistration(context, node_index2, &node2, &registration2);
        for (auto tensor_index_outputs : TfLiteIntArrayView(node2->outputs))
        {
          TfLiteTensor *output_tensor = &context->tensors[tensor_index_outputs];
          if((input_tensor->name != NULL) && (output_tensor->name != NULL))
          {
            if(strcmp(input_tensor->name, output_tensor->name) == 0)
            {
              inputAdjacentNodes.push_back(node_index2);
            }
          }
        }
      }
    }
    adjacencyList.push_back(inputAdjacentNodes);
  }
  return adjacencyList;
}

void TIDL_tfLiteRtFillActParams(sTIDL_ActParams_t & actParams, int32_t tfLiteActType)
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

#ifdef __cplusplus
}
#endif

template <class Tw>
uint32_t TIDL_tfliteRtKernelReshape(Tw * param, uint32_t w, uint32_t h, uint32_t ci, uint32_t co)
{
  Tw * tPtr = (Tw * )my_malloc(w*h*ci*co*sizeof(Tw));
  int32_t counter = 0;
  for(int l1 = 0; l1 < co; ++l1){
    for(int l = 0; l < ci; ++l){
      int k = l;
      for(int j = 1; j<=w*h; ++j){
        tPtr[counter] = param[l1*w*h*ci + k];
        k+=ci;
        counter++;
      }
    }
  }
  memcpy(param,tPtr,w*h*ci*co*sizeof(Tw));
  free(tPtr);
  return 0;
}

/** Dequantize tensor and save as float */
void TIDL_tfliteRtDequantTensor(sBuffer_t &buf, sBuffer_t &scaleBuf, sBuffer_t &zpBuf, int32_t size)
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

/** This function is used to copy constant input tensors e.g. weights/biases in conv layer to TIDL layer buffers
 * Also saves corresponding scale and zero point values
 */
int32_t TIDL_tfliteRtCopyInputConstTensor(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context,
                                            int32_t inIdx, sBuffer_t &buf, sBuffer_t &scaleBuf, sBuffer_t &zpBuf)
{

  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[inIdx]];

  auto * ptr = reinterpret_cast<float *>(input->data.raw);

  if(input->type == kTfLiteFloat32)
  {
    buf.bufSize = input->bytes / sizeof(float);
    buf.ptr = (float *)my_malloc(buf.bufSize*sizeof(float));
    memcpy(buf.ptr, ptr, buf.bufSize*sizeof(float));
    scaleBuf.ptr = NULL;
    zpBuf.ptr = NULL;
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  else if (input->type == kTfLiteInt32)
  {
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
      zpBuf.bufSize = scaleBuf.bufSize;
      zpBuf.ptr = (int *)my_malloc(zpBuf.bufSize * sizeof(int));

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
        uint8_t * src = (uint8_t *)ptr;
        for (int i = 0; i < buf.bufSize; i++)
        {
          dst[i] = src[i];
        }
      }
      else
      {
        uint8_t * dst = (uint8_t *)buf.ptr;
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
        int8_t * src = (int8_t *)ptr;
        for (int i = 0; i < buf.bufSize; i++)
        {
          dst[i] = src[i];
        }
      }
      else
      {
        int8_t * dst = (int8_t *)buf.ptr;
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
      zpBuf.bufSize = zero_point->size;
      zpBuf.ptr = (int *)my_malloc(zpBuf.bufSize*sizeof(int));

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

/** This function checks if a given tensor is variable */
bool TIDL_tfliteRtIsTensorVariable(TfLiteTensor * tensor)
{
  // Only kTfLiteArenaRw tensors are variable
  return (tensor->allocation_type == kTfLiteArenaRw);
}

/** Function to compute number of variable inputs of layer
 * Also populates corresponding indices of variable inputs
 * Checks for variable tensors among the valid tensors if valid tensors list provided, else checks all tensors
 */
int32_t TIDL_tfliteRtNumVariableLayerInputs(TfLiteContext* context, const TfLiteNode* node, std::vector<int32_t> &varIdx, const std::vector<int32_t> validInputIndices = {})
{
  int32_t numVarInputs = 0;
  int32_t numInputs = 0;
  if(! validInputIndices.empty())
  {
    for(int j = 0; j < validInputIndices.size(); j++)
    {
      if(TIDL_tfliteRtIsTensorVariable(&context->tensors[node->inputs->data[validInputIndices[j]]]))
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
      if(TIDL_tfliteRtIsTensorVariable(&context->tensors[node->inputs->data[j]]))
      {
        numVarInputs++;
        varIdx.push_back(j);
      }
    }
  }
  return numVarInputs;
}

/** Find number of valid inputs of a tensor and populate corresponding indices
 * This function is needed since tflite layers can have some inputs with tensor location -1 rendering them invalid
 * e.g fully connected layer without bias has the bias tensor input with location -1 and hence is not valid
 */
int32_t TIDL_tfliteRtNumValidInputs(const TfLiteNode* node, std::vector<int32_t> &validInputIndices)
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

/** Function to save layer level tensor properties metadata to be used for allowlisting
 * Saves data for sTIDL_allowlistingMetaData structure
 */
void TIDL_tfliteRtSaveAllowlistingMetaData(TfLiteContext* context, const TfLiteNode* node, sTIDL_LayerPC_t &layer)
{
  layer.allowlistingMetaData.numInputs = node->inputs->size;
  layer.allowlistingMetaData.numOutputs = node->outputs->size;
  layer.allowlistingMetaData.numValidInputs = TIDL_tfliteRtNumValidInputs(node, layer.allowlistingMetaData.validInputIndices);
  layer.allowlistingMetaData.numVarInputs = TIDL_tfliteRtNumVariableLayerInputs(context, node, layer.allowlistingMetaData.varTensorIndices, layer.allowlistingMetaData.validInputIndices);
  layer.allowlistingMetaData.numConstInputs = layer.allowlistingMetaData.numValidInputs - layer.allowlistingMetaData.numVarInputs;

  //Find indices of constant tensors -- indices which are not variable are const
  for(int j = 0; j < layer.allowlistingMetaData.validInputIndices.size(); j++)
  {
    if(std::find(layer.allowlistingMetaData.varTensorIndices.begin(), layer.allowlistingMetaData.varTensorIndices.end(), layer.allowlistingMetaData.validInputIndices[j])
         == layer.allowlistingMetaData.varTensorIndices.end()) //idx not found in variable tensors indices => constant tensor idx
    {
      layer.allowlistingMetaData.constTensorIndices.push_back(layer.allowlistingMetaData.validInputIndices[j]);
    }
  }

  //save dimensions of variable and constant tensors
  for(auto& varIdx : layer.allowlistingMetaData.varTensorIndices)
  {
    const TfLiteTensor* varTensor = &context->tensors[node->inputs->data[varIdx]];
    std::vector<int32_t> tensorDim;
    for(int i = 0; i < varTensor->dims->size; i++)
    {
      tensorDim.push_back(varTensor->dims->data[i]);
    }
    layer.allowlistingMetaData.varTensorsDims.push_back(tensorDim);
  }
  for(auto& constIdx : layer.allowlistingMetaData.constTensorIndices)
  {
    const TfLiteTensor* constTensor = &context->tensors[node->inputs->data[constIdx]];
    std::vector<int32_t> tensorDim;
    for(int i = 0; i < constTensor->dims->size; i++)
    {
      tensorDim.push_back(constTensor->dims->data[i]);
    }
    layer.allowlistingMetaData.constTensorsDims.push_back(tensorDim);
  }
  for(int outputIdx=0; outputIdx<layer.allowlistingMetaData.numOutputs; outputIdx++)
  {
    const TfLiteTensor* outputTensor = &context->tensors[node->outputs->data[outputIdx]];
    std::vector<int32_t> tensorDim;
    for(int i = 0; i < outputTensor->dims->size; i++)
    {
      tensorDim.push_back(outputTensor->dims->data[i]);
    }
    layer.allowlistingMetaData.outputTensorDims.push_back(tensorDim);
  }

  const TfLiteTensor* outTensor = &context->tensors[node->outputs->data[0]];
  std::vector<int32_t> tensorDim;
  for(int i = 0; i < outTensor->dims->size; i++)
  {
    tensorDim.push_back(outTensor->dims->data[i]);
  }
  layer.allowlistingMetaData.outputTensorDims.push_back(tensorDim);

  //convert NHWC to NCHW dimensions for compatibility with ONNX RT checks
  for(int i = 0; i < layer.allowlistingMetaData.varTensorsDims.size(); i++)
  {
    if(layer.allowlistingMetaData.varTensorsDims[i].size() == 4)
    {
      std::vector<int32_t> tempDims = layer.allowlistingMetaData.varTensorsDims[i];
      layer.allowlistingMetaData.varTensorsDims[i][2] = tempDims[1];
      layer.allowlistingMetaData.varTensorsDims[i][3] = tempDims[2];
      layer.allowlistingMetaData.varTensorsDims[i][1] = tempDims[3];
    }
  }
  for(int i = 0; i < layer.allowlistingMetaData.constTensorsDims.size(); i++)
  {
    /* Conversion of hwc to chw needs to be supported,
    anything beyond 3 dims can be converted */
    if(layer.allowlistingMetaData.constTensorsDims[i].size() >= 3)
    {
      std::vector<int32_t> tempDims = layer.allowlistingMetaData.constTensorsDims[i];
      layer.allowlistingMetaData.constTensorsDims[i][2] = tempDims[1];
      layer.allowlistingMetaData.constTensorsDims[i][3] = tempDims[2];
      layer.allowlistingMetaData.constTensorsDims[i][1] = tempDims[3];
    }
  }
  for(int i = 0; i < layer.allowlistingMetaData.outputTensorDims.size(); i++)
  {
    if(layer.allowlistingMetaData.outputTensorDims[i].size() == 4)
    {
      std::vector<int32_t> tempDims = layer.allowlistingMetaData.outputTensorDims[i];
      layer.allowlistingMetaData.outputTensorDims[i][2] = tempDims[1];
      layer.allowlistingMetaData.outputTensorDims[i][3] = tempDims[2];
      layer.allowlistingMetaData.outputTensorDims[i][1] = tempDims[3];
    }
  }
}

int32_t TIDL_tfliteRtMapConvParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status;
  int32_t padType;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  sTIDL_ConvParams_t &convParams      = layer.layerParams.convParams;

  layer.layerType = TIDL_ConvolutionLayer;

  layer.outData[0].elementType = tidl_getElementType(1);

  TfLiteTensor* output;
  output = &context->tensors[node->outputs->data[0]];
  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[0]];
  const TfLiteTensor* filter;
  filter = &context->tensors[node->inputs->data[1]];

  // OHWI layout for tflite conv filter
  convParams.numInChannels   = filter->dims->data[3];
  convParams.numOutChannels  = filter->dims->data[0];
  convParams.kernelW         = filter->dims->data[2];
  convParams.kernelH         = filter->dims->data[1];

  convParams.numGroups       = 1;
  convParams.dilationW       = 1;
  convParams.dilationH       = 1;
  convParams.strideW         = 1;
  convParams.strideH         = 1;
  convParams.padW            = 0;
  convParams.padH            = 0;
  convParams.padL            = -1;
  convParams.padR            = -1;
  convParams.padT            = -1;
  convParams.padB            = -1;
  convParams.enableBias      = 0;
  convParams.enablePooling   = 0;

  const TfLiteConvParams* conv_params =
          reinterpret_cast<const TfLiteConvParams*>(node->builtin_data);

  convParams.strideW = conv_params->stride_width;
  convParams.strideH = conv_params->stride_height;
  convParams.dilationW = conv_params->dilation_width_factor;
  convParams.dilationH = conv_params->dilation_height_factor;

  TIDL_tfLiteRtFillActParams(layer.actParams,  conv_params->activation);

  padType = conv_params->padding;
  layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
  if (padType == kTfLitePaddingSame)   // SAME : Padding done to input
  {
    convParams.padW = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    convParams.padH = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
  }
  else if (padType == kTfLitePaddingValid)  // VALID : No padding to input
  {
    layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    convParams.padL            = 0;
    convParams.padR            = 0;
    convParams.padT            = 0;
    convParams.padB            = 0;
  }

  status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, 1, layer.weights, layer.weightScales, layer.weightZeroPoints);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED) return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
  {
    TIDL_tfliteRtKernelReshape((float *)layer.weights.ptr, convParams.kernelW, convParams.kernelH, convParams.numInChannels, convParams.numOutChannels);
    TIDL_tfliteRtDequantTensor(layer.weights, layer.weightScales, layer.weightZeroPoints, convParams.kernelW * convParams.kernelH * convParams.numInChannels);
  }
  else
  {
    TIDL_tfliteRtKernelReshape((int8_t *)layer.weights.ptr, convParams.kernelW, convParams.kernelH, convParams.numInChannels, convParams.numOutChannels);
  }

  if (node->inputs->size == 3)
  {
    status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, 2, layer.bias, layer.biasScales, layer.biasZeroPoints);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED) return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
      TIDL_tfliteRtDequantTensor(layer.bias, layer.biasScales, layer.biasZeroPoints, 1);
    }
    convParams.enableBias = 1;
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapDWConvParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status;
  int32_t padType;
  sTIDL_ConvParams_t &convParams      = layer.layerParams.convParams;
  int32_t depth_multiplier;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  layer.layerType = TIDL_ConvolutionLayer;
  layer.outData[0].elementType = tidl_getElementType(1);

  TfLiteTensor* output;
  output = &context->tensors[node->outputs->data[0]];
  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[0]];
  const TfLiteTensor* filter;
  filter = &context->tensors[node->inputs->data[1]];

  // OHWI layout for tflite conv filter
  convParams.numOutChannels  = output->dims->data[3];
  convParams.kernelW         = filter->dims->data[2];
  convParams.kernelH         = filter->dims->data[1];

  convParams.numGroups       = 1;
  convParams.dilationW       = 1;
  convParams.dilationH       = 1;
  convParams.strideW         = 1;
  convParams.strideH         = 1;
  convParams.padL            = -1;
  convParams.padR            = -1;
  convParams.padT            = -1;
  convParams.padB            = -1;
  convParams.padW            = 0;
  convParams.padH            = 0;
  convParams.enableBias      = 0;
  convParams.enablePooling   = 0;

  const TfLiteDepthwiseConvParams* DWConv2DParams =
          reinterpret_cast<const TfLiteDepthwiseConvParams*>(node->builtin_data);

  convParams.strideW = DWConv2DParams->stride_width;
  convParams.strideH = DWConv2DParams->stride_height;
  convParams.dilationW = DWConv2DParams->dilation_width_factor;
  convParams.dilationH = DWConv2DParams->dilation_height_factor;
  depth_multiplier = DWConv2DParams->depth_multiplier;
  TIDL_tfLiteRtFillActParams(layer.actParams,  DWConv2DParams->activation);

  padType = DWConv2DParams->padding;
  layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
  if (padType == kTfLitePaddingSame)   /* SAME : Padding done to input */
  {
    convParams.padW = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    convParams.padH = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
  }
  else if (padType == kTfLitePaddingValid)  /* VALID : No padding to input */
  {
    layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    convParams.padL            = 0;
    convParams.padR            = 0;
    convParams.padT            = 0;
    convParams.padB            = 0;
  }

  convParams.numInChannels = (convParams.numOutChannels / depth_multiplier);
  convParams.numGroups      = convParams.numInChannels;

  status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, 1, layer.weights, layer.weightScales, layer.weightZeroPoints);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED) return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
  {
    TIDL_tfliteRtKernelReshape((float *)layer.weights.ptr, convParams.kernelW, convParams.kernelH,
    convParams.numOutChannels, convParams.numInChannels/ convParams.numGroups );
    TIDL_tfliteRtDequantTensor(layer.weights, layer.weightScales, layer.weightZeroPoints, convParams.kernelW * convParams.kernelH * (convParams.numInChannels/convParams.numGroups));
  }
  else
  {
    TIDL_tfliteRtKernelReshape((int8_t *)layer.weights.ptr, convParams.kernelW, convParams.kernelH,
    convParams.numOutChannels, convParams.numInChannels/ convParams.numGroups );
  }

  if(node->inputs->size == 3)
  {
    status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, 2, layer.bias, layer.biasScales, layer.biasZeroPoints);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED) return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
      TIDL_tfliteRtDequantTensor(layer.bias, layer.biasScales, layer.biasZeroPoints, 1);
    }
    convParams.enableBias = 1;
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapMaxPoolParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  status = TIDL_checkPoolingTensorProperties(layer);
  if(status == -1) return -1;

  int32_t padType;

  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;

  layer.layerType = TIDL_PoolingLayer;
  poolParams.poolingType = TIDL_MaxPooling;

  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;

  const TfLitePoolParams* Pool2DParams =
          reinterpret_cast<const TfLitePoolParams*>(node->builtin_data);
  poolParams.strideW = Pool2DParams->stride_width;
  poolParams.strideH = Pool2DParams->stride_height;
  poolParams.kernelW = Pool2DParams->filter_width;
  poolParams.kernelH = Pool2DParams->filter_height;

  padType = Pool2DParams->padding;
  if (padType == kTfLitePaddingSame)
  {
    poolParams.padW = ((poolParams.kernelW - 1)) / 2;
    poolParams.padH = ((poolParams.kernelH - 1)) / 2;
    poolParams.padT = ((poolParams.kernelH - 1)) / 2;
    poolParams.padB = ((poolParams.kernelH - 1)) / 2;
    poolParams.padL = ((poolParams.kernelW - 1)) / 2;
    poolParams.padR = ((poolParams.kernelW - 1)) / 2;
  }

  return 0;
}

int32_t TIDL_tfliteRtMapAvgPoolParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  status = TIDL_checkPoolingTensorProperties(layer);
  if(status == -1) return -1;

  TfLiteTensor * tensor = &context->tensors[node->inputs->data[0]];

  layer.layerType = TIDL_PoolingLayer;
  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;
  poolParams.useCeil = 0;  // TODO : check the useCeil values for tflite

  // Convert avgpool2d to global pooling if the kernel size matches the HxW dimensions.
  // This happens in TIDL_tfOutReshapePoolingLayer but we need it prior to calling the model
  // checker for allow/deny.

  const TfLitePoolParams* Pool2DParams =
        reinterpret_cast<const TfLitePoolParams*>(node->builtin_data);
  uint32_t kernelW = Pool2DParams->filter_width;
  uint32_t kernelH = Pool2DParams->filter_height;
  tensor = &context->tensors[node->inputs->data[0]];
  int  dims = tensor->dims->size;
  if (dims != 4) return -1;
  //NHWC format
  uint32_t inH = tensor->dims->data[1];
  uint32_t inW = tensor->dims->data[2];
  if (kernelH == inH && kernelW == inW) //global avg pool
  {
    poolParams.poolingType = TIDL_AveragePooling;
    // Pool size is set to input layer size
    poolParams.kernelH = 0;
    poolParams.kernelW = 0;
    poolParams.padH    = 0;
    poolParams.padW    = 0;
    poolParams.padT    = 0;
    poolParams.padB    = 0;
    poolParams.padL    = 0;
    poolParams.padR    = 0;
    poolParams.strideH = 1;
    poolParams.strideW = 1;
  }
  else
  {
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfliteRtMapMaxPoolParams(registration, node, context, layer), "");
    poolParams.poolingType = TIDL_AveragePooling;
    // Convert 1x1 average pooling into max pooling (operations are identical)
    // mxnet_resnet50_v1d
    if (poolParams.kernelH == 1 && poolParams.kernelW == 1 &&
       ((poolParams.strideH == 2 && poolParams.strideW == 2) ||
        (poolParams.strideH == 1 && poolParams.strideW == 1)))
    {
      poolParams.poolingType = TIDL_MaxPooling;
    }
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapReluParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_ReLULayer;
  layer.actParams.actType = TIDL_RelU;
  return 0;
}

int32_t TIDL_tfliteRtMapPReluParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_ReLULayer;
  layer.actParams.actType = TIDL_PRelU;
  return 0;
}

int32_t TIDL_tfliteRtMapLeakyReluParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_LeakyReluLayer;
  layer.outData[0].elementType = tidl_getElementType(1);

  const TfLiteLeakyReluParams* LeakyReluParams = reinterpret_cast<const TfLiteLeakyReluParams*>(node->builtin_data);
  layer.layerPCParams.leakyReluParams.alpha = LeakyReluParams->alpha;
  return 0;
}

int32_t TIDL_tfliteRtMapSoftmaxParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SoftMaxLayer;

  TfLiteTensor* tensor;
  int32_t numDims;

  for (int i = 0; i < node->inputs->size; i++)
  {
    tensor = &context->tensors[node->inputs->data[i]];
    if(tensor->allocation_type == kTfLiteArenaRw)
    {
      numDims = tensor->dims->size;
      // NHWC input data format
      layer.inData[0].dimValues[TIDL_DIM_BATCH]  =  (numDims > 4) ? tensor->dims->data[numDims - 4] : 1;
      layer.inData[0].dimValues[TIDL_DIM_NUMCH]  = (numDims > 3) ? tensor->dims->data[numDims - 3] : 1;
      layer.inData[0].dimValues[TIDL_DIM_HEIGHT] = (numDims > 2) ? tensor->dims->data[numDims - 2] : 1;
      layer.inData[0].dimValues[TIDL_DIM_WIDTH]  = tensor->dims->data[numDims - 1];
    }
  }
  layer.layerParams.softMaxParams.axis = TIDL_DIM_WIDTH;
  layer.layerParams.softMaxParams.outTranspose = 0;
  return 0;
}

int32_t TIDL_tfliteRtMapAddParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  status = TIDL_checkAddMulSubDivTensorProperties(layer, BASE_OP_ADD);
  if(status == -1) return -1;

  const TfLiteAddParams* AddParams =
          reinterpret_cast<const TfLiteAddParams*>(node->builtin_data);

  TIDL_tfLiteRtFillActParams(layer.actParams,  AddParams->activation);

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t is1DConstTensor = TIDL_canConvertToBatchNorm (layer);
  if(md.numConstInputs > 0) //addition with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    TfLiteTensor* constTensor = &context->tensors[node->inputs->data[constTensorIdx]];
    int32_t varTensorIdx = md.varTensorIndices[0];

    sBuffer_t scale;
    sBuffer_t zero_point;
    if (is1DConstTensor == 1)
    {
      if(constTensor->dims->size == 0) //tensor dim size = 0 --- Add constant to layer
      {
        layer.layerType = TIDL_BatchNormLayer;
        layer.isConverted = 1;
        status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.bias, scale, zero_point);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
        {
          TIDL_tfliteRtDequantTensor(layer.bias, scale, zero_point, 1);
        }
        int32_t biasSize = md.varTensorsDims[0][1]; //broadcast the constant along number of channels

        TIDL_broadcastFloatTensor(layer.bias, biasSize);  //already dequantized tensor, so float
      }
      else //addition with constant vector
      {
        layer.layerType = TIDL_BatchNormLayer;
        layer.isConverted = 1;
        status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.bias, scale, zero_point);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
        {
          TIDL_tfliteRtDequantTensor(layer.bias, scale, zero_point, 1);
        }
      }

      layer.weights.ptr             = my_malloc(layer.bias.bufSize*sizeof(float));
      layer.weights.bufSize         = layer.bias.bufSize;
      float *ptr = (float *)layer.weights.ptr;
      for (int lc = 0; lc < layer.weights.bufSize; lc++)
        ptr[lc] = 1.0;
      return 0;
    }
    else
    {
      //Addition with const tensor
      layer.layerType = TIDL_EltWiseLayer;
      layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSum;
      layer.numInBufs = md.numInputs;
      status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.weights, layer.weightScales, layer.weightZeroPoints);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }
  else //both inputs variable
  {
    layer.layerType = TIDL_EltWiseLayer;
    layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSum;
    layer.numInBufs = md.numInputs;
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapSubParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  status = TIDL_checkAddMulSubDivTensorProperties(layer,BASE_OP_SUB);
  if(status == -1) return -1;
  const TfLiteSubParams* SubParams = reinterpret_cast<const TfLiteSubParams*>(node->builtin_data);
  TIDL_tfLiteRtFillActParams(layer.actParams,  SubParams->activation);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  if(md.numConstInputs > 0) //subtraction with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    TfLiteTensor* constTensor = &context->tensors[node->inputs->data[constTensorIdx]];
    int32_t varTensorIdx = md.varTensorIndices[0];

    sBuffer_t scale;
    sBuffer_t zero_point;
    if(constTensor->dims->size == 0) //tensor dim size = 0 --- Sub constant to layer
    {
      layer.layerType = TIDL_BatchNormLayer;
      layer.isConverted = 1;
      status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.bias, scale, zero_point);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
      {
        TIDL_tfliteRtDequantTensor(layer.bias, scale, zero_point, 1);
      }
      int32_t biasSize = md.varTensorsDims[0][1]; //broadcast the constant along number of channels
      TIDL_broadcastFloatTensor(layer.bias, biasSize);  //already dequantized tensor, so float
    }
    else //subtraction with constant vector
    {
      layer.layerType = TIDL_BatchNormLayer;
      layer.isConverted = 1;
      status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.bias, scale, zero_point);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
      {
        TIDL_tfliteRtDequantTensor(layer.bias, scale, zero_point, 1);
      }
    }
    layer.weights.ptr             = my_malloc(layer.bias.bufSize*sizeof(float));
    layer.weights.bufSize         = layer.bias.bufSize;
    float *ptr = (float *)layer.weights.ptr;
    /* case: const - X */
    if(md.constTensorIndices[0]  < md.varTensorIndices[0] )
    {
      for (int lc = 0; lc < layer.weights.bufSize; lc++)
        ptr[lc] = -1.0;
    }
    /* case: X - const */
    else
    {
      /* scaling bias with: -1 */
      float *biasPtr = (float *)layer.bias.ptr;
      for (int lc = 0; lc < layer.bias.bufSize; lc++)
        biasPtr[lc] = -1.0*biasPtr[lc];

      for (int lc = 0; lc < layer.weights.bufSize; lc++)
        ptr[lc] = 1.0;
    }
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapMulParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  status = TIDL_checkAddMulSubDivTensorProperties(layer, BASE_OP_MUL);
  if(status == -1) return -1;

  const TfLiteMulParams* MulParams =
          reinterpret_cast<const TfLiteMulParams*>(node->builtin_data);

  TIDL_tfLiteRtFillActParams(layer.actParams,  MulParams->activation);

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t is1DConstTensor = TIDL_canConvertToBatchNorm (layer);
  if(md.numConstInputs > 0) //addition with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    TfLiteTensor* constTensor = &context->tensors[node->inputs->data[constTensorIdx]];
    int32_t varTensorIdx = md.varTensorIndices[0];

    sBuffer_t scale;
    sBuffer_t zero_point;
    if (is1DConstTensor == 1)
    {
      if(constTensor->dims->size == 0) //tensor dim size = 0 --- Add constant to layer
      {
        layer.layerType = TIDL_BatchNormLayer;
        layer.isConverted = 1;
        status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.weights, scale, zero_point);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
        {
          TIDL_tfliteRtDequantTensor(layer.weights, scale, zero_point, 1);
        }
        int32_t wtSize = md.varTensorsDims[0][1]; //broadcast the constant along number of channels

        TIDL_broadcastFloatTensor(layer.weights, wtSize);  //already dequantized tensor, so float
      }
      else //addition with constant vector
      {
        layer.layerType = TIDL_BatchNormLayer;
        layer.isConverted = 1;
        status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.weights, scale, zero_point);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
        {
          TIDL_tfliteRtDequantTensor(layer.weights, scale, zero_point, 1);
        }
      }

      layer.bias.ptr             = my_malloc(layer.weights.bufSize*sizeof(float));
      layer.bias.bufSize         = layer.weights.bufSize;
      float *ptr = (float *)layer.bias.ptr;
      for (int lc = 0; lc < layer.bias.bufSize; lc++)
        ptr[lc] = 0.0;
      return 0;
    }
    else
    {
      layer.layerType = TIDL_EltWiseLayer;
      layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseProduct;
      layer.numInBufs = md.numInputs;
      status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.weights, layer.weightScales, layer.weightZeroPoints);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }
  else //both inputs variable
  {
    layer.layerType = TIDL_EltWiseLayer;
    layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseProduct;
    layer.numInBufs = md.numInputs;
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapDivParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  /** Do not map any parameters before checking the tensor properties, however, layer type needs to be mapped to TIDL layer type for cases like tflite OD with
   * detection post process layer where this layer is marked supported irrespective of allowlisting pass, but needs to be mapped to TIDL layer for
   * further network optimizations like converting to TIDL detection output layer
   */
  layer.layerType = TIDL_BatchNormLayer;
  layer.isConverted = 1;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  status = TIDL_checkAddMulSubDivTensorProperties(layer, BASE_OP_DIV);
  if(status == -1) return -1;
  const TfLiteDivParams* DivParams =
          reinterpret_cast<const TfLiteDivParams*>(node->builtin_data);
  TIDL_tfLiteRtFillActParams(layer.actParams,  DivParams->activation);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  sBuffer_t scale;
  sBuffer_t zero_point;
  if(md.numConstInputs > 0) //addition with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    TfLiteTensor* constTensor = &context->tensors[node->inputs->data[constTensorIdx]];
    int32_t varTensorIdx = md.varTensorIndices[0];
    layer.layerType = TIDL_BatchNormLayer;
    status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, constTensorIdx, layer.weights, scale, zero_point);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
      TIDL_tfliteRtDequantTensor(layer.weights, scale, zero_point, 1);
    }
    if(constTensor->dims->size == 0) //tensor dim size = 0 --- Add constant to layer
    {
      int32_t wtSize = md.varTensorsDims[0][1];  //broadcast the constant along number of channels

      TIDL_broadcastFloatTensor(layer.weights, wtSize);  //already dequantized tensor, so float
    }
    else //addition with constant vector
    {
            //Do nothing
    }
    float *wptr = (float *)layer.weights.ptr;
    for (int i = 0; i < layer.weights.bufSize; i++)
      wptr[i] = 1.0/ wptr[i];
    layer.bias.ptr             = my_malloc(layer.weights.bufSize*sizeof(float));
    layer.bias.bufSize         = layer.weights.bufSize;
    float *ptr = (float *)layer.bias.ptr;
    for (int lc = 0; lc < layer.bias.bufSize; lc++)
      ptr[lc] = 0.0;
    return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
  }
   return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapConcatParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_ConcatLayer;
  layer.numInBufs = node->inputs->size;
  sTIDL_ConcatParams_t &concatParams = layer.layerParams.concatParams;

  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  const TfLiteConcatenationParams* concat_params =
          reinterpret_cast<const TfLiteConcatenationParams*>(node->builtin_data);

  int32_t dims = context->tensors[node->inputs->data[0]].dims->size;

  if (dims != 4)
  {
    //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Concat input dims %d  layer.numInBufs %d ", dims, layer.numInBufs);
    TIDL_GLOBAL_REPORT_WARNING("Concat requires 4D input tensors - only %d dims are present, ignore if object detection network", dims);
    return -1;
  }

  int32_t axis = concat_params->axis;
  if (axis < 0)
    axis += dims;
  // NHWC --> NCHW
  axis = axis == 1 ? TIDL_DIM_HEIGHT :   // H
        axis == 2 ? TIDL_DIM_WIDTH :   // W
        axis == 3 ? TIDL_DIM_NUMCH :   // C
        axis;

  concatParams.axis = axis;

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapTransposeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;

  /** Do not map any parameters before checking the tensor properties, however, layer type needs to be mapped to TIDL layer type for cases like tflite OD with
   * detection post process layer where this layer is marked supported irrespective of allowlisting pass, but needs to be mapped to TIDL layer for
   * further network optimizations like converting to TIDL detection output layer
   */
  layer.layerType = TIDL_TransposeLayer;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  status = TIDL_checkTransposeTensorProperties(layer);
  if(status == -1) return -1;
  TfLiteTensor * tensor = &context->tensors[node->inputs->data[1]];
  int32_t numDims = tensor->dims->data[0];
  int32_t * src_permute = (int32_t *)tensor->data.i32;

  /**
   * Tensorflow transpose axis map to NHWC format, we need to adjust permute parameters in TIDL as we follow NCHW format
  */
  int32_t dimIdx;
  int32_t *tidl_weights = (int32_t*)malloc (sizeof(int32_t) * TIDL_DIM_MAX);

  int32_t ref_permute[] = { TIDL_DIM_NUMCH, TIDL_DIM_WIDTH, TIDL_DIM_HEIGHT, TIDL_DIM_DIM2, TIDL_DIM_DIM1, TIDL_DIM_BATCH};
  int32_t remDim = TIDL_DIM_MAX - numDims;

  for (int i = 0; i < remDim; i++)
  {
    tidl_weights[i] = i;
  }

  for (int i = 0; i < numDims; i++)
  {
    dimIdx = TIDL_find_index (src_permute, numDims - i - 1, numDims);
    tidl_weights[dimIdx + remDim] = ref_permute[i];
  }

  // change from nhwc to nchw format
  int32_t c,h,w;

  c = *(tidl_weights + TIDL_DIM_MAX - 1);
  w = *(tidl_weights + TIDL_DIM_MAX - 2);
  h = *(tidl_weights + TIDL_DIM_MAX - 3);

  *(tidl_weights + TIDL_DIM_MAX - 3) = c;
  *(tidl_weights + TIDL_DIM_MAX - 2) = h;
  *(tidl_weights + TIDL_DIM_MAX - 1) = w;

  layer.weights.ptr = tidl_weights;
  layer.weights.bufSize = TIDL_DIM_MAX;


  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapReshapeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;

  /** Do not map any parameters before checking the tensor properties, however, layer type needs to be mapped to TIDL layer type for cases like tflite OD with
   * detection post process layer where this layer is marked supported irrespective of allowlisting pass, but needs to be mapped to TIDL layer for
   * further network optimizations like converting to TIDL detection output layer
   */
  layer.layerType = TIDL_ReshapeLayer;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  status = TIDL_checkReshapeTensorProperties(layer);
  if(status == -1) return -1;
  TfLiteTensor * tensor = &context->tensors[node->inputs->data[1]];
  int32_t numDims = tensor->dims->data[0];
  layer.weights.ptr = my_malloc(numDims*sizeof(int32_t));
  layer.weights.bufSize = numDims;
  int32_t * shape = (int32_t *)layer.weights.ptr;
  int32_t * reshapeDims = (int32_t *)tensor->data.i32;

  for(int i = 0; i < numDims; i++)
  {
    shape[i] = reshapeDims[i];
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapMeanParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status;
  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;

  layer.layerType = TIDL_PoolingLayer;

  poolParams.poolingType = TIDL_AveragePooling;
  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;
  poolParams.kernelW = 0;
  poolParams.kernelH = 0;
  return 0;
}

int32_t TIDL_tfliteRtMapPadParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t idx,j;
  float * ptr;

  layer.layerType = TIDL_PadLayer;

  const TfLiteTensor* padInput;
  padInput = &context->tensors[node->inputs->data[1]]; // node->inputs->data[1] gives paddings (array of size [4,2])

  const TfLiteTensor* output;
  output = &context->tensors[node->outputs->data[0]]; // node->inputs->data[1] gives paddings (array of size [4,2])
  std::string outputName = output->name;

  if(padInput)
  {
    ptr = (float *)malloc(padInput->bytes);
    memcpy(ptr, (uint8_t *) padInput->data.raw, padInput->bytes);
  }
  if(padInput->bytes / 4 == 8)
  {
    if((((int32_t *)ptr)[0] != 0) || (((int32_t *)ptr)[1] != 0) || (((int32_t *)ptr)[6] != 0) || (((int32_t *)ptr)[7] != 0))
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Padding is only supported in width/height axis - layer name - %s", outputName);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Pad layer is expected to provide 8 pad values, found %d - layer name - %s", (padInput->bytes / 4), outputName);
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  int32_t padT, padB, padL, padR;
  padT = ((int32_t *)ptr)[2];
  padB = ((int32_t *)ptr)[3];
  padL = ((int32_t *)ptr)[4];
  padR = ((int32_t *)ptr)[5];
  layer.layerParams.padLayerParams.padT = padT;
  layer.layerParams.padLayerParams.padB = padB;
  layer.layerParams.padLayerParams.padL = padL;
  layer.layerParams.padLayerParams.padR = padR;

  my_free(ptr);
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapFullyConnectedParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  status = TIDL_checkInnerProductTensorProperties(layer);
  if(status == -1) return -1;

  sTIDL_InnerProductParams_t &innerProductParams = layer.layerParams.innerProductParams;
  
  /*Default:*/
  innerProductParams.numBChannels = 1;

  const TfLiteFullyConnectedParams* fc_params =
          reinterpret_cast<const TfLiteFullyConnectedParams*>(node->builtin_data);
  TIDL_tfLiteRtFillActParams(layer.actParams,  fc_params->activation);

  layer.layerType = TIDL_InnerProductLayer;
  layer.outData[0].elementType = tidl_getElementType(1);

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  if (layer.allowlistingMetaData.numConstInputs != 0)
  {
    layer.allowlistingMetaData.numConstInputs = 0;
  }

  if((md.constTensorIndices.size() == 1) || (md.constTensorIndices.size() == 2)) //filter, bias
  {
    int32_t filterIdx = md.constTensorIndices[0]; //first constant tensor should be filter
    innerProductParams.constIdx = -1;
    innerProductParams.numInRows  = 1;
    innerProductParams.isBias     = 1;
    innerProductParams.numOutCols = md.constTensorsDims[0][0];
    innerProductParams.numInCols  = md.constTensorsDims[0][1];
    innerProductParams.inputATranspose = 0;
    innerProductParams.inputBTranspose = 1;

    layer.inData[0].dimValues[TIDL_DIM_BATCH]  = 1;
    layer.inData[0].dimValues[TIDL_DIM_NUMCH]  = 1;
    layer.inData[0].dimValues[TIDL_DIM_HEIGHT] = 1;
    layer.inData[0].dimValues[TIDL_DIM_WIDTH]  = innerProductParams.numInCols;
    layer.outData[0].dimValues[TIDL_DIM_WIDTH] = innerProductParams.numOutCols;

    status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, filterIdx, layer.weights, layer.weightScales, layer.weightZeroPoints);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
      TIDL_tfliteRtDequantTensor(layer.weights, layer.weightScales, layer.weightZeroPoints, 1);
    }

    if(md.constTensorIndices.size() == 2) //both weight and bias present
    {
      int32_t biasIdx = md.constTensorIndices[1]; //2nd constant tensor should be bias
      status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, biasIdx, layer.bias, layer.biasScales, layer.biasZeroPoints);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read constant tensor : Only float, int32, uint8 and int8 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
      {
        TIDL_tfliteRtDequantTensor(layer.bias, layer.biasScales, layer.biasZeroPoints, 1);
      }
    }
    else  //bias tensor not specified, should default to 0
    {
      if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
      {
        layer.bias.ptr = calloc(innerProductParams.numOutCols, sizeof(float));
      }
      else
      {
        layer.bias.ptr = calloc(innerProductParams.numOutCols, sizeof(int8_t));
      }
      layer.bias.bufSize = innerProductParams.numOutCols;
    }
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapDeConvParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
  int32_t padType;

  /**************** Tensor property checks ****************/
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);

  sTIDL_ConvParams_t &convParams      = layer.layerParams.convParams;
  layer.layerType = TIDL_Deconv2DLayer;
  layer.outData[0].elementType = tidl_getElementType(1);

  TfLiteTensor* output;
  output = &context->tensors[node->outputs->data[0]];
  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[0]];
  const TfLiteTensor* filter;
  filter = &context->tensors[node->inputs->data[1]];

  // OHWI layout for tflite conv filter
  convParams.numInChannels   = filter->dims->data[3];
  convParams.numOutChannels  = filter->dims->data[0];
  convParams.kernelW         = filter->dims->data[2];
  convParams.kernelH         = filter->dims->data[1];

  convParams.numGroups       = 1;
  convParams.dilationW       = 1;
  convParams.dilationH       = 1;
  convParams.strideW         = 1;
  convParams.strideH         = 1;
  convParams.padW            = 0;
  convParams.padH            = 0;
  convParams.enableBias      = 0;
  convParams.enablePooling   = 0;

  const TfLiteTransposeConvParams* conv_params =
          reinterpret_cast<const TfLiteTransposeConvParams*>(node->builtin_data);

  convParams.strideW = conv_params->stride_width;
  convParams.strideH = conv_params->stride_height;

  padType = conv_params->padding;
  layer.strideOffsetMethod = TIDL_StrideOffsetCenter;

  if (padType == kTfLitePaddingSame)   // SAME : Padding done to input
  {
    convParams.padW = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    convParams.padH = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
  }
  else if (padType == kTfLitePaddingValid)  // VALID : No padding to input
  {
    layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    convParams.padL            = 0;
    convParams.padR            = 0;
    convParams.padT            = 0;
    convParams.padB            = 0;
  }

  status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, 1, layer.weights, layer.weightScales, layer.weightZeroPoints);
  if(status == -1) return -1;
  if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
  {
    TIDL_tfliteRtKernelReshape((float *)layer.weights.ptr, convParams.kernelW, convParams.kernelH, convParams.numInChannels, convParams.numOutChannels);
    TIDL_tfliteRtDequantTensor(layer.weights, layer.weightScales, layer.weightZeroPoints, convParams.kernelW * convParams.kernelH * convParams.numInChannels);
  }
  else
  {
    TIDL_tfliteRtKernelReshape((int8_t *)layer.weights.ptr, convParams.kernelW, convParams.kernelH, convParams.numInChannels, convParams.numOutChannels);
  }

  if (node->inputs->size == 4)
  {
    status = TIDL_tfliteRtCopyInputConstTensor(registration, node, context, 3, layer.bias, layer.biasScales, layer.biasZeroPoints);
    if(status == -1) return -1;
    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
      TIDL_tfliteRtDequantTensor(layer.bias, layer.biasScales, layer.biasZeroPoints, 1);
    }
    convParams.enableBias = 1;
  }

  int32_t output_dims = input->dims->data[0];
  int32_t *output_data = (int32_t*)input->data.i32;

  if (output_dims == 4)
  {
    layer.outData[0].dimValues[TIDL_DIM_HEIGHT] = -output_data[1];
    layer.outData[0].dimValues[TIDL_DIM_WIDTH]  = -output_data[2];
  }

  return status;
}

int32_t TIDL_tfliteRtMapStridedSliceParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;

  /**************** Tensor property checks ****************/
  TIDL_tfliteRtSaveAllowlistingMetaData(context, node, layer);
  status = TIDL_checkSliceTensorProperties(layer);
  if(status == -1) return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;

  sTIDL_SliceLayerParams_t &sliceParams = layer.layerParams.sliceParams;

  const TfLiteStridedSliceParams* strided_slice_params =
          reinterpret_cast<const TfLiteStridedSliceParams*>(node->builtin_data);

  /**************** Map attributes ****************/

  int32_t beginMask = strided_slice_params->begin_mask;
  int32_t endMask = strided_slice_params->end_mask;
  int32_t ellipsisMask = strided_slice_params->ellipsis_mask;
  int32_t newAxisMask = strided_slice_params->new_axis_mask;
  int32_t shrinkAxisMask = strided_slice_params->shrink_axis_mask;

  /****************  Basic attribute checks **************/

  if((ellipsisMask != 0) || (newAxisMask != 0) || (shrinkAxisMask != 0))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice : ellipsis_mask, new_axis_mask and shrink_axis_mask must be 0");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if((beginMask < 0) || (beginMask > 15) || (endMask < 0) || (endMask > 15)) /* since only 4 input dims are supported, mask can be only 4 bit - this is a sanity check */
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice : begin_mask/end_mask not in supported range - [0,15]");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /***************** Map inputs/parameters *****************/

  const TfLiteTensor* tensorBegin = &context->tensors[node->inputs->data[1]];
  const TfLiteTensor* tensorEnd = &context->tensors[node->inputs->data[2]];
  const TfLiteTensor* tensorStrides = &context->tensors[node->inputs->data[3]];

  int32_t * dataBegin = (int32_t *)malloc(tensorBegin->bytes);
  memcpy(dataBegin, (uint8_t *) tensorBegin->data.raw, tensorBegin->bytes);
  int32_t * dataEnd = (int32_t *)malloc(tensorEnd->bytes);
  memcpy(dataEnd, (uint8_t *) tensorEnd->data.raw, tensorEnd->bytes);
  int32_t * dataStrides = (int32_t *)malloc(tensorStrides->bytes);
  memcpy(dataStrides, (uint8_t *) tensorStrides->data.raw, tensorStrides->bytes);


  /**************** Mapped inputs/parameters related checks *************/
  for(int i = 0; i < 4; i++)
  {
    if(dataStrides[i] != 1)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice : only stride 1 slice is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  /****************** Derived parameters *****************/

  /* Convert beginMask and endMask to binary bits */
  std::vector<int> beginMaskBits, endMaskBits;
  for(int i = 0; i < 4; i++) /* beginMask = 7 corresponds to 1110 in bits, 4 corresponds to 0010 - bits are reversed as per tflite interpretation */
  {
    beginMaskBits.push_back(beginMask % 2);
    beginMask /= 2;
    endMaskBits.push_back(endMask % 2);
    endMask /= 2;
  }

  const TfLiteTensor* outTensor = &context->tensors[node->outputs->data[0]];
  const TfLiteTensor* inTensor = &context->tensors[node->inputs->data[0]];
  std::vector<int32_t> outTensorDims, inTensorDims;
  for(int i = 0; i < 4; i++)  /* TIDL_checkSliceTensorProperties ensures both input_0 and output_0 have 4 dimensions */
  {
    outTensorDims.push_back(outTensor->dims->data[i]);
    inTensorDims.push_back(inTensor->dims->data[i]);
  }

  std::vector<int32_t> axes;
  for(int i = 1; i < 4; i++) /* Only batch size = 1 supported as checked in TIDL_checkSliceTensorProperties. Batch slicing not supported and batch bits not checked in tflite runtime implementation */
  {
    if(beginMaskBits[i] == 1) /* begin ignored for index i, set begin to 0 : no slicing at beginning */
    {
      dataBegin[i] = 0;
    }
    if(endMaskBits[i] == 1) /* end ignored for index, set end same as the corresponding input dimension : no slicing at end */
    {
      dataEnd[i] = inTensorDims[i];
    }
    if((dataBegin[i] != 0) || (dataEnd[i] != inTensorDims[i])) /* implies slicing happens in this axis */
    {
      axes.push_back(i);
    }
  }

  /****************** Check derived parameters *****************/
  if(axes.size() != 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice : Slicing is supported only across either of height, width or channel axis, not across multiple axes");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /*************** Mapping of attributes and inputs/parameters to TIDL layer *************/
  int32_t NhwcAxis = axes[0];
  int32_t NchwAxis;
  NchwAxis = NhwcAxis == 1 ? TIDL_DIM_HEIGHT :   // H
            NhwcAxis == 2 ? TIDL_DIM_WIDTH :   // W
            NhwcAxis == 3 ? TIDL_DIM_NUMCH :   // C
            NhwcAxis;

  layer.layerParams.sliceParams.axis = NchwAxis;
  layer.layerParams.sliceParams.stride = 1;
  layer.layerParams.sliceParams.slicePoints[0] = dataBegin[NhwcAxis];
  layer.layerParams.sliceParams.slicePoints[1] = dataEnd[NhwcAxis];

  layer.layerType = TIDL_SliceLayer;
  layer.numInBufs = 1;
  layer.numOutBufs = 1;

  if(dataBegin) free(dataBegin);
  if(dataEnd) free(dataEnd);
  if(dataStrides) free(dataStrides);

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapBiInterPResizeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  sTIDL_ResizeLayerParams_t& params = layer.layerParams.resizeParams;
  layer.layerType = TIDL_ResizeLayer;

  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[0]];
  const TfLiteTensor* resizeShape;
  resizeShape = &context->tensors[node->inputs->data[1]];

  int32_t * newSize = reinterpret_cast<int32_t *>(resizeShape->data.raw);
  int new_height = newSize[0];
  int new_width  = newSize[1];


  int orig_height = input->dims->data[1];
  int orig_width  = input->dims->data[2];

  /* initialize resizeRatio by 1, later will be changed acc to scales given */
  for(int32_t i = 0; i < TIDL_DIM_MAX; i++)
  {
    params.resizeRatio[i] = 1.0;
  }

  params.resizeRatio[TIDL_DIM_HEIGHT] = (new_height * 1.0) / orig_height;
  params.resizeRatio[TIDL_DIM_WIDTH]  = (new_width  * 1.0) / orig_width;

  layer.layerParams.resizeParams.mode = TIDL_ResizeBilinear;

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_tfliteRtMapNNResizeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfliteRtMapBiInterPResizeParams(registration, node, context, layer), "");
  layer.layerParams.resizeParams.mode = TIDL_ResizeNearest;
  return 0;
}

int32_t TIDL_tfliteRtMapArgmaxParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  int32_t status = -1;

  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[0]];
  const TfLiteTensor* argMaxDim;
  argMaxDim = &context->tensors[node->inputs->data[1]];

  int32_t * argMaxDimData = reinterpret_cast<int32_t *>(argMaxDim->data.raw);

  int32_t dims = input->dims->size;

  int32_t axis = argMaxDimData[0];
  if (axis < 0)
    axis += dims;
  // NHWC --> NCHW
  axis = axis == 1 ? TIDL_DIM_HEIGHT :   // H
        axis == 2 ? TIDL_DIM_WIDTH :   // W
        axis == 3 ? TIDL_DIM_NUMCH :   // C
        axis;

  layer.layerType = TIDL_ArgOpLayer;
  layer.layerParams.argOpParams.argOpType = TIDL_ArgMax;
  // TIDL supports argmax only on axis 1.
  // Check attributes here because tidlModelCheckLayer() doesn't have access to them

  if (axis == TIDL_DIM_NUMCH)
  {
    status = 0;  // No error - this op is supported
  }

  // Return -1 if there are unsupported attributes

  return status;
}

int32_t TIDL_tfliteRtMapSigmoidParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SigmoidLayer;
  layer.actParams.actType = TIDL_Sigmoid;
  return 0;
}

int32_t TIDL_tfliteRtMapTanhParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_TanhLayer;
  layer.actParams.actType = TIDL_Tanh;
  return 0;
}

int32_t TIDL_tfliteRtMapHardSigmoidParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_HardSigmoidLayer;
  layer.actParams.actType = TIDL_HardSigmoid;
  return 0;
}

int32_t TIDL_tfliteRtMapELUParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_ELULayer;
  layer.actParams.actType = TIDL_ELU;
  return 0;
}

int32_t TIDL_tfliteRtMapBatchToSpaceParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_BatchToSpaceLayer;

  //fill block size
  const TfLiteTensor* tensor_blockSize;
  tensor_blockSize = &context->tensors[node->inputs->data[1]];
  auto * blockSizeData = reinterpret_cast<int32_t *>(tensor_blockSize->data.raw);
  int32_t * ptrBlockSize = (int32_t *)malloc(tensor_blockSize->bytes);
  memcpy(ptrBlockSize, blockSizeData, tensor_blockSize->bytes);
  layer.layerPCParams.batchToSpaceParams.blockHeight = ptrBlockSize[0];
  layer.layerPCParams.batchToSpaceParams.blockWidth = ptrBlockSize[1];

  //fill cropping dimensions
  const TfLiteTensor* tensor_crop;
  tensor_crop = &context->tensors[node->inputs->data[2]];
  auto * croppingData = reinterpret_cast<int32_t *>(tensor_crop->data.raw);
  int32_t * ptrCropping = (int32_t *)malloc(tensor_crop->bytes);
  memcpy(ptrCropping, croppingData, tensor_crop->bytes);
  layer.layerPCParams.batchToSpaceParams.cropT = ptrCropping[0];
  layer.layerPCParams.batchToSpaceParams.cropB = ptrCropping[1];
  layer.layerPCParams.batchToSpaceParams.cropL = ptrCropping[2];
  layer.layerPCParams.batchToSpaceParams.cropR = ptrCropping[3];

  free(ptrCropping);
  free(ptrBlockSize);
  return 0;
}

int32_t TIDL_tfliteRtMapSpaceToBatchParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SpaceToBatchLayer;

  // fill block shape
  const TfLiteTensor* tensor_blockSize;
  tensor_blockSize = &context->tensors[node->inputs->data[1]];
  auto * blockSizeData = reinterpret_cast<int32_t *>(tensor_blockSize->data.raw);
  int32_t * ptrBlockSize = (int32_t *)malloc(tensor_blockSize->bytes);
  memcpy(ptrBlockSize, blockSizeData, tensor_blockSize->bytes);
  layer.layerPCParams.spaceToBatchParams.blockHeight = ptrBlockSize[0];
  layer.layerPCParams.spaceToBatchParams.blockWidth = ptrBlockSize[1];

  //fill padding
  const TfLiteTensor* tensor_padding;
  tensor_padding = &context->tensors[node->inputs->data[2]];
  auto * padding = reinterpret_cast<int32_t *>(tensor_padding->data.raw);
  int32_t * ptrPadding = (int32_t *)malloc(tensor_padding->bytes);
  memcpy(ptrPadding, padding, tensor_padding->bytes);
  layer.layerPCParams.spaceToBatchParams.padT = ptrPadding[0];
  layer.layerPCParams.spaceToBatchParams.padB = ptrPadding[1];
  layer.layerPCParams.spaceToBatchParams.padL = ptrPadding[2];
  layer.layerPCParams.spaceToBatchParams.padR = ptrPadding[3];

  free(ptrBlockSize);
  free(ptrPadding);
  return 0;
}

int32_t TIDL_tfliteRtMapSqueezeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SqueezeLayer;
  const TfLiteSqueezeParams* squeeze_params =
          reinterpret_cast<const TfLiteSqueezeParams*>(node->builtin_data);
  int32_t numDims = squeeze_params->num_squeeze_dims;
  int32_t * squeezeDims = (int32_t *)squeeze_params->squeeze_dims;

  const TfLiteTensor* input;
  input = &context->tensors[node->inputs->data[0]];

  for (int32_t  j = 0; j < numDims; j++)
  {
    if((squeezeDims[j] >= 0)  && (squeezeDims[j] < TIDL_DIM_MAX)  && (input->dims->data[squeezeDims[j]] == 1))
    {
      layer.layerPCParams.squeezeParams.axis[squeezeDims[j]] = 1;
    }
  }
  int32_t c,h,w;
  c = layer.layerPCParams.squeezeParams.axis[3];
  h = layer.layerPCParams.squeezeParams.axis[1];
  w = layer.layerPCParams.squeezeParams.axis[2];
  layer.layerPCParams.squeezeParams.axis[TIDL_DIM_NUMCH]  = c;
  layer.layerPCParams.squeezeParams.axis[TIDL_DIM_HEIGHT] = h;
  layer.layerPCParams.squeezeParams.axis[TIDL_DIM_WIDTH]  = w;

  return 0;
}

int32_t TIDL_tfliteRtMapPackParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_PackLayer;
  const TfLitePackParams* pack_params =
          reinterpret_cast<const TfLitePackParams*>(node->builtin_data);
  layer.layerPCParams.packParams.axis = pack_params->axis;
  layer.layerPCParams.packParams.valuesCount = pack_params->values_count;
  return 0;
}

int32_t TIDL_tfliteRtMapCastParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_CastLayer;
  /*Defaulting to a supported variant*/
  layer.layerPCParams.castParams.terminal = TIDL_CastNotTerminal;
  return 0;
}

int32_t TIDL_tfliteRtMapQuantizeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_DataConvertLayer;
  layer.layerParams.dataConvertParams.type = TIDL_DC_TYPE_INTERMEDIATE;

  /*TODO: check for any other case.Assuming quantisation supports only 1 input and 1 output*/
  auto input_tensor = &context->tensors[node->inputs->data[0]];
  auto output_tensor = &context->tensors[node->outputs->data[0]];
  layer.layerParams.dataConvertParams.outZeroPoint = output_tensor->params.zero_point;
  layer.layerParams.dataConvertParams.inZeroPoint = input_tensor->params.zero_point;
  return 0;
}

int32_t TIDL_tfliteRtMapDequantizeParams(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer)
{
  auto input_tensor = &context->tensors[node->inputs->data[0]];
  auto output_tensor = &context->tensors[node->outputs->data[0]];
  /* Only map to dataConvert if it layer is from uint8/int8 -> float*/
  //TODO:Figure out the condition to switch between the identity and data convert
  if (((input_tensor->type == kTfLiteUInt8) || (input_tensor->type == kTfLiteInt8)) && ((output_tensor->type == kTfLiteFloat32)))
  {
    layer.layerType = TIDL_DataConvertLayer;
    layer.layerParams.dataConvertParams.type = TIDL_DC_TYPE_OUTPUT;
    layer.outData[0].elementType = TIDL_SinglePrecFloat;
    layer.outData[0].tensorScale = 1;
    /*TODO: check for any other case.Assuming dequantisation supports only 1 input and 1 output*/
    TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)output_tensor->quantization.params;
    layer.layerParams.dataConvertParams.outZeroPoint = output_tensor->params.zero_point;
    quantization = (TfLiteAffineQuantization*)input_tensor->quantization.params;
    layer.layerParams.dataConvertParams.inZeroPoint = quantization->zero_point->data[0];
    return 0;
  }

  layer.layerType = TIDL_DequantizeLayer;
  auto tensor = &context->tensors[node->inputs->data[0]];
  layer.layerPCParams.dequantParams.scale = 0.1;
  layer.layerPCParams.dequantParams.zeroPoint = 0.0;

  if (((tensor->type == kTfLiteUInt8) || (tensor->type == kTfLiteInt8)) && (gParams.addDataConvertToNet))
  {
    TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)tensor->quantization.params;
    if(quantization)
    {
        auto * scale = quantization->scale;
        auto *zero_point = quantization->zero_point;
        if (scale && zero_point)
        {
          layer.layerPCParams.dequantParams.scale     = scale->data[0];
          layer.layerPCParams.dequantParams.zeroPoint = zero_point->data[0];
        }
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CREATE_METAARCH
/* This function creates meta architecture file using information from Tflite detection post process layer which can be used for compilation of OD networks */
int32_t TIDL_createMetaarchFile(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context,
    sTIDL_odParameters_t * odUserParams,int numTFMetaLayers,vector<int> numAspectRatios,std::vector<string> OD_heads,int height,int width,int reduce_boxes_in_lowest_layer
    ,int score_converter,string pathForMetaFile)
{
  const flexbuffers::Map& m = flexbuffers::GetRoot(reinterpret_cast<const uint8_t*>(node->custom_initial_data), node->custom_initial_data_size).AsMap();
  int32_t max_detections = m["max_detections"].AsInt32();
  int32_t max_classes_per_detection = m["max_classes_per_detection"].AsInt32();
  int32_t detections_per_class;
  if (m["detections_per_class"].IsNull())
    detections_per_class = 100;
  else
    detections_per_class = m["detections_per_class"].AsInt32();
  if(detections_per_class == 0)
  {
    detections_per_class = 100;
  }
  bool use_regular_non_max_suppression;
  if (m["use_regular_nms"].IsNull())
    use_regular_non_max_suppression = false;
  else
    use_regular_non_max_suppression = m["use_regular_nms"].AsBool();

  float nms_score_threshold = m["nms_score_threshold"].AsFloat();
  float iou_threshold = m["nms_iou_threshold"].AsFloat();

  int32_t num_classes = m["num_classes"].AsInt32();
  float y_scale = m["y_scale"].AsFloat();
  float x_scale = m["x_scale"].AsFloat();
  float h_scale = m["h_scale"].AsFloat();
  float w_scale = m["w_scale"].AsFloat();

  if(odUserParams->confidence_threshold != -1) //Not default, provided by user
  {
    nms_score_threshold = odUserParams->confidence_threshold;
  }
  if(odUserParams->nms_threshold != -1)
  {
    iou_threshold = odUserParams->nms_threshold;
  }
  if(odUserParams->top_k != -1)
  {
    detections_per_class = odUserParams->top_k;
  }
  if(odUserParams->keep_top_k != -1)
  {
    if(odUserParams->keep_top_k > max_detections)
    {
      TIDL_GLOBAL_REPORT_WARNING("keep_top_k(%d) cannot be specified to be greater than 'max_detections'(%d) parameters in the tflite detection post processing layer. Setting it to max_detections = %d\n", odUserParams->keep_top_k, max_detections, max_detections);
    }
    else
    {
      max_detections = odUserParams->keep_top_k;
    }
  }

  protos::TrainEvalPipelineConfig main;         //Main Class/Parent Class
  protos::DetectionModel detectionmodel;       //types-->kFasterRcnn = 1,kSsd = 2,MODEL_NOT_SET = 0,
  main.set_allocated_model(&detectionmodel);
  protos::Ssd param;
  detectionmodel.set_allocated_ssd(&param);
  param.set_num_classes(num_classes);
  param.set_encode_background_as_zeros(true);
  for(int i = 0;i < OD_heads.size();i++)
  {
    if(i%2 == 0)
    {
      param.add_box_input(OD_heads[i]);
    }
    else{
      param.add_class_input(OD_heads[i]);
    }
  }
  //Image Resizer
  protos::ImageResizer imageresizer;
  protos::FixedShapeResizer fixedshaperesizer;
  param.set_allocated_image_resizer(&imageresizer);
  imageresizer.set_allocated_fixed_shape_resizer(&fixedshaperesizer);
  if(imageresizer.has_fixed_shape_resizer())
  {
    fixedshaperesizer.set_height(height);
    fixedshaperesizer.set_width(width);
  }
  //BoxCoder
  protos::BoxCoder boxcoder;
  param.set_allocated_box_coder(&boxcoder);
  protos::FasterRcnnBoxCoder fasterrcnnboxcoder;
  boxcoder.set_allocated_faster_rcnn_box_coder(&fasterrcnnboxcoder);
  if(boxcoder.has_faster_rcnn_box_coder())
  {
    fasterrcnnboxcoder.set_x_scale(x_scale);
    fasterrcnnboxcoder.set_y_scale(y_scale);
    fasterrcnnboxcoder.set_height_scale(h_scale);
    fasterrcnnboxcoder.set_width_scale(w_scale);
  }
  //Anchor Generator
  protos::AnchorGenerator anchorgenerator;
  param.set_allocated_anchor_generator(&anchorgenerator);
  protos::SsdAnchorGenerator ssdanchorgenerator;
  anchorgenerator.set_allocated_ssd_anchor_generator(&ssdanchorgenerator);
  if(anchorgenerator.has_ssd_anchor_generator())
  {
    ssdanchorgenerator.set_num_layers(OD_heads.size()/2);
    ssdanchorgenerator.set_min_scale(0.20000000298);       //neeed to remove hard coded
    ssdanchorgenerator.set_max_scale(0.949999988079);
    ssdanchorgenerator.set_reduce_boxes_in_lowest_layer(reduce_boxes_in_lowest_layer);
    // ssdanchorgenerator.aspect_ratios_size();
    // for(int i=0;i<5;i++)
    // {
    //   ssdanchorgenerator.add_aspect_ratios(i);  //array[i];
    // }
    ssdanchorgenerator.add_aspect_ratios(1.0);                  //neeed to remove hard coded
    ssdanchorgenerator.add_aspect_ratios(2.0);
    ssdanchorgenerator.add_aspect_ratios(0.5);
    ssdanchorgenerator.add_aspect_ratios(3.0);
    ssdanchorgenerator.add_aspect_ratios(0.333299994469);
  }
  //PostProcessing
  protos::PostProcessing postprocessing;
  param.set_allocated_post_processing(&postprocessing);
  protos::BatchNonMaxSuppression batchnonmaxsuppresion;
  protos::KeypointBoxCoder keypointboxcoder;
  postprocessing.set_allocated_batch_non_max_suppression(&batchnonmaxsuppresion);
  if(score_converter == 1)     //Types-->PostProcessing_ScoreConverter_IDENTITY = 0,PostProcessing_ScoreConverter_SIGMOID = 1, --> logistic,PostProcessing_ScoreConverter_SOFTMAX = 2 -->softmax
  {
    postprocessing.set_score_converter(PostProcessing_ScoreConverter_SIGMOID);
  }
  else
  {
    postprocessing.set_score_converter(PostProcessing_ScoreConverter_SOFTMAX);
  }
  if(postprocessing.has_batch_non_max_suppression())
  {
    batchnonmaxsuppresion.set_score_threshold(nms_score_threshold);
    batchnonmaxsuppresion.set_iou_threshold(iou_threshold);
    batchnonmaxsuppresion.set_max_detections_per_class(detections_per_class); //need to check()
    batchnonmaxsuppresion.set_max_total_detections(max_detections);
  }

  std::ofstream ofs;
  ofs.open (pathForMetaFile, std::ofstream::out);
  ofs << main.Utf8DebugString();                    //writing to file
  //Releasing all classes
  imageresizer.release_fixed_shape_resizer();
  param.release_image_resizer();
  boxcoder.release_faster_rcnn_box_coder();
  param.release_box_coder();
  anchorgenerator.release_ssd_anchor_generator();
  param.release_anchor_generator();
  postprocessing.release_batch_non_max_suppression();
  param.release_post_processing();
  detectionmodel.release_ssd();
  main.release_model();
  ofs.close();
}
#endif

/** Derive number of convolution heads from anchor boxes*/
int32_t TIDL_deriveNumberOfConvHeadsFromAnchorBoxes(const TfLiteTensor * anchor_boxes)
{
    auto * boxPtr = reinterpret_cast<float *>(anchor_boxes->data.raw);
    float * anchorBoxInput = (float *)malloc(anchor_boxes->bytes);
    if((boxPtr == NULL) || (anchorBoxInput == NULL))
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot retrieve number of convolution heads, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    memcpy(anchorBoxInput, boxPtr, anchor_boxes->bytes);

    float  xCoordinate = anchorBoxInput[0];
    float yCoordinate =  anchorBoxInput[1];
    float xCurr = xCoordinate, yCurr = yCoordinate;
    float xPrev, yPrev;
    int numConvHeads = 0, cnt = 0;
    float epsilon = 0.0001; /* values may not be exact and there can be minor differences in prev and curr floating point values, so keep margin */
    while(cnt < anchor_boxes->bytes/4)
    {
        xPrev = xCurr;
        yPrev = yCurr;
        xCurr = anchorBoxInput[cnt + 0];
        yCurr = anchorBoxInput[cnt + 1];

        if((xCurr + epsilon) < xPrev && (yCurr + epsilon) < yPrev)
        {
            numConvHeads++;
        }
        cnt = cnt + 4;
    }
    if(anchorBoxInput) free(anchorBoxInput);
    return numConvHeads + 1; /* Last head will not be accounted for based on current logic*/
}

//To get a vector of inputs of previous layer: if it matches with present layer and inputs are matching  heads are retrieved
std::vector<string> TIDL_findLayerAndReturnInputsGivenOutputs(TfLiteContext* context,TfLiteIntArray* plan, int tfliteLayerType,std::vector<string> outputNames)
{
    TfLiteNode* node;
    TfLiteRegistration* registration;
    std::vector<string> heads = {};
    for(int node1 : TfLiteIntArrayView(plan))
    {
        context->GetNodeAndRegistration(context, node1, &node, &registration);
        TfLiteTensor* tensor1 = &context->tensors[node->outputs->data[0]];
        if(registration->builtin_code == tfliteLayerType)
        {
            if(tensor1->allocation_type == kTfLiteArenaRw)
            {
                for(int i=0;i<outputNames.size();i++)
                {
                    if(outputNames[i] == tensor1->name)
                    {
                        for(int k=0;k<node->inputs->size;k++)
                        {
                            TfLiteTensor* tensor2 = &context->tensors[node->inputs->data[k]];
                            if(tensor2->allocation_type == kTfLiteArenaRw)
                            {
                                heads.push_back(tensor2->name);
                            }
                        }
                    }
                }
            }
        }
    }
    return heads;
}

/* Returns input node index for given input tensor for a particular node */
int32_t TIDL_getInputNodeIdx(TfLiteContext* context,TfLiteIntArray* plan, TfLiteTensor * inTensor)
{
    int32_t index = -1;
    TfLiteNode* node;
    TfLiteRegistration* registration;
    for(int node_index : TfLiteIntArrayView(plan))
    {
        context->GetNodeAndRegistration(context, node_index, &node, &registration);
        for(int i = 0; i < node->outputs->size; i++)
        {
            TfLiteTensor* tensor = &context->tensors[node->outputs->data[i]];
            if(inTensor->name == tensor->name)
            {
                return node_index;
            }
        }
    }
    return index;
}

/** Check our assumption of SSD post processing structure are met and return true/false based on if Tflite SSD processing is supported
 * Secondary aim : To derive OD head names and populate few more OD specific parameter if SSD structure pattern assumptions are met
 * {BOX CONV                             CLASS CONV}     -- Same number on both sides
 * {BOX RESHAPE                          CLASS RESHAPE}   -- Same number on both sides
 * {CONCATENATION                        CONCATENATION(MUST BE PRESENT) / DIV_BY_1 (Additional known combination - supported if present)}
 * {RESHAPE(MAY BE PRESENT OR ABSENT),   LOGISTIC OR SOFTMAX}
 * {TFLite_Detection_PostProcess Layer}
*/
bool TIDL_isSsdOdPostProcStructureSupported(TfLiteContext* context,TfLiteIntArray* plan, std::vector<string> &odHeads, TIDL_osrtOptions * options,
                                            std::vector<int> &numAspectRatios,int &reduce_boxes_in_lowest_layer,int &score_convertor)
{
    TfLiteNode* node;
    TfLiteRegistration* registration;
    int32_t num_classes;
    int32_t noOfdetectionOutputs;

    /* Get inputs of detection layer */
    vector<string> inputsofCustom;
    int32_t numDetectionLayers = 0;
    int32_t detectionNodeIdx;
    for(int node_index : TfLiteIntArrayView(plan))
    {
        context->GetNodeAndRegistration(context, node_index, &node, &registration);
        if(registration->builtin_code == kTfLiteBuiltinCustom)
        {
            numDetectionLayers++;
            detectionNodeIdx = node_index;
            for(int i=0;i<node->inputs->size;i++)
            {
                TfLiteTensor* tensor = &context->tensors[node->inputs->data[i]];
                if(tensor->allocation_type == kTfLiteArenaRw)
                {
                    inputsofCustom.push_back(tensor->name);
                }
            }
            noOfdetectionOutputs = node->outputs->size;
        }
    }

    /* Basic checks on detection layer assumptions */
    if(numDetectionLayers > 1)
    {
        TIDL_GLOBAL_REPORT_ERROR("Number of detection layers must be 1, found %d", numDetectionLayers);
        return false;
    }
    if(noOfdetectionOutputs != 4)
    {
        TIDL_GLOBAL_REPORT_ERROR("Detection layer must have 4 outputs, found %d - structure requirements for OD post processing optimization not met ", noOfdetectionOutputs);
        return false;
    }
    if(inputsofCustom.size() != 2)
    {
        TIDL_GLOBAL_REPORT_ERROR("Detection layer must have 2 inputs, found %d -  structure requirements for OD post processing optimization not met ", inputsofCustom.size());
        return false;
    }

    /*** Derive number of convolution heads ***/
    context->GetNodeAndRegistration(context, detectionNodeIdx, &node, &registration);
    const TfLiteTensor* anchor_boxes = &context->tensors[node->inputs->data[2]];
    int32_t numConvHeads = TIDL_deriveNumberOfConvHeadsFromAnchorBoxes(anchor_boxes);
    if(numConvHeads == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      return false;
    }

    /**************** Trace back and check the structure assumptions *****************/

    vector<string> inputsofReshape = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinReshape,inputsofCustom);
    vector<string> inputsofConcatBox;
    if(inputsofReshape.size() == 0 ) /* Concat -> Detection */
    {
        inputsofConcatBox = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinConcatenation,inputsofCustom);
    }
    else  /* Reshape -> Concat -> Detection */
    {
        inputsofConcatBox = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinConcatenation,inputsofReshape);
    }

    if(inputsofConcatBox.size() == 0)
    {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Concat -> Reshape -> Detection structure requirements for OD post processing optimization not met ");
        return false;
    }

    vector<string> inputsofLogistic = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinLogistic,inputsofCustom);
    vector<string> inputsofSoftmax = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinSoftmax,inputsofCustom);
    vector<string> inputsofLogisticOrSoftmax;
    if(inputsofLogistic.size() > 0 && inputsofSoftmax.size() > 0)
    {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Logistic/Softmax -> Detection structure requirements for OD post processing optimization not met ");
        return false;
    }
    if(inputsofLogistic.size() > 0)
    {
        for (auto i = inputsofLogistic.begin(); i != inputsofLogistic.end(); ++i)
        {
            inputsofLogisticOrSoftmax.push_back(*i);
        }
    }
    else if(inputsofSoftmax.size() > 0)
    {
        for (auto i = inputsofSoftmax.begin(); i != inputsofSoftmax.end(); ++i)
        {
            inputsofLogisticOrSoftmax.push_back(*i);
        }
    }
    else
    {
        TIDL_GLOBAL_REPORT_ERROR("Logistic/Softmax -> Detection structure requirements for OD post processing optimization not met");
        return false;
    }

    score_convertor = inputsofLogistic.size() != 0 ? 1 : 2; /* 1-->logistic/sigmoid and 2->softmax */

    vector<string> inputsofDiv = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinDiv,inputsofLogisticOrSoftmax);
    vector<string> inputsofConcatClass;

    if( inputsofDiv.size() == 0) /* Concat -> Logistic/Softmax -> Detection */
    {
        inputsofConcatClass = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinConcatenation,inputsofLogisticOrSoftmax);
    }
    else /* Concat -> Div by 1 -> Logistic/Softmax -> Detection*/
    {
        /* Check if division is divide by 1 operation, else not supported */
        TfLiteNode* node;
        TfLiteRegistration* registration;
        for(int node_index : TfLiteIntArrayView(plan))
        {
            context->GetNodeAndRegistration(context, node_index, &node, &registration);
            if(registration->builtin_code == kTfLiteBuiltinDiv)
            {
                if(context->tensors[node->outputs->data[0]].name == inputsofLogisticOrSoftmax[0])
                {
                    TfLiteTensor* tensordiv = &context->tensors[node->inputs->data[1]];
                    if(((*(tensordiv->data.f)) == (float)1) && (tensordiv->bytes == 4) && (tensordiv->type == kTfLiteFloat32))
                    {
                        inputsofConcatClass = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinConcatenation,inputsofDiv);
                    }
                    else
                    {
                        TIDL_GLOBAL_REPORT_ERROR("Div_by_1 -> Logistic/Softmax structure requirements for OD post processing optimization not met ");
                        return false;
                    }
                }
            }
        }
    }
    if(inputsofConcatClass.size() == 0)
    {
        TIDL_GLOBAL_REPORT_ERROR("Concat -> Logistic/Softmax -> Detection structure requirements for OD post processing optimization not met ");
        return false;
    }
    if((inputsofConcatBox.size() != numConvHeads) || (inputsofConcatClass.size() != numConvHeads))
    {
        TIDL_GLOBAL_REPORT_ERROR("Number of box inputs != Convolution heads OR Number of class inputs != Convolution heads for concat - structure requirements for OD post processing optimization not met ");
        return false;
    }

    vector<string> inputsofreshapeBox = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinReshape,inputsofConcatBox);
    vector<string> inputsofreshapeClass = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinReshape,inputsofConcatClass);
    if(((inputsofreshapeBox.size() == 0) || (inputsofreshapeClass.size() == 0)))
    {
        TIDL_GLOBAL_REPORT_ERROR("Reshape -> Concat -> Reshape -> Detection structure requirements for OD post processing optimization not met ");
        return false;
    }
    if((inputsofreshapeBox.size() != numConvHeads) || (inputsofreshapeClass.size() != numConvHeads))
    {
        TIDL_GLOBAL_REPORT_ERROR("Number of box inputs != Convolution heads OR Number of class inputs != Convolution heads for reshape - structure requirements for OD post processing optimization not met ");
        return false;
    }

    vector<string> inputsofconvolutionBox = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinConv2d,inputsofreshapeBox);
    vector<string> inputsofconvolutionClass = TIDL_findLayerAndReturnInputsGivenOutputs(context,plan,kTfLiteBuiltinConv2d,inputsofreshapeClass);

    if(((inputsofconvolutionBox.size() == 0) || (inputsofconvolutionClass.size() == 0)) || (inputsofconvolutionBox.size() != inputsofconvolutionClass.size()))
    {
        TIDL_GLOBAL_REPORT_ERROR("Conv -> Reshape -> Concat -> Reshape -> Detection OR Conv -> Reshape -> Concat -> Logistic -> Detection structure requirements for OD post processing optimization not met ");
        return false;
    }
    if((inputsofconvolutionBox.size() != numConvHeads) || (inputsofconvolutionClass.size() != numConvHeads))
    {
        TIDL_GLOBAL_REPORT_ERROR("Number of box inputs != Convolution heads OR Number of class inputs != Convolution heads for reshape - structure requirements for OD post processing optimization not met ");
        return false;
    }

    /* Populate OD head values */
    for(int i = 0; i < inputsofconvolutionBox.size(); i++)
    {
        odHeads.push_back(inputsofreshapeBox[i]);
        odHeads.push_back(inputsofreshapeClass[i]);
    }

    /****************** Miscellaneous checks ****************/

    /* If any layer in post processing is part of deny list, optimized post processing won't be supported */

    std::vector<int> postProcLayerTypes = {kTfLiteBuiltinCustom, kTfLiteBuiltinReshape, kTfLiteBuiltinConcatenation};
    if(inputsofSoftmax.size() > 0) postProcLayerTypes.push_back(kTfLiteBuiltinSoftmax);
    if(inputsofLogistic.size() > 0) postProcLayerTypes.push_back(kTfLiteBuiltinLogistic);
    if(inputsofDiv.size() > 0) postProcLayerTypes.push_back(kTfLiteBuiltinDiv);

    bool isAnyPostProcLayerInDenyList = false;
    std::string layerType;
    for(int i = 0; i < options->m_deny_list_layer_type.size(); i++)
    {
        for(int j = 0; j < postProcLayerTypes.size(); j++)
        {
            if(std::atoi(options->m_deny_list_layer_type[i].c_str()) == postProcLayerTypes[j])
            {
                isAnyPostProcLayerInDenyList = true;
                layerType = options->m_deny_list_layer_type[i];
                break;
            }
        }
    }
    if(isAnyPostProcLayerInDenyList)
    {
        TIDL_GLOBAL_REPORT_ERROR("Optimized post processing using meta architecture is not being used since one of the post processing layers with tflite layer type : %s is specified in deny_list", layerType.c_str());
        return false;
    }

    std::vector<std::string> postProcLayerNames = {};
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofCustom.begin(), inputsofCustom.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofReshape.begin(), inputsofReshape.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofConcatBox.begin(), inputsofConcatBox.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofreshapeBox.begin(), inputsofreshapeBox.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofLogisticOrSoftmax.begin(), inputsofLogisticOrSoftmax.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofDiv.begin(), inputsofDiv.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofConcatClass.begin(), inputsofConcatClass.end());
    postProcLayerNames.insert(postProcLayerNames.end(), inputsofreshapeClass.begin(), inputsofreshapeClass.end());
    isAnyPostProcLayerInDenyList = false;
    std::string layerName;
    for(int i = 0; i < options->m_deny_list_layer_name.size(); i++)
    {
        for(int j = 0; j < postProcLayerNames.size(); j++)
        {
            if(options->m_deny_list_layer_name[i] == postProcLayerNames[j])
            {
                isAnyPostProcLayerInDenyList = true;
                layerName = options->m_deny_list_layer_name[i];
                break;
            }
        }
    }
    if(isAnyPostProcLayerInDenyList)
    {
        TIDL_GLOBAL_REPORT_ERROR("Optimized post processing using meta architecture is not being used since one of the post processing layers with tflite layer : %s is specified in deny_list", layerName.c_str());
        return false;
    }

    /* Calculate aspect ratios for all the heads (they may not be same - the first head may have less number of aspect ratios, in that case -
    reduce_boxes_in_lowest_layer must be set to 1)*/
    for(int node_index : TfLiteIntArrayView(plan))
    {
        context->GetNodeAndRegistration(context, node_index, &node, &registration);
        TfLiteTensor* tensor = &context->tensors[node->outputs->data[0]];
        for(int i = 0; i < inputsofreshapeBox.size(); i++)
        {
            if(tensor->name == inputsofreshapeBox[i])
            {
                numAspectRatios.push_back(tensor->dims->data[3] / 4); /* no of aspect ratio for each head */
            }
        }
    }

    int allAreEqual = numAspectRatios[0];
    for(int i = 0; i < numAspectRatios.size(); i++)
    {
        /* TODO : Put a condition to check if aspect ratio[0] == aspect_ratio[1]/2 == 3 based on assumption */
        if(allAreEqual != numAspectRatios[i])
        {
            reduce_boxes_in_lowest_layer = 1;
            break;
        }
    }

    return true;
}

/* This function maps Tflite detection post process layer parameters to TIDL_TFSSDConfig_t structure which is used as part of import optimization
    of OD networks - this can be viewed as indirect meta architecture generation */
int32_t TIDL_mapDetectionLayerParamsToSsdConfig(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_odParameters_t * odUserParams,
                                   vector<int> numAspectRatios,std::vector<string> odHeads, int height, int width, int reduce_boxes_in_lowest_layer, int score_converter)
{
  int32_t numTFMetaLayers = 1; /* Only 1 detection layer supported as checked in TIDL_isSsdOdPostProcStructureSupported*/

  const flexbuffers::Map& m = flexbuffers::GetRoot(reinterpret_cast<const uint8_t*>(node->custom_initial_data), node->custom_initial_data_size).AsMap();
  int32_t max_detections = m["max_detections"].AsInt32();
  int32_t max_classes_per_detection = m["max_classes_per_detection"].AsInt32();
  int32_t detections_per_class;
  if (m["detections_per_class"].IsNull())
    detections_per_class = 100;
  else
    detections_per_class = m["detections_per_class"].AsInt32();
  if(detections_per_class == 0)
  {
    detections_per_class = 100;
  }
  bool use_regular_non_max_suppression;
  if (m["use_regular_nms"].IsNull())
    use_regular_non_max_suppression = false;
  else
    use_regular_non_max_suppression = m["use_regular_nms"].AsBool();

  float nms_score_threshold = m["nms_score_threshold"].AsFloat();
  float iou_threshold = m["nms_iou_threshold"].AsFloat();

  int32_t num_classes = m["num_classes"].AsInt32();
  float y_scale = m["y_scale"].AsFloat();
  float x_scale = m["x_scale"].AsFloat();
  float h_scale = m["h_scale"].AsFloat();
  float w_scale = m["w_scale"].AsFloat();

  if(odUserParams->confidence_threshold != -1) //Not default, provided by user
  {
    nms_score_threshold = odUserParams->confidence_threshold;
  }
  if(odUserParams->nms_threshold != -1)
  {
    iou_threshold = odUserParams->nms_threshold;
  }
  if(odUserParams->top_k != -1)
  {
    detections_per_class = odUserParams->top_k;
  }
  if(odUserParams->keep_top_k != -1)
  {
    if(odUserParams->keep_top_k > max_detections)
    {
      TIDL_GLOBAL_REPORT_WARNING("keep_top_k(%d) cannot be specified to be greater than 'max_detections'(%d) parameters in the tflite detection post processing layer. Setting it to max_detections = %d", odUserParams->keep_top_k, max_detections, max_detections);
    }
    else
    {
      max_detections = odUserParams->keep_top_k;
    }
  }

  for(int i = 0; i< numTFMetaLayers; i++)
  {
    TIDL_TFSSDConfig_t &tidl_TFSSDConfig = tidl_TFSSDConfigs[0];
    tidl_TFSSDConfig.num_classes = num_classes;
    tidl_TFSSDConfig.max_detections_per_class = detections_per_class;  //topk
    tidl_TFSSDConfig.max_total_detections = max_detections;    //keeptopk
    tidl_TFSSDConfig.iou_threshold = iou_threshold;
    tidl_TFSSDConfig.score_threshold = nms_score_threshold;
    tidl_TFSSDConfig.x_scale = x_scale;
    tidl_TFSSDConfig.y_scale =  y_scale;
    tidl_TFSSDConfig.height_scale = h_scale;
    tidl_TFSSDConfig.width_scale =  w_scale;
    tidl_TFSSDConfig.num_keypoints = 0;
    tidl_TFSSDConfig.num_layers = odHeads.size()/2;
    tidl_TFSSDConfig.score_converter = score_converter;
    tidl_TFSSDConfig.interpolated_scale_aspect_ratio = 1;
    tidl_TFSSDConfig.base_anchor_height = 1;
    tidl_TFSSDConfig.base_anchor_width = 1;
    tidl_TFSSDConfig.reduce_boxes_in_lowest_layer = reduce_boxes_in_lowest_layer; /* Not used currently in tidl_addMetaArchLayersToNet, but populated for sake of completion */
    tidl_TFSSDConfig.num_ars = numAspectRatios; /* Instead of relying on reduce_boxes_in_lowest_layer set correctly (different aspect ratios),
    better and more robust to actually derive numAspectRatios from conv heads outputs and populate as array */

    tidl_TFSSDConfig.encode_background_as_zeros = 1;
    tidl_TFSSDConfig.num_classes++;

    const TfLiteTensor* anchor_boxes;
    anchor_boxes = &context->tensors[node->inputs->data[2]];

    auto * boxPtr = reinterpret_cast<float *>(anchor_boxes->data.raw);
    float * anchorBoxInput = (float *)malloc(anchor_boxes->bytes);
    memcpy(anchorBoxInput, boxPtr, anchor_boxes->bytes);
    tidl_TFSSDConfig.anchorInputsFromModel = anchorBoxInput;
  }
  return 0;
}

#ifdef __cplusplus
}
#endif

// Convert a tflite operator to a TIDL layer
static int32_t TIDL_tfliteRtMapNode(const TfLiteRegistration* registration, const TfLiteNode* node, TfLiteContext* context, sTIDL_LayerPC_t &layer,
                                    sTIDL_odParameters_t * odUserParams)
{
  int32_t status = TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
  TfLiteTensor * tensor;
  /*** TODO: Add layer mapping using table as in tfliteImport ***/

  /*** [Doubt] no hard sigmoid in tflite ***/
  // else if(registration->builtin_code == kTfLiteBuiltinTanh)
  //   status = TIDL_tfliteRtMapHardSigmoidParams(registration, node, context, layer);

  layer = TidlParseTflite(registration, node, context, layer).layer;
  status = tidlCheckAllowlistingConstraints(layer);
  return status;
}

int32_t TIDL_tfliteRtLayerUpdateConsumerCount(TfLiteContext* context, const TfLiteDelegateParams* params, int32_t layerIndex)
{
  int32_t i0, i1, i2;
  int32_t numCons = 0;
  TfLiteNode* node;
  TfLiteRegistration* registration;
  TfLiteTensor* tensor;

  for (i0 = 0; i0 < orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs; i0++)
  {
    for (i1 = 0; i1 < params->nodes_to_replace->size; i1++)
    {
      TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(context, params->nodes_to_replace->data[i1], &node, &registration));
      for (i2 = 0; i2 < node->inputs->size; i2++)
      {
        tensor = &context->tensors[node->inputs->data[i2]];
        if(tensor->allocation_type == kTfLiteArenaRw)
        {
          if(registration->builtin_code == kTfLiteBuiltinPack)
          {
            if(node->inputs->size == 2)
            {
              if((strncmp((const char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[i0], tensor->name, TIDL_STRING_SIZE) == 0)
                 && (strcmp(context->tensors[node->inputs->data[0]].name, context->tensors[node->inputs->data[1]].name) == 0))
              {
                //pack layer has both inputs with same name, but actually only one edge connection in graph, so outConsumerCnt should be 1
                numCons = 1;
                break;
              }
            }
          }
          else if (strncmp((const char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[i0], tensor->name, TIDL_STRING_SIZE) == 0)
          {
            numCons++;
          }
        }
      }
    }
    for (i2 = 0; i2 < numNetOutData; i2++)
    {
      if (strcmp((const char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[i0], outDataNames[i2]) == 0)
      {
        numCons++;
      }
    }
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerCnt[i0] = numCons;
  }
  return 0;
}

int32_t TIDL_tfliteRtLayerFillTensorNames(TfLiteContext* context, TfLiteNode* node, int32_t layerIndex)
{
  int32_t j, inBufIdx = 0;
  TfLiteTensor* outTensor = &context->tensors[node->outputs->data[0]];
  TfLiteTensor* inTensor;
  TfLiteTensor* tensor;

  std::vector<int32_t> varIdx;
  int32_t numVarInputs = TIDL_tfliteRtNumVariableLayerInputs(context, node, varIdx);

  /** For single input, we are doing strncpy of 512 strings, so when we do comparison to input tensor name for linking inputs/outputs,
   * we are comparing 512 vs 512 chars, but if there are 2 inputs, on casting to char*, more characters are read(>512 since no NULL terminator)
   * for 1st string vs 512 in the string you are trying to compare against, resulting in issue
   * Now, problem will still exist in single input case, if first 512 characters of both strings being compared are same
   * Fix with always exiting if name > 512 chars results in mlperf tflite ssd mobilenetv2 model to fail
   * Permanent fix is to replace char arrays with strings
   */
  if((strlen(outTensor->name) >= TIDL_STRING_SIZE) && (node->outputs->size > 1)) //last character should be NULL
  {
    TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", outTensor->name, TIDL_STRING_SIZE);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else
  {
    strncpy((char*)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, outTensor->name, TIDL_STRING_SIZE);
  }

  if (orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs > 0)
  {
    for (j = 0; j < node->inputs->size; j++)
    {
      inTensor = &context->tensors[node->inputs->data[j]];
      //if(orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType == TIDL_Deconv2DLayer)
      //{
      //  strcpy((char*)orgTIDLNetStructure.TIDLPCLayers[layerIndex].inDataNames[j], tensors->Get(op->inputs()->Get(j+2))->name()->c_str());
      //}
      // For Now add all contansts. We will have to do this specifically for Eltwise layers
      if(inTensor->allocation_type == kTfLiteArenaRw || orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType == TIDL_EltWiseLayer)
      {
        if((strlen(inTensor->name) >= TIDL_STRING_SIZE) && (numVarInputs > 1))
        {
          TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", inTensor->name, TIDL_STRING_SIZE);
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        else
        {
          strncpy((char*)orgTIDLNetStructure.TIDLPCLayers[layerIndex].inDataNames[inBufIdx], inTensor->name, TIDL_STRING_SIZE);
        }

        /* Populate tensor scale and zero point if 8-bit:*/
        if(gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
        {
          if ((inTensor->type == kTfLiteUInt8) || (inTensor->type == kTfLiteInt8))
          {
            TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)inTensor->quantization.params;
            auto * scale = quantization->scale;
            auto *zero_point = quantization->zero_point;
            orgTIDLNetStructure.TIDLPCLayers[layerIndex].inData[inBufIdx].tensorScale = scale->data[0];
            orgTIDLNetStructure.TIDLPCLayers[layerIndex].inData[inBufIdx].tensorZeroPoint = zero_point->data[0];
          }
        }
        /******************************************/
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].inData[inBufIdx].dataId = -1;
        inBufIdx++;
      }
    }
  }
  if (orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs > 0)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerLinked[0] = 0;
    for (j = 0; j < orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs; j++)
    {
      tensor = &context->tensors[node->outputs->data[j]];

      if((strlen(tensor->name) >= TIDL_STRING_SIZE) && (node->outputs->size > 1))
      {
        TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", tensor->name, TIDL_STRING_SIZE);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      else
      {
        strncpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[j], tensor->name, TIDL_STRING_SIZE);
      }

      // Save intermediate tensor name in outDataNames[0] and the OD post proc layers names in outDataNames[1,2,3,4]
      if(orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType == TIDL_DetectionOutputLayer)
      {
        strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[j], "TfLite_Detection_Process_Intermediate");
      }

      /* Populate tensor scale and zero point if 8-bit: */
      if(gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
      {
        if ((tensor->type == kTfLiteUInt8) || (tensor->type == kTfLiteInt8))
        {
          TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)tensor->quantization.params;
          auto * scale = quantization->scale;
          auto *zero_point = quantization->zero_point;
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[j].tensorScale = scale->data[0];
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[j].tensorZeroPoint = zero_point->data[0];
       }
      }
      /******************************************/

      /*if(orgTIDLNetStructure.TIDLPCLayers[layerIndex].actParams.actType == TIDL_RelU6)
      {
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.isClipEnabled = TIDL_Clip;
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.clipMax = 6.0;
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.clipMin = 0;
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].elementType = tidl_getElementType(0);

      }*/
      if ((tensor->type == kTfLiteUInt8) || (tensor->type == kTfLiteInt8))
      {
        TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)tensor->quantization.params;
        auto * scale = quantization->scale;
        auto *zero_point = quantization->zero_point;
        if (scale && zero_point)
        {
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.isClipEnabled = 1;

          int32_t inMax = 127;
          int32_t inMin = -128;

          if (tensor->type == kTfLiteUInt8)
          {
            inMax = 255;
            inMin = 0;
          }

          orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.clipMax = (inMax - zero_point->data[0]) * scale->data[0];
          orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.clipMin = (inMin-zero_point->data[0]) * scale->data[0];
          if(orgTIDLNetStructure.TIDLPCLayers[layerIndex].clipParams.clipMin >= 0)
          {
            orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].elementType = tidl_getElementType(0);
          }
          else
          {
            orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].elementType = tidl_getElementType(1);
          }
        }
      }
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerLinked[j] = 0;
    }
  }
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].weightsElementSizeInBits = NUM_WHGT_BITS;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

#ifdef __cplusplus
extern "C"
{
#endif

/** Add a layer type to deny list option */
void TIDL_addLayerToDenyList(int32_t * denyList, int32_t * denyListSize, int32_t layerTfliteIdx)
{
  bool alreadyInDenyList = false;
  for(int i = 0; i < *denyListSize; i++)
  {
    if(denyList[i] ==  layerTfliteIdx)
    {
      alreadyInDenyList = true;
      break;
    }
  }
  if(! alreadyInDenyList)
  {
    denyList[*denyListSize] = layerTfliteIdx;
    *denyListSize++;
  }
}

/* Return shape of input for given node */
std::vector<int32_t> getNodeInputShape(TfLiteContext* context, const TfLiteNode* node)
{
    std::vector<int32_t> dims;
    for (auto tensor_index : TfLiteIntArrayView(node->inputs))
    {
        if (tensor_index == kTfLiteOptionalTensor)
        {
            continue;
        }
        TfLiteTensor* tensor = &context->tensors[tensor_index];
        if(tensor->allocation_type == kTfLiteArenaRw)
        {
            int32_t numDims = tensor->dims->size;
            for(int i = 0; i < numDims; i++)
            {
                dims.push_back(tensor->dims->data[i]);
            }
            break; //write dims for first ArenaRw input
        }
    }
    return dims;
}

/* For a layer, TIDL does not support if input has more than 4 dimensions */
int32_t TIDL_checkLayerInputDimConstraints(TfLiteContext* context, const TfLiteNode* node)
{
  std::vector<int32_t> nodeInputDims = getNodeInputShape(context, node);
  if(nodeInputDims.size() > TIDL_DIM_MAX) //layer supported only if number of dimensions of input <= 4
  {
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

// Called from tfl_delegate to identify nodes that are supported by TIDL.
int32_t TIDL_tfliteAllowlistNode(const TfLiteRegistration* registration, const TfLiteNode* node, int32_t node_index, TfLiteContext* context,
                                   TIDL_osrtOptions * osrtOptions, bool isObjectDetectionNetwork, std::vector<int> odProcNodesComplement)
{
  int32_t osrtDebugPrintLevel = osrtOptions->osrtDebugPrintLevel;
  int32_t * denyList = osrtOptions->m_deny_list_int.data();
  int32_t denyListSize = osrtOptions->m_deny_list_int.size();
  std::vector<std::string> denyList_layer_type = osrtOptions->m_deny_list_layer_type;
  std::vector<std::string> denyList_layer_name = osrtOptions->m_deny_list_layer_name;
  std::vector<std::string> allowList_layer_name = osrtOptions->m_allow_list_layer_name;
  sTIDL_odParameters_t * odUserParams = &osrtOptions->odUserParams;
  int metaarchtype = osrtOptions->m_meta_arch_type;

  sTIDL_LayerPC_t layer;
  memset(&layer, 0, sizeof(sTIDL_LayerPC_t));

  /* Populating gParams required by model checker */
  gParams.modelType = TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT;
  gParams.numParamBits = osrtOptions->m_num_param_bits;
  char * tidl_tools_path = const_cast<char *>(osrtOptions->m_tidl_tools_path.c_str());
  if(tidl_tools_path)
  {
    strcpy((char*)gParams.perfSimConfig, tidl_tools_path);
    strcat((char*)gParams.perfSimConfig,"/device_config.cfg");
  }
  else
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Please provide TIDL tools path");
  }

  /* Device specific allow-listing checks require deviceName to be populated */
  int32_t status = TIDL_readDeviceConfigParams((char*)gParams.perfSimConfig);
  /* aborting here */
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
  }

  TfLiteTensor * tensor;
  tensor = &context->tensors[node->outputs->data[0]];

  /* If allowlist is specified, all layers which are not in allow-list must be delegated to ARM */
  if(allowList_layer_name.size() > 0)
  {
    int32_t isNodeInAllowListName = 0;
    for(int i = 0; i < allowList_layer_name.size(); i++)
    {
      if(tensor->name == allowList_layer_name[i])
      {
        isNodeInAllowListName = 1;
        break;
      }
    }
    if(isNodeInAllowListName == 0)
    {
      TIDL_LOG_INFO(gDiags.gDiagList, "Layer %s added to unsupported nodes since it is not part of allow list specified in runtime options --- tflite layer code - %s", tensor->name, registration->builtin_code);
      if (osrtDebugPrintLevel)
      {
        gDiags.reportLastModeDiag();
      }
      return 0;
    }
  }

  //First check deny list
  int32_t isNodeInDenyList = 0;                         //option for deny_list
  for(int i = 0; i < denyListSize; i++)
  {
    if(registration->builtin_code == denyList[i])
    {
      isNodeInDenyList = 1;
      break;
    }
  }

  int32_t isNodeInDenyListType = 0;                     //option for deny_list_type
  for(int i = 0; i < denyList_layer_type.size(); i++)
  {
    if(std::to_string(registration->builtin_code) == denyList_layer_type[i])
    {
      isNodeInDenyListType = 1;
      break;
    }
  }

  int32_t isNodeInDenyListName = 0;                       //option for deny_list_name
  for(int i = 0; i < denyList_layer_name.size(); i++)
  {
    if(tensor->name == denyList_layer_name[i])
    {
      isNodeInDenyListName = 1;
      break;
    }
  }

  if(isNodeInDenyList || isNodeInDenyListType || isNodeInDenyListName)              //option for deny_list
  {
    TIDL_LOG_INFO(gDiags.gDiagList, "Layer %s added to unsupported nodes as specified in deny list --- tflite layer code - %d", tensor->name, registration->builtin_code);
    if (osrtDebugPrintLevel)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }
  //OD related checks
  if(isObjectDetectionNetwork)
  {
    if((odProcNodesComplement.size() == 0)) //means post processing on ARM mode, allowlist based on numDims != 4
    {
      //tflite detection postproc is handled separately, no neeed for this check which is especially to deny post proc layers
      TfLiteTensor * tensor;
      int32_t numDims;
      for(int i = 0; i < node->inputs->size; i++)
      {
        tensor = &context->tensors[node->inputs->data[i]];
        if(tensor->allocation_type == kTfLiteArenaRw) //inputs coming from previous layer
        {
          numDims = tensor->dims->size;
          if(numDims != 4)
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Layer %d --- tflite layer type - %d, number of input dims(%d) is not supported by TIDL, maximum dimension allowed is %d", node_index, registration->builtin_code, numDims, TIDL_DIM_MAX);
            if (osrtDebugPrintLevel)
            {
              gDiags.reportLastModeDiag();
            }
            return 0;
          }
        }
      }
    }
    else if(std::find(odProcNodesComplement.begin(), odProcNodesComplement.end(), node_index) == odProcNodesComplement.end()) //if node is not part of backbone network, mark it supported
    {
      //this corresponds to case where all post processing nodes are allowed to TIDL and converted to optimized post proc layer
      TIDL_LOG_INFO(gDiags.gDiagList, "Not part of backbone network, will be delegated in post-processing");
      return 1;
    }
  }

  if(TIDL_checkLayerInputDimConstraints(context, node) == -1) //if layer input has more than 4 dimensions, not supported
  {
      int32_t numDims =  getNodeInputShape(context, node).size();
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Layer %d --- Tflite layer type - %d, number of input dims(%d) is not supported by TIDL, maximum dimension allowed is %d", node_index, registration->builtin_code, numDims, TIDL_DIM_MAX);
      if (osrtDebugPrintLevel)
      {
        gDiags.reportLastModeDiag();
      }
      return 0;
  }

  /* will use it to check the logs which got pushed while checking allowlisting for current layer */
  int32_t diagListSize = gDiags.gDiagList.size();

  status = TIDL_tfliteRtMapNode(registration, node, context, layer, odUserParams);
  tensor = &context->tensors[node->outputs->data[0]];

  if (status == TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Layer type not supported by TIDL --- Tflite layer type --- %d  layer output name--- %15s", registration->builtin_code, tensor->name);
    if (osrtDebugPrintLevel > 0)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    if (osrtDebugPrintLevel > 0)
    {
      for(int i=diagListSize;i<gDiags.gDiagList.size();i++)
      {
        auto diag = gDiags.gDiagList[i];
        gDiags.modelDiag(diag);
      }
      TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported (TIDL check) TIDL layer type --- %d Tflite layer type --- %d layer output name--- %15s ", layer.layerType, registration->builtin_code, tensor->name);
    }
    return 0;
  }

  TIDL_LOG_SUPPORTED(gDiags.gDiagList, "Supported TIDL layer type --- %d Tflite layer type --- %d layer output name--- %15s ", layer.layerType, registration->builtin_code, tensor->name);
  if (osrtDebugPrintLevel)
  {
    gDiags.reportLastModeDiag();
  }
  return 1;
}

int32_t TIDL_getInputDimensions(TfLiteTensor * tensor, int32_t &numBatch, int32_t &numCh, int32_t &height, int32_t &width)
{
  int32_t numInputDims = tensor->dims->size;
  if(numInputDims == 2)
  {
    numBatch = tensor->dims->data[0];
    width = tensor->dims->data[1];
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Standard number of input dimensions is 4, given 2 [%d, %d]-- Will be converted to [%d, %d, %d, %d] (NCHW format) internally ",
    tensor->dims->data[0], tensor->dims->data[1], numBatch, numCh, height, width);
  }
  else if(numInputDims == 3)
  {
    numBatch = tensor->dims->data[0];
    height = tensor->dims->data[1];
    width = tensor->dims->data[2];
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Standard number of input dimensions is 4, given 3 [%d, %d, %d]-- Will be converted to [%d, %d, %d, %d] (NCHW format) internally ",
    tensor->dims->data[0], tensor->dims->data[1], tensor->dims->data[2], numBatch, numCh, height, width);
  }
  else if(numInputDims == 4)  //Standard case
  {
    numBatch = tensor->dims->data[0];
    height = tensor->dims->data[1];
    width = tensor->dims->data[2];
    numCh = tensor->dims->data[3];
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Number of input dimensions = '%d' is not supported, please provide input with 4 dimensions", numInputDims);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
/** This function adds input and output data layers to subgraphs and sets their properties
 * Also validates the import paramters and does meta architecture import for OD models with meta arch file provided */
int32_t TIDL_tfliteRtImportInit(TfLiteContext* context, const TfLiteDelegateParams* params, int32_t subgraph_id, TIDL_osrtOptions * osrt_options,
                                  std::vector<std::string>outDataOd, bool isSubgraphOD)
{
  TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "In TIDL_tfliteRtImportInit subgraph_id=%d", subgraph_id);

  // Reset all the memories to to NULL, there could be multiple subgraphs
  memset(&orgTIDLNetStructure, 0, sizeof(sTIDL_OrgNetwork_t));
  memset(&tIDLNetStructure,    0, sizeof(sTIDL_Network_t));
  memset(inDataNames,          0, sizeof(inDataNames));
  memset(outDataNames,         0, sizeof(outDataNames));

  runtimes_import_state.layerIndex = 0;
  runtimes_import_state.dataIndex  = 0;

  // Initialize layer independent parameters of gParams
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runtimesGparamsInit(osrt_options, TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT), "");

  int32_t status;

  sprintf((char *)gParams.metaLayersNamesList, osrt_options->m_meta_layers_names_list.c_str());
  int currInIdx = 0, currOutIdx = 0;
  void *ptr;
  int32_t elementSize;

  if((outDataOd.size() == 0) || (! isSubgraphOD)) //Not an OD network / Subgraph does not contain OD post processing part
  {
    gParams.metaArchType = -1;
    for (auto tensor_index : TfLiteIntArrayView(params->output_tensors)) // output tensors of the subgraph
    {
      if (tensor_index == kTfLiteOptionalTensor)
      {
        continue;
      }
      TfLiteTensor* tensor = &context->tensors[tensor_index];
      // Const tensors should be added as const nodes during graph construction.
      if(tensor->allocation_type == kTfLiteArenaRw)
      {
        int32_t layerIndex = TIDL_addOutputDataLayer(currOutIdx, tensor->name);

        TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "Layer %d, subgraph id %d, name=%s", layerIndex, subgraph_id, tensor->name);
        if(gParams.addDataConvertToNet & ADD_DC_LAYER_AT_OUTPUT)
        {
          gParams.outLayout[currOutIdx] = TIDLRT_LT_NHWC;
          TIDL_tfliteRtGetScaleAndZeroPoint(tensor, &gParams.outTensorScale[currOutIdx], &gParams.outZeroPoint[currOutIdx]);
          if(gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
          {
            /** TIDL_tfliteRtGetScaleAndZeroPoint populates outTensorScale = 1 / Model scale
             * Scale given by this quantization style = Model scale so set below */
            gParams.outTensorScale[currOutIdx] = 1.0 / gParams.outTensorScale[currOutIdx];
          }
          status =  TIDL_tfliteRtGetTypeAndPtr(tensor, &gParams.outElementType[currOutIdx], &ptr, &elementSize);
          if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
          {
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
        }
        currOutIdx++;
      }
    }
    numNetOutData = currOutIdx;
  }
  else
  {
    gParams.metaArchType = osrt_options->m_meta_arch_type;
    numNetOutData = outDataOd.size() ; //The OD heads are treated as outputs, since meta arch import assumes output data layers at heads
    for (int i = 0; i < numNetOutData; i++)
    {
      int32_t layerIndex = TIDL_addOutputDataLayer(i, (char *)outDataOd[i].c_str());

      TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "Layer %d, subgraph id %d, name=%s", layerIndex, subgraph_id, (char*)outDataOd[i].c_str());
      if(gParams.addDataConvertToNet & ADD_DC_LAYER_AT_OUTPUT)
      {
        gParams.outLayout[currOutIdx] = TIDLRT_LT_NHWC;
        gParams.outTensorScale[currOutIdx] = 1.0;
        gParams.outZeroPoint[currOutIdx] = 0;
        gParams.outElementType[currOutIdx] = TIDL_SinglePrecFloat;
      }
      currOutIdx++;
    }
  }

  for (auto tensor_index : TfLiteIntArrayView(params->input_tensors))
  {
    if (tensor_index == kTfLiteOptionalTensor)
    {
      continue;
    }
    TfLiteTensor* tensor = &context->tensors[tensor_index];

    // Check if tensor is a network layer and not a parameter input
    if(tensor->allocation_type == kTfLiteArenaRw)
    {
      int32_t numBatch = 1, numCh = 1, height = 1, width = 1;
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_getInputDimensions(tensor, numBatch, numCh, height, width), "");

      int32_t layerIndex = TIDL_addInputDataLayer(currInIdx, numBatch, numCh, height, width, tensor->name);
      TIDL_tfliteRtGetScaleAndZeroPoint(tensor, &gParams.inQuantFactor[currInIdx], &gParams.inZeroPoint[currInIdx]);
      if(gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
      {
        /** TIDL_tfliteRtGetScaleAndZeroPoint populates inTensorScale = 1 / Model scale
           * Scale required by this quantization style = Model scale so set below */
        gParams.inQuantFactor[currInIdx] =  1 / gParams.inQuantFactor[currInIdx];
      }
      status =  TIDL_tfliteRtGetTypeAndPtr(tensor, &gParams.inElementType[currInIdx], &ptr, &elementSize);
      if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      if(gParams.addDataConvertToNet & ADD_DC_LAYER_AT_INPUT)
      {
        gParams.inLayout[currInIdx] = TIDLRT_LT_NHWC;
        gParams.rawDataInElementType[currInIdx]   = TIDL_SinglePrecFloat;
      }
      else
      {
        if(gParams.numFeatureBits == 32)
        {
          gParams.inElementType[currInIdx] = TIDL_SinglePrecFloat;
        }
        else if (osrt_options->m_num_param_bits > 8)
        {
          gParams.inElementType[currInIdx] = TIDL_SignedShort;
        }
        else
        {
          gParams.inElementType[currInIdx] = TIDL_UnsignedChar;
          if(tensor->type == kTfLiteInt8)
          {
            gParams.inElementType[currInIdx] = TIDL_SignedChar;
          }
          else if(tensor->type == kTfLiteUInt8)
          {
            gParams.inElementType[currInIdx] = TIDL_UnsignedChar;
          }
          /* For case of models having quantize layer with input float */
          else if((tensor->type == kTfLiteFloat32) &&
                  (gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL))
          {
            gParams.inElementType[currInIdx] = TIDL_SinglePrecFloat;
          }
        }
      }

      TIDL_tfliteRtLayerUpdateConsumerCount(context, params, layerIndex);
      tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
      TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "Layer %d, subgraph id %d, name=%s", layerIndex, subgraph_id, tensor->name);
      currInIdx++;
    }
  }

  // Initialize rest of the layers
  for (int i = currInIdx; i < TIDL_NUM_MAX_PC_LAYERS; i++)
  {
    sTIDL_LayerPC_t& layer_i = orgTIDLNetStructure.TIDLPCLayers[i];
    layer_i.actParams.actType  = TIDL_NoAct;
    layer_i.strideOffsetMethod = TIDL_StrideOffsetCenter;
  }

  /* Subgraph inputs are saved in raw file in float data format with corresponding quantization scales */
  for (int i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
  {
    gParams.rawDataInElementType[i]   = TIDL_SinglePrecFloat;
  }

  if (tidlValidateImportParams(&gParams) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Validation of TIDL tflite runtime import config parameters failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  if(gParams.metaArchType != -1)
  {
    if(gParams.metaArchType == TIDL_metaArchTFSSD)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tf_metaArch_import(&gParams), "");
    }
    else if(gParams.metaArchType != TIDL_metaArchTfliteRtSsdDetectionLayer)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_metaArch_import(&gParams), "");
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** This function maps Tflite nodes to TIDL nodes and links them to form TIDL imported graph */
int32_t TIDL_tfliteRtImportAndLinkNode(TfLiteRegistration* registration, TfLiteContext* context, const TfLiteDelegateParams* params,
                                         TfLiteNode* node, int32_t osrtDebugPrintLevel, sTIDL_odParameters_t * odUserParams)
{
  int32_t status = 0;

  // Get new layerIndex, dataIndex
  int32_t layerIndex = runtimes_import_state.layerIndex++;
  int32_t dataIndex  = runtimes_import_state.dataIndex++;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  // set layer defaults
  layer.numInBufs         = 1;
  layer.numOutBufs        = 1;
  layer.outData[0].dataId = dataIndex;

  /* Parse the node */
  layer = TidlParseTflite(registration, node, context, layer).layer;
  TIDL_GLOBAL_REPORT_INFO(osrtDebugPrintLevel, "In TIDL_tfliteRtImportNode, TIDL Layer type - %d, Tflite builtin code type - %d ", layer.layerType, registration->builtin_code);

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfliteRtLayerFillTensorNames(context, node, layerIndex), "");
  TIDL_tfliteRtLayerUpdateConsumerCount(context, params, layerIndex);
  tidl_linkInputTensors(&orgTIDLNetStructure, layerIndex);
  tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);

  return status;
}

#ifdef __cplusplus
}
#endif

}  //namespace tfl_delegate
}  // namespace tflite
