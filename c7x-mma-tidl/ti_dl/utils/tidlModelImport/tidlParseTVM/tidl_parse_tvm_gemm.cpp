/*
*
* Copyright (c) {2025} Texas Instruments Incorporated
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



#include "tidl_parse_tvm.h"
using namespace std;
using namespace tvm;
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
#include <tvm/relay/analysis.h>
#include <tvm/runtime/ndarray.h>

template<>
int32_t TidlParseTVM:: parse<OpNameStr("tidl.gemm")> ()
{
  int32_t status = 0;
  int32_t transA = 0;
  int32_t transB = 0;
  float alpha = 1.0;
  float beta  = 1.0;
  std::string op_name;
  sTIDL_InnerProductParams_t &innerProductParams = layer.layerParams.innerProductParams;
  sTIDL_InnerProductPCParams_t &innerProductPCParams = layer.layerPCParams.innerProductParams;

  layer.layerType = TIDL_InnerProductLayer;
  layer.outData[0].elementType = tidl_getElementType(1);

  const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
  auto fn = GetRef<tvm::relay::Function>(fn_node);
  const auto* attrs = fn->attrs.as<DictAttrsNode>();
  if (attrs->dict.count("Composite")) {
    op_name = Downcast<String>(attrs->dict.at("Composite"));
  }

  if (attrs->dict.count("transA")) {
    transA = Downcast<Integer>(attrs->dict.at("transA"))->value;
  }

  if (attrs->dict.count("transB")) {
    transB = Downcast<Integer>(attrs->dict.at("transB"))->value;
  }

  if (attrs->dict.count("alpha")) {
    alpha = Downcast<FloatImm>(attrs->dict.at("alpha"))->value;
  }

  if (attrs->dict.count("beta")) {
    beta = Downcast<FloatImm>(attrs->dict.at("beta"))->value;
  }
  
  innerProductParams.inputATranspose = transA;
  innerProductParams.inputBTranspose = transB;
    /*Default value for numBchannels:*/
  innerProductParams.numBChannels = 1;
  innerProductPCParams.alpha = alpha;
  innerProductPCParams.beta = beta;
  
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.numInBufs = 2;
  layer.layerPCParams.innerProductParams.isGemm = 0;

  std::vector<int32_t> filterDims;
  int32_t numEle;
  int weight_zp = 0;
  float weight_scale = 1.0f;
  if(strcmp("tidl.gemm", op_name.c_str()) == 0)
  {
    layer.layerPCParams.innerProductParams.isGemm = 1;
    /* input A and input B is present */
    if(md.numInputs == 2)
    {
      /* Input B is constant */
      if(md.constTensorIndices.size() == 1)
      {
        int32_t filterIdx = md.constTensorIndices[0];
        layer.weights=TIDL_extractConstantTensorData(call,filterIdx,md,0);
        if(layer.weights.ptr==NULL)
        {
          TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", filterIdx);
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        filterDims = md.constTensorsDims[0];
        if(md.varTensorIndices[0] < md.constTensorIndices[0])
        {
          /* B is constant */
          innerProductParams.constIdx = 1;
        }
        else
        {
          innerProductParams.constIdx = 0;
        }
      }

      /* C is not present */
      innerProductParams.isBias = 0;
    }
    /* All three inputs (A, B & C) are present */
    else if(md.numInputs == 3)
    {
      std::vector<int32_t> biasDims;
      if(md.constTensorIndices.size() == 1)
      {
        /* Input C is constant */
        if(md.constTensorIndices[0] == 2)
        {
          int32_t biasIndex = md.constTensorIndices[0];
          layer.bias=TIDL_extractConstantTensorData(call,biasIndex,md,0);
          if(layer.bias.ptr==NULL)
          {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", biasIndex);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          innerProductParams.isBias = 1;
          biasDims = md.constTensorsDims[0];
        }
        /* Either A or B is constant */
        else
        {
          int32_t filterIdx = md.constTensorIndices[0];
          layer.weights=TIDL_extractConstantTensorData(call,filterIdx,md,0);
          if(layer.weights.ptr==NULL)
          {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", filterIdx);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          filterDims = md.constTensorsDims[0];
          if(md.varTensorIndices[0] < md.constTensorIndices[0])
          {
            /* B is constant */
            innerProductParams.constIdx = 1;
          }
          else
          {
            innerProductParams.constIdx = 0;
          }
        }
      }
      /* Input (A or B) & C are constant */ 
      else if(md.constTensorIndices.size() == 2)
      {
        int32_t filterIdx = md.constTensorIndices[0];
        layer.weights=TIDL_extractConstantTensorData(call,filterIdx,md,0);
        if(layer.weights.ptr==NULL)
        {
          TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", filterIdx);
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        if(md.constTensorIndices[0] == 1)
        {
          /* B is constant */
          filterDims = md.constTensorsDims[0];
        }
        else
        {
          /* A is constant, B is variable */
          filterDims = md.varTensorsDims[0];
        }
         if(md.constTensorIndices[1] == 2)
        {
             int32_t biasIndex = md.constTensorIndices[1];
             layer.bias=TIDL_extractConstantTensorData(call,biasIndex,md,1);
             if(layer.bias.ptr==NULL)
             {
               TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", biasIndex);
               return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
             }
        }
        biasDims = md.constTensorsDims[1];
         if(md.constTensorIndices[0] == 0 && md.constTensorIndices[1] == 1)
        {
          /* Both A and B are constant, will be denied */
        }
        else if(md.varTensorIndices[0] < md.constTensorIndices[0])
        {
          /* B is constant */
          innerProductParams.constIdx = 1;
        }
        else
        {
          innerProductParams.constIdx = 0;
        }
      }
      else
      {
        /* all three are variable - will get denied in allowlisting */
      }

       innerProductParams.isBias = 1;
    }

  }
  /* Matmul */
  else
  {
    /* Input B is constant */
    if(md.constTensorIndices.size() == 1)
    {
      int32_t filterIdx = md.constTensorIndices[0];
      layer.weights=TIDL_extractConstantTensorData(call,filterIdx,md,0);
      if(layer.weights.ptr==NULL)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", filterIdx);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      filterDims = md.constTensorsDims[0];
      if (md.varTensorIndices[0] < md.constTensorIndices[0])
      {
        innerProductParams.constIdx = 1;
        if(filterDims.size() >= 3)
        {
          /*Number of B channels*/
          innerProductParams.numBChannels = filterDims[filterDims.size() - 3];
        }
      }
      else
      {
        innerProductParams.constIdx = 0;
      }
    }
    /*Number of constant tensor is zero*/
    else if(md.constTensorIndices.size() == 0)
    {
      innerProductParams.numInRows  = -1;
      innerProductParams.numInCols  = -1;
      innerProductParams.numOutCols = -1;
      innerProductParams.isBias     = 0;
      innerProductParams.constIdx = -1;
      layer.weights.ptr = NULL;
      layer.weights.bufSize = 0;
      layer.bias.ptr= NULL;
      layer.bias.bufSize = 0;
    }

    /* No bias in matmul */
    innerProductParams.isBias     = 0;
  }

  /* got filter as constant input */
  if(filterDims.size() > 1)
  {
    int32_t dimOffset = TIDL_DIM_MAX - filterDims.size();
    int32_t filter_rows = filterDims[TIDL_DIM_HEIGHT - dimOffset];
    int32_t filter_cols = filterDims[TIDL_DIM_WIDTH - dimOffset];
    if (md.varTensorIndices[0] < md.constTensorIndices[0])
    {
      /* If const is the second input*/
      if(transB)
      {
        innerProductParams.numOutCols = filter_rows;
        innerProductParams.numInCols  = filter_cols;
      }
      else
      {
        innerProductParams.numOutCols = filter_cols;
        innerProductParams.numInCols  = filter_rows;
      }
    }
    else
    {
      /* If const is the second input*/
      if(transA)
      {
        innerProductParams.numInCols = filter_rows;
        innerProductParams.numInRows = filter_cols;
      }
      else
      {
        innerProductParams.numInCols = filter_cols;
        innerProductParams.numInRows = filter_rows;
      }
    }
  }
  if(strcmp("tidl.gemm", op_name.c_str()) == 0)
  {
    /* C is not present */
    if(md.numInputs == 2)
    {
      innerProductParams.isBias = 0;
      if (layer.numInBufs == 2 && md.constTensorIndices.size() > 0)
      {
        if (md.varTensorIndices[0] < md.constTensorIndices[0])
        {
          innerProductParams.constIdx = 1;
        }
        else
        {
          innerProductParams.constIdx = 0;
        }
      }
      layer.bias.ptr = calloc(innerProductParams.numOutCols * innerProductParams.numBChannels, sizeof(float));
      layer.bias.bufSize = innerProductParams.numOutCols * innerProductParams.numBChannels;
    }
  }
  return 0;
}

