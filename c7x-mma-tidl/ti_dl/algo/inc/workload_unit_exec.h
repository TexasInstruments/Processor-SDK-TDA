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
 *  \file workload_unit_exec.h
 *
 *  \brief This file contains interface to setup and execute a workload unit
 *
 *
 */


#ifndef WORKLOAD_UNIT_EXEC_H_
#define WORKLOAD_UNIT_EXEC_H_

#include <dmautils.h>
#include "itidl_ti.h"
#include "gc.h"
#include "gc_helper.h"
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TIDL_MAX_NUM_CHANNELS_PER_LINK    (6)
#define TIDL_MAX_NUM_CHANNELS_PER_TRIGGER (5)
#define WORKLOADUNIT_MAX_TR_PER_CHANNEL  (3)

#define WORKLOADUNIT_EXEC_MAX_CONNECTED_JOINTS (MAX_SRC)

/**
 *  \anchor eTIDL_TransferLinkMode
 *  \name   TIDL various dataflow types supported
 *
 *  This group defines the different types of modes for transfer link
 *
 */
/** \brief Transfer link mode to indicate that transfer needs to be triggeredl*/
#define WorkloadUnitExec_TransferLinkModeTrigger       ((int32_t) 0)
/** \brief Transfer link mode to indicate that transfer needs to be sync/wait*/
#define WorkloadUnitExec_TransferLinkModeWait     ((int32_t) 1)

/**
 *  \anchor eTIDL_MixedPrecisionConfig
 *  \name   TIDL specific handling for input/output of link
 *
 *  This group defines the different configs possible for handling mixed precision
 *
 */
/** \brief Transfer link mode to indicate that transfer needs to be triggeredl*/
#define WorkloadUnitExec_MixedPrecInput       ((int32_t) 1)
/** \brief Transfer link mode to indicate that transfer needs to be sync/wait*/
#define WorkloadUnitExec_MixedPrecOutput      ((int32_t) 2)

/* @} */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/**
 * @enum WorkloadUnitExec_ExecuteState
 * @brief This enum contains the state of the link to be used by custom layers to
 *        configure custom transfer links
 */
typedef enum
{
  WORKLOADUNIT_EXECUTE_STATE_DMA_TRIGGER      = 1U << 0U,
  WORKLOADUNIT_EXECUTE_STATE_DMA_WAIT         = 1U << 1U,
  WORKLOADUNIT_EXECUTE_STATE_PROCESS          = 1U << 2U,
  WORKLOADUNIT_EXECUTE_STATE_DONOTHING        = 1U << 3U
}WorkloadUnitExec_ExecuteState;

/*To optimize things, it is always assumed that the first member is the layer
  context*/
typedef enum
{
  TIDL_XFRLINK_CONTEXT=0,
  TIDL_XFRLINK_TRMEM,
  TIDL_MAX_XFRLINK_HANDLE_MEMBERS
}eTIDL_XfrLinkHandleMembers_t;

/**
 @struct  WorkloadUnitExec_DmaChannelInfo
 @brief   This structure defines the per channel parameter
            for a dma configuration
*/
typedef struct
{
  int32_t  channelId;
  int32_t  trIdx[WORKLOADUNIT_MAX_TR_PER_CHANNEL];
  int32_t  numTrPerChannel;
}WorkloadUnitExec_DmaChannelInfo;

/**
 @struct  WorkloadUnitExec_DmaInfo
 @brief   This structure defines dma configuratoin for all
             the channel
*/
typedef struct
{
  int32_t   memOffsets[TIDL_MAX_XFRLINK_HANDLE_MEMBERS];
  int32_t   numChannelPerTrigger;
  /* For TIDL_DEVICE_L2_WRITETHROUGH = 0, If staging in OnChip and Output in DDR => L1DWB
     For TIDL_DEVICE_L2_WRITETHROUGH = 1, If staging in DDR and Output in DDR => L1DWB */
  int32_t   forceL1DCacheWb;
  // /* For TIDL_DEVICE_L2_WRITETHROUGH = 0, If staging in DDR and Output in DDR => L2DWB */
  // int32_t   forceL2DCacheWb;//Disabling now to keep the same flow as it was earlier
  int32_t   numFlowStages;
  int32_t   superOffset[2];
  uint8_t   srcElemSize;
  uint8_t   dstElemSize;
  WorkloadUnitExec_DmaChannelInfo dmaChannelInfo[MAX_FLOW_STAGES][TIDL_MAX_NUM_CHANNELS_PER_TRIGGER];
  CSL_UdmapTR * trMem; /*Eventually will be removed*/
}WorkloadUnitExec_DmaInfo;

/**
 @struct  WorkloadUnitExec_LinkFrameInitParams_t
 @brief   This structure defines additional params to be populated by workload during
          process call. Expected to be used only for specific process time config carried
          out before executing the link
 */
typedef struct
{
  /* Tile iteration */
  int32_t stIter;
  /* Flag to indicate whether the link has mixed precision */
  int32_t mixPrecConfig;
  /* Pointer to dmaUtilsContext object */
  void * dmaUtilsContext;
  /* Pointer to inputs */
  void *inPtr[MAX_SRC];
  /* Pointer to output */
  void *outPtr;
  /* Pointer to gcHelperHandle object */
  const sGCHelperHandle* gcHelperHandle;
  /* Pointer to user defined custom params */
  void *frameInitParams; 
}WorkloadUnitExec_LinkFrameInitParams_t;

/**
@struct  WorkloadUnitExec_ProcLinkExecArgs
@brief   This structure defines arguments which will be passed
           to the call back funciton WorkloadUnitExec_LinkExecFuncPtr during
           process link execution*/

typedef struct
{
  void *srcPtr[MAX_SRC+1U];
  void *sinkPtr;
  int32_t numSrc;
  int32_t wlIter;
}WorkloadUnitExec_ProcLinkExecArgs;

/**
 @struct  WorkloadUnitExec_XfrLinkExecArgs
 @brief   This structure defines arguments which will be passed
            to the call back funciton WorkloadUnitExec_LinkExecFuncPtr during
            trasnfer link execution*/
typedef struct
{
  void    *dmaUtilsContext;
  int32_t  nextState;
  int32_t  mode;
}WorkloadUnitExec_XfrLinkExecArgs;


/**
 @struct  WorkloadUnitExec_LinkExecArgs
 @brief   This structure defines arguments which will be passed
            to the call back funciton WorkloadUnitExec_LinkExecFuncPtr during
            any link execution*/
typedef union {
  WorkloadUnitExec_XfrLinkExecArgs  xfrLinkArgs;
  WorkloadUnitExec_ProcLinkExecArgs procLinkArgs;
}WorkloadUnitExec_LinkExecArgs;

/**
 @struct  WorkloadUnitExec_padParams
 @brief   This structure defines padding related arguments to be used by padding
          routine */
typedef struct
{
  /* Pointer to output buffer params */
  const sBufParams_t  *outBufParams;
  /* Specific tile type */
  uint32_t tileType;
  /* Valid pixel offset as decided by the workload */
  int32_t outOffset;
  /* Offset due to pad */
  int32_t outPadOffset;
  /* Offset per workload repeat */
  int32_t superOffset;
  /* Size of the elements in the buffer */
  int32_t elemSize;
  /* Type of flow (ref/target) */
  uint32_t flowCtrl;
  /* Height of output tile */
  int32_t outHeight;
  /* Number of channels in the output tile */
  int32_t outChannels;
  /* Does first tile exist or not */
  uint32_t isFirstTile;
  uint32_t isLastTile;
}WorkloadUnitExec_padParams;

//:TODO: Rename this and use it correctly
typedef struct
{
  uint8_t                 * l1BasePtr[TIDL_MAX_NUM_CORES];
  uint8_t                 * l2BasePtr[TIDL_MAX_NUM_CORES];
  uint8_t                 * l3ReservedMem[TIDL_MAX_NUM_CORES];
  uint8_t                 * l3BasePtr[TIDL_MAX_NUM_CORES];
  uint8_t                 * ddrBasePtr[TIDL_MAX_NUM_CORES];
  uint8_t                 * ddrPersistentBasePtr[TIDL_MAX_NUM_CORES];
}TIDL_sysScratchPtr;


/**
 @struct  WorkloadUnitExec_CommonParams
 @brief   This structure defines the parameters which are common
            across all the workload units
*/
typedef struct
{
  /* Pointer to system memory structue, note that during init this will be used only to alloacate
     memories*/
  sTIDL_sysMemHandle_t            *sysMems;
  /* Pointer to system scratch memory given to NC , this will be used to drive various IO pointers*/
  TIDL_sysScratchPtr              *sysScratchPtr;
  /* Pointer to 1k vector of zeros, typically used to DMA pad into a buffer */
  uint8_t                         *zeroVector1k;
  /* Pointer to dmautilsContext */
  void                            *dmaUtilsContext;
  /* Pointer to DMA channel allocator context*/
  void                            *dmaChannelAllocContext;
  /* Relative Core Id*/
  uint32_t                        coreId;
  /* Force a negative test case */
  int32_t                          forceNegativeTest;
}WorkloadUnitExec_CommonParams;

/**
 * 
*/
typedef struct
{
  /* Workload iteration count as generated by overall execution of wl super group*/
  int32_t                         wlIter;
  /* User defined handle for frame level params */
  void                            *frameInitParams;
  /* This structure defines padding related arguments to be used by padding routine*/
  WorkloadUnitExec_padParams      *padParams;
  /* Force a negative test case */
  int32_t                          forceNegativeTest;
}WorkloadUnitExec_InParams;


/**
 @struct  WorkloadUnitExec_OutParams
 @brief   This structure defines output parameters after workload execution
*/
typedef struct
{
  /* Pointer to the profile points */
  uint64_t *profilePoint;
}WorkloadUnitExec_OutParams;


/**
*******************************************************************************
*  \brief      This is a call back function to get the handle size required for a given link
*
*
*  \param [in]  initParams :  Pointer to structure containing parameters required for initialization
*                                         of a given link. This pointer will come from WorkloadUnitExec_LinkParams
*                                         which in turn is part of WorkloadUnitExec_InitParams
*
*  \param [in]  link      : Pointer to the link
*
*  \param [in]  gcHelperHandle      : Pointer to Graph Compiler Database
*
*  \return      Handle size for a given link
*
*  \remarks
*******************************************************************************
*/
typedef int32_t (*WorkloadUnitExec_LinkGetHandleSizeFuncPtr)(void *linkInitParams,
                                                                   const sLink_t              *link,
                                                                   const sGCHelperHandle * gcHelperHandle);


/**
*******************************************************************************
*  \brief      This is a call back function for initialization of a link. In
*               this function link initalization is done and most of the one time operation
*               which are required by  a link are performed and corresponding results are
*               stored in this handle. On top of this any information required
*               during runtime should also be kept inside the user defined processLinkHandle,
*               as this is the only pointer which will be available as part of exec call back function.
*               If the link type is a transfer link then user should return the outInfo with the information
*               about TR and channel for DMA.
*
*
*  \param [in]  linkHandle :  Handle ( pointer) to store private information needed by a given
*                                    link.  Memory for this will be allocated as part of WorkloadUnitExec_Init
*                                    function and corresponding size should be passed as part of
*                                    WorkloadUnitExec_ProcessLinkParams which in turn is part of WorkloadUnitExec_InitParams
*  \param [in]  linkInitParams :  Pointer to structure containing parameters required for initialization
*                                         of a given link. This pointer will come from WorkloadUnitExec_LinkParams
*                                         which in turn is part of WorkloadUnitExec_InitParams
*  \param [in]  commonParams : Pointer to the structure containing common parameters across all the links
*                                          This pointer will come from WorkloadUnitExec_LinkParams
*
*  \param [in]  link      : Pointer to the link
*
*  \param [in]  gcHelperHandle      : Pointer to Graph Compiler Database
*
*  \param [out]  infoArgs : This is any output information generated during initialization of link and needed
*                                  by workload execution function. Note that currently this is only needed for transfer
*                                  link
*
*
*  \return      Status of success or failure
*
*  \remarks
*******************************************************************************
*/
typedef int32_t (*WorkloadUnitExec_LinkInitFuncPtr)(void                           *linkHandle,
                                                       void                                *linkInitParams,
                                                       const WorkloadUnitExec_CommonParams *commonParams,
                                                       const sLink_t                       *link,
                                                       const sGCHelperHandle *gcHelperHandle);
/**
*******************************************************************************
*  \brief      This is a call back function for execution of a link.
*
*  \param [in]  linkHandle :  Handle ( pointer) to link handle created as part of
*                                               WorkloadUnitExec_InitFuncPtr callback function
*
*  \param [in]  linkExecArgs : Arguments expected to be used by this call back function during
*                                    exectution of the link
*
*  \param [in]  currFlowStage[] :  Current stage of execution (PIPEUP, PIPELINE, PIPEDOWN) for each
*                                            of the connected joints ( i.e. input, output, parameters)
*
*  \param [in]  currIterCount :  Current iteration number for  link
*
*  \return      Status of success or failure
*
*  \remarks
*******************************************************************************
*/
//:TODO: Define currFlowStage values
typedef int32_t (*WorkloadUnitExec_LinkExecFuncPtr)(void                            *linkHandle,
                                                       const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                                                       int32_t                          currFlowStage[],
                                                       int32_t                          currIterCount);

/**
*******************************************************************************
*  \brief      This is a call back function for frame level init of a link which happens once before processing
               the link.
*
*  \param [in]  linkHandle :  Handle ( pointer) to link handle created as part of
*                                               WorkloadUnitExec_InitFuncPtr callback function
*
*  \param [in]  frameInitParams : Arguments expected to be used by this call back function during
*                                    exectution of the link
*
*  \param [in]  link      : Pointer to the link
*
*  \return      Status of success or failure
*
*  \remarks
*******************************************************************************
*/
typedef int32_t (*WorkloadUnitExec_LinkFrameInitFuncPtr)(void *linkHandle,
                                      WorkloadUnitExec_LinkFrameInitParams_t *frameInitParams,
                                      const sLink_t *link);

/**
 @struct  WorkloadUnitExec_LinkParams
 @brief   This structure defines the parameters for link initialization
            will be used as part of WorkloadUnitExec_LinkInitFuncPtr call back function
*/
typedef struct
{
  /** init parameters for link. This will be passed as it is during WorkloadUnitExec_LinkInitFuncPtr
         call back function */
  void                      *initParams;
  /** Call back function to get handle size of a link.*/
  WorkloadUnitExec_LinkGetHandleSizeFuncPtr getHandleSizeFuncPtr;
  /** Call back function for link initialization. For transfer links this can be set as NULL if user wants to use existing
  dataflow */
  WorkloadUnitExec_LinkInitFuncPtr initFuncPtr;
  /** Call back function for the link execution. For transfer links this can be set as NULL if user wants to use existing
  dataflow */
  WorkloadUnitExec_LinkExecFuncPtr execFuncPtr;
  /** Call back function for frame level initialization of some */
  WorkloadUnitExec_LinkFrameInitFuncPtr frameInitFuncPtr;
}WorkloadUnitExec_LinkParams;


/**
 @struct  WorkloadUnitExec_InitParams
 @brief   This structure define the parameters needed during WorkloadUnitExec_Init
          function call
*/
typedef struct
{
  /* Flag to tell whether mixed precision is enabled for the workload */
  int32_t isMixPrecEnabled;
  /* pointer to structure containing work load unit information */
  const sWorkloadUnit_t               *workLoadUnit;
  /* pointer to structure containing the database for a given core*/
  const sGCHelperHandle *gcHelperHandle;
  /* pointer to any parameters which are required across all the links.
  These will be passed as it is to each link during corresponing init calls*/
  const WorkloadUnitExec_CommonParams *commonParams;
  /* Link init parameters */
  WorkloadUnitExec_LinkParams         linkInitParams[MAX_LINKS_PER_WL];
}WorkloadUnitExec_InitParams;




/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


/**
 *  \brief This function is to get the handle size required for workload unit execution. Work load unit
 *          creates a single handle for all the links so as to bring them to internal memory in one shot DMA
 *
 *  \param  processLinkHandleSize       [IN] Process link handle size
 *  \param  transferLinkHandleSize       [IN] Array for transferLink handle size
 *  \return Error Status
 */
int32_t WorkloadUnitExec_GetHandleSize(WorkloadUnitExec_InitParams *initParams);


/**
 *  \brief This function initialized the workload handle.
 *
 *  \param  workloadHandle       [IN] Handle for the workload unit. This will be allocated as part of
 *                                               WorkloadUnitExec_Init API itself
 *
 *  \param  initParams       [IN] Pointer to initParams for work load
 *  \return Error Status
 */
int32_t WorkloadUnitExec_Init(void *workloadHandle,
                              WorkloadUnitExec_InitParams   *initParams);

/**
 *  \brief This function initialized the dataflow handle.
 *
 *  \param  workloadHandle      [IN] Handle for the dataFlow ( this is created via WorkloadUnitExec_Init API)
 *  \param  inPtrs[]         [IN] List of pointers input tensors
 *  \param  outPtr          [IN] Pointer to the output tensor
 *  \param  wlIter          [IN] Iteration of workload (needed for st)
 *  \param  outParams   [Out] Performance profile points
 *
 *  \return Error Status
 */
int32_t WorkloadUnitExec_Process(void * workloadHandle,
                                void *inPtrs[],
                                void *outPtr,
                                sTIDL_Layer_t              *tidlLayer,
                                const WorkloadUnitExec_InParams  *inParams,
                                WorkloadUnitExec_OutParams *outParams);

/**
 *  \brief This function handles padding for the workload.
 *
 *  \param  outPtr          Pointer to the output buffer
 *  \param  wlIter          ST iteration
 *  \param  commonParams    Pointer to common params structure
 *  \param  padParams       Pointer to important pad info
 *  \param  workloadUnit    Pointer to workload
 *  \param  gcHelperHandle      Pointer to gcDataBase
 *
 *  \return Error Status
 */
int32_t WorkloadUnitExec_padBufferExec(void *outPtr,
                                int32_t wlIter,
                                sTIDL_Layer_t *TIDLLayer,
                                const WorkloadUnitExec_CommonParams *commonParams,
                                const WorkloadUnitExec_padParams  *padParams,
                                const sWorkloadUnit_t *workloadUnit,
                                const sGCHelperHandle   *gcDatabase,
                                int32_t chPitch,
                                int32_t roiPitch);
#endif /* WORKLOADUNIT_H_*/

