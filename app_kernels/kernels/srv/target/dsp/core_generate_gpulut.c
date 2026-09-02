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

#include "TI/tivx.h"
#include "TI/tivx_srv_kernels.h"
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_generate_gpulut.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "core_generate_gpulut.h"
#ifdef HOST_EMULATION
#include "C6xSimulator.h"
#else
#if defined (__C7100__) || defined (__C7120__) || defined (__C7524__) || defined (__C7604__)
    #include <c7x.h>
    #if defined (C6X_MIGRATION)
        #include <c6x_migration.h>
    #endif
    #define RESTRICT restrict
#else
    #include <c6x.h>
#endif
#endif

//#define DEBUG_1
#define NUM_ELEMENTS (7)
#ifdef DEBUG_1
#include <stdio.h>
#endif

/* Pull in inline for divsp */
static inline float divspMod_atanspi(float a, float b) {
    cmn_DIVSP(b, a);
}

static inline float atan2f_sr1i_atanspi(float g1, int s, int an)
{
    float coef;
    float       g2;
    float       g4;
    float       g6;
    float       g8;
    float       g10;
    float       g12;
    float       pol;
    float       tmp1;
    float       tmp2;
    float pih = 1.57079632679f;
    float       c1 = 0.00230158202f;
    float       c2 = -0.01394551000f;
    float       c3 = 0.03937087815f;
    float       c4 = -0.07235669163f;
    float       c5 = 0.10521499322f;
    float       c6 = -0.14175076797f;
    float       c7 = 0.19989300877f;
    float       c8 = -0.33332930041f;

    /* get coef based on the flags */
    coef = pih;
    if (!s) {
        coef = 0;
    }

    if (an) {
        coef = -coef;
    }

    /* calculate polynomial */
    g2 = g1*g1;
    g4 = g2*g2;
    g6 = g2*g4;
    g8 = g4*g4;
    g10 = g6*g4;
    g12 = g8*g4;

    tmp1 = ((c5 * g8) + (c6 * g6)) + ((c7 * g4) + (c8 * g2));
    tmp2 = (((c1 * g4 + c2 * g2) + c3) * g12) + (c4 * g10);

    pol = tmp1 + tmp2;
    pol = pol*g1 + g1;

    return (s ? (coef - pol) : (coef + pol));
}

static inline float atansp_i(float a)
{
    float       g;
    float       res;
    float       temp = 1.0f;
    int         an;
    int         s = 0;

    an = (a < 0) ? 1 : 0;   /* flag for a negative */

    /* swap a and b before calling division sub routine if a > b */
    if (_fabsf(a) > 1.0f) {
        temp = a;
        a = 1.0f;
        s = 1;             /* swap flag */
    }

    g = divspMod_atanspi(temp, a);

    /* do polynomial estimation */
    res = atan2f_sr1i_atanspi(g, s, an);

    if (a == 0.0f) {
        res = 0.0f;
    }

    return (res);
}

static inline float recipsp (float a)
{
    float    two  =  2.0f;
    //float    max  = 1.7976931348623157E+308;
    float    x;
    int32_t     i;

    x = _rcpsp(a);

    for (i = 0; i < 2; i++) {
        x = x * (two - (a * x));
    }

    //if( a > max ) { // might be zero but not max
    //    x = 0.0;
    //}

    return (x);
}

#define OPTIMIZE 1

static inline float sqrtsp (float a);

static inline float sqrtsp (float a)
{
    float    half  =  0.5f;
    float    OneP5 =  1.5f;
    float    x, y;
    int32_t     i;

    x = _rsqrsp(a);

    for( i = 0; i < 2; i++ ) {
        x = x * (OneP5 - (a * x * x * half));
    }

    y = a * x; /* TODO: we may need to check if a is zero ... removing for now
                  to achieve performance */

    /* a > DBL_MAX checks unnecesary since they won't happen
     * in this situation */

    return (y);
}


/*===============================================================================
*
* Name:        svProjectiveTransform()
*
* Description: Transform a given world point according to camera co-ordinates
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
* Effects:
*
===============================================================================*/
static inline void svProjectiveTransform(svGpuLutGen_t *sv, svLdcLut_t  *ldclut, tivxGenerateGpulutParams *prms, float calmat_scaled[restrict], int32_t regmark, float X, float Y, float Z, float point_u[restrict])
{
    float x, y, z;
    float x_norm, y_norm;
    float z_recip;

    // Multiplying with x'=[R|t]X to go to camera co-ordinate frame (typecast to floats)
    // CALMAT_R_SHIFT and CALMAT_T_SHIFT are varied based on the precision needs of Rotation and Translation matrices
    x = calmat_scaled[regmark * 12 + 0]*X + calmat_scaled[regmark * 12 + 3]*Y \
        + calmat_scaled[regmark * 12 + 6]*Z + calmat_scaled[regmark * 12 + 9];

    y = calmat_scaled[regmark * 12 + 1]*X + calmat_scaled[regmark * 12 + 4]*Y \
        + calmat_scaled[regmark * 12 + 7]*Z + calmat_scaled[regmark * 12 + 10];

    z = calmat_scaled[regmark * 12 + 2]*X + calmat_scaled[regmark * 12 + 5]*Y \
        + calmat_scaled[regmark * 12 + 8]*Z + calmat_scaled[regmark * 12 + 11];


    // Reflect regions from behind camera to regions in front of camera (Suitable for LDC processing)
    if (z < 0.0f)
        z *= (-1.0f);

    z_recip = recipsp(z);

    // Normalize co-ordinates
    x_norm = (x) * z_recip;
    y_norm = (y) * z_recip;

    // Multiply with intrisic matrix K : x = K*(R[I|t])*X where x = K*x'
    //Shashank: Be careful here, regmark probaby refers to camera id; print these parameters to make sure
#if 0    
    point_u[0] = ((float)sv->focalLength[regmark] * x_norm) + (float)sv->InCamFrmDistortionCen[regmark * 2 + 1];
    point_u[1] = ((float)sv->focalLength[regmark] * y_norm) + (float)sv->InCamFrmDistortionCen[regmark * 2];
#endif
    // SD TODO; Reconfirm
    point_u[0] = ((float)ldclut->ldc[regmark].distFocalLength * x_norm) + (float)ldclut->ldc[regmark].distCenterX;
    point_u[1] = ((float)ldclut->ldc[regmark].distFocalLength * y_norm) + (float)ldclut->ldc[regmark].distCenterY;
}

static inline void svFisheyeTransformUndistToDist(__float2_t point_u, int32_t *restrict loc, __float2_t sixteenf2, uint32_t maxCmp, const float lut[restrict], __float2_t distCenter, dtype distFocalLengthInv, dtype lut_u2d_stepInv)
{
    dtype point_d[2];
    dtype lut_in_val;
    dtype lut_out_val;
    dtype ru;
    dtype ru_recip;

    __float2_t diff = _dsubsp(point_u, distCenter);
    __float2_t diffsq = _dmpysp(diff, diff);
    ru = sqrtsp(_hif2(diffsq) + _lof2(diffsq));
    lut_in_val = atansp_i(ru*distFocalLengthInv);

    dtype ind = lut_in_val * lut_u2d_stepInv;
    int32_t N = (int32_t)ind;
    dtype indMinN = ind - (dtype)N;
    lut_out_val = (1.0f - indMinN)*lut[N] + indMinN * lut[N + 1];

    ru_recip = recipsp(ru);
    if (ru == 0.0f) {
        ru_recip = 0.0f;
    }
    point_d[0] = lut_out_val * _lof2(diff) * ru_recip + _lof2(distCenter);
    point_d[1] = lut_out_val * _hif2(diff) * ru_recip + _hif2(distCenter);

    __float2_t locf2 = _ftof2(point_d[0], point_d[1]);

    /* Multply coordinates by 16 before converting to fixed point */
    locf2 = _dmpysp(locf2, sixteenf2);

    /* Convert coordinates from float to s16 integer with round S15.4 */
    uint32_t locYX = _dspinth(locf2);

    /* Repeat pixels if we try to access a pixel outside the camera input image */
    locYX = _max2(0, locYX);
    locYX = _min2(locYX, maxCmp);
 
    *loc = locYX;
}



void svGenerate_3D_GPULUT(svGpuLutGen_t *sv, svLdcLut_t  *ldclut, tivxGenerateGpulutParams *prms, svACCalmatStruct_t* calmat_scaled, float *lut3dxyz, uint16_t *out_gpulut, int32_t *lut_count)
{


    float X,Y,Z;
    int32_t count=0;
    int32_t viewId1[4];
    int32_t viewId2[4];
    __float2_t distCenter[4];
    uint32_t shift=  31U - _lmbd(1U, sv->subsampleratio);
    int32_t no_of_elems = NUM_ELEMENTS;       /* Originally 9 but will be 7 eventually */
    count = 0;
    int undist_cnt = 0;

    int32_t quad,i,j;
    int32_t rowStart[4];
    int32_t colStart[4];
    int32_t rowEnd[4];
    int32_t colEnd[4];
//    float   calmat_scaled[12*4];

    viewId1[0] = 0;
    viewId2[0] = 3;
    viewId1[1] = 1;
    viewId2[1] = 0;
    viewId1[2] = 2;
    viewId2[2] = 1;
    viewId1[3] = 3;
    viewId2[3] = 2;

    LensDistortionCorrection *ldcMod = &ldclut->ldc[0];
    dtype distCenterX = ldcMod->distCenterX;
    dtype distCenterY = ldcMod->distCenterY;
    distCenter[0] = _ftof2(distCenterY, distCenterX);
    dtype *lut = ldcMod->lut_u2d;
    dtype distFocalLengthInv = ldcMod->distFocalLengthInv;
    dtype lut_u2d_stepInv = ldcMod->lut_u2d_stepInv;

    LensDistortionCorrection *ldcMod1 = &ldclut->ldc[1];
    distCenterX = ldcMod1->distCenterX;
    distCenterY = ldcMod1->distCenterY;
    distCenter[1] = _ftof2(distCenterY, distCenterX);

    LensDistortionCorrection *ldcMod2 = &ldclut->ldc[2];
    distCenterX = ldcMod2->distCenterX;
    distCenterY = ldcMod2->distCenterY;
    distCenter[2] = _ftof2(distCenterY, distCenterX);

    LensDistortionCorrection *ldcMod3 = &ldclut->ldc[3];
    distCenterX = ldcMod3->distCenterX;
    distCenterY = ldcMod3->distCenterY;
    distCenter[3] = _ftof2(distCenterY, distCenterX);

    /* Quadrant 1 */
    rowStart[0] = 0;
    rowEnd[0]   = (sv->SVOutDisplayHeight / 2)>>shift;
    colStart[0] = 0;
    colEnd[0]   = (sv->SVOutDisplayWidth / 2)>>shift;

    /* Quadrant 2 */
    /* if(j>=(sv->SVOutDisplayWidth/2-sv->subsampleratio) && i<=sv->SVOutDisplayHeight/2) */
    rowStart[1] = 0;
    rowEnd[1] = (sv->SVOutDisplayHeight / 2) >> shift;
    colStart[1] = (sv->SVOutDisplayWidth / 2 - sv->subsampleratio) >> shift;
    colEnd[1] = (sv->SVOutDisplayWidth - 1) >> shift;

    /* Quadrant 3 */
    /* if(j>=(sv->SVOutDisplayWidth/2-sv->subsampleratio) && i>=(sv->SVOutDisplayHeight/2-sv->subsampleratio)) */
    rowStart[2] = (sv->SVOutDisplayHeight / 2 - sv->subsampleratio) >> shift;
    rowEnd[2] = (sv->SVOutDisplayHeight - 1) >> shift;
    colStart[2] = (sv->SVOutDisplayWidth / 2 - sv->subsampleratio) >> shift;
    colEnd[2] = (sv->SVOutDisplayWidth - 1) >> shift;

    /* Quadrant 4 */
    /* if(j<=sv->SVOutDisplayWidth/2 && i>=(sv->SVOutDisplayHeight/2-sv->subsampleratio)) */
    rowStart[3] = (sv->SVOutDisplayHeight / 2 - sv->subsampleratio) >> shift;
    rowEnd[3] = (sv->SVOutDisplayHeight - 1) >> shift;
    colStart[3] = 0;
    colEnd[3] = (sv->SVOutDisplayWidth / 2) >> shift;


    __float2_t sixteenf2 = _ftof2(16.0f, 16.0f);

    uint32_t maxCmp = _pack2((sv->SVInCamFrmWidth - 2) << 4, (sv->SVInCamFrmHeight - 2) << 4);

    /* 3 Buffers: */

    /* Input buffer storing XYZ coordinates of bowl */
    float *restrict pLutXYZ = lut3dxyz;

    /* Intermediate buffer storing undistorted 2D coordinates for both views */
    float *restrict pUndist = prms->buf_GLUT3d_undist_ptr;

    /* Output buffer storing GPU LUT */
    uint16_t *restrict pLut   = out_gpulut;

    int32_t index0 = 0;
    int32_t index1 = 2;
    int32_t undist_index = 3;
    int32_t undist_index2 = 3;

    /* For each quadrant */
    for ( quad = 0; quad < 4; quad++ ) {
        for (i = rowStart[quad]; i <= rowEnd[quad]; i++) {
            count = ((i)*(sv->SVOutDisplayWidth>>shift)+colStart[quad])*3;
            undist_cnt = 0;

            /* Add new loop w/ pLut = i, j */
            for (j = colStart[quad]; j <= colEnd[quad]; j++){
#ifdef __OPTIMIZED__
                /* The below was optimized for a size of 1080x1080   */
                /* The size was hardcoded, however for generic perf  */
                /* we use this as X/2, Y/2                           */
                /* For optimized perfromance set the number below to */
                /* the correct value prior to enabling this branch   */
                pLut[index0+0] = (j << shift) - 540; //X/step_size; // X co-ordinate
                pLut[index0+1] = 540 - (i << shift); //-Y/step_size; // Y co-ordinate
#else
                pLut[index0+0] = (j << shift) - sv->SVOutDisplayWidth/2;  //  X co-ordinate
                pLut[index0+1] = sv->SVOutDisplayHeight/2 - (i << shift); //  Y co-ordinate
#endif

                index0 += no_of_elems;
            }

            /* Stage 1: for each XYZ coordinate, perform perspective transform
             *          from 3D to 2D coordinates
             */
            for (j = colStart[quad]; j <= colEnd[quad]; j++){

                float point_u1[2], point_u2[2];

                X = pLutXYZ[count++];
                Y = pLutXYZ[count++];
                Z = pLutXYZ[count++];

                pLut[index1] = (int16_t)Z;
                svProjectiveTransform(sv,ldclut, prms, calmat_scaled->scaled_outcalmat, viewId1[quad], X, Y, Z, point_u1);
                svProjectiveTransform(sv,ldclut, prms, calmat_scaled->scaled_outcalmat, viewId2[quad], X, Y, Z, point_u2);

                pUndist[undist_cnt++] = point_u1[0];
                pUndist[undist_cnt++] = point_u1[1];
                pUndist[undist_cnt++] = point_u2[0];
                pUndist[undist_cnt++] = point_u2[1];

                index1 += no_of_elems;
            }

            undist_cnt = 0;

            /* Stage 2: for each 2D coordinate, perform fisheye transformation from undistorted to
             *          distorted.
             *          Combined as one loop has ii = 97
             *          Separated into 2 loops has ii=36+35 = 71
             */

            #pragma MUST_ITERATE(20, ,)
            for (j = colStart[quad]; j <= colEnd[quad]; j ++){

                __float2_t point_u1;
                int32_t loc;

                point_u1 = _amem8_f2_const(&pUndist[undist_cnt + 0]);

                undist_cnt += 4;

                // Dominant view
                #ifdef DEBUG_1
                printf("Prior to 1st svFishEyeTransform \n");
                float point_t1,point_t2;
                point_t1 = pUndist[undist_cnt];
                point_t2 = pUndist[undist_cnt+1];
                printf ("points are %f %f \n", point_t1,point_t2);
                #endif
                svFisheyeTransformUndistToDist(point_u1, &loc, sixteenf2, maxCmp, lut, distCenter[viewId1[quad]], distFocalLengthInv, lut_u2d_stepInv);
                //Shashank: Beware of this bug, ldc can potentially be different for each camera too
                _mem4(&pLut[undist_index + 0]) = loc; // Image 1 Y, X co-ordinates

                undist_index += no_of_elems;
                #ifdef DEBUG_1
                printf("Completed call to 1st svFishEyeTransform \n");
                #endif
            }

            undist_cnt = 0;
            #pragma MUST_ITERATE(20, ,)
            for (j = colStart[quad]; j <= colEnd[quad]; j ++){

                __float2_t point_u2;
                int32_t loc;

                point_u2 = _amem8_f2_const(&pUndist[undist_cnt + 2]);

                undist_cnt += 4;

                svFisheyeTransformUndistToDist(point_u2, &loc, sixteenf2, maxCmp, lut, distCenter[viewId2[quad]], distFocalLengthInv, lut_u2d_stepInv);

                _mem4(&pLut[undist_index2 + 2]) = loc; // Image 2 Y, X co-ordinates

                undist_index2 += no_of_elems;
            }
        }
    }

    *lut_count = (((2 + sv->SVOutDisplayWidth / sv->subsampleratio)*(2 + sv->SVOutDisplayHeight / sv->subsampleratio))) * NUM_ELEMENTS;

    #ifdef PC_VERSION
        fp = fopen("GAlingLUT3D.bin", "wb");
        count = (((2 + sv->SVOutDisplayWidth / sv->subsampleratio)*(2 + sv->SVOutDisplayHeight / sv->subsampleratio))) * NUM_ELEMENTS;
        fwrite(sv->GAlignLUT3D, sizeof(short), count, fp);
        fclose(fp);
    #endif
}

