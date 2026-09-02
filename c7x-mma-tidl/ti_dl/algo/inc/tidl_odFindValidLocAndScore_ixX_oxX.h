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
******************************************************************************
*  @file     tidl_odFindValidLocAndScore_ixX_ixX_oxX.h
*
*  @brief    Public header file for TIDL_odFindValidLocAndScore_ixX_ixX_oxX
*            function
*
*  @version  0.1 - Aug 2020 : Initial Version
*******************************************************************************
*/

#ifndef _TIDL_FINDVALIDLOACANDSCORE_IXX_IXX_OXX_H_
#define _TIDL_FINDVALIDLOACANDSCORE_IXX_IXX_OXX_H_

#include "tidl_generic_datatypes.h"

typedef struct
{
  int32_t reserved;
}TIDL_odFindValidLocAndScore_ixX_oxX_ExecInArgs;

typedef struct
{
  int32_t reserved;
}TIDL_odFindValidLocAndScore_ixX_oxX_ExecOutArgs;

#define TEMPLATE 1
/** @defgroup TIDL_odFindValidLocAndScore_ixX_ixX_oxX */
/** @ingroup  TIDL_odFindValidLocAndScore_ixX_ixX_oxX */
/* @{ */

/**
*  @enum       TIDL_ODFindValidLocAndScore_IXX_IXX_OXX_STATUS_NAME
*  @brief      Enumeration for different Error codes for odFindValidLocAndScore kernel
*
*/
typedef enum
{
  TIDL_ODFindValidLocAndScore_IXX_IXX_OXX_ERR_TBD  = TIDL_ERROR_START, /** Error case of TBD */
  /**@todo Add more appropriate error codes */
  TIDL_ODFindValidLocAndScore_IXX_IXX_OXX_ERR_MAX
} TIDL_ODFindValidLocAndScore_IXX_IXX_OXX_STATUS_NAME ;


/**<
*******************************************************************************
@struct TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs
@brief  This structure holds all the initialization parameter for CNN odFindValidLocAndScore
Kernel
@param funcType: Variant of the function refre @VXLIB_FUNCTION_STYLE
*******************************************************************************
*/
typedef struct
{
  int8_t funcStyle;
  int32_t Ni;             // Number of input channels to be processed
  int32_t kernelType;

  int32_t inputHeight;         // Specifies the input height
  int32_t numSplitsPerCh;       /* Specifies the num of Splits Per Channel */
  int32_t totalNumKernelCalls; // Specifies the total number of kernel calls.
  sTIDL_DetectOutputParams_t params;
  float32_tidl* priorData;
  sTIDL_ALgDetectOutputParams_t * algDetLyrParams;
} TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs ;



/**
*******************************************************************************
*  @func         TIDL_odFindValidLocAndScore_ixX_oxX_getHandleSize
*  @brief        This is a query function to kernel to get the size of internal
*                handle
*  @param [in]   pKerInitArgs  : Pointer to structure holding init parameters
*  @return       Size of the buffer in bytes
*  @remarks      Application is expected to allocate buffer of the requested
*                size and provide it during init and exec function calls
*******************************************************************************
*/
int32_t TIDL_odFindValidLocAndScore_ixX_oxX_getHandleSize(
  TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs *pKerInitArgs);


/**
*******************************************************************************
*  @func        TIDL_odFindValidLocAndScore_ixX_oxX_init
*  @brief       This function call is required to initialize the handle. In
*               this function most of the one time operation are performed
*               and results are stored in handle
*
*  @param [in]  handle       :  Active handle to the kernel
*  @param [in]  srcAddr      :  Pointer to structure containing dimensional
*                               information of source data
*  @param [in]  dstAddr      :  Pointer to structure containing dimensional
*                               information of dst
*  @param [in]  pKerInitArgs : Pointer to structure holding init parameters

*  @return      Status of success or Error with Error Codes
*               refer @TIDL_ODFindValidLocAndScore_IXX_IXX_OXX_STATUS_NAME
*
*  @remarks     Application is expected to do provide valid handle
*******************************************************************************
*/
int32_t TIDL_odFindValidLocAndScore_ixX_oxX_init(
  TIDL_KernelHandle handle,
  const TIDL_bufParams3D_t * srcAddr,
  const TIDL_bufParams3D_t * dstAddr,
  const TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs *pKerInitArgs);


/**
*******************************************************************************
*  @func        TIDL_odFindValidLocAndScore_ixX_oxX_exec
*  @brief       This function is the main compute function, and performs
*               the odFindValidLocAndScoreing operation for CNN. It is called multiple times
*
*  @details     The flow and the expectations of this function are as follows
*                -# @todo Add more details
*                -# TBD
*                -# TBD
*                -# TBD
*
*  @param [in]  handle      : Active handle to the kernel
*  @param [in]  src         : Pointer to buffer holding input data
*  @param [out] dst[]       : Pointer to buffer holding output feature map data
*  @param [in]  pKerInArgs  : Pointer to structure holding input Arguments
*  @param [out] pKerOutArgs : Pointer to structure holding output Arguments

*  @return      Status of success or Error with Error Codes
*               refer @TIDL_ODFindValidLocAndScore_IXX_IXX_OXX_STATUS_NAME
*
*  @par Assumptions:
*    - I/O buffer pointers are assumed to be not aliased.
*
*  @par Performance Considerations:
*    - For best performance, the following parameter settings are recommended:
*      - Set widths equal to strides
*      - Align all pointers to SIMD byte boundaries
*      - Set all stride values to a multiple of SIMD
*      - Set all width values to a multiple of SIMD
*      - @todo Modify and correct this list
*  @remarks     Application is expected to do call of checkParams function prior
*               to this function as it avoids check of paramaters for each
*               invocation for optimization
*******************************************************************************
*/

int32_t TIDL_odFindValidLocAndScore_ixX_oxX_exec(
  TIDL_KernelHandle handle,
  const void *src,
  void *dst,
  const void *pKerInArgs,
  void *pKerOutArgs);


/** @} */

#endif /* _TIDL_FINDVALIDLOACANDSCORE_IXX_IXX_OXX_H_ */

/* ======================================================================== */
/*  End of file:  TIDL_odFindValidLocAndScore_ixX_ixX_oxX.h                     */
/* ======================================================================== */
