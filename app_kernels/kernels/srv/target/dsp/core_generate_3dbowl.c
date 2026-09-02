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

#include "ti/vxlib/vxlib.h"
#include "TI/tivx_srv_kernels.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"
#include "core_generate_3dbowl.h"
#include "DSPF_sp_svd_cn.h"
#include "DSPF_dp_svd_cn.h"
#include "DSPF_dp_qrd_cn.h"

#include <math.h>
#include <stdio.h>

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
* Name:        svGenerate_3D_Bowl()
*
* Description: Creates 3D Bowl Mesh in array of X, Y, Z coordinates for 3D SRV
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
void svGenerate_3D_Bowl(svGpuLutGen_t *sv,svGeometric_t *in_offset, float calmat_scaled[], float *GAlignLUT3D_XYZ )
{
    int16_t i,j;
    float X,Y,Z;
    int32_t count=0;
    float maxZ=0;
    float rel_dist_bowl=0;
    float rel_dist_bowl_sq=0;


    int32_t offsetXleft, offsetXright, offsetYfront, offsetYback;
 /* Current release doesn't support ADAPTIVE                               */
 /* However adaptive can be enabled later by defining the directives below */
 /* ADAPTIVE_FRONT : Only front camera is modulalted on depth              */
 /* ADAPTIVE_ENABLE_ALL : Remainng 3 sides modulated on depth              */
#ifdef ADAPTIVE_FRONT
    int32_t mmDistanceFront = (sv->ultrasonicCaptureInPtr->deviceInfo[0].distanceShort/2)*0.001*343;
#endif
#ifdef ADAPTIVE_ENABLE_ALL
    int32_t mmDistanceRight = (sv->ultrasonicCaptureInPtr->deviceInfo[1].distanceShort/2)*0.001*343;
    int32_t mmDistanceBack = (sv->ultrasonicCaptureInPtr->deviceInfo[2].distanceShort/2)*0.001*343;
    int32_t mmDistanceLeft = (sv->ultrasonicCaptureInPtr->deviceInfo[3].distanceShort/2)*0.001*343;
#endif

    int32_t offsetXmin;
    int32_t offsetYmin;

    int32_t max_dist=0;

    // Grid cell size in real-world measrements
    float step_size = 5; // in mm
    float SV_origin[2] = {0,0};
    float FC_loc[2] = {0,0};
    float BC_loc[2] = {0,0};
    float cam_distance=0;

    int32_t centerY = sv->SVOutDisplayHeight/2; // Center position
    int32_t centerX = sv->SVOutDisplayWidth/2;
    float *restrict pXYZBuffer = GAlignLUT3D_XYZ;

    /* Assume subsampleratio is a power of 2, find the shift factor */
    float  constant_slope =   480.0/480.0;
    float denom_inv;
    int32_t startY[4];
    int32_t endY[4];
    int32_t startX[4];
    int32_t endX[4];
    int32_t offsetX[4];
    int32_t offsetY[4];
    int32_t region;

    for(count=0;count<4;count++)
    {
        // -R't to calculate origin (>>20 for both t and R because they are multiplied)

        // Typecast to floats
        SV_origin[0] += -( calmat_scaled[0 + 12 * count] * calmat_scaled[ 9 + 12 * count]  \
                         + calmat_scaled[1 + 12 * count] * calmat_scaled[10 + 12 * count]  \
                         + calmat_scaled[2 + 12 * count] * calmat_scaled[11 + 12 * count] ) / sv->numCameras;

        SV_origin[1] += -( calmat_scaled[3 + 12 * count] * calmat_scaled[ 9 + 12 * count]  \
                         + calmat_scaled[4 + 12 * count] * calmat_scaled[10 + 12 * count]  \
                         + calmat_scaled[5 + 12 * count] * calmat_scaled[11 + 12 * count] ) / sv->numCameras;
//        printf("SV_Origin :%f\t%f\n", SV_origin[0], SV_origin[1]);

    }

    // Calculate step_size from calmat (assume sv->calmat[] array is already calculated (scale correction)

    /* Front camera */
    count =0;
    FC_loc[0] = -(  calmat_scaled[0 + 12 * count] * calmat_scaled[ 9 + 12 * count] \
                  + calmat_scaled[1 + 12 * count] * calmat_scaled[10 + 12 * count] \
                  + calmat_scaled[2 + 12 * count] * calmat_scaled[11 + 12 * count] );

    FC_loc[1] = -(  calmat_scaled[3 + 12 * count] * calmat_scaled[ 9 + 12 * count] \
                  + calmat_scaled[4 + 12 * count] * calmat_scaled[10 + 12 * count] \
                  + calmat_scaled[5 + 12 * count] * calmat_scaled[11 + 12 * count] );

    /* Back camera */
    count=2;
    BC_loc[0] = -(  calmat_scaled[0 + 12 * count] * calmat_scaled[ 9 + 12 * count] \
                  + calmat_scaled[1 + 12 * count] * calmat_scaled[10 + 12 * count] \
                  + calmat_scaled[2 + 12 * count] * calmat_scaled[11 + 12 * count] );

    BC_loc[1] = -(  calmat_scaled[3 + 12 * count] * calmat_scaled[ 9 + 12 * count] \
                  + calmat_scaled[4 + 12 * count] * calmat_scaled[10 + 12 * count] \
                  + calmat_scaled[5 + 12 * count] * calmat_scaled[11 + 12 * count] );


    // Front to back camera distance gives scale of car

    cam_distance = sqrt( (FC_loc[0]-BC_loc[0])*(FC_loc[0]-BC_loc[0]) + (FC_loc[1]-BC_loc[1])*(FC_loc[1]-BC_loc[1]) );

    // Step_size divides the length of the vehicle into hundred grid points. (100 can be user defined parameter)
    step_size = cam_distance/100;

#ifdef ADAPTIVE_ENABLE_ALL
    // Left bowl calculation
    if ( (mmDistanceLeft / step_size) > MAX_BOWL)
    {
        offsetXleft = -1 * MAX_BOWL;
    }
    else if ( (mmDistanceLeft / step_size) < MIN_BOWL)
    {
        offsetXleft = -1 * MIN_BOWL;
    }
    else
    {
        offsetXleft = (-1 * mmDistanceLeft) / step_size;
    }

    // Right bowl calculation
    if ( (mmDistanceRight / step_size) > MAX_BOWL)
    {
        offsetXright = MAX_BOWL;
    }
    else if ( (mmDistanceRight / step_size) < MIN_BOWL)
    {
        offsetXright = MIN_BOWL;
    }
    else
    {
        offsetXright = mmDistanceRight / step_size;
    }

    // Back bowl calculation
    int32_t mmTotalDistanceBack = mmDistanceBack + (step_size*100) / 2;
    if ( (mmTotalDistanceBack / step_size) > MAX_BOWL)
    {
        offsetYback = MAX_BOWL;
    }
    else if ( (mmTotalDistanceBack / step_size) < MIN_BOWL)
    {
        offsetYback = MIN_BOWL;
    }
    else
    {
        offsetYback = mmTotalDistanceBack / step_size;
    }
#else
//    offsetXleft = sv->offsetXleft; // = usm depth left (mm) / step_size;
//    offsetXright = sv->offsetXright; // = usm depth right (mm) / step_size;
//    offsetYback = sv->offsetYback;
    offsetXleft =  in_offset->offsetXleft; // = usm depth left (mm) / step_size;
    offsetXright = in_offset->offsetXright; // = usm depth right (mm) / step_size;
    offsetYback =  in_offset->offsetYback;
#endif


#ifdef ADAPTIVE_FRONT
    // Front bowl calculation
    int32_t mmTotalDistanceFront = mmDistanceFront + (step_size*100) / 2;
    if ( (mmTotalDistanceFront / step_size) > MAX_BOWL)
    {
        offsetYfront = -1 * MAX_BOWL;
    }
    else if ( (mmTotalDistanceFront / step_size) < MIN_BOWL)
    {
        offsetYfront = -1 * MIN_BOWL;
    }
    else
    {
        offsetYfront = (-1 * mmTotalDistanceFront) / step_size;
    }
#else
    offsetYfront = in_offset->offsetYfront;
#endif

    if (sv->useWideBowl)
    {
        offsetYfront = -1*FLAT_BOWL;
    }

    #ifdef FLATWORDVIEW
    maxZ = 0; //in mm
    #else
    //maxZ = 150*step_size;
    maxZ =   (540+ offsetYfront) * constant_slope *step_size;
    #endif


    // Max dist set to diagonal
    max_dist = sqrt( ((centerY)*(centerY)) + ((centerX)*(centerX)) );

    offsetXmin = (offsetXright < offsetXleft) ? offsetXright : offsetXleft;
    offsetYmin = (offsetYfront < offsetYback) ? offsetYfront : offsetYback;

    denom_inv = 1.0f / (max_dist - (sqrt(offsetYmin*offsetYmin + offsetXmin*offsetXmin)));

#if 0
    count = 0;
    for (i=0; i<sv->SVOutDisplayHeight; i+=sv->subsampleratio){
        /* Note: skip is hard-coded here to 4 for pipeline efficiency ... this value should match sv->subsampleratio */
        for (j=0; j<sv->SVOutDisplayWidth; j+=SKIP){

            int16_t j_dist = j - centerX;
            int16_t i_dist = i - centerY;

            // Bowl with rectangular base (rel_dist for bowl z height)
            // Max distance updatesd for non-uniform sampling

            rel_dist_bowl = 0;

            if(i_dist <= offsetYfront)
            {
                if(j_dist >= offsetXright)
                {
                    rel_dist_bowl = sqrtsp((i_dist-offsetYfront)*(i_dist-offsetYfront) + (j_dist-offsetXright)*(j_dist-offsetXright));
                }
                else if(j_dist <= offsetXleft)
                {
                    rel_dist_bowl = sqrtsp((i_dist-offsetYfront)*(i_dist-offsetYfront) + (j_dist-offsetXleft)*(j_dist-offsetXleft));
                }
                else
                {
                    rel_dist_bowl = abs(i_dist-offsetYfront);
                }
            }

            else if(i_dist >= offsetYback)
            {
                if(j_dist >= offsetXright)
                {
                    rel_dist_bowl = sqrtsp((i_dist-offsetYback)*(i_dist-offsetYback) + (j_dist-offsetXright)*(j_dist-offsetXright));
                }
                else if(j_dist <= offsetXleft)
                {
                    rel_dist_bowl = sqrtsp((i_dist-offsetYback)*(i_dist-offsetYback) + (j_dist-offsetXleft)*(j_dist-offsetXleft));
                }
                else
                {
                    rel_dist_bowl = abs(i_dist-offsetYback);
                }
            }
            else
            {
                if(j_dist >= offsetXright)
                {
                    rel_dist_bowl = abs(j_dist-offsetXright);
                }
                else if(j_dist <= offsetXleft)
                {
                    rel_dist_bowl = abs(j_dist-offsetXleft);
                }
                else
                {
                    rel_dist_bowl = 0;
                }
            }

            rel_dist_bowl = rel_dist_bowl * denom_inv;

            // Calculate World points
            X = (float)( ( (j - centerX) >> shift )*step_size);
            Y = (float)( ( (i - centerY) >> shift )*step_size);
            Z = (float)(maxZ*((rel_dist_bowl)));

            Y=-Y;

            // Shift co-ordinates in the new ego frame of reference
            // Put in XYZ buffer
            X = X + (SV_origin[0]);
            Y = Y + (SV_origin[1]);

            pXYZBuffer[count++] = X;
            pXYZBuffer[count++] = Y;
            pXYZBuffer[count++] = Z;
        }
    }
#else


    /* PROCESS CORNERS */

    /* Top left */
    startY[0] = 0;
    endY[0] = ((centerY + offsetYfront + SKIP) / SKIP) * SKIP;
    startX[0] = 0;
    endX[0] = ((centerX + offsetXleft + SKIP) / SKIP) * SKIP;
    offsetX[0] = offsetXleft;
    offsetY[0] = offsetYfront;

    /* Top right */
    startY[1] = 0;
    endY[1] = ((centerY + offsetYfront + SKIP) / SKIP) * SKIP;
    startX[1] = ((centerX + offsetXright + (SKIP-1)) / SKIP) * SKIP;
    endX[1] = sv->SVOutDisplayWidth;
    offsetX[1] = offsetXright;
    offsetY[1] = offsetYfront;

    /* Back left */
    startY[2] = ((centerY + offsetYback + (SKIP-1)) / SKIP) * SKIP;
    endY[2] = sv->SVOutDisplayHeight;
    startX[2] = 0;
    endX[2] = ((centerX + offsetXleft + SKIP) / SKIP) * SKIP;
    offsetX[2] = offsetXleft;
    offsetY[2] = offsetYback;

    /* Back right */
    startY[3] = ((centerY + offsetYback + (SKIP-1)) / SKIP) * SKIP;
    endY[3] = sv->SVOutDisplayHeight;
    startX[3] = ((centerX + offsetXright + (SKIP-1)) / SKIP) * SKIP;
    endX[3] = sv->SVOutDisplayWidth;
    offsetX[3] = offsetXright;
    offsetY[3] = offsetYback;

    count = 0;
    for (region = 0; region < 4; region++) {
        for (i=startY[region]; i<endY[region]; i+=sv->subsampleratio){

            int i_dist = i - centerY;
            count = (i / SKIP * (sv->SVOutDisplayWidth / SKIP) + startX[region] / SKIP) * 3;

            /* Note: skip is hard-coded here to SKIP for pipeline efficiency ... this value should match sv->subsampleratio */
            for (j=startX[region]; j<endX[region]; j+=SKIP){

                int j_dist = j - centerX;

                // Bowl with rectangular base (rel_dist for bowl z height)
                // Max distance updatesd for non-uniform sampling

                rel_dist_bowl_sq = ((i_dist-offsetY[region])*(i_dist-offsetY[region]) +
                                       (j_dist-offsetX[region])*(j_dist-offsetX[region]));
                rel_dist_bowl  = 0.0;
                if(0.0 != rel_dist_bowl_sq)
                {
                    rel_dist_bowl = sqrtsp(rel_dist_bowl_sq);
                }
                /* Need to check for NAN since sqrt of 0 is undefined by sqrtsp */
               
                rel_dist_bowl = rel_dist_bowl * denom_inv;
                // Calculate World points
                X = (float)( (j - centerX)*step_size);
                Y = (float)( (i - centerY)*step_size);
                Z = (float)(maxZ*((rel_dist_bowl)));

                Y=-Y;

                // Shift co-ordinates in the new ego frame of reference
                // Put in XYZ buffer
                X = X + (SV_origin[0]);
                Y = Y + (SV_origin[1]);

                pXYZBuffer[count++] = X;
                pXYZBuffer[count++] = Y;
                pXYZBuffer[count++] = Z;
            }
        }
    }

    /* PROCESS SIDES */

    /* Top */
    startY[0] = 0;
    endY[0] = ((centerY + offsetYfront + SKIP) / SKIP) * SKIP;
    startX[0] = ((centerX + offsetXleft + SKIP) / SKIP) * SKIP;
    endX[0] = ((centerX + offsetXright + (SKIP-1)) / SKIP) * SKIP;
    offsetX[0] = offsetXleft;
    offsetY[0] = offsetYfront;

    /* Left */
    startY[1] = ((centerY + offsetYfront + SKIP) / SKIP) * SKIP;
    endY[1] = ((centerY + offsetYback + (SKIP-1)) / SKIP) * SKIP;
    startX[1] = 0;
    endX[1] = ((centerX + offsetXleft + SKIP) / SKIP) * SKIP;
    offsetX[1] = offsetXleft;
    offsetY[1] = offsetYfront;

    /* Right */
    startY[3] = ((centerY + offsetYfront + SKIP) / SKIP) * SKIP;
    endY[3] = ((centerY + offsetYback + (SKIP-1)) / SKIP) * SKIP;
    startX[3] = ((centerX + offsetXright + (SKIP-1)) / SKIP) * SKIP;
    endX[3] = sv->SVOutDisplayWidth;
    offsetX[3] = offsetXright;
    offsetY[3] = offsetYback;

    /* Back */
    startY[2] = ((centerY + offsetYback + (SKIP-1)) / SKIP) * SKIP;
    endY[2] = sv->SVOutDisplayHeight;
    startX[2] = ((centerX + offsetXleft + SKIP) / SKIP) * SKIP;
    endX[2] = ((centerX + offsetXright + (SKIP-1)) / SKIP) * SKIP;
    offsetX[2] = offsetXright;
    offsetY[2] = offsetYback;

    for (region = 0; region < 4; region++) {
        for (i=startY[region]; i<endY[region]; i+=sv->subsampleratio){

            int i_dist = i - centerY;
            int i_diff = abs(i_dist - offsetY[region]);
            count = (i/SKIP*(sv->SVOutDisplayWidth/SKIP)+startX[region]/SKIP)*3;

            /* Note: skip is hard-coded here to SKIP for pipeline efficiency ... this value should match sv->subsampleratio */
            for (j=startX[region]; j<endX[region]; j+=SKIP){

                int j_dist = j - centerX;
                int j_diff = abs(j_dist - offsetX[region]);

                // Bowl with rectangular base (rel_dist for bowl z height)
                // Max distance updatesd for non-uniform sampling

                rel_dist_bowl = (region & 1) ? j_diff : i_diff;
                
                rel_dist_bowl = rel_dist_bowl * denom_inv;

                // Calculate World points
                X = (float)( (j - centerX)*step_size);
                Y = (float)( (i - centerY)*step_size);
                Z = (float)(maxZ*((rel_dist_bowl)));

                Y=-Y;

                // Shift co-ordinates in the new ego frame of reference
                // Put in XYZ buffer
                X = X + (SV_origin[0]);
                Y = Y + (SV_origin[1]);

                pXYZBuffer[count++] = X;
                pXYZBuffer[count++] = Y;
                pXYZBuffer[count++] = Z;
            }
        }
    }

    /* PROCESS CENTER */

    startY[0] = ((centerY + offsetYfront + SKIP) / SKIP) * SKIP;
    endY[0] = ((centerY + offsetYback + (SKIP-1)) / SKIP) * SKIP;
    startX[0] = ((centerX + offsetXleft + SKIP) / SKIP) * SKIP;
    endX[0] = ((centerX + offsetXright + (SKIP-1)) / SKIP) * SKIP;

    Z = 0.0f;
    for (i=startY[0]; i<endY[0]; i+=sv->subsampleratio){
        /* Note: skip is hard-coded here to SKIP for pipeline efficiency ... this value should match sv->subsampleratio */
        Y = (float)( (i - centerY)*step_size);
        Y=-Y;
        Y = Y + (SV_origin[1]);
        count = (i / SKIP * (sv->SVOutDisplayWidth / SKIP) + startX[0] / SKIP) * 3;
        for (j=startX[0]; j<endX[0]; j+=SKIP){

            // Calculate World points
            X = (float)( (j - centerX)*step_size);

            // Shift co-ordinates in the new ego frame of reference
            // Put in XYZ buffer
            X = X + (SV_origin[0]);

            pXYZBuffer[count++] = X;
            pXYZBuffer[count++] = Y;
            pXYZBuffer[count++] = Z;
        }
    }

#endif
#ifdef PC_VERSION
    FILE *fp;
    fp = fopen("XYZLUT3D.bin", "wb");
    count = sv->SVOutDisplayHeight / SKIP * sv->SVOutDisplayWidth/SKIP * 3;
    fwrite(pXYZBuffer, sizeof(float), count, fp);
    fclose(fp);
#endif
}

