/*
*
* Copyright (c) {2015 - 2021} Texas Instruments Incorporated
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
 *  \file tidl_temp.h
 *
 *  \brief Temp header file for cleanup this will be removed eventually
 *
 *
 */


#ifndef TIDL_TEMP_H_
#define TIDL_TEMP_H_

#include "perfsim.h"
#include "itidl_ti.h"
#include "workload_unit_exec.h"



#define TIDL_LAYER_MEMORY_SCRATCH    (0) /*Scratch memory, hence can be re-used across layer/workload unit boundary*/
#define TIDL_LAYER_MEMORY_PERSISTENT (1) /*Persistent memory, cannot be re-used across different instances*/
#define TIDL_LAYER_MEMORY_OUTPUT     (2) /* Can be scratch but should be persistent when preemption is enabled */
#define TIDL_LAYER_MEMORY_MAX        (3)

#define TIDL_ALIGNMENT_SIZE (128)


/**
 @struct  sTIDL_LayerBuf_t
 @brief   This structure define the variables used in finding free
           output buffer in TIDL
 @param  outDataSize
          New outDataSize after finding free output buffer for current layer
 @param  newInDataId
          Address pointing to the new input buffer after finding
          free output buffer
 @param  newOutDataId
          Address pointing to the new output buffer after finding
          free output buffer
*/
typedef struct {
  int32_t outDataSize[TIDL_NUM_MAX_LAYERS];
#ifdef HOST_EMULATION
  int16_t newInDataId[TIDL_NUM_MAX_LAYERS][TIDL_NUM_IN_BUFS];
  int16_t newOutDataId[TIDL_NUM_MAX_LAYERS][TIDL_NUM_OUT_BUFS];
#endif
}sTIDL_LayerBuf_t;


//:TODO: Temporary location need to work on right location for this structure
typedef struct
{
  /* Pointer to the Graph compiler database */
  const sGCHelperHandle   *gcHelperHandle;
  /* Network pointer */
  sTIDL_Network_t       *net;
  /* Pointer to 1k zero vector stored in L2 */
  uint8_t * zeroVector1k;
  /*Pointer required for memory allocation for reference flow when NC is not used. */
  sTIDL_LayerBuf_t      *TIDLLayersBufPtr;
  /* Network level create time parameters */
  const TIDL_CreateParams *createParams;
  /* TIDL core params */
  WorkloadUnitExec_CommonParams *tidlCommonParams;
  /* Per-call cache layer control */
  int32_t cacheImport;
  /* Per-call cache layer control */
  int32_t cacheExport;
  /* Per-call cache layer control */
  int32_t cacheClear;
}TIDL_NetworkCommonParams;


typedef struct
{
  /* Pointer to a given workload unit to be executed */
  const sWorkloadUnit_t *workloadUnit;
  int32_t isMixedPrecEnabled;
}TIDL_LayerSpecificParams;

typedef struct
{
  const TIDL_CreateParams *createParams;
  int32_t                  layerIdx;
  const void               *workloadUnit;
  void                     *layerSpecificParams;
}TIDL_LayerInitParams;



//:TODO: Temporary location need to work on right location for this structure
typedef struct
{
  int32_t totalNumChannels;
  int32_t currChannelIdx;
}TIDL_dmaChannelAllocContext;


//:TODO: Find right location of this function
int32_t WorkloadUnitLayerSetup_setInitArgs(const sWorkloadUnit_t *workLoadUnit,
                                    const sGCHelperHandle   *gcDataBase,
                                    int32_t                layerIdx,
                                    TIDL_LayerInitParams        *layerInitParams,
                                    const WorkloadUnitExec_CommonParams *tidlCommonParams,
                                    WorkloadUnitExec_InitParams *initParams);


#endif

