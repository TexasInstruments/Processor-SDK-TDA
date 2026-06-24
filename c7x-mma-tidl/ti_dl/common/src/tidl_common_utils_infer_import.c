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

/**
----------------------------------------------------------------------------
@file    tidl_common_utils_infer_import.c
@brief   This file contains TILD common utilities between Inference and Import
@version 0.1 (Sep 2025) : Initial version [NK]
----------------------------------------------------------------------------
*/

#include "tidl_common_utils_infer_import.h"

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getDatElementSize
@brief      Function to gives the element size based on elementType

@param      elementType            : Current data element type
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_getDatElementSize(int32_t elementType)
{
  int32_t size = 1;
  if ((elementType == TIDL_SignedChar) || (elementType == TIDL_UnsignedChar) || (elementType == TIDL_Bool))
  {
    size = 1;
  }
  else if ((elementType == TIDL_SignedShort) || (elementType == TIDL_UnsignedShort))
  {
    size = 2;
  }
  else if ((elementType == TIDL_SinglePrecFloat) || (elementType == TIDL_UnsignedWord) || (elementType == TIDL_SignedWord))
  {
    size = 4;
  }
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  else if ((elementType == TIDL_UnsignedDoubleWord) || (elementType == TIDL_SignedDoubleWord))
  {
    /* LDRA_JUSTIFY_END */
    size = 8;
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  else
  {
    size = 1;
  }
  /* LDRA_JUSTIFY_END */
  return size;
}

void TIDL_innerProductGetBiasScaleSize(const sTIDL_DataParams_t        * outData,
                                  const sTIDL_InnerProductParams_t * innerProductParams,
                                  int32_t                     layerKernelType,
                                  int32_t                   * biasSize,
                                  int32_t                   * scaleSize,
                                  int32_t                   * shiftSize)
{
  int32_t numOutputChannels = 1;
  if((innerProductParams->constIdx == 1) &&
     (layerKernelType == false) ){
    numOutputChannels = innerProductParams->numBChannels * innerProductParams->numOutCols;
  }
  else if(innerProductParams->constIdx == 0)
    numOutputChannels = innerProductParams->numInRows;
  else
    numOutputChannels = innerProductParams->numOutCols;

  // *scaleSize = (outData->dimValues[TIDL_DIM_WIDTH] * 
  //           outData->dimValues[TIDL_DIM_NUMCH]  *
  //           outData->dimValues[TIDL_DIM_DIM1]   * 
  //           outData->dimValues[TIDL_DIM_DIM2]) * (int64_t)sizeof(uint8_t);
  
  *scaleSize = numOutputChannels * (int64_t)sizeof(uint8_t);
  *shiftSize = *scaleSize;
  *biasSize = (*scaleSize) * (int64_t)sizeof(int32_t) * TIDL_getDatElementSize(outData->elementType) ;
  return;
}
