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
  *  @file     TIDL_odFindValidLocAndScore_ixX_oxX_priv.h
  *
  *  @brief    File to hold private parameter of function TIDL_odFindValidLocAndScore
  *
  *  @version  0.1 - Aug 2020 : Initial Version
  *
 *******************************************************************************
*/

#ifndef _TIDL_ODFINDVALIDLOC_IXX_IXX_OXX_PRIV_H_
#define _TIDL_ODFINDVALIDLOC_IXX_IXX_OXX_PRIV_H_

#include "tidl_detectionOutput.h"
#include "tidl_odFindValidLocAndScore_ixX_oxX.h"
#include <math.h>
#include <stdlib.h>


/** @defgroup TIDL_odFindValidLocAndScore_ixX_ixX_oxX */
/** @ingroup  TIDL_odFindValidLocAndScore_ixX_ixX_oxX */
/* @{ */

/**< Function Pointer for execute */
// Feel free to modify this function - it should be lean function
// for example of keroutargs are not needed - don;t pass,
// also if handle is not needed don't pass,
typedef int32_t (*pFxnTIDL_odFindValidLocAndScore_ixX_oxX_exec)(
  TIDL_KernelHandle handle,
  const void *src,
  void *dst,
  const TIDL_odFindValidLocAndScore_ixX_oxX_ExecInArgs *pKerInArgs,
  TIDL_odFindValidLocAndScore_ixX_oxX_ExecOutArgs *pKerOutArgs);

#define TIDL_MAX_CONF_HEAD (16)

typedef struct
{
  uint32_t seTemplate[TIDL_MAX_CONF_HEAD][SE_PARAM_WORDS];
  uint32_t seTemplateBack[TIDL_MAX_CONF_HEAD][SE_PARAM_WORDS];
  uint32_t seTemplateMinMax[TIDL_MAX_CONF_HEAD][SE_PARAM_WORDS];
  int32_t numCh;
  int32_t elementsPerCh;
  int32_t elementsPerChLastBlock;
  int32_t srcStrideY;
  int32_t dstStrideY;
} TIDL_odFindValidLocAndScore_bufParamsC7x;

typedef struct
{
  TIDL_bufParams3D_t srcBuf3D;
  TIDL_bufParams3D_t dstBuf3D;
} TIDL_odFindValidLocAndScore_bufParamsNatC;

typedef union
{
  TIDL_odFindValidLocAndScore_bufParamsC7x    c7x;
  TIDL_odFindValidLocAndScore_bufParamsNatC   natc;
} TIDL_odFindValidLocAndScore_ixX_oxX_BufParams;


/**<
***************************************************************************
@struct TIDL_odFindValidLocAndScore_ixX_oxX_PrivArgs
@brief  This structure holds all the private arguments for CNN odFindValidLocAndScoreing
kernel
@param initArgs: Arguments passed during init
@param execute:  Function pointer for execution
@param state1  : Description of state1
@param state2  : Description of state2
***************************************************************************
*/

typedef struct
{
  TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs initArgs;
  pFxnTIDL_odFindValidLocAndScore_ixX_oxX_exec execute;
  //To add all private paramaters
  //SE/SA Config parameters
  TIDL_odFindValidLocAndScore_ixX_oxX_BufParams bufParams;

  int32_t state1;
  int32_t state2;
} TIDL_odFindValidLocAndScore_ixX_oxX_PrivArgs;

typedef int32_t (*pFxnTIDL_odFindValidLocAndScore_ixX_oxX_init)(
  TIDL_odFindValidLocAndScore_ixX_oxX_PrivArgs *pKerPrivArgs,
  const TIDL_bufParams3D_t * srcAddr,
  const TIDL_bufParams3D_t * dstAddr,
  const TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs *pKerInitArgs);

int32_t  TIDL_odFindValidLocAndScore_ixX_oxX_init_cn(
  TIDL_odFindValidLocAndScore_ixX_oxX_PrivArgs *pKerPrivArgs,
  const TIDL_bufParams3D_t * srcAddr,
  const TIDL_bufParams3D_t * dstAddr,
  const TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs *pKerInitArgs);

int32_t  TIDL_odFindValidLocAndScore_ixX_oxX_init_c7x(
  TIDL_odFindValidLocAndScore_ixX_oxX_PrivArgs *pKerPrivArgs,
  const TIDL_bufParams3D_t * srcAddr,
  const TIDL_bufParams3D_t * dstAddr,
  const TIDL_odFindValidLocAndScore_ixX_oxX_InitArgs *pKerInitArgs);

/**< Natural C Function for execute */
template <class Tconf>
int32_t TIDL_odFindValidLocAndScore_exec_cn(
  TIDL_KernelHandle handle,
  const void* src,
  void* dst,
  const TIDL_odFindValidLocAndScore_ixX_oxX_ExecInArgs *pKerInArgs,
  TIDL_odFindValidLocAndScore_ixX_oxX_ExecOutArgs *pKerOutArgs) ;

/**< explicit versions */
template <class Tconf, class TconfV>
int32_t TIDL_odFindValidLocAndScore_exec_c7x(
  TIDL_KernelHandle handle,
  const void *src,
  void *dst,
  const TIDL_odFindValidLocAndScore_ixX_oxX_ExecInArgs *pKerInArgs,
  TIDL_odFindValidLocAndScore_ixX_oxX_ExecOutArgs *pKerOutArgs);

/** @} */

#endif /* _TIDL_ODFINDVALIDLOC_IXX_IXX_OXX_PRIV_H_ */

/* ======================================================================== */
/*  End of file:  TIDL_odFindValidLocAndScore_ixX_ixX_oxX.h                              */
/* ======================================================================== */
