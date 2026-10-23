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


/** @file tidl_image_preproc.c
*
*  @brief  This file contains image read related
*       test code for TI DL test application
*
*
*  @date  Aug 2017
*
*/


#if (HOST_EMULATION)
#ifdef BUILD_WITH_OPENCV
#define USE_OPENCV (1)
#else
#define USE_OPENCV (0)
#endif
#else
#define USE_OPENCV (0)
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "tidl_config.h"
#include "itidl_ti.h"
extern sTIDL_IOBufDesc_t  gIOParams;

#if USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#ifndef _WIN32
#include <opencv2/imgcodecs/legacy/constants_c.h>
#endif
#include <iostream>
#include  <cstring>

using namespace cv;
using namespace std;

#define TIDL_RAW_IMG_BGR_PLANAR (0)
#define TIDL_RAW_IMG_RGB_PLANAR (1)

static Mat image;
static Mat spl[3];
static Mat tempImage;

int32_t readResizeCrop(uint8_t *imageFile, uint8_t *data, int32_t resizeWidth, int32_t resizeHeight, int32_t cropWidth, int32_t cropHeight, int32_t outFormat, int32_t inResizeType, int32_t numChs)
{
  std::string fileName = (char*)imageFile;
  if(numChs == 1)
  {
    image = cv::imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);  // Read the file for single channel input
  }
  else
  {
    image = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);  // Read the file for input with 3 channels
  }

  if (image.empty() == true)
  {
    printf("Error in reading %s, %s\n", fileName.c_str(), imageFile);
    exit(1);
  }

  int32_t i0;
  if (TIDL_RAW_IMG_RGB_PLANAR == outFormat)
  {
    if(numChs != 1)  // no conversion needed in case of single channel in the input image
    {
      cv::cvtColor(image, image, CV_BGR2RGB);
    }
  }

  if (inResizeType == TIDL_inResizeTypeKeepAR)
  {
    float  img_width = image.cols;
    float  img_height = image.rows;
    float aspect = img_width / img_height ;
    if (aspect > 1)
    {
      resizeWidth = (aspect * resizeWidth);
    }
    else if(aspect < 1)
    {
      resizeHeight = (resizeHeight / aspect);
    }
  }

  int32_t halfWidth = resizeWidth / 2;
  int32_t halfHeight = resizeHeight / 2;

  int32_t startX = halfWidth - cropWidth / 2;
  int32_t startY = halfHeight - cropHeight / 2;

  cv::Rect myROI(startX, startY,cropWidth, cropHeight);
  cv::resize(image, tempImage, Size(resizeWidth, resizeHeight), 0, 0, cv::INTER_AREA);
  image = tempImage(myROI);

  if(image.channels() != numChs)
  {
    printf("Number of channels in input buffer as given by IO buffer descriptor differs from number of channels in the actual image \n");
    exit(1);
  }

  split(image, spl);
  for (i0 = 0; i0 < image.channels(); i0++)
  {
    std::memcpy(&data[i0*((int)image.total())], spl[i0].data, ((int)image.total()) * sizeof(uint8_t));
  }

  for (i0 = image.channels(); i0 < numChs; i0++)
  {
    std::memcpy(&data[i0*((int)image.total())], &data[0], ((int)image.total()) * sizeof(uint8_t));
  }
  return (1);
}
#endif
uint8_t *tidl_loadBMP(const char *filepath, int32_t *width, int32_t *height);
void tidl_imageResize(uint8_t *pixels, uint8_t *ptr, int32_t inWidth, int32_t inHeight, int32_t outWidth, int32_t outHeight, int32_t mode, int32_t numChs);
int32_t tidl_imageCrop(uint8_t *pixels, uint8_t *ptr, int32_t width, int32_t height, int32_t startX, int32_t startY, int32_t outWidth, int32_t outHeight, int32_t numChs);
int32_t tidl_imagePlanarToInter(uint8_t *pixels, int32_t width, int32_t height, int32_t outFormat);
int32_t tidl_imageInterToPlanar(uint8_t * pixels, int32_t width, int32_t height, int32_t outFormat);

int32_t readBMPResizeCrop(uint8_t *imageFile, uint8_t *data, int32_t resizeWidth, int32_t resizeHeight, int32_t cropWidth, int32_t cropHeight, int32_t outFormat, int32_t inResizeType, int32_t numChs, int32_t inResizeMode)
{
  int32_t img_width, img_height;

  uint8_t *rdPtr;
  rdPtr = tidl_loadBMP((const char *)imageFile, &img_width, &img_height);
  if (rdPtr == NULL)
  {
    printf("Error in reading %s\n", imageFile);
    exit(1);
  }
  tidl_imageInterToPlanar(rdPtr, img_width, img_height, outFormat);

  if (inResizeType == TIDL_inResizeTypeKeepAR)
  {
    float aspect = (1.0 * img_width) / (1.0* img_height) ;
    if (aspect > 1)
    {
      resizeWidth = (aspect * resizeWidth);
    }
    else if(aspect < 1)
    {
      resizeHeight = (resizeHeight / aspect);
    }
  }

  int32_t halfWidth = resizeWidth / 2;
  int32_t halfHeight = resizeHeight / 2;
  int32_t startX = halfWidth - cropWidth / 2;
  int32_t startY = halfHeight - cropHeight / 2;
  int32_t imreadChannels = numChs;
  uint8_t *tempImage = (uint8_t *)malloc(imreadChannels * resizeWidth * resizeHeight);
  tidl_imageResize(rdPtr, tempImage, img_width, img_height, resizeWidth, resizeHeight, inResizeMode, numChs);
  tidl_imageCrop(tempImage, data, resizeWidth, resizeHeight, startX, startY,  cropWidth, cropHeight, numChs);
  free(rdPtr);
  free(tempImage);
  return (1);
}


int32_t readImage(uint8_t *imageFile, uint8_t *ptr, int32_t offset, int16_t n, int16_t width, int16_t height, int32_t inBufIdx,  int32_t inResizeMode)
{
  int32_t resizeWidth, resizeHeight, cropWidth, cropHeight, outFormat, inResizeType;

  uint8_t inExt[4];
  int32_t bmpFile = 0;
  strcpy((char *)inExt, (const char *)&imageFile[strlen((const char *)imageFile) - 3]);
  if(strcmp((const char *)inExt,"bmp") == 0  || (strcmp((const char *)inExt,"BMP") == 0))
  {
    bmpFile = 1;
  }
  cropWidth     = width;
  cropHeight    = height;
  outFormat     = gIOParams.inDataFormat[inBufIdx];
  resizeWidth   = gIOParams.resizeWidth[inBufIdx];
  resizeHeight  = gIOParams.resizeHeight[inBufIdx];
  /* gIOParams.inResizeType */
  inResizeType = gIOParams.inResizeType[inBufIdx];
  if(bmpFile == 1)
  {
    readBMPResizeCrop(imageFile, ptr, resizeWidth, resizeHeight, cropWidth, cropHeight, outFormat, inResizeType, n, inResizeMode);
  }
  else
  {
#if USE_OPENCV
    readResizeCrop(imageFile, ptr, resizeWidth, resizeHeight, cropWidth, cropHeight, outFormat, inResizeType, n);
#else
    printf("%s file read not supported. Only BMP file read is supported in target \n", inExt);
    return (-1);
#endif
  }
  return 1;
}



