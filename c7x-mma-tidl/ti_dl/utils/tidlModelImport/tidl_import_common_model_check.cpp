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
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <cmath>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <list>
#include <vector>

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#include "tidl_import_common.h"
#include "tidl_deviceInfo.h"
#include "tidl_import_common_tidlrt_osrt.h"
#include "tidl_import_diag.h"
#include "tidlAllowlistingConstraints/tidl_constraint.h"

#include "tidlAllowlistingConstraints/tidl_constraint.h"
#define __MODULE__ "[PARSER]"

#define TIDL_SIMD_WIDTH (64)
#define MODEL_CHECK_CONV_SUGGESTIONS 0
static bool is16bit = false;



void tidlModelCheckLayer(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags);

/* Returns 1 if constant tensor (first constant input) is:
 *    either scalar
 *    or vector with only one non-singleton dimension of same shape as channel dimension of variable input & is along channel dimension
 * else returns 0
 * Common across tflite RT and ONNX RT
*/
int32_t TIDL_canConvertToBatchNorm (sTIDL_LayerPC_t &layer)
{
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t nonSingletonDimNum = 0, constDim = 0;
  if (md.numConstInputs == 0)
  {
    return 0;
  }
  if (md.constTensorsDims[0].size() == 0)
  {
    if(gParams.debugTraceLevel > 0)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Constant tensor in node %s is scalar, operator will map to BatchNorm and value will be broadcasted to number of channels", layer.name);
    }
    return 1;
  }

  for(int i = 0; i < md.constTensorsDims[0].size(); i++)
  {
    if(md.constTensorsDims[0][i] != 1)
    {
      constDim = i;
      nonSingletonDimNum++;
    }
  }

  int32_t canConvertToBatchNorm = 0;

  /* Check if the constant tensor non-singleton dimension is along channel and is same as variable input's channel */
  if(nonSingletonDimNum == 0)
  {
    /* all dimensions are 1 -> only one value is there, it will be broadcasted to number of channels */
    canConvertToBatchNorm = 1;
  }
  else if(nonSingletonDimNum == 1)
  {
    int32_t numDims = md.varTensorsDims[0].size();
    if (numDims >= 3)
    {
      if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT)
      {
        /* For TFLITE (NHWC format), no need to check position of C in constant tensor */
        if(md.constTensorsDims[0][constDim] == md.varTensorsDims[0][numDims - 3])
        {
          canConvertToBatchNorm = 1;
        }
      }
      else
      {
        /* For ONNX (NCHW format), check the non-singular dimension is along channel dimension  */
        if(md.constTensorsDims[0].size() >= 3 && constDim == md.constTensorsDims[0].size() - 3)
        {
          if(md.constTensorsDims[0][constDim] == md.varTensorsDims[0][numDims - 3])
          {
            canConvertToBatchNorm = 1;
          }
        }
      }
    }
  }
  else
  {
    canConvertToBatchNorm = 0;
  }

  return canConvertToBatchNorm;
}


int32_t checkShapeInferenceforOnnx(sTIDL_allowlistingMetaData md)
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
  /* Const tensors carry their own data, so shape is always known — scalars
   * legitimately have dims.size() == 0 (0-D tensor) and must not be flagged. */
  for (int i=0; i<outtensordims.size();i++)
  {
    if (outtensordims[i].size() == 0)
    {
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_checkAddMulSubDivTensorProperties(const sTIDL_LayerPC_t &layerPC, TIDL_base_operator_t oper)
{
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;
  /** Conditions for allowlisting:
   * Necessary attributes : No attributes
   * Number of inputs : 2
   * Input type : both variable, "input 1" variable and "input 2" constant
   * Number of dimensions : Variable input - TIDL_DIM_MAX, Constant input - any number of dimensions but only one dimension can be non-1,
   * and non-1 dim size should match number of input channels of variable tensor
   */
  if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX)
  {
    int32_t status = checkShapeInferenceforOnnx(md);
    if (status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Add/Mul/Sub/Div with constant tensor requires input dimensions of that layer to be present as part of the network. Use open source runtimes offering to run this model or run shape inference on this model before executing import");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }
  if(md.numInputs != 2)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Add/Mul/Sub layer : Only 2 inputs supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
  {
    for(auto & varDims : md.varTensorsDims)
    {
      if(varDims.size() > TIDL_DIM_MAX)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList ,"Allowlisting : Add/Mul/Sub/Div layer : The variable inputs of layer must be less than %d", TIDL_DIM_MAX);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  if(md.varTensorIndices.size() == 2) //2 variable inputs
  {
    if ((oper == BASE_OP_ADD) || (oper == BASE_OP_MUL))
    {
      bool isEltwise = true;
      if (md.varTensorsDims[0].size() == md.varTensorsDims[1].size()) /* must be same dimensions */
      {
        int32_t heightDim = md.varTensorsDims[0].size() - 2;
        int32_t widthDim = md.varTensorsDims[0].size() - 1;

        for (int32_t idx = 0; idx < md.varTensorsDims[0].size(); idx++)
        {
          if (md.varTensorsDims[0][idx] != md.varTensorsDims[1][idx])
          {
            /*
            special case for allowing NCHW x NC11
            only possible mismatch is height & width both are mismatching and both are 1
            */
            /*Identify which tensor needs to be broadcasted*/
            int32_t broadCastTensorIdx = -1;
            if(md.varTensorsDims[0][idx] == 1)
            {
              broadCastTensorIdx = 0;
            }
            else if(md.varTensorsDims[1][idx] == 1)
            {
              broadCastTensorIdx = 1;
            }
            else
            {
              /*In this case tensor dimensions are genuinely mismatching*/
              broadCastTensorIdx = -1;
            }

            /*Firmware specific check to control allowlisting for broadcast mul (Available in Firmware > 09_02_06_00)*/
            if(!TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion, "09_02_06_00"))
            {
              isEltwise = false;
              break;
            }

            if( broadCastTensorIdx != -1)
            {
              /*Only broadcast on height & width is allowed with a strict format of NxCx1x1 for the tensor being broadcasted:*/
              if ((idx == heightDim )||(idx == widthDim))
              {
                if(md.varTensorsDims[broadCastTensorIdx][heightDim] == md.varTensorsDims[broadCastTensorIdx][widthDim] == 1)
                {
                  isEltwise = true;
                  break;
                }
                else
                {
                  isEltwise = false;
                  break;
                }
              }
              else
              {
                isEltwise = false;
              }
            }
            else
            {
              isEltwise = false;
            }
          }
        }
      }
      /* Not same number of dimensions */
      else
      {
        isEltwise = false;
      }

      if (!isEltwise)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Add/Mul/Sub/Div layer : The variable inputs must of be same dimensions or broadcast-able for c7x firmware version 09_02_07_00 or above");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    else if(oper == BASE_OP_DIV)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Div layer : both variable inputs not supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  else //addition with constant input
  {
      if(md.constTensorsDims[0].size() != 0)
      {
        //find non singleton dimensions in const tensor
        int32_t nonSingletonDimNum = 0;
        int32_t nonSingletonDimIdx = 0;
        for(int i = 0; i < md.constTensorsDims[0].size(); i++)
        {
          if(md.constTensorsDims[0][i] != 1)
          {
            nonSingletonDimNum++;
            nonSingletonDimIdx = i;
          }
        }
        //const tensor > 1D check
        if((nonSingletonDimNum > 1) ||  /* not batchnorm but broadcast cases */
          (
            /* even if there is only one non-singular dim in const -- it is not in channel */
            (nonSingletonDimNum == 1) && (nonSingletonDimIdx != (md.constTensorsDims[0].size()-3)) &&
            /* ONLY VALID IF NOT TFLITE - TFLITE CONVERSION DOES NOT WORK FOR LESS THAN 3 DIMS AND
            CONSIDERS LAST DIM AS WIDTH HERE EVEN THOUGH SHOULD GO TO BROADCAST, HENCE  STRICTLY NOT
            VALID CHECK FOR TFLITE */
            (
              (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE) &&
              (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) &&
              (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT)
            )
          )
        )
        {
          if (oper == BASE_OP_MUL || oper == BASE_OP_ADD)
          {
            bool isEltwise = false;
            if (md.constTensorsDims.size() == 1 && md.varTensorIndices.size() == 1) //eltwise mul between a variable tensor and constant tensor
            {
                int32_t varTensorNumDims    = md.varTensorsDims[0].size();
                int32_t constTensorNumDims  = md.constTensorsDims[0].size();

                /* Dimension matching to create a pattern */
                /**
                 * Store the status of comparison of dims in this array
                 * If same and non-singular       => 4
                 * If same and singular           => 3
                 * not same and one is singular   => 2
                 * not same and none is singular  => 1
                */
                int32_t dimMatchStatus[TIDL_DIM_MAX];
                /* Start dim indices from the last i.e., width */
                int32_t vIdx = varTensorNumDims-1, cIdx = constTensorNumDims-1;
                for (int32_t idx = TIDL_DIM_WIDTH; idx >= 0; idx--)
                {
                  /* reset comaprison status */
                  dimMatchStatus[idx] = 0;
                  /* check if tensor dim exists */
                  if ((vIdx >= 0) && (cIdx >= 0))
                  {
                    /* both same */
                    if (md.varTensorsDims[0][vIdx] == md.constTensorsDims[0][cIdx])
                    {
                      /* same and singular */
                      if (md.varTensorsDims[0][vIdx] == 1)
                      {
                        dimMatchStatus[idx] = 3;
                      }
                      /* same and not singular */
                      else
                      {
                        dimMatchStatus[idx] = 4;
                      }
                    }
                    /* not same */
                    else
                    {
                      /* not same and one of them is singular */
                      if ((md.varTensorsDims[0][vIdx] == 1) || (md.constTensorsDims[0][cIdx] == 1))
                      {
                        dimMatchStatus[idx] = 2;
                      }
                      /* not same and none is singular */
                      else
                      {
                        dimMatchStatus[idx] = 1;
                      }
                    }
                    vIdx--; cIdx--;
                  }
                  /* var tensor dim exists, const does not
                    this makes certain the corresponding const dim is 1
                  */
                  else if ((vIdx >= 0) and (cIdx < 0))
                  {
                    /* singluar and same */
                    if (md.varTensorsDims[0][vIdx] == 1)
                    {
                      dimMatchStatus[idx] = 3;
                    }
                    /* one is singular and not same */
                    else
                    {
                      dimMatchStatus[idx] = 2;
                    }
                    vIdx--;
                  }
                  else if ((vIdx < 0) and (cIdx >= 0))
                  {
                    /* singluar and same */
                    if (md.constTensorsDims[0][cIdx] == 1)
                    {
                      dimMatchStatus[idx] = 3;
                    }
                    /* one is singular and not same */
                    else
                    {
                      dimMatchStatus[idx] = 2;
                    }
                    cIdx--;
                  }
                  /* must be 1 */
                  else
                  {
                    /* same and singular */
                    dimMatchStatus[idx] = 3;
                  }
                }


                int32_t matchConsolidatedResult = 0;
                for(int32_t idx = 0; idx < TIDL_DIM_MAX; idx++)
                {
                  matchConsolidatedResult = matchConsolidatedResult*10 + dimMatchStatus[idx];
                }
                /**
                * Supported special patterns
                * 1x1x1x1x1xW   &   1x1x1x1xHxW    ===> 333324
                * 1x1x1x1x1xW   &   1x1x1xCxHxW    ===> 333224
                * 1x1x1x1x1xW   &   1x1xD2xCxHxW   ===> 332224
                *
                * 1x1x1x1xHxW   &   1x1x1xCxHxW    ===> 333244
                * 1x1x1x1xHxW   &   1x1xD2xCxHxW   ===> 332244
                *
                * 1x1x1xCxHxW   &   1x1xD2xCxHxW   ===> 332444
                *
                * 1x1xD2x1xHxW  &   1x1xD2xCxHxW   ===> 334244
                *
                * 1x1x1x1xHxW   &   1x1x1x1xHx1    ===> 333342
                 */
                /**
                 * TODO: conflict if batch is there with last 2 patterns
                 * for e.g. -> 3X3X224 && 4x3x3x224
                */

                std::vector<int32_t> suuportedDimCmp{333324, 333224, 332224, 333234,
                                                    333244, 332244,
                                                    332444,
                                                    334244,
                                                    333342
                                                    };

                for (const int32_t& j: suuportedDimCmp)
                {
                  if (j == matchConsolidatedResult)
                  {
                    isEltwise = true;
                    break;
                  }
                  else
                  {

                  }
                }

                /* Similar dimension are by default supported
                to check if similar dimension it is sufficient to check
                if all elements in match status are only 3 or 4
                */
                if (!isEltwise) /* Check only if earlier broadcast checks fail*/
                {
                  int32_t isAllDimSame = 0;
                  for (int32_t idx = 0; idx < TIDL_DIM_MAX; idx++)
                  {
                    if ((dimMatchStatus[idx] == 3) || (dimMatchStatus[idx] == 4))
                    {
                      isAllDimSame += 0;  /* Added 0 when a 3 or 4 is found*/
                    }
                    else
                    {
                      isAllDimSame += 1; /* Becomes non zero whenever something else is encountered*/
                    }
                  }

                  if (isAllDimSame == 0)
                  {
                    isEltwise = true;
                  }
                }
            }

            if(!isEltwise)
            {
              TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Add/Mul layer : Eltwise operator is supported only with operands of similar dimensions or broadcast supported patterns of both inputs");
              return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
          }
          else
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Sub/Div layer : Constant tensor must be a number or vector, only one dimension can be > 1");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
          }
        }
        else
        {
          if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
          {
            /** Const input is either single value or 1D tensor
             * If single value, always supported
             * If 1D tensor -> must be same as channel of var input
             */
            int32_t varTensorNumDim = md.varTensorsDims[0].size();
            int32_t varTensorChDim = varTensorNumDim - 3;
            if((nonSingletonDimNum == 1)  &&          /* 1D tensor */
              (md.constTensorsDims[0][nonSingletonDimIdx] != md.varTensorsDims[0][varTensorChDim]) /* shape not same as channel of var input */
            )
            {
              TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Add/Mul/Sub/Div layer : 1D vector dimension doesn't match with channel or width dimension, unsupported broadcast");
              return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
          }
        }
      }
    }
  return 0;
}

/** Common across tflite RT and ONNX RT
 * ONNX RT Layer -- Gemm
 * Tflite RT Layer -- FullyConnected
 */
int32_t TIDL_checkInnerProductTensorProperties(const sTIDL_LayerPC_t &layerPC)
{
  /** Conditions for allowlisting:
   * Necessary attributes : kernel_shape, strides, pads
   * Number of inputs : 2 or 3
   * Input type : Only 1 variable, others constant
   * Number of dimensions : 2
   */

  /** Check for variable input dimensions
   * Gemm layer definition indicates variable input dimension must be 2, so exported layer itself will be 2 dimensional (M, K) ---> (1, K) for TIDL
   * Weight tensor as per definition should be (K, N)
   * Bias tensor should be broadcastable to (M, N)
   * Output is (M, N)
   */

  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;

  //variable input checks

  int32_t varIdx = md.varTensorIndices[0]; //Only one variable input

  if((md.constTensorIndices.size() == 1) || (md.constTensorIndices.size() == 2)) //filter, bias
  {
    int32_t filter = 0;
    int32_t bias = 1;
    if((md.constTensorsDims[filter].size() < 2) )
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : INNER PRODUCT layer : Filter tensor input should have atleast 2 dimensions, observed %d", md.constTensorsDims[0].size());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
    {
      int32_t filterNumDim = md.constTensorsDims[filter].size();
      int32_t varTensorNumDim = md.varTensorsDims[0].size();
      bool isIPLayer = true;

      if (filterNumDim > 2 && varTensorNumDim > 2)
      {
        int32_t filterChDim = filterNumDim - 3;
        int32_t varTensorChDim = varTensorNumDim - 3;
        /* Both filter and varTensor have channel dimensions*/
        if (md.constTensorsDims[filter][filterChDim] != md.varTensorsDims[0][varTensorChDim]) /* Not same channel */
        {
          /* one of them must be 1*/
          if ((md.constTensorsDims[filter][filterChDim] == 1) || (md.varTensorsDims[0][varTensorChDim] == 1))
          {
            /* okay */
          }
          else
          {
            isIPLayer = false;
          }
        }
        else
        {
          /* same channel dim, okay */
        }

      }
      if (varTensorNumDim > 3)
      {
        int32_t vardim1 = varTensorNumDim - 4;
        int32_t filterdim1 = filterNumDim - 4;

        if (md.varTensorsDims[0][vardim1] != 1)
        {
          if (filterNumDim < 4 || md.constTensorsDims[0][filterdim1] != md.varTensorsDims[0][filterdim1])
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Inner Product layer : Unsupported broadcast with filter");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
          }
        }
      }

      if (!isIPLayer)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Inner Product layer : Mismatch in filter dimension and input dimension -- Filter dimension = [%d %d],  input dimensions = [%d %d]", md.constTensorsDims[filter][0], md.constTensorsDims[filter][1], md.varTensorsDims[0][0], md.varTensorsDims[0][1]);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }

    if(md.constTensorIndices.size() == 2) //both weight and bias present
    {
      //Bias tensor will have number of dimensions either 1 or 2 as per Gemm layer definition
      if(md.constTensorsDims[bias].size() == 2)
      {
        if((md.constTensorsDims[bias][0] != 1) || (md.constTensorsDims[bias][1] != md.constTensorsDims[filter][0]))
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Inner Product layer : Bias tensor input should be a vector (1, N) and N should match output dimension");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else if(md.constTensorsDims[bias].size() == 1)
      {
        if(md.constTensorsDims[bias][0] != md.constTensorsDims[filter][0])
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Inner Product layer : Bias tensor input should be a vector (1, N) and N should match output dimension");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Inner Product layer : Dimension of bias vector can either be [1, N] or [N]");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  return 0;
}
int32_t TIDL_checkGatherTensorProperties(const sTIDL_LayerPC_t &layerPC)
{
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;
  int32_t indiceDim = md.varTensorsDims[1].size();
  int32_t inDataDim = md.varTensorsDims[0].size();

  int32_t status = 0, i1;

  const sTIDL_GatherLayerParams_t& params = layerPC.layerParams.gatherParams;
  int32_t axis = params.axis;

  /**
   * Only line/vector gather is supported
   * Input data should be 2D (HxW or 1xHxW or 1x1xHxW etc.)
   * Indice data should be 1D (W or 1xW or 1x1xW etc)
  */

  if (inDataDim < 2)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gather layer : Input data dim is <=1, unsupported configuration for gather layer");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  else if (inDataDim > 2)
  {
    for (i1 = inDataDim - 3; i1 > -1; i1--)
    {
      if (md.varTensorsDims[0][i1] != 1)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gather layer : Unsupported gather configuration, Channel & higher dims for indata should be 1");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  /**
   * Axis check
  */
  if (axis != TIDL_DIM_HEIGHT)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gather layer : Unsupported axis configuration for gather layer, only line gather is supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  return status;
}
/** Common across tflite RT and ONNX RT
 * ONNX RT Layer -- Reshape
 * Tflite RT Layer -- kTfLiteBuiltinReshape
 */
int32_t TIDL_checkReshapeTensorProperties(const sTIDL_LayerPC_t &layerPC)
{
  /** Conditions for allowlisting:
   * Necessary attributes : NA
   * Input type : variable
   * Number of dimensions : TIDL_DIM_MAX
   */
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;
  int32_t status = 0;
  return status;
}

/**
 * ONNX RT Layer -- Transpose
 */
int32_t TIDL_checkTransposeTensorProperties(const sTIDL_LayerPC_t &layerPC)
{
  /** Conditions for allowlisting:
   * Necessary attributes : NA
   * Input type : variable
   * Number of dimensions : TIDL_DIM_MAX
   */
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;
  int32_t shapeInferencestatus = checkShapeInferenceforOnnx(md);
  int32_t status = 0, nonZeroHigherDims = 0;

  int32_t *perm  = (int32_t*)layerPC.weights.ptr;
  int32_t numDim = layerPC.weights.bufSize;

  if (shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL && perm != NULL)
  {
    if(numDim > 4)
    {
      for (int i = numDim - 5; i>= 0; i--)
      {
        if (md.varTensorsDims[0][i] != 1)
        {
          nonZeroHigherDims++;
        }
      }
      if (perm[numDim -1] != numDim - 1 && nonZeroHigherDims != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Transpose layer : Unsupported transpose, only permutes (last dim not changing) are supported when numDim > 4");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  return status;
}

/** Common across tflite RT and ONNX RT
 * ONNX RT Layer -- MaxPool, AveragePool, GlobalAveragePool
 * Tflite RT Layer -- kTfLiteBuiltinAveragePool2d, kTfLiteBuiltinMaxPool2d
 */
int32_t TIDL_checkPoolingTensorProperties(const sTIDL_LayerPC_t &layerPC)
{
  /** Conditions for allowlisting:
   * Necessary attributes : kernel_shape, strides, pads
   * Number of inputs : 1
   * Input type : variable
   * Number of dimensions : 4
   */
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;

  if(md.numInputs != 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pooling layer : Only one input supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(md.numVarInputs != 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pooling layer : Input should be variable and not constant/initializer");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
  {
    if(md.varTensorsDims[0].size() > TIDL_DIM_MAX)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pooling layer : Number of input dimensions must be less than %d", TIDL_DIM_MAX);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}


/** Common across tflite RT and ONNX RT
 * ONNX RT Layer -- Slice
 * Tflite RT Layer -- Slice, Strided Slice
 */
int32_t TIDL_checkSliceTensorProperties(const sTIDL_LayerPC_t &layerPC)
{
  int32_t status = TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
  /** Conditions for allowlisting:
   * Necessary attributes :
   * Number of inputs : 4
   * Input type : variable
   * Number of dimensions :
   */
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;
  if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))
  {
    if(md.numInputs != 4)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Must have 4 inputs supported, got %d", md.numInputs);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if(md.numVarInputs != 1)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Only one input should be variable - not constant/initializer");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if(md.numConstInputs != 3)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Layer should have 3 constant/initializer inputs, got %d", md.numConstInputs);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if(md.varTensorsDims[0].size() != 4)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Number of dimensions for variable inputs must be 4, found %d", md.varTensorsDims[0].size());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    for(int i = 0; i < md.constTensorsDims.size(); i++)
    {
      if(md.constTensorsDims[i].size() != 1)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Number of dimensions for constant/initializer inputs must be 1, found %d for constant input %d",
        md.constTensorsDims[i].size(), i);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if(md.constTensorsDims[i][0] != 4)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Constant/initializer inputs must have 4 values, got %d", md.constTensorsDims[i][0]);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    if(md.outputTensorDims[0].size() != 4)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Number of dimensions for output tensor must be 4, got %d", md.outputTensorDims[0].size());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    if(md.varTensorsDims[0][0] != 1)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Only batch size = 1 supported, got batch size %d", md.varTensorsDims[0][0]);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  return status;
}



static void checkConvLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  int32_t i2, i3, i4;

  bool notOptimal_input64align = false;
  bool notOptimal_output64align = false;
  bool validated = false;
  bool patch_embedding_pattern = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  const sTIDL_ConvParams_t& params = layerPC.layerParams.convParams;

  /*
   * Validated combinations
   */
  int32_t validatedParams[][8] =
  {
    // -1 means dont care
    // kernelH kernelW strideH strideW dilationH dilationW padH padW
    {        1,      1,      1,      1,       -1,       -1,  -1,  -1},
    {        1,      1,      2,      2,       -1,       -1,  -1,  -1},
    {        2,      2,      1,      1,       -1,       -1,  -1,  -1},
    {        2,      2,      2,      2,        1,        1,  -1,  -1},
    {        3,      3,      1,      1,       -1,       -1,  -1,  -1},
    {        3,      3,      2,      2,        1,        1,  -1,  -1},
    {        3,      3,      3,      3,        1,        1,  -1,  -1},
    {        5,      5,      1,      1,       -1,       -1,  -1,  -1},
    {        5,      5,      2,      2,        1,        1,  -1,  -1},
    {        7,      7,      1,      1,       -1,       -1,  -1,  -1},
    {        7,      7,      2,      2,        1,        1,  -1,  -1},
    {        1,      3,      1,      1,       -1,       -1,  -1,  -1},
    {        3,      1,      1,      1,       -1,       -1,  -1,  -1},
    {        1,      5,      1,      1,       -1,       -1,  -1,  -1},
    {        5,      1,      1,      1,       -1,       -1,  -1,  -1},
    {        1,      7,      1,      1,       -1,       -1,  -1,  -1},
    {        7,      1,      1,      1,       -1,       -1,  -1,  -1},
  };

  for(i2 = 0; i2 < sizeof(validatedParams)/8/sizeof(int32_t); i2++)
  {
    if((validatedParams[i2][0] == -1 || params.kernelH == validatedParams[i2][0]) && (validatedParams[i2][1] == -1 || params.kernelW == validatedParams[i2][1]))
    {
      validated = true;
    }
    else
    {
      validated = false;
      continue;
    }

    if((validatedParams[i2][2] == -1 || params.strideH == validatedParams[i2][2]) && (validatedParams[i2][3] == -1 || params.strideW == validatedParams[i2][3]))
    {
      validated = true;
    }
    else
    {
      validated = false;
      continue;
    }

    if((validatedParams[i2][4] == -1 || params.dilationH == validatedParams[i2][4]) && (validatedParams[i2][5] == -1 || params.dilationW == validatedParams[i2][5]))
    {
      validated = true;
    }
    else
    {
      validated = false;
      continue;
    }

    if((validatedParams[i2][6] == -1 || params.padH == validatedParams[i2][6]) && (validatedParams[i2][7] == -1 || params.padW == validatedParams[i2][7]))
    {
      validated = true;
    }
    else
    {
      validated = false;
      continue;
    }

    if(validated)
      break;
  }

  /* Depthwise separable convolution */
  if (((params.numGroups == params.numInChannels) || (params.numInChannels ==1)) &&
      (params.numGroups == params.numOutChannels))
  {
    // kernelH kernelW strideH strideW dilationH dilationW padH padW
    #define DEPTHWISE_SUPPORTED_PARAMS (4)
    int32_t supportedConfigs[][DEPTHWISE_SUPPORTED_PARAMS] =
    {
      /*kernelW, kernelH, strideW, strideH*/
      {   3,          3,             1,           1},
      {   3,          3,             2,           2},
      {   5,          5,             1,           1},
      {   7,          7,             1,           1},
    };

    for(i2 = 0; i2 < sizeof(validatedParams)/(DEPTHWISE_SUPPORTED_PARAMS *sizeof(int32_t)); i2++)
    {
      if ( ( params.kernelW == supportedConfigs[i2][0] ) &&
	( params.kernelH == supportedConfigs[i2][1] ) &&
	( params.strideW == supportedConfigs[i2][2] ) &&
	( params.strideH == supportedConfigs[i2][3] ) )
      {
	/* Only these combinations are supported */
	validated = true;
	break;
      }
      else
      {
	validated = false;
      }
    }

    if ( validated!= true)
    {
      /* Any other combination is not supported */
      fatalError = true;
      TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: Depthwise convolution layer is either not supported or doesn't have optimized implementation available", layerPC.name);
    }

    if(layerPC.layerType == TIDL_ConvolutionLayer )
    {
      /* MMALIB Bug MMALIB-566*/
      int32_t simd_width=-1;
      if((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A)
      {
        simd_width = 32U;
      }
      else
      {
        simd_width = 64U;
      }
      //check for 16-bit
      if( gParams.numFeatureBits > 8 )
      {
        simd_width = simd_width/2;
      }
      if(params.padW > simd_width)
      {
        fatalError = true;
        TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: Dilation greater than the MMA width is not supported", layerPC.name);
      }
    }
  }
  else if((layerPC.layerType == TIDL_ConvolutionLayer) && ((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A))
  {
    bool isSupportedPad = true;
    if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT))
    {
      if(params.padW > layerPC.allowlistingMetaData.varTensorsDims[0][layerPC.allowlistingMetaData.varTensorsDims[0].size()-1])
      {
        isSupportedPad = false;
      }
    }
    else if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
    {
      isSupportedPad = true; /* Add check after ensuring dimValues are available in TVM */
    }
    else /* TIDL-RT */
    {
      if(params.padW > layerPC.inData[0].dimValues[TIDL_DIM_WIDTH]) /* inData is not populated before allow-listing call in OSRT */
      {
        isSupportedPad = false;
      }
    }

    if(isSupportedPad == false)
    {
      fatalError = true;
      TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: Padding greater than input width is not supported", layerPC.name);
    }
  }


  /*
    Convolution which can be reduced to supported Patch embedding format
  */
  if (
        (validated == false) && // check if previous checks fails
        // check if satisfy: N X N stride N
        (params.kernelH == params.kernelW) &&   // N X N
        (params.strideH == params.strideW) &&   // same stride in both dimensions
        (params.kernelH == params.strideH) &&   // stride N
        (params.kernelH >= 3)                   // N > 3
     )
  {
    validated = true;
    patch_embedding_pattern = true;
  }


  /*
   * Not supported
   */
  if(params.kernelH == 11 && params.kernelW == 11 &&
     params.strideH == 4 && params.strideW == 4)
  {
    warning = true;
    TIDL_LOG_NOT_VERIFIED(diags, "Convolution layer %s: kernel size 11x11 with stride 4 has gone through limited verification",
       layerPC.name);
  }

  if(params.kernelH > 7  && params.kernelW > 7 &&
     params.strideH == 2 && params.strideW == 2)
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: kernel size larger than 7 with stride 2 is not supported", layerPC.name);
  }

  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE)
  {
    if((params.numGroups > params.numInChannels) || (params.numGroups > params.numOutChannels))
    {
      fatalError = true;
      TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: numGroups is greater than In or Out channels", layerPC.name);
    }
  }

  if(params.kernelH != 11 && params.kernelW != 11 &&
     params.strideH == 4 && params.strideW == 4   &&
     (!patch_embedding_pattern)     // support by reducing to stride 1 makes this check irrelevant
    )
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: stride 4 only supports kernel size 11x11", layerPC.name);
  }

  /*Only for AM62A*/
  if(params.kernelH == 3 && params.kernelW == 3 &&
     params.strideH == 3 && params.strideW == 3 &&
     layerPC.layerType == TIDL_ConvolutionLayer &&
     ((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A))
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: Kernel size 3x3 is not supported with stride 3", layerPC.name);
  }

  if(params.strideH != params.strideW)
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "Convolution layer %s: strideH != strideW --- only symmetric stride supported", layerPC.name);
  }


  /*
   * Performance Suggestion
   */
  if((params.kernelH * params.kernelW * layerPC.inData[0].dimValues[TIDL_DIM_NUMCH] / params.numGroups + params.enableBias) % 64)
  {
    notOptimal = true;
    notOptimal_input64align = true;
  }
  if(layerPC.outData[0].dimValues[TIDL_DIM_NUMCH] / params.numGroups % 64)
  {
    notOptimal = true;
    notOptimal_output64align = true;
  }

  /*
   * Conclusion
   */
  if(!validated && !fatalError)
  {
    TIDL_LOG_NOT_VERIFIED(diags, "Convolution layer %s: Layer parameter combination has undergone limited validation and "
       "may have some issues. Following are the parameters: Kernel %dx%d, Stride %dx%d, dilation %dx%d, Pad %dx%d, Bias %d",
       layerPC.name,
       params.kernelH,   params.kernelW,
       params.strideH,   params.strideW,
       params.dilationH, params.dilationW,
       params.padH,      params.padW,
       params.enableBias);
  }

#if MODEL_CHECK_CONV_SUGGESTIONS
  if(notOptimal_input64align)
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_ConvolutionLayer "
       "%s: params.kernelH * params.kernelW * layer.inData[0].dimValues[TIDL_DIM_NUMCH] / params.numGroups "
       "is not 64 aligned, the performance is not efficient on MMA",
       layerPC.name);

  if(notOptimal_output64align)
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_ConvolutionLayer "
       "%s: layer.outData[0].dimValues[TIDL_DIM_NUMCH] / params.numGroups "
       "is not 64 aligned, the performance is not efficient on MMA",
       layerPC.name);
#endif
}


static void checkPoolingLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_PoolingParams_t& params = layerPC.layerParams.poolParams;
  sTIDL_allowlistingMetaData md  = layerPC.allowlistingMetaData;
  int32_t shapeInferencestatus = checkShapeInferenceforOnnx(md);
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  /*
   * Validated combinations
   */
  if(params.kernelH == 0 && params.kernelW == 0)
  {
    validated = true; // Global pooling
    if (shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      int32_t size = md.varTensorsDims[0].size(), planeSize = 1;
      /**
       * Following the generic pattern for global average pool input tensor,
       * N X C x D1 x D2 x ....
       * Average is calculated over D1, D2, .. and so on
       * For image case -> N x C x H x W, average is calculated over H & W
       * You skip batch & channel dimensions (0,1) & calculate planeSize from dimIdx 2 onwards
      */

      if (size >= 3)
      {
        for (int32_t i = 2; i < size; i++)
        {
          planeSize *= md.varTensorsDims[0][i];
        }
      }

      if (planeSize > 1024)
      {
        validated = false;
      }
    }
  }

  if(params.kernelH == 2 && params.kernelW == 2 &&
     params.strideH == 2 && params.strideW == 2)
  {
    validated = true;
  }

  if(params.kernelH == 1 && params.kernelW == 1 &&
     ((params.strideH == 2 && params.strideW == 2) || (params.strideH == 1 && params.strideW == 1 )) &&
     params.poolingType == TIDL_MaxPooling)
  {
    validated = true;
  }

  if(params.kernelH == 3 && params.kernelW == 3 &&
     ((params.strideH == 1 && params.strideW == 1) || (params.strideH == 2 && params.strideW == 2)) &&
     params.padH    == 1 && params.padW    == 1)
  {
    validated = true;
  }


  if(params.kernelH == 3 && params.kernelW == 3 &&
     params.strideH == 2 && params.strideW == 2 &&
     params.padH    == 0 && params.padW    == 0)
  {
    validated = true;
  }

  if(params.kernelH == 1 && params.kernelW == 1)
  {
    if((params.strideH == 2 && params.strideW == 1) || (params.strideH == 1 && params.strideW == 2)
    || (params.strideH == 3 && params.strideW == 1) || (params.strideH == 1 && params.strideW == 3))
    {
      validated = true;
    }
  }

  if(params.kernelH == 1 && params.kernelW == 2 && params.strideH == 1 && params.strideW == 2)
  {
    validated = true;
  }
  else if(params.kernelH == 1 && params.kernelW == 3 && params.strideH == 1 && params.strideW == 3)
  {
    validated = true;
  }
  else if(params.kernelH == 2 && params.kernelW == 1 && params.strideH == 2 && params.strideW == 1)
  {
    validated = true;
  }
  else if(params.kernelH == 3 && params.kernelW == 1 && params.strideH == 3 && params.strideW == 1)
  {
    validated = true;
  }

  if(is16bit && params.poolingType == TIDL_MaxPooling)
  {
    validated = true;
  }


  /*
   * Performance Suggestion
   */
  if(params.kernelH == 4 && params.kernelW == 4)
  {
    notOptimal = true;
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_PoolingLayer "
       "%s: 4x4 pooling is not efficient on MMA", layerPC.name);
  }

  if (params.kernelH == 0 && params.kernelW == 0 && !validated)
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_PoolingLayer '%s': Plane size (%dx%d) > 1024 not supported for global average pooling, consider using our ONNX optimizer tool to replace this layer with a supported layer\
       https://github.com/TexasInstruments/edgeai-tidl-tools/tree/master/scripts/osrt_model_tools/onnx_tools/tidl-onnx-model-optimizer",
       layerPC.name, params.kernelH, params.kernelW);
  }

  else if (!validated)
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_PoolingLayer '%s': "
       "kernel size %dx%d with stride %dx%d not supported",
       layerPC.name, params.kernelH, params.kernelW, params.strideH, params.strideW);
}


static void checkBatchNormLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_BatchNormParams_t& params = layerPC.layerParams.batchNormParams;
  bool notOptimal = false;

  /*
   * Performance Suggestion
   */
  if((is16bit) && (layerPC.actParams.actType != TIDL_Sigmoid) && (layerPC.actParams.actType != TIDL_Tanh)
        && (layerPC.actParams.actType != TIDL_HardSigmoid) && (layerPC.actParams.actType != TIDL_ELU)
        && (layerPC.actParams.actType != TIDL_GELU))
  {
    notOptimal = true;
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_BatchNormLayer "
       "%s: 16 bits is not optimal in this release", layerPC.name);
  }
}


static void checkInnerProductLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_InnerProductParams_t& params = layerPC.layerParams.innerProductParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;
  int32_t inNodesA = (params.inputATranspose == 0) ? layerPC.inData[0].dimValues[3] : layerPC.inData[0].dimValues[2];
  int32_t inNodesB = (params.inputBTranspose == 0) ? layerPC.inData[1].dimValues[2] : layerPC.inData[1].dimValues[3];
  /*
   * Not supported
   */
  if(inNodesA  !=  inNodesB)
  {
    /*
    fatalError = true;
    diags.emplace_back(TIDL_ModelDiagnostic::DK_NotSupported, "[TIDL_InnerProductLayer] "
        "%s input shape of A and B is not suitable for matrix multiplication", layerPC.name);
    */
  }

  /*
   * Performance Suggestion
   */
  if(layerPC.inData[0].dimValues[TIDL_DIM_WIDTH ] * layerPC.outData[0].dimValues[TIDL_DIM_WIDTH ] > 2048 * 2048)
  {
    notOptimal = true;
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_InnerProductLayer "
       "%s: Size larger than 2048 * 2048 is not optimal", layerPC.name);
  }
}


static void checkDeconvLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ConvParams_t& params = layerPC.layerParams.convParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  /*
   * Validated combinations
   */
  if(params.kernelH == 4 && params.kernelW == 4 &&
     params.strideH == 2 && params.strideW == 2 &&
     params.padW == 1 && params.padH == 1)
  {
    validated = true;
  }
  else if(params.kernelH == 3 && params.kernelW == 3 &&
     params.strideH == 2 && params.strideW == 2 &&
     params.padW == 1 && params.padH == 1)
  {
    validated = true;
  }
  else if (params.kernelH == 2 && params.kernelW == 2 &&
     params.strideH == 2 && params.strideW == 2 &&
     params.padW == 0 && params.padH == 0)
  {
    validated = true;
  }
  else
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_Deconv2DLayer %s: not supported with params: kernel %dx%d, stride %dx%d, padH = %d, padW = %d", layerPC.name, params.kernelH, params.kernelW, params.strideH, params.strideW, params.padH, params.padW);
  }

  /*
   * Performance Suggestion & Limited Testing
   */
  if (params.kernelH == 7 && params.kernelW == 7 &&
      params.strideH == 2 && params.strideW == 2)
  {
    TIDL_LOG_NOT_VERIFIED (diags, "TIDL_Deconv2DLayer %s: with params: kernel %dx%d, stride %dx%d, has gone through limited testing, please run this layer on CPU (deny) if you face any issues", layerPC.name, params.kernelH, params.kernelW, params.strideH, params.strideW);
  }

  TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_Deconv2DLayer %s:"
  " Change to Upsample/Resize if possible. Upsample/Resize will be more efficient", layerPC.name);
}

static void checkDetectionOutLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_DetectOutputParams_t& params = layerPC.layerParams.detectOutParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

}


static void checkShuffleChannelLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ShuffleLayerParams_t& params = layerPC.layerParams.shuffleLayerParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  /*
   * Performance Suggestion
   */
  notOptimal = true;
  TIDL_LOG_NOT_PERFORMANT(diags, "[TIDL_ShuffleChannelLayer] "
      "%s ShuffleChannel layer is not optimal in this version", layerPC.name);
}

static void checkDepthToSpaceLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ShuffleLayerParams_t& params = layerPC.layerParams.shuffleLayerParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  int32_t blockSize = layerPC.layerParams.depthToSpaceParams.blockSize;
  /*
   * Validated combinations
   */
  if(blockSize == 8 || blockSize == 4 || blockSize == 2)
  {
    validated = true;
  }
  else
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_DepthToSpaceLayer "
       "%s: is not supported", layerPC.name);
  }

  /*
   * Performance Suggestion
   */
  notOptimal = true;
  TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_DepthToSpaceLayer "
      "%s: Standalone DepthToSpace layer is not optimal in this version, it is optimal if it is next to 1x1 convolutions", layerPC.name);
}

static void checkBatchToSpaceLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  fatalError = true;
  if((gParams.modelType == 0) || (gParams.modelType == 1) || (gParams.modelType == 2) || (gParams.modelType == 3))
  TIDL_LOG_UNSUPPORTED(diags, "TIDL_BatchToSpaceLayer/TIDL_SpaceToBatchLayer "
      "%s: marked supported only for runtimes(to enable dispatch to TIDL), should be already merged for standalone TIDL import check", layerPC.name);
}

static void checkColorConversionLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  return;
}

static void checkResizeLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ResizeLayerParams_t& params = layerPC.layerParams.resizeParams;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  /*
   * Validated combinations
   */

  if ( params.resizeRatio[TIDL_DIM_WIDTH] == params.resizeRatio[TIDL_DIM_HEIGHT] )
  {
    if (( params.resizeRatio[TIDL_DIM_WIDTH] == 2.0 ) ||
      (params.resizeRatio[TIDL_DIM_WIDTH] == 4.0 ))
    {
      TIDL_LOG_INFO(diags, "TIDL_ResizeLayer "
         "%s: Any resize ratio which is power of 2 and greater than 4 will be placed by combination of 4x4 resize layer and 2x2 resize layer. "
         "For example a 8x8 resize will be replaced by 4x4 resize followed by 2x2 resize", layerPC.name);
    }
    else
    {
      warning = 1;
      TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_ResizeLayer "
         "%s: Resize kernel with non-power of 2 resize ratio is not optimal", layerPC.name);
    }
  }
  else
  {
    warning = 1;
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_ResizeLayer "
       "%s: Resize layer with non-symmetric resize ratio across width and height is not optimal", layerPC.name);
  }
  if(params.mode != TIDL_ResizeNearest && params.mode != TIDL_ResizeBilinear)
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_ResizeLayer "
       "%s: Resize layer mode must be either nearest_neighbor or bilinear", layerPC.name);
  }
}


static void checkPriorBoxLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  TIDL_LOG_UNSUPPORTED(diags, "TIDL_PriorBoxLayer "
     "%s: should be removed in import process. Make sure you have detection out layer in the model. "
     "If not, this model will not work", layerPC.name);
}


static void checkPermuteLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  TIDL_LOG_UNSUPPORTED(diags, "TIDL_PermuteLayer "
     "%s: should be removed in import process. Make sure you have detection out layer in the model. "
     "If not, this model will not work", layerPC.name);
}


static void checkEltwiseLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_EltWiseParams_t& params = layerPC.layerParams.eltWiseParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

#ifdef TIDL_EN_MULTI_TENSOR_ELT_WISE
  /* curently using (layer.inData[0].elementType >> 1)+1 instead of TIDL_getDatElementSize */
  if(((layerPC.numInBufs > 2) && (params.eltWiseType == TIDL_EltWiseProduct)) ||
     ((layerPC.numInBufs > ((TIDL_SIMD_WIDTH>>1)/((layerPC.inData[0].elementType >> 1) + 1)))
      && (params.eltWiseType == TIDL_EltWiseSum)))
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_EltWiseLayer "
       "%s: Too many inputs for elementwise operator", layerPC.name);
  }
#else
  /*
   * Not supported
   */
  if(layerPC.numInBufs > 2)
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_EltWiseLayer %s: Only supports 2 inputs, found %d", layerPC.name, layerPC.numInBufs);
  }
#endif
}

static void checkFlattenLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  if(is16bit)
  {
    notOptimal = true;
    TIDL_LOG_NOT_PERFORMANT(diags, "TIDL_FlattenLayer "
       "%s: Flatten with 16 bit is not optimal in this version", layerPC.name);
  }
}

static void checkSliceLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_SliceLayerParams_t& params = layerPC.layerParams.sliceParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

    /*
    * Not supported
    */
    if (params.axis == TIDL_DIM_BATCH)
    {
      fatalError = true;
      TIDL_LOG_UNSUPPORTED(diags, "TIDL_SliceLayer "
        "%s: Only supports non-batch dimension.", layerPC.name);
    }
  }



static void checkArgOpLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ArgOpParams_t& params = layerPC.layerParams.argOpParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  /*
   * Not supported
   */
  if(layerPC.outData[0].dimValues[TIDL_DIM_BATCH] != 1 ||
     layerPC.outData[0].dimValues[TIDL_DIM_NUMCH] != 1)
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_ArgOpLayer "
       "%s: Only supports axis == 1", layerPC.name);
  }
}


static void checkQuantizeLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  bool fatalError = false;
  if(gParams.preQuantizedModel == 0)
  {
    /*Quantize operator is only supported for pre-quantized models*/
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_QuantizeLayer "
    "%s: only supported for pre-quantized ONNX models", layerPC.name);
  }
}

static void checkDequantizeLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_DequantizeParams_t& params = layerPC.layerPCParams.dequantParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

  /*
   * Not supported
   */
  if(gParams.preQuantizedModel == 0)
  {
    if((params.scale != 1.0 || params.zeroPoint != 0.0 ))
    {
      fatalError = true;
      TIDL_LOG_UNSUPPORTED(diags, "TIDL_DequantizeLayer "
        "%s: Only scale = 1.0 and zeroPoint = 0.0 are supported", layerPC.name);
    }
  }

}

static void checkCastLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;
  /*
   * Not supported
   */
  if(gParams.addDataConvertToNet == 0)
  {
    fatalError = true;
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_CastLayer "
       "%s: is not supported", layerPC.name);
  }
}


static void checkSoftmaxLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_SoftMaxParams_t& params = layerPC.layerParams.softMaxParams;
  bool validated = false;
  bool warning = false;
  bool notOptimal = false;
  bool fatalError = false;

}

static void checkConcatenateLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ConcatParams_t &concatParams = layerPC.layerParams.concatParams;
  sTIDL_allowlistingMetaData md  = layerPC.allowlistingMetaData;
  int32_t shapeInferencestatus = checkShapeInferenceforOnnx(md);
  int32_t numDim = 0;
  if(shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    numDim = layerPC.allowlistingMetaData.varTensorsDims[0].size();
  }
  else
  {
    numDim = 4;
  }
  if (numDim > 3 && concatParams.axis == 0)
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_ConcatLayer "
       ": Concatenate in batch axis is not supported");
  }
}

static void checkClipLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ClipParams_t &clipParams = layerPC.clipParams;

  // Not supported, except min <= 0 and max > 0
  if(!((clipParams.clipMin <= 0) && (clipParams.clipMax > 0)))
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_ConcatLayer: Clip must have min <= 0 and max > 0")
  }
}

static void checkReshapeLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  int32_t numDims = layerPC.weights.bufSize;

  int32_t inBatchDim  = layerPC.inData[0].dimValues[TIDL_DIM_BATCH];
  int32_t outBatchDim = layerPC.outData[0].dimValues[TIDL_DIM_BATCH];
  int32_t inChDim     = layerPC.inData[0].dimValues[TIDL_DIM_NUMCH];
  int32_t outChDim    = layerPC.outData[0].dimValues[TIDL_DIM_NUMCH];

  if((layerPC.layerPCParams.reshapeParams.isInduced == 0) && (numDims > TIDL_DIM_MAX))
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_ReshapeLayer "
       "%s: Reshape output dimensions > %d not supported ", layerPC.name, TIDL_DIM_MAX);
    return;
  }
  /*
  Temporarily disabling this check
  if((inBatchDim != outBatchDim))
  {
    diags.emplace_back(TIDL_ModelDiagnostic::DK_NotSupported, "[TIDL_ReshapeLayer] "
       "%s Reshape over batch dimension is not supported currently ", layerPC.name);
  }
  */

}

static void checkTransposeLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  return;
}

static void checkSqueezeLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  if(gParams.modelType == 0 || gParams.modelType == 1 || gParams.modelType == 3) //not supposed to be there in network if not open source runtime
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_SqueezeLayer "
       "%s: should be removed in import process, otherwise this model will not work", layerPC.name);
    return;
  }
  else
  {
    //supported
  }

}

static void checkGatherLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  sTIDL_allowlistingMetaData md = layerPC.allowlistingMetaData;
  int32_t indiceDim = md.varTensorsDims[1].size();
  int32_t inDataDim = md.varTensorsDims[0].size();

  int32_t status = 0, i1;

  const sTIDL_GatherLayerParams_t& params = layerPC.layerParams.gatherParams;
  int32_t axis = params.axis;

  /**
   * Only line/vector gather is supported
   * Input data should be 2D (HxW or 1xHxW or 1x1xHxW etc.)
   * Indice data should be 1D (W or 1xW or 1x1xW etc)
  */

  if (inDataDim < 2)
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_GatherLayer %s: Input data dim <=1, unsupported configuration for gather layer", layerPC.name);
  }
  else if (inDataDim > 2)
  {
    for (i1 = inDataDim - 3; i1 > -1; i1--)
    {
      if (layerPC.inData[0].dimValues[i1] != 1)
      {
        TIDL_LOG_UNSUPPORTED(diags, "TIDL_GatherLayer %s: Unsupported gather configuration, Channel & higher dims for indata should be 1", layerPC.name);
      }
    }
  }

  /**
   * Axis check
  */
  if (axis != TIDL_DIM_HEIGHT)
  {
    TIDL_LOG_UNSUPPORTED(diags, "TIDL_GatherLayer %s: Unsupported axis configuration for gather layer, only line gather is supported", layerPC.name);
  }

}
// This function is used to post an error for layers that should be removed by the import process.
static void checkScatterElementsLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ScatterElementsParams_t& params = layerPC.layerParams.scatterElementsParams;

  int32_t isScatterNd = (params.axis == -1)?1:0;

  if(isScatterNd)
  {
    int32_t c, reduction;
    c = layerPC.inData[2].dimValues[TIDL_DIM_NUMCH];
    reduction = layerPC.layerParams.scatterElementsParams.reduction;

    if((c > 1) && (reduction == TIDL_ScatterElementsAdd))
    {
      TIDL_LOG_NOT_VERIFIED(diags, "Updates tensor shape not supported. "
       "Updates tensor should not have more than 1 channel");
    }
  }
  else if((params.axis != TIDL_DIM_WIDTH) && (!isScatterNd)) /*only width direction scatter is supported*/
  {
    TIDL_LOG_UNSUPPORTED(diags, "%s :"
      "%s should be removed in import process, otherwise this model will not work",
      TIDL_GetLayerString(layerPC.layerType), layerPC.name);
  }
}

static void checkReduceLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  const sTIDL_ReduceParams_t& params = layerPC.layerParams.reduceParams;

  if((params.axis != TIDL_DIM_HEIGHT) || (params.keepDims != 1 || !(params.ops == TIDL_inReduceOpMax || params.ops == TIDL_inReduceOpMin)))
  {
    TIDL_LOG_UNSUPPORTED(diags, "%s "
      "%s: should be removed in import process, otherwise this model will not work",
      TIDL_GetLayerString(layerPC.layerType), layerPC.name);
  }
}

// This function is used to post an error for layers that should be removed by the import process.
static void checkFoldedLayers(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  TIDL_LOG_UNSUPPORTED(diags, "%s "
     "%s: should be removed in import process to support this model",
     TIDL_GetLayerString(layerPC.layerType), layerPC.name);
}

static void checkQuantStatsAvailable(sTIDL_Network_t * resultTIDLNetStructure, sTIDL_OrgNetwork_t * orgTIDLNetStructure, DiagList_t &diags)
{
  if(resultTIDLNetStructure->isQuantStatsAvailable == 0)
  {
    TIDL_LOG_UNSUPPORTED(diags, "[TIDL_E_QUANT_STATS_NOT_AVAILABLE] "
       "tidl_quant_stats_tool.out fails to collect dynamic range. Please look into quant stats log. This model will get fault on target");
  }
}


static void checkDataflowInfoAvailable(sTIDL_Network_t * resultTIDLNetStructure, sTIDL_OrgNetwork_t * orgTIDLNetStructure, DiagList_t &diags)
{
  if(resultTIDLNetStructure->dataFlowInfo == 0)
  {
    if(((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT) ||
                            (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY))
             && ((gParams.numParamBits == 32) || (gParams.numParamBits == 16 && gParams.inferencePrecisionMode == TIDL_InferencePrecisionModeFloatingPoint)))
    {
      /* OSRT floating pass -- Corresponding warning is added in OSRT */
    }
    else
    {
      TIDL_GLOBAL_REPORT_WARNING("\n********************************************************************"
                                 "\n*     Network compiler returned with error or didn't executed      *"
                                 "\n*      This model can only be used on PC/Host emulation mode       *"
                                 "\n*            It is not expected to work on target/EVM              *"
                                 "\n********************************************************************\n");
    }
  }
}

/* PC layers which are not supported individually */
static const std::unordered_set<int32_t> individualUnsupportedTIDLOps {
  TIDL_PowLayer,
  TIDL_SigmoidLayer,
  TIDL_TanhLayer,
  TIDL_HardSigmoidLayer,
  TIDL_ELULayer,
  TIDL_ScaleLayer,
  TIDL_DropOutLayer,
  TIDL_ClipLayer,
  TIDL_MinimumLayer,
  TIDL_BiasLayer,
  TIDL_ReduceMeanLayer,
  TIDL_ReduceSumLayer,
  TIDL_AsinLayer,
  TIDL_AsinhLayer,
  TIDL_HardSwishLayer,
  TIDL_MishLayer,
  TIDL_LogLayer,
  TIDL_AbsLayer,
  TIDL_FloorLayer,
  TIDL_ExpLayer,
  TIDL_SinLayer,
  TIDL_ErfLayer,
  TIDL_AcosLayer,
  TIDL_AtanLayer,
  TIDL_SinhLayer,
  TIDL_NegLayer,
  TIDL_CosLayer,
  TIDL_CoshLayer,
  TIDL_TanLayer,
  TIDL_SwishLayer,
  TIDL_SoftPlusLayer,
  TIDL_SoftSignLayer,
  TIDL_CeilLayer,
  TIDL_CeluLayer,
  TIDL_SeluLayer,
  TIDL_RoundLayer,
  TIDL_SignLayer,
  TIDL_AttentionLayer,
};

int tidlModelCheck(tidl_import_config * params, sTIDL_OrgNetwork_t * orgTIDLNetStructure)
{
  sTIDL_Network_t * resultNetStructure;
  int32_t ret;

  FILE* fp = fopen((char*)params->outputNetFile, "rb");
  if(fp == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("TIDL Model check: cannot open the result network file %s", params->outputNetFile);
    return 1;
  }

  resultNetStructure = (sTIDL_Network_t*)malloc(sizeof(sTIDL_Network_t));

  ret = fread(resultNetStructure, 1, sizeof(sTIDL_Network_t), fp);

  is16bit = params->numFeatureBits == 16;

  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "****************************************************");
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "**               TIDL Model Checker               **");
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "****************************************************");
  }

  // Check each layer for legality and accumulate diagnostics
  DiagList_t diags;
  int32_t errorCount = 0;
  int32_t warningCount = 0;

  for (int i = 0; i < orgTIDLNetStructure->numLayers; i++)
  {
    if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX ||
        gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT ||
        gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT || 
        gParams.modelType ==  TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY
    )
    {
      if (individualUnsupportedTIDLOps.find(orgTIDLNetStructure->TIDLPCLayers[i].layerType) != individualUnsupportedTIDLOps.end())
      {
        TIDL_LOG_UNSUPPORTED(diags, "Layer %s, type %d not supported as individual node", (char *)orgTIDLNetStructure->TIDLPCLayers[i].name, orgTIDLNetStructure->TIDLPCLayers[i].layerType);
      }
      else
      {
        tidlCheckAllowlistingConstraints(orgTIDLNetStructure->TIDLPCLayers[i], diags);
      }
    }
    else
    {
      tidlModelCheckLayer(orgTIDLNetStructure->TIDLPCLayers[i], diags);
    }
  }

  // Report any errors or warnings
  for (auto diag : diags)
  {
    gDiags.modelDiag(diag);
    if (diag.isError())
       ++errorCount;
    else
       ++warningCount;
  }

  // Check global model properties
  checkQuantStatsAvailable(resultNetStructure, orgTIDLNetStructure, diags);
  checkDataflowInfoAvailable(resultNetStructure, orgTIDLNetStructure, diags);

  if((gParams.numParamBits != 32) && (gParams.inferencePrecisionMode == TIDL_InferencePrecisionModeFixedPoint))
  {
    if((warningCount == 0) && (errorCount == 0))
    {
      // TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
      TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Green, "Subgraph Compiled Successfully");
      // TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
     }
    else
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
      TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Yellow, "Subgraph Compiled with %3d Warnings and %3d Errors", warningCount, errorCount);
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
    }
  }
  free(resultNetStructure) ;
  return errorCount;
}


// Check a single layer for legality and enque any errors or warnings. This function is the basis
// of both the model check pass and the allowlist API used by external frameworks.
void tidlModelCheckLayer(const sTIDL_LayerPC_t &layerPC, DiagList_t &diags)
{
  switch (layerPC.layerType)
  {
    case TIDL_DataLayer:
      // fully supported
      break;
    case TIDL_ConvolutionLayer:
      checkConvLayers(layerPC, diags);
      break;
    case TIDL_PoolingLayer:
      checkPoolingLayers(layerPC, diags);
      break;
    case TIDL_ReLULayer:
      // Converted to batchnorm unconditionally if not merged
      break;
    case TIDL_PReLULayer:
      // Converted to batchnorm unconditionally if not merged
      break;
    case TIDL_SigmoidLayer:
      // should be merged or be folded out
      // This layer has no specific check, and is converted to BN layer, so mark as supported for runtimes allowlisting, since it won't be folded during first allowlisting pass
      if ((gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX_RT) && (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT))
        checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_TanhLayer:
      if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))
      {
        checkFoldedLayers(layerPC, diags);
      }

      // should be merged or be folded out
      // This layer has no specific check, and is converted to BN layer, so mark as supported for runtimes allowlisting, since it won't be folded during first allowlisting pass
      break;
    case TIDL_HardSigmoidLayer:
      if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))
      {
        checkFoldedLayers(layerPC, diags);
      }

      // should be merged or be folded out
      // This layer has no specific check, and is converted to BN layer, so mark as supported for runtimes allowlisting, since it won't be folded during first allowlisting pass
      break;
    case TIDL_ELULayer:
      if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))
      {
        checkFoldedLayers(layerPC, diags);
      }

      // should be merged or be folded out
      // This layer has no specific check, and is converted to BN layer, so mark as supported for runtimes allowlisting, since it won't be folded during first allowlisting pass
      break;
    case TIDL_EltWiseLayer:
      checkEltwiseLayers(layerPC, diags);
      break;
    case TIDL_InnerProductLayer:
      checkInnerProductLayers(layerPC, diags);
      break;
    case TIDL_SoftMaxLayer:
      if (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX_RT)
        checkSoftmaxLayers(layerPC, diags);
      break;
    case TIDL_BatchNormLayer:
      checkBatchNormLayers(layerPC, diags);
      break;
    case TIDL_BiasLayer:
      // should be merged or be folded out
      checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_ScaleLayer:
      checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_Deconv2DLayer:
      checkDeconvLayers(layerPC, diags);
      break;
    case TIDL_ConcatLayer:
      checkConcatenateLayers(layerPC, diags);
      break;
    case TIDL_SplitLayer:
      // should be merged or be folded out
      //checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_SliceLayer:
      checkSliceLayers(layerPC, diags);
      break;
    case TIDL_CropLayer:
      // fully supported
      break;
    case TIDL_FlattenLayer:
      checkFlattenLayers(layerPC, diags);
      break;
    case TIDL_DropOutLayer:
      // should be folded out
      checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_ArgOpLayer:
      checkArgOpLayers(layerPC, diags);
      break;
    case TIDL_DetectionOutputLayer:
      checkDetectionOutLayers(layerPC, diags);
      break;
    case TIDL_ShuffleChannelLayer:
      checkShuffleChannelLayers(layerPC, diags);
      break;
    case TIDL_ResizeLayer:
      checkResizeLayers(layerPC, diags);
      break;
    case TIDL_RoiPoolingLayer:
      // fully supported
      break;
    case TIDL_OdPostProcessingLayer:
      // fully supported
      break;
    case TIDL_DepthToSpaceLayer:
      checkDepthToSpaceLayers(layerPC, diags);
    case TIDL_ColorConversionLayer:
      checkColorConversionLayers(layerPC, diags);
    case TIDL_PadLayer:
      // fully supported
      break;
    case TIDL_OdOutputReformatLayer:
      //fully supported
      break;
    case TIDL_DataConvertLayer:
      //fully supported
      break;
    case TIDL_BatchToSpaceLayer:
      checkBatchToSpaceLayers(layerPC, diags);
      break;
    case TIDL_SpaceToBatchLayer:
      checkBatchToSpaceLayers(layerPC, diags);
      break;
    case TIDL_BatchReshapeLayer:
      // fully supported
      break;
    case TIDL_CustomLayer:
      // fully supported
      break;
    case TIDL_UnsupportedLayer:
      // already reported
      break;

    // These layerTypes are represented only in the PC imoprt data structure, not in the target model.
    // None of them are supported.
    case TIDL_ConstDataLayer:
      // fully supported
      break;
    case TIDL_PriorBoxLayer:
      checkPriorBoxLayers(layerPC, diags);
      break;
    case TIDL_PermuteLayer:
      checkPermuteLayers(layerPC, diags);
      break;
    case TIDL_ReshapeLayer:
        checkReshapeLayers(layerPC, diags);
      break;
    case TIDL_ShapeLayer:
      break;
    case TIDL_SizeLayer:
      break;
    case TIDL_SqueezeLayer:
      checkSqueezeLayers(layerPC, diags);
      break;
    case TIDL_TransposeLayer:
        checkTransposeLayers(layerPC, diags);
      break;
    case TIDL_DequantizeLayer:
      //checkDequantizeLayers(layerPC, diags);
      break;
    case TIDL_CastLayer:
      checkCastLayers(layerPC, diags);
      break;
    case TIDL_ClipLayer:
      checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_MinimumLayer:
      checkFoldedLayers(layerPC, diags);
      break;
    case TIDL_LeakyReluLayer:
      //converted to Batchnorm unconditionally
      break;
    case TIDL_ScatterElementsLayer:
      checkScatterElementsLayers(layerPC, diags);
      break;
    case TIDL_ReduceLayer:
      checkReduceLayers(layerPC, diags);
      break;
    case TIDL_QuantizeLayer:
      //checkQuantizeLayers(layerPC, diags);
      break;
    case TIDL_GatherLayer:
      //checkGatherLayers(layerPC, diags);
      break;
    //Add check for all:
    case TIDL_ReduceMeanLayer:
      break;
    case TIDL_ReduceSumLayer:
      break;
    case TIDL_PowLayer:
      break;
    case TIDL_SqrtLayer:
      break;
    case TIDL_SubLayer:
      break;
    case TIDL_DivLayer:
      break;
    case TIDL_LayerNormLayer:
      //Add check
      break;
    case TIDL_GridSampleLayer:
      break;
    case TIDL_AsinLayer:
      break;
    case TIDL_AsinhLayer:
      break;
    case TIDL_HardSwishLayer:
      break;
    case TIDL_MishLayer:
      break;
    case TIDL_LogLayer:
      break;
    case TIDL_AbsLayer:
      break;
    case TIDL_FloorLayer:
      break;
    case TIDL_ExpLayer:
      break;
    case TIDL_SinLayer:
      break;
    case TIDL_ErfLayer:
      break;
    case TIDL_AcosLayer:
      break;
    case TIDL_AtanLayer:
      break;
    case TIDL_SinhLayer:
      break;
    case TIDL_CosLayer:
      break;
    case TIDL_CoshLayer:
      break;
    case TIDL_TanLayer:
      break;
    case TIDL_NegLayer:
      break;
    case TIDL_SwishLayer:
      break;
    case TIDL_SoftPlusLayer:
      break;
    case TIDL_SoftSignLayer:
      break;
    case TIDL_CeilLayer:
      break;
    case TIDL_CeluLayer:
      break;
    case TIDL_SeluLayer:
      break;
    case TIDL_RoundLayer:
      break;
    case TIDL_SignLayer:
      break;
    case TIDL_GatherElementsLayer:
      break;
    case TIDL_AttentionLayer:
      break;
    default:
      TIDL_LOG_UNSUPPORTED(diags, "Unknown layer type: %d", layerPC.layerType);
  }
}

// This function is called by the allowlist interface to decide if a node can/should be supported
// as part of a subgraph offloaded to TIDL.
bool tidlModelCheckOffload(const tidl_import_config &params, const sTIDL_LayerPC_t &layerPC)
{
  is16bit = params.numFeatureBits == 16;

  /* size before checking for this layer */
  int32_t diagListSize = gDiags.gDiagList.size();

  // Check layers which are folded out during import. They need to be checked separately here
  // because tidlModelCheckLayer() assumes folded layers are not present in the optimized graph.
  // These checking can be moved to tidlModelCheckLayer() in future if tidlModelCheckLayer() is
  // called before optimization which does layer folding.
  // Relay op "clip" is mapped to either TIDL_ReLULayer with Relu6 or TIDL_ClipLayer. Relu6 has
  // no constraints but TIDL_ClipLayer has.
  if(layerPC.layerType == TIDL_ReLULayer)
    ; // this is needed because tidlModelCheckLayer() would flag this as error
  else if (layerPC.layerType == TIDL_ArgOpLayer)
    ; // skip calling tidlModelCheckLayer which checks output dimension that is not available now
  else if (layerPC.layerType == TIDL_BiasLayer)
    ; // will be folded or converted to BatchNorm
  else if (layerPC.layerType == TIDL_ClipLayer)
    checkClipLayers(layerPC, gDiags.gDiagList);
  // Check layers which are not folded out during import
  else if (layerPC.layerType == TIDL_IdentityLayer)
    ; // PC only layer, skip checking, it will be merged in optimizeNet()
  else if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY &&
           layerPC.allowlistingMetaData.numInputs > 0)
  {
    tidlCheckAllowlistingConstraints(layerPC, gDiags.gDiagList);
  }
  else
    tidlModelCheckLayer(layerPC, gDiags.gDiagList);

  /* check if something is pushed about this layer */
  if(gDiags.gDiagList.size() == diagListSize)
  {
    /* push generic stats */
    TIDL_LOG_INFO(gDiags.gDiagList, "Supported layer");
  }

  /* iterate over diags which got pushed during tidlModelCheckLayer*/
  for(int i=diagListSize;i<gDiags.gDiagList.size();i++)
  {
    auto diag = gDiags.gDiagList[i];
    if(!diag.isOKToOffload())
    {
      if (params.debugTraceLevel > 0)  gDiags.modelDiag(diag);
      return false;
    }
  }

  return true;

}

int tidlInputTensorDimCheck(sTIDL_OrgNetwork_t &orgTIDLNetStructure)
{
  for (int i1 = 0; i1 < orgTIDLNetStructure.numLayers; i1++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i1].layerType == TIDL_DataLayer)
    {
      for (int i2 = 0; i2 < orgTIDLNetStructure.TIDLPCLayers[i1].numOutBufs; i2++)
      {
        for (int i3 = 0; i3 < TIDL_DIM_MAX; i3++)
        {
          if(orgTIDLNetStructure.TIDLPCLayers[i1].outData[i2].dimValues[i3] <= 0)
          {
              TIDL_GLOBAL_REPORT_UNSUPPORTED("All the input tensor dimensions has to be greater then zero. For tensor %s, id %d - Dim %d is %d", orgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[i2], orgTIDLNetStructure.TIDLPCLayers[i1].outData[i2].dataId, i3, orgTIDLNetStructure.TIDLPCLayers[i1].outData[i2].dimValues[i3]);
              return 0;
          }
        }
      }
    }
  }
  return 1;
}

int tidlModelTensorDimCheck(sTIDL_OrgNetwork_t &orgTIDLNetStructure)
{
  for (int i1 = 0; i1 < orgTIDLNetStructure.numLayers; i1++)
  {
    for (int i2 = 0; i2 < orgTIDLNetStructure.TIDLPCLayers[i1].numOutBufs; i2++)
    {
      for (int i3 = 0; i3 < TIDL_DIM_MAX; i3++)
      {
        if(orgTIDLNetStructure.TIDLPCLayers[i1].outData[i2].dimValues[i3] <= 0)
        {
            TIDL_GLOBAL_REPORT_UNSUPPORTED("All the tensor dimensions has to be greater then zero. For tensor %s, id %d - Dim %d is %d", orgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[i2], orgTIDLNetStructure.TIDLPCLayers[i1].outData[i2].dataId, i3, orgTIDLNetStructure.TIDLPCLayers[i1].outData[i2].dimValues[i3]);
            return 0;
        }
      }
    }
  }
  return 1;
}
