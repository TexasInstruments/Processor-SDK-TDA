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
@file    tidl_alg_utils.h
@brief   This file contains TILD alg inteface utilities API
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/
#ifndef ITIDL_ALG_UTILS_H
#define ITIDL_ALG_UTILS_H

#include "tidl_device_utils.h"
#include "tidl_device_utils_nc.h"


/* Size of the memory in bytes allocated in DDR for custom layer */
#define TIDL_CUSTOM_LAYER_DDR_SIZE (64U*1024U)

#define TIDL_CONTEXT_STR           (0)
#define TIDL_CONTEXT_RSTR          (1)
#define TIDL_CONTEXT_COPY_MULTI_CORE  (2)




/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_getSysScratchPtr
@brief      Function gives allocates pointer starting from the currOffset and update currOffset
              by accounting for the alignment and size. If basePtr is NULL then this function
              just increments the currOffset without allocating any pointer
@param      basePtr : base pointer from which allocation needs to start. Can be NULL if
                            user is only looking to get the size without allocation
@param      currOffset    : Allocation starts from the currOffset from basePtr, currOffset is
                                    also updated by the function to account for size and alignment
@param      size   : Size of memory to be allocated in basePtr
 @param      alignMent   : ALignment required for the allocation
@param      assignPtr   : Alocated pointer.

@remarks   None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_AllocatePtr(intptr_t basePtr,
                         int32_t *currOffset,
                         int32_t size,
                         int32_t alignMent,
                         void ** assignPtr);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_getSysScratchPtr
@brief      Function gives allocates pointer starting from the currOffset and update currOffset
              by accounting for the alignment and size. If basePtr is NULL then this function
              just increments the currOffset without allocating any pointer
@param      basePtr : base pointer from which allocation needs to start. Can be NULL if
                            user is only looking to get the size without allocation
@param      currOffset    : Allocation starts from the currOffset from basePtr, currOffset is
                                    also updated by the function to account for size and alignment
@param      size   : Size of memory to be allocated in basePtr
 @param      alignMent   : ALignment required for the allocation
@param      assignPtr   : Alocated pointer.

@remarks   None
@return     None
----------------------------------------------------------------------------
*/
//:TODO: THis file is not the right file to have this function
int32_t TIDL_conv2dPersistentMemAllocateAndComputeSize(void   *basePtr,
                                                       const sTIDL_ConvParams_t *conv2dparams,
                                                       const TIDL_LayerSpecificParams *layerSpecificParams,
                                                       const TIDL_NetworkCommonParams   *commonParams,
                                                       int32_t layerIdx,
                                                       sTIDL_AlgLayer_t *algLayer);

//:TODO: THis file is not the right file to have this function
void TIDL_conv2dSetupBuffParams(const sTIDL_ConvParams_t *conv2dparams,
                               const sTIDL_Network_t  * net,
                               sTIDL_AlgLayer_t * algLayer,
                               const sTIDL_Layer_t    * tidlLayer,
                               int32_t layerIdx,
                               tidlConv2dBuffParams_t * buffParams);

#if defined TIDL_COVERAGE_DEAD_CODE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_getSysScratchPtr
@brief      Function gives you a pointer to system memory used to store networks
            intermediate output
@param      bufInfo : Pointer to buffer info. This will come from dataflow
@param      sysScratchPtr    : Pointer which stores the base pointer for all system scratch
@param      baseAccessFlag   : Set it to one to get base pointer of feature map. Set it to zero to
                             go to the location where DMA/CPU will READ/WRITE.

@remarks    None
@return     Pointer to the memory. NULL if  buffers accessor is NONE
----------------------------------------------------------------------------
*/
void * TIDL_getSysScratchPtr(const sBufferInfo_t * bufInfo, const TIDL_sysScratchPtr *sysScratchPtr, uint32_t baseAccessFlag);
#endif
/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_FillPaddedRows
@brief      Fills the pading as per the requirements of the tensor
@param      inPtr : Input data pointer, which is used in wrap around padding
@param      ptr : Pointer to buffer  to fill
@param      bufInfo : Pointer to buffer info. This will come from dataflow

@remarks    None
@return     Pointer to the memory. NULL if  buffers accessor is NONE
----------------------------------------------------------------------------
*/
int32_t  TIDL_FillPaddedRows(uint8_t * inPtr,
                                     uint8_t * ptr,
                                     sBufferInfo_t * bufInfo,
                                     int32_t width,
                                     int32_t height,
                                     int32_t numBatches,
                                     int32_t linePitch,
                                     int32_t channelPitch,
                                     uint32_t batchPitch,
                                     void * dmautilsContext,
                                     uint8_t *zeroVec,
                                     uint8_t *isPadDmaPending,
                                     uint8_t * trMem,
                                     int32_t elementType,
                                     int32_t paddingType,
                                     void * padLayerParams,
                                     int32_t outDataPadH,
                                     int32_t outDataPadW,
                                     int32_t padFillValue,
                                     int32_t isFlowCtrlNatc,
                                     void * perChannelPadValPtr,
                                     sTIDL_DataParams_t * inDataParams); // input data parameters to get dimensions and pitchs for wrap around padding
/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_FillPaddedColsWithZero
@brief      Fills the pading as per the requirements of the tensor
@param      ptr : Pointer to buffer  to fill
@param      bufInfo : Pointer to buffer info. This will come from dataflow

@remarks    None
@return     Pointer to the memory. NULL if  buffers accessor is NONE
----------------------------------------------------------------------------
*/
int32_t  TIDL_FillPaddedCols(uint8_t * ptr, const sBufferInfo_t * bufInfo, int32_t width, int32_t height, int32_t numBatches,
                                     int32_t linePitch, int32_t channelPitch, int32_t batchPitch, int32_t elementType, int32_t flowCtrl, int32_t paddingType,
                                     void * padLayerParams, int32_t outDataPadH, int32_t outDataPadW, int32_t padFillValue, void * perChannelPadValPtr, int32_t inHeight);


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
void * TIDL_getMemoryChunkFromSysmem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX],
  uint32_t size, uint32_t alignment, uint32_t space, uint32_t attribute);


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
void TIDL_resetSysmem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX]);

int32_t TIDL_getDatElementSign(int32_t elementType);


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
int32_t TIDL_isInDataBuff(const sTIDL_Network_t * pTIDLNetStructure,
int32_t dataId, int32_t layersGroupId);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_getLayerNum
@brief      Function to get the layer index of data ID

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Current data buffer ID
@param      layersGroupId     : current algorithm instance Group ID
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_getLayerNum(const sTIDL_Network_t * pTIDLNetStructure, int32_t dataId);



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
void * TIDL_getDataBuff(IVISION_BufDesc *bufDesc[], uint32_t numBufs,
int32_t dataId, int32_t pitch[]);


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
int32_t TIDL_getLayerIDForOutDataID(const sTIDL_Network_t * pTIDLNetStructure,
  int32_t dataId);


int32_t TIDL_updateDataParamsPitch(const sTIDL_DataParams_t  * dataParams);


int32_t TIDL_refGetOutDataSize(const TIDL_NetworkCommonParams *commonParams,
                                const TIDL_LayerSpecificParams *layerSpecificParams,
                                int32_t layerIdx);

int32_t TIDL_refGetScratchDataSize(const TIDL_NetworkCommonParams *commonParams,
                                          int32_t layerIdx);


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
int32_t layerIdx, IALG_MemRec memRec[], int32_t outDataSize);


void TIDL_scratchBufAlloc(IALG_MemRec memRec[], int32_t outDataSize);

void TIDL_scratchBufInit(int32_t layerIdx,
  const IALG_MemRec memRec[], void ** outPtr, int32_t outDataSize);


/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_outBuffInit
@brief      Function to Initialize the Layer output Memory

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      algLayer     : Pointer to current layer algorithm specific params
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer output Pointers list
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
void TIDL_outBuffInit(const TIDL_CreateParams *params,
int32_t layerIdx, int32_t *dataMemTabOffset,
const IALG_MemRec memRec[],const void ** outPtr, int32_t outDataSize);

void TIDL_conv2dSetupBiasPointers(sTIDL_ConvParams_t *conv2dparams,
                                           const TIDL_LayerSpecificParams *specificParams,
                                           const TIDL_NetworkCommonParams *commonParams,
                                           sTIDL_AlgLayer_t               *algLayer,
                                           int32_t layerIdx);

int32_t TIDL_conv2dFillPerChannelParamters(const sTIDL_ConvParams_t *conv2dparams,
                                                     const TIDL_NetworkCommonParams *commonParams,
                                                     const sTIDL_AlgLayer_t               *algLayer,
                                                     int32_t layerIdx);


/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_flattenAlloc
@brief      Function to Request Memory required by the Flatten layer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

int32_t TIDL_flattenAlloc(const TIDL_CreateParams *params,
  int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_flattenInit
@brief      Function to Initialize the EltWise layer specific params

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      algLayer     : Pointer to current layer algorithm specific params
@param      paramMemTabOffset : Current Offset in ALG_LAYERS_PARAMS_BUFF_MEMREC
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer output Pointers list
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

int32_t TIDL_flattenInit(
  const TIDL_CreateParams *params, int32_t layerIdx,
  sTIDL_AlgLayer_t * algLayer, int32_t *paramMemTabOffset,
  int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
  void ** outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_detectOutAlloc
@brief      Function to Request Memory required by the SSD detcetion output layer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_detectOutAlloc(const TIDL_CreateParams *params,
  int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr);


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
@remarks    None
@return     IALG_EOK   - Successful
IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_detectOutInit(
  const TIDL_CreateParams *params, int32_t layerIdx,
  sTIDL_AlgLayer_t * algLayer, int32_t *paramMemTabOffset,
  int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
  void ** outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_roiPoolingAlloc
@brief      Function to Request Memory required by the ArgOp layer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_roiPoolingAlloc(const TIDL_CreateParams *params,
int32_t layerIdx, IALG_MemRec *memRec, sTIDL_LayerBuf_t *TIDLLayersBufPtr);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_roiPoolingInit
@brief      Function to Initialize the ArgOp layer specific params

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      algLayer     : Pointer to current layer algorithm specicfic params
@param      paramMemTabOffset : Current Offset in ALG_LAYERS_PARAMS_BUFF_MEMREC
@param      dataMemTabOffset : Current Offset in ALG_SCRATCH_DATA_BUFF_MEMREC
@param      memRec       : memory table populated by the Algorithm
@param      outPtr       : Pointer to Layer ouput Pointers list
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_roiPoolingInit(
const TIDL_CreateParams *params, int32_t layerIdx,
sTIDL_AlgLayer_t * algLayer, int32_t *paramMemTabOffset,
int32_t *dataMemTabOffset, const IALG_MemRec *memRec,
void ** outPtr, sTIDL_LayerBuf_t *TIDLLayersBufPtr);


int32_t TIDL_resolveLayerOutputShape(IVISION_InBufs *inBufs,
                                     IVISION_InBufs *outBufs,
                                     TIDL_Handle intAlgHandle,
                                     sTIDL_AlgLayer_t *algLayer,
                                     sTIDL_Layer_t *TIDLLayer);

int32_t TIDL_getLayerInPtrs(
  IVISION_InBufs      *inBufs,
  IVISION_InBufs      *outBufs,
  TIDL_Handle          intAlgHandle,
  sTIDL_AlgLayer_t     * algLayer,
  sTIDL_Layer_t        * TIDLLayer,
  void                 * inPtrs[],
  int32_t i,
  int32_t orgNumBatches[]);


int32_t TIDL_getLayerOutPtrs(
  IVISION_InBufs      *outBufs,
  TIDL_Handle          intAlgHandle,
  const sTIDL_AlgLayer_t     * algLayer,
  sTIDL_Layer_t        * TIDLLayer,
  void                 * outPtrs[],
  int32_t i
  );


int32_t TIDL_copyPersistentWeightsToIntMem(TIDL_Handle intAlgHandle);

int32_t TIDL_switchHandles(void* algHandle);
int32_t TIDL_getNumConsumersForDataId(const sTIDL_Network_t * pTIDLNetStructure, int32_t dataId);

/* Need to amend function once pad info becomes available as part of pad link */
void TIDL_getPadParams(WorkloadUnitExec_padParams *padParams,
                  sTIDL_Layer_t * tidlLayer,
                  int32_t flowCtrl,
                  const sWorkloadUnit_t *workloadUnit,
                  const sGCHelperHandle * gcHelperHandle);

/*sets the index of the multicore context link inside algLayer*/
void TIDL_setMultiCoreCtxtLinkIndex(TIDL_Handle algHandle, sTIDL_AlgLayer_t* algLayer, int32_t currAlgLayer);

void TIDL_forceTargetDeviceNonOTF(uint32_t *targetDevice, int32_t layerType, int32_t convType);

/* Get the flowType of the convolution layer */
int32_t TIDL_getDataFlowType(const sWorkloadUnit_t* workloadUnit,
                                  const sGCHelperHandle * gcHelperHandle);

int32_t TIDL_elementPromotionEnabledInAvgPool(const sTIDL_Network_t * net, sTIDL_Layer_t *tidlLayer);

#endif /* ITIDL_ARGOP_H*/
