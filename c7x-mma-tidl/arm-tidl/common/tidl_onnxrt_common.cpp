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

#include "tidl_onnxrt_common.h"

/** Get TIDL element type corresponding to ONNX data type */
int32_t TIDL_ortGetType(int64_t ortType, int32_t * type, int32_t * size)
{
  int32_t status = 0;
  if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8)
  {
    *type =  TIDL_UnsignedChar;
    *size = sizeof(uint8_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8)
  {
    *type =  TIDL_SignedChar;
    *size = sizeof(int8_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16)
  {
    *type =  TIDL_UnsignedShort;
    *size = sizeof(uint16_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16)
  {
    *type =  TIDL_SignedShort;
    *size = sizeof(int16_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
  {
    *type =  TIDL_SinglePrecFloat;
    *size = sizeof(float);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32)
  {
    *type =  TIDLRT_Uint32;
    *size = sizeof(uint32_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
  {
    *type =  TIDLRT_Int32;
    *size = sizeof(int32_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64)
  {
    *type =  TIDLRT_Uint64;
    *size = sizeof(uint64_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
  {
    *type =  TIDLRT_Int64;
    *size = sizeof(int64_t);
  }
  else if(ortType == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
  {
    *type =  TIDLRT_Bool;
    *size = sizeof(bool);
  }
  else
  {
    printf("ERROR : ONNX RT data type : %d not supported by TIDL\n", (int32_t)ortType);
    status = -1;
  }
  return status;
}

void TIDL_ortUpdateType(int32_t* type)
{
  if (*type == TIDLRT_Int64)
  {
    *type = TIDLRT_Int32;
  }
}

/** Set TIDL RT input and output tensor properties required for calling invoke */
int32_t TIDL_setRtTensorParameters(sTIDLRT_Tensor_t * allocatedPtrs, sTIDLRT_Tensor_t * rtPtrs[], int32_t currTensorNum, sTIDL_IOBufDesc_t * ioBufDesc, 
                                      sTIDL_tidlRtDynamicLoading_t * infer_ops, onnxRtParams_t * onnxRtParams, int32_t isInfer, int32_t isInput)
{
  int32_t status = 0;

  int32_t elementType, elementSize, memType = 0, bufferSize = -1;
  int32_t numVirtualCores = ioBufDesc->numVirtualCores;
  int32_t numSuperBatches = ioBufDesc->numSuperBatches;
  int32_t numBatches = numVirtualCores * numSuperBatches;
  int32_t numIosPerCore;

  uint8_t * ptr;
  int32_t numElementsPerBatch;
  char tensorName[TIDL_MAX_ALG_IN_BUFS][TIDL_STRING_SIZE];

  if(isInput == 1)
  {
    ptr = (uint8_t *)onnxRtParams->inputTensorData[currTensorNum];
    status |= TIDL_ortGetType(onnxRtParams->inputTensorElementType[currTensorNum], &elementType, &elementSize);
    strcpy((char *)tensorName, (char *)onnxRtParams->inDataNames[currTensorNum]);
    numElementsPerBatch = ioBufDesc->inDIM1[currTensorNum] * ioBufDesc->inDIM2[currTensorNum] * ioBufDesc->inNumChannels[currTensorNum] * ioBufDesc->inHeight[currTensorNum] * ioBufDesc->inWidth[currTensorNum];
    numIosPerCore = ioBufDesc->numInputBuf / numBatches;
    /* Actual buffer size coming from OSRT*/
    bufferSize = 1;
    for (int32_t k = 0; k < TIDL_DIM_MAX; k++)
    {
      bufferSize *= onnxRtParams->tensorShape[currTensorNum][k];
    }
  }
  else
  {
    ptr = (uint8_t *)onnxRtParams->outputTensorData[currTensorNum];
    status |= TIDL_ortGetType(onnxRtParams->outputTensorElementType[currTensorNum], &elementType, &elementSize);
    strcpy((char *)tensorName, (char *)onnxRtParams->outDataNames[currTensorNum]);
    numElementsPerBatch = ioBufDesc->outDIM1[currTensorNum] * ioBufDesc->outDIM2[currTensorNum] * ioBufDesc->outNumChannels[currTensorNum] * ioBufDesc->outHeight[currTensorNum] * ioBufDesc->outWidth[currTensorNum];
    numIosPerCore = ioBufDesc->numOutputBuf / numBatches;
  }

  if(infer_ops->TIDLRT_isSharedMem(ptr) && isInfer)
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
      rtPtrs[idx]->layout = TIDLRT_LT_NCHW;
      rtPtrs[idx]->bufferSize = bufferSize; /* This is set only for input to handle TIDL-4466, output buffer size is kept -1 by default */
      if(isInput == 1)
      {
        for (int32_t l = 0; l < TIDL_DIM_MAX; l++)
        {
          rtPtrs[idx]->dimValues[l] = onnxRtParams->tensorShape[currTensorNum][l];
        }
        rtPtrs[idx]->padValues[0] = ioBufDesc->inPadL[currTensorNum];
        rtPtrs[idx]->padValues[1] = ioBufDesc->inPadR[currTensorNum];
        rtPtrs[idx]->padValues[2] = ioBufDesc->inPadT[currTensorNum];
        rtPtrs[idx]->padValues[3] = ioBufDesc->inPadB[currTensorNum];

        // Channel pitch
        rtPtrs[idx]->pitch[TIDL_CHANNEL_PITCH] = rtPtrs[idx]->dimValues[TIDL_DIM_WIDTH] * rtPtrs[idx]->dimValues[TIDL_DIM_HEIGHT];
      }

      strcpy((char *)rtPtrs[idx]->name, (char *)tensorName);
      rtPtrs[idx]->elementType = elementType;
      rtPtrs[idx]->ptr = ptr + l * numVirtualCores * numElementsPerBatch * elementSize + k * numElementsPerBatch * elementSize; /* Ptr must be start of each batch */
      if(memType == TIDLRT_MEM_SHARED)
      {
        rtPtrs[idx]->memType = TIDLRT_MEM_SHARED;
      }
    }
  }
  return status;
}

/** Set input and output TIDL RT tensor properties and call TIDL RT Invoke */
int32_t TIDL_subgraphRtInvoke(int32_t osrtDebugPrintLevel, OnnxTIDLSubGraphParams * subgraphParams, sTIDL_tidlRtDynamicLoading_t * infer_ops, int32_t isInfer)
{
  TIDL_osrtDebugPrint(osrtDebugPrintLevel, "*******   In TIDL_subgraphRtInvoke  ******** \n");
  int status = 0;
  int j = 0;
  onnxRtParams_t * onnxRtParams = &subgraphParams->onnxRtParams;
  void *handle = subgraphParams->tidlRtParams.rtHandle;
  sTIDLRT_PerfStats_t *stats = (sTIDLRT_PerfStats_t *)subgraphParams->tidlRtParams.stats;
  sTIDL_IOBufDesc_t * ioBufDesc = (sTIDL_IOBufDesc_t *)subgraphParams->tidlRtParams.ioBufDesc;

  sTIDLRT_Tensor_t *in[128];
  sTIDLRT_Tensor_t *out[128];
  sTIDLRT_Tensor_t *ins;
  sTIDLRT_Tensor_t *outs;
 
  ins = (sTIDLRT_Tensor_t *)subgraphParams->tidlRtParams.rtInList;
  outs = (sTIDLRT_Tensor_t *)subgraphParams->tidlRtParams.rtOutList;

  if ((ins == NULL) || (outs == NULL))
  {
    printf("Invoke  : ERROR: Unable to allocate memory for TIDL RT in[] out [] tensor struct\n");
    return -1;
  }
  else
  {
    int32_t currInIdx = 0;
    /* Input tesnsors property set up */
    for (j = 0; j < onnxRtParams->numNetInData; j++)
    {
      TIDL_setRtTensorParameters(ins, in, j, ioBufDesc, infer_ops, onnxRtParams, isInfer, 1);
    }
    /* Output tesnsors property set up */
    for (j = 0; j < onnxRtParams->numNetOutData; j++)
    {
      TIDL_setRtTensorParameters(outs, out, j, ioBufDesc, infer_ops, onnxRtParams, isInfer, 0);
    }
  }
  status = infer_ops->TIDLRT_invoke(handle, in, out);

  if(osrtDebugPrintLevel)
  {
    TIDL_printSubgraphStats(stats);
  }
  TIDL_osrtDebugPrint(osrtDebugPrintLevel, "*******  TIDL_subgraphRtInvoke done  ******** \n");
  return status;
}

int32_t getTidlRtOutShapeEnv()
{
    int32_t env;
    char *str;
    str = getenv("TIDL_RT_ONNX_VARDIM");
    if(!str)
    {
      env = 1;
    }
    else
    {
      env = atoi(str);
    }
    return env;
}

extern "C"
{
/** Find output shape for a particular ONNX RT output name using TIDL RT Io buf descriptor */ 
int32_t TIDL_getOutputShape(void * ioBufDescVPtr, int8_t onnxName[], std::vector<int64_t> &shape)
{
  int32_t status = 0;
  sTIDL_IOBufDesc_t *ioBufDescPtr = (sTIDL_IOBufDesc_t *)ioBufDescVPtr;
  std::vector<int64_t> nchw_shape;
  std::vector<int64_t> nchw_var_shape;
  int32_t varDim = getTidlRtOutShapeEnv();
  char onnxNameCrop[TIDL_STRING_SIZE];
  strcpy(onnxNameCrop, (char *)onnxName);
  if(ioBufDescPtr->inferenceMode == TIDL_inferenceModeLowLatency)
  {
    strcat(onnxNameCrop, "_crop_layer"); /* Low latency mode has known requirement for crop layer at the output which appends _crop_layer to the actual output name */
  }

  for(int i = 0; i < ioBufDescPtr->numOutputBuf; i++)
  {
    if((strcmp((char *)ioBufDescPtr->outDataName[i], (char *)onnxName) == 0) ||
        (strcmp((char *)ioBufDescPtr->outDataName[i], (char *)onnxNameCrop) == 0))
    {
      nchw_shape = { ioBufDescPtr->outNumBatches[i], ioBufDescPtr->outDIM1[i], ioBufDescPtr->outDIM2[i], ioBufDescPtr->outNumChannels[i], ioBufDescPtr->outHeight[i], ioBufDescPtr->outWidth[i]}; 
      if(ioBufDescPtr->inferenceMode == TIDL_inferenceModeHighThroughput)
      {
        /* Multi core throughput mode is treated as single batch from TIDL perspective, so outNumBatches = 1. numVirtualCores are used to indicate the number of such 
        TIDL instances to be run on different cores. numSuperBatches indicates number of times parallel inference needs to be run to infer all batches. 
        ONNX runtime requests dimensions from TIDL to allocate output buffers and get the corresponding ONNX runtime tensors,
        so memory needs to be allocated for the actual number of batches in model which is numSuperBatches * numVirtualCores */
        nchw_shape = { ioBufDescPtr->numSuperBatches * ioBufDescPtr->numVirtualCores, ioBufDescPtr->outDIM1[i], ioBufDescPtr->outDIM2[i], ioBufDescPtr->outNumChannels[i], ioBufDescPtr->outHeight[i], ioBufDescPtr->outWidth[i]};           
      }
      /*Slice based on valid number of output vectors:*/
      int32_t vectorOffset = TIDL_DIM_MAX - ioBufDescPtr->numValidTensorDims[i];
      if(nchw_shape[0] > 1)
      {
        /*TIDL batch dimension is not contigious */
        vectorOffset++;
        nchw_var_shape = std::vector<int64_t>(nchw_shape.begin() + vectorOffset, nchw_shape.end());
        nchw_var_shape.insert(nchw_var_shape.begin(), nchw_shape[0]);
      }
      else
      {
        nchw_var_shape = std::vector<int64_t>(nchw_shape.begin() + vectorOffset, nchw_shape.end());
      }
      
    }
  }

  if(varDim != 0)
  {
    shape = nchw_var_shape;
  }
  else
  {
    shape = nchw_shape;
  }
  
  if(shape.size() == 0)
  {
    // Should we error here?
    printf("Warning : Couldn't find corresponding ioBuf tensor for onnx tensor with matching name \n");
  }

  return status;
}

int32_t TIDLEP_getSubGraphStats(OnnxTIDLSubGraphParams * state_subGraph, char **node_name, void **node_data)
{
  sTIDLRT_PerfStats_t * stats = (sTIDLRT_PerfStats_t*)state_subGraph->tidlRtParams.stats;
  std::vector<uint64_t> *v = new std::vector<uint64_t>();
  v->push_back(uint64_t(stats->cpIn_time_start));
  v->push_back(uint64_t(stats->cpIn_time_end));
  v->push_back(uint64_t(stats->proc_time_start));
  v->push_back(uint64_t(stats->proc_time_end));
  v->push_back(uint64_t(stats->cpOut_time_start));
  v->push_back(uint64_t(stats->cpOut_time_end));
  *node_data = static_cast<void *>(v);
  *node_name = const_cast<char *>(state_subGraph->subGraphName_);
  return 0;
}

} //extern "C"