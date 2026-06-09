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
 *  \file tidl_dataflow.h
 *
 *  \brief This file contains interface to use TIDL's DMA dataflow to pipeline
 *         optimized kernel processing with DMA.
 *
 *   Usage Scenario 1 :
 *   User wants to write a best optimized version where they have some processing
 *   task which is not required to be repeated with every time kernel is called
 *   with new input within a layer's processing. To allow user to have this one
 *   time kind of processing an initialization function option is provided.
 *   User can implement such processing and then provide the function pointer to
 *   TIDL_DataflowInit function as part of TIDL_DataflowInitParams->initFuncPtr.
 *   User is expected to adhere to the signature of initFuncPtr and the same can
 *   be found in TIDL_DataflowKernelInitFuncPtr definition
 *   When user provides initFuncPtr, TIDL will internally call this function to
 *   create multiple kernel handles based on which region of the tensor processing
 *   is happening ( e.g. whether its last block of processing where dimension of
 *   processing block can be different). The information of the dimension is provided
 *   by srcAddr and dstAddr arguments to the init function and user is expected to use
 *   these dimension for the implementation of this function.
 *   Typically things which are needed only once per tensor/frame can be computed inside
 *   this function and stored in the handle. This will avoid their computation during execute
 *   call. Eg. of such information can be SE/SA templates creation which are used during kernel
 *   implementation, or some one time buffer used by the kernel.
 *   This is the preferred usage as it will be optimal without user needing to handle
 *   various regions of processing.
 *
 *   Below code shows a sample usage of these API's for this scenario
 *
 *   TIDL_DataflowInitParams initParams;
 *   TIDL_CustomMaxPoolIxXOxXInitArgs kernelInitArgs;
 *
 *   initParams.dataFlowType     = TIDL_DataFlowTypeOneTensorInProcOneChannel;
 *   initParams.getHandleSize    = TIDL_customMaxPool_ixX_oxX_getHandleSize;
 *   initParams.initFuncPtr      = TIDL_customMaxPool_ixX_oxX_init;
 *   initParams.execFuncPtr      = TIDL_customMaxPool_ixX_oxX_exec;
 *   initParams.kernelInitArgs   = & kernelInitArgs;
 *
 *   status = TIDL_DataflowInit(tidlHandle,
 *                              &initParams);
 *
 *   status = TIDL_DataflowProcess(tidlHandle,
 *                                 inPtrs,
 *                                 outPtrs);
 *
 *
 *   Usage Scenario 2 :
 *   User want to quick start with a basic implementation without separating initialization
 *   and core repetitive processing. Then they can avoid providing initFuncPtr and the same
 *   can be set it to NULL.
 *   In this scenario user is expected to adhere to the signature of execFuncPtr and the same can
 *   be found in TIDL_DataflowKernelExecFuncPtr located in this file. Mainly srcAddr and dstAddr
 *   should be used for implemenation. These two parameters can be ignored ( they will be NULL)
 *   if initFuncPtr != NULL
 *
 *    srcAddr and dstAddr uses a structure TIDL_bufParams3D_t which can be found inside
 *    ti_dl\tidl_bufParams.h. It has following fields :
 *    data_type
 *    dim_x : The width of the buffer in X dimension in elements. This corresponds to width
 *            of the tensor which is going to processed in a given kernel call
 *    dim_y : The Height of the buffer in Y dimension in elements. This corresponds to height
 *            of the tensor which is going to processed in a given kernel call
 *    dim_z : The depth of the buffer in Z dimension in elements. This corresponds to number of
 *            channels which are going to processed in a given kernel call
 *    stride_y : Jump in bytes to move in dimesntion Y
 *    stride_z : Jump in bytes to move in dimesntion Z
 *
 *
 *    Note :
 *    - Currently this API only supports layer which takes single input and generates single output.
 *    - Upscale kind of layers which increases the output resolution have not been validated
 *    - Ratio of input and output dimension is expected to be an integer.
 *    - Current implementation does not handle optimization for large resolution
 *
 */


#ifndef TIDL_DATAFLOW_H_
#define TIDL_DATAFLOW_H_

#include "itidl_ti.h"


/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \anchor TIDL_data_type_e
 *  \name   TIDL Data Type
 *
 *  Group containing all data types
 *
 *  @{
 */
/* Standard fixed point data types */
#define TIDL_INT8       ((uint32_t) 0)
#define TIDL_INT16      ((uint32_t) 1)
#define TIDL_INT32      ((uint32_t) 2)
#define TIDL_INT64      ((uint32_t) 3)
#define TIDL_UINT8      ((uint32_t) 4)
#define TIDL_UINT16     ((uint32_t) 5)
#define TIDL_UINT24     ((uint32_t) 6)
#define TIDL_UINT32     ((uint32_t) 7)
#define TIDL_UINT64     ((uint32_t) 8)
/* Standard floating point data types */
#define TIDL_FLOAT16    ((uint32_t) 9)
#define TIDL_FLOAT32    ((uint32_t) 10)
#define TIDL_FLOAT64    ((uint32_t) 11)
#define TIDL_BFLOAT16    ((uint32_t) 12)
/* @} */



/**
 @struct  TIDL_DataflowInitParams
 @brief   A structure for a 3 dimensional buffer descriptor.
*/

typedef struct
{
  /** Data type of input @ref TIDL_data_type_e*/
  uint32_t data_type;
  /** Width of buffer in X dimension in elements. This corresponds to
  width of the tensor which is going to be processed in a given kernel call */
  uint32_t dim_x;
  /** Height of buffer in Y dimension in elements. This corresponds to
  height of the tensor which is going to be processed in a given kernel call */
  uint32_t dim_y;
  /** Stride in Y dimension in bytes. */
  int32_t  stride_y;
  /** Depth of patch in Z dimension in elements.This corresponds to
  number of channels which is going to be processed in a given kernel call */
  uint32_t dim_z;
  /** Stride in Z dimension in bytes. */
  int32_t  stride_z;
  /** Pointer to the array containing the tensor's tensor scales on a per channel basis, if quantization is per-tensor only one value is populated **/
  float32_tidl * tensorScaleList;
  /** Pointer to the array containing the tensor's zero points on a per channel basis, if quantization is per-tensor only one value is populated **/
  int32_t * tensorZeroPointList;
  /** Stride in X dimension in bytes. */
  int32_t  stride_x;
} TIDL_bufParams3D_t;

/**
 @struct  TIDL_bufParams6D_t
 @brief   A structure for a 6 dimensional buffer descriptor.
*/
typedef struct
{
  uint32_t data_type;
  uint32_t dim_0; /*This corresponds to width of the tensor which is going to be processed in a given kernel call*/
  int32_t  stride_0;

  uint32_t dim_1; /*This corresponds to height of the tensor which is going to be processed in a given kernel call*/
  int32_t  stride_1;

  uint32_t dim_2;  /*This corresponds to number of channels which is going to be processed in a given kernel call*/
  int32_t  stride_2; 

  uint32_t dim_3;  /*This corresponds to number of bacthes in the tensor*/
  int32_t  stride_3; 

  uint32_t dim_4;  
  int32_t  stride_4; 

  uint32_t dim_5;  
  int32_t  stride_5; 


} TIDL_bufParams6D_t;

/**
*******************************************************************************
*  \brief       This function call is required to know the memroy required by
*               kernel handle.
*
*  \param [in]  pKerInitArgs : Pointer to structure holding init parameters

*  \return      Size of the handle
*
*  \remarks
*******************************************************************************
*/
typedef int32_t (*TIDL_DataflowKernelGetHandleSize)(void * pKerInitArgs);

/**
*******************************************************************************
*  \brief       This function call is required to initialize the handle. In
*               this function most of the one time operation are performed
*               and results are stored in handle
*
*  \param [in]  kernelHandle :  Active handle to the kernel
*  \param [in]  srcAddr      :  Pointer to structure containing dimensional
*                               information of source data
*  \param [in]  dstAddr      :  Pointer to structure containing dimensional
*                               information of dst
*  \param [in]  pKerInitArgs : Pointer to structure holding init parameters

*  \return      Status of success or failure
*
*  \remarks
*******************************************************************************
*/
typedef int32_t (*TIDL_DataflowKernelInitFuncPtr)(void *kernelHandle,
                                                      const TIDL_bufParams3D_t *srcAddr,
                                                      const TIDL_bufParams3D_t *dstAddr,
                                                      void *pKerInitArgs);

/**
*******************************************************************************
*  @brief       This function is the main compute function, and performs
*               the argmaxing operation for CNN. It is called multiple times
*
*  @param [in]  kernelHandle: Active handle to the kernel
*  @param [in]  srcAddr    :  Pointer to structure containing dimensional
*                             information of source data.
*  @param [in]  dstAddr    :  Pointer to structure containing dimensional
*                               information of dst
*  @param [in]  srcPtr[]    : List of Pointers to buffer holding input data
*  @param [in]  dstPtr      : Pointer to buffer holding output feature map data

*  @return      Status of success or failure
*******************************************************************************
*/
typedef int32_t (*TIDL_DataflowKernelExecFuncPtr)(void *kernelHandle,
                                                      const TIDL_bufParams3D_t *srcAddr,
                                                      const TIDL_bufParams3D_t *dstAddr,
                                                      const void *srcPtr[],
                                                      void *dstPtr);


/**
 *  \anchor eTIDL_DataFlowType
 *  \name   TIDL various dataflow types supported
 *
 *  This group defines the different types of dataflow types that are supported
 *
 *  @{TIDL_DataFlowTypeMulInProcOneByOne
 */
/** \brief Kernel processing happens with one input tensor and one input channel
           is used to produce one output channel - example depth wise separable
           convolution, pooling etc. Note that processing can happen on more than
           one channel at time but for producing an output of a channel is not
           dependent on the other channel*/
#define TIDL_DataFlowTypeOneTensorInProcOneChannel       ((int32_t) 0)
/** \brief Kernel processing happens with one input tensor but all input channels
           are used to produce one output channel - example convolution, argmax*/
#define TIDL_DataFlowTypeOneTensorInProcMultiChannel     ((int32_t) 1)
/** \brief Number of input tensors are more than one but processing happens with
           one tensor at a time producing one output tensor, also dimension (C,H,W)
           of all tensors can be different - example Concat */
#define TIDL_DataFlowTypeMultiTensorInProcOneTensor     ((int32_t) 2)
/** \brief Number of input tensors are more than one and all tensors are used to
           produce output, also dimension (C,H,W) of all tensors should be same.
           example Eltwise */
#define TIDL_DataFlowTypeMultiTensorInProcMultiTensor   ((int32_t) 3)

/*NOTE : Current implementation only supports TIDL_DataFlowTypeOneTensorInProcOneChannel */

/* @} */


/**
 @struct  TIDL_DataflowInitParams
 @brief   This structure define the parameters needed during TIDL_DataflowInit
          function call
*/
typedef struct
{
  /** Number of input tensors for the processing @ref eTIDL_DataFlowType*/
  int32_t dataFlowType;
   /** Function pointer to get the kernel's handle size requirement.
   This can be given as Null if kernel doesn't need any handle */
  TIDL_DataflowKernelGetHandleSize getHandleSize;
  /** Function pointer for the kernel's Intilization call. User is
  expected to implement their kernel by adhering to the srcAddr and
  dstAddr dimensions as TIDL will internally call this function
  to handle changes in dimensions based on various regions of processing.
  This can be given as NULL if kernel doesn't need any handle. When this is NULL
  then user is expected to implement its kernel by handling all the
  dimensions as give srcAddr and dstAddr parameter in execute call.
  Given that execute call is called mutliple times, user can avoid
  computation in execute call by providing a valid initFuncPtr */
  TIDL_DataflowKernelInitFuncPtr initFuncPtr;
  /** Function pointer for the kernel's execute call. This cannot be
  given as NULL. If initFuncPtr is NULL then user is expected to adhere
  to the srcAddr and dstAddr arguments. If initFuncPtr is not NULL then
  user can ignore these two parameters during execute call*/
  TIDL_DataflowKernelExecFuncPtr execFuncPtr;
  /** Pointer to pass any arguments to the ther kernel.
  This pointer will be passed as it is to the initFuncPtr. If this
  information is required for the execute call then user is expected
  to store this in its internal handle. */
  void * kernelInitArgs;
 } TIDL_DataflowInitParams;


/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */



/**
 *  \brief This function initialized the dataflow handle.
 *
 *  \param  tidlHandle       [IN] Handle for the dataFlow ( this is created via TIDL_DataflowInit API)
 *  \param  initParams       [IN] Pointer to initParams
 *  \return Error Status
 */
int32_t TIDL_DataflowInit(void * tidlHandle,
                            TIDL_DataflowInitParams   * initParams);

/**
 *  \brief This function initialized the dataflow handle.
 *
 *  \param  tidlHandle      [IN] Handle for the dataFlow ( this is created via TIDL_DataflowInit API)
 *  \param  inPtrs[]         [IN] List of pointers input tensors
 *  \param  outPtr          [IN] Pointer to the output tensors
 *
 *  \return Error Status
 */
int32_t TIDL_DataflowProcess(void * tidlHandle,
                                    void *inPtrs[],
                                    void *outPtr[]);

#endif /* TIDL_DATAFLOW_H_*/

