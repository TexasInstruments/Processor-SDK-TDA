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
  *  @file     tidl_types.h
  *
  *  @brief    File to hold common structure, enums, macros and functions for
  *            TIDL
  *
  *  @version  0.1 - Jan 2018 : Initial Version - wrapper on VXLIB [PKS]
  *
 *******************************************************************************
*/
#ifndef TIDL_TYPES_H_
#define TIDL_TYPES_H_ 1

/* This header is needed to be included in OpenCL programs which link
 * against TIDL, but OpenCL doesn't need the following headers */
#ifndef __OPENCL_VERSION__
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include "VXLIB_types.h"
#include "tidl_bufParams.h"
#endif

#ifdef CORE_DSP
#ifdef HOST_EMULATION
#include <assert.h>
#define restrict
#define _nassert assert
#else
#include "c7x.h"
#endif
#endif

#include "tidl_scalability_support.h"

typedef VXLIB_D64 TIDL_D64;  /*!< Double precision floating point */
typedef VXLIB_F32 TIDL_F32;  /*!< Single precision floating point */

/**
 *  \anchor TIDL_STATUS_NAME
 *  \name   TIDL Status Name
 *
 *  Different status codes used by TIDL
 *
 *  @{
 */
/** \brief No Error */
#define TIDL_SUCCESS                      ((int32_t) VXLIB_SUCCESS)
/** \brief Unspecified Error */
#define TIDL_ERR_FAILURE                  ((int32_t) VXLIB_ERR_FAILURE)
/** \brief Invalid parameter value */
#define TIDL_ERR_INVALID_VALUE            ((int32_t) VXLIB_ERR_INVALID_VALUE)
/** \brief Invalid parameter type (VXLIB_data_type_e data_type) */
#define TIDL_ERR_INVALID_TYPE             ((int32_t) VXLIB_ERR_INVALID_TYPE)
/** \brief Dimension parameter (width/height) is too big/small */
#define TIDL_ERR_INVALID_DIMENSION        ((int32_t) VXLIB_ERR_INVALID_DIMENSION)
/** \brief Unsupported null pointer condition */
#define TIDL_ERR_NULL_POINTER             ((int32_t) VXLIB_ERR_NULL_POINTER)
/** \brief Parameter configuration is not supported/implemented */
#define TIDL_ERR_NOT_IMPLEMENTED          ((int32_t) VXLIB_ERR_NOT_IMPLEMENTED)
/** \brief Stride should be equal to width * element size */
#define TIDL_ERR_NOT_EQUAL_WIDTH_STRIDE   ((int32_t) VXLIB_ERR_NOT_EQUAL_WIDTH_STRIDE)
/** \brief Pointers and stride values are not aligned to documented value */
#define TIDL_ERR_NOT_ALIGNED_PTRS_STRIDES ((int32_t) VXLIB_ERR_NOT_ALIGNED_PTRS_STRIDES)
/** \brief Width values are not aligned to documented value */
#define TIDL_ERR_NOT_ALIGNED_WIDTHS       ((int32_t) VXLIB_ERR_NOT_ALIGNED_WIDTHS)
/** \brief Buffer size not large enough */
#define TIDL_ERR_BUFFER_TOO_SMALL         ((int32_t) VXLIB_ERR_BUFFER_TOO_SMALL)
/** \brief Add custom errors after this */
#define TIDL_ERROR_START                  ((int32_t) 256)
/* @} */

typedef int32_t TIDL_STATUS;    /*!< return value for all VXLIB functions */


#define TIDL_min(x,y) VXLIB_min(x, y) /*!< A macro to return the minimum of 2 values. */
#define TIDL_max(x,y) VXLIB_max(x, y) /*!< A macro to return the maximum of 2 values. */

typedef void* TIDL_KernelHandle;

/**
 *  \anchor TIDL_FUNCTION_STYLE
 *  \name   TIDL Function Style
 *
 *  Function style in TIDL
 *
 *  @{
 */
/** \brief Natural C implementation of the function */
#define TIDL_FUNCTION_NATC            ((int32_t) 0)
/** \brief Optimized Function using MMA + C7x architecture */
#define TIDL_FUNCTION_OPTIMIZED_C7x   ((int32_t) 1)
/** \brief Max function style */
#define TIDL_FUNC_STYLE_MAX           ((int32_t) 2)
/** \brief Performance modelling of the function */
#define  TIDL_FUNCTION_PERFMODEL_FLAG ((int32_t) 0x40)
/** \brief Masking Bits of performance modelling flag. */
#define TIDL_FUNCTION_PERFMODEL_MASK  ((TIDL_FUNCTION_PERFMODEL_FLAG - 1))
/** \brief Performance modelling of the function */
#define TIDL_FUNCTION_PERFMODEL       ((int32_t) (TIDL_FUNCTION_PERFMODEL_FLAG | TIDL_FUNCTION_OPTIMIZED_C7x) )
/* @} */

#define TIDL_NUM_BLOCKS(width, stride) (((width)+(stride)-1U)/(stride))

#define TIDL_SIMD_WIDTH_SHIFT        (SIMD_SHIFT_FROM_WIDTH<VEC_NELEM(uchar)>::_value) /* 2^6 = 648b elements in a 512b vector */
#define TIDL_SIMD_WIDTH              ((uint32_t)(VEC_NELEM(uchar)))
#define TIDL_SIMD_WIDTH_RND          (TIDL_SIMD_WIDTH >> 1U)
#define TIDL_NUM_SIMD_BLOCKS(width)  (((width)+(TIDL_SIMD_WIDTH)-1)>>(TIDL_SIMD_WIDTH_SHIFT))

#define TIDL_16BIT_SIMD_WIDTH_SHIFT        (SIMD_SHIFT_FROM_WIDTH<VEC_NELEM(ushort)>::_value) /* 2^6 = 648b elements in a 512b vector */
#define TIDL_16BIT_SIMD_WIDTH              ((uint32_t)(VEC_NELEM(ushort)))
#define TIDL_16BIT_SIMD_WIDTH_RND          (TIDL_16BIT_SIMD_WIDTH >> 1U)
#define TIDL_16BIT_NUM_SIMD_BLOCKS(width)  (((width)+(TIDL_16BIT_SIMD_WIDTH)-1)>>(TIDL_16BIT_SIMD_WIDTH_SHIFT))

#define TIDL_32BIT_SIMD_WIDTH_SHIFT        (SIMD_SHIFT_FROM_WIDTH<VEC_NELEM(uint)>::_value) /* 2^6 = 648b elements in a 512b vector */
#define TIDL_32BIT_SIMD_WIDTH              ((uint32_t)(VEC_NELEM(uint)))
#define TIDL_32BIT_SIMD_WIDTH_RND          (TIDL_32BIT_SIMD_WIDTH >> 1U)
#define TIDL_32BIT_NUM_SIMD_BLOCKS(width)  (((width)+(TIDL_32BIT_SIMD_WIDTH)-1)>>(TIDL_32BIT_SIMD_WIDTH_SHIFT))

#if __C7X_VEC_SIZE_BITS__ == 1024
#elif __C7X_VEC_SIZE_BITS__ == 512
#define VPRED_ZERO (0x0ULL)
#define REINTERPRET_VPRED(x) __as_uchar8((x))
typedef unsigned long long vpred_ull;
#elif __C7X_VEC_SIZE_BITS__ == 256
#define VPRED_ZERO (0x0U)
#define REINTERPRET_VPRED(x) __as_uchar4((x))
typedef unsigned int vpred_ull;
#else
#error "unsupported machine SIMD width"
#endif

#endif /* TIDL_TYPES_H_ */

/* ======================================================================== */
/*  End of file:  TIDL_types.h                                             */
/* ======================================================================== */
