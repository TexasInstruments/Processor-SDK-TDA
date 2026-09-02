/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#ifndef TIVX_SAMPLE_KERNELS_H_
#define TIVX_SAMPLE_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_vision_apps_kernels_sample TIVX Example Kernels 
 *
 * \brief This section documents the kernels which are targted to be examples of writing different kernels
 * 
 * \ingroup group_vision_apps_kernels
 * 
 * @{
 */

/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief Name for OpenVX Extension kernel module: sample
 */
#define TIVX_MODULE_NAME_SAMPLE    "sample"

/*! \brief The list of kernels supported in sample module
 *
 * Each kernel listed here can be used with the <tt> vxGetKernelByName</tt> call.
 * When programming the parameters, use
 * \arg <tt> VX_INPUT</tt> for [in]
 * \arg <tt> VX_OUTPUT</tt> for [out]
 * \arg <tt> VX_BIDIRECTIONAL</tt> for [in,out]
 *
 * When programming the parameters, use
 * \arg <tt> VX_TYPE_IMAGE</tt> for a <tt> vx_image</tt> in the size field of <tt> vxGetParameterByIndex</tt> or <tt> vxSetParameterByIndex</tt>
 * \arg <tt> VX_TYPE_ARRAY</tt> for a <tt> vx_array</tt> in the size field of <tt> vxGetParameterByIndex</tt> or <tt> vxSetParameterByIndex</tt>
 * \arg or other appropriate types in  vx_type_e.
 */

/*! \brief opengl_mosaic kernel name
 */
#define TIVX_KERNEL_OPENGL_MOSAIC_NAME      "com.ti.sample.opengl_mosaic"

/*********************************
 * OpenGL Mosaic STRUCTURES
 *********************************/
/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_OPENGL_MOSAIC kernel.
 *
 */

/**< Mosaic/Render Type - 1x1 (single window) mosaic */
#define  TIVX_KERNEL_OPENGL_MOSAIC_TYPE_1x1  (0)

/**< Mosaic/Render Type - 2x2 (4 window) mosaic */
#define  TIVX_KERNEL_OPENGL_MOSAIC_TYPE_2x2  (1)

/**
 * \brief OpenGL Mosaic Node parameters
 */ 
typedef struct
{
    uint32_t renderType;
    /**< Mosaic/Render Type - supported options: 1x1 & 2x2 */

} tivx_opengl_mosaic_params_t;


/*********************************
 * Function Prototypes
 *********************************/

/*!
 * \brief Used for the Application to load the sample kernels into the context.
 */
void tivxSampleLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the sample kernels from the context.
 */
void tivxSampleUnLoadKernels(vx_context context);

/*!
 * \brief Used to print the performance of the kernels.
 */
void tivxSamplePrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* TIVX_SAMPLE_KERNELS_H_ */


