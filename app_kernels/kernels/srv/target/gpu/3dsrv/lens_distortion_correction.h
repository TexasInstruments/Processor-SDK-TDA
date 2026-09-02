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

#ifndef LENS_DISTORTION_CORRECTION_INCLUDED
#define LENS_DISTORTION_CORRECTION_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "ldc_config.h"

/*================================================
Data Structure
================================================== */

/*data type*/
#if LDC_LIB_DATA_TYPE == 2
    typedef Int32 dtype;
#elif LDC_LIB_DATA_TYPE == 1
typedef double dtype;
#elif LDC_LIB_DATA_TYPE == 0
typedef float dtype;
#endif

    /*status flag*/
    typedef enum
    {
        LDC_STATUS_OK,
        LDC_STATUS_FAIL
    } LDC_status;

    /*main struct*/
    typedef struct
    {
        dtype  distCenterX;
        dtype  distCenterY;
        dtype  distFocalLength;
        dtype  distFocalLengthInv;
        dtype* lut_d2u;
        Int32  lut_d2u_indMax;
        dtype  lut_d2u_step;
        dtype  lut_d2u_stepInv;
        dtype* lut_u2d;
        Int32  lut_u2d_indMax;
        dtype  lut_u2d_step;
        dtype  lut_u2d_stepInv;
    } LensDistortionCorrection;

    /*================================================
    Function API
    ================================================== */
    LDC_status LDC_Init_gpu(LensDistortionCorrection* ldc, dtype distCenterX, dtype distCenterY,
                            dtype distFocalLength, dtype* lut_d2u, Int32 lut_d2u_length,
                            dtype lut_d2u_step, dtype* lut_u2d, Int32 lut_u2d_length,
                            dtype lut_u2d_step);

    LDC_status LDC_DistToUndist_gpu(LensDistortionCorrection* ldc, dtype point_in[2],
                                    dtype point_out[2], dtype* lut_in_val);
    LDC_status LDC_UndistToDist_gpu(LensDistortionCorrection* ldc, dtype point_in[2],
                                    dtype point_out[2]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
