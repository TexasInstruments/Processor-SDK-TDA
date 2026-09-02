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

#include "TI/tivx_srv_kernels.h"
#include "ti/vxlib/vxlib.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"
#include "core_pose_estimation.h"
#include "DSPF_sp_svd_cn.h"
#include "DSPF_dp_svd_cn.h"
#include "DSPF_dp_qrd_cn.h"

#include <math.h>
#include <stdio.h>


//matrix-vector multiply vout = m * vin;

//matrix multiplication m3 = m1 * m2;
static inline void multiply(Matrix3D_f *m1, Matrix3D_f *m2, Matrix3D_f *mout){
        mout->xx = m1->xx * m2->xx + m1->xy * m2->yx + m1->xz * m2->zx;
        mout->xy = m1->xx * m2->xy + m1->xy * m2->yy + m1->xz * m2->zy;
        mout->xz = m1->xx * m2->xz + m1->xy * m2->yz + m1->xz * m2->zz;

        mout->yx = m1->yx * m2->xx + m1->yy * m2->yx + m1->yz * m2->zx;
        mout->yy = m1->yx * m2->xy + m1->yy * m2->yy + m1->yz * m2->zy;
        mout->yz = m1->yx * m2->xz + m1->yy * m2->yz + m1->yz * m2->zz;

        mout->zx = m1->zx * m2->xx + m1->zy * m2->yx + m1->zz * m2->zx;
        mout->zy = m1->zx * m2->xy + m1->zy * m2->yy + m1->zz * m2->zy;
        mout->zz = m1->zx * m2->xz + m1->zy * m2->yz + m1->zz * m2->zz;
}
//matrix multiplication mout = m1 * m2 * m3;
static inline void multiply3(Matrix3D_f *m1, Matrix3D_f *m2, Matrix3D_f *m3, Matrix3D_f *mout){
        Matrix3D_f temp;
        multiply(m1, m2, &temp);
        multiply(&temp, m3, mout);
}

//matrix multiplication mout = m1 * m2 * m3 * m4;
#if 0
static inline void multiplyMatrixVector(Matrix3D_f *m, Point3D_f *vin, Point3D_f *vout)
{
        vout->x = m->xx*vin->x + m->xy*vin->y + m->xz*vin->z;
        vout->y = m->yx*vin->x + m->yy*vin->y + m->yz*vin->z;
        vout->z = m->zx*vin->x + m->zy*vin->y + m->zz*vin->z;
}

static inline void multiply4(Matrix3D_f *m1, Matrix3D_f *m2, Matrix3D_f *m3, Matrix3D_f *m4, Matrix3D_f *mout){
        Matrix3D_f temp1;
        Matrix3D_f temp2;
        multiply(m1, m2, &temp1);
        multiply(m3, m4, &temp2);
        multiply(&temp1, &temp2, mout);
}

//matrix transpose m = m';
static inline void selftranspose(Matrix3D_f *m_in_out){
        Flouble temp;
        temp = m_in_out->xy; m_in_out->xy = m_in_out->yx; m_in_out->yx = temp;
        temp = m_in_out->xz; m_in_out->xz = m_in_out->zx; m_in_out->zx = temp;
        temp = m_in_out->yz; m_in_out->yz = m_in_out->zy; m_in_out->zy = temp;
}

//matrix transpose m_out = m_in';
static inline void transpose(Matrix3D_f *m_in, Matrix3D_f *m_out){
        m_out->xx = m_in->xx; m_out->xy = m_in->yx; m_out->xz = m_in->zx;
        m_out->yx = m_in->xy; m_out->yy = m_in->yy; m_out->yz = m_in->zy;
        m_out->zx = m_in->xz; m_out->zy = m_in->yz; m_out->zz = m_in->zz;
}

//Flouble to diagonal mat data assignment
static inline void Flouble2diagmat(Flouble *in, Matrix3D_f *out){
        out->xx = in[0];
        out->yy = in[1];
        out->zz = in[2];
}

//Flouble to mat data assignment
static inline void Flouble2mat(Flouble **in, Matrix3D_f *out){
        out->xx = in[0][0];
        out->xy = in[0][1];
        out->xz = in[0][2];
        out->yx = in[1][0];
        out->yy = in[1][1];
        out->yz = in[1][2];
        out->zx = in[2][0];
        out->zy = in[2][1];
        out->zz = in[2][2];
}

//Flouble to mat data assignment
static inline void mat2Flouble(Matrix3D_f *in, Flouble **out){
        out[0][0] = in->xx;
        out[0][1] = in->xy;
        out[0][2] = in->xz;
        out[1][0] = in->yx;
        out[1][1] = in->yy;
        out[1][2] = in->yz;
        out[2][0] = in->zx;
        out[2][1] = in->zy;
        out[2][2] = in->zz;
}

//matrix inverse m_out = inv(m_in);
static inline void inverse(Matrix3D_f *m_in, Matrix3D_f *m_out){
        Flouble det = determinant(m_in);
        Flouble invdet = 1 / det;
        m_out->xx = invdet * (m_in->yy * m_in->zz - m_in->yz * m_in->zy);
        m_out->yx =-invdet * (m_in->yx * m_in->zz - m_in->yz * m_in->zx);
        m_out->zx = invdet * (m_in->yx * m_in->zy - m_in->yy * m_in->zx);
        m_out->xy =-invdet * (m_in->xy * m_in->zz - m_in->xz * m_in->zy);
        m_out->yy = invdet * (m_in->xx * m_in->zz - m_in->xz * m_in->zx);
        m_out->zy =-invdet * (m_in->xx * m_in->zy - m_in->xy * m_in->zx);
        m_out->xz = invdet * (m_in->xy * m_in->yz - m_in->xz * m_in->yy);
        m_out->yz =-invdet * (m_in->xx * m_in->yz - m_in->xz * m_in->yx);
        m_out->zz = invdet * (m_in->xx * m_in->yy - m_in->xy * m_in->yx);
}

//pose inverse
static inline void invRT(Matrix3D_f *R_in, Point3D_f *t_in, Matrix3D_f *R_out, Point3D_f *t_out){
        R_out->xx = R_in->xx; R_out->xy = R_in->yx; R_out->xz = R_in->zx;
        R_out->yx = R_in->xy; R_out->yy = R_in->yy; R_out->yz = R_in->zy;
        R_out->zx = R_in->xz; R_out->zy = R_in->yz; R_out->zz = R_in->zz;

        t_out->x = -(R_out->xx*t_in->x + R_out->xy*t_in->y + R_out->xz*t_in->z);
        t_out->y = -(R_out->yx*t_in->x + R_out->yy*t_in->y + R_out->yz*t_in->z);
        t_out->z = -(R_out->zx*t_in->x + R_out->zy*t_in->y + R_out->zz*t_in->z);
}

//p = -p;
static inline void changesign_vec(Point3D_f *p){
        p->x = -p->x;
        p->y = -p->y;
        p->z = -p->z;
}

//returns the determinant of the matrix
static inline Flouble determinant(Matrix3D_f *m){
        return(m->xx * (m->yy*m->zz - m->yz*m->zy)
                 - m->xy *(m->yx*m->zz - m->yz*m->zx)
                 + m->xz*(m->yx*m->zy - m->yy*m->zx)
                 );
}

#endif








// m = -m;
static inline void changesign_mat(Matrix3D_f *m){
        m->xx = -m->xx; m->xy = -m->xy; m->xz = -m->xz;
        m->yx = -m->yx; m->yy = -m->yy; m->yz = -m->yz;
        m->zx = -m->zx; m->zy = -m->zy; m->zz = -m->zz;
}


/*=======================================================================
*
* Name:        svPoseEstimate()
*
* Description: Call pose estimate process
*
*
* Input:
*   sv:             Handle for the SV structure
*   inCornerPoints  detected corner points from automatic chart detection
*   outcalmat       output calibration matrix
*
* Returns:
*               return if successful or not
*
* Effects:
*
*
=======================================================================*/

int32_t svPoseEstimate(svPoseEstimation_t *sv, svLdcLut_t *ldcLut, tivxPoseEstimationParams *prms,  svACDetectStructFourCameraCorner_t *inCornerPointsStruct, int8_t* inChartPos, int32_t* outcalmat,  uint8_t* bLeftChart, int8_t pEstimateMode)
{


    int16_t i,idx;
    int32_t *inCornerPoints;
    int32_t vIdx;
    int16_t firstChartLeft;

#if 0
    if (sv->SingleChartPose == 1) {
        memcpy(sv->detectedPointNum, pointNum, sizeof(uint8_t)*MAX_INPUT_CAMERAS);
        memcpy(sv->bLeftChartOnly, bLeftChart, sizeof(uint8_t)*MAX_INPUT_CAMERAS);
    }
#endif
    /* copy numFPDetected to detectedPointNum to maintain consistency with the rest of the code */
    for (i=0;i<MAX_INPUT_CAMERAS;i++)
    {
        prms->detectedPointNum[i] =  inCornerPointsStruct->finalCameraCorners[i].numFPDetected; 
        prms->bLeftChartOnly[i] = 0;
    }

    /**********************************************************************************
     * Notes:
     *  1. sv->numCameras, sv->SVInCamFrmHeight and sv->SVInCamFrmWidth should be available beforehand (not existing in this function)
     *  2. ldcLut should be available beforehand (not existing in this function)
     *  3. pEstimateMode can be used to diable/enable pose estimation
     *  4. defaultCalmat is used as calmat when pose estimation fails.
     *  5. Need to initialize chart position - initChartPoints(sv);
     *  6. need to rearrange detected corner points in the format that pose estimation needs - copyCornerPoints(sv, inCornerPoints);
     *  8. calmat should be contstructed from R and T - constructCalmat(sv)
     *  9. inChartPos is from binary file. It has 128 byte header, which include the # of charts.
     *     After 128 bytes, it has chart size, (x, y) offsets of each chart w.r.t bottom-left chart.
     *     We can read as follows,
     *
     *     FILE*stream = fopen("chartPos.bin", "rb");
     *     if (stream != NULL) {
     *        fread(inChartPos, CHART_POS_INPUT_SIZE, sizeof(uint8_t), stream); // CHART_POS_INPUT_SIZE = 164
     *      }
     *
     *  1 ~ 9 could be move to Interface file
     **********************************************************************************/

    int32_t defaultCalmat[] = { 1048228, -24618, 11152, -22292, -542958, 896778, -15280, -896718, -543301, -415248858, 643731818, -609801271, -22251, -572800, 878018, -1047745, 41739, 677, -35320, -877308, -573232, 596605052, 478182630, -316322287, -1048064, 21245, -24946, 32104, 825577, -645681, 6559, -646129, -825824, 402937714, -142613410, 412598008, 67754, 552206, -888814, 1045850, -7271, 75207, 33442, -891363, -551240, -608558653, 51440753, 353065845 };

    /* Rearrange inCornerPoints as used by the legacy code */
    inCornerPoints = prms->buf_inCornerPoints_ptr;
    inCornerPoints[0] = 1;   
    inCornerPoints++;

    firstChartLeft =1;
    for (vIdx =0; vIdx < sv->numCameras; vIdx++) {
        for (i = 0; i < 2; i++)  // FP_TO_DETECT = 2
        {
            //Create 1 d array and flip x/y ordering inCornerPointsStruct
            idx = firstChartLeft ? i : 1 - i;
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS]     = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][1];  // flip x- and y- coordinates
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 1] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][0];
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 2] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][3];  // flip x- and y- coordinates
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 3] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][2];
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 4] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][5];  // flip x- and y- coordinates
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 5] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][4];
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 6] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][7];  // flip x- and y- coordinates
            inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 7] = inCornerPointsStruct->finalCameraCorners[vIdx].finalCorners[idx][6];

            #ifdef DEBUG_1
            printf("Corners are %d %d %d %d %d %d %d %d \n",inCornerPoints[vIdx*16 +i*8 + 0],
                       inCornerPoints[vIdx*16 +i*8 + 1],inCornerPoints[vIdx*16 +i*8 + 2],inCornerPoints[vIdx*16 +i*8 + 3],
                       inCornerPoints[vIdx*16 +i*8 + 4],inCornerPoints[vIdx*16 +i*8 + 5],inCornerPoints[vIdx*16 +i*8 + 6],
                       inCornerPoints[vIdx*16 +i*8 + 7]);
           #endif
         }
    }
    
    inCornerPoints--; /* Get inCornerPoints pointer to default */

    for (i = 0; i<sv->numCameras * 12; i++) {
        outcalmat[i] = defaultCalmat[i];
    }

    if (pEstimateMode == 0)
    {
        return (ALGORITHM_PROCESS_DISABLED);
    }
    else if (pEstimateMode == 1)
    {
        // check if pattern detection is successful
        // if not successful, we use default calmat
        if (inCornerPoints[0] == 0)
            return (ALGORITHM_PROCESS_FAIL);

        inCornerPoints++;

        // set chart position
        initChartPoints(sv,prms, inChartPos);

        // rearrange detected corner points
        copyCornerPoints(sv, prms, inCornerPoints);


        // set frame center and init camera inistrinsics
        svSetFrameCentersPEstimate(sv,ldcLut,prms);

        // do-kwon: JUST FOR TEST
        for (vIdx = 0; vIdx < sv->numCameras; vIdx++) {
            // place holder
            svExtrinsicPoseEstimation(sv, ldcLut,prms, inCornerPointsStruct, vIdx, prms->buf_chartPoints_ptr[vIdx],
            prms->buf_cornerPoints_ptr[vIdx], prms->buf_normCornerPoint_ptr, 
            prms->buf_cip_ptr[vIdx]->distFocalLength, 
            prms->buf_cip_ptr[vIdx]->distCenterX, prms->buf_cip_ptr[vIdx]->distCenterY);

            // calculate H_cg
            prms->Rt.xx = prms->buf_R_gc_ptr[vIdx].xx;  prms->Rt.xy = prms->buf_R_gc_ptr[vIdx].xy;  prms->Rt.xz = prms->buf_t_gc_ptr[vIdx].x;
            prms->Rt.yx = prms->buf_R_gc_ptr[vIdx].yx;  prms->Rt.yy = prms->buf_R_gc_ptr[vIdx].yy;  prms->Rt.yz = prms->buf_t_gc_ptr[vIdx].y;
            prms->Rt.zx = prms->buf_R_gc_ptr[vIdx].zx;  prms->Rt.zy = prms->buf_R_gc_ptr[vIdx].zy;  prms->Rt.zz = prms->buf_t_gc_ptr[vIdx].z;

            multiply(&prms->buf_cip_ptr[vIdx]->K, &prms->Rt, &prms->buf_H_cg_ptr[vIdx]);


        }

        // construct calmat from R and T
        constructCalmat(sv,prms,outcalmat);
        return ALGORITHM_PROCESS_OK;
    }
    else
    {
        return ALGORITHM_PROCESS_FAIL;
    }
}



/*===============================================================================
*
* Name:        initChartPoints()
*
* Description: Set chart positions
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
* Effects:
*
===============================================================================*/
 void initChartPoints(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms, int8_t* inChartPos)
{
    int32_t i;
#ifdef PC_VERSION
    int32_t vIdx;
#endif

    int16_t chartNum, chartSize, blOffset[2], tlOffset[2], trOffset[2], brOffset[2];
    int32_t *inChartPos32;


    // default chart offsets
    chartNum = 4;
    chartSize = 200;

    blOffset[0] = 0;   // x
    blOffset[1] = 1;   // y
    tlOffset[0] = 0;   // x
    tlOffset[1] = 764; // y
    trOffset[0] = 561; // x
    trOffset[1] = 764; // y
    brOffset[0] = 561; // x
    brOffset[1] = 0;   // y

    if (inChartPos != NULL)
    {
        inChartPos32 = (int32_t*)inChartPos;
        chartNum = inChartPos32[0];

        if (chartNum != 4)
        {
#ifdef PC_VERSION
            printf("The number of charts should be 3! \n");
#endif
        }
        else
        {
            inChartPos32 = (int32_t *)(inChartPos + 128); // header size = 128

            chartSize = inChartPos32[0];

            // assuming header size is 128 bytes
            blOffset[0] = inChartPos32[1]; // x
            blOffset[1] = inChartPos32[2]; // y

            tlOffset[0] = inChartPos32[3]; // x
            tlOffset[1] = inChartPos32[4]; // y

            trOffset[0] = inChartPos32[5]; // x
            trOffset[1] = inChartPos32[6]; // y

            brOffset[0] = inChartPos32[7]; // x
            brOffset[1] = inChartPos32[8]; // y

            #ifdef DEBUG_1
            printf (" Chart Size  = %d\n",inChartPos32[0]);
            printf (" Chart Pos 1 = %d,%d\n",inChartPos32[1], inChartPos32[2]);
            printf (" Chart Pos 2 = %d,%d\n",inChartPos32[3], inChartPos32[4]);
            printf (" Chart Pos 3 = %d,%d\n",inChartPos32[5], inChartPos32[6]);
            printf (" Chart Pos 4 = %d,%d\n",inChartPos32[7], inChartPos32[8]);
            #endif
        }
    }


    prms->buf_baseChartPoints_ptr[0][0].x = 0;                prms->buf_baseChartPoints_ptr[0][0].y = 0;
    prms->buf_baseChartPoints_ptr[0][1].x = chartSize;        prms->buf_baseChartPoints_ptr[0][1].y = 0;
    prms->buf_baseChartPoints_ptr[0][2].x = chartSize;        prms->buf_baseChartPoints_ptr[0][2].y = -chartSize;
    prms->buf_baseChartPoints_ptr[0][3].x = 0;                prms->buf_baseChartPoints_ptr[0][3].y = -chartSize;

    prms->buf_baseChartPoints_ptr[1][0].x = chartSize;        prms->buf_baseChartPoints_ptr[1][0].y = 0;
    prms->buf_baseChartPoints_ptr[1][1].x = chartSize;        prms->buf_baseChartPoints_ptr[1][1].y = -chartSize;
    prms->buf_baseChartPoints_ptr[1][2].x = 0;                prms->buf_baseChartPoints_ptr[1][2].y = -chartSize;
    prms->buf_baseChartPoints_ptr[1][3].x = 0;                prms->buf_baseChartPoints_ptr[1][3].y = 0;

    prms->buf_baseChartPoints_ptr[2][0].x = chartSize;        prms->buf_baseChartPoints_ptr[2][0].y = -chartSize;
    prms->buf_baseChartPoints_ptr[2][1].x = 0;                prms->buf_baseChartPoints_ptr[2][1].y = -chartSize;
    prms->buf_baseChartPoints_ptr[2][2].x = 0;                prms->buf_baseChartPoints_ptr[2][2].y = 0;
    prms->buf_baseChartPoints_ptr[2][3].x = chartSize;        prms->buf_baseChartPoints_ptr[2][3].y = 0;

    prms->buf_baseChartPoints_ptr[3][0].x = 0;                prms->buf_baseChartPoints_ptr[3][0].y = -chartSize;
    prms->buf_baseChartPoints_ptr[3][1].x = 0;                prms->buf_baseChartPoints_ptr[3][1].y = 0;
    prms->buf_baseChartPoints_ptr[3][2].x = chartSize;        prms->buf_baseChartPoints_ptr[3][2].y = 0;
    prms->buf_baseChartPoints_ptr[3][3].x = chartSize;        prms->buf_baseChartPoints_ptr[3][3].y = -chartSize;

    prms->chartShift_tl.x = tlOffset[0];              prms->chartShift_tl.y = tlOffset[1] + chartSize;
    prms->chartShift_tr.x = trOffset[0];              prms->chartShift_tr.y = trOffset[1] + chartSize;
    prms->chartShift_bl.x = blOffset[0];              prms->chartShift_bl.y = blOffset[1] + chartSize;
    prms->chartShift_br.x = brOffset[0];              prms->chartShift_br.y = brOffset[1] + chartSize;


    for (i = 0; i < 4; i++)
    {
        // camera0
        prms->buf_chartPoints_ptr[0][i].x     = prms->buf_baseChartPoints_ptr[0][i].x + prms->chartShift_tl.x;
        prms->buf_chartPoints_ptr[0][i].y     = prms->buf_baseChartPoints_ptr[0][i].y + prms->chartShift_tl.y;;
        prms->buf_chartPoints_ptr[0][i + 4].x = prms->buf_baseChartPoints_ptr[0][i].x + prms->chartShift_tr.x;
        prms->buf_chartPoints_ptr[0][i + 4].y = prms->buf_baseChartPoints_ptr[0][i].y + prms->chartShift_tr.y;

        // camera1
        prms->buf_chartPoints_ptr[1][i].x     = prms->buf_baseChartPoints_ptr[1][i].x + prms->chartShift_tr.x;
        prms->buf_chartPoints_ptr[1][i].y     = prms->buf_baseChartPoints_ptr[1][i].y + prms->chartShift_tr.y;
        prms->buf_chartPoints_ptr[1][i + 4].x = prms->buf_baseChartPoints_ptr[1][i].x + prms->chartShift_br.x;
        prms->buf_chartPoints_ptr[1][i + 4].y = prms->buf_baseChartPoints_ptr[1][i].y + prms->chartShift_br.y;

        // camera2
        prms->buf_chartPoints_ptr[2][i].x     = prms->buf_baseChartPoints_ptr[2][i].x + prms->chartShift_br.x;
        prms->buf_chartPoints_ptr[2][i].y     = prms->buf_baseChartPoints_ptr[2][i].y + prms->chartShift_br.y;
        prms->buf_chartPoints_ptr[2][i + 4].x = prms->buf_baseChartPoints_ptr[2][i].x + prms->chartShift_bl.x;
        prms->buf_chartPoints_ptr[2][i + 4].y = prms->buf_baseChartPoints_ptr[2][i].y + prms->chartShift_bl.y;

        // camera3
        prms->buf_chartPoints_ptr[3][i].x     = prms->buf_baseChartPoints_ptr[3][i].x + prms->chartShift_bl.x;
        prms->buf_chartPoints_ptr[3][i].y     = prms->buf_baseChartPoints_ptr[3][i].y + prms->chartShift_bl.y;
        prms->buf_chartPoints_ptr[3][i + 4].x = prms->buf_baseChartPoints_ptr[3][i].x + prms->chartShift_tl.x;
        prms->buf_chartPoints_ptr[3][i + 4].y = prms->buf_baseChartPoints_ptr[3][i].y + prms->chartShift_tl.y;;
    }


#ifdef PC_VERSION
    printf("\nchart positions: \n");
    for (vIdx = 0; vIdx < 4; vIdx++)
    {
        printf("camera %d - \n", vIdx);
        printf("(%.1f, %.1f), (%.1f, %.1f), (%.1f, %.1f), (%.1f, %.1f), (%.1f, %.1f), (%.1f, %.1f), (%.1f, %.1f), (%.1f, %.1f)\n",
            prms->buf_chartPoints_ptr[vIdx][0].x, prms->buf_chartPoints_ptr[vIdx][0].y, prms->buf_chartPoints_ptr[vIdx][1].x, prms->buf_chartPoints_ptr[vIdx][1].y,
            prms->buf_chartPoints_ptr[vIdx][2].x, prms->buf_chartPoints_ptr[vIdx][2].y, prms->buf_chartPoints_ptr[vIdx][3].x, prms->buf_chartPoints_ptr[vIdx][3].y,
            prms->buf_chartPoints_ptr[vIdx][4].x, prms->buf_chartPoints_ptr[vIdx][4].y, prms->buf_chartPoints_ptr[vIdx][5].x, prms->buf_chartPoints_ptr[vIdx][5].y,
            prms->buf_chartPoints_ptr[vIdx][6].x, prms->buf_chartPoints_ptr[vIdx][6].y, prms->buf_chartPoints_ptr[vIdx][7].x, prms->buf_chartPoints_ptr[vIdx][7].y);
    }
#endif

}

/*===============================================================================
*
* Name:        copyCornerPoints()
*
* Description: rearrange detected corner points
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
* Effects:
*
===============================================================================*/
 void copyCornerPoints(svPoseEstimation_t *sv,tivxPoseEstimationParams *prms,  int32_t * inCornerPoints)
{
    int32_t i, vIdx;

#ifdef PC_VERSION
    printf("\ndetected corner positions: \n");
#endif
    for (vIdx = 0; vIdx < sv->numCameras; vIdx++)
    {
        for (i = 0; i < FP_TO_DETECT; i++)
        {
            // flip x- and y- coordinates
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART].x = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART].y = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 1] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 1].x = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 2] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 1].y = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 3] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 2].x = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 4] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 2].y = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 5] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 3].x = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 6] / 16.0;
            prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 3].y = inCornerPoints[vIdx * NUM_CHART_CORNERS * 2 + i * NUM_CHART_CORNERS + 7] / 16.0;

#ifdef PC_VERSION
            printf("camera %d - corner %d\n", vIdx, i);
            printf("(%.4f, %.4f), (%.4f, %.4f), (%.4f, %.4f), (%.4f, %.4f)\n",
                prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART].x, prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART].y,
                prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 1].x, prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 1].y,
                prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 2].x, prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 2].y,
                prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 3].x, prms->buf_cornerPoints_ptr[vIdx][i*NUM_CORNERS_PER_CHART + 3].y);
#endif
        }
    }

}

/*===============================================================================
 *
 * Name:        svSetFrameCentersPEstimate()
 *
 * Description: Set input frame distortion centers and find the center of each input
 *              view in the output frame
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
 void svSetFrameCentersPEstimate(svPoseEstimation_t* sv,svLdcLut_t *ldcLut, tivxPoseEstimationParams *prms )
{
    int32_t i;

    for (i = 0; i < sv->numCameras; i++)
    {
        prms->buf_cip_ptr[i]->distFocalLength = ldcLut->ldc[i].distFocalLength;
        prms->buf_cip_ptr[i]->distCenterY     = ldcLut->ldc[i].distCenterY;
        prms->buf_cip_ptr[i]->distCenterX     = ldcLut->ldc[i].distCenterX;

        initializePECameraIntrinsics(prms->buf_cip_ptr[i]);
    }
}

 void initializePECameraIntrinsics(CameraIntrinsicParams* cip)
{
    cip->K.xx = cip->distFocalLength;
    cip->K.xy = 0;
    cip->K.xz = cip->distCenterX;
    cip->K.yx = 0;
    cip->K.yy = cip->distFocalLength;
    cip->K.yz = cip->distCenterY;
    cip->K.zx = 0;
    cip->K.zy = 0;
    cip->K.zz = 1;

    Flouble invF = 1.0 / cip->distFocalLength;
    cip->invK.xx = invF;
    cip->invK.xy = 0;
    cip->invK.xz = -invF * cip->distCenterX;
    cip->invK.yx = 0;
    cip->invK.yy = invF;
    cip->invK.yz = -invF * cip->distCenterY;
    cip->invK.zx = 0;
    cip->invK.zy = 0;
    cip->invK.zz = 1;

}


/*=======================================================================
*
* Name:        svExtrinsicPoseEstimation()
*
* Description: estimate pose of each camera
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
*
* Effects:
*
*
=======================================================================*/

 void svExtrinsicPoseEstimation(svPoseEstimation_t *sv,svLdcLut_t *ldcLut, tivxPoseEstimationParams *prms,
                svACDetectStructFourCameraCorner_t *inCornerPointsStruct, 
                int16_t vIdx,  Point2D_f *chartPoints, Point2D_f *cornerPoints, Point2D_f *normCornerPoints, uint16_t focalLength,
                uint16_t distCentX, uint16_t distCentY)
{
    int16_t i;
    uint8_t peMethod = sv->Ransac;
    Point2D_f * chartPointsWorld = chartPoints;
    Flouble point_hom_2d[3];

    // LDC
    dtype point_d[2], point_u[2];
    dtype rdSq;


    for (i = 0; i < NUM_CHART_CORNERS; i++)
    {
        // fisheye correction
        // LDC
        point_d[0] = (dtype)cornerPoints[i].x;
        point_d[1] = (dtype)cornerPoints[i].y;

        LDC_DistToUndist(&ldcLut->ldc[vIdx], point_d, point_u, &rdSq);
        normCornerPoints[i].x = point_u[0];
        normCornerPoints[i].y = point_u[1];

        // image point normalization
        point_hom_2d[0] = prms->buf_cip_ptr[vIdx]->invK.xx * normCornerPoints[i].x + prms->buf_cip_ptr[vIdx]->invK.xy * normCornerPoints[i].y + prms->buf_cip_ptr[vIdx]->invK.xz;
        point_hom_2d[1] = prms->buf_cip_ptr[vIdx]->invK.yx * normCornerPoints[i].x + prms->buf_cip_ptr[vIdx]->invK.yy * normCornerPoints[i].y + prms->buf_cip_ptr[vIdx]->invK.yz;
        point_hom_2d[2] = prms->buf_cip_ptr[vIdx]->invK.zx * normCornerPoints[i].x + prms->buf_cip_ptr[vIdx]->invK.zy * normCornerPoints[i].y + prms->buf_cip_ptr[vIdx]->invK.zz;

        normCornerPoints[i].x = point_hom_2d[0] / point_hom_2d[2];
        normCornerPoints[i].y = point_hom_2d[1] / point_hom_2d[2];

#ifdef PC_VERSION
        printf("%d) normCP.x:%lf, normCP.y:%lf\n ", i, normCornerPoints[i].x, normCornerPoints[i].y);
#else
        #ifdef DEBUG_1
        printf("%d) normCP.x:%lf, normCP.y:%lf\n ", i, normCornerPoints[i].x, normCornerPoints[i].y);
        #endif
#endif

    }

   
    if (peMethod == ESTIMATE_RT_DLT || inCornerPointsStruct->finalCameraCorners[vIdx].numFPDetected  == 4)
    {
        estimateRtBetweenChartAndCamera(sv,prms, vIdx, chartPointsWorld, normCornerPoints, 
        prms->buf_points1norm_ptr, prms->buf_points2norm_ptr, &prms->T1, &prms->T1Inv, &prms->T2, &prms->T2Inv);
    }
    else if (peMethod == ESTIMATE_RT_RANSAC)
    {
        estimateRtBetweenChartAndCamera_ransac(sv,prms, vIdx, chartPointsWorld, normCornerPoints, prms->buf_points1norm_ptr, prms->buf_points2norm_ptr, &prms->T1, &prms->T1Inv, &prms->T2, &prms->T2Inv);
    }
    else
    {
        printf("ERROR: peMethod not set");
        //UTILS_assert(0);
    } 

}
#if LDC_LIB_DATA_TYPE<2 

#endif





/*=======================================================================
*
* Name:        estimateRtBetweenChartAndCamera()
*
* Description: pose estimation based on DLT
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
*
* Effects:
*
*
=======================================================================*/
 void estimateRtBetweenChartAndCamera(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms, int16_t vIdx, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Point2D_f* points1norm, Point2D_f* points2norm, Matrix3D_f * T1, Matrix3D_f * T1Inv, Matrix3D_f * T2, Matrix3D_f * T2Inv)
{
    Point2D_f points1[NUM_CHART_CORNERS], points2[NUM_CHART_CORNERS];
    int16_t i;

    /* step 1: initial estiamte from linear homography estimation (DLT) */
    if (sv->SingleChartPose) {

        int16_t offset;
        uint32_t  indexPts[NUM_CHART_CORNERS];

        if (prms->detectedPointNum[vIdx] == 8)
        {
            for (i = 0; i < prms->detectedPointNum[vIdx]; i++)
                indexPts[i] = i;

            getPtArrFromIndex(pointsCharts, pointsCamera, points1, points2, indexPts, prms->detectedPointNum[vIdx]);
        }
        else
        {
            offset = prms->bLeftChartOnly[vIdx] == 0 ? 4 : 0;
            for (i = 0; i < prms->detectedPointNum[vIdx]; i++)
                indexPts[i] = i + offset;

            getPtArrFromIndex(pointsCharts, pointsCamera, points1, points2, indexPts, prms->detectedPointNum[vIdx]);

            // duplicate last points
            points2[NUM_CHART_CORNERS / 2].x = points2[NUM_CHART_CORNERS / 2 - 1].x;
            points2[NUM_CHART_CORNERS / 2].y = points2[NUM_CHART_CORNERS / 2 - 1].y;
            points1[NUM_CHART_CORNERS / 2].x = points1[NUM_CHART_CORNERS / 2 - 1].x;
            points1[NUM_CHART_CORNERS / 2].y = points1[NUM_CHART_CORNERS / 2 - 1].y;
            prms->detectedPointNum[vIdx] += 1;
        }
    }
    else
    {
        for (i = 0; i < NUM_CHART_CORNERS; i++)
        {
            points1[i] = pointsCharts[i];
            points2[i] = pointsCamera[i];
        }
        prms->detectedPointNum[vIdx] = NUM_CHART_CORNERS; 
        prms->detectedPointNum[vIdx] = A_NROWS / 2;
    }

    // Debug-1 SD
    #ifdef DEBUG_2
    printf ("Printing points1 \n");
    print_point2D_f (points1);
    printf ("Printing points2 \n");
    print_point2D_f (points2);
    #endif

    estimateH_2d_DLT_SRV(sv,prms, points1, points2, points1norm, points2norm, T1, T1Inv, T2, T2Inv, &prms->H, prms->detectedPointNum[vIdx], (uint16_t)(prms->detectedPointNum[vIdx]) * 2, A_NCOLS);
    #ifdef DEBUG_2
    printf("Printing H before scale \n");
    print_matrix3D_f(&prms->H);
    #endif
    scale2UnitMatrix(&prms->H);
    #ifdef DEBUG_2
    printf("Printing H after  scale \n");
    print_matrix3D_f(&prms->H);
    #endif

    extractRandT(sv, prms->H, prms->buf_R_DLT_ptr, prms->buf_t_DLT_ptr);
    /* step 2: Levenberg Marquardt algorithm */
    estimateRt_LM(sv,prms, points1, points2, prms->buf_R_DLT_ptr, prms->buf_t_DLT_ptr, &prms->P, &prms->Pnew, prms->detectedPointNum[vIdx]);

    extractRandT(sv, prms->P, prms->buf_R_DLT_ptr, prms->buf_t_DLT_ptr);

    /* step 3: R_gc, R_cg, t_gc and t_cg */
    prms->buf_R_gc_ptr[vIdx].xx = prms->buf_R_DLT_ptr[0];  prms->buf_R_gc_ptr[vIdx].xy = prms->buf_R_DLT_ptr[1];  prms->buf_R_gc_ptr[vIdx].xz = prms->buf_R_DLT_ptr[2];
    prms->buf_R_gc_ptr[vIdx].yx = prms->buf_R_DLT_ptr[3];  prms->buf_R_gc_ptr[vIdx].yy = prms->buf_R_DLT_ptr[4];  prms->buf_R_gc_ptr[vIdx].yz = prms->buf_R_DLT_ptr[5];
    prms->buf_R_gc_ptr[vIdx].zx = prms->buf_R_DLT_ptr[6];  prms->buf_R_gc_ptr[vIdx].zy = prms->buf_R_DLT_ptr[7];  prms->buf_R_gc_ptr[vIdx].zz = prms->buf_R_DLT_ptr[8];

    prms->buf_t_gc_ptr[vIdx].x  = prms->buf_t_DLT_ptr[0];  prms->buf_t_gc_ptr[vIdx].y  = prms->buf_t_DLT_ptr[1];  prms->buf_t_gc_ptr[vIdx].z  = prms->buf_t_DLT_ptr[2];

    prms->buf_R_cg_ptr[vIdx].xx = prms->buf_R_DLT_ptr[0];  prms->buf_R_cg_ptr[vIdx].xy = prms->buf_R_DLT_ptr[3];  prms->buf_R_cg_ptr[vIdx].xz = prms->buf_R_DLT_ptr[6];
    prms->buf_R_cg_ptr[vIdx].yx = prms->buf_R_DLT_ptr[1];  prms->buf_R_cg_ptr[vIdx].yy = prms->buf_R_DLT_ptr[4];  prms->buf_R_cg_ptr[vIdx].yz = prms->buf_R_DLT_ptr[7];
    prms->buf_R_cg_ptr[vIdx].zx = prms->buf_R_DLT_ptr[2];  prms->buf_R_cg_ptr[vIdx].zy = prms->buf_R_DLT_ptr[5];  prms->buf_R_cg_ptr[vIdx].zz = prms->buf_R_DLT_ptr[8];

    prms->buf_t_cg_ptr[vIdx].x = -(prms->buf_R_DLT_ptr[0] * prms->buf_t_DLT_ptr[0] + prms->buf_R_DLT_ptr[3] * prms->buf_t_DLT_ptr[1] + prms->buf_R_DLT_ptr[6] * prms->buf_t_DLT_ptr[2]);
    prms->buf_t_cg_ptr[vIdx].y = -(prms->buf_R_DLT_ptr[1] * prms->buf_t_DLT_ptr[0] + prms->buf_R_DLT_ptr[4] * prms->buf_t_DLT_ptr[1] + prms->buf_R_DLT_ptr[7] * prms->buf_t_DLT_ptr[2]);
    prms->buf_t_cg_ptr[vIdx].z = -(prms->buf_R_DLT_ptr[2] * prms->buf_t_DLT_ptr[0] + prms->buf_R_DLT_ptr[5] * prms->buf_t_DLT_ptr[1] + prms->buf_R_DLT_ptr[8] * prms->buf_t_DLT_ptr[2]);
}


/*=======================================================================
*
* Name:        estimateRtBetweenChartAndCamera_ransac()
*
* Description: pose estimation based on RANSAC
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
*
* Effects:
*
*
=======================================================================*/

 void estimateRtBetweenChartAndCamera_ransac(svPoseEstimation_t *sv,tivxPoseEstimationParams *prms,  int16_t vIdx, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Point2D_f* points1norm, Point2D_f* points2norm, Matrix3D_f * T1, Matrix3D_f * T1Inv, Matrix3D_f * T2, Matrix3D_f * T2Inv)
{
    int16_t i, k, numIters = 1000;
    int16_t numInliers = 0, minInliers = 0;
    int16_t numPoints = NUM_CHART_CORNERS;
    int16_t numRansacPts = 4;

    uint32_t indRand[NUM_CHART_CORNERS], indPts[NUM_CHART_CORNERS];
    uint32_t inliersInd[NUM_CHART_CORNERS], inliersFinalInd[NUM_CHART_CORNERS];

    Point2D_f points1[NUM_CHART_CORNERS], points2[NUM_CHART_CORNERS];

    Flouble condNum;
    Flouble thresh = 15;
    Flouble sumProjErr;
    Flouble minSumProjErr = 1000;

    Flouble reproErrsTemp[NUM_CHART_CORNERS];

    /* step 1: initial estiamte from linear homography estimation (DLT) with RANSAC*/
    for (i = 0; i < numIters; i++)
    {
        // random number generation
        randomPerm(numPoints, indPts, numRansacPts, indRand);

        // get points array from random indices
        getPtArrFromIndex(pointsCharts, pointsCamera, points1, points2, indRand, numRansacPts);

        // duplicate last points
        points2[numRansacPts].x = points2[numRansacPts - 1].x;
        points2[numRansacPts].y = points2[numRansacPts - 1].y;
        points1[numRansacPts].x = points1[numRansacPts - 1].x;
        points1[numRansacPts].y = points1[numRansacPts - 1].y;

        // DLT
        condNum = estimateH_2d_DLT_SRV(sv,prms, points2, points1, points2norm, points1norm, T1, T1Inv, T2, T2Inv, &prms->Htemp, numRansacPts+1, 2 * (numRansacPts+1), 9);

        if (condNum < 1000)
        {
            scale2UnitMatrix(&prms->Htemp);

            numInliers = 0;
            for (k = 0; k < numPoints; k++)
            {
                reproErrsTemp[k] = computePEGeometricError(pointsCamera[k], pointsCharts[k], prms->Htemp);
                if (reproErrsTemp[k] <= thresh)
                {
                    inliersInd[numInliers] = k;
                    numInliers++;
                }
            }
        }

        sumProjErr = vecSum(reproErrsTemp, numInliers) / numInliers;

        if (numInliers >= minInliers && numInliers >= 4)
        {
           if (sumProjErr <= minSumProjErr)
            {
                minSumProjErr = sumProjErr;

                // get points array from inlier points
                getPtArrFromIndex(pointsCharts, pointsCamera, points1, points2, inliersInd, numInliers);
                // DLT
                estimateH_2d_DLT_SRV(sv, prms,points1, points2, points1norm, points2norm, T1, T1Inv, T2, T2Inv, &prms->Htemp, numInliers, numInliers*2, 9);
                scale2UnitMatrix(&prms->Htemp);

                // update minInliers, final inliers index, H
                prms->H.xx = prms->Htemp.xx; prms->H.xy = prms->Htemp.xy; prms->H.xz = prms->Htemp.xz;
                prms->H.yx = prms->Htemp.yx; prms->H.yy = prms->Htemp.yy; prms->H.yz = prms->Htemp.yz;
                prms->H.zx = prms->Htemp.zx; prms->H.zy = prms->Htemp.zy; prms->H.zz = prms->Htemp.zz;

                for (k = 0; k < numPoints; k++)
                    inliersFinalInd[k] = inliersInd[k];

                minInliers = numInliers;
            }
        }
    }

    extractRandT(sv, prms->H, prms->buf_R_DLT_ptr, prms->buf_t_DLT_ptr);

    /* step 2: Levenberg Marquardt algorithm */
    // get points array from inlier points
    getPtArrFromIndex(pointsCharts, pointsCamera, points1, points2, inliersFinalInd, minInliers);
    // DLT
    estimateRt_LM(sv,prms, points1, points2, prms->buf_R_DLT_ptr, prms->buf_t_DLT_ptr, &prms->P, &prms->Pnew, minInliers);
    extractRandT(sv, prms->P, prms->buf_R_DLT_ptr, prms->buf_t_DLT_ptr);


    /* step 3: R_gc, R_cg, t_gc and t_cg */
    prms->buf_R_gc_ptr[vIdx].xx = prms->buf_R_DLT_ptr[0];  prms->buf_R_gc_ptr[vIdx].xy = prms->buf_R_DLT_ptr[1];  prms->buf_R_gc_ptr[vIdx].xz = prms->buf_R_DLT_ptr[2];
    prms->buf_R_gc_ptr[vIdx].yx = prms->buf_R_DLT_ptr[3];  prms->buf_R_gc_ptr[vIdx].yy = prms->buf_R_DLT_ptr[4];  prms->buf_R_gc_ptr[vIdx].yz = prms->buf_R_DLT_ptr[5];
    prms->buf_R_gc_ptr[vIdx].zx = prms->buf_R_DLT_ptr[6];  prms->buf_R_gc_ptr[vIdx].zy = prms->buf_R_DLT_ptr[7];  prms->buf_R_gc_ptr[vIdx].zz = prms->buf_R_DLT_ptr[8];

    prms->buf_t_gc_ptr[vIdx].x = prms->buf_t_DLT_ptr[0];  prms->buf_t_gc_ptr[vIdx].y = prms->buf_t_DLT_ptr[1];  prms->buf_t_gc_ptr[vIdx].z = prms->buf_t_DLT_ptr[2];

    prms->buf_R_cg_ptr[vIdx].xx = prms->buf_R_DLT_ptr[0];  prms->buf_R_cg_ptr[vIdx].xy = prms->buf_R_DLT_ptr[3];  prms->buf_R_cg_ptr[vIdx].xz = prms->buf_R_DLT_ptr[6];
    prms->buf_R_cg_ptr[vIdx].yx = prms->buf_R_DLT_ptr[1];  prms->buf_R_cg_ptr[vIdx].yy = prms->buf_R_DLT_ptr[4];  prms->buf_R_cg_ptr[vIdx].yz = prms->buf_R_DLT_ptr[7];
    prms->buf_R_cg_ptr[vIdx].zx = prms->buf_R_DLT_ptr[2];  prms->buf_R_cg_ptr[vIdx].zy = prms->buf_R_DLT_ptr[5];  prms->buf_R_cg_ptr[vIdx].zz = prms->buf_R_DLT_ptr[8];

    prms->buf_t_cg_ptr[vIdx].x = -(prms->buf_R_DLT_ptr[0] * prms->buf_t_DLT_ptr[0] + prms->buf_R_DLT_ptr[3] * prms->buf_t_DLT_ptr[1] + prms->buf_R_DLT_ptr[6] * prms->buf_t_DLT_ptr[2]);
    prms->buf_t_cg_ptr[vIdx].y = -(prms->buf_R_DLT_ptr[1] * prms->buf_t_DLT_ptr[0] + prms->buf_R_DLT_ptr[4] * prms->buf_t_DLT_ptr[1] + prms->buf_R_DLT_ptr[7] * prms->buf_t_DLT_ptr[2]);
    prms->buf_t_cg_ptr[vIdx].z = -(prms->buf_R_DLT_ptr[2] * prms->buf_t_DLT_ptr[0] + prms->buf_R_DLT_ptr[5] * prms->buf_t_DLT_ptr[1] + prms->buf_R_DLT_ptr[8] * prms->buf_t_DLT_ptr[2]);
}



 void getPtArrFromIndex(Point2D_f* inPoints1, Point2D_f* inPoints2, Point2D_f* outPoints1, Point2D_f* outPoints2, uint32_t *idxArray, int16_t numPoints)
{
    int16_t k;

    for (k = 0; k < numPoints; k++)
    {
        outPoints1[k].x = inPoints1[idxArray[k]].x;
        outPoints1[k].y = inPoints1[idxArray[k]].y;

        outPoints2[k].x = inPoints2[idxArray[k]].x;
        outPoints2[k].y = inPoints2[idxArray[k]].y;
    }
}



/*=======================================================================
*
* Name:        estimateH_2d_DLT_SRV()
*
* Description: DLT
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
*
* Effects:
*
*
=======================================================================*/
 Flouble estimateH_2d_DLT_SRV(svPoseEstimation_t *sv,tivxPoseEstimationParams *prms, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Point2D_f* points1norm, Point2D_f* points2norm, Matrix3D_f * T1, Matrix3D_f * T1Inv, Matrix3D_f * T2, Matrix3D_f * T2Inv, Matrix3D_f* H, int16_t numPoints, int16_t a_nrows, int16_t a_ncols)
{
    int16_t j, k;
    int16_t a_size = a_nrows * a_ncols;
    int16_t hRows = a_nrows / 2;
    int16_t status;
    Flouble condNum;
    Flouble *A    = prms->buf_A_ptr;
    Flouble *U    = prms->buf_U_ptr;
    Flouble *U1   = prms->buf_U1_ptr;
    Flouble *V    = prms->buf_V_ptr;
    Flouble *Diag = prms->buf_Diag_ptr;
    Flouble *h    = prms->buf_h_ptr;

#if 0
    int16_t pass_cn;
    double max_error_cn,  avg_error_SVD_cn;
    char print_txt[16];
#endif

    // normalize points so that centroid is(0, 0) and average distance is sqrt(2)
    normalizeData_2d(pointsCharts, points1norm, T1, T1Inv, numPoints);
    normalizeData_2d(pointsCamera, points2norm, T2, T2Inv, numPoints);

    // Assemble A
    for (j = 0; j < a_size; j++)
        A[j] = 0;

    for (j = 0; j < hRows; j++)
    {
        A[j * a_ncols + 3] = -points1norm[j].x;
        A[j * a_ncols + 4] = -points1norm[j].y;
        A[j * a_ncols + 5] = -1;

        A[j * a_ncols + 6] = points2norm[j].y * points1norm[j].x;
        A[j * a_ncols + 7] = points2norm[j].y * points1norm[j].y;
        A[j * a_ncols + 8] = points2norm[j].y;
    }

    for (j = hRows, k = 0; j < a_nrows; j++, k++)
    {
        A[j * a_ncols + 0] = points1norm[k].x;
        A[j * a_ncols + 1] = points1norm[k].y;
        A[j * a_ncols + 2] = 1;

        A[j * a_ncols + 6] = -points2norm[k].x * points1norm[k].x;
        A[j * a_ncols + 7] = -points2norm[k].x * points1norm[k].y;
        A[j * a_ncols + 8] = -points2norm[k].x;
    }

    // sovle for H
    status = DSPF_dp_svd_cn(a_nrows, a_ncols, A, U, V, U1, Diag, prms->buf_superDiag_ptr);
    if (status == -1) {
        printf("SV decomposition failed in estimateH_2d_DLT_SRV()!\n");
    }

#if 0
    pass_cn = check_decompostion(A_NROWS, A_NCOLS, sv->A, sv->U, sv->Diag, sv->V, sv->U1, &max_error_cn, &avg_error_SVD_cn, print_txt);
#endif


    //condNum = sv->Diag[0] / sv->Diag[MIN(a_nrows, a_ncols) - 1];
    condNum = Diag[0] / Diag[7];

    for (j = 0; j < a_ncols; j++)
        h[j] = V[j*a_ncols + a_ncols - 1];

    prms->Hnorm.xx = h[0]; prms->Hnorm.xy = h[1]; prms->Hnorm.xz = h[2];
    prms->Hnorm.yx = h[3]; prms->Hnorm.yy = h[4]; prms->Hnorm.yz = h[5];
    prms->Hnorm.zx = h[6]; prms->Hnorm.zy = h[7]; prms->Hnorm.zz = h[8];

    //multiply3(T2Inv, &sv->Hnorm, T1, &sv->H);
    multiply3(T2Inv, &prms->Hnorm, T1, H);

    return condNum;
}


 void normalizeData_2d(Point2D_f *points, Point2D_f* pointsnorm, Matrix3D_f* T, Matrix3D_f* TInv, int16_t numPoints)
{
    int16_t i;
    //int16_t numPoints = NUM_CHART_CORNERS;
    Flouble meanX = 0, meanY = 0, sum = 0, scale;

    for (i = 0; i < numPoints; i++)
    {
        meanX += points[i].x;
        meanY += points[i].y;
    }

    meanX /= numPoints;
    meanY /= numPoints;


    for (i = 0; i < numPoints; i++)
    {
        pointsnorm[i].x = points[i].x - meanX;
        pointsnorm[i].y = points[i].y - meanY;
        sum += SQRT(pointsnorm[i].x *pointsnorm[i].x + pointsnorm[i].y*pointsnorm[i].y);
    }


    scale = SQRT(2) * numPoints / sum;

    for (i = 0; i < numPoints; i++)
    {
        pointsnorm[i].x *= scale;
        pointsnorm[i].y *= scale;
    }

    T->xx = scale;       T->xy = 0;          T->xz = -(scale)*meanX;
    T->yx = 0;           T->yy = scale;      T->yz = -(scale)*meanY;
    T->zx = 0;           T->zy = 0;          T->zz = 1;

    TInv->xx = 1/scale;  TInv->xy = 0;       TInv->xz = meanX;
    TInv->yx = 0;        TInv->yy = 1/scale; TInv->yz = meanY;
    TInv->zx = 0;        TInv->zy = 0;       TInv->zz = 1;

}



/*=======================================================================
*
* Name:        extractRandT()
*
* Description: extract R and T
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
*
* Effects:
*
*
=======================================================================*/
 void extractRandT(svPoseEstimation_t *sv, Matrix3D_f M, Flouble* R, Flouble* t)
{
    int32_t i;


    Flouble C1[3], C2[3], C3[3];
    Flouble tInv[3];


    for (i = 0; i < 3; i++)
    {
        // get first 2 columns vector from M
        C1[0] = M.xx; C1[1] = M.yx; C1[2] = M.zx;
        C2[0] = M.xy; C2[1] = M.yy; C2[2] = M.zy;

        // cross product
        crossProduct(C1, C2, C3);

        // extract R
        R[0] = C1[0]; R[1] = C2[0]; R[2] = C3[0];
        R[3] = C1[1]; R[4] = C2[1]; R[5] = C3[1];
        R[6] = C1[2]; R[7] = C2[2]; R[8] = C3[2];

        // extract T
        t[0] = M.xz;  t[1] = M.yz;  t[2] = M.zz;

        // -R' * t
        tInv[2] = -(R[2] * t[0] + R[5] * t[1] + R[8] * t[2]);
        if (tInv[2] <= 0)
            changesign_mat(&M);
        else
            break;
    }

    if (i == 3)
        printf("Warning: Camera height is negative!\n");

    // svd


    // even though status != 1, returned values could be garbage. So comment out the below codes
    /*
    status = DSPF_dp_svd_cn(3, 3, R, sv->U, sv->V, sv->U1, sv->Diag, sv->superDiag);
    if (status == -1) {
        Vps_printf("SV decomposition failed in extractRandT()!\n");
    } else
    {
        // do-kwon: apply only when SVD is successful

        // transpose of V, output in U1
        transposeMatrix(3, 3, sv->V, sv->U1);
        // multiply U and U1 and output in R_DLT
        multiplyMatrix(3, 3, 3, sv->U, sv->U1, R);
    }
    */

}



/*=======================================================================
*
* Name:        estimateRt_LM()
*
* Description: Levenberg - Marquardt algorithm to update pose
*
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
*
* Effects:
*
*
=======================================================================*/
 void estimateRt_LM(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms, Point2D_f *pointsCharts, Point2D_f *pointsCamera, Flouble* R, Flouble* t, Matrix3D_f* P, Matrix3D_f* Pnew, int16_t numPoints)
{
    int16_t i;
    int16_t numIters = 8;
    //int16_t numPoints = NUM_CHART_CORNERS;
    Flouble cost, costnew, gamma = 0;

    Flouble lambda[3] = {7.5, 7.5, 0.75};

    // initialize P from R and t
    P->xx = R[0]; P->xy = R[1]; P->xz = t[0];
    P->yx = R[3]; P->yy = R[4]; P->yz = t[1];
    P->zx = R[6]; P->zy = R[7]; P->zz = t[2];

    // get X
    getX(sv, pointsCamera, prms->buf_xvec_ptr, lambda, numPoints);

    // compute Jacobian
    computeJ(sv, pointsCharts, P, lambda, prms->buf_Jacob_ptr, prms->buf_JacobT_ptr, 19, 9, numPoints);

    transposeMatrix(19, 9, prms->buf_Jacob_ptr, prms->buf_JacobT_ptr);
    multiplyMatrix(9, 19, 9, prms->buf_JacobT_ptr, prms->buf_Jacob_ptr, prms->buf_A_ptr);  // reuse sv->A to store output
    getDiagonal(9, 9, prms->buf_A_ptr, prms->buf_Diag_ptr);                   // reuse sv->Diag

    for (i = 0; i < 9; i++)
        gamma += prms->buf_Diag_ptr[i];
    gamma /= 9;
    gamma *= 0.001;


    // compute cost
    cost = computeCost(sv, pointsCharts, P, prms->buf_xvec_ptr, prms->buf_fvec_ptr, lambda, numPoints);

    // iterate
    for (i = 0; i < numIters; i++)
    {
        computeJ(sv, pointsCharts, P, lambda, prms->buf_Jacob_ptr, prms->buf_JacobT_ptr, 19, 9, numPoints);
        transposeMatrix(19, 9, prms->buf_Jacob_ptr, prms->buf_JacobT_ptr);
        #ifdef DEBUG_2
        if (i==0) {
          printf("#### Printing after transposeMatrix ##### jacob[0] = %f jacob[20]= %f  jacob[50]= %f \n",prms->buf_Jacob_ptr[0],prms->buf_Jacob_ptr[20],prms->buf_Jacob_ptr[50]);
          printf("#### Printing after transposeMatrix ##### jacobT[0] = %f jacobT[50]= %f  jacobT[51]= %f \n",prms->buf_JacobT_ptr[0],prms->buf_JacobT_ptr[50],prms->buf_JacobT_ptr[51]);
         }
        #endif

        // gamma * I(9,9)
        initMatrix(9, 9, prms->buf_gammaIdentity_ptr, 1);
        muliplyScalerMatrix(9, 9, gamma, prms->buf_gammaIdentity_ptr, prms->buf_gammaIdentity_ptr);

        // A = J' * J + gamma * I(9, 9)
        multiplyMatrix(9, 19, 9, prms->buf_JacobT_ptr, prms->buf_Jacob_ptr, prms->buf_A_ptr);  // reuse sv->A to store output

        addMatrix(9, 9, prms->buf_A_ptr, prms->buf_gammaIdentity_ptr, prms->buf_A_ptr);


        // bvec = -J' * (fvec - xvec)
        computeF(sv, pointsCharts, P, prms->buf_fvec_ptr, lambda, numPoints);

        subtractMatrix(19, 1, prms->buf_fvec_ptr, prms->buf_xvec_ptr, prms->buf_tempvec_ptr);

        muliplyScalerMatrix(9, 19, -1, prms->buf_JacobT_ptr, prms->buf_JacobT_ptr);

        multiplyMatrix(9, 19, 1, prms->buf_JacobT_ptr, prms->buf_tempvec_ptr, prms->buf_bvec_ptr);


        // compute delta
        solveDelta(9, 9, prms->buf_A_ptr, prms->buf_Q_ptr, prms->buf_R_ptr, prms->buf_deltavec_ptr, prms->buf_bvec_ptr, prms->buf_yvec_ptr);

        // compute Pnew
        // TODO print Pnew, pointsCharts, buf_xvec_ptr, buf_tempvec_ptr, lambda & numbPoints
        Pnew->xx = P->xx + prms->buf_deltavec_ptr[0]; Pnew->xy = P->xy + prms->buf_deltavec_ptr[3]; Pnew->xz = P->xz + prms->buf_deltavec_ptr[6];
        Pnew->yx = P->yx + prms->buf_deltavec_ptr[1]; Pnew->yy = P->yy + prms->buf_deltavec_ptr[4]; Pnew->yz = P->yz + prms->buf_deltavec_ptr[7];
        Pnew->zx = P->zx + prms->buf_deltavec_ptr[2]; Pnew->zy = P->zy + prms->buf_deltavec_ptr[5]; Pnew->zz = P->zz + prms->buf_deltavec_ptr[8];

        #ifdef DEBUG_2
        if (i==0) {
        printf("P->xx = %f \n",P->xx);
        printf("buf_deltavec_ptr = %f \n",prms->buf_deltavec_ptr[0]);
        printf("Printing Pnew \n");
        print_matrix3D_f (Pnew);
        //printf("Printing PointsCharts \n");
        print_point2D_f (pointsCharts);
        printf("lambda values are %f %f %f \n",lambda[0],lambda[1],lambda[2]);
        printf("numpoints = %d \n",numPoints);
        printf("printing fvec and xvec \n");
          
        /*
        printf("\nfvec = " );
        for (j=0;j<19;j++){
          printf(" %f ",prms->buf_fvec_ptr[j] );
        }
        printf("\nxvec = " );
        for (j=0;j<19;j++){
          printf("%f ",prms->buf_xvec_ptr[j] );
        }
        */
        }

        #endif
        // compute cost
        costnew = computeCost(sv, pointsCharts, Pnew, prms->buf_xvec_ptr, prms->buf_fvec_ptr, lambda, numPoints);

        if (costnew < cost)
        {
            // set Pnew to P;
            P->xx = Pnew->xx; P->xy = Pnew->xy; P->xz = Pnew->xz;
            P->yx = Pnew->yx; P->yy = Pnew->yy; P->yz = Pnew->yz;
            P->zx = Pnew->zx; P->zy = Pnew->zy; P->zz = Pnew->zz;

            cost = costnew;
            gamma = gamma * 0.1;
        }
        else
        {
            gamma = gamma * 10;
        }

    }
}



/******************** For Levenberg-Marquardt *********************/

 void getX(svPoseEstimation_t *sv, Point2D_f* points, Flouble* xvec, Flouble* lambda, int16_t numPoints)
{
    int16_t i;

    for (i = 0; i < numPoints * 2; i++)
    {
        xvec[i * 2 + 0] = points[i].x;
        xvec[i * 2 + 1] = points[i].y;
    }

    xvec[2 * numPoints + 0] = lambda[0];
    xvec[2 * numPoints + 1] = lambda[1];
    xvec[2 * numPoints + 2] = 0;
}


 void computeJ(svPoseEstimation_t *sv, Point2D_f* points, Matrix3D_f* P, Flouble* lambda, Flouble* J, Flouble* JT, int16_t Nrows, int16_t Ncols, int16_t numPoints)
{
    int16_t i;
    int16_t JSize = Nrows * Ncols;
    Flouble oneOverDen, gx, gy;

    // init Jacobian
    for (i = 0; i < JSize; i++) {
            J[i] = 0;
    }

    for (i = 0; i < numPoints; i++)
    {
        oneOverDen = 1 / (P->zx * points[i].x + P->zy * points[i].y + P->zz);

        // x components
        gx = oneOverDen * (P->xx * points[i].x + P->xy * points[i].y + P->xz);

        J[(i * 2) * Ncols + 0] = oneOverDen * points[i].x;           // r11
        J[(i * 2) * Ncols + 1] = 0;                                  // r21
        J[(i * 2) * Ncols + 2] = -oneOverDen * gx * points[i].x;     // r31
        J[(i * 2) * Ncols + 3] = oneOverDen * points[i].y;           // r12
        J[(i * 2) * Ncols + 4] = 0;                                  // r22
        J[(i * 2) * Ncols + 5] = -oneOverDen * gx * points[i].y;     // r32
        J[(i * 2) * Ncols + 6] = oneOverDen;                         // t1
        J[(i * 2) * Ncols + 7] = 0;                                  // t2
        J[(i * 2) * Ncols + 8] = -oneOverDen * gx;                   // t3

        // y components
        gy = oneOverDen * (P->yx * points[i].x + P->yy * points[i].y + P->yz);

        J[(i * 2 + 1) * Ncols + 0] = 0;                              // r11
        J[(i * 2 + 1) * Ncols + 1] = oneOverDen * points[i].x;       // r21
        J[(i * 2 + 1) * Ncols + 2] = -oneOverDen * gy * points[i].x; // r31
        J[(i * 2 + 1) * Ncols + 3] = 0;                              // r12
        J[(i * 2 + 1) * Ncols + 4] = oneOverDen * points[i].y;       // r22
        J[(i * 2 + 1) * Ncols + 5] = -oneOverDen * gy * points[i].y; // r32
        J[(i * 2 + 1) * Ncols + 6] = 0;                              // t1
        J[(i * 2 + 1) * Ncols + 7] = oneOverDen;                     // t2
        J[(i * 2 + 1) * Ncols + 8] = -oneOverDen * gy;               // t3
    }

    // constraints
    // lambda1
    J[(numPoints * 2 + 0) * Ncols + 0] = 2 * lambda[0] * P->xx;      // r11
    J[(numPoints * 2 + 0) * Ncols + 1] = 2 * lambda[0] * P->yx;      // r21
    J[(numPoints * 2 + 0) * Ncols + 2] = 2 * lambda[0] * P->zx;      // r31
    J[(numPoints * 2 + 0) * Ncols + 3] = 0;                          // r12
    J[(numPoints * 2 + 0) * Ncols + 4] = 0;                          // r22
    J[(numPoints * 2 + 0) * Ncols + 5] = 0;                          // r32
    J[(numPoints * 2 + 0) * Ncols + 6] = 0;                          // t1
    J[(numPoints * 2 + 0) * Ncols + 7] = 0;                          // t2
    J[(numPoints * 2 + 0) * Ncols + 8] = 0;                          // t3

    // lambda2
    J[(numPoints * 2 + 1) * Ncols + 0] = 0;                          // r11
    J[(numPoints * 2 + 1) * Ncols + 1] = 0;                          // r21
    J[(numPoints * 2 + 1) * Ncols + 2] = 0;                          // r31
    J[(numPoints * 2 + 1) * Ncols + 3] = 2 * lambda[1] * P->xy;      // r12
    J[(numPoints * 2 + 1) * Ncols + 4] = 2 * lambda[1] * P->yy;      // r22
    J[(numPoints * 2 + 1) * Ncols + 5] = 2 * lambda[1] * P->zy;      // r32
    J[(numPoints * 2 + 1) * Ncols + 6] = 0;                          // t1
    J[(numPoints * 2 + 1) * Ncols + 7] = 0;                          // t2
    J[(numPoints * 2 + 1) * Ncols + 8] = 0;                          // t3

    // lambda3
    J[(numPoints * 2 + 2) * Ncols + 0] = lambda[2] * P->xy;          // r11
    J[(numPoints * 2 + 2) * Ncols + 1] = lambda[2] * P->yy;          // r21
    J[(numPoints * 2 + 2) * Ncols + 2] = lambda[2] * P->zy;          // r31
    J[(numPoints * 2 + 2) * Ncols + 3] = lambda[2] * P->xx;          // r12
    J[(numPoints * 2 + 2) * Ncols + 4] = lambda[2] * P->yx;          // r22
    J[(numPoints * 2 + 2) * Ncols + 5] = lambda[2] * P->zx;          // r32
    J[(numPoints * 2 + 2) * Ncols + 6] = 0;                          // t1
    J[(numPoints * 2 + 2) * Ncols + 7] = 0;                          // t2
    J[(numPoints * 2 + 2) * Ncols + 8] = 0;                          // t3

#ifdef PC_VERSION
    //printMatrix(Nrows, Ncols, J);
#endif

}

 Flouble computeCost(svPoseEstimation_t *sv, Point2D_f* points, Matrix3D_f* P, Flouble* xvec, Flouble* fvec, Flouble* lambda, int16_t numPoints)
{
    int16_t i, vlen = numPoints * 2 + 3;
    Flouble cost;

    computeF(sv, points, P, fvec, lambda, numPoints);

    for (i = 0; i < vlen; i++)
        fvec[i] -= xvec[i];

    cost = vecNorm(fvec, vlen, 0);
    cost *= 0.5;

    return cost;
}


 void computeF(svPoseEstimation_t *sv, Point2D_f* points, Matrix3D_f* P, Flouble* fvec, Flouble* lambda, int16_t numPoints)
{
    int16_t i, vlen = numPoints * 2 + 3;
    Flouble oneOverDen;

    for (i = 0; i < vlen; i++)
        fvec[i] = 0;

    for (i = 0; i < numPoints; i++)
    {
        oneOverDen = 1 / (P->zx * points[i].x + P->zy * points[i].y + P->zz);
        fvec[2 * i] = oneOverDen * (P->xx * points[i].x + P->xy * points[i].y + P->xz);
        fvec[2 * i + 1] = oneOverDen * (P->yx * points[i].x + P->yy * points[i].y + P->yz);
    }

    fvec[2 * numPoints + 0] = lambda[0] * (P->xx * P->xx + P->yx * P->yx + P->zx *  P->zx);
    fvec[2 * numPoints + 1] = lambda[1] * (P->xy * P->xy + P->yy * P->yy + P->zy *  P->zy);
    fvec[2 * numPoints + 2] = lambda[2] * (P->xx * P->xy + P->yx * P->yy + P->zx *  P->zy);
}



/////////////////////////////////////////////////
// matrix utils - could be in a separted file  //
/////////////////////////////////////////////////
 Flouble vecNorm(Flouble * vector, int16_t length, uint8_t bSqrt)
{
    int16_t i;
    Flouble norm = 0;

    for (i = 0; i < length; i++)
    {
        norm += (vector[i] * vector[i]);
    }

    if (bSqrt)
    {
        norm = SQRT(norm);
    }

    return norm;
}


 Flouble vecSum(Flouble * vector, int16_t length)
{
    int16_t i;
    Flouble sum = 0;

    for (i = 0; i < length; i++)
    {
        sum += vector[i];
    }

    return sum;
}



 void crossProduct(Flouble *v1, Flouble* v2, Flouble* vout)
{
    vout[0] = -v1[2] * v2[1] + v1[1] * v2[2];
    vout[1] =  v1[2] * v2[0] - v1[0] * v2[2];
    vout[2] = -v1[1] * v2[0] + v1[0] * v2[1];
}

// Nrows: # of rows of input
// Ncols: # of cols of input
 void transposeMatrix(int16_t Nrows, int16_t Ncols, Flouble *inM, Flouble *outM)
{
    int16_t i, j;

    for (j = 0; j < Ncols; j++)
        for (i = 0; i < Nrows; i++)
            outM[j * Nrows + i] = inM[i* Ncols + j];

}

// Nrows: # of rows of inM1
// Ncols: # of cols of inM1, #of rows of inM2
// Ncols2: # of cols of inM2
 void multiplyMatrix(int16_t Nrows, int16_t Ncols, int16_t Ncols2, Flouble *inM1, Flouble *inM2, Flouble *outM)
{
    int16_t i, j, k;

    // init
    for (j = 0; j < Nrows; j++)
        for (i = 0; i < Ncols2; i++)
            outM[j * Ncols2 + i] = 0;

    for (j = 0; j < Nrows; j++)
        for (i = 0; i < Ncols2; i++)
            for (k = 0; k < Ncols; k++)
                outM[j * Ncols2 + i] += inM1[j * Ncols + k] * inM2[k * Ncols2 + i];

}

// Nrows: # of rows of inM1
// Ncols: # of cols of inM1
 void addMatrix(int16_t Nrows, int16_t Ncols, Flouble *inM1, Flouble *inM2, Flouble *outM)
{
    int16_t i, j;

    for (j = 0; j < Nrows; j++)
        for (i = 0; i < Ncols; i++)
            outM[j * Ncols + i] = inM1[j * Ncols + i] + inM2[j * Ncols + i];
}


// Nrows: # of rows of inM1
// Ncols: # of cols of inM1
 void subtractMatrix(int16_t Nrows, int16_t Ncols, Flouble *inM1, Flouble *inM2, Flouble *outM)
{
    int16_t i, j;

    for (j = 0; j < Nrows; j++)
        for (i = 0; i < Ncols; i++)
            outM[j * Ncols + i] = inM1[j * Ncols + i] - inM2[j * Ncols + i];
}

 void initMatrix(int16_t Nrows, int16_t Ncols, Flouble* outM, uint8_t identity)
{
    int16_t i, j;

    for (j = 0; j < Nrows; j++)
        for (i = 0; i < Ncols; i++)
            outM[j * Ncols + i] = 0;

    if (identity)
    {
        int16_t length = MIN(Nrows, Ncols);
        for (i = 0; i < length; i++)
            outM[i * Ncols + i] = 1;
    }
}


 void muliplyScalerMatrix(int16_t Nrows, int16_t Ncols, Flouble scaler, Flouble* inM, Flouble* outM)
{
    int16_t i, j;

    for (j = 0; j < Nrows; j++)
        for (i = 0; i < Ncols; i++)
            outM[j * Ncols + i] = scaler * inM[j * Ncols + i];
}


 void getDiagonal(int16_t Nrows, int16_t Ncols, Flouble* inM, Flouble* diag)
{
    int16_t i, length = MIN(Nrows, Ncols);

    for (i = 0; i < length; i++)
        diag[i] = inM[i * Ncols + i];
}




 void scale2UnitMatrix(Matrix3D_f* M)
{
    int16_t sign;
    Flouble vec1[3], vec2[3];
    Flouble norm1, norm2, norm;

    // scale such that columns 1 and 2 are close to unit and H(3,3) is positive
    vec1[0] = M->xx; vec1[1] = M->yx; vec1[2] = M->zx;
    vec2[0] = M->xy; vec2[1] = M->yy; vec2[2] = M->zy;

    sign = SIGNVAR(M->zz);
    norm1 = vecNorm(vec1, 3, 1);
    norm2 = vecNorm(vec2, 3, 1);
    norm = norm1 + norm2;

    M->xx = sign * 2 * M->xx / norm; M->xy = sign * 2 * M->xy / norm;  M->xz = sign * 2 * M->xz / norm;
    M->yx = sign * 2 * M->yx / norm; M->yy = sign * 2 * M->yy / norm;  M->yz = sign * 2 * M->yz / norm;
    M->zx = sign * 2 * M->zx / norm; M->zy = sign * 2 * M->zy / norm;  M->zz = sign * 2 * M->zz / norm;

}


 void solveDelta(int16_t Nrows, int16_t Ncols, Flouble* A, Flouble* Q, Flouble* R, Flouble *x, Flouble *b, Flouble* u)
{
    int16_t row, col, k;
    int16_t status, pass, invertible;
    Flouble sum, error, tolerance = 0.000001;

    // decompose A into Q and R, where A=Q*R
    status = DSPF_dp_qrd_cn(Nrows, Ncols, A, Q, R, u);
    if (status == -1) {
        printf("QR decomposition failed!\n");
    }


    pass = 1;
    // check decompositon Q*R = A
    for (row = 0; row<Nrows; row++) {
        for (col = 0; col<Ncols; col++) {
            sum = 0;
            for (k = 0; k<Nrows; k++) {
                sum += Q[k + row*Nrows] * R[col + k*Ncols];
            }
            error = fabs(A[col + row*Ncols] - sum);
            if (error>tolerance) {
                pass = 0;
                printf("nat decomposition error=%e, %d\n", error, pass);
            }
        }
    }

    // check transformation: Q*Q'=I
    for (row = 0; row<Nrows; row++) {
        for (col = 0; col<Nrows; col++) {
            sum = 0;
            for (k = 0; k<Nrows; k++) {
                sum += Q[k + row*Nrows] * Q[k + col*Nrows];
            }
            if (row == col) {
                error = fabs(sum - 1.0);
            }
            else {
                error = fabs(sum);
            }
            if (error>tolerance) {
                pass = 0;
            }
        }
    }

    // check if A is invertible by looking for zeros on R diagonal
    if (Nrows == Ncols) {
        invertible = 1;
        for (row = 0; row<Nrows; row++) {
            if (fabs(R[row + row*Ncols])<tolerance) {
                invertible = 0;
            }
        }
    }
    else {
        invertible = 0;
    }

    if (invertible == 0)
        printf("A is not invertible to solve Ax = b!\n");


    // solve Ax= b
    if (invertible) {
        status = DSPF_dp_qrd_solver_cn(Nrows, Ncols, Q, R, b, u, x);
        if (status == -1) {
            printf("Ax = b solver failed!\n");
        }
    }

}







static unsigned long next = 1;
/* RAND_MAX assumed to be 32767 */
static uint32_t myrandPE(void) {
    next = next * 1103515245 + 12345;
    return((uint32_t)(next / 65536) % 32768);
}


 void randomPerm(int32_t max_index, uint32_t *array_in, int32_t count, uint32_t *array_out)
{
    int32_t i;
    uint32_t  randNo, temp;
    int random;
    for (i = 0; i < max_index; i++)
        array_in[i] = i;
    for (i = max_index - 1; i > max_index - count - 1; i--){
        random = myrandPE();
        randNo = (uint32_t)random  % (i+1);
        array_out[max_index - i - 1] = array_in[randNo];
        //swap indices[rand] with indices[i]
        temp = array_in[randNo];
        array_in[randNo] = array_in[i];
        array_in[i] = temp;
    }
}


 Flouble computePEGeometricError(Point2D_f points1, Point2D_f points2, Matrix3D_f M)
{
    Flouble error = 0;

    Point3D_f h;

    // M * [points1.x points2.y 1]'
    h.x = M.xx * points1.x + M.xy * points1.y + M.xz;
    h.y = M.yx * points1.x + M.yy * points1.y + M.yz;
    h.z = M.zx * points1.x + M.zy * points1.y + M.zz;

    // h = h / h(3)
    h.x /= h.z;
    h.y /= h.z;
    h.z /= h.z;

    error = (h.x - points2.x) * (h.x - points2.x) + (h.y - points2.y) * (h.y - points2.y);
    error = SQRT(error);

    return error;
}

/*===============================================================================
*
* Name:        constructCalmat()
*
* Description: generate calmat from R and T
*
* Input:
*   sv:        Handle for the SV structure
*
* Returns:
*
* Effects:
*
===============================================================================*/
 void constructCalmat(svPoseEstimation_t *sv, tivxPoseEstimationParams *prms,int32_t* outcalmat)
{
    int32_t idx, vIdx;

    double R_mfactor = pow(2, CALMAT_R_SHIFT);
    double T_mfactor = pow(2, CALMAT_T_SHIFT);

    for (vIdx = 0; vIdx < sv->numCameras; vIdx++)
    {
        idx = vIdx * 12;

        outcalmat[idx]      = (int32_t)(prms->buf_R_gc_ptr[vIdx].xx * R_mfactor);
        outcalmat[idx + 1]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].yx * R_mfactor);
        outcalmat[idx + 2]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].zx * R_mfactor);
        outcalmat[idx + 3]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].xy * R_mfactor);
        outcalmat[idx + 4]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].yy * R_mfactor);
        outcalmat[idx + 5]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].zy * R_mfactor);
        outcalmat[idx + 6]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].xz * R_mfactor);
        outcalmat[idx + 7]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].yz * R_mfactor);
        outcalmat[idx + 8]  = (int32_t)(prms->buf_R_gc_ptr[vIdx].zz * R_mfactor);
        outcalmat[idx + 9]  = (int32_t)(prms->buf_t_gc_ptr[vIdx].x * T_mfactor);
        outcalmat[idx + 10] = (int32_t)(prms->buf_t_gc_ptr[vIdx].y * T_mfactor);
        outcalmat[idx + 11] = (int32_t)(prms->buf_t_gc_ptr[vIdx].z * T_mfactor);
            
#ifdef PC_VERSION

        printf("%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ",
         prms->buf_R_gc_ptr[vIdx].xx, prms->buf_R_gc_ptr[vIdx].yx, prms->buf_R_gc_ptr[vIdx].zx, 
         prms->buf_R_gc_ptr[vIdx].xy, prms->buf_R_gc_ptr[vIdx].yy, prms->buf_R_gc_ptr[vIdx].zy,
         prms->buf_R_gc_ptr[vIdx].xz, prms->buf_R_gc_ptr[vIdx].yz, prms->buf_R_gc_ptr[vIdx].zz,
         prms->buf_t_gc_ptr[vIdx].x, prms->buf_t_gc_ptr[vIdx].y, prms->buf_t_gc_ptr[vIdx].z);
#endif
    }


#ifdef PC_VERSION
    printf("\n");
#endif

}

#ifdef DEBUG_2
 void print_point2D_f (Point2D_f pt[NUM_CHART_CORNERS]) {
    int i;
    for (i=0;i<NUM_CHART_CORNERS;i++) {
      printf("i= %d pt.x = %f  pt.y = %f \n",i,pt[i].x,pt[i].y);
    }
}


 void print_matrix3D_f (Matrix3D_f *H_pt) {
      printf("xx = %f xy = %f xz=%f \n",H_pt->xx,H_pt->xy,H_pt->xz);
      printf("yx = %f yy = %f yz=%f \n",H_pt->yx,H_pt->yy,H_pt->yz);
      printf("zx = %f zy = %f zz=%f \n",H_pt->zx,H_pt->zy,H_pt->zz);
}
#endif
