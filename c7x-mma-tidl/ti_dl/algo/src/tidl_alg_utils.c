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

/**
----------------------------------------------------------------------------
@file    tidl_alg.c
@brief   This file contains TILD alg inteface implementation
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/
#include "tidl_alg_int.h"
#include "tidl_types.h"
#include "tidl_layer.h"
#include "itidl_ti.h"
#include "perfsim.h"
#include "tidl_priv_algo.h"
#include "tidl_deviceInfo.h"
#include "tidl_device_functions.h"
#include "tidl_device_utils.h"
#include "tidl_alg_utils.h"
#include "tidl_commonUtils.h"
#include "tidl_innerProduct_mma.h"
#include "tidl_forceNegativeTest.h"
#include "tidl_alg_utils_ref.h"
#include "bfloat16.h"
#if defined TIDL_DYNAMIC_SHAPE
#include "tidl_shapeInference.h"
#endif // TIDL_DYNAMIC_SHAPE


#include "gc.h"
#include "gc_helper.h"

#include <math.h>
#include <float.h>

#define TIDL_L3_SCRATCH_SIZE (128U * 1024U)
#define TIDL_L3_MSMC_DDR_STAGING_MEM (1025U * 1024U)

#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)
#define TIDL_ALG_INPUT (0)
#define TIDL_ALG_OUTPUT (1)

void TIDL_AllocatePtr(intptr_t basePtr,
                      int32_t *currOffset,
                      int32_t size,
                      int32_t alignMent,
                      void **assignPtr)
{
  intptr_t outPtr = 0;
  int32_t offset = *currOffset;
  offset = TIDL_ALIGN_CEIL(offset, alignMent);

  if (basePtr != 0)
  {
    outPtr = basePtr + (int32_t)offset;
    *assignPtr = (void *)outPtr;
  }
  else
  {
    *assignPtr = NULL;
  }

  *currOffset = (int32_t)offset + size;

  return;
}

void TIDL_getMMAv2_ScaleAndShift(float32_tidl scaleRatio, uint8_t *scale, uint8_t *shift)
{
  int32_t shiftBits = 0;
  float32_tidl newScaleRatio = scaleRatio;
  // int32_t mmaV2MaxScale = 255;
  // Check if scaleRatio > 255
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
  Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
  This does not impact functional correctness or safety.
  <justification end> */
  if (scaleRatio > 255.0f)
  {
    tidl_printf(0, "TIDL_getMMAv2_ScaleAndShift: ScaleRatio exceeds representation capability \n");
  }
  /* LDRA_JUSTIFY_END */
  // Since exponent goes only in one direction, repeated multiplication with 2 is performed till it exceeds the max range.
  int32_t flag = 1;
  while (flag == 1)
  {
    newScaleRatio *= 2.0f;
    if (shiftBits >= 40)
    {
      flag = 0; // Max capability of a right shift of 40 for 8-bit.
    }
    else if (newScaleRatio > 255.0)
    {
      newScaleRatio /= 2.0f;
      flag = 0;
    }
    else
    {
      /*do nothing*/
    }
    if (flag == 1)
    {
      shiftBits++;
    }
  }

  *shift = shiftBits;
  /*Add nudge of 0.5 to round correctly:*/
  *scale = (newScaleRatio + 0.5);
}

void TIDL_getMMAv2_EltwiseScalesAndShift(float32_tidl scale1, float32_tidl scale2, uint8_t *intScale1, uint8_t *intScale2, uint8_t *cScale, uint8_t *shift)
{
  float32_tidl fMax;
  float32_tidl fMin;
  int32_t maxScaleID;
  /*TIDL's MMA based eltwise addition kernel has a restriction that the scales in the A vector and B panel need to be 7-bit,
    hence the max ceiling is set to 127U (0b1111111)*/
  uint8_t fScaleVal = 127U;
  if (scale1 > scale2)
  {
    maxScaleID = 0;
    fMax = scale1;
    fMin = scale2;
  }
  else
  {
    maxScaleID = 1;
    fMax = scale2;
    fMin = scale1;
  }
  float32_tidl fVal = fMax / (float32_tidl)fScaleVal;
  float64_tidl iMin;
  TIDL_getMMAv2_ScaleAndShift(fVal, cScale, shift);
  float32_tidl commonFactor = (float32_tidl)(*cScale) / (pow(2, (*shift)));
  iMin = floor((float64_tidl)fMin / commonFactor);
  /*Add nudge of 0.5 to round correctly:*/
  if (maxScaleID == 0)
  {
    *intScale1 = (float32_tidl)fScaleVal + 0.5f;
    *intScale2 = iMin + 0.5;
  }
  else
  {
    *intScale1 = iMin + 0.5;
    *intScale2 = (float32_tidl)fScaleVal + 0.5;
  }
}

/**
 * @brief This function fills padded rows with zero
 *
 * @param inPtr : input data pointer, which is used in wrap around padding
 * @param ptr : Buffer pointer that needs pad filling
 * @param bufInfo : Properties of the buffer to be filled
 * @param width : Width of the buffer
 * @param height : Height of the buffer
 * @param linePitch : Pitch of the buffer between two rows
 * @param channelPitch :Pitch of the buffer between two channels
 * @param dmautilsContext : Memory to store DMA properties
 * @param zeroVec : Vector containing zeros for row filling
 * @param isPadDmaPending : Flag to indicate the DMA trigger status
 * @param trMem : Memory to store DMA trasfer properties
 * @param elementType : The element type of the pad buffer
 * @param paddingType : Indicates the type of padding to be done
 * @param padLayerParams : Layer level parameters of the Pad layer
 * @param outDataPadH : Pad height of the output buffer
 * @param outDataPadW : Pad Width of the output buffer
 * @param isFlowCtrlNatc : Flag to indicate the flow status
 * @param perChannelPadValPtr : Pointer indicating Pad values per channel
 * @param inDataParams : input data parameters to get dimensions and pitchs for wrap around padding
 * @return int32_t : retuns the status of Pad filling
 */
int32_t TIDL_FillPaddedRows(uint8_t *inPtr,
                            uint8_t *restrict ptr,
                            sBufferInfo_t *bufInfo,
                            int32_t width,
                            int32_t height,
                            int32_t numBatches,
                            int32_t linePitch,
                            int32_t channelPitch,
                            uint32_t batchPitch,
                            void *dmautilsContext,
                            uint8_t *zeroVec,
                            uint8_t *isPadDmaPending,
                            uint8_t *trMem,
                            int32_t elementType,
                            int32_t paddingType,
                            void *padLayerParams,
                            int32_t outDataPadH,
                            int32_t outDataPadW,
                            int32_t padFillValue,
                            int32_t isFlowCtrlNatc,
                            void *perChannelPadValPtr,
                            sTIDL_DataParams_t *inDataParams)
{

  int32_t i, j, k;
  uint8_t *temp_ptr = ptr;
  int32_t retVal = IALG_EOK;
  int32_t elementSize = TIDL_getDatElementSize(elementType);

  int32_t orgBufHeight = bufInfo->bufHeight;

  bufInfo->bufHeight = bufInfo->bufHeight / numBatches;

  /* We have allocated only 1k of zero vector so maximum supported offset filling via dma is 1k */
  if (((dmautilsContext == NULL) || (bufInfo->accessoffset > ((int32_t)TIDL_1K_ZERO_VECTOR))) && (TIDL_PADDING_TYPE_PAD_LAYER != paddingType) && (TIDL_PADDING_TYPE_PAD_LAYER_BOTH != paddingType) && (TIDL_PADDING_TYPE_PAD_LAYER_TOP != paddingType) && (TIDL_PADDING_TYPE_PAD_LAYER_BOTTOM != paddingType))
  {
    for (k = 0; k < numBatches; k++)
    {
      for (j = 0; j < bufInfo->bufHeight; j++)
      {
        if (TIDL_PADDING_TYPE_BOTH == paddingType)
        {
          for (i = 0; i < bufInfo->accessoffset; i++)
          {
            temp_ptr[(j * channelPitch) + i] = padFillValue;
            temp_ptr[(j * channelPitch) + i + (bufInfo->activeBufWidth - bufInfo->accessoffset)] = padFillValue;
          }
        }
        else if (TIDL_PADDING_TYPE_TOP == paddingType)
        {
          for (i = 0; i < bufInfo->accessoffset; i++)
          {
            temp_ptr[(j * channelPitch) + i] = padFillValue;
          }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else if (TIDL_PADDING_TYPE_BOTTOM == paddingType)
        {
          /* LDRA_JUSTIFY_END */
          for (i = 0; i < bufInfo->accessoffset; i++)
          {
            temp_ptr[(j * channelPitch) + i + ((outDataPadH + height) * linePitch)] = padFillValue;
          }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else if ((TIDL_PADDING_TYPE_TOP_LEFT == paddingType) ||
                 (TIDL_PADDING_TYPE_ST_TOP_LEFT == paddingType))
        {
          for (i = 0; i < (bufInfo->padCReq * elementSize); i++)
          {
            temp_ptr[(j * channelPitch) + (bufInfo->accessoffset - (bufInfo->padCReq * elementSize)) + i] = padFillValue;
          }
        }
        else
        {
          /* Do nothing */
        }
        /* LDRA_JUSTIFY_END */
      }
      temp_ptr = temp_ptr + batchPitch;
    }
  }
  else
  {
    if (trMem == NULL)
    {
      tidl_printf(0, "TIDL_DspFillPaddedRows : TrMem is null \n");
      retVal = IALG_EFAIL;
    }
    if (retVal == IALG_EOK)
    {
      sTIDL_PadLayerParams_t *padParams = (sTIDL_PadLayerParams_t *)padLayerParams;
      for (k = 0; k < numBatches; k++)
      {
        if (padParams->padType == (int32_t)TIDL_PadWrapAround)
        {
          retVal = TIDL_DspFillPaddedRows((inPtr) + (k * inDataParams->pitch[TIDL_ROI_PITCH]), (ptr) + (k * (int32_t)batchPitch), bufInfo, width, height, linePitch, channelPitch,
                                          dmautilsContext, zeroVec, isPadDmaPending, trMem,
                                          elementType, paddingType, padLayerParams, outDataPadH, outDataPadW, isFlowCtrlNatc, perChannelPadValPtr, inDataParams);
        }
        else
        {
          retVal = TIDL_DspFillPaddedRows(inPtr, (ptr) + (k * (int32_t)batchPitch), bufInfo, width, height, linePitch, channelPitch,
                                          dmautilsContext, zeroVec, isPadDmaPending, trMem,
                                          elementType, paddingType, padLayerParams, outDataPadH, outDataPadW, isFlowCtrlNatc, perChannelPadValPtr, inDataParams);
        }

        if ((retVal == IALG_EOK) && (k < (numBatches - 1)))
        {
      #ifndef PERF_MODELLING /* for target run lets call wait from here */
          DmaUtilsAutoInc3d_wait(dmautilsContext, TIDL_DMA_CHANNEL_MEMCPY);
      #else
          DmaUtilsAutoInc3d_wait_wrapper(dmautilsContext, TIDL_DMA_CHANNEL_MEMCPY);
      #endif
          retVal = DmaUtilsAutoInc3d_deconfigure(dmautilsContext, TIDL_DMA_CHANNEL_MEMCPY, trMem, 1);
        }
      }
    }
  }
  bufInfo->bufHeight = orgBufHeight;
  return retVal;
}

/**
 * @brief This function fills padded columns with zero
 *
 * @param ptr : Buffer pointer that needs pad filling
 * @param bufInfo : Properties of the buffer to be filled
 * @param width : Width of the buffer
 * @param height : Height of the buffer
 * @param linePitch : Pitch of the buffer between two rows
 * @param channelPitch :Pitch of the buffer between two channels
 * @param elementType : The element type of the pad buffer
 * @param flowCtrl : Flag to indicate the flow status
 * @param paddingType : Indicates the type of padding to be done
 * @param padLayerParams : Layer level parameters of the Pad layer
 * @param outDataPadH : Pad height of the output buffer
 * @param outDataPadW : Pad Width of the output buffer
 * @param perChannelPadValPtr : Pointer indicating Pad values per channel
 * @return int32_t : retuns the status of Pad filling
 */
int32_t TIDL_FillPaddedCols(uint8_t *ptr, const sBufferInfo_t *bufInfo, int32_t width, int32_t height, int32_t numBatches,
                            int32_t linePitch, int32_t channelPitch, int32_t batchPitch, int32_t elementType, int32_t flowCtrl, int32_t paddingType,
                            void *padLayerParams, int32_t outDataPadH, int32_t outDataPadW, int32_t padFillValue, void *perChannelPadValPtr, int32_t inHeight)
{

  int32_t elementSize = TIDL_getDatElementSize(elementType);
  if (TIDL_PADDING_TYPE_PAD_LAYER == paddingType)
  {
    for (int32_t n = 0; n < numBatches; n++)
    {
      if (elementType == TIDL_UnsignedChar)
      {
        uchar_vec padVector = (uchar_vec)0;
        (void)TIDL_DspFillPaddedColsPadLayer((uint8_t *)ptr + (n * batchPitch), padVector, bufInfo, width, inHeight, linePitch, channelPitch, elementSize, flowCtrl,
                                             paddingType, padLayerParams, outDataPadH, outDataPadW, perChannelPadValPtr);
      }
      else if (elementType == TIDL_SignedChar)
      {
        char_vec padVector = (char_vec)0;
        (void)TIDL_DspFillPaddedColsPadLayer((int8_t *)ptr + (n * batchPitch), padVector, bufInfo, width, inHeight, linePitch, channelPitch, elementSize, flowCtrl,
                                             paddingType, padLayerParams, outDataPadH, outDataPadW, perChannelPadValPtr);
      }
      else if (elementType == TIDL_UnsignedShort)
      {
        ushort_vec padVector = (ushort_vec)0;
        (void)TIDL_DspFillPaddedColsPadLayer((uint16_t *)(ptr + (n * batchPitch)), padVector, bufInfo, width, inHeight, linePitch, channelPitch, elementSize, flowCtrl,
                                             paddingType, padLayerParams, outDataPadH, outDataPadW, perChannelPadValPtr);
      }
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
      Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
      This does not impact functional correctness or safety.
      <justification end> */
      else if (elementType == TIDL_SignedShort)
      /* LDRA_JUSTIFY_END */
      {
        short_vec padVector = (short_vec)0;
        (void)TIDL_DspFillPaddedColsPadLayer((int16_t *)(ptr + (n * batchPitch)), padVector, bufInfo, width, inHeight, linePitch, channelPitch, elementSize, flowCtrl,
                                             paddingType, padLayerParams, outDataPadH, outDataPadW, perChannelPadValPtr);
      }
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
      Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
      This does not impact functional correctness or safety.
      <justification end> */
      else
      {
        /*do nothing*/
      }
      /* LDRA_JUSTIFY_END */
    }
  }
  else
  {
    (void)TIDL_DspFillPaddedCols((uint8_t *)ptr, bufInfo, width, height, linePitch, channelPitch, elementSize, flowCtrl,
                                 paddingType, padLayerParams, outDataPadH, outDataPadW, perChannelPadValPtr, numBatches, batchPitch, padFillValue);
  }

  return IALG_EOK;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_getMemoryChunkFromSysmem
@brief      Function gives you a pointer to system scratch memory based on user given input
            parameters

@param      sysMems : sysMem Handle
@param      size    : Number of bytes to be allocated
@param      alignment  : Alignment required for the pointer
@param      space     : in which memory memory is required. Refer \ref eSysMemScratch for valid values
@param      attribute : IALG_PERSIST/IALG_SCRATCH, note that all persistent memory should be allocated
                       in the beginning
@remarks    None
@return     Pointer to the allocated memory. NULL if there is not enough memory available
----------------------------------------------------------------------------
*/
void *TIDL_getMemoryChunkFromSysmem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX],
                                    uint32_t size, uint32_t alignment, uint32_t space, uint32_t attribute)
{
  uint8_t *mem;
  uint8_t *ptrCurr = &((uint8_t *)sysMems[space].base)[sysMems[space].offset];
  int32_t alignmentBytes;
  int32_t offsetOrig = sysMems[space].offset;
  if (alignment > 0U)
  {
    uint32_t tempAlignBytes = (((((uintptr_t)ptrCurr) + alignment - 1U) & (~(alignment - 1U))) - ((uintptr_t)ptrCurr));
    alignmentBytes = (int32_t)tempAlignBytes;
  }
  else
  {
    alignmentBytes = 0;
  }
  if ((sysMems[space].size - sysMems[space].offset) < (alignmentBytes + (int32_t)size))
  {
    /* ----------------------------------------------------*/
    /* This scenario arrives if remaining space in SL2     */
    /* is not sufficient for the requested chunk of memory */
    /* Control never hits below  instruction at run-time   */
    /* This while(1) is kept here to catch the SL2 memory  */
    /* insufficiency during development phase              */
    /* ----------------------------------------------------*/
    /* Removing while loop. If a memory is not avilable then
     * test app can take action to allocate this memory from
     * DDR instead of just getting stuck in while(1)
     */
    /*while(1){
        ;
    }*/
    mem = NULL;
  }
  else
  {
    mem = &ptrCurr[alignmentBytes];
    sysMems[space].offset += (alignmentBytes + (int32_t)size);
    if (attribute == (uint32_t)IALG_PERSIST)
    {
      sysMems[space].base = &mem[(int32_t)size];
      sysMems[space].size = sysMems[space].size - (offsetOrig + alignmentBytes + (int32_t)size);
      sysMems[space].offset = 0;
    }
  }
  return mem;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_resetSysmem
@brief      Function gives clears all the memories present in sysMem

@param      sysMems : sysMem Handle
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_resetSysmem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX])
{
  uint32_t i;

  for (i = 0; i < TIDL_SYSMEM_MAX; i++)
  {
    sysMems[i].offset = 0;
  }

  return;
}

#if defined TIDL_COVERAGE_DEAD_CODE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_isOutDataFlat
@brief      Function to Check whether current out data buffer needs to
            flattened

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Current data buffer ID
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_isOutDataFlat(const sTIDL_Network_t *pTIDLNetStructure,
                           int32_t dataId)
{
  int32_t i, j;
  int32_t status = 1;
  for (i = 0; i < pTIDLNetStructure->numLayers; i++)
  {
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if ((pTIDLNetStructure->TIDLLayers[i].layerType != TIDL_InnerProductLayer) &&
          (pTIDLNetStructure->TIDLLayers[i].inData[j] == dataId))
      {
        status = 0;
        break;
      }
    }
    if (status == 0)
    {
      break;
    }
  }

  return status;
}
#endif

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getDatElementSign
@brief      Function to gives the element sign based on elementType

@param      elementType            : Current data element type
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_getDatElementSign(int32_t elementType)
{
  int32_t sign = 1;
  if ((elementType == TIDL_SignedChar) || (elementType == TIDL_SignedShort) || (elementType == TIDL_SignedWord) || (elementType == TIDL_SinglePrecFloat) || (elementType == TIDL_BFloat16))
  {
    sign = 1;
  }
  else
  {
    sign = 0;
  }
  return sign;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_isInDataBuff
@brief      Function to Check whether current data buffer is Input to
            current algorithm instance

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Current data buffer ID
@param      layersGroupId     : current algorithm instance Group ID
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_isInDataBuff(const sTIDL_Network_t *pTIDLNetStructure,
                          int32_t dataId, int32_t layersGroupId)
{
  int32_t i, j;
  int32_t status = 0;
  for (i = 0; i < pTIDLNetStructure->numLayers; i++)
  {
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numOutBufs; j++)
    {
      if ((pTIDLNetStructure->TIDLLayers[i].layersGroupId != layersGroupId) &&
          (pTIDLNetStructure->TIDLLayers[i].outData.dataId == dataId))
      {
        status = 1;
        break;
      }
    }
    if (status == 1)
    {
      break;
    }
  }

  return status;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getLayerNum
@brief      Function to get the layer index of data ID

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Current data buffer ID
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_getLayerNum(const sTIDL_Network_t *pTIDLNetStructure, int32_t dataId)
{
  int32_t i, j;
  int32_t layerNum = -1;
  for (i = 0; i < pTIDLNetStructure->numLayers; i++)
  {
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numOutBufs; j++)
    {
      if (pTIDLNetStructure->TIDLLayers[i].outData.dataId == dataId)
      {
        layerNum = i;
      }
    }
  }
  return layerNum;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getDataBuff
@brief      Function to get base buffer pointer from Ivision buffer

@param      bufDesc      : ivision buffer descriptor list
@param      numBufs      : Number of buffers in the list
@param      dataId       : Current data ID
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
void *TIDL_getDataBuff(IVISION_BufDesc *bufDesc[], uint32_t numBufs,
                       int32_t dataId, int32_t numBatches, int32_t pitch[])
{
  int32_t i;
  void *ptr;
  void *retVal = NULL;
  for (i = 0; i < (int32_t)numBufs; i++)
  {
    if (bufDesc[i]->bufferId == dataId)
    {
      IVISION_BufPlanes(*ivisionBufPlane)[] = (IVISION_BufPlanes(*)[])((void *)(&bufDesc[i]->bufPlanes[0]));

      ptr = (void *)(get_uint8_t_pointer((uint8_t *)(*ivisionBufPlane)[0].buf,
                                         (((((int32_t)(*ivisionBufPlane)[0].frameROI.topLeft.y) * (int32_t)(*ivisionBufPlane)[0].width)) +
                                          ((int32_t)(*ivisionBufPlane)[0].frameROI.topLeft.x))));
      retVal = ptr;
      if (((uint32_t)pitch[TIDL_LINE_PITCH] > (*ivisionBufPlane)[0].width))
      {
        retVal = NULL;
      }
      break;
    }
  }

  return retVal;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getLayerIDForOutDataID
@brief      Function to give the layer with specified outData ID

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Current data buffer ID
@remarks    None
@return     Layer ID of the layer in sTIDL_Network_t
----------------------------------------------------------------------------
*/
int32_t TIDL_getLayerIDForOutDataID(const sTIDL_Network_t *pTIDLNetStructure,
                                    int32_t dataId)
{
  int32_t i, j;
  int32_t index = -1;
  if (pTIDLNetStructure->numLayers < TIDL_NUM_MAX_LAYERS)
  {
    for (i = 0; i < pTIDLNetStructure->numLayers; i++)
    {
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numOutBufs; j++)
      {
        if (pTIDLNetStructure->TIDLLayers[i].outData.dataId == dataId)
        {
          index = i;
          break;
        }
      }
      if (index != -1)
      {
        break;
      }
    }
  }
  else
  {
    index = -1;
  }
  return index;
}

/**
----------------------------------------------------------------------------
@fn         TIDL_getNumConsumersForDataId
@brief      Function to Check number of consumers for given data Id

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Indata Id of current layer
@remarks    None
@return     Number of consumers with given data Id
----------------------------------------------------------------------------
*/
int32_t TIDL_getNumConsumersForDataId(const sTIDL_Network_t *pTIDLNetStructure, int32_t dataId)
{
  int32_t numConsumers = 0;
  for (int i = 0; i < pTIDLNetStructure->numLayers; i++)
  {
    for (int j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if (pTIDLNetStructure->TIDLLayers[i].inData[j] == dataId)
      {
        numConsumers++;
      }
    }
  }
  return numConsumers;
}

/**
 * @brief This function returns the ROI pitch in bytes
 *
 * @param dataParams : This has the parameters of data or kernel buffer
 * @return int32_t :  returns the ROI pitch in bytes
 */
int32_t TIDL_updateDataParamsPitch(const sTIDL_DataParams_t *dataParams)
{
  int32_t dataSize;
  dataSize = dataParams->dimValues[TIDL_DIM_BATCH] * dataParams->pitch[TIDL_ROI_PITCH] * TIDL_getDatElementSize(dataParams->elementType);

  return (dataSize);
}

#if defined TIDL_COVERAGE_DEAD_CODE
/**
 * @brief This function the no of Input and output tensors that requires the
 *        new generic dataflow
 *
 * @param params : Create time parameters for tidl
 * @param layerIdx : Index of the layer to be processed
 * @param numInTensors : no of input tensors that requires the generic dataflow
 * @param numOutTensors : no of output tensors that requires the generic dataflow
 * @return int32_t : returns function status
 */
int32_t TIDL_getNumInOutTensors(const TIDL_CreateParams *params,
                                int32_t layerIdx,
                                int32_t *numInTensors,
                                int32_t *numOutTensors)
{
  int32_t status = IALG_EOK;
  *numInTensors = params->net->TIDLLayers[layerIdx].numInBufs;
  *numOutTensors = params->net->TIDLLayers[layerIdx].numOutBufs;
  sTIDL_Layer_t *tidlLayer = &params->net->TIDLLayers[layerIdx];
  /* For concat layer we process one tensor at a time, so set numInTensors as 1 */
  if (tidlLayer->layerType == TIDL_ConcatLayer)
  {
    *numInTensors = 1;
  }

  if (tidlLayer->layerType == TIDL_DetectionOutputLayer)
  {
    *numInTensors = 1;
  }
  if (*numInTensors > TIDL_MAX_IN_TENSORS)
  {
    tidl_printf(0, "For Eltwise layer num of Inputs are morethan TIDL_MAX_IN_TENSORS supported for New GenericFlow. \n");
    status = IALG_EFAIL; /* TODO : Replace with proper error code */
  }

  return status;
}

/**
 * @brief This function returns the output processing height for the given inHeight
 *
 * @param inHeight : input processing height
 * @param currentLayer : layer properties of the current layer
 * @return int32_t : returns the output processing height
 */
int32_t TIDL_getOutProcHeight(int32_t inHeight, sTIDL_Layer_t *currentLayer, sTIDL_DataParams_t *inDataParams)
{
  int32_t retVal;
  if ((currentLayer->layerType == TIDL_ArgOpLayer) ||
     (currentLayer->layerType == TIDL_EltWiseLayer) ||
     (currentLayer->layerType == TIDL_BatchNormLayer) ||
     (currentLayer->layerType == TIDL_BatchReshapeLayer) ||
     (currentLayer->layerType == TIDL_ConcatLayer) ||
     (currentLayer->layerType == TIDL_DataConvertLayer) ||
     (currentLayer->layerType == TIDL_ReduceLayer) ||
     (currentLayer->layerType == TIDL_ScatterElementsLayer) ||
     (currentLayer->layerType == TIDL_GatherLayer) ||
     (currentLayer->layerType == TIDL_LayerNormLayer) ||
     (currentLayer->layerType == TIDL_GatherElementsLayer))
  {
    retVal = inHeight;
  }
  else if (currentLayer->layerType == TIDL_PoolingLayer)
  {
    int32_t strideH = currentLayer->layerParams.poolParams.strideH;
    /* TODO : Need to re-visit this code */
    retVal = TIDL_CEIL_DIV(inHeight, strideH);
  }
  else if (currentLayer->layerType == TIDL_ResizeLayer)
  {
    int32_t resizeFact = currentLayer->layerParams.resizeParams.resizeRatio[3];
    retVal = inHeight * resizeFact;
  }
  else if (currentLayer->layerType == TIDL_ColorConversionLayer)
  {
    retVal = inHeight * 2;
  }
  else if (currentLayer->layerType == TIDL_CustomLayer)
  {
    /* For custom layer if more than one inputs are present it is expected
    that the input to output ratio is 1. If not then it is assumed that input
    to output ratio is same for all input heights */
    float32_tidl outInHeightRatio = 1.0;
    if (currentLayer->numInBufs == 1)
    {
      outInHeightRatio = (float32_tidl)currentLayer->outData.dimValues[TIDL_DIM_HEIGHT] /
                         (float32_tidl)inDataParams->dimValues[TIDL_DIM_HEIGHT];
    }
    retVal = inHeight * outInHeightRatio;
  }
  else
  {
    tidl_printf(0, "This layer is not currently supported for New GenericFlow. \n");
    retVal = -1;
  }
  return retVal;
}

void TIDL_poolingGetUpdatedPadValues(int32_t Fr, int32_t strideVal, int32_t padH, int32_t strideOffsetMethod, int32_t heightVal, int32_t *topPad, int32_t *bottomPad)
{
  if (strideOffsetMethod == TIDL_StrideOffsetTopLeft)
  {
    if ((strideVal == 2) && ((heightVal & 1) == 0))
    {
      if (padH > 0)
      {
        *bottomPad -= 1U;
      }
    }
  }
  else
  {
    if ((strideVal == 2) && ((heightVal & 1) == 0))
    {
      if (padH > 0)
      {
        *topPad -= 1U;
      }
    }
  }
}
#endif

#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_refGetOutDataSize.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
#endif
int32_t TIDL_refGetOutDataSize(const TIDL_NetworkCommonParams *commonParams,
                               const TIDL_LayerSpecificParams *layerSpecificParams,
                               int32_t layerIdx)
{
  int32_t outDataSize;
  outDataSize = TIDL_updateDataParamsPitch(&commonParams->net->TIDLLayers[layerIdx].outData);
  if (commonParams->createParams->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
#ifdef HOST_EMULATION
    outDataSize = TIDL_findFreeOutBuff(commonParams->createParams, layerIdx,
                                       outDataSize, commonParams->TIDLLayersBufPtr);
#endif
    commonParams->TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }
  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DEVICE_UTILS_FORCE_OUITPUT_SIZE_NEG)
  {
    outDataSize = -1;
  }
  if (outDataSize >= 0)
  {
    /* If NC information is available then no output data required to be allocated */
    if (layerSpecificParams->workloadUnit != NULL)
    {
      outDataSize = 0;
    }
  }
  return outDataSize;
}

#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_refGetScratchDataSize.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
#endif
int32_t TIDL_refGetScratchDataSize(const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx)
{
  int32_t outBatchPitch;
  int32_t scratchDataSize;
  sTIDL_DataParams_t *dataParams = TIDL_getDataParams(commonParams->net,
                                                      commonParams->net->TIDLLayers[layerIdx].inData[0]);

  outBatchPitch = commonParams->net->TIDLLayers[layerIdx].outData.pitch[TIDL_ROI_PITCH];
  scratchDataSize = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH] * outBatchPitch *
                    TIDL_getDatElementSize(dataParams->elementType);

  float32_tidl tempValue = ((float32_tidl)scratchDataSize * 1.5f);
  scratchDataSize = (int32_t)tempValue; //: TODO: This will not be needed if we use perfsim output during alloc also
  /*Need to implement callbacks for scratchsizes!*/

  if((commonParams->net->TIDLLayers[layerIdx].layerType == TIDL_EltWiseLayer &&
      commonParams->net->TIDLLayers[layerIdx].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseMod) &&
     (dataParams->elementType == TIDL_SignedWord || dataParams->elementType == TIDL_UnsignedWord))
  {
    /* For eltwise layer int32/uint32, we need to allocate scratch buffer for 64bit accumulator */
    scratchDataSize *= (((int32_t)(sizeof(int64_t))) / TIDL_getDatElementSize(dataParams->elementType));
  }
  else if (TIDL_getDatElementSize(commonParams->net->TIDLLayers[layerIdx].outData.elementType) <= 2)
  {
    scratchDataSize *= ((commonParams->net->TIDLLayers[layerIdx].weightsElementSizeInBits + 7) / 8) * ((int32_t)(sizeof(int32_t)));
  }
  else
  {
    /*empty else for MisraC*/
  }
  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DEVICE_UTILS_FORCE_SCRATCHDATASIZE_NEG)
  {
    scratchDataSize = -1;
  }

  return scratchDataSize;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_outBuffAlloc
@brief      Function to Request Memory for layer output Memory

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
void TIDL_outBuffAlloc(const TIDL_CreateParams *params,
                       int32_t layerIdx, IALG_MemRec memRec[], int32_t outDataSize)
{
#ifdef HOST_EMULATION
  if ((TIDL_isOutDataBuff(params->net,
                          params->net->TIDLLayers[layerIdx].outData.dataId,
                          params->currLayersGroupId) == 0) &&
      (params->net->dataFlowInfo == NULL))
  {
    memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size += ((uint32_t)outDataSize + 128U);
    // tidl_printf(0," ALG_SCRATCH_DATA_BUFF_MEMREC Size for Layer , %4d,  %8d \n", layerIdx, memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size);
  }
#endif
  return;
}
#if defined TIDL_COVERAGE_DEAD_CODE
/**
 * @brief This function requests scratch memory size
 *
 * @param layerIdx : current layer index
 * @param memRec : memory table populated by the Algorithm
 * @param outDataSize : Ouptut buffer size
 */
void TIDL_scratchBufAlloc(IALG_MemRec memRec[], int32_t outDataSize)
{
  /* Allocate the maximum scratch buf required */
  if (outDataSize > (int32_t)memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size)
  {
    memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size = (uint32_t)outDataSize + 128U;
  }

  /*tidl_printf(0," ALG_REF_SCRATCH_BUFF_MEMREC Size for Layer , %4d,  %8d \n", layerIdx, memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size);*/
  return;
}
#endif
/**
 * @brief This function initializes scratch memory pointer
 *
 * @param layerIdx : current layer index
 * @param memRec : memory table populated by the Algorithm
 * @param outPtr : ouptut scratch pointer
 * @param outDataSize : Ouptut buffer size
 */
#if defined TIDL_COVERAGE_DEAD_CODE
void TIDL_scratchBufInit(int32_t layerIdx,
                         const IALG_MemRec memRec[], void **outPtr, int32_t outDataSize)
{
  *outPtr = memRec[ALG_REF_SCRATCH_BUFF_MEMREC].base;

  return;
}
#endif

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_outBuffInit
@brief      Function to Initialize the Layer output Memory

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer output Pointers list
@param      outDataSize  : Size of the ouput buffer to be allocated
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
void TIDL_outBuffInit(const TIDL_CreateParams *params,
                      int32_t layerIdx, int32_t *dataMemTabOffset,
                      const IALG_MemRec memRec[], void **outPtr, int32_t outDataSize)
{
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if ((TIDL_isOutDataBuff(params->net,
                          params->net->TIDLLayers[layerIdx].outData.dataId,
                          params->currLayersGroupId) == 1)
#ifdef HOST_EMULATION
      || (params->net->dataFlowInfo != NULL)
#endif
  )
  /* LDRA_JUSTIFY_END */
  {
    *outPtr = NULL;
  }
#ifdef HOST_EMULATION
  else
  {
    /* outDataSize is 0 means, not allocate the new memory for output buffer */
    if (outDataSize == 0)
    {
      *outPtr = NULL;
    }
    else
    {
      *outPtr = (void *)(get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].base, *dataMemTabOffset));
      uint32_t alignSizeValue = ALIGN_SIZE((uint32_t)outDataSize, 128U);
      *dataMemTabOffset += (int32_t)alignSizeValue;
    }
  }
#endif
  return;
}

#if defined TIDL_COVERAGE_DEAD_CODE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_shuffleChannelAlloc
@brief      Function to Request Memory required by the ShuffleChannel layer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_shuffleChannelAlloc(const TIDL_CreateParams *params,
                                 int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);
  int32_t scratchDataSize = outDataSize * (((int32_t)(sizeof(int32_t))));

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
    outDataSize = TIDL_findFreeOutBuff(params, layerIdx,
                                       outDataSize, TIDLLayersBufPtr);
    TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }
  TIDL_outBuffAlloc(params, layerIdx, memRec, outDataSize);
  TIDL_scratchBufAlloc(memRec, scratchDataSize);

  return 0;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_shuffleChannelInit
@brief      Function to Initialize the ShuffleChannel layer specific params

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      algLayer     : Pointer to current layer algorithm specific params
@param      paramMemTabOffset : Current Offset in ALG_LAYERS_PARAMS_BUFF_MEMREC
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer output Pointers list
@param      TIDLLayersBuf: Temp structure to store outDataSize and Data Id's
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_shuffleChannelInit(
    const TIDL_CreateParams *params, int32_t layerIdx,
    sTIDL_AlgLayer_t *algLayer, int32_t *paramMemTabOffset,
    int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
    void **outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t status = IALG_EOK;

  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);
  int32_t scratchDataSize = outDataSize * (((int32_t)(sizeof(int32_t))));

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
    outDataSize = TIDL_findFreeOutBuff(params, layerIdx,
                                       outDataSize, TIDLLayersBufPtr);
    TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }
  if (status == IALG_EOK)
  {
    TIDL_outBuffInit(params, layerIdx, dataMemTabOffset, memRec, outPtr, outDataSize);
    TIDL_scratchBufInit(layerIdx, memRec, &algLayer->scratchMem, scratchDataSize);
    algLayer->scratchSize = (int32_t)memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size;

    algLayer->metaData.totalOps =
        (params->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH]); // Verify this:
    algLayer->metaData.actualOps = algLayer->metaData.totalOps;
  }
  return status;
}

int32_t TIDL_padAlloc(const TIDL_CreateParams *params,
                      int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t status = TIDL_flattenAlloc(params, layerIdx, memRec, TIDLLayersBufPtr);
  sTIDL_PadLayerParams_t *layerParams = &params->net->TIDLLayers[layerIdx].layerParams.padLayerParams;
  int32_t padParamSize = layerParams->perChannelPadConstTensorSize;
  memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size += ((uint32_t)padParamSize + 128U);

  return status;
}

int32_t TIDL_padInit(
    const TIDL_CreateParams *params, int32_t layerIdx,
    sTIDL_AlgLayer_t *algLayer, int32_t *paramMemTabOffset,
    int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
    void **outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t status = IALG_EOK;
  sTIDL_PadLayerParams_t *layerParams = &params->net->TIDLLayers[layerIdx].layerParams.padLayerParams;
  int32_t paramSize = layerParams->perChannelPadConstTensorSize;
  int32_t paramOffset = layerParams->perChannelPadConstTensorOffset;
  void *dst;

  sTIDL_AlgPadParams_t *algPadParams = &algLayer->layerParams.padParams;

  status = TIDL_flattenInit(
      params, layerIdx, algLayer, paramMemTabOffset,
      dataMemTabOffset, memRec, outPtr, TIDLLayersBufPtr);

  dst = get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].base, *paramMemTabOffset);
  *paramMemTabOffset += ALIGN_SIZE(paramSize, 128);

  algPadParams->perChannelPadConstTensorPtr = dst;
  algPadParams->perChannelPadConstTensorSize = paramSize;

  if (paramSize && paramOffset)
  {
    void *src = ((int8_t *)(params->net) + paramOffset);
    (void)memcpy(dst, src, paramSize);
  }

  return status;
}
#endif

/**
----------------------------------------------------------------------------
@fn         TIDL_detectOutAlloc
@brief      Function to Request Memory required by the SSD detcetion output layer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@param      TIDLLayersBuf: Temp structure to store outDataSize and Data Id's
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_detectOutAlloc(const TIDL_CreateParams *params,
                            int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);
  int32_t numConfPts = 0;
  int32_t curData = 0;
  int32_t scratchDataSize;
  uint32_t curCnfPlaneSize = 0;
  uint32_t maxConfPlaneSize = 0;
  uint32_t elementSize = 0;
  int32_t priorBoxParamSize;

  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(params->net,
                                                        params->net->TIDLLayers[layerIdx].inData[0]);
  elementSize = TIDL_getDatElementSize(inDataParams->elementType);

  sTIDL_DetectOutputParams_t *detectOutParams = &params->net->TIDLLayers[layerIdx].layerParams.detectOutParams;
  priorBoxParamSize = detectOutParams->numHeads * detectOutParams->numSubHeads * (int32_t)sizeof(sTIDL_AnchorBoxParams_t);
  memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size += ((uint32_t)priorBoxParamSize + 128U);

  if (detectOutParams->metaArchType != (int32_t)TIDL_metaArchTIDLYolo)
  {
    /*total number input buffers will be twice the number of heads, and first half are location heads,
      and second half is confidence head*/
    for (int32_t i = ((uint32_t)params->net->TIDLLayers[layerIdx].numInBufs >> 1U); i < (params->net->TIDLLayers[layerIdx].numInBufs); i++)
    {
      const int32_t (*indimValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->dimValues;

      const int32_t (*inpitchValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->pitch;

      curData = ((int32_t)(*indimValues)[TIDL_DIM_BATCH] *
                 (int32_t)(*indimValues)[TIDL_DIM_NUMCH] *
                 ((int32_t)(*indimValues)[TIDL_DIM_HEIGHT]) *
                 ((int32_t)(*inpitchValues)[TIDL_LINE_PITCH]));

      curCnfPlaneSize = ((int32_t)(*indimValues)[TIDL_DIM_HEIGHT]) * ((int32_t)(*inpitchValues)[TIDL_LINE_PITCH]);

      numConfPts += curData;

      if (maxConfPlaneSize < curCnfPlaneSize)
      {
        maxConfPlaneSize = curCnfPlaneSize;
      }
    }
  }
  else
  {
    for (int32_t i = 0; i < (params->net->TIDLLayers[layerIdx].numInBufs); i++)
    {
      const int32_t (*indimValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->dimValues;

      const int32_t (*inpitchValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->pitch;

      curData = ((int32_t)(*indimValues)[TIDL_DIM_BATCH] *
                 (((int32_t)(*indimValues)[TIDL_DIM_NUMCH]) - 5) * // yolo has conf and location as common head, and it has 5 channels for box parameter(4) and objectness score(1)
                 ((int32_t)(*indimValues)[TIDL_DIM_HEIGHT]) *
                 ((int32_t)(*inpitchValues)[TIDL_LINE_PITCH]));

      numConfPts += curData;

      curCnfPlaneSize = ((int32_t)(*indimValues)[TIDL_DIM_HEIGHT]) * ((int32_t)(*inpitchValues)[TIDL_LINE_PITCH]);

      if (maxConfPlaneSize < curCnfPlaneSize)
      {
        maxConfPlaneSize = curCnfPlaneSize;
      }
    }
  }

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
#ifdef HOST_EMULATION
    if (((uint32_t)params->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
    {
      outDataSize = TIDL_findFreeOutBuff(params, layerIdx, outDataSize, TIDLLayersBufPtr);
    }
#endif
    TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }
  TIDL_outBuffAlloc(params, layerIdx, memRec, outDataSize);

  /*For worst scenario all the sratch memory is reserved in DDR*/

  // L2 related pointers
  /* topMScore, topMScoreSorted, topMIndices, topMIndicesSorted*/
  scratchDataSize = (numConfPts * (((int64_t)sizeof(int16_t) * 2) + ((int64_t)sizeof(int32_t) * 2))) + (128 * 4);

  // L1 related pointers
  scratchDataSize += ((detectOutParams->topK * (int64_t)sizeof(BBox)) + 128);               // topKBbox
  scratchDataSize += ((detectOutParams->topK * (int64_t)sizeof(uint16_t)) + 128);           // topKScore
  scratchDataSize += ((detectOutParams->topK * (int64_t)sizeof(int32_t)) + 128);            // topKIndices
  scratchDataSize += ((detectOutParams->topK * (int64_t)sizeof(int32_t)) + 128);            // nmsKeptIndices
  scratchDataSize += (((int64_t)sizeof(int32_t) * detectOutParams->numClasses) + 128);      // countMList
  scratchDataSize += (((int64_t)sizeof(int32_t) * detectOutParams->numClasses) + 128);      // countMListAcc
  scratchDataSize += ((detectOutParams->numClasses * (int64_t)sizeof(float32_tidl)) + 128); // if (softmax) tempScore

  // L3 related pointers
  maxConfPlaneSize = (maxConfPlaneSize >> 6U) << 6U; // make it multiple of 64 for SIMD width
  uint32_t tempSizeValue = 0U;
  tempSizeValue = (((maxConfPlaneSize + 63U) >> 1U) + 128U);
  scratchDataSize += (int32_t)tempSizeValue; // pred
  tempSizeValue = ((maxConfPlaneSize * 2U * elementSize) + 128U);
  scratchDataSize += (int32_t)tempSizeValue; // featMaxMinVal

  if ((uint32_t)scratchDataSize > memRec[ALG_SCRATCH_DDR_MEM_MEMREC].size)
  {
    memRec[ALG_SCRATCH_DDR_MEM_MEMREC].size = (uint32_t)(scratchDataSize);
  }

  return 0;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_detectOutInit
@brief      Function to Initialize the SSD detcetion output layer specific params

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      algLayer     : Pointer to current layer algorithm specific params
@param      paramMemTabOffset : Current Offset in ALG_LAYERS_PARAMS_BUFF_MEMREC
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer output Pointers list
@param      TIDLLayersBuf: Temp structure to store outDataSize and Data Id's
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_detectOutInit(
    const TIDL_CreateParams *params, int32_t layerIdx,
    sTIDL_AlgLayer_t *algLayer, int32_t *paramMemTabOffset,
    int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
    void **outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t status = IALG_EOK;
  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);
  int32_t numConfPts = 0;
  uint32_t priorBoxParamSize;
  int32_t curData = 0;
  int32_t scratchDataSize;
  int32_t locScratchOffset = 0;
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(params->net,
                                                        params->net->TIDLLayers[layerIdx].inData[0]);
  sTIDL_ALgDetectOutputParams_t *algDetectOutParams = &algLayer->layerParams.detectionOutputParams;
  float32_tidl *priorData;

  sTIDL_DetectOutputParams_t *detectOutParams = &params->net->TIDLLayers[layerIdx].layerParams.detectOutParams;
  priorData = (float32_tidl *)(((int8_t *)params->net) + detectOutParams->priorBox);

  if (detectOutParams->metaArchType != (int32_t)TIDL_metaArchTIDLYolo)
  {
    /*total number input buffers will be twice the number of heads, and first half are location heads,
      and second half is confidence head*/
    for (int32_t i = ((uint32_t)params->net->TIDLLayers[layerIdx].numInBufs >> 1U); i < (params->net->TIDLLayers[layerIdx].numInBufs); i++)
    {
      const int32_t (*indimValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->dimValues;

      const int32_t (*inpitchValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->pitch;

      curData = ((int32_t)(*indimValues)[TIDL_DIM_BATCH] *
                 (int32_t)(*indimValues)[TIDL_DIM_NUMCH] *
                 ((int32_t)(*indimValues)[TIDL_DIM_HEIGHT]) *
                 ((int32_t)(*inpitchValues)[TIDL_LINE_PITCH]));

      numConfPts += curData;
    }
  }
  else
  {
    for (int32_t i = 0; i < (params->net->TIDLLayers[layerIdx].numInBufs); i++)
    {
      const int32_t (*indimValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->dimValues;

      const int32_t (*inpitchValues)[TIDL_DIM_MAX] =
          (const int32_t (*)[TIDL_DIM_MAX])
              inDataParams->pitch;

      curData = ((int32_t)(*indimValues)[TIDL_DIM_BATCH] *
                 (((int32_t)(*indimValues)[TIDL_DIM_NUMCH]) - 5) * // yolo has conf and location as common head, and it has 5 channels for box parameter(4) and objectness score(1)
                 ((int32_t)(*indimValues)[TIDL_DIM_HEIGHT]) *
                 ((int32_t)(*inpitchValues)[TIDL_LINE_PITCH]));

      numConfPts += curData;
    }
  }

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
#ifdef HOST_EMULATION
    if (((uint32_t)params->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
    {
      outDataSize = TIDL_findFreeOutBuff(params, layerIdx, outDataSize, TIDLLayersBufPtr);
    }
#endif
    TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }

  priorBoxParamSize = detectOutParams->numHeads * detectOutParams->numSubHeads * (int32_t)sizeof(sTIDL_AnchorBoxParams_t);
  algDetectOutParams->priorBoxPtr =
      get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].base, *paramMemTabOffset);
  uint32_t tempAlignSize = ALIGN_SIZE(priorBoxParamSize, 128U);
  *paramMemTabOffset += (int32_t)tempAlignSize;
  (void)memcpy((void *)algDetectOutParams->priorBoxPtr, (const void *)priorData, priorBoxParamSize);

  uint32_t alignScratchData = 0U;

  TIDL_outBuffInit(params, layerIdx, dataMemTabOffset, memRec, outPtr, outDataSize);
  // if (status == IALG_EOK)  /* TIDL_outBuffInit() function doesn't support error handling */
  {
    algLayer->scratchSize = 0;
    scratchDataSize = numConfPts * (int64_t)sizeof(int16_t);
    algLayer->layerParams.detectionOutputParams.topMScoreDdr =
        (uint16_t *)get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_SCRATCH_DDR_MEM_MEMREC].base, locScratchOffset);
    alignScratchData = ALIGN_SIZE((uint32_t)scratchDataSize, 128U);
    locScratchOffset = locScratchOffset + (int32_t)alignScratchData;

    scratchDataSize = numConfPts * (int64_t)sizeof(int16_t);
    algLayer->layerParams.detectionOutputParams.topMScoreSortedDdr =
        (uint16_t *)get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_SCRATCH_DDR_MEM_MEMREC].base, locScratchOffset);
    alignScratchData = ALIGN_SIZE((uint32_t)scratchDataSize, 128U);
    locScratchOffset = locScratchOffset + (int32_t)alignScratchData;

    scratchDataSize = numConfPts * (int64_t)sizeof(int32_t);
    algLayer->layerParams.detectionOutputParams.topMIndicesDdr =
        (int32_t *)get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_SCRATCH_DDR_MEM_MEMREC].base, locScratchOffset);
    alignScratchData = ALIGN_SIZE((uint32_t)scratchDataSize, 128U);
    locScratchOffset = locScratchOffset + (int32_t)alignScratchData;

    scratchDataSize = numConfPts * (int64_t)sizeof(int32_t);
    algLayer->layerParams.detectionOutputParams.topMIndicesSortedDdr =
        (int32_t *)get_uint8_t_pointer((uint8_t *)(void *)memRec[ALG_SCRATCH_DDR_MEM_MEMREC].base, locScratchOffset);
    alignScratchData = ALIGN_SIZE((uint32_t)scratchDataSize, 128U);
    locScratchOffset = locScratchOffset + (int32_t)alignScratchData;

    algLayer->layerParams.detectionOutputParams.topMDdr = numConfPts;
    algLayer->layerParams.detectionOutputParams.scratchDDRConsumed = locScratchOffset;

    algLayer->metaData.totalOps =
        (params->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH] *
         params->net->TIDLLayers[layerIdx].layerParams.eltWiseParams.numInData);
    algLayer->metaData.actualOps = algLayer->metaData.totalOps;

    params->net->TIDLLayers[algLayer->layerIdx].layerParams.detectOutParams.reserve3 = params->forceNegativeTest;
    TIDL_collectLocConfHeadInfo(params->net->TIDLLayers, algLayer, NULL, algDetectOutParams->priorBoxPtr);

#ifdef HOST_EMULATION
    if (((uint32_t)params->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U)
#endif
    {
      status = TIDL_odFindValidLocAndScoreKernelInit(params, algLayer, &params->net->TIDLLayers[layerIdx], 0, 0, 0);
    }
  }

  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_roiPoolingAlloc
@brief      Function to Request Memory required by the ArgMax layer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@param      TIDLLayersBuf: Temp structure to store outDataSize and Data Id's
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_roiPoolingAlloc(const TIDL_CreateParams *params,
                             int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
    outDataSize = TIDL_findFreeOutBuff(params, layerIdx,
                                       outDataSize, TIDLLayersBufPtr);
    TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }
  TIDL_outBuffAlloc(params, layerIdx, memRec, outDataSize);

  return 0;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_roiPoolingInit
@brief      Function to Initialize the ArgMax layer specific params

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      algLayer     : Pointer to current layer algorithm specicfic params
@param      paramMemTabOffset : Current Offset in ALG_LAYERS_PARAMS_BUFF_MEMREC
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer ouput Pointers list
@param      TIDLLayersBuf: Temp structure to store outDataSize and Data Id's
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_roiPoolingInit(
    const TIDL_CreateParams *params, int32_t layerIdx,
    sTIDL_AlgLayer_t *algLayer, int32_t *paramMemTabOffset,
    int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
    void **outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  int32_t status = IALG_EOK;

  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
    outDataSize = TIDL_findFreeOutBuff(params, layerIdx,
                                       outDataSize, TIDLLayersBufPtr);
    TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }

  if (status == IALG_EOK)
  {
    TIDL_outBuffInit(params, layerIdx, dataMemTabOffset, memRec, outPtr, outDataSize);
  }
  return status;
}
#endif

int32_t TIDL_elementPromotionEnabledInAvgPool(const sTIDL_Network_t *net, sTIDL_Layer_t *tidlLayer)
{
  sTIDL_PoolingParams_t *poolParams = &tidlLayer->layerParams.poolParams;

  int32_t promoteModeOn = 0;
  if (net->quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
  {
    if (((poolParams->kernelW == 3) && (poolParams->kernelH == 3)) && ((poolParams->strideW == 2) && (poolParams->strideH == 2)))
    {
      promoteModeOn = 1;
    }
  }
  return promoteModeOn;
}

#if defined TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_getScaleMemSize(sTIDL_PoolingParams_t *poolParams,
                             int32_t outWidth,
                             int32_t outHeight,
                             int32_t outLinePitch,
                             int32_t scaleFact,
                             int32_t elementSize,
                             int32_t scaleElementSize,
                             int32_t promoteModeOn)
{
  int32_t scaleMemSize;
  if (promoteModeOn)
  {
    elementSize *= 2;
  }
  if ((poolParams->kernelW == 0) && (poolParams->kernelH == 0))
  {
    scaleMemSize = 0;
  }
  else if (poolParams->poolingType == TIDL_AveragePooling)
  {
    if ((poolParams->strideW == 2) && (poolParams->strideH == 2) &&
        (((poolParams->kernelW == 2) && (poolParams->kernelH == 2)) ||
         ((poolParams->kernelW == 3) && (poolParams->kernelH == 3))))
    {
      scaleMemSize = 2 * outWidth * elementSize;
    }
    else
    {
      scaleMemSize = outHeight * outLinePitch * scaleElementSize * scaleFact;
    }
  }
  else
  {
    scaleMemSize = 0;
  }

  return scaleMemSize;
}
#endif

#if defined TIDL_DYNAMIC_SHAPE
/**
 * @brief This function returns resolves the output shape of a layer
 *
 * @param inBufs : Input buffers
 * @param outBufs : Ouput buffers
 * @param intAlgHandle : Algorithm Instance handle
 * @param algLayer : Pointer to current layer algorithm specific params
 * @param TIDLLayer : Pointer to common layer parameters
 *
 * @return int32_t : returns the status
 */
int32_t TIDL_resolveLayerOutputShape(IVISION_InBufs *inBufs,
                                     IVISION_InBufs *outBufs,
                                     TIDL_Handle intAlgHandle,
                                     sTIDL_AlgLayer_t *algLayer,
                                     sTIDL_Layer_t *TIDLLayer)
{
  int32_t status = IALG_EOK;
  int32_t shapeStatus = TIDL_SHAPE_INFERENCE_OK;
  
  sTIDL_DataParams_t *inDataParams[TIDL_MAX_ALG_IN_BUFS];
  int32_t numFound = 0;
  
  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_NUMINBUFS_ZERO)
  {
    TIDLLayer->numInBufs = 0;
  }

  if (TIDLLayer->numInBufs > 0)
  {
    numFound = TIDL_getMultipleDataParams(intAlgHandle->createParams->net,
                                          TIDLLayer->inData,
                                          TIDLLayer->numInBufs,
                                          inDataParams);
    
    if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_NUMFOUND_MISMATCH)
    {
      numFound = TIDLLayer->numInBufs - 1; /* Force mismatch */
    }

    if (numFound != TIDLLayer->numInBufs)
    {
      tidl_printf(0, "TIDL_resolveLayerOutputShape: Failed to resolve all input data ids. Found %d out of %d\n",
                  numFound, TIDLLayer->numInBufs);
      status = IALG_EFAIL;
    }

    /* Perform shape inference if all inputs resolved successfully */
    if (status == IALG_EOK)
    {
      int32_t            layerType = TIDLLayer->layerType;
      sTIDL_DataParams_t *outData  = &TIDLLayer->outData;

      if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_SHAPE_INFER_OUTDATAPARAM_NULL)
      {
        outData = NULL;
      }

      if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_SHAPE_INFER_LAYERTYPE_INVALID)
      {
        layerType = -1; /* NULL shape infer function */
      }

      if ((intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_SHAPE_INFER_GATHERND_INDATAPARAMS1_NULL) &&
          (layerType == TIDL_GatherNDLayer))
      {
        inDataParams[1] = NULL; /* Force NULL indices param for GatherND shape inference negative test */
      }

      shapeStatus = TIDL_inferShapeGeneric(layerType,
                                          &TIDLLayer->layerParams,
                                          inDataParams,
                                          TIDLLayer->numInBufs,
                                          outData,
                                          NULL);  /* NULL context for runtime */

      if (shapeStatus == TIDL_SHAPE_INFERENCE_OK)
      {
        if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_RECALCPITCH_DATAPARAM_NULL)
        {
          outData = NULL;
        }
  
        if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_RECALCPITCH_PADW_NONZERO)
        {
          outData->padW = 1;
        }

        TIDL_recalcDataParamsPitch(outData);
      }
      else if (shapeStatus == TIDL_SHAPE_INFERENCE_ERR_UNSUPPORTED_LAYER)
      {
        tidl_printf(1, "TIDL_resolveLayerOutputShape: Runtime Shape inference for layer type %d is not registered\n",
                    TIDLLayer->layerType);
      }
      else
      {
        tidl_printf(0, "TIDL_resolveLayerOutputShape: Runtime Shape inference failed with status %d for layer type %d\n",
                    shapeStatus, TIDLLayer->layerType);
        status = IALG_EFAIL;
      }
    }
  }
  else
  {
    tidl_printf(0, "TIDL_resolveLayerOutputShape: Number of input buffer is zero\n");
    status = IALG_EFAIL;
  }

  return status;
}
#endif

/**
 * @brief This function returns the input buffer pointers for the layer
 *
 * @param inBufs : Inpupt buffers
 * @param outBufs : Ouput buffers
 * @param intAlgHandle : Algorithm Instance handle
 * @param algLayer : Pointer to current layer algorithm specific params
 * @param TIDLLayer : Pointer to common layer parameters
 * @param inPtrs : input buffer pointers
 * @param i : layer Index
 * @param orgNumBatches : Array of original number of batches for each input buffer of curr layer,
 *                        this will only be used for writing traces for Data Layer
 * @return int32_t : returns the status
 */
int32_t TIDL_getLayerInPtrs(
    IVISION_InBufs *inBufs,
    IVISION_InBufs *outBufs,
    TIDL_Handle intAlgHandle,
    sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *TIDLLayer,
    void *inPtrs[],
    int32_t i,
    int32_t orgNumBatches[])
{
  int32_t j, twStatus = IALG_EOK;
  int32_t relativeCoreId = GET_RELATIVE_COREIDX(intAlgHandle->createParams->coreId, intAlgHandle->createParams->coreStartIdx);

  for (j = 0; ((j < TIDLLayer->numInBufs) && (twStatus == IALG_EOK)); j++)
  {
    int32_t curInDataId = TIDLLayer->inData[j];
#ifdef HOST_EMULATION
    if (intAlgHandle->createParams->optimiseExtMem != TIDL_OptimiseExtMemL0)
    {
      curInDataId = (int32_t)intAlgHandle->TIDLLayersBuf->newInDataId[i][j];
    }
#endif

    if (algLayer->isInData[j] == 1)
    {
      sTIDL_DataParams_t *inDataParams;
      inDataParams = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].outData;
      if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_INPTRS_INDATAPARAMS_NULL) /* forcing inDataParams = NULL to check the negative case of TIDL_ERR_NULL_POINTER */
      {
        inDataParams = NULL;
      }
      if ((intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_INPTRS_GC_TRACE_SCRATCHSIZE_INVALID) ||
          (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_INPTRS_NC_TRACE_SCRATCHSIZE_INVALID)) /* forcing valid max scratch size to skip this case */
      {
        intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = TRACE_STRING_SIZE * TRACE_STRINGS_MEM_SIZE;
      }
      if (inDataParams != NULL)
      {
        if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_PITCHWIDTH_INVALID)
        {
          inDataParams->pitch[TIDL_LINE_PITCH] = inDataParams->pitch[TIDL_LINE_PITCH] + 2;
        }
        inPtrs[j] =
            TIDL_getDataBuff(inBufs->bufDesc, inBufs->numBufs, curInDataId, inDataParams->dimValues[TIDL_DIM_BATCH], inDataParams->pitch);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: The condition in this if statement is necessary to ensure that the code does not attempt to write trace data when the input pointer is NULL, 
        which could lead to undefined behavior. This check is crucial for robustness and safety, especially in scenarios where the input data may not be available or valid.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if (inPtrs[j] != NULL)
        {
          /* LDRA_JUSTIFY_END */
          TIDLLayer->inData[j] = intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].outData.dataId;
          twStatus = tidl_writeTraceDataBuf((int8_t *)inPtrs[j],
                                            intAlgHandle->createParams->net,
                                            algLayer,
                                            curInDataId,
                                            (int8_t *)intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base,
                                            intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size,
                                            0,
                                            0,
                                            0,
                                            inDataParams->dimValues[TIDL_DIM_HEIGHT],
                                            inDataParams->dimValues[TIDL_DIM_NUMCH],
                                            orgNumBatches[j],
                                            orgNumBatches[j],
                                            inDataParams->pitch[TIDL_CHANNEL_PITCH],
                                            relativeCoreId,
                                            intAlgHandle->createParams->traceBaseName,
                                            1);
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start>
          Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
          Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
          This does not impact functional correctness or safety.
          <justification end> */
          if (twStatus != IALG_EOK)
          {
            tidl_printf(0, "Trace write failed\n");
          }
          /* LDRA_JUSTIFY_END */
        }
      }
      else
      {
        inPtrs[j] = NULL;
        tidl_printf(0, "TIDL_getLayerInPtrs: Invalid inDataParams for dataId %d\n", curInDataId);
        twStatus = TIDL_ERR_NULL_POINTER;
        break;
      }
    }
    else if (algLayer->isInOutData[j] == 1)
    {
      sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(intAlgHandle->createParams->net,
                                                            TIDLLayer->inData[j]);

      inPtrs[j] =
          TIDL_getDataBuff(outBufs->bufDesc, outBufs->numBufs, curInDataId, inDataParams->dimValues[TIDL_DIM_BATCH], inDataParams->pitch);
    }
    else
    {
      if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_INPTRS_GC_TRACE_SCRATCHSIZE_INVALID) /* forcing invalid scratch size to check the negative case */
      {
        intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = 0;
      }
      if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_INPTRS_NC_TRACE_SCRATCHSIZE_INVALID) /* forcing invalid scratch size to check the negative case */
      {
        intAlgHandle->gcHelperHandle = NULL;
        intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = 0;
      }
      if (intAlgHandle->gcHelperHandle != NULL)
      {
        /*TIDL_LDRA_TAG_ALGUTILS_PRIOR_CHECK_001*/
        // const sTIDL_AlgLayer_t *inAlgLayer = get_AlgLayer_t_pointer(intAlgHandle->alglayerParams, algLayer->inWorkloadIdx[j]);

        inPtrs[j] = TIDL_getBasePointerFromGCUsingBufIdx(intAlgHandle->gcHelperHandle,
                                                         &intAlgHandle->sysScratchPtr,
                                                         algLayer->inWorkloadIdx[j]);

        /* Code to print the traces for the const data layer */
        if ((intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].layerType == TIDL_ConstDataLayer) && (inPtrs[j] != NULL))
        {
          sTIDL_Layer_t &constLayer = intAlgHandle->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[j]];
          twStatus = tidl_writeTraceDataBuf((int8_t *)inPtrs[j],
                                            intAlgHandle->createParams->net,
                                            algLayer,
                                            constLayer.outData.dataId,
                                            (int8_t *)intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base,
                                            intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size,
                                            0,
                                            0,
                                            0,
                                            constLayer.outData.dimValues[TIDL_DIM_HEIGHT],
                                            constLayer.outData.dimValues[TIDL_DIM_NUMCH],
                                            orgNumBatches[j],
                                            orgNumBatches[j],
                                            constLayer.outData.pitch[TIDL_CHANNEL_PITCH],
                                            relativeCoreId,
                                            intAlgHandle->createParams->traceBaseName,
                                            1);

          if (twStatus != IALG_EOK)
          {
            tidl_printf(0, "Trace write failed\n");
            break;
          }
        }
      }
      else
      {
        if (intAlgHandle->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[j]].layerType != TIDL_ConstDataLayer)
        {
          inPtrs[j] = (uint8_t *)intAlgHandle->dataBuf[curInDataId];
        }
        else
        {
          int32_t algIdx = getAlgLyrIdx(intAlgHandle->alglayerParams, (int32_t)algLayer->inLayerIdx[j]);
          sTIDL_AlgLayer_t *constalgLayer = get_AlgLayer_t_pointer(intAlgHandle->alglayerParams, algIdx);
          inPtrs[j] = constalgLayer->layerParams.constDataParams.paramPtr;

          sTIDL_Layer_t &constLayer = intAlgHandle->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[j]];
          twStatus = tidl_writeTraceDataBuf((int8_t *)inPtrs[j],
                                            intAlgHandle->createParams->net,
                                            algLayer,
                                            constLayer.outData.dataId,
                                            (int8_t *)intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base,
                                            intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size,
                                            0,
                                            0,
                                            0,
                                            constLayer.outData.dimValues[TIDL_DIM_HEIGHT],
                                            constLayer.outData.dimValues[TIDL_DIM_NUMCH],
                                            orgNumBatches[j],
                                            orgNumBatches[j],
                                            constLayer.outData.pitch[TIDL_CHANNEL_PITCH],
                                            relativeCoreId,
                                            intAlgHandle->createParams->traceBaseName,
                                            1);
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: This condition is necessary to handle the scenario where the trace write operation fails, 
          ensuring that the error is properly reported and handled.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if (twStatus != IALG_EOK)
          {
            /* LDRA_JUSTIFY_END */
            tidl_printf(0, "Trace write failed\n");
          }
        }
      }
    }
  }

#ifdef HOST_EMULATION
  if (twStatus == IALG_EOK)
  {
    if (((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
    {
      if (intAlgHandle->createParams->net->dataFlowInfo != 0)
      {
        int32_t totalBufSize = 0;
        for (j = 0; j < TIDLLayer->numInBufs; j++)
        {
          sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(intAlgHandle->createParams->net,
                                                                TIDLLayer->inData[j]);

          int32_t dataElementSize = TIDL_getDatElementSize(inDataParams->elementType);
          int32_t totalBatchesToCopy = inDataParams->dimValues[TIDL_DIM_BATCH];
          /* For first only 16-bit BN layer dataElementSize is different from that of rest of the layers in the network */
          if ((TIDL_getDatElementSize(inDataParams->elementType) != TIDL_getDatElementSize(TIDLLayer->outData.elementType)) &&
              (TIDLLayer->layerType == TIDL_BatchNormLayer))
          {
            dataElementSize = TIDL_getDatElementSize(inDataParams->elementType);
          }
          int32_t numBytesToCopy = inDataParams->pitch[TIDL_ROI_PITCH] * dataElementSize * totalBatchesToCopy;
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start> PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
           This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
           TIDL_LDRA_TAG :  TIDL_LDRA_TAG_ALGUTILS_PRIOR_CHECK_001
          <justification end> */
          if ((intAlgHandle->gcHelperHandle != NULL))
          {
            const sBufParams_t *bufParams = getBufParamsFromBufIndex(intAlgHandle->gcHelperHandle, algLayer->inWorkloadIdx[j]);
            numBytesToCopy = bufParams->bufSize * dataElementSize;
          }
          /* LDRA_JUSTIFY_END */
          (void)memcpy(intAlgHandle->refScratchBuf + totalBufSize, inPtrs[j], numBytesToCopy);
          inPtrs[j] = intAlgHandle->refScratchBuf + totalBufSize;
          totalBufSize += numBytesToCopy;
        }
      }
    }
  }
#endif // TIDL_DYNAMIC_SHAPE

  return twStatus;
}
#ifdef HOST_EMULATION
/**
 * @brief This function zeros out the input buffer with zeroPoint values
 *
 * @param inPtr : Input buffer
 * @param zeroPoint: Zeropoint value to zero with
 * @param bufSize : Buffer size
 */
template<class Tin> void TIDL_zeroOutputTensors(Tin *inPtr, Tin zeroPoint, uint32_t bufSize)
{
  uint32_t i0;
  for (i0 = 0; i0 < bufSize; i0++)
  {
    inPtr[i0] = zeroPoint;
  }
}
#endif
/**
 * @brief This function returns the input buffer pointers for the layer
 *
 * @param outBufs : Output buffers
 * @param intAlgHandle : Algorithm Instance handle
 * @param algLayer : Pointer to current layer algorithm specific params
 * @param TIDLLayer : Pointer to common layer parameters
 * @param outPtrs : output buffer pointers
 * @param i : layer Index
 * @return int32_t : returns the status
 */
int32_t TIDL_getLayerOutPtrs(
    IVISION_InBufs *outBufs,
    TIDL_Handle intAlgHandle,
    const sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *TIDLLayer,
    void *outPtrs[],
    int32_t i)
{
  int32_t j, status = IALG_EOK;

  for (j = 0; j < TIDLLayer->numOutBufs; j++)
  {
    int32_t outDataId = TIDLLayer->outData.dataId;
    if (algLayer->isOutData[j] == 1)
    {
      outPtrs[j] = (uint8_t *)
          TIDL_getDataBuff(outBufs->bufDesc, outBufs->numBufs, outDataId, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH], TIDLLayer->outData.pitch);
      if (outPtrs[j] == NULL)
      {
        TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_INVALID_IO_LINE_PITCH);
        status = IALG_EFAIL;
        break;
      }
    }
    else
    {
#ifdef HOST_EMULATION
      if (intAlgHandle->gcHelperHandle == NULL)
      {
        outPtrs[j] = (uint8_t *)intAlgHandle->dataBuf[outDataId];
        if (outPtrs[j] == NULL)
        {
          int32_t curOutDataId = (int32_t)intAlgHandle->TIDLLayersBuf->newOutDataId[i][j];
          outPtrs[j] = (uint8_t *)intAlgHandle->dataBuf[curOutDataId];
          intAlgHandle->dataBuf[outDataId] = (uint8_t *)(outPtrs[j]);
        }
      }
      else
#endif
      {
        outPtrs[j] = TIDL_getBasePointerFromGCUsingBufIdx(intAlgHandle->gcHelperHandle,
                                                          &intAlgHandle->sysScratchPtr,
                                                          algLayer->outBufIdxWl);
      }
    }

#ifdef HOST_EMULATION
    if (((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
    {
      /* This is required so that padded regions are filled with zero's as Padding is done only
      when NC information is available */
      if (intAlgHandle->createParams->net->dataFlowInfo == 0)
      {
        int32_t dataElementSize = TIDL_getDatElementSize(TIDLLayer->outData.elementType);
        /* Assign to zero point */
        if (dataElementSize > 1)
        {
          if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_OUTPRTS_INVALID_DATATYPE) /* forcing invalid elementType to check the negative case */
          {
            TIDLLayer->outData.elementType = TIDL_SignedDoubleWord + 1;
          }
          if (TIDLLayer->outData.elementType == TIDL_SignedShort)
          {
            TIDL_zeroOutputTensors((int16_t *)outPtrs[j], (int16_t)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else if (TIDLLayer->outData.elementType == TIDL_UnsignedShort)
          {
            TIDL_zeroOutputTensors((uint16_t *)outPtrs[j], (uint16_t)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
           else if (TIDLLayer->outData.elementType == TIDL_BFloat16)
          {
            TIDL_zeroOutputTensors((bfloat16_tidl *)outPtrs[j], (bfloat16_tidl)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else if (TIDLLayer->outData.elementType == TIDL_SinglePrecFloat)
          {
            TIDL_zeroOutputTensors((float32_tidl *)outPtrs[j], (float32_tidl)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else if (TIDLLayer->outData.elementType == TIDL_SignedWord)
          {
            TIDL_zeroOutputTensors((int32_t *)outPtrs[j], (int32_t)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else if (TIDLLayer->outData.elementType == TIDL_UnsignedWord)
          {
            TIDL_zeroOutputTensors((uint32_t *)outPtrs[j], (uint32_t)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else if (TIDLLayer->outData.elementType == TIDL_SignedDoubleWord)
          {
            TIDL_zeroOutputTensors((int64_t *)outPtrs[j], (int64_t)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else if (TIDLLayer->outData.elementType == TIDL_UnsignedDoubleWord)
          {
            TIDL_zeroOutputTensors((uint64_t *)outPtrs[j], (uint64_t)TIDLLayer->outData.tensorZeroPoint, TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] * TIDLLayer->outData.pitch[TIDL_ROI_PITCH]);
          }
          else
          {
            /*Unsupported*/
            status = IALG_EFAIL;
          }
        }
        else
        {
          (void)memset(outPtrs[j], TIDLLayer->outData.tensorZeroPoint,
                       TIDLLayer->outData.dimValues[TIDL_DIM_BATCH] *
                           TIDLLayer->outData.pitch[TIDL_ROI_PITCH] *
                           dataElementSize);
        }
      }
    }
#endif
  }
  return status;
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_copyPersistentWeightsToIntMem.* <function end>
<justification start>
Rationale - FUTURE_USE: This function is present to support future testing scenarios and it is retained for robustness and exception handling..
Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
This does not impact functional correctness or safety.
<justification end> */

/**
 * @brief This function copies persistent weights to internal memory
 *
 * @param intAlgHandle : Algorithm Instance handle
 */
int32_t TIDL_copyPersistentWeightsToIntMem(TIDL_Handle intAlgHandle)
{
  int32_t status = TIDL_SUCCESS;
  if ((intAlgHandle->gcHelperHandle != NULL) &&
      (((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U))
  {
    /* Need to get group number somehow */
    int32_t groupId = intAlgHandle->groupId;
    if (groupId <= (intAlgHandle->wlSuperGrp->numGroups - 1))
    {
      int32_t wlActivateGrpIdx;
      sWorkloadGroup_t *wlGroup = &intAlgHandle->wlSuperGrp->workloadGrpList[groupId];
      sGCHelperHandle *gcHelperHandle = intAlgHandle->gcHelperHandle;

      wlActivateGrpIdx = wlGroup->activateGroup.startIndexWLUnitDB;

      while (wlActivateGrpIdx <= wlGroup->activateGroup.endIndexWLUnitDB)
      {
        sWorkloadUnit_t *workloadUnit = getWLUnitPtr(gcHelperHandle, wlActivateGrpIdx);
        if ((workloadUnit != NULL))
        {
          for (int32_t linkIdx = 0; (linkIdx < workloadUnit->numLinks); linkIdx++)
          {
            sLink_t *linkPtrList[MAX_LINKS_PER_WL];
            getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
            sLink_t *link = linkPtrList[0];
            if (link->type == (int32_t)LINK_PROC)
            {
              status = TIDL_ERROR_ALGUTILS_INVALID_LNKTYPE_ERROR;
              tidl_printf(0, "Process link with id = %d not supported during activate\n", link->id);
              break;
            }
            else
            {
              int8_t *pSrcMem = (int8_t *)TIDL_getPointerFromGC(&(link->src[0]), gcHelperHandle, (TIDL_sysScratchPtr *)&intAlgHandle->sysScratchPtr);
              int8_t *pDstMem = (int8_t *)TIDL_getPointerFromGC(&(link->sink), gcHelperHandle, (TIDL_sysScratchPtr *)&intAlgHandle->sysScratchPtr);

              int32_t srcElemSize = getByteCnt(getBufParamsFromBufIndex(gcHelperHandle, link->src[0].bufDBindex)->dataType);
              int32_t dstElemSize = getByteCnt(getBufParamsFromBufIndex(gcHelperHandle, link->sink.bufDBindex)->dataType);

              if (link->src[0].flowCnt[FLOW_PIPELINE][0] != 0)
              {
                (void)TIDL_memcpy2D(pDstMem,
                                    pSrcMem,
                                    link->src[0].flowCnt[FLOW_PIPELINE][0] * srcElemSize,
                                    link->src[0].flowCnt[FLOW_PIPELINE][1],
                                    link->sink.flowDim[FLOW_PIPELINE][0] * dstElemSize,
                                    link->src[0].flowDim[FLOW_PIPELINE][0] * srcElemSize,
                                    intAlgHandle->dmaUtilsContext,
                                    intAlgHandle->memcpyTr);
              }
            }
          }
        }
        else
        {
          status = TIDL_ERROR_ALGUTILS_INVALID_GRPID_ERROR; /*input for grpidx is grpid so keeping same error code as grpid*/
        }

        wlActivateGrpIdx++;
      }
    }
    else
    {
      status = TIDL_ERROR_ALGUTILS_INVALID_GRPID_ERROR;
    }
  }
  return status;
}

#if TIDL_DEVICE_MULTICORE
void TIDL_setMultiCoreCtxtLinkIndex(TIDL_Handle algHandle, sTIDL_AlgLayer_t *algLayer, int32_t currAlgLayer)
{
  int32_t numMultiCoreCtxtLink = 0;
  sLink_t *link;
  sWorkloadUnit_t *pWLUnit = getWLUnitPtr(algHandle->gcHelperHandle, currAlgLayer);
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];
  /* LDRA_JUSTIFY_START
   <metric start> statement branch <metric end>
   <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
   <justification end> */
  if ((pWLUnit != NULL))
  {
    /* LDRA_JUSTIFY_END */
    for (int32_t linkIdx = 0; (linkIdx < pWLUnit->numLinks); linkIdx++)
    {
      getLinkPtrs(pWLUnit, NOT_VALID, linkIdx, linkPtrList);
      link = linkPtrList[0];
      if (link->subType == (int32_t)LINK_X_FM_CTXT_ACROSS_CORE)
      {
        algLayer->multiCoreCtxtLinkIndex[numMultiCoreCtxtLink] = linkIdx;
        numMultiCoreCtxtLink++;
      }
    }
  }
}
#endif

void TIDL_getPadParams(WorkloadUnitExec_padParams *padParams,
                       sTIDL_Layer_t *tidlLayer,
                       int32_t flowCtrl,
                       const sWorkloadUnit_t *workloadUnit,
                       const sGCHelperHandle *gcHelperHandle)
{
  (void)memset(padParams, 0, sizeof(*padParams));
  sTIDL_DataParams_t *dataParams = &tidlLayer->outData;

  padParams->flowCtrl = flowCtrl;

  padParams->outPadOffset = ((dataParams->padH) * dataParams->pitch[TIDL_LINE_PITCH]) + (dataParams->padW);
  padParams->outOffset = padParams->outPadOffset;
  padParams->elemSize = TIDL_getDatElementSize(tidlLayer->outData.elementType);
  padParams->outChannels = tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
  padParams->isFirstTile = 1; // NO_TILE case exhibit property of FIRST_TILE and LAST_TILE so setting to 1 by default
  padParams->isLastTile = 1;  // NO_TILE case exhibit property of FIRST_TILE and LAST_TILE so setting to 1 by default

  /* This is a temporary implementation for getting pad info from workload until pad links are made
      available */
  if ((workloadUnit != NULL) && (gcHelperHandle != NULL))
  {
    sWLMetaDataID_t wlMetaData = {0, 0, 0, 0, 0, 0, 0};
    getWLMetaDataID(workloadUnit->metadata, &wlMetaData);

    padParams->tileType = wlMetaData.tileType;
    padParams->outHeight = wlMetaData.outTileHeight;

    padParams->isFirstTile = wlMetaData.isFirstTile;
    padParams->isLastTile = wlMetaData.isLastTile;

    for (int32_t linkIdx = 0; linkIdx < workloadUnit->numLinks; linkIdx++)
    {
      sLink_t *linkPtrList[MAX_LINKS_PER_WL];
      getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
      sLink_t *link = linkPtrList[0];

      if ((getBufParamsFromBufIndex(gcHelperHandle, link->sink.bufDBindex)->type == (int32_t)BUF_FM_FULL) &&
          !((link->subType == (int32_t)LINK_X_FM_CTXT_RST) || (link->subType == (int32_t)LINK_X_FM_CTXT_ACROSS_CORE)))
      {
        if (padParams->outBufParams == NULL)
        {
          padParams->outOffset = link->sink.offset;
          padParams->superOffset = link->sink.superOffset;
          padParams->outBufParams = getBufParamsFromBufIndex(gcHelperHandle, link->sink.bufDBindex);
          padParams->outChannels = padParams->outBufParams->bufHeight;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else if (padParams->outBufParams == getBufParamsFromBufIndexStrict(gcHelperHandle, link->sink.bufDBindex))
        /* LDRA_JUSTIFY_END */
        {
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start>
          Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
          Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
          This does not impact functional correctness or safety.
          <justification end> */
          /* Concat layer with no output DMA may show this situation where proc links point to the same buffer */
          if (link->sink.offset <= padParams->outOffset)
          {
            /* the lowest offset in the out buffer indicates where the first valud pixel starts and hence where
              row pad should end */
            padParams->outOffset = link->sink.offset;
            padParams->superOffset = link->sink.superOffset;
          }
          /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else
        {
          /*do nothing*/
        }
        /* LDRA_JUSTIFY_END */
      }
    }
  }
}

int32_t TIDL_getDataFlowType(const sWorkloadUnit_t *workloadUnit,
                             const sGCHelperHandle *gcHelperHandle)
{
  int32_t dataFlowType;
  sWLMetaDataID_t WLMetaDataID;
  getWLMetaDataID(workloadUnit->metadata, &WLMetaDataID);
  dataFlowType = WLMetaDataID.dataFlowType;

  return dataFlowType;
}

int32_t TIDL_switchHandles(void *algHandle)
{
  int32_t retVal = IALG_EOK;
  TIDL_Handle handle = (TIDL_Handle)algHandle;
  void *lastActiveHandle = TIDL_getLastActiveHandle(handle->createParams->coreId);
#if defined(SOC_J721S2) || defined(SOC_AM62A)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
This does not impact functional correctness or safety.
<justification end> */
#endif
  if ((lastActiveHandle != handle) && (lastActiveHandle != NULL))
  {
    TIDL_deactivate((IALG_Handle)lastActiveHandle);
  }
#if defined(SOC_J721S2) || defined(SOC_AM62A)
/* LDRA_JUSTIFY_END */
#endif
  if ((lastActiveHandle != handle) || (handle->algState == (uint8_t)ALG_NOT_ACTIVE))
  {
    TIDL_activate((IALG_Handle)(void *)handle);
    if (handle->algState != (uint8_t)ALG_ACTIVE)
    {
      retVal = TIDL_ERROR_ALGUTILS_FAIL_ACTIVATE_ERROR;
    }
    TIDL_setLastActiveHandle(handle->createParams->coreId, handle);
  }
  return retVal;
}
 
int32_t TIDL_referencFlow(const TIDL_CreateParams *pCreateParams)
{
  int32_t refFlow = 0;
#ifdef HOST_EMULATION
  if ((((uint32_t)pCreateParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) != 0U) ||
      (((uint32_t)pCreateParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U))
  {
    refFlow = 1;
  }
#endif

  return refFlow;
}

uint32_t TIDL_getSizeNet(const TIDL_CreateParams *pCreateParams)
{
  uint32_t size = 128;
  /** we need the basic structure and compiled part of network always
   * and additionally the parameter part in case of reference flow
   */
  if (pCreateParams->net->netInitBackupDataOffset[0] == 0)
  {
    size = pCreateParams->net->netBufInfo[TIDL_NET_STRUCTURE_BUF].size +
           pCreateParams->net->netBufInfo[TIDL_NET_COMPILER_BUF].size +
           pCreateParams->net->netBufInfo[TIDL_NET_GC_BUF].size;
#ifdef HOST_EMULATION
    if (TIDL_referencFlow(pCreateParams) != 0)
    {
      /* TIDL_NET_PARAMS_BUF is allocated memory at the last position in ALG_PERSIST_DDR_NET_MEMREC */
      size = pCreateParams->net->netBufInfo[TIDL_NET_GC_BUF].offset + pCreateParams->net->netBufInfo[TIDL_NET_GC_BUF].size;
    }
#endif
  }
  return size;
}

void *TIDL_CopyAndReturnGCPrivateMemPtr(TIDL_Handle algHandle, const IALG_MemRec memRec[], int32_t isReferenceFlow)
{
  int8_t *src, *dst, *pBaseSrc;
  sTIDL_Network_t *pNet = algHandle->createParams->net;
  pBaseSrc = (int8_t *)pNet;

  src = get_int8_t_pointer(pBaseSrc, pNet->originalGcBufInfoOffset); /* Copy over GC info from the original offset in net to the params offset, new offset is updated below */
  if (pNet->netInitBackupDataOffset[0] == 0)
  {
    int32_t dstPtrOffset; // = pNet->originalGcBufInfoOffset;
#ifdef HOST_EMULATION
    dstPtrOffset = pNet->originalGcBufInfoOffset;
    if (isReferenceFlow != 1)
#endif
    {
      /* For target flow we do not need parameters from the net as they are not
      required during process (whatever is required will be copied in a separate memrec as
      part of either reorder or direct copy), hence use the offset for parameters for storing
      GC. This way we will not be required to allocate memory for parameters */
      dstPtrOffset = pNet->netBufInfo[TIDL_NET_PARAMS_BUF].offset;
    }
    dst = get_int8_t_pointer((int8_t *)memRec[ALG_PERSIST_DDR_NET_MEMREC].base, dstPtrOffset);
    (void)memcpy(dst, src, pNet->netBufInfo[TIDL_NET_GC_BUF].size);
#ifdef HOST_EMULATION
    if (isReferenceFlow != 1)
#endif
    {
      /* For target flow update the offsets for GC as it will be using the offset for parameters*/
      pNet->netBufInfo[TIDL_NET_GC_BUF].offset = pNet->netBufInfo[TIDL_NET_PARAMS_BUF].offset;
    }
  }
  else
  {
    dst = src;
  }
  return dst;
}

int32_t TIDL_MoveNetToPrivateMemory(TIDL_Handle algHandle,
                                    const IALG_MemRec memRec[], int32_t relCoreId)
{
  int32_t status = IALG_EOK;
  sTIDL_Network_t *pNet = algHandle->createParams->net;
  if (pNet->netInitBackupDataOffset[0] == 0)
  {
    int32_t size;
    uint32_t bufIdx;
    int8_t *src, *dst, *pBaseSrc;
    int32_t isReference = TIDL_referencFlow(algHandle->createParams);

    pBaseSrc = (int8_t *)pNet;
    for (bufIdx = TIDL_NET_STRUCTURE_BUF; bufIdx < TIDL_NET_TOTAL_BUF; bufIdx++)
    {
      if ((isReference != 0) ||
          (bufIdx == TIDL_NET_STRUCTURE_BUF) ||
          (bufIdx == TIDL_NET_COMPILER_BUF))
      {
        src = get_int8_t_pointer(pBaseSrc, pNet->netBufInfo[bufIdx].offset);
        dst = get_int8_t_pointer((int8_t *)memRec[ALG_PERSIST_DDR_NET_MEMREC].base,
                                 pNet->netBufInfo[bufIdx].offset);
        size = pNet->netBufInfo[bufIdx].size;
        (void)memcpy(dst, src, size);
      }
    }
    algHandle->createParams->net = (sTIDL_Network_t *)memRec[ALG_PERSIST_DDR_NET_MEMREC].base;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: This condition is necessary to ensure that the data flow information is valid before attempting to access it,
    preventing potential null pointer dereferences or invalid memory accesses.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if (algHandle->createParams->net->dataFlowInfo != 0)
    {
      /* LDRA_JUSTIFY_END */
      sGraphCompilerOutArgs_t *gcOutArgs = NULL;
      sGCDataBase_t *gcDatabase = NULL;
      sGCCommonDataBase_t *gcCommonDataBase = NULL;
      sWorkloadSuperGroup_t *wlSuperGrp = NULL;
      sGraphCompilerOutArgsCore_t *gcOut = NULL;
      // TODO: [8.5 rebase]. Make this similar to perfsimInfo

      gcOutArgs = (sGraphCompilerOutArgs_t *)TIDL_getNetObjBufPtr(algHandle->createParams->net, memRec, TIDL_NET_GC_BUF);

      gcOut = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs, (int32_t)DATABASE_GCOUT_1 + relCoreId);
      gcDatabase = &gcOut->dataBase;
      gcCommonDataBase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs, DATABASE_COMMON);
      wlSuperGrp = &gcOut->superWorkload;
      algHandle->wlSuperGrp = wlSuperGrp;

      sBufParams_t *bufNodesDB = (sBufParams_t *)&gcCommonDataBase->dbPayLoad[gcCommonDataBase->dbParams[DB_COMMON_BUF].startOffset];
      sWorkloadUnit_t *workLoadUnitDB = (sWorkloadUnit_t *)&gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_WL].startOffset];
      sWorkloadUnitAuxilary_t *auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t *)&gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_AUXWL].startOffset];

      gcOutArgs->commonDatabaseInfo.bufDB.pBufList = bufNodesDB;
      gcOut->dataBaseInfo.wlDB.pWLList = workLoadUnitDB;
      gcOut->dataBaseInfo.wlAuxDB.pWLAuxList = auxilaryWorkloadDB;
    }
  }
  else
  {
    sTIDL_AlgLayer_t *alglayerParams = algHandle->alglayerParams;
    sTIDL_DetectOutputParams_t *pDetParams;
    sTIDL_ALgDetectOutputParams_t *algDetLyrParams = NULL;

    for (int32_t i = 0; i < pNet->numLayers; i++)
    {

      switch (pNet->TIDLLayers[i].layerType)
      {
      case TIDL_DetectionOutputLayer:
#ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
      case TIDL_OdPostProcessingLayer:
#endif
      {
        // Find the alg Layer Params for this layer
        for (int32_t j = 0; (j < pNet->numLayers) && (alglayerParams != NULL); j++)
        {
          if (alglayerParams[j].layerIdx == i)
          {
            algDetLyrParams = &alglayerParams[j].layerParams.detectionOutputParams;
            break;
          }
        }
        pDetParams = &pNet->TIDLLayers[i].layerParams.detectOutParams;
        if (algDetLyrParams != NULL)
        {
          algDetLyrParams->priorBoxPtr = (void *)get_int8_t_pointer((int8_t *)pNet,
                                                                    pDetParams->priorBox);
        }
        else
        {
          status = TIDL_ERR_NULL_POINTER;
        }
        break;
      }
      default:
      {
        break;
      }
      }
    }
  }
  return status;
}

void *TIDL_getNetObjBufPtr(sTIDL_Network_t *pNet,
                           const IALG_MemRec memRec[],
                           uint32_t netMember)
{
  void *netMemberSrcAddr = NULL;
  void *netMemberDstAddr = NULL;
  int32_t size = 0;
  int32_t offset = 0;
  int32_t checkflag = 1;
#if defined TIDL_COVERAGE_DEAD_CODE
  if (netMember >= TIDL_NET_TOTAL_BUF)
  {
    tidl_printf(0, "Member not available as part of net structure \n");
    assert(0);
  }
#endif

  size = pNet->netBufInfo[netMember].size;
  offset = pNet->netBufInfo[netMember].offset;

  netMemberSrcAddr = (uint8_t *)get_int8_t_pointer((int8_t *)(pNet), offset);
  netMemberDstAddr = netMemberSrcAddr;
  if (pNet->netInitBackupDataOffset[0] != 0)
  {
    checkflag = 0; // removed memRec to avoid MISRA C violations
  }
  if (checkflag == 1)
  {
    netMemberDstAddr = (uint8_t *)get_int8_t_pointer((int8_t *)memRec[ALG_PERSIST_DDR_NET_MEMREC].base, offset);
    (void)memcpy(netMemberDstAddr, netMemberSrcAddr, size);
  }
  return netMemberDstAddr;
}

void TIDL_forceTargetDeviceNonOTF(uint32_t *targetDevice, int32_t layerType, int32_t convType)
{
  if ((layerType == TIDL_Deconv2DLayer) || (convType == TIDLCONV_TYPE_DECONV))
  {
    *targetDevice &= (~TIDL_OTF_FLAG_BIT);
  }
}

#if defined TIDL_COVERAGE_DEAD_CODE
template<class paramType> void TIDL_setTensorVirtualPad(sTIDL_virtualPad *virtualPad, paramType *params, int32_t targetDevice)
{
  if (TIDL_isPadOTF(targetDevice))
  {
    /*Set the default tensor padding:*/
    virtualPad->padT = params->padT;
    virtualPad->padL = params->padL;
    virtualPad->padR = params->padR;
    virtualPad->padB = params->padB;
  }
  else
  {
    virtualPad->padT = 0;
    virtualPad->padL = 0;
    virtualPad->padR = 0;
    virtualPad->padB = 0;
  }
}
#endif
