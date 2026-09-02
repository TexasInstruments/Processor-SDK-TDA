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

#include "tidl_constraint_utils.h"


int32_t tidlCheckShapeInferenceforOnnx(sTIDL_allowlistingMetaData md)
{
  /** Make sure all the required tensor dimensions are populated, else use open source runtimes offering to run this model
   * or run shape inference on this model before executing import **/
  std::vector<std::vector<int32_t>> vartensordims   = md.varTensorsDims;
  std::vector<std::vector<int32_t>> consttensordims = md.constTensorsDims;
  std::vector<std::vector<int32_t>> outtensordims   = md.outputTensorDims;

  if (md.numInputs == 0)
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  for (int i=0; i<md.varTensorIndices.size();i++)
  {
    if (vartensordims[i].size() == 0)
    {
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  for (int i=0; i<md.constTensorIndices.size();i++)
  {
    if (consttensordims[i].size() == 0)
    {
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  for (int i=0; i<outtensordims.size();i++)
  {
    if (outtensordims[i].size() == 0)
    {
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/*
 * Function to skipConstraintCheck 
 */
bool skipConstraintCheck(sTIDL_LayerPC_t layer)
{
  bool skip = false;

  /*
   * Skip if layer is induced by TIDL.
   * Currently there is no flag to check if layer is induced.
   * 
   * HACK: Use allowlistingMetadata.numVarInputs.
   * 
   * TODO: Add proper flag if layer is induced by TIDL
   */
  if ((layer.allowlistingMetaData.numVarInputs == 0) &&
      (layer.layerType != TIDL_DataLayer))
  {
    skip = true;
  }

  /*
   * The hack above skips slice if it has no variable input originally as well.
   */
  if(skip == true &&
     layer.layerType == TIDL_SliceLayer &&
     layer.optimized == 0)
  {
    skip = false;
  }

  return skip;
}

int32_t tidlGetNonSingletonNumDims(std::vector<int32_t> dims)
{
  if(dims.size() == 0)
  {
    return 0;
  }

  for(int i = 0; i < dims.size(); i++)
  {
      if(dims[i] != 1)
      {
          return (dims.size() - i);
      }
  }

  return 1;
}
