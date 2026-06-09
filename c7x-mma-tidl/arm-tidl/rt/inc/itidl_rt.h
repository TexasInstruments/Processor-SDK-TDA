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
@file      itidl_rt.h
@brief     This file defines the public interface for TIDL Runtime API. \n
           This same API supports a client on a different CPU, out-of-process on the same CPU, or in-process. \n
           This interface is targeted for users developing inference application \n
           with imported TIDL models and users developing offload APIs for Open source \n
           runtime engines like TFlite (Delegate) and ONNX runtime (Execution provider) etc.

@version 0.1 June 2020 : Initial Code

*/

/** @ingroup    iTI_DL_RT */
/*@{*/
#ifndef ITIDL_RT_H_
#define ITIDL_RT_H_

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIDLRT_STRING_SIZE        ((int32_t) 512)
#define TIDLRT_PAD_DIM_MAX        ((uint32_t) 4)
#define TIDLRT_MAX_MEM_RECS       ((int32_t) 128)

/**
 *  \anchor eTIDLRT_ElementType
 *  \name   TIDL Element Type
 *
 *  This group defines the different types of element type used by TIDL library
 *
 *  @{
 */
#define TIDLRT_Uint8           ((int32_t) 0)
#define TIDLRT_Int8            ((int32_t) 1)
#define TIDLRT_Uint16          ((int32_t) 2)
#define TIDLRT_Int16           ((int32_t) 3)
#define TIDLRT_Uint32          ((int32_t) 4)
#define TIDLRT_Int32           ((int32_t) 5)
#define TIDLRT_Float32         ((int32_t) 6)
#define TIDLRT_Uint64          ((int32_t) 7)
#define TIDLRT_Int64           ((int32_t) 8)
#define TIDLRT_Bool            ((int32_t) 9)

/* @} */

/**
 *  \anchor eTIDLRT_DataDimIndex
 *  \name   TIDL Data Dimension Index
 *
 *  This group defines the indices of dimension array of layer data buffer in TIDL library
 *
 *  @{
 */
#define TIDLRT_DIM_BATCH             ((int32_t) 0)
#define TIDLRT_DIM_DIM1              ((int32_t) 1)
#define TIDLRT_DIM_DIM2              ((int32_t) 2)
#define TIDLRT_DIM_NUMCH             ((int32_t) 3)
#define TIDLRT_DIM_HEIGHT            ((int32_t) 4)
#define TIDLRT_DIM_WIDTH             ((int32_t) 5)
#define TIDLRT_DIM_MAX               (TIDLRT_DIM_WIDTH+1)
/* @} */

/**
 *  \anchor eTIDLRT_PitchDimIndex
 *  \name   TIDL Pitch Dimension Index
 *
 *  This group defines the indices of pitch array of layer data buffer in TIDL library
 *
 *  @{
 */
#define TIDLRT_ROI_PITCH             ((uint32_t) 0)
#define TIDLRT_DIM1_PITCH            ((uint32_t) 1)
#define TIDLRT_DIM2_PITCH            ((uint32_t) 2)
#define TIDLRT_CHANNEL_PITCH         ((uint32_t) 3)
#define TIDLRT_LINE_PITCH            ((uint32_t) 4)
#define TIDLRT_PITCH_MAX             ((uint32_t) (TIDLRT_DIM_MAX - 1))
/* @} */

/**
 *  \anchor eTIDLRT_MemType
 *  \name   Memory types in TIDL RT
 *
 *  Memory types definition used in the context of TIDL RT
 *
 *  @{
 */
#define TIDLRT_MEM_USER_SPACE        ((uint32_t) 0)
#define TIDLRT_MEM_SHARED            ((uint32_t) 1)
#define TIDLRT_MEM_NATIVE            ((uint32_t) 2)
#define TIDLRT_MEM_MAX             (TIDLRT_MEM_NATIVE+1)
/* @} */

/**
 *  \anchor eTIDLRT_TensorLayout
 *  \name   Tensor Layout Type
 *
 *  This group defines the indices Tensor Layout Type supported by TIDL RT library. \n
 *  TIDLRT_LT_NCHW --- Channels first, (Number_of_ROIs, Number_Of_Channels, Height , Width)- example (1,3,360,720 ) \n
 *  TIDLRT_LT_NHWC --- Channels last,  (Number_of_ROIs, Height , Width, Number_Of_Channels)- example (1,360,720,3 ).
 *
 *  @{
 */
#define TIDLRT_LT_NCHW             ((uint32_t) 0) 
#define TIDLRT_LT_NHWC             ((uint32_t) 1)
#define TIDLRT_LT_MAX               (TIDLRT_LT_NHWC+1)
/* @} */

typedef struct {
/** Unique name for the tensor holding feature vector ,
 * When we have multiple tensors as inputs, the tensors in the imported model and 
 * runtime may not be in same order. This name be part Model IO buffer descriptor 
 * file. This name would help in associating each tensor to model input */
int8_t name[TIDLRT_STRING_SIZE];
/** Element type of the feature vector as defined in \ref eTIDLRT_ElementType*/
int32_t elementType;
/** Number of dimensions supported by the tensor */
int32_t numDim;
/** Values of dimensions as defined by \ref eTIDLRT_DataDimIndex*/
int32_t dimValues[TIDLRT_DIM_MAX];
/** Pitch for each dimension as defined by \ref eTIDL_PitchDimIndex*/
int32_t pitch[TIDLRT_DIM_MAX-1];
/** Padding available in Spatial Dimension Left, right, top, Bot*/
int32_t padValues[TIDLRT_PAD_DIM_MAX];
/** Pointer to the buffer Base address*/
void * ptr;
/** Actual tensor data start offset in elements from the base address – Excluding the padding */
int32_t dataOffset;
/** Layout of the data NCHW or NHWC define in \ref eTIDLRT_TensorLayout*/
uint32_t layout;
/** Zero point for Asymmetric Fixed point Representation */
int32_t zeroPoint;
/** Scale used for float to fixed point conversion*/
float scale;
/** memory type – Linux user space memory or Shared memory define in \ref eTIDLRT_MemType*/
uint32_t memType;
/** Actual size of buffer in elements */
int32_t bufferSize;
}sTIDLRT_Tensor_t;

typedef struct {
    uint64_t cpIn_time_start;
    uint64_t cpIn_time_end;
    uint64_t proc_time_start;
    uint64_t proc_time_end;
    uint64_t cpOut_time_start;
    uint64_t cpOut_time_end;
} sTIDLRT_PerfStats_t;

typedef struct {
  /** Pointer to TIDL net structure */
  void*   netPtr;
  /** Pointer to the Input and Output tensors descriptor */
  void* ioBufDescPtr;
  /** Variable to store network size */
  int32_t net_capacity; 
  /** Variable to store io params size */
  int32_t io_capacity; 

  /*-------------------------------------------------------------------------*/
  /* Control Variables for DDR foot print and init time                      */
  /*-------------------------------------------------------------------------*/
  /** Enable dumping reordered params data computed during init time */
  int32_t dumpNetInitBackupData;
  /** 
   * Flag to indicate early release of input-output tensors during TIDLRT_create()
   * CAUTION! - If this flag is set then user MUST provide a buffer allocated
   * in shared memory pool which can be swapped with OpenVx buffers.
   * If the user sets this flag to 1 and provides a buffer allocated in a different
   * pool, the program will return with an error as it will not have a buffer in
   * shared memory to copy. By default this flag is set to 0 which indicates that
   * the OpenVx buffer will be released during TIDLRT_invoke() if the user
   * provided buffer meets the condition to be swapped. Otherwise it will copy the
   * contents of the user buffer to OpenVx memory buffer.
   * RECOMMENDATION - For optimal use of system it is advised to set this flag, 
   * it results in lesser DDR foot print
   */
  int32_t releaseIOTensorsAtCreate;

  /** Flag to indicate if config and network checksum should be calculated */
  int32_t computeChecksum;

  /** Flags to control the data flow for current execution, 
   * recommended to use default value for end user
   */
  uint32_t flowCtrl;

  /*-----------------------------------------------------------------------*/
  /* Control Variables for quantization                                    */
  /* Recommended to use default                                            */
  /*-----------------------------------------------------------------------*/
  /** Expansion factor used for range of activation while quantization */
  float quantRangeExpansionFactor;
  /** Update factor used for range of activation while quantization */
  float quantRangeUpdateFactor;

  /*-------------------------------------------------------------------------*/
  /* Control Variables to control multi DNN and preemption among them        */
  /* Recommended to use default, so read below as advanced user              */
  /* If the user wants multiple AI models in system with a capability of     */
  /* allowing different priority level for each, then should use this feature*/
  /* By using this feature, user can preempt an AI model-1 execution by      */
  /* scheduling another higher priority model-2. Priority can be set using   */
  /* argument targetPriority                                                 */
  /* One more argument maxPreEmptDelay is provided to user to control the    */
  /* urgency of scheduling a higher priority DNN, higher the value of this   */
  /* means that ongoing lower priority DNN can still continue to execute for */
  /* the stated time before giving importance to higher priority DNN         */
  /* Example :                                                               */
  /* DNN1 - higher priority (targetPriority = 0, maxPreEmptDelay = 30),      */
  /* DNN2 - lower priority  (targetPriority = 1, maxPreEmptDelay = 1),       */
  /* In above example DNN2 has a tolerance of 1 ms before getting preempted  */
  /* from the time when a request of scheduling DNN1 appeared                */
  /*-------------------------------------------------------------------------*/
  /** Int value of priority for selecting the target name
   * 0(highest prior)-7(lowest prior)
   */
  int targetPriority;
  /** Maximum Tolerated delay for TIDL pre-emption in millisecond */
  float maxPreEmptDelay;

  /*-----------------------------------------------------------------------*/
  /* Control Variables for multi core devices                              */
  /*-----------------------------------------------------------------------*/
  /**
   * Specify core number for execution - 1 for J721E, J721S2 and any one of
   * 1,2,3,4 for J784S4
   */
  uint32_t coreNum;
  /** Core idx to start processing from */
  int32_t coreStartIdx;

  /*-----------------------------------------------------------------------*/
  /* Control Variables for full code coverage test for safety applications */
  /*-----------------------------------------------------------------------*/
  /** Flag to force negative test cases for error injection */
  int32_t forceNegativeTest;
  /** Flag to enable code coverage in EVM, Default value is 0 
   *  and it is not recommended for user to change it
  */
  uint32_t enableCodeCoverage;

  /*-----------------------------------------------------------------------*/
  /* Control Variables for Debug and Trace                                 */
  /*-----------------------------------------------------------------------*/
  /** Flags to control debugTrace Level*/
  int32_t traceLogLevel;
  /**
   * Level for debug trace dumps of intermediate tensors and other data 
   * buffers into file
   */
  int32_t traceWriteLevel;
  /**  Trace files base Name */
  char traceBaseName[TIDLRT_STRING_SIZE];
  /**
   * Updated version of traceBaseName with an appended subgraph id to 
   * identify subgraphs for which corresponding traces are generated
   */
  char traceSubgraphName[TIDLRT_STRING_SIZE];
  /** Path to redirect temporary buffers */
  char tempBufferDir[TIDLRT_STRING_SIZE];
  
  /** Pointer to structure holding perf data */
  sTIDLRT_PerfStats_t *stats;

  /** Call back Function pointer to Write Log*/
  int32_t(*TIDLVprintf)(const char * format, va_list arg);
  /** Call back Function pointer to Write Binary data to a file*/
  int32_t(*TIDLWriteBinToFile)(const char * fileName, void * addr, 
    int32_t size, int32_t count);
  /** Call back Function pointer to read data from a binary file */
  int32_t(*TIDLReadBinFromFile)(const char * fileName, void * addr, 
    int32_t size, int32_t count);

}sTIDLRT_Params_t;

/**
* \brief      Function to create an instance of TIDL runtime
* \param      prms     : Pointer to user configuration as defined in \ref sTIDLRT_Params_t
* \param      handle   : Pointer to the Successfully created instance is returned with this
* \remarks    None
* \return     VX_SUCCESS  (0)    - Successful \n
              VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_create(sTIDLRT_Params_t *prms, void **handle);

/**
* @brief      Function to delete an instance of TIDL runtime
* @param      handle   : Pointer to the Successfully created TIDL RT instance
* @remarks    None
* @return     VX_SUCCESS  (0)    - Successful \n
              VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_delete(void *handle);

/**
* @brief      Function to run inference on input using a TIDL RT instance handle
* @param      handle   : Pointer to the Successfully created TIDL RT instance
* @param      in       : Pointer to list of input tensors as defined in \ref sTIDLRT_Tensor_t
* @param      out      : Pointer to list of output tensors as defined in \ref sTIDLRT_Tensor_t
* @remarks    None
* @return     VX_SUCCESS  (0)    - Successful \n
              VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_invoke(void *handle, sTIDLRT_Tensor_t *in[], sTIDLRT_Tensor_t *out[]);

/**
* @brief      Function to deactivate a instance of TIDL runtime. Activate will 
              be done by the invoke if the handle is not active.
* @param      handle   : Pointer to the Successfully created TIDL RT instance
* @remarks    None
* @return     VX_SUCCESS  (0)    - Successful \n
*             VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_deactivate(void *handle);

/**
 @brief      Function to set default values for parameters defined in \ref sTIDLRT_Params_t
 @param      prms   : Pointer to a instance of \ref sTIDLRT_Params_t
 @remarks    None
 @return     VX_SUCCESS  (0)    - Successful \n
             VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_setParamsDefault(sTIDLRT_Params_t *prms);

/**
* @brief      Function to set default values of parameters defined in \ref sTIDLRT_Tensor_t
* @param      tensor   : Pointer to a instance of \ref sTIDLRT_Tensor_t
* @remarks    None
* @return     VX_SUCCESS  (0)    - Successful \n
              VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_setTensorDefault(sTIDLRT_Tensor_t *tensor);

/**
* @brief      Function to get the number of bytes read and written during the last inference call \ref TIDLRT_invoke
* @param      read_bytes   : Number of bytes read from DDR memory
* @param      write_bytes  : Number of bytes written to DDR memory
* @remarks    None
* @return     VX_SUCCESS  (0)    - Successful \n
            VX_FAILURE  ( < 0) - Unspecified error
*/
int32_t TIDLRT_getDdrStats(uint64_t *read_bytes, uint64_t *write_bytes);

/**
* @brief      Function to allocate memory in the shared heap memory section between the Host core and DL accelerator.
              Allocating memory here would help in avoiding buffer copy between the cores
* @param      alignment   : Bases address alignment required in number of bytes
* @param      size        : Number of bytes to be allocated
* @remarks    None
* @return     Pinter to allocated memory   - Successful \n
              NULL                         - Error
*/
void   *TIDLRT_allocSharedMem(int32_t alignment, int32_t size);

/**
* @brief      Function to free the memory allocated using \ref  TIDLRT_allocSharedMem 
* @param      ptr        : Pointer to allocated memory
* @remarks    None
* @return     None
*/
void    TIDLRT_freeSharedMem(void *ptr);

/**
* \brief      Function to check whether  The memory pointed by input pointer
*             is part of shared heap section or NOT
* \param      ptr   : Pointer to the memory
* \remarks    None
* \return     1   - True \n 0   - False
*/
int32_t TIDLRT_isSharedMem(void *ptr);

int32_t TIDLRT_control(sTIDLRT_Params_t *prms, void *handle);
int32_t TIDLRT_controlCallTest(sTIDLRT_Params_t *prms, void *handle);

#ifdef __cplusplus
}
#endif

/*@}*/
/* iTI_DL_RT */

#endif /*ITIDL_RT_H_ */

