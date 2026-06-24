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


/* map TVM elem_type to TIDL element types:
    TVM element types:
    int32 -> 0
    float -> 2
    uint32 -> 1
*/
map<int,int>TVMtoTIDLTypesTable = {
{0, TIDL_SignedWord},
{1, TIDL_UnsignedWord},
{2, TIDL_SinglePrecFloat}
};

int32_t TidlParseTVM::getTIDLDataTypeFromTVMDataType(int32_t tvmDataType)
{
  int32_t tidlDataType = -1;

  if (TVMtoTIDLTypesTable.find(tvmDataType) != TVMtoTIDLTypesTable.end())
  {
    tidlDataType = TVMtoTIDLTypesTable[tvmDataType];
  }

  return tidlDataType;
}
// Helper function to convert int64 indices to int32
int32_t TidlParseTVM::convertIndicesToInt32(sBuffer_t& weights)
{
    int32_t *index_dst_ptr = (int32_t*)my_malloc(sizeof(int32_t) * weights.bufSize);
    if (index_dst_ptr == NULL)
    {
        TIDL_LOG_ERROR(gDiags.gDiagList, "ScatterNDLayer : Could not allocate memory for ScatterND constant indices");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    int64_t *index_src_ptr = (int64_t*)weights.ptr;
    int64_t val = 0;
    for (int32_t j = 0; j < weights.bufSize; j++)
    {
        val = index_src_ptr[j];
        index_dst_ptr[j] = (int32_t)val;
    }
    my_free(index_src_ptr);
    weights.ptr = (void*)index_dst_ptr;

    return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

/**
 * @brief Check if layer's input/output buffer counts are within supported limits
 *
 * This function validates that the number of input and output tensors for a layer
 * doesn't exceed TIDL's maximum supported buffer counts (TIDL_NUM_IN_BUFS and
 * TIDL_NUM_OUT_BUFS). This check is performed in the parser before allowlisting
 * constraints are evaluated, following the same pattern as ONNX RT flow.
 *
 * @param layer The layer structure containing allowlisting metadata
 * @return true if buffer counts are supported, false otherwise
 */
bool TidlParseTVM::isLayerIONumSupported(sTIDL_LayerPC_t &layer)
{
  bool isLayerSupported = true;
  sTIDL_allowlistingMetaData& md = layer.allowlistingMetaData;

  if (md.numInputs > TIDL_NUM_IN_BUFS)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported number of input tensors (> %d)", (char *)layer.name, TIDL_NUM_IN_BUFS);
    isLayerSupported = false;
  }
  else if (md.numOutputs > TIDL_NUM_OUT_BUFS)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported number of output tensors (> %d)", (char *)layer.name, TIDL_NUM_OUT_BUFS);
    isLayerSupported = false;
  }

  return isLayerSupported;
}
