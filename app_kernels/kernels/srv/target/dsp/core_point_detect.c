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
#include "TI/tivx_srv.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"
#include "core_point_detect.h"
#include <math.h>
#include <stdio.h>
#include <float.h>

#define MAX_FP_ALL                   80
#define MAX_INPUT_CAMERAS            4
#define FP_TO_DETECT                 2      


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


//#define DEBUG_1
//#define DEBUG_2
//#define DEBUG_3



static inline double sqrtdp_i(double a)
{
    double  half = 0.5;
    double  OneP5 = 1.5;
    double  x, y;
    int32_t     i;

    x = _rsqrdp(a);

#pragma UNROLL(1)        //PRAGMA: do not unroll this loop 
    for (i = 0; i< 3; i++) {
        x = x * (OneP5 - (a*x*x*half));
    }
    y = a * x;

    if (a <= 0.0) {
        y = 0.0;
    }
    if (a > DBL_MAX) {
        y = DBL_MAX;
    }

    return (y);
} 



/*===============================================================================
 *
 * Name:        svGetFinderPatterns()
 *
 * Description: Find find patterns from a fisheye input
 *
 *
 * Input:
 *   sv:        Handle for the SV structure
 *   prms:      Scratch Buffers
 *   in_image   Input Luma image
 *
 *
 * Returns:
 *              1 if finder pattern are detected, 0 otherwise
 * Effects:
 *
 ===============================================================================*/
 uint16_t svGetFinderPatterns(svPointDetect_t *sv, svLdcLut_t *ldcLut, tivxPointDetectParams *prms,  uint8_t *in_image, uint8_t *out_bw_image,svACDetectStructFinalCorner_t *finalCornerSet )
{
    int32_t numCandPts;
    int16_t numFPs;
    int16_t numFPDetected;


    svBinarizeImage(sv, prms, in_image, out_bw_image);

    numCandPts = svSearchFinderPatterns(sv, prms);
    

    numFPs = svFindFPCentroids(sv,prms, numCandPts);
    #ifdef DEBUG_3
       printf("##### TEST POINT : svFindFPCentroids ###### \n");
       printf("numFPs = %d \n",numFPs);
       printf("Centroids coordinates =  %d %d %d %d \n",
               prms->buf_intOutCenter_ptr[0],prms->buf_intOutCenter_ptr[1],
               prms->buf_intOutCenter_ptr[2],prms->buf_intOutCenter_ptr[3]);
    #endif
    
    if (numFPs > MAX_FP_ALL)
        return 0;

    numFPDetected = svRemoveNoiseFP(sv, ldcLut, prms,  numFPs, finalCornerSet);
    finalCornerSet->numFPDetected = numFPDetected *4; /* Expose numFPDetectec to the API */

    if (numFPDetected != FP_TO_DETECT)
        return 0;
    /* Failure condition if both squares are not detected */
    #ifdef DEBUG_3
    printf("##### TEST POINT : svRemoveNoiseFP ###### \n");
    printf("numFPDetected = %d \n",numFPDetected);
    #endif

    return 1;


}


/*===============================================================================
 *
 * Name:        svBinarizeImage()
 *
 * Description: Binarize input picture based on local mean
 *
 *
 * Input:
 *   sv:        Handle for the SV structure
 *   view_number: view id
 *
 * Returns:
 *
 * Effects:
 *
 ===============================================================================*/
void svBinarizeImage(svPointDetect_t* sv, tivxPointDetectParams *prms, uint8_t *in_image, uint8_t *bwLumaFrame)
{
    double bias;
    int16_t  SVInCamFrmHeight;
    int16_t  SVInCamFrmWidth;
    int16_t  SVInPitch[2];

    SVInCamFrmHeight = prms->img_prms.dim_y;
    SVInCamFrmWidth  = prms->img_prms.dim_x;
    SVInPitch[0]     = prms->img_prms.stride_y;

    if (sv->thresholdMode == 0)
        bias = 1.1;
    else if (sv->thresholdMode == 1)
        bias = 0.9;
    else if (sv->thresholdMode == 2)
        bias = 1.0;
    else if (sv->thresholdMode == 3)
        bias = 1.15;
    else if (sv->thresholdMode == 4)
        bias = 1.25;
    else
        bias = 1.1;

    int16_t ofst = sv->binarizeOffset;
    if (sv->windowMode == 1)
        ofst = BINARIZE_OFFSET_SMALL_WINDOW;

    int16_t i, j;
    int32_t blkSize;
    double avg;


    if(SVInCamFrmHeight >= 960)
    {
        /* For 2MP resolution */
        ofst += 10;
    }

    blkSize = (ofst+ofst + 1)*(ofst+ofst + 1);


    // Integral Image
    VXLIB_bufParams2D_t src_addr, dst_addr;
    src_addr.data_type = VXLIB_UINT8;
    src_addr.dim_x = SVInCamFrmWidth;
    src_addr.dim_y = SVInCamFrmHeight;
    src_addr.stride_y = SVInPitch[0] * sizeof(uint8_t);
    dst_addr.data_type = VXLIB_UINT32;
    dst_addr.dim_x = SVInCamFrmWidth;
    dst_addr.dim_y = SVInCamFrmHeight;
    dst_addr.stride_y = SVInPitch[0] * sizeof(uint32_t);

    uint32_t *IntegralImg = prms->buf_IntegralImg_ptr;
    uint32_t *rows = prms->buf_IntegralRows_ptr;
    uint8_t  *buf_bwLumaFrame = prms->buf_bwLumaFrame_ptr;
    uint8_t  *buf_grayLumaFrame = prms->buf_grayLumaFrame_ptr;



    memset(rows, 0, SVInCamFrmWidth * sizeof(unsigned int));
    VXLIB_integralImage_i8u_o32u(in_image, &src_addr, IntegralImg, &dst_addr, rows, NULL, 0);

    for (j = 0; j < SVInCamFrmHeight; j++) {
        for (i = 0; i < SVInCamFrmWidth; i++) {

            // Edge Case
            /* Primary I/O bwLumaFrame is only for debug, in the final implmentation */
            /* only the buf_bwLumaFrame intermediate buffer will be used             */
            if (j <= ofst || j >= SVInCamFrmHeight - ofst || i <= ofst || i >= SVInCamFrmWidth - ofst) {
                if (in_image[j * SVInPitch[0] + i] > 150) {
                    bwLumaFrame[j * SVInPitch[0] + i] = 255;
                    buf_bwLumaFrame[j * SVInPitch[0] + i] = 255;
                }
                else {
                    bwLumaFrame[j * SVInPitch[0] + i] = 0;
                    buf_bwLumaFrame[j * SVInPitch[0] + i] = 0;
                }
            }
            else { // Adaptive Theshold
                avg = IntegralImg[(j + ofst) * SVInPitch[0] + (i + ofst)];
                avg -= IntegralImg[(j - ofst - 1) * SVInPitch[0] + (i + ofst)];
                avg -= IntegralImg[(j + ofst) * SVInPitch[0] + (i - ofst - 1)];
                avg += IntegralImg[(j - ofst - 1) * SVInPitch[0] + (i - ofst - 1)];
                if (in_image[j * SVInPitch[0] + i] <= avg / blkSize * bias) {
                    bwLumaFrame[j * SVInPitch[0] + i] = 0;
                    buf_bwLumaFrame[j * SVInPitch[0] + i] = 0;
                } else {
                    bwLumaFrame[j * SVInPitch[0] + i] = 255;
                    buf_bwLumaFrame[j * SVInPitch[0] + i] = 255;
                }
            }
        }
    }
    // Close - clears black specks in white noise (dilate -> erode) 
    /* Primary I/O bwLumaFrame is only for debug, in the final implmentation */
    /* only the buf_bwLumaFrame intermediate buffer will be used             */
    if (sv->enableSV_ACDetect == 1) {
        VXLIB_dilate_3x3_i8u_o8u(bwLumaFrame, &src_addr, in_image, &src_addr);
        VXLIB_erode_3x3_i8u_o8u(in_image, &src_addr, bwLumaFrame, &src_addr);
    }
    if (sv->enableSV_ACDetect == 1) {
        VXLIB_dilate_3x3_i8u_o8u(buf_bwLumaFrame, &src_addr, buf_grayLumaFrame, &src_addr);
        VXLIB_erode_3x3_i8u_o8u(buf_grayLumaFrame, &src_addr, buf_bwLumaFrame, &src_addr);
    }
}


/*===============================================================================
 *
 * Name:        svSearchFinderPatterns()
 *
 * Description: Search finder patterns according to the pattern profiles
 *
 *
 * Input:
 *   sv:        Handle for the SV structure
 *   prms:      Struct containing scratch buffers
 *
 * Returns:
 *              Number of detected candidate FPs
 * Effects:
 *
 ===============================================================================*/
 int32_t svSearchFinderPatterns(svPointDetect_t* sv, tivxPointDetectParams *prms)
{
    /////////////////////////////////////////////////////////
    int16_t  SVInCamFrmHeight = prms->img_prms.dim_y ;
    int16_t  SVInCamFrmWidth  = prms->img_prms.dim_x;
    int16_t  SVInPitch[2]     ={prms->img_prms.stride_y,0};
    // Parameters for the algorithm. Could be configurable
    int32_t verBorderOfst = sv->borderOffset;
    int32_t horBorderOfst = sv->borderOffset;

    int32_t blkWinSmall = sv->smallestCenter;  // for the 1st step
    int32_t blkWinLarge = sv->largestCenter;   // for the 2nd step

    uint32_t firstRoiTop = (sv->firstROITop * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t firstRoiBottom = (sv->firstROIBottom * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t firstRoiLeft = (sv->firstROILeft * SVInCamFrmWidth)/sv->SVROIWidth;
    uint32_t firstRoiRight = (sv->firstROIRight * SVInCamFrmWidth)/sv->SVROIWidth;

    uint32_t secondRoiTop = (sv->secondROITop * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t secondRoiBottom = (sv->secondROIBottom * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t secondRoiLeft = (sv->secondROILeft * SVInCamFrmWidth)/sv->SVROIWidth;
    uint32_t secondRoiRight = (sv->secondROIRight * SVInCamFrmWidth)/sv->SVROIWidth;
    /////////////////////////////////////////////////////////

    int16_t i, j, m, n;
    int32_t numFPs = 0;
    int32_t startVer, endVer, startHor, endHor, roi;
    uint8_t bAllWhite, bShortHor, bShortVer, bCandPix;
    uint8_t centerValue = 255; // white

    uint8_t *bwLumaFrame = prms->buf_bwLumaFrame_ptr;
    int16_t *candFPId    = prms->buf_candFPId_ptr;
    int16_t *candFPx     = prms->buf_candFPx_ptr;
    int16_t *candFPy     = prms->buf_candFPy_ptr;
    SV_ACDetect_FPBoundaryPos    *svFPBoundaryPosStruct = prms->buf_svFPBoundaryPosStruct_ptr; 


    if(SVInCamFrmHeight >= 960)
    {
        /* For 2MP resolution */
        verBorderOfst += 10;
        horBorderOfst += 10;
    }

    for (roi = 0; roi < 2; roi++) {
        if (roi == 0) {
            startVer = firstRoiTop;
            startHor = firstRoiLeft;
            endVer = firstRoiBottom;
            endHor = firstRoiRight;
        }
        else
        {
            startVer = secondRoiTop;
            startHor = secondRoiLeft;
            endVer = secondRoiBottom;
            endHor = secondRoiRight;
        }

           
        // file debug 
        #ifdef FILE_DEBUG
        {
            FILE *f =0;
            f = fopen("temp.yuv","wb");
            fwrite(bwLumaFrame,1,1280*720,f);
            fclose(f);
        }
        #endif
        
        for (m = startVer; m < endVer; m+=2) {
            for (n = startHor; n < endHor; n+=2) { 

                // 1st step: check if AxA pixels are all black
                bAllWhite = 1;
                for (j = -blkWinSmall; j <= blkWinSmall; j++) {
                    for (i = -blkWinSmall; i <= blkWinSmall; i++)
                    {
                        if (bwLumaFrame[(m + j) * SVInPitch[0] + n + i] != centerValue) {
                            bAllWhite = 0;
                            break;
                        }
                    }

                    if (bAllWhite == 0)
                        break;
                }

                //printf("Before exit-1, ballWhite  \n");
                // exit condition
                if (bAllWhite == 0) {
                    continue;
                }


                // 2nd step: check if black square is not too big - weak condition, but to speed up
                bShortHor = 0;
                for (i = -blkWinLarge; i <= blkWinLarge; i++) {
                    if (bwLumaFrame[m * SVInPitch[0] + n + i] != centerValue) {
                        bShortHor = 1;
                        break;
                    }
                }

                bShortVer = 0;
                for (j = -blkWinLarge; j <= blkWinLarge; j++) {
                    if (bwLumaFrame[(m + j) * SVInPitch[0] + n] != centerValue) {
                        bShortVer = 1;
                        break;
                    }
                }

                // exit condition
                if (bShortHor == 0 || bShortVer == 0) {
                    continue;
                }


                // 3rd step
                bCandPix = svCheckPatternProfileHiDia(sv, bwLumaFrame, m, n, SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[0], &svFPBoundaryPosStruct[4], 0.0, 1);
                // exit condition
                if (bCandPix == 0) {
                    continue;
                }



                // 4th step
                bCandPix = svCheckPatternProfileHiDia(sv, bwLumaFrame, m, n, SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[1], &svFPBoundaryPosStruct[5], 0.5, 1);
                if (bCandPix == 0) {
                    continue;
                }


                //// 5th step
                bCandPix = svCheckPatternProfileLowDia(sv, bwLumaFrame, m, n, SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[2], &svFPBoundaryPosStruct[6], 1.0, 1);
                // exit condition
                if (bCandPix == 0) {
                    continue;
                }


                // 6th step
                bCandPix = svCheckPatternProfileLowDia(sv, bwLumaFrame, m, n, SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[3], &svFPBoundaryPosStruct[7], 0.5, 1);
                // exit condition
                if (bCandPix == 0) {
                    continue;
                }


                bCandPix = svCheckAllDirections(svFPBoundaryPosStruct);



                if (bCandPix == 1) {
                    candFPId[numFPs] = -1;
                    candFPy[numFPs] = m;
                    candFPx[numFPs] = n;
                    numFPs++;
                }
            }
        }
    }

    return numFPs;
}

/*===============================================================================
 *
 * Name:            svCheckPatternProfileHiDia()
 *
 * Description:     Check finder pattern profile in  diagonal directions
 *
 *
 * Input:
 *   bwLumaFrame:   Binarized fisheye input
 *
 * Returns:
 *                  1 if the current pixel is candidate pixel of finder pattern
 * Effects:
 *
 ===============================================================================*/
 uint8_t svCheckPatternProfileHiDia(svPointDetect_t* sv, uint8_t* bwLumaFrame, int16_t posy, int16_t posx, int16_t height, int16_t width, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosA, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosB, double slope, uint8_t bScreen)
{
    /////////////////////////////////////////////////////////
    // Parameters for the algorithm. Could be configurable
    int16_t winW = sv->maxWinWidth;
    int16_t winH = sv->maxWinHeight;

    int16_t maxBandLen = sv->maxBandLen;
    int16_t minBandLen = sv->minBandLen;

    int16_t minChartSize = MIN_CHART_SIZE;

    double hMiddleTh = HIGH_MID_TH;
    double lMiddleTh = LOW_MID_TH;

    double hSideTh = SIDE_TH;
    /////////////////////////////////////////////////////////

    int32_t i, j;

    double leftMark45[4], rightMark45[4];
    double upMark135[4], downMark135[4];

    double leftBrun = 0, rightBrun = 0;
    double upBrun = 0, downBrun = 0;
    double centerRun45 = 0, centerRun135 = 0;

    double hTh, lTh;
    double prevI, prevJ, prevX, prevY, xinc, yinc;
    double dist;
    double temp1, temp2;
    int32_t xinc_r, yinc_r;

    uint8_t marker, numMarker;
    uint8_t numMarkerNeeded = 2;
    uint8_t firstMarker = 0;  // first marker is black

    if(height >= 960)
    {
        /* For 2 MP height the winW and winH are increased by 70 pixels */
        winW += 70;
        winH += 70;
    }

    ///////////////////////////////
    // down (left) scan
    ///////////////////////////////
    prevJ = 0.0;
    xinc = 0;
    j = 1;
    marker = firstMarker;
    numMarker = 0;

    while (j <= winH) {
        prevX = xinc;
        xinc_r = (int32_t)(j * slope + 0.5);
        xinc = j*slope;
        if (posy + j < height && posx - xinc_r >= 0 && bwLumaFrame[(posy + j)* width + posx - xinc_r] == marker) {
            if (bwLumaFrame[(posy + j)* width + posx - xinc_r] == marker) {
                
                leftMark45[numMarker*2]     = posy + prevJ;
                leftMark45[numMarker*2 + 1] = posx - prevX;

                if (numMarker == 1) {
                    temp1 = leftMark45[2] - leftMark45[0];
                    temp2 = leftMark45[3] - leftMark45[1];
                    //dist =  sqrt (pow( (double)(leftMark45[numMarker*2] - leftMark45[numMarker*2 - 2]), 2) + pow((double)(leftMark45[numMarker*2 + 1] - leftMark45[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 0.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevJ = j;
        j++;
    }

    if (numMarker < numMarkerNeeded)
        return 0;


    ///////////////////////////////
    // up (right) scan
    ///////////////////////////////
    prevJ = 0;
    xinc = 0;
    j = -1;
    marker = firstMarker;
    numMarker = 0;

    while (j >= -winH) {

        prevX = xinc;
        xinc_r = (int32_t)(-j * slope + 0.5);
        xinc = -j * slope;
        if (posy + j >= 0 && posx + xinc_r < width) {
            if (bwLumaFrame[(posy + j)* width + posx + xinc_r] == marker) {

                rightMark45[numMarker*2]     = posy + prevJ;
                rightMark45[numMarker*2 + 1] = posx + prevX;

                if (numMarker == 1) {
                    temp1 = rightMark45[2] - rightMark45[0];
                    temp2 = rightMark45[3] - rightMark45[1];
                    //dist =  sqrt (pow( (double)(rightMark45[numMarker*2] - rightMark45[numMarker*2 - 2]), 2) + pow((double)(rightMark45[numMarker*2 + 1] - rightMark45[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 0.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevJ = j;
        j--;
    }


    if (numMarker < numMarkerNeeded)
        return 0;


    if (bScreen) {
        // profile check
        temp1 = leftMark45[0] - leftMark45[2];
        temp2 = leftMark45[1] - leftMark45[3];
        //leftBrun = sqrt(pow((double)(leftMark45[0] - leftMark45[2]), 2) + pow((double)(leftMark45[1] - leftMark45[3]), 2));
        leftBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (leftBrun < minBandLen || leftBrun > maxBandLen)
            return 0;

        // profile check
        temp1 = rightMark45[0] - rightMark45[2];
        temp2 = rightMark45[1] - rightMark45[3];
        //rightBrun = sqrtdp_i(pow((double)(rightMark45[0] - rightMark45[2]), 2) + pow((double)(rightMark45[1] - rightMark45[3]), 2));
        rightBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (rightBrun < minBandLen || rightBrun > maxBandLen)
            return 0;

        // check ratio of all runs
        temp1 = rightMark45[0] - leftMark45[0] - 1.0;
        temp2 = rightMark45[1] - leftMark45[1] - 1.0;
        //centerRun45 = sqrt(pow((double)(rightMark45[0] - leftMark45[0]) - 1, 2) + pow((double)(rightMark45[1] - leftMark45[1]) - 1, 2));
        centerRun45 = sqrtdp_i(temp1*temp1 + temp2*temp2);

        hTh = centerRun45 * hMiddleTh;
        lTh = centerRun45 * lMiddleTh;

        // check pattern size - weak condition
        if (leftBrun < lTh || leftBrun > hTh || rightBrun < lTh || rightBrun > hTh)
            return 0;
        if ((rightBrun > hSideTh * leftBrun) || (leftBrun > hSideTh * rightBrun))
            return 0;
        if (centerRun45 + leftBrun + rightBrun < minChartSize)
            return 0;
    }

    // store boundary position
    svFPBoundaryPosA->xPos[0] = leftMark45[1];
    svFPBoundaryPosA->yPos[0] = leftMark45[0];
    svFPBoundaryPosA->xPos[1] = leftMark45[3];
    svFPBoundaryPosA->yPos[1] = leftMark45[2];
    svFPBoundaryPosA->xPos[2] = rightMark45[1];
    svFPBoundaryPosA->yPos[2] = rightMark45[0];
    svFPBoundaryPosA->xPos[3] = rightMark45[3];
    svFPBoundaryPosA->yPos[3] = rightMark45[2];

    svFPBoundaryPosA->centerRun = centerRun45;
    svFPBoundaryPosA->firstBrun = leftBrun;
    svFPBoundaryPosA->secondBrun = rightBrun;

    ////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////
    // (left) up scan
    ///////////////////////////////
    prevI = 0;
    yinc = 0;
    i = -1;
    marker = firstMarker;
    numMarker = 0;

    while (i >= -winW) {
        prevY = yinc;
        yinc_r = (int32_t)(-i * slope + 0.5);
        yinc = -i * slope;
        if (posx + i >= 0 && posy - yinc_r >= 0) {
            if (bwLumaFrame[(posy - yinc_r)* width + posx + i] == marker) {

                upMark135[numMarker*2] = posy - prevY;
                upMark135[numMarker*2 + 1] = posx + prevI;

                if (numMarker == 1) {
                    temp1 = upMark135[2] - upMark135[0];
                    temp2 = upMark135[3] - upMark135[1];
                    //dist =  sqrt (pow( (double)(upMark135[numMarker*2] - upMark135[numMarker*2 - 2]), 2) + pow((double)(upMark135[numMarker*2 + 1] - upMark135[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 0.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevI = i;
        i--;
    }

    if (numMarker < numMarkerNeeded)
        return 0;



    ///////////////////////////////
    // (right) down scan
    ///////////////////////////////
    prevI = 0;
    yinc = 0;
    i = 1;
    marker = firstMarker;
    numMarker = 0;

    while (i <= winW) {
        prevY = yinc;
        yinc_r = (int32_t)(i * slope + 0.5);
        yinc = i * slope;
        if (posx + i < width && posy + yinc_r < height) {
            if (bwLumaFrame[(posy + yinc_r)* width + posx + i] == marker) {

                downMark135[numMarker*2] = posy + prevY;
                downMark135[numMarker*2 + 1] = posx + prevI;

                if (numMarker == 1) {
                    temp1 = downMark135[2] - downMark135[0];
                    temp2 = downMark135[3] - downMark135[1];
                    //dist =  sqrt (pow( (double)(downMark135[numMarker*2] - downMark135[numMarker*2 - 2]), 2) + pow((double)(downMark135[numMarker*2 + 1] - downMark135[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 0.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevI = i;
        i++;
    }

    if (numMarker < numMarkerNeeded)
        return 0;


    if (bScreen) {
        // profile check
        temp1 = upMark135[0] - upMark135[2];
        temp2 = upMark135[1] - upMark135[3];
        //upBrun = sqrt(pow((double)(upMark135[0] - upMark135[2]), 2) + pow((double)(upMark135[1] - upMark135[3]), 2));
        upBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (upBrun < minBandLen || upBrun > maxBandLen)
            return 0;

        // profile check
        temp1 = downMark135[0] - downMark135[2];
        temp2 = downMark135[1] - downMark135[3];
        //downBrun = sqrt(pow((double)(downMark135[0] - downMark135[2]), 2) + pow((double)(downMark135[1] - downMark135[3]), 2));
        downBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (downBrun < minBandLen || downBrun > maxBandLen)
            return 0;

        // check ratio of all runs
        temp1 = downMark135[0] - upMark135[0] - 1.0;
        temp2 = downMark135[1] - upMark135[1] - 1.0;
        //centerRun135 = sqrt(pow((double)(downMark135[0] - upMark135[0] - 1), 2) + pow((double)(downMark135[1] - upMark135[1] - 1), 2));
        centerRun135 = sqrtdp_i(temp1*temp1 + temp2*temp2);

        hTh = centerRun135 * hMiddleTh;
        lTh = centerRun135 * lMiddleTh;

        if (upBrun < lTh || upBrun > hTh || downBrun < lTh || downBrun > hTh)
            return 0;
        if ((downBrun > hSideTh * upBrun) || (upBrun > hSideTh * downBrun))
            return 0;

        if (centerRun135 + upBrun + downBrun < minChartSize)
            return 0;
    }

    // store boundary position
    svFPBoundaryPosB->xPos[0] = upMark135[1];
    svFPBoundaryPosB->yPos[0] = upMark135[0];
    svFPBoundaryPosB->xPos[1] = upMark135[3];
    svFPBoundaryPosB->yPos[1] = upMark135[2];
    svFPBoundaryPosB->xPos[2] = downMark135[1];
    svFPBoundaryPosB->yPos[2] = downMark135[0];
    svFPBoundaryPosB->xPos[3] = downMark135[3];
    svFPBoundaryPosB->yPos[3] = downMark135[2];

    svFPBoundaryPosB->centerRun = centerRun135;
    svFPBoundaryPosB->firstBrun = upBrun;
    svFPBoundaryPosB->secondBrun = downBrun;


    return 1;
}


/*===============================================================================
 *
 * Name:            svCheckPatternProfileLowDia()
 *
 * Description:     Check finder pattern profile in low diagonal directions
 *
 *
 * Input:
 *   bwLumaFrame:   Binarized fisheye input
 *
 * Returns:
 *                  1 if the current pixel is candidate pixel of finder pattern
 * Effects:
 *
 ===============================================================================*/
 uint8_t svCheckPatternProfileLowDia(svPointDetect_t* sv, uint8_t* bwLumaFrame, int16_t posy, int16_t posx, int16_t height, int16_t width, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosA, SV_ACDetect_FPBoundaryPos* svFPBoundaryPosB, double slope, uint8_t bScreen)
{
    /////////////////////////////////////////////////////////
    // Parameters for the algorithm. Could be configurable
    int16_t winW = sv->maxWinWidth;
    int16_t winH = sv->maxWinHeight;

    int16_t maxBandLen = sv->maxBandLen;
    int16_t minBandLen = sv->minBandLen;

    int16_t minChartSize = MIN_CHART_SIZE;

    double hMiddleTh = HIGH_MID_TH;
    double lMiddleTh = LOW_MID_TH;

    double hSideTh = SIDE_TH;
    /////////////////////////////////////////////////////////

    int32_t i, j;

    double leftMark45[4], rightMark45[4];
    double upMark135[4], downMark135[4];

    double leftBrun = 0, rightBrun = 0;
    double upBrun = 0, downBrun = 0;
    double centerRun45 = 0, centerRun135 = 0;

    double hTh, lTh;
    double prevI, prevJ, prevX, prevY, xinc, yinc;
    double dist;
    double temp1, temp2;
    int32_t xinc_r, yinc_r;

    uint8_t marker, numMarker;
    uint8_t numMarkerNeeded = 2;
    uint8_t firstMarker = 0;  // first marker is black

    if(height >= 960)
    {
        /* For 2 MP height the winW and winH are increased by 70 pixels */
        winW += 70;
        winH += 70;
    }

    ///////////////////////////////
    // (down) left scan
    ///////////////////////////////
    prevI = 0;
    yinc = 0;
    i = -1;
    marker = firstMarker;
    numMarker = 0;

    while (i >= -winW) {
        prevY = yinc;
        yinc_r = (int32_t)(-i * slope + 0.5);
        yinc = -i * slope;
        if (posx + i >= 0 && posy + yinc_r < height) {
            if (bwLumaFrame[(posy + yinc_r)* width + posx + i] == marker) {

                leftMark45[numMarker*2]     = posy + prevY;
                leftMark45[numMarker*2 + 1] = posx + prevI;

                if (numMarker == 1) {
                    temp1 = leftMark45[2] - leftMark45[0];
                    temp2 = leftMark45[3] - leftMark45[1];
                    //dist =  sqrt (pow( (double)(leftMark45[numMarker*2] - leftMark45[numMarker*2 - 2]), 2) + pow((double)(leftMark45[numMarker*2 + 1] - leftMark45[numMarker*2 - 1]), 2) );

                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 1.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevI = i;
        i--;
    }

    if (numMarker < numMarkerNeeded)
        return 0;


    ///////////////////////////////
    // (up) right scan
    ///////////////////////////////
    prevI = 0;
    yinc = 0;
    i = 1;
    marker = firstMarker;
    numMarker = 0;

    while (i <= winW) {
        prevY = yinc;
        yinc_r = (int32_t)(i * slope + 0.5);
        yinc = i * slope;
        if (posx + i < width && posy - yinc_r >= 0) {
            if (bwLumaFrame[(posy - yinc_r)* width + posx + i] == marker) {

                rightMark45[numMarker*2]     = posy - prevY;
                rightMark45[numMarker*2 + 1] = posx + prevI;

                if (numMarker == 1) {
                    temp1 = rightMark45[2] - rightMark45[0];
                    temp2 = rightMark45[3] - rightMark45[1];
                    //dist =  sqrt (pow( (double)(rightMark45[numMarker*2] - rightMark45[numMarker*2 - 2]), 2) + pow((double)(rightMark45[numMarker*2 + 1] - rightMark45[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 1.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevI = i;
        i++;
    }

    if (numMarker < numMarkerNeeded)
        return 0;


    if (bScreen) {
        // profile check
        temp1 = leftMark45[0] - leftMark45[2];
        temp2 = leftMark45[1] - leftMark45[3];
        //leftBrun = sqrt(pow((double)(leftMark45[0] - leftMark45[2]), 2) + pow((double)(leftMark45[1] - leftMark45[3]), 2));
        leftBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (leftBrun < minBandLen || leftBrun > maxBandLen)
            return 0;

        // profile check
        temp1 = rightMark45[0] - rightMark45[2];
        temp2 = rightMark45[1] - rightMark45[3];
        //rightBrun = sqrt(pow((double)(rightMark45[0] - rightMark45[2]), 2) + pow((double)(rightMark45[1] - rightMark45[3]), 2));
        rightBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (rightBrun < minBandLen || rightBrun > maxBandLen)
            return 0;

        // check ratio of all runs
        temp1 = rightMark45[0] - leftMark45[0] - 1.0;
        temp2 = rightMark45[1] - leftMark45[1] - 1.0;
        //centerRun45 = sqrt(pow((double)(rightMark45[0] - leftMark45[0]) - 1, 2) + pow((double)(rightMark45[1] - leftMark45[1]) - 1, 2));
        centerRun45 = sqrtdp_i(temp1*temp1 + temp2*temp2);
        
        hTh = centerRun45 * hMiddleTh;
        lTh = centerRun45 * lMiddleTh;

        // check pattern size - weak condition
        if (leftBrun < lTh || leftBrun > hTh || rightBrun < lTh || rightBrun > hTh)
            return 0;
        if ( (rightBrun > hSideTh * leftBrun) || (leftBrun > hSideTh * rightBrun) )
            return 0;
        if (centerRun45 + leftBrun + rightBrun < minChartSize)
            return 0;
    }


    // store boundary position
    svFPBoundaryPosA->xPos[0] = leftMark45[1];
    svFPBoundaryPosA->yPos[0] = leftMark45[0];
    svFPBoundaryPosA->xPos[1] = leftMark45[3];
    svFPBoundaryPosA->yPos[1] = leftMark45[2];
    svFPBoundaryPosA->xPos[2] = rightMark45[1];
    svFPBoundaryPosA->yPos[2] = rightMark45[0];
    svFPBoundaryPosA->xPos[3] = rightMark45[3];
    svFPBoundaryPosA->yPos[3] = rightMark45[2];

    svFPBoundaryPosA->centerRun = centerRun45;
    svFPBoundaryPosA->firstBrun = leftBrun;
    svFPBoundaryPosA->secondBrun = rightBrun;
    ////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////
    // (left) up scan
    ///////////////////////////////
    prevJ = 0;
    xinc = 0;
    j = -1;
    marker = firstMarker;
    numMarker = 0;

    while (j >= -winH) {
        prevX = xinc;
        xinc_r = (int32_t)(-j * slope + 0.5);
        xinc = -j * slope;
        if (posy + j >= 0 && posx - xinc_r >= 0) {
            if (bwLumaFrame[(posy + j)* width + posx - xinc_r] == marker) {

                upMark135[numMarker*2] = posy + prevJ;
                upMark135[numMarker*2 + 1] = posx - prevX;

                if (numMarker == 1) {
                    temp1 = upMark135[2] - upMark135[0];
                    temp2 = upMark135[3] - upMark135[1];
                    //dist =  sqrt (pow( (double)(upMark135[numMarker*2] - upMark135[numMarker*2 - 2]), 2) + pow((double)(upMark135[numMarker*2 + 1] - upMark135[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 1.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevJ = j;
        j--;
    }

    if (numMarker < numMarkerNeeded)
        return 0;


    ///////////////////////////////
    // (right) down scan
    ///////////////////////////////
    prevJ = 0;
    xinc = 0;
    j = 1;
    marker = firstMarker;
    numMarker = 0;

    while (j <= winH) {
        prevX = xinc;
        xinc_r = (int32_t)(j * slope + 0.5);
        xinc = j * slope;
        if (posy + j < height && posx + xinc_r < width) {
            if (bwLumaFrame[(posy + j)* width + posx + xinc_r] == marker) {

                downMark135[numMarker*2] = posy + prevJ;
                downMark135[numMarker*2 + 1] = posx + prevX;

                if (numMarker == 1) {
                    temp1 = downMark135[2] - downMark135[0];
                    temp2 = downMark135[3] - downMark135[1];
                    //dist =  sqrt (pow( (double)(downMark135[numMarker*2] - downMark135[numMarker*2 - 2]), 2) + pow((double)(downMark135[numMarker*2 + 1] - downMark135[numMarker*2 - 1]), 2) );
                    dist = sqrtdp_i(temp1*temp1 + temp2*temp2);
                }

                if (numMarker == 0 || slope == 1.0 || (numMarker >  0 && dist > 4) )
                {
                    // toggle marker
                    marker = (marker == 255) ? 0: 255;
                    numMarker++;
                }
            }
        }

        if (numMarker == numMarkerNeeded)
            break;

        prevJ = j;
        j++;
    }

    if (numMarker < numMarkerNeeded)
        return 0;

    if (bScreen) {
        // profile check
        temp1 = upMark135[0] - upMark135[2];
        temp2 = upMark135[1] - upMark135[3];
        //upBrun = sqrt(pow((double)(upMark135[0] - upMark135[2]), 2) + pow((double)(upMark135[1] - upMark135[3]), 2));
        upBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (upBrun < minBandLen || upBrun > maxBandLen)
            return 0;

        // profile check
        temp1 = downMark135[0] - downMark135[2];
        temp2 = downMark135[1] - downMark135[3];
        //downBrun = sqrt(pow((double)(downMark135[0] - downMark135[2]), 2) + pow((double)(downMark135[1] - downMark135[3]), 2));
        downBrun = sqrtdp_i(temp1*temp1 + temp2*temp2);
        // check pattern size - weak condition
        if (downBrun < minBandLen || downBrun > maxBandLen)
            return 0;

        // check ratio of all runs
        temp1 = downMark135[0] - upMark135[0] - 1.0;
        temp2 = downMark135[1] - upMark135[1] - 1.0;
        //centerRun135 = sqrt(pow((double)(downMark135[0] - upMark135[0] - 1), 2) + pow((double)(downMark135[1] - upMark135[1] - 1), 2));
        centerRun135 = sqrtdp_i(temp1*temp1 + temp2*temp2);

        hTh = centerRun135 * hMiddleTh;
        lTh = centerRun135 * lMiddleTh;

        if (upBrun < lTh || upBrun > hTh || downBrun < lTh || downBrun > hTh)
            return 0;
        if ( (downBrun > hSideTh * upBrun) || (upBrun > hSideTh * downBrun) )
            return 0;
        if (centerRun135 + upBrun + downBrun < minChartSize)
            return 0;
    }

    // store boundary position
    svFPBoundaryPosB->xPos[0] = upMark135[1];
    svFPBoundaryPosB->yPos[0] = upMark135[0];
    svFPBoundaryPosB->xPos[1] = upMark135[3];
    svFPBoundaryPosB->yPos[1] = upMark135[2];
    svFPBoundaryPosB->xPos[2] = downMark135[1];
    svFPBoundaryPosB->yPos[2] = downMark135[0];
    svFPBoundaryPosB->xPos[3] = downMark135[3];
    svFPBoundaryPosB->yPos[3] = downMark135[2];

    svFPBoundaryPosB->centerRun = centerRun135;
    svFPBoundaryPosB->firstBrun = upBrun;
    svFPBoundaryPosB->secondBrun = downBrun;

    return 1;
}

/*===============================================================================
 *
 * Name:            svCheckAllDirections()
 *
 * Description:     check if a boundary point meets some conditions
 *
 *
 * Input:
 *   sv:            Handle for the SV structure
 *
 * Returns:
 *                  1 if the current pixel is candidate pixel of finder pattern
 * Effects:
 *
 ===============================================================================*/
 uint8_t svCheckAllDirections(SV_ACDetect_FPBoundaryPos* svFPBoundaryPosStruct)
{
    int16_t i;

    double outBlkLength[8];
    double outBlkMin, outBlkMax;

    SV_ACDetect_FPBoundaryPos *pFPBoundaryPos = svFPBoundaryPosStruct;

    for (i = 0; i < 8; i++) {
        outBlkLength[i] = pFPBoundaryPos[i].centerRun + pFPBoundaryPos[i].firstBrun + pFPBoundaryPos[i].secondBrun;
    }

    /* Check min and max chart sizes */
    outBlkMin = 16384;
    outBlkMax = 0;

    for (i = 0; i <8; i++) {
        if (outBlkLength[i] > outBlkMax) outBlkMax = outBlkLength[i];
        if (outBlkLength[i] < outBlkMin) outBlkMin = outBlkLength[i];
    }

    if (outBlkMax > MIN_MAX_CHART_RATIO * outBlkMin)
        return 0;


    return 1;
}

/*===============================================================================
 *
 * Name:            svFindFPCentroids()
 *
 * Description:     Find finder pattern clusters
 *
 *
 * Input:
 *   sv:            Handle for the SV structure
 *   numPts         numbre of candidate points
 *
 * Returns:
 *                  number of detected FP centroids
 * Effects:
 *
 ===============================================================================*/
 int16_t svFindFPCentroids(svPointDetect_t* sv, tivxPointDetectParams *prms, int16_t numPts) 
{
    int32_t j, idx;
    int16_t idFound, centerID = -1;
    double minDist, dist;

    int16_t *candFPId  = prms->buf_candFPId_ptr;
    int16_t *candFPx   = prms->buf_candFPx_ptr;
    int16_t *candFPy   = prms->buf_candFPy_ptr;


    for (j = 0; j < numPts; j++) {
        if (candFPId[j] == -1) {

            if (centerID  == -1)
            {
                // for the first sample
                centerID++;
                candFPId[j] = centerID;
                prms->buf_outCenterNum_ptr[centerID] = 1;
                prms->buf_outCenter_ptr[centerID*2] = candFPy[j];
                prms->buf_outCenter_ptr[centerID*2 + 1] = candFPx[j];
            } else {

                // search cluster to which the pixel belongs to
                idFound = svSearchCluster(sv, prms, candFPy[j], candFPx[j], centerID);

                if (idFound != -1) {
                    candFPId[j] = idFound;

                    // update a center
                    prms->buf_outCenter_ptr[idFound*2] = (prms->buf_outCenter_ptr[idFound*2]  * prms->buf_outCenterNum_ptr[idFound] + candFPy[j]) * 1.0 / (prms->buf_outCenterNum_ptr[idFound] + 1);
                    prms->buf_outCenter_ptr[idFound*2 + 1] = (prms->buf_outCenter_ptr[idFound*2 + 1]  * prms->buf_outCenterNum_ptr[idFound] + candFPx[j]) * 1.0 / (prms->buf_outCenterNum_ptr[idFound] + 1);
                    prms->buf_outCenterNum_ptr[idFound]++;

                } else {
                    centerID++;
                    candFPId[j] = centerID;
                    prms->buf_outCenterNum_ptr[centerID] = 1;
                    prms->buf_outCenter_ptr[centerID*2] = candFPy[j];
                    prms->buf_outCenter_ptr[centerID*2 + 1] = candFPx[j];
                }
            }
        }
    }

    // The final center is one among found candidate that is closest to the center.
    // Otherwise, when searching corner points, zero crossing points might be wrong
    for (idx = 0; idx < centerID + 1; idx++)
    {
        minDist = 1000000;
        for (j = 0; j < numPts; j++)
        {
            if (candFPId[j] == idx)
            {
                dist = fabs(prms->buf_outCenter_ptr[idx * 2] - candFPy[j]) + fabs(prms->buf_outCenter_ptr[idx * 2 + 1] - candFPx[j]);
                if (dist < minDist) {
                    minDist = dist;
                    prms->buf_intOutCenter_ptr[idx * 2] = candFPy[j];
                    prms->buf_intOutCenter_ptr[idx * 2 + 1] = candFPx[j];
                }
            }
        }
    }

#if 0
    for (idx = 0; idx < centerID + 1; idx++)
    {
        printf("%d, %d, count %d\n", sv->intOutCenter[idx * 2], sv->intOutCenter[idx * 2 + 1], sv->outCenterNum[idx]);

    }
#endif

    // if centroidID is more than MAX_FP_ALL, we assume something is wrong and we cannot find finder patterns.
    return (centerID + 1);
}

/*===============================================================================
 *
 * Name:            svSearchCluster()
 *
 * Description:     find cluster that a point belongs to
 *
 *
 * Input:
 *   x:             current pixel x pos
 *   y:             current pixel y pos
 *
 * Returns:
 *
 * Effects:
 *
 ===============================================================================*/
 int16_t svSearchCluster(svPointDetect_t* sv, tivxPointDetectParams *prms, int16_t y, int16_t x, int16_t numCenters)
{
    int16_t idFound = -1;
    int16_t i, m, n, rx, ry;

    double minDist = 10000, dist;
    int16_t bPixCount;
    int16_t maxX, maxY, minX, minY;

#ifdef PC_VERSION
    int16_t *candFPId  = prms->buf_candFPId_ptr;
    int16_t *candFPx   = prms->buf_candFPx_ptr;
    int16_t *candFPy   = prms->buf_candFPy_ptr;
#endif

    double slope, intersect;

    uint8_t *bwLumaFrame = prms->buf_bwLumaFrame_ptr;
    int16_t  SVInPitch[2]     ={prms->img_prms.stride_y,0};
    int16_t width = SVInPitch[0];

    uint8_t invalidPix = 0; // black

    for (i = 0; i < numCenters + 1; i++) {
        ry = (int16_t)(prms->buf_outCenter_ptr[i*2] + 0.5);
        rx = (int16_t)(prms->buf_outCenter_ptr[i*2 + 1] + 0.5);

        dist = sqrt(pow((double)(y - ry), 2) + pow((double)(x - rx), 2));

        if (dist < minDist) {
            if (x == rx && y == ry) {
                minDist = dist;
                idFound = i;
            } else if (x == rx) {
                bPixCount = 0;
                maxY = MAX(y, ry);
                minY = MIN(y, ry);

                for (m = minY + 1; m <= maxY; m++) {
                    if (bwLumaFrame[(m - 1) * width + x] == invalidPix &&
                        bwLumaFrame[(m) * width + x - 1] == invalidPix && bwLumaFrame[(m) * width + x] == invalidPix &&
                        bwLumaFrame[(m) * width + x + 1] == invalidPix && bwLumaFrame[(m + 1) * width + x] == invalidPix) {
                            bPixCount++;
                            break;
                    }
                }

                if (bPixCount == 0) {
                    minDist = dist;
                    idFound = i;
                }

            } else if (y == ry) {
                bPixCount = 0;
                maxX = MAX(x, rx);
                minX = MIN(x, rx);

                for (n = minX + 1; n <= maxX; n++) {
                    if (bwLumaFrame[(y - 1) * width + n] == invalidPix &&
                        bwLumaFrame[(y) * width + n - 1] == invalidPix && bwLumaFrame[(y) * width + n] == invalidPix &&
                        bwLumaFrame[(y) * width + n + 1] == invalidPix && bwLumaFrame[(y + 1) * width + n] == invalidPix) {
                            bPixCount++;
                            break;
                    }
                }

                if (bPixCount == 0) {
                    minDist = dist;
                    idFound = i;
                }
            } else {
                slope = (y - ry) * 1.0 / (x - rx);
                intersect = y - slope * x;

                bPixCount = 0;

                if (abs(x - rx) > abs(y-ry)) {
                    maxX = MAX(x, rx);
                    minX = MIN(x, rx);

                    for (n = minX + 1; n <= maxX; n++) {
                        m = (int16_t) (slope * n + intersect);

                        if (bwLumaFrame[(m - 1) * width + n] == invalidPix &&
                            bwLumaFrame[(m) * width + n - 1] == invalidPix && bwLumaFrame[(m) * width + n] == invalidPix &&
                            bwLumaFrame[(m) * width + n + 1] == invalidPix && bwLumaFrame[(m + 1) * width + n] == invalidPix) {
                                bPixCount++;
                                break;
                        }
                    }

                } else {
                    maxY = MAX(y, ry);
                    minY = MIN(y, ry);

                    for (m = minY + 1; m < maxY; m++) {
                        n = (int16_t)((m - intersect) / slope + 0.5);

                        if (bwLumaFrame[(m - 1) * width + n] == invalidPix &&
                            bwLumaFrame[(m) * width + n - 1] == invalidPix && bwLumaFrame[(m) * width + n] == invalidPix &&
                            bwLumaFrame[(m) * width + n + 1] == invalidPix && bwLumaFrame[(m + 1) * width + n] == invalidPix) {
                                bPixCount++;
                                break;
                        }
                    }


                }

                if (bPixCount == 0) {
                    minDist = dist;
                    idFound = i;
                }
            }
        }

    }

    return idFound;
}


 int16_t svRemoveNoiseFP(svPointDetect_t* sv,svLdcLut_t *ldcLut,  tivxPointDetectParams *prms,  int16_t numFPs,svACDetectStructFinalCorner_t *finalCornerSet )
// int16_t svRemoveNoiseFP(svPointDetect_t* sv, tivxPointDetectParams *prms,  int16_t numFPs)
{
    int16_t i, k;
    int16_t camera_id;

    uint8_t bValidCenter, bValidCenter1, bValidCenter2;
    int16_t validNumCenter = 0, finalNumCenter = 0;
    int16_t  SVInCamFrmHeight = prms->img_prms.dim_y ;
    int16_t  SVInPitch[2]     ={prms->img_prms.stride_y,0};

#ifdef PC_VERSION
    int16_t hWidth = SVInPitch[0] / 2;
#endif

    int16_t totalBoundaryPts = NUM_PT_BOUNDARY;
    //int8_t bCandPixHiDia[8], bCandPixLowDia[8];
    uint8_t bCandPixHiDia[16], bCandPixLowDia[16];
    int16_t lineStart[4], lineEnd[4];

    uint8_t  *bwLumaFrame = prms->buf_bwLumaFrame_ptr;
    int16_t  *outCenterNum = prms->buf_outCenterNum_ptr;

    int16_t  *candidCenter = prms->buf_candidCenter_ptr;
    int16_t  *intOutCenter = prms->buf_intOutCenter_ptr;

    double *boundX      = prms->buf_boundX_ptr;
    double *boundY      = prms->buf_boundY_ptr;
    double *tempBoundX  = prms->buf_tempBoundX_ptr;
    double *tempBoundY  = prms->buf_tempBoundY_ptr;
    double *fcBoundX    = prms->buf_fcBoundX_ptr;
    double *fcBoundY    = prms->buf_fcBoundY_ptr;
    double *tempCorner  = prms->buf_tempCorner_ptr;
    double *tempCorner1 = prms->buf_tempCorner1_ptr;
    double *tempCorner2 = prms->buf_tempCorner2_ptr;
    double *matchScore  = prms->buf_matchScore_ptr;

    SV_ACDetect_FPBoundaryPos *svFPBoundaryPosStruct = prms->buf_svFPBoundaryPosStruct_ptr;
    double patternSize;
    double minError, minError1, minError2;

    // LDC
    /* dtype is typically set to float */
    dtype point_d[2], point_u[2];
    dtype rdSq;
    camera_id   = sv->camera_id;




    for (i = 0; i < numFPs; i++) {
        if (outCenterNum[i] >= sv->minSampleInCluster) {
            candidCenter[validNumCenter * 2]     = intOutCenter[i*2];
            candidCenter[validNumCenter * 2 + 1] = intOutCenter[i*2 + 1];
            validNumCenter++;
        }
    }

#ifdef PC_VERSION
    printf("Number of initial valid centers: %d \n", validNumCenter);
#endif

    // exit condition
    if (validNumCenter == 0) {
        return 0;
    }

    if (validNumCenter > 0) {
        for (i = 0; i< validNumCenter; i++)
        {
            bCandPixHiDia[0] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[0], &svFPBoundaryPosStruct[32], 0.0, 0);
            bCandPixHiDia[1] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[1], &svFPBoundaryPosStruct[33], 0.0625, 0);
            bCandPixHiDia[2] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[2], &svFPBoundaryPosStruct[34], 0.125, 0);
            bCandPixHiDia[3] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[3], &svFPBoundaryPosStruct[35], 0.1875, 0);
            bCandPixHiDia[4] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[4], &svFPBoundaryPosStruct[36], 0.25, 0);
            bCandPixHiDia[5] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[5], &svFPBoundaryPosStruct[37], 0.3125, 0);
            bCandPixHiDia[6] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[6], &svFPBoundaryPosStruct[38], 0.375, 0);
            bCandPixHiDia[7] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[7], &svFPBoundaryPosStruct[39], 0.4375, 0);
            bCandPixHiDia[8] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[8], &svFPBoundaryPosStruct[40], 0.5, 0);
            bCandPixHiDia[9] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[9], &svFPBoundaryPosStruct[41], 0.5625, 0);
            bCandPixHiDia[10] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[10], &svFPBoundaryPosStruct[42], 0.625, 0);
            bCandPixHiDia[11] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[11], &svFPBoundaryPosStruct[43], 0.6875, 0);
            bCandPixHiDia[12] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[12], &svFPBoundaryPosStruct[44], 0.75, 0);
            bCandPixHiDia[13] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[13], &svFPBoundaryPosStruct[45], 0.8125, 0);
            bCandPixHiDia[14] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[14], &svFPBoundaryPosStruct[46], 0.875, 0);
            bCandPixHiDia[15] = svCheckPatternProfileHiDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[15], &svFPBoundaryPosStruct[47], 0.9375, 0);

            bCandPixLowDia[0] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[16], &svFPBoundaryPosStruct[48], 1.0, 0);
            bCandPixLowDia[1] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[17], &svFPBoundaryPosStruct[49], 0.9375, 0);
            bCandPixLowDia[2] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[18], &svFPBoundaryPosStruct[50], 0.875, 0);
            bCandPixLowDia[3] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[19], &svFPBoundaryPosStruct[51], 0.8125, 0);
            bCandPixLowDia[4] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[20], &svFPBoundaryPosStruct[52], 0.75, 0);
            bCandPixLowDia[5] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[21], &svFPBoundaryPosStruct[53], 0.6875, 0);
            bCandPixLowDia[6] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[22], &svFPBoundaryPosStruct[54], 0.625, 0);
            bCandPixLowDia[7] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[23], &svFPBoundaryPosStruct[55], 0.5625, 0);
            bCandPixLowDia[8] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[24], &svFPBoundaryPosStruct[56], 0.5, 0);
            bCandPixLowDia[9] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[25], &svFPBoundaryPosStruct[57], 0.4375, 0);
            bCandPixLowDia[10] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[26], &svFPBoundaryPosStruct[58], 0.375, 0);
            bCandPixLowDia[11] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[27], &svFPBoundaryPosStruct[59], 0.3125, 0);
            bCandPixLowDia[12] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[28], &svFPBoundaryPosStruct[60], 0.25, 0);
            bCandPixLowDia[13] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[29], &svFPBoundaryPosStruct[61], 0.1875, 0);
            bCandPixLowDia[14] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[30], &svFPBoundaryPosStruct[62], 0.125, 0);
            bCandPixLowDia[15] = svCheckPatternProfileLowDia(sv, bwLumaFrame, candidCenter[i * 2], candidCenter[i * 2 + 1], SVInCamFrmHeight, SVInPitch[0], &svFPBoundaryPosStruct[31], &svFPBoundaryPosStruct[63], 0.0625, 0);

            // init boundary points
            /* Move memset to the init functions */
            //memset(sv->boundX, 0, sizeof(double) * totalBoundaryPts);
            //memset(sv->boundY, 0, sizeof(double) * totalBoundaryPts);

            for (k = 0; k < 16; k++)
            {
                if (bCandPixHiDia[k]) {
                    boundX[k]      = svFPBoundaryPosStruct[k].xPos[1];      boundY[k]      = svFPBoundaryPosStruct[k].yPos[1];
                    boundX[k + 64] = svFPBoundaryPosStruct[k].xPos[3];      boundY[k + 64] = svFPBoundaryPosStruct[k].yPos[3];
                    boundX[k + 32] = svFPBoundaryPosStruct[k + 32].xPos[1]; boundY[k + 32] = svFPBoundaryPosStruct[k + 32].yPos[1];
                    boundX[k + 96] = svFPBoundaryPosStruct[k + 32].xPos[3]; boundY[k + 96] = svFPBoundaryPosStruct[k + 32].yPos[3];
                }


                if (bCandPixLowDia[k]) {
                    boundX[k + 16]  = svFPBoundaryPosStruct[k + 16].xPos[1];  boundY[k + 16]  = svFPBoundaryPosStruct[k + 16].yPos[1];
                    boundX[k + 80]  = svFPBoundaryPosStruct[k + 16].xPos[3];  boundY[k + 80]  = svFPBoundaryPosStruct[k + 16].yPos[3];
                    boundX[k + 48]  = svFPBoundaryPosStruct[k + 48].xPos[1];  boundY[k + 48]  = svFPBoundaryPosStruct[k + 48].yPos[1];
                    boundX[k + 112] = svFPBoundaryPosStruct[k + 48].xPos[3];  boundY[k + 112] = svFPBoundaryPosStruct[k + 48].yPos[3];
                }
            }

            // remove the disqualified points
            if (sv->Ransac == 0)
            {
                int16_t cx, cy, exceptionCnt;
                double dist0, dist1, dist2, dx, dy;
                exceptionCnt = 0;
                cx = candidCenter[i * 2 + 1];
                cy = candidCenter[i * 2 + 0];
                for (k = 1; k < 127; k++)
                {
                    dx = (boundX[k - 1] - cx);
                    dy = (boundY[k - 1] - cy);
                    dist0 = sqrt(dx*dx + dy*dy);

                    dx = (boundX[k + 0] - cx);
                    dy = (boundY[k + 0] - cy);
                    dist1 = sqrt(dx*dx + dy*dy);

                    dx = (boundX[k + 1] - cx);
                    dy = (boundY[k + 1] - cy);
                    dist2 = sqrt(dx*dx + dy*dy);

                    if ((dist0 / dist2 > 0.8) && (dist1 / dist2 < 0.5))
                    {
                        boundX[k] = 0;
                        boundY[k] = 0;
                        exceptionCnt++;
                    }
                    if (exceptionCnt > 5)
                        break;
                }
            }
            
            // interpolate mission boundaries
            svInterpolateMissingBoundary(boundY, boundX, tempBoundY, tempBoundX, totalBoundaryPts);

            // LDC
            #ifdef DEBUG_2
            printf("Printing x coordinates followed by y \n ");
            #endif
            for (k = 0; k < totalBoundaryPts; k++) { //128 points
                point_d[0] = (dtype) boundX[k]; //DKY: Dump these points in distorted domain and plot on matlab on image
                point_d[1] = (dtype) boundY[k];

                LDC_DistToUndist(&ldcLut->ldc[camera_id], point_d, point_u, &rdSq);
                /* sv->ldc is an object of LensDistortionCorrection) */
                #ifdef DEBUG_2
                //Debug point to ensure LDC map is correct
                if (k==0) {
                    printf("distorted = %f %f undistorted = %f  %f \n",point_d[0],point_d[1],point_u[0],point_u[1]);
                    printf("ldc_lut->ldc[camera_id]->distCenterX = %f \n",ldcLut->ldc[camera_id]->distCenterX);
                    printf("sv->ldc[0].distCenterX = %f \n",sv->ldc[0].distCenterX);
                    printf("sv->ldc[0].distCenterY = %f \n",sv->ldc[0].distCenterY);
                }
                #endif

                fcBoundX[k] = point_u[0];
                fcBoundY[k] = point_u[1];

                #ifdef DEBUG_2
                /* Use these to plot out all the 128 corner points */
                /* The prints can be imported to matlab            */
                printf("plot (");
                printf("%f, %f ",boundX[k], boundY[k]);
                printf(",'r*') \n");
                #endif

            }

#if 0
            printf("=========== Boundary\n");
            for (k =0; k < totalBoundaryPts; k++)
                printf("%.4lf, %.4lf\n", sv->boundX[k], sv->boundY[k]);
            printf("=========== Boundary fisheye correct\n");
            for (k =0; k < totalBoundaryPts; k++)
                printf("%.4lf, %.4lf\n", sv->fcBoundX[k], sv->fcBoundY[k]);

            printf("\n");
#endif
            // RANSAC (outlier removal)
            if (sv->Ransac == 1)
            {
                minError = svRansacLineFit(sv,prms, boundY, boundX, fcBoundY, fcBoundX, tempCorner, totalBoundaryPts, &ldcLut->ldc[camera_id]);
                bValidCenter = svCheckFPCondition(sv,prms, tempCorner);
            }
            else
            {
                // get initial points on each boundary line
                svGetInitialLines(sv, prms, boundY, boundX, fcBoundY, fcBoundX, totalBoundaryPts, lineStart, lineEnd);

                // search four corners
                minError1 = svRefineFPCorner(sv, prms, boundY, boundX, fcBoundY, fcBoundX, tempCorner1, totalBoundaryPts, lineStart, lineEnd, &ldcLut->ldc[camera_id], 1); //DKY - Fisheye domain

                minError2 = svRefineFPCorner(sv, prms, fcBoundY, fcBoundX, NULL, NULL, tempCorner2, totalBoundaryPts, lineStart, lineEnd, &ldcLut->ldc[camera_id], 0); //DKY- corrected domain

                // check if corners are valid

                bValidCenter1 = svCheckFPCondition(sv,prms, tempCorner1);
                bValidCenter2 = svCheckFPCondition(sv,prms, tempCorner2);

                bValidCenter = 0;
                if (bValidCenter1 && bValidCenter2) {
                    bValidCenter = 1;
                    if (minError1 <= minError2) {
                        memcpy(tempCorner, tempCorner1, 8 * sizeof(double));
                        minError = minError1;
                    }
                    else {
                        memcpy(tempCorner, tempCorner2, 8 * sizeof(double));
                        minError = minError2;
                    }
                }
                else if (bValidCenter1) {
                    memcpy(tempCorner, tempCorner1, 8 * sizeof(double));
                    minError = minError1;
                    bValidCenter = 1;
                }
                else if (bValidCenter2) {
                    memcpy(tempCorner, tempCorner2, 8 * sizeof(double));
                    minError = minError2;
                    bValidCenter = 1;
                }
            }

            // compare minError to selet minimum error corners
            if (bValidCenter)
            {
                if (validNumCenter > FP_TO_DETECT) {
                    patternSize  = sqrt(pow(tempCorner[0] - tempCorner[2],2) + pow(tempCorner[1] - tempCorner[3], 2));
                    patternSize += sqrt(pow(tempCorner[2] - tempCorner[4],2) + pow(tempCorner[3] - tempCorner[5], 2));
                    patternSize += sqrt(pow(tempCorner[4] - tempCorner[6],2) + pow(tempCorner[5] - tempCorner[7], 2));
                    patternSize += sqrt(pow(tempCorner[6] - tempCorner[0],2) + pow(tempCorner[7] - tempCorner[1], 2));

                    matchScore[finalNumCenter] = minError / sqrt(patternSize);
                }

                for (k = 0 ; k < 8; k++) {
                    prms->buf_candidCorners_ptr[finalNumCenter][k] = (int32_t) (tempCorner[k]*16 + 16 + 0.5); // Add 16 to make MATLAB coordinate
                }

                finalNumCenter++;
            }
        } //validnumCenter
    }

#ifdef PC_VERSION
    printf("Number of valid set of corners: %d \n", finalNumCenter);
#endif

    // pick the best corners based on score
    int cpy;

    if (finalNumCenter <= FP_TO_DETECT)
    {
        for (k = 0; k < finalNumCenter; k++)
            //memcpy((finalCornerSet->finalCorners[k]), prms->buf_candidCorners_ptr[k], 8 * sizeof(int32_t));
            for (cpy =0;cpy<8;cpy++)  {
                finalCornerSet->finalCorners[k][cpy] = prms->buf_candidCorners_ptr[k][cpy];
            }

        return finalNumCenter;

    } else {
        int16_t minIdx = 0;

        for (i = 0; i < FP_TO_DETECT; i++)
        {
            // sort - can be optimized
            minError = 5000000;
            for (k = 0; k < finalNumCenter; k++) {
                if (matchScore[k] != -1 && matchScore[k] < minError) {
                    minError = matchScore[k];
                    minIdx =k;
                }
            }

            //memcpy(prms->buf_finalCorners_ptr[0][i], prms->buf_candidCorners_ptr[minIdx], 8 * sizeof(int32_t));
            for (cpy =0;cpy<8;cpy++)  {
                finalCornerSet->finalCorners[i][cpy] = prms->buf_candidCorners_ptr[minIdx][cpy];
            }
            matchScore[minIdx] = -1; // exclude in the next iteration
        }

        return FP_TO_DETECT;
    }


}


 void svInterpolateMissingBoundary(double* inY, double* inX, double *tempY, double *tempX, int16_t numPts)
{
    int16_t i, idx, lIdx, rIdx, lDist, rDist;

    memcpy(tempX, inX, sizeof(double)*numPts);
    memcpy(tempY, inY, sizeof(double)*numPts);



    for (i = 0; i < numPts; i++)
    {
        if (tempX[i] == 0) {
            lDist = rDist = 0;
            lIdx = rIdx = 0;

            for (idx = 1; idx <= 3; idx++)
            {
                lIdx = (i - idx + numPts) % numPts;
                if (tempX[lIdx] != 0) {
                    lDist = i;
                    break;
                }
            }

            for (idx = 1; idx <= 3; idx++)
            {
                rIdx = (i + idx + numPts) % numPts;
                if (tempX[rIdx] != 0) {
                    rDist = i;
                    break;
                }
            }

            inX[i] = (tempX[lIdx] * rDist + tempX[rIdx] * lDist) / (lDist + rDist);
            inY[i] = (tempY[lIdx] * rDist + tempY[rIdx] * lDist) / (lDist + rDist);

        } else {
            inX[i] = tempX[i];
            inY[i] = tempY[i];
        }
    }
}



 LDC_status LDC_DistToUndist(LensDistortionCorrection* ldc, dtype point_in[2], dtype point_out[2], dtype *rdSq_out)
{
#if LDC_LIB_DATA_TYPE!=0 && LDC_LIB_DATA_TYPE!=1
        "LDC_LIB_DATA_TYPE must be 0 (float) or 1 (double)"
#endif
#if LDC_LIB_D2U_LUT_TYPE!=0 
        "LDC_LIB_D2U_LUT_TYPE must be 0 "
#endif

        LDC_status status;
        dtype diffX, diffY;
        dtype ruDivRd;

        diffX = point_in[0] - ldc->distCenterX;
        diffY = point_in[1] - ldc->distCenterY;
        *rdSq_out = diffX*diffX + diffY*diffY;

#if LDC_LIB_D2U_LUT_TYPE == 0
        ruDivRd = lut_lookup_floating(ldc->lut_d2u, *rdSq_out, ldc->lut_d2u_indMax, ldc->lut_d2u_stepInv, &status);
        point_out[0] = diffX * ruDivRd + ldc->distCenterX;
        point_out[1] = diffY * ruDivRd + ldc->distCenterY;
#endif

        return status;
}

/***********************************************************************************
*
************************************************************************************/


/*===============================================================================
*
* Name:        svRansacLineFit()
*
* Description: Calculates corner points & error using RANSAC to estimate the 4 lines 
*
*
* Input:
*   sv:             Handle for the SV structure
*   inX & inY:      Pattern Boundary Points
*   fcX & fcY:      Fiesheye Corrected Boundary Points
*   corner:         Pointer for storing coner locations
*   numPts:         Number of boundary points
*
* Returns:
*              returns line fit error measurement 
*
===============================================================================*/

 double svRansacLineFit(svPointDetect_t* sv,tivxPointDetectParams *prms ,double* inY, double* inX, double* fcY, double* fcX, double* corner, int16_t numPts, LensDistortionCorrection *ldc)
{
    double minAvgError = 100000;
    double tempError;
    double* tempCorner = prms->buf_tempCorner1_ptr;
    double* line1PtsY  = prms->buf_line1PtsY_ptr;
    double* line1PtsX  = prms->buf_line1PtsX_ptr;
    double* line2PtsY  = prms->buf_line2PtsY_ptr;
    double* line2PtsX  = prms->buf_line2PtsX_ptr;


    double slope[4], icpt[4], XValue[4];
    int minNumOutlier = numPts;
    int16_t i, j, k;

    int32_t idxArrTop[4];
    int32_t idxArrBot[4];
    int32_t randomNumber[64];
    uint8_t optLineIdxArr[128];
    //int8_t LineIdxArr[128];
    uint8_t * LineIdxArr = (uint8_t *)calloc(128, sizeof(uint8_t)); //Shashank: Need to see why this was changed
    uint8_t numOutlier;
    uint8_t candLineIdx[128][4];
    uint8_t sIdx;
    uint8_t eIdx;
    uint8_t lineIdx[8];
    uint8_t temp;
    uint8_t NaNslope[4];
    uint8_t finalLineIdx[8];
    uint8_t errTh = 4;

    if (NULL == LineIdxArr)
    {
#ifdef PC_VERSION
        printf("WARNING: LDC stepped out of bounds");
#else
        VX_PRINT(VX_ZONE_ERROR, "WARNING: LDC stepped out of bounds\n");
#endif
        return -1;
    }

    // LDC
    dtype point_u[2], point_d[2];
    LDC_status status;

    for (i = 0; i < 5000; i++)
    {
        for (j = 0; j < numPts; j++)
            for (k = 0; k < 4; k++)
                candLineIdx[j][k] = 0;

        // Assign Random Points for Lines
        svRandomPerm(64, randomNumber, 4, idxArrTop);
        svRandomPerm(64, randomNumber, 4, idxArrBot);
        for (j = 0; j < 4; j++)
        {
            idxArrTop[j] = idxArrTop[j] + 32;
            idxArrBot[j] = idxArrBot[j] + 95;
        }

        // Sort the random points (improve efficiency)
        for (k = 0; k < 3; k++)
        {
            for (j = 0; j < 3; j++)
            {
                if (idxArrTop[j] > idxArrTop[j + 1])
                {
                    temp = idxArrTop[j];
                    idxArrTop[j] = idxArrTop[j + 1];
                    idxArrTop[j + 1] = temp;
                }
            }
        }
        for (k = 0; k < 3; k++)
        {
            for (j = 0; j < 3; j++)
            {
                if (idxArrBot[j] > idxArrBot[j + 1])
                {
                    temp = idxArrBot[j];
                    idxArrBot[j] = idxArrBot[j + 1];
                    idxArrBot[j + 1] = temp;
                }
            }
        }

        /* Re-arrange points so that
        line1: points 0 and 1, line2 : points 2 and 3
        line3 : points 4 and 5, line4 : points 6 and 7 */
        for (j = 0; j < 8; j++)
        {
            if (j > 2 && j != 7)
                lineIdx[j] = idxArrBot[j - 3] % numPts;
            else if (j < 3)
                lineIdx[j] = idxArrTop[j + 1];
            else
                lineIdx[j] = idxArrTop[0];
        }

        // Line estimates 
        for (j = 0; j < 4; j++)
        {
            line1PtsY[j] = inY[lineIdx[j * 2]];
            line1PtsX[j] = inX[lineIdx[j * 2]];
            line2PtsY[j] = inY[lineIdx[j * 2 + 1]];
            line2PtsX[j] = inX[lineIdx[j * 2 + 1]];
            svGetLineFrom2Pts(sv,prms, &NaNslope[j], &slope[j], &icpt[j], &XValue[j], j);
        }

        // Set Canidate Lines for all boundary points
        for (j = 0; j < 3; j++)
        {
            sIdx = lineIdx[j * 2];
            eIdx = lineIdx[j * 2 + 2] - 1;
            if (eIdx < sIdx)
                eIdx = eIdx + 128;
            for (k = sIdx; k <= eIdx; k++)
                candLineIdx[k % numPts][j] = 1;


            sIdx = lineIdx[j * 2 + 1] + 1;
            eIdx = lineIdx[j * 2 + 2] - 1;
            if (eIdx < sIdx)
                eIdx = eIdx + 128;
            for (k = sIdx; k <= eIdx; k++)
                candLineIdx[k % numPts][j + 1] = 1;
        }
        sIdx = lineIdx[6];
        eIdx = lineIdx[0] - 1;
        if (eIdx < sIdx)
            eIdx = eIdx + 128;
        for (k = sIdx; k <= eIdx; k++)
            candLineIdx[k % numPts][3] = 1;

        sIdx = lineIdx[7] + 1;
        eIdx = lineIdx[0] - 1;
        if (eIdx < sIdx)
            eIdx = eIdx + 128;
        for (k = sIdx; k <= eIdx; k++)
            candLineIdx[k % numPts][0] = 1;


        // Associaete all boundary points to one of lines
        tempError = svAssociatePointsToLine(sv,prms, NaNslope, slope, icpt, XValue, numPts, errTh, candLineIdx, LineIdxArr, &numOutlier);

        // Replace if better fit
        if (numOutlier < minNumOutlier)
        {
            minAvgError = tempError;
            minNumOutlier = numOutlier;
            memcpy(optLineIdxArr, LineIdxArr, 128 * sizeof(uint8_t));
            memcpy(finalLineIdx, lineIdx, 8 * sizeof(uint8_t));
        }
        else if (numOutlier == minNumOutlier)
        {
            if (tempError < minAvgError)
            {
                minAvgError = tempError;
                memcpy(optLineIdxArr, LineIdxArr, 128 * sizeof(uint8_t));
                memcpy(finalLineIdx, lineIdx, 8 * sizeof(uint8_t));
            }
        }
    }

    // Line & Corner Estimates using canidate points post outlier removal
    GetLineAfterRansac(sv, prms, optLineIdxArr, NaNslope, slope, icpt, XValue, numPts, 0);
    GetLineAfterRansac(sv, prms, optLineIdxArr, NaNslope, slope, icpt, XValue, numPts, 1);
    GetLineAfterRansac(sv, prms, optLineIdxArr, NaNslope, slope, icpt, XValue, numPts, 2);
    GetLineAfterRansac(sv, prms, optLineIdxArr, NaNslope, slope, icpt, XValue, numPts, 3);

    svGetCornerFromLine(tempCorner, NaNslope, slope, icpt, XValue, 3, 0, 0);
    svGetCornerFromLine(tempCorner, NaNslope, slope, icpt, XValue, 0, 1, 1);
    svGetCornerFromLine(tempCorner, NaNslope, slope, icpt, XValue, 1, 2, 2);
    svGetCornerFromLine(tempCorner, NaNslope, slope, icpt, XValue, 2, 3, 3);

    memcpy(corner, tempCorner, sizeof(double) * 8);
    
    // corners remapped to fisheye images
    for (i = 0; i < 4; i++)
    {
        // LDC
        point_u[0] = (dtype)tempCorner[i * 2 + 1];
        point_u[1] = (dtype)tempCorner[i * 2 + 0];

        status = LDC_UndistToDist(ldc, point_u, point_d);
        corner[i * 2] = (double)point_d[1];
        corner[i * 2 + 1] = (double)point_d[0];

        if (status == LDC_STATUS_FAIL)
        {
#ifdef PC_VERSION
            printf("WARNING: LDC stepped out of bounds");
#else
            VX_PRINT(VX_ZONE_ERROR, "WARNING: LDC stepped out of bounds\n");
#endif
        }
    }

    free(LineIdxArr);

    return minAvgError;
}

static unsigned long next = 1;
/* RAND_MAX assumed to be 32767 */
static uint32_t myrandPE(void) {
    next = next * 1103515245 + 12345;
    return((uint32_t)(next / 65536) % 32768);
}

 void svRandomPerm(int32_t max_index, int32_t *array_in, int32_t count, int32_t *array_out)
{
    int32_t i, randNo;
    int32_t temp;
    int random;
    for (i = 0; i < max_index; i++)
        array_in[i] = i;
    for (i = max_index - 1; i > max_index - count - 1; i--) {
        random = myrandPE();
        randNo = (int32_t)random % (i + 1);
        array_out[max_index - i - 1] = array_in[randNo];
        //swap indices[rand] with indices[i]
        temp = array_in[randNo];
        array_in[randNo] = array_in[i];
        array_in[i] = temp;
    }
}


/*===============================================================================
*
* Name:       svAssociatePointsToLine()
*
* Description: Associates each boundary point to one of four lines or labels as outlier
*
*
* Input:
*   sv:                   Handle for the SV structure
*   errThr:               Defines what classifies as an outlier         
*   candLineIdx:          Array with possible canidate points for each line
*   LineIdxArr:           Pointer for storing information correlating each point to a line
*   NumOutliers:          Pointer for storing the number of outliers
*
===============================================================================*/

 double svAssociatePointsToLine(svPointDetect_t* sv,tivxPointDetectParams *prms, uint8_t* NaNslope, double* slope, double* icpt, double* xIcpt, int16_t totalPoints, uint8_t errTh, uint8_t candLineIdx[128][4], uint8_t* LineIdxArr, uint8_t* numOutlier)
{
    double avgError = 0;
    double error = 0;
    double minError;
    double *boundX = prms->buf_boundX_ptr;
    double *boundY = prms->buf_boundY_ptr;

    int outliers = 0; 
    uint8_t i, j, bHorDist, lineIdx;

    for (i = 0; i < totalPoints; i++)
    {
        minError = 100000;
        for (j = 0; j < 4; j++)
        {
            if (j == 1 || j == 3)
                bHorDist = 1;
            else
                bHorDist = 0;
            if (candLineIdx[i][j] == 1)
            {
                error = Calculate1DErrorForPoint(boundX[i], boundY[i], NaNslope[j], slope[j], icpt[j], xIcpt[j], bHorDist);
                if (error < minError)
                {
                    lineIdx = j + 1;
                    minError = error;
                }
            }
        }
        avgError = avgError + minError;
        if (minError >(double) errTh)
        {
            LineIdxArr[i] = 0;
            outliers++;
        }
        else
            LineIdxArr[i] = lineIdx;
    }

    *numOutlier = outliers;
    avgError = avgError / totalPoints;

    return avgError;
}


 double Calculate1DErrorForPoint(double ptx, double pty, uint8_t NaNslope, double slope, double icpt, double xIcpt, uint8_t bHorDist)
{
    double errLine = 0;
    double est;

    if (bHorDist)
    {
        if (NaNslope)
            est = xIcpt;
        else
        {
            if (slope == 0)
                est = ptx;
            else
                est = (pty - icpt) / slope;
        }
        errLine = abs(est - ptx);
    }
    else
    {
        est = slope * ptx + icpt;
        errLine = abs(est - pty);
    }

    return errLine;
}

/*===============================================================================
*
* Name:        svGetLineFrom2Pts()
*
* Description: Estimates line givin 2 points
*
*
* Input:
*   sv:             Handle for the SV structure
*   NaNSlope:       Pointer for storing NaNslope (1 or 0)
*   Slope:          Pointer for storing line slope
*   icpt:           Pointer for storing Y intercept
*   xIcpt:          Pointer for storing X intercept
*   lineIdx:        Line ID (1-4)
*
===============================================================================*/

 void svGetLineFrom2Pts(svPointDetect_t* sv,tivxPointDetectParams *prms, uint8_t* NaNslope, double* slope, double* icpt, double* xIcpt, uint8_t lineIdx)
{
    *NaNslope = 0;
    *xIcpt = 0;
    *slope = 0;
    *icpt = 0;
    double *line1PtsX = prms->buf_line1PtsX_ptr;
    double *line2PtsX = prms->buf_line2PtsX_ptr;
    double *line1PtsY = prms->buf_line1PtsY_ptr;
    double *line2PtsY = prms->buf_line2PtsY_ptr;

    if (abs(line1PtsX[lineIdx] - line2PtsX[lineIdx]) <= 0.1)
        *NaNslope = 1;

    if (*NaNslope == 0)
    {
        *slope = (line2PtsY[lineIdx] - line1PtsY[lineIdx]) / (line2PtsX[lineIdx] - line1PtsX[lineIdx]);
        *icpt = line1PtsY[lineIdx] - (slope[0] * line1PtsX[lineIdx]);
    }
    else
        *xIcpt = line1PtsX[lineIdx];

}


 double GetLineAfterRansac(svPointDetect_t* sv, tivxPointDetectParams *prms, uint8_t* optLineIdxArr, uint8_t* NaNSlope, double* slope, double* icpt, double* XValue, uint8_t totalPoints, uint8_t lineIdx)
{
    uint8_t numPtLine = 0;
    uint8_t i;
    uint8_t bHorDist = 0;
    double errLine;
    double *linePtsX = prms->buf_line1PtsX_ptr;
    double *linePtsY = prms->buf_line1PtsY_ptr;
    double *fcBoundX = prms->buf_fcBoundX_ptr;
    double *fcBoundY = prms->buf_fcBoundY_ptr;

    for (i = 0; i < totalPoints; i++)
        if (optLineIdxArr[i] == lineIdx + 1)
        {
            linePtsX[numPtLine] = fcBoundX[i];
            linePtsY[numPtLine] = fcBoundY[i];
            numPtLine++;
        }

    if (lineIdx == 1 || lineIdx == 3)
        bHorDist = 1;

    errLine = svGetLineFitError(linePtsY, linePtsX, &NaNSlope[lineIdx], &XValue[lineIdx], &slope[lineIdx], &icpt[lineIdx], numPtLine, bHorDist);
    
    return errLine;
}


 double svGetLineFitError(double* linePtsY, double* linePtsX, uint8_t* NaNSlope, double* XValue, double* slope, double* icpt, int16_t numPtLine,  uint8_t bHorDist)
{
    int16_t i;

    double sumXY = 0.0, sumXsq = 0.0, sumX = 0.0, sumY = 0.0;
    double estX, estY, errLine;

    *NaNSlope = 0; *XValue = 0; *slope = 0; *icpt = 0;
    errLine = 0;

    for (i = 0; i < numPtLine; i++)
    {
        sumXY += linePtsX[i]*linePtsY[i];
        sumXsq += linePtsX[i]*linePtsX[i];
        sumX += linePtsX[i];
        sumY += linePtsY[i];
    }

    if (numPtLine * sumXsq - sumX * sumX == 0)
        *NaNSlope = 1;

    if (*NaNSlope) {
        *XValue = sumX / numPtLine;
    } else {
        *slope = (numPtLine * sumXY - sumX * sumY) / (numPtLine * sumXsq - sumX * sumX);
        *icpt = (sumY - *slope * sumX) / numPtLine;
    }

    for (i = 0; i < numPtLine; i++)
    {
        if (bHorDist) {
            // y = ax + b => x = (y-b) / a
            if (*NaNSlope) {
                estX = *XValue;
            } else {
                if (*slope == 0)
                    estX = linePtsX[i];
                else
                    estX = (linePtsY[i] - *icpt) / *slope;
            }

            errLine += fabs(estX - linePtsX[i]);

        } else {
            // y = ax + b
            estY = *slope * linePtsX[i] + *icpt;
            errLine += fabs(estY - linePtsY[i]);
        }
    }

    return errLine;
}


 void svGetCornerFromLine(double* corner, uint8_t* NaNSlope, double* slope, double*icpt, double* XValue, uint8_t lineIdx1st, uint8_t lineIdx2nd, uint8_t cornerIdx)
{
    uint8_t tempNaNSlope1, tempNaNSlope2;
    double tempSlope1, tempSlope2, tempIcpt1, tempIcpt2, tempXValue1, tempXValue2;

    tempNaNSlope1 = NaNSlope[lineIdx1st];
    tempNaNSlope2 = NaNSlope[lineIdx2nd];

    tempSlope1 = slope[lineIdx1st];
    tempSlope2 = slope[lineIdx2nd];

    tempIcpt1 = icpt[lineIdx1st];
    tempIcpt2 = icpt[lineIdx2nd];

    tempXValue1 = XValue[lineIdx1st];
    tempXValue2 = XValue[lineIdx2nd];


    if (tempNaNSlope1 == 1) {
        corner[2*cornerIdx+1] = tempXValue1;
        corner[2*cornerIdx] = tempSlope2 * tempXValue1 + tempIcpt2;
    } else if (tempNaNSlope2 == 1) {
        corner[2*cornerIdx+1] = tempXValue2;
        corner[2*cornerIdx] = tempSlope1 * tempXValue2 + tempIcpt1;

    } else {
        corner[2*cornerIdx+1] = (tempIcpt2 - tempIcpt1) / (tempSlope1 - tempSlope2);
        corner[2*cornerIdx] = tempSlope1 * corner[2*cornerIdx+1] + tempIcpt1;
    }
}



/*===============================================================================
 *
 * Name:            svCheckFPCondition()
 *
 * Description:     check whether finder patterns are valid ones
 *
 *
 * Input:
 *   inputCenter:   detected finder patterns
 *
 * Returns:
 *
 * Effects:
 *
 ===============================================================================*/
 uint8_t svCheckFPCondition(svPointDetect_t* sv,tivxPointDetectParams *prms, double* inputCorner)
{
    double dist01, dist23, dist03, dist12, dist02, dist13;
    double hor01, hor23, ver03, ver12;
    int16_t  SVInCamFrmHeight = prms->img_prms.dim_y ;
    int16_t  SVInCamFrmWidth  = prms->img_prms.dim_x;

    /*  The ROI values defined are for 720P
        They are scaled to match the actual input resolution */
    uint32_t firstRoiTop = (sv->firstROITop * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t firstRoiBottom = (sv->firstROIBottom * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t firstRoiLeft = (sv->firstROILeft * SVInCamFrmWidth)/sv->SVROIWidth;
    uint32_t firstRoiRight = (sv->firstROIRight * SVInCamFrmWidth)/sv->SVROIWidth;

    uint32_t secondRoiTop = (sv->secondROITop * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t secondRoiBottom = (sv->secondROIBottom * SVInCamFrmHeight)/sv->SVROIHeight;
    uint32_t secondRoiLeft = (sv->secondROILeft * SVInCamFrmWidth)/sv->SVROIWidth;
    uint32_t secondRoiRight = (sv->secondROIRight * SVInCamFrmWidth)/sv->SVROIWidth;

    // 2 horizontal distance
    dist01 = sqrt(pow((double)(inputCorner[0*2] - inputCorner[1*2]), 2) + pow((double)(inputCorner[0*2 + 1] - inputCorner[1*2 + 1]), 2));
    dist23 = sqrt(pow((double)(inputCorner[2*2] - inputCorner[3*2]), 2) + pow((double)(inputCorner[2*2 + 1] - inputCorner[3*2 + 1]), 2));

    // 2 vertical distance
    dist03 = sqrt(pow((double)(inputCorner[0*2] - inputCorner[3*2]), 2) + pow((double)(inputCorner[0*2 + 1] - inputCorner[3*2 + 1]), 2));
    dist12 = sqrt(pow((double)(inputCorner[1*2] - inputCorner[2*2]), 2) + pow((double)(inputCorner[1*2 + 1] - inputCorner[2*2 + 1]), 2));

    // 2 diagonal distance
    dist02 = sqrt(pow((double)(inputCorner[0*2] - inputCorner[2*2]), 2) + pow((double)(inputCorner[0*2 + 1] - inputCorner[2*2 + 1]), 2));
    dist13 = sqrt(pow((double)(inputCorner[1*2] - inputCorner[3*2]), 2) + pow((double)(inputCorner[1*2 + 1] - inputCorner[3*2 + 1]), 2));

    // check horizontal length condition
    if (dist01 > MAX_HOR_LENGTH_RATIO*dist23 || dist23 > MAX_HOR_LENGTH_RATIO*dist01)
        return 0;

    // check vertical length condition
    if (dist03 > MAX_VER_LENGTH_RATIO*dist12 || dist12 > MAX_VER_LENGTH_RATIO*dist03)
        return 0;

    // check diagonal length condition
    if (dist02 > MAX_DIA_LENGTH_RATIO*dist13 || dist13 > MAX_DIA_LENGTH_RATIO*dist02)
        return 0;


    // Check boundary length
    hor01 = inputCorner[1*2 + 1] - inputCorner[0*2 + 1];
    hor23 = inputCorner[2*2 + 1] - inputCorner[3*2 + 1];
    ver03 = inputCorner[3*2] - inputCorner[0*2];
    ver12 = inputCorner[2*2] - inputCorner[1*2];

    if (hor01 < MIN_HOR_LENGTH || hor23 < MIN_HOR_LENGTH || ver03 < MIN_VER_LENGTH || ver12 < MIN_VER_LENGTH)
        return 0;

    if (hor01 + hor23 + ver03 + ver12 < MIN_CHART_LENGTH)
        return 0;


    // check ROI
    if ((inputCorner[0*2 + 1] <  firstRoiLeft  || inputCorner[0*2 + 1] > firstRoiRight  || inputCorner[0*2] < firstRoiTop  || inputCorner[0*2] > firstRoiBottom) &&
        (inputCorner[0*2 + 1] <  secondRoiLeft || inputCorner[0*2 + 1] > secondRoiRight || inputCorner[0*2] < secondRoiTop || inputCorner[0*2] > secondRoiBottom))
        return 0;

    if ((inputCorner[1*2 + 1] <  firstRoiLeft  || inputCorner[1*2 + 1] > firstRoiRight  || inputCorner[1*2] < firstRoiTop  || inputCorner[1*2] > firstRoiBottom) &&
        (inputCorner[1*2 + 1] <  secondRoiLeft || inputCorner[1*2 + 1] > secondRoiRight || inputCorner[1*2] < secondRoiTop || inputCorner[1*2] > secondRoiBottom))
        return 0;

    if ((inputCorner[2*2 + 1] <  firstRoiLeft  || inputCorner[2*2 + 1] > firstRoiRight  || inputCorner[2*2] < firstRoiTop  || inputCorner[2*2] > firstRoiBottom) &&
        (inputCorner[2*2 + 1] <  secondRoiLeft || inputCorner[2*2 + 1] > secondRoiRight || inputCorner[2*2] < secondRoiTop || inputCorner[2*2] > secondRoiBottom))
        return 0;

    if ((inputCorner[3*2 + 1] <  firstRoiLeft  || inputCorner[3*2 + 1] > firstRoiRight  || inputCorner[3*2] < firstRoiTop  || inputCorner[3*2] > firstRoiBottom) &&
        (inputCorner[3*2 + 1] <  secondRoiLeft || inputCorner[3*2 + 1] > secondRoiRight || inputCorner[3*2] < secondRoiTop || inputCorner[3*2] > secondRoiBottom))
        return 0;


    return 1;
}


 void svGetInitialLines(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inY, double* inX, double* fcY, double* fcX, int16_t numPts, int16_t* lineStart, int16_t* lineEnd)
{
    int16_t L0S, L0E, L2S, L2E;

    uint8_t i, idx, actIdx, iter, lineNumPts, interPtNum, initPtNum = 7; // 4;
    uint8_t tempLineEnd, tempLineStart, maxIter;
    double firstX, lastX;

    uint8_t tempNaNSlope;
    double minAbsSlope, minSlope, slopeDiff = 0.03;
#ifdef PC_VERSION
    double errLine;
#endif
    double tempSlope, tempIcpt, tempXValue;

    double *linePtsX  = prms->buf_line1PtsX_ptr;
    double *linePtsY  = prms->buf_line1PtsY_ptr;

    L0S = 41;
    L0E = 89;
    L2S = 105;
    L2E = 25;

    // initial starting points of line 0 and line2
    lineStart[0] = 63;
    lineEnd[0] = 69;
    lineStart[2] = 127;
    lineEnd[2] = 5;

    if (L0E < L0S) L0E = numPts + L0E;
    if (L2E < L2S) L2E = numPts + L2E;

    ////////////////// for Line0 //////////////////////////
    // 1. get initial 7 points
    minAbsSlope = minSlope = 5000000;
    for (iter = L0S; iter <= L0E - (initPtNum-1); iter++)
    {
        firstX = inX[iter % numPts];
        lastX = inX[(iter + initPtNum - 1) % numPts];
        if (abs(firstX - lastX) < 9)
            continue;

        for (i = 0; i <= initPtNum - 1; i++)
        {
            idx = iter+i;
            actIdx = idx % numPts;
            linePtsX[i] = fcX[actIdx];
            linePtsY[i] = fcY[actIdx];
        }
#ifdef PC_VERSION
        errLine = svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, initPtNum, 1);
#else
        svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, initPtNum, 1);
#endif
        if (tempNaNSlope == 0) {
            if (fabs(tempSlope) <= minAbsSlope)
            {
                minAbsSlope = fabs(tempSlope);
                minSlope = tempSlope;
                lineStart[0] = iter;
                lineEnd[0] = iter + initPtNum - 1;
            }
        }
    }

    // 2-1. refine points
    lineNumPts = 0;
    for (i = lineStart[0]; i <= L0E; i++)
    {
        actIdx = i % numPts;
        linePtsX[lineNumPts] = fcX[actIdx];
        linePtsY[lineNumPts] = fcY[actIdx];
        lineNumPts++;
    }

    maxIter = L0E - lineEnd[0];
    tempLineEnd = lineEnd[0];
    for (i = 1; i<= maxIter; i++)
    {
        lineNumPts = initPtNum + i;
#ifdef PC_VERSION
        errLine = svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#else
        svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#endif
        if (tempNaNSlope == 0) {
            if (fabs(minSlope - tempSlope) <= slopeDiff)
                lineEnd[0] = tempLineEnd + i;
            else
                break;
        }

    }

    // 2-2. refine points
    lineNumPts = 0;
    for (i = lineEnd[0]; i >= L0S; i--)
    {
        actIdx = i % numPts;
        linePtsX[lineNumPts] = fcX[actIdx];
        linePtsY[lineNumPts] = fcY[actIdx];
        lineNumPts++;
    }

    maxIter = lineStart[0] - L0S;
    tempLineStart = lineStart[0];
    interPtNum = lineEnd[0] - lineStart[0] - 1;
    for (i = 1; i<= maxIter; i++)
    {
        lineNumPts = interPtNum + i;
#ifdef PC_VERSION
        errLine = svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#else
        svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#endif
        if (tempNaNSlope == 0) {
            if (fabs(minSlope - tempSlope) <= slopeDiff)
                lineStart[0] = tempLineStart - i;
            else
                break;
        }
    }

    ////////////////// for Line2 //////////////////////////
    // 1. get initial 4 points
    minAbsSlope = minSlope = 5000000;
    for (iter = L2S; iter <= L2E - (initPtNum-1); iter++)
    {
        firstX = inX[iter % numPts];
        lastX = inX[(iter + initPtNum - 1) % numPts];
        if (abs(firstX - lastX) < 9)
            continue;

        for (i = 0; i <= initPtNum - 1; i++)
        {
            idx = iter+i;
            actIdx = idx % numPts;
            linePtsX[i] = fcX[actIdx];
            linePtsY[i] = fcY[actIdx];
        }

#ifdef PC_VERSION
        errLine = svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, initPtNum, 1);
#else
        svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, initPtNum, 1);
#endif
        if (tempNaNSlope == 0) {
            if (fabs(tempSlope) <= minAbsSlope)
            {
                minAbsSlope = fabs(tempSlope);
                minSlope = tempSlope;
                lineStart[2] = iter;
                lineEnd[2] = iter + initPtNum - 1;
            }
        }
    }

    // 2-1. refine points
    lineNumPts = 0;
    for (i = lineStart[2]; i <= L2E; i++)
    {
        actIdx = i % numPts;
        linePtsX[lineNumPts] = fcX[actIdx];
        linePtsY[lineNumPts] = fcY[actIdx];
        lineNumPts++;
    }

    maxIter = L2E - lineEnd[2];
    tempLineEnd = lineEnd[2];
    for (i = 1; i<= maxIter; i++)
    {
        lineNumPts = initPtNum + i;
#ifdef PC_VERSION
        errLine = svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#else
        svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#endif
        if (tempNaNSlope == 0) {
            if (fabs(minSlope - tempSlope) <= slopeDiff)
                lineEnd[2] = tempLineEnd + i;
            else
                break;
        }

    }

    // 2-2. refine points
    lineNumPts = 0;
    for (i = lineEnd[2]; i >= L2S; i--)
    {
        actIdx = i % numPts;
        linePtsX[lineNumPts] = fcX[actIdx];
        linePtsY[lineNumPts] = fcY[actIdx];
        lineNumPts++;
    }

    maxIter = lineStart[2] - L2S;
    tempLineStart = lineStart[2];
    interPtNum = lineEnd[2] - lineStart[2] - 1;
    for (i = 1; i<= maxIter; i++)
    {
        lineNumPts = interPtNum + i;
#ifdef PC_VERSION
        errLine = svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#else
        svGetLineFitError(linePtsY, linePtsX, &tempNaNSlope, &tempXValue, &tempSlope, &tempIcpt, lineNumPts, 1);
#endif
        if (tempNaNSlope == 0) {
            if (fabs(minSlope - tempSlope) <= slopeDiff)
                lineStart[2] = tempLineStart - i;
            else
                break;
        }
    }

    // warped index
    lineStart[0] = lineStart[0] % numPts;
    lineEnd[0] = lineEnd[0] % numPts;
    lineStart[2] = lineStart[2] % numPts;
    lineEnd[2] = lineEnd[2] % numPts;


}


 double svRefineFPCorner(svPointDetect_t* sv,tivxPointDetectParams *prms, double* inY, double* inX, double *fcY, double* fcX, double* corner, int16_t numPts, int16_t* lineStart, int16_t* lineEnd, LensDistortionCorrection *ldc, uint8_t bFisheye)
{
    int16_t i;
#if 0
    int16_t lineStart[4], lineEnd[4];
#endif

    uint8_t NaNSlope[4];
    double slope[4], icpt[4], XValue[4];
    double totalMinError, minError[4];

    double* tempCorner = prms->buf_tempCorner_ptr;

    // LDC
    dtype point_u[2], point_d[2];
    LDC_status status;

    //////////////////////////////////////////////////
    //                 line0
    //          __________________
    //         |                 |
    //  line3  |                 |  line1
    //         |                 |
    //         |_________________|
    //                  line2
    //////////////////////////////////////////////////

    // initial points of each line
    lineStart[1] = (lineEnd[0] + 1 + numPts) % numPts;
    lineEnd[1] = (lineStart[2] - 1 + numPts) % numPts;

    lineStart[3] = (lineEnd[2] + 1 + numPts) % numPts;
    lineEnd[3] = (lineStart[0] - 1 + numPts) % numPts;

    // stopping rule should be added
    for (i = 0; i < 5; i++)
    {
        minError[0] = svUpdateBoundaryPoints(sv,prms, inY, inX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 3, 0, 1, 0);
        minError[1] = svUpdateBoundaryPoints(sv,prms, inY, inX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 0, 1, 0, 1);
        minError[2] = svUpdateBoundaryPoints(sv,prms, inY, inX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 1, 2, 1, 0);
        minError[3] = svUpdateBoundaryPoints(sv,prms, inY, inX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 2, 3, 0, 1);
        #ifdef DEBUG_2
        //Debug point for failures, print  lineStart and line end
        printf("lineStart  =%d %d %d %d \n",lineStart[0],lineStart[1],lineStart[2],lineStart[3]);
        printf("lineEnd  =%d %d %d %d \n",lineEnd[0],lineEnd[1],lineEnd[2],lineEnd[3]);
        printf("minErrors = %f %f %f %f \n",minError[0],minError[1],minError[2],minError[3]);
        printf("NanSlopes = %d %d %d %d \n",NaNSlope[0],NaNSlope[1], NaNSlope[2], NaNSlope[3]);
        #endif
    }


    totalMinError = minError[0] + minError[1] + minError[2] + minError[3];

    if (bFisheye == 1)
    {
        // get lines after fisheye correction and calculate errors
        minError[0] = svGetLine(sv, prms, fcY, fcX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 0, 0);
        minError[1] = svGetLine(sv, prms, fcY, fcX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 1, 1);
        minError[2] = svGetLine(sv, prms, fcY, fcX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 2, 0);
        minError[3] = svGetLine(sv, prms, fcY, fcX, numPts, lineStart, lineEnd, NaNSlope, slope, icpt, XValue, 3, 1);

        totalMinError = minError[0] + minError[1] + minError[2] + minError[3];
    }

    // Get corners from adjacent lines
    svGetCornerFromLine(tempCorner, NaNSlope, slope, icpt, XValue, 3, 0, 0);
    svGetCornerFromLine(tempCorner, NaNSlope, slope, icpt, XValue, 0, 1, 1);
    svGetCornerFromLine(tempCorner, NaNSlope, slope, icpt, XValue, 1, 2, 2);
    svGetCornerFromLine(tempCorner, NaNSlope, slope, icpt, XValue, 2, 3, 3);

    // corners remapped to fisheye images
    for (i = 0; i < 4; i++)
    {
        // LDC
        point_u[0] = (dtype) tempCorner[i*2 +1];
        point_u[1] = (dtype) tempCorner[i*2 +0];

        status = LDC_UndistToDist(ldc, point_u, point_d);
        corner[i*2]     = (double) point_d[1];
        corner[i*2 + 1] = (double) point_d[0];

        if (status==LDC_STATUS_FAIL)
        {
#ifdef PC_VERSION
            printf("WARNING: LDC stepped out of bounds");
#else
            VX_PRINT(VX_ZONE_ERROR, "WARNING: LDC stepped out of bounds\n");
#endif
        }
    }

    return totalMinError;
}

 double svUpdateBoundaryPoints(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inY, double* inX, int16_t numPts, int16_t* startPos, int16_t* endPos, uint8_t* NaNSlope, double* slope, double* icpt, double* XValue, uint8_t lineIdx1st, uint8_t lineIdx2nd, uint8_t bHorDist1, uint8_t bHorDist2)
{
    int16_t i;
    int16_t line1s, line1e, line2e;

    uint8_t tempNaNSlope1, tempNaNSlope2;
    double tempSlope1, tempSlope2, tempIcpt1, tempIcpt2, tempXValue1, tempXValue2;

    int16_t initNumPtLine1 = 0, numPtLine1, numPtLine2, numTotalPts;
#ifdef PC_VERSION
    int16_t line2s;
    int16_t initNumPtLine2;
#endif
    int16_t idx, actIdx;

    uint8_t bContinue;

    double totalError, minTotalError, errLine1, errLine2;

    double *line1PtsX  = prms->buf_line1PtsX_ptr;
    double *line1PtsY  = prms->buf_line1PtsY_ptr;
    double *line2PtsX  = prms->buf_line2PtsX_ptr;
    double *line2PtsY  = prms->buf_line2PtsY_ptr;

    line1s = startPos[lineIdx1st]; line1e = endPos[lineIdx1st];
#ifdef PC_VERSION
    line2s = startPos[lineIdx2nd]; line2e = endPos[lineIdx2nd];
#else
    line2e = endPos[lineIdx2nd];
#endif

    // gather all data from line1s to line2e
    numPtLine1 = 0;
    idx = line1s;
    actIdx = (idx + numPts) % numPts;
    line1PtsX[numPtLine1] = inX[actIdx];
    line1PtsY[numPtLine1] = inY[actIdx];
    numPtLine1++;

    if (actIdx == line1e) initNumPtLine1 = numPtLine1;

    bContinue = 1;
    if (actIdx == line2e) bContinue = 0;

    while (bContinue) {
        idx++;
        actIdx = (idx + numPts) % numPts;
        line1PtsX[numPtLine1] = inX[actIdx];
        line1PtsY[numPtLine1] = inY[actIdx];
        numPtLine1++;

        if (actIdx == line1e) initNumPtLine1 = numPtLine1;
        if (actIdx == line2e) bContinue = 0;
    }


    // gather all data from line2e to line1s
    numPtLine2 = 0;
    idx = line2e;
    actIdx = (idx + numPts) % numPts;
    line2PtsX[numPtLine2] = inX[actIdx];
    line2PtsY[numPtLine2] = inY[actIdx];
    numPtLine2++;
#ifdef PC_VERSION
    if (actIdx == line2s) initNumPtLine2 = numPtLine2;
#endif

    bContinue = 1;
    if (actIdx == line1s) bContinue = 0;

    while (bContinue) {
        idx--;
        actIdx = (idx + numPts) % numPts;
        line2PtsX[numPtLine2] = inX[actIdx];
        line2PtsY[numPtLine2] = inY[actIdx];
        numPtLine2++;
#ifdef PC_VERSION
        if (actIdx == line2s) initNumPtLine2 = numPtLine2;
#endif
        if (actIdx == line1s) bContinue = 0;
    }

    // Now we update two lines
    numTotalPts = numPtLine1;
    minTotalError = 5000000;

    // may need to add early stop
    for (i = -24; i <= 24; i++) {
        numPtLine1 = initNumPtLine1 + i;
        numPtLine2 = numTotalPts - numPtLine1;

        // estimate line 1
        if (numPtLine1 > 4 && numPtLine2 > 4) {
            errLine1 = svGetLineFitError(line1PtsY, line1PtsX, &tempNaNSlope1, &tempXValue1, &tempSlope1, &tempIcpt1, numPtLine1, bHorDist1);

        } else {
            tempNaNSlope1 = 0;
            tempXValue1 = 0;
            tempSlope1 = 0;
            tempIcpt1 = 0;
            errLine1 = 5000000;
        }

        // estimate line 2
        if (numPtLine1 > 4 && numPtLine2 > 4) {
            errLine2 = svGetLineFitError(line2PtsY, line2PtsX, &tempNaNSlope2, &tempXValue2, &tempSlope2, &tempIcpt2, numPtLine2, bHorDist2);

        } else {
            tempNaNSlope2 = 0;
            tempXValue2 = 0;
            tempSlope2 = 0;
            tempIcpt2 = 0;
            errLine2 = 5000000;
        }

        totalError = errLine1 + errLine2;
        if (totalError <= minTotalError)
        {
            minTotalError = totalError;

            slope[lineIdx1st] = tempSlope1; icpt[lineIdx1st] = tempIcpt1; NaNSlope[lineIdx1st] = tempNaNSlope1; XValue[lineIdx1st] = tempXValue1;
            slope[lineIdx2nd] = tempSlope2; icpt[lineIdx2nd] = tempIcpt2; NaNSlope[lineIdx2nd] = tempNaNSlope2; XValue[lineIdx2nd] = tempXValue2;

            endPos[lineIdx1st] = (line1s + numPtLine1 - 1 + numPts) % numPts;
            startPos[lineIdx2nd] = (line1s + numPtLine1 + numPts) % numPts;
        }

    }

    return minTotalError;
}

 double svGetLine(svPointDetect_t* sv, tivxPointDetectParams *prms, double* inY, double* inX, int16_t numPts, int16_t* startPos, int16_t* endPos, uint8_t* NaNSlope, double* slope, double* icpt, double* XValue, uint8_t lineIdx, uint8_t bHorDist)
{
    int16_t lineStart, lineEnd;
    int16_t idx, actIdx, numPtLine;

    uint8_t bContinue;

    double errLine;

    double *linePtsX  = prms->buf_line1PtsX_ptr;
    double *linePtsY  = prms->buf_line1PtsY_ptr;

    lineStart = startPos[lineIdx]; lineEnd = endPos[lineIdx];

    // gather all data from lineStart to lineEnd
    numPtLine = 0;
    idx = lineStart;
    actIdx = (idx + numPts) % numPts;
    linePtsX[numPtLine] = inX[actIdx];
    linePtsY[numPtLine] = inY[actIdx];
    numPtLine++;

    bContinue = 1;
    if (actIdx == lineEnd) bContinue = 0;

    while (bContinue) {
        idx++;
        actIdx = (idx + numPts) % numPts;
        linePtsX[numPtLine] = inX[actIdx];
        linePtsY[numPtLine] = inY[actIdx];
        numPtLine++;

        if (actIdx == lineEnd) bContinue = 0;
    }


    errLine = svGetLineFitError(linePtsY, linePtsX, &NaNSlope[lineIdx], &XValue[lineIdx], &slope[lineIdx], &icpt[lineIdx], numPtLine, bHorDist);

    return errLine;
}


