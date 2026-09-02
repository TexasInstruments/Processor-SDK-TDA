/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter owns
 * or controls to make, have made, use, import, offer to sell and sell
 * ("Utilize") this software subject to the terms herein.  With respect to the
 * foregoing patent license, such license is granted solely to the extent that
 * any such patent is necessary to Utilize the software alone. The patent
 * license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI
 * Devices"). No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this
 * license (including the above copyright notice and the disclaimer and (if
 * applicable) source code license limitations below) in the documentation
 * and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted
 * provided that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this
 * software is permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI
 * Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the
 * software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution
 * of the source code are permitted provided that the following conditions are
 * met:
 *
 * *       any redistribution and use of the source code, including any
 * resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the
 * source code and any resulting derivative works, are licensed by TI for use
 * only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its
 * suppliers
 *
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __LDC_CONFIG_H__
#define __LDC_CONFIG_H__
/*platform-specific data types*/
typedef int32_t Int32;

/*user parameters*/
#define LDC_LIB_DATA_TYPE                                                                          \
    0 // 0: float, (1:double currently not supported since ldc_lensParameters
      // does only support float format)
#define LDC_LIB_U2D_LUT_TYPE 2         // 0: theta->rd/ru, 1: ru->rd/ru, 2: theta->rd
#define LDC_LIB_D2U_LUT_TYPE 0         // 0: rdSquared->ru/rd
#define LDC_LIB_INTERPOLATION_METHOD 2 // 0: 'previous', 1: 'nearest', 2:'linear'

#define LDC_MAX_NUM_CAMERAS (6)         // maximum number of cameras allowed
#define LDC_U2D_TABLE_MAX_LENGTH (1024) // maximum u2d table length allowed
#define LDC_D2U_TABLE_MAX_LENGTH (1024) // maximum d2d table length allowed

typedef struct
{
    Int32 ldcLUT_numCams;
    /**< Num of cameras */
    Int32 ldcLUT_distortionCenters[2 * LDC_MAX_NUM_CAMERAS];
    /**< Num of Lens Distortion centres */
    float ldcLUT_focalLength;
    /**< Lens focal length */

    Int32 U2D_type;
    /**< Lens Undistort to Distort type (must match macro
     * LDC_LIB_U2D_LUT_TYPE)*/
    Int32 ldcLUT_U2D_length;
    /**< Lens Undistort to Distort length */
    float ldcLUT_U2D_step;
    /**< Lens Undistort to Distort step */
    float ldcLUT_U2D_table[LDC_U2D_TABLE_MAX_LENGTH];
    /**< Lens Undistort to Distort table */

    Int32 D2U_type;
    /**< Lens Distort to Undistort type (must match macro
     * LDC_LIB_D2U_LUT_TYPE)*/
    Int32 ldcLUT_D2U_length;
    /**< Lens Distort to Undistort length */
    float ldcLUT_D2U_step;
    /**< Lens Distort to Undistort step */
    float ldcLUT_D2U_table[LDC_D2U_TABLE_MAX_LENGTH];
    /**< Lens Distort to Undistort table */
} ldc_lensParameters;

#endif // __LDC_CONFIG_H__
