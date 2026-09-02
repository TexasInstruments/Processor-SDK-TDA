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
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "tidl_config.h"
#include "ti_draw_utils.h"
#include "itidl_ti.h"
#include <float.h>
extern sTIDL_IOBufDesc_t  gIOParams;

#if USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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

int32_t readInImage(uint8_t *imageFile, uint8_t **r, uint8_t **g, uint8_t **b, int32_t * width, int32_t * height)
{
  std::string fileName = (char*)imageFile;
  image = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);  // Read the file
  if (image.empty() == true)
  {
    printf("Error in reading %s, %s\n", fileName.c_str(), imageFile);
    return(1);
  }
  *height = image.rows;
  *width = image.cols;

  split(image, spl);
  *r = (uint8_t*)malloc((*width)*(*height));
  *g = (uint8_t*)malloc((*width)*(*height));
  *b = (uint8_t*)malloc((*width)*(*height));

  std::memcpy(*b, spl[0].data, ((int)image.total()) * sizeof(uint8_t));
  std::memcpy(*g, spl[1].data, ((int)image.total()) * sizeof(uint8_t));
  std::memcpy(*r, spl[2].data, ((int)image.total()) * sizeof(uint8_t));

  return (1);
}

int32_t readandResizeInImage(uint8_t *imageFile, uint8_t **r, uint8_t **g, uint8_t **b, int32_t width, int32_t height)
{
  std::string fileName = (char*)imageFile;
  tempImage = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);  // Read the file
  if (tempImage.empty() == true)
  {
    printf("Error in reading %s, %s\n", fileName.c_str(), imageFile);
    return(1);
  }
  cv::resize(tempImage, image, Size(width, height), 0, 0, cv::INTER_AREA);

  split(image, spl);
  *r = (uint8_t*)malloc((width)*(height));
  *g = (uint8_t*)malloc((width)*(height));
  *b = (uint8_t*)malloc((width)*(height));

  std::memcpy(*b, spl[0].data, ((int)image.total()) * sizeof(uint8_t));
  std::memcpy(*g, spl[1].data, ((int)image.total()) * sizeof(uint8_t));
  std::memcpy(*r, spl[2].data, ((int)image.total()) * sizeof(uint8_t));

  return (1);
}


int32_t readandResizesegGt(uint8_t *imageFile, uint8_t *gt, int32_t width, int32_t height)
{
  std::string fileName = (char*)imageFile;
  tempImage = cv::imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);  // Read the file
  if (tempImage.empty() == true)
  {
    printf("Error in reading %s, %s\n", fileName.c_str(), imageFile);
    return(1);
  }
  cv::resize(tempImage, image, Size(width, height), 0, 0, cv::INTER_NEAREST);
  std::memcpy(gt, image.data, ((int)image.total()) * sizeof(uint8_t));
  return (1);
}

int32_t writeOutImage(uint8_t *imageFile, uint8_t *r, uint8_t *g, uint8_t *b, int32_t width, int32_t height)
{
  std::string fileName = (char*)imageFile;
  Mat image(height, width, CV_8UC3);
  vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(9);

  for (int i = 0; i < image.rows; ++i)
  {
    for (int j = 0; j < image.cols; ++j)
    {
      Vec3b& rgba = image.at<Vec3b>(i, j);
      rgba[0] = b[i*image.cols + j];
      rgba[1] = g[i*image.cols + j];
      rgba[2] = r[i*image.cols + j];
    }
  }

  imwrite(fileName, image, compression_params);
  return (1);
}
#endif
uint8_t *tidl_loadBMP(const char *filepath, int32_t *width, int32_t *height);
int32_t tidl_saveBMP(const char *filepath, uint8_t *pixels, int32_t width, int32_t height);
void tidl_imageResize(uint8_t *pixels, uint8_t *ptr, int32_t inWidth, int32_t inHeight, int32_t outWidth, int32_t outHeight, int32_t mode);
int32_t tidl_imageCrop(uint8_t *pixels, uint8_t *ptr, int32_t width, int32_t height, int32_t startX, int32_t startY, int32_t outWidth, int32_t outHeight);
int32_t tidl_imagePlanarToInter(uint8_t *pixels, int32_t width, int32_t height, int32_t outFormat);
int32_t tidl_imageInterToPlanar(uint8_t * pixels, int32_t width, int32_t height, int32_t outFormat);

int32_t readBMPResizeCrop(uint8_t *imageFile, uint8_t *data, int32_t resizeWidth, int32_t resizeHeight, int32_t cropWidth, int32_t cropHeight, int32_t outFormat, int32_t inResizeType, int32_t numChs, int32_t inResizeMode);


int32_t readInBMPImage(uint8_t *imageFile, uint8_t **r, uint8_t **g, uint8_t **b, int32_t * width, int32_t * height)
{
  int32_t img_width, img_height;

  uint8_t *rdPtr;
  rdPtr = tidl_loadBMP((const char *)imageFile, &img_width, &img_height);
  if (rdPtr == NULL)
  {
    printf("Error in reading %s\n", imageFile);
    return(1);
  }
  tidl_imageInterToPlanar(rdPtr, img_width, img_height, 0);
  *height = img_height;
  *width = img_width;

  *r = (uint8_t*)malloc((*width)*(*height));
  *g = (uint8_t*)malloc((*width)*(*height));
  *b = (uint8_t*)malloc((*width)*(*height));

  memcpy(*b, rdPtr, ((*width)*(*height) * sizeof(uint8_t)));
  memcpy(*g, &rdPtr[(*width)*(*height)], ((*width)*(*height)* sizeof(uint8_t)));
  memcpy(*r, &rdPtr[2*(*width)*(*height)], ((*width)*(*height)* sizeof(uint8_t)));
  free(rdPtr);

  return (1);
}

int32_t readandResizeInBMPImage(uint8_t *imageFile, uint8_t **r, uint8_t **g, uint8_t **b, int32_t width, int32_t height)
{
  uint8_t *data = (uint8_t*)malloc(3*(width*height));
  readBMPResizeCrop(imageFile, data, width, height, width, height, 0, 0, 3, 1);
  if (data == NULL)
  {
    printf("Error in allocating memory for BMP read %s\n", imageFile);
    return(1);
  }
  *r = (uint8_t*)malloc((width)*(height));
  *g = (uint8_t*)malloc((width)*(height));
  *b = (uint8_t*)malloc((width)*(height));

  memcpy(*b, data, ((width*height) * sizeof(uint8_t)));
  memcpy(*g, &data[(width*height)], ((width*height)* sizeof(uint8_t)));
  memcpy(*r, &data[2*(width*height)], ((width*height)* sizeof(uint8_t)));
  free(data);
  return (1);
}


int32_t readandResizeBMPsegGt(uint8_t *imageFile, uint8_t *gt, int32_t width, int32_t height)
{
  uint8_t *data = (uint8_t*)malloc(3*(width*height));
  readBMPResizeCrop(imageFile, data, width, height, width, height, 0, 0, 3, 1);
  if (data == NULL)
  {
    printf("Error in allocating memory for BMP read %s\n", imageFile);
    return(1);
  }

  memcpy(gt, data, ((width*height) * sizeof(uint8_t)));
  free(data);
  return (1);
}

int32_t writeOutBMPImage(uint8_t *imageFile, uint8_t *r, uint8_t *g, uint8_t *b, int32_t width, int32_t height)
{
  uint8_t *data = (uint8_t*)malloc(3*(width*height));
  memcpy((void*)data, b, ((width*height) * sizeof(uint8_t)));
  memcpy((void*)&data[(width*height)], g, ((width*height)* sizeof(uint8_t)));
  memcpy((void*)&data[2*(width*height)], r, ((width*height)* sizeof(uint8_t)));

  tidl_imagePlanarToInter(data, width, height, 0);
  tidl_saveBMP((const char *)imageFile, data, width, height);

  free(data);
  return (1);
}



static uint8_t IdtoColorMap[21][3] =
{
  { 0,255,0 },          //0	Bg
  { 0,255,0 },      //1	aeroplane     //TS
  { 255,0,0 },          //2	 bicycle // Person
  { 200,200,0 },     //3	bird       // Car
  { 0,0,142 },        //4	boat
  { 0,0,0 },          //5	bottle
  { 128,64,128 },     //6	bus
  { 0,0,200 },        //7	car
  { 250,170,30 },     //8	cat
  { 0,0,142 },        //9	chair
  { 0,0,0 },          //10	cow
  { 128,64,128 },     //11	diningtable
  { 220,20,60 },      //12	dog
  { 250,170,30 },     //13	horse
  { 255,0,0 },        //14	motorbike
  { 255,0,0 },         //15	person
  { 128,64,128 },     //16	pottedplant
  { 220,20,60 },      //17	sheep
  { 250,170,30 },     //18	sofa
  { 0,0,142 },        //19	train
  { 0,255,0 },        //20	tvmonitor
};
extern int32_t totalTestCount;
extern int32_t passTestCount;
extern int32_t top5passTestCount;
extern int64_t confusion_matrix[MAX_SEG_CLASS_IDX][MAX_SEG_CLASS_IDX];

int32_t getFileNameAndIdxFromList(char* listFile, int32_t frameCount, int32_t fileIdx, char* currFile);

char* getFileNameFromPath(char* path)
{
  int32_t i;
  for (i = strlen(path) - 1; i; i--)
  {
    if ((path[i] == '/') || (path[i] == '\\'))
    {
      return &path[i + 1];
    }
  }
  return path;
}

void getDirFromPath(char* path)
{
  int32_t i;
  for (i = strlen(path) - 1; i; i--)
  {
    if ((path[i] == '/') || (path[i] == '\\'))
    {
     path[i] = '\0';
     return;
    }
  }
  path[0] = '\0';
  return;
}

int32_t clip_offst(int32_t input, int32_t max)
{
  int32_t output;
  output = (input >= max) ? max - 1 : input;
  output = output < 0 ? 0 : output;
  return output;
}
#define MAX_SEG_COLOR_IDX (55)
uint8_t segIdtoColorMap[MAX_SEG_COLOR_IDX][3] = {
{ 255,	0,	0 },
{255,	23	,0   },
{ 255,	47	,0 },
{ 255,	71	,0 },
{ 255,	95	,0 },
{ 255,	119	,0 },
{ 255,	143	,0 },
{ 255,	167	,0 },
{ 255,	191	,0 },
{ 255,	215	,0 },
{ 255,	239	,0 },
{ 247,	255	,0 },
{ 223,	255	,0 },
{ 199,	255	,0 },
{ 175,	255	,0 },
{ 151,	255	,0 },
{ 127,	255	,0 },
{ 103,	255	,0 },
{ 79	,255	,0 },
{ 55	,255	,0 },
{ 31	,255	,0 },
{ 7	,255	,0 },
{ 0	,255	,15 },
{ 0	,255	,39 },
{ 0	,255	,63 },
{ 0	,255	,87 },
{ 0	,255	,111 },
{ 0	,255	,135 },
{ 0	,255	,159 },
{ 0	,255	,183 },
{ 0	,255	,207 },
{ 0	,255	,231 },
{ 0	,255	,255 },
{ 0	,231	,255 },
{ 0	,207	,255 },
{ 0	,183	,255 },
{ 0	,159	,255 },
{ 0	,135	,255 },
{ 0	,111	,255 },
{ 0	,87,	255 },
{ 0	,63,	255 },
{ 0	,39,	255 },
{ 0	,15,	255 },
{ 7	,0,	255  },
{ 31	,0,	255  },
{ 55	,0,	255  },
{ 79	,0,	255  },
{ 103,	0	,255 },
{ 127,	0	,255 },
{ 151,	0	,255 },
{ 175,	0	,255 },
{ 199,	0	,255 },
{ 223,	0	,255 },
{ 247,	0	,255 },
{ 255,	0	,239 },
};




void tidl_tb_rgbToY(
  uint8_t * r,
  uint8_t * g,
  uint8_t * b,
  uint8_t * y,
  uint16_t   cols,
  uint16_t   rows)
{
  int32_t R, G, B;
  int32_t Y;
  uint32_t i;
  uint32_t j;
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      R = r[cols*i + j];
      G = g[cols*i + j];
      B = b[cols*i + j];
      Y = ((66 * (R)+129 * (G)+25 * (B)+128) >> 8) + 16;
      y[cols*i + j] = Y > 255 ? 255 : Y;
    }
}

void tidl_tb_rgbToUV(
  uint8_t * r,
  uint8_t * g,
  uint8_t * b,
  uint8_t * u,
  uint8_t * v,
  uint16_t   cols,
  uint16_t   rows)
{
  int32_t R, G, B;
  int32_t U, V;
  uint32_t i;
  uint32_t j;
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      R = r[cols*i + j];
      G = g[cols*i + j];
      B = b[cols*i + j];

      U = ((-38 * (R)-74 * (G)+112 * (B)+128) >> 8) + 128;
      V = ((112 * (R)-94 * (G)-18 * (B)+128) >> 8) + 128;

      u[cols*i + j] = (U > 0) ? (U > 255 ? 255 : U) : 0;
      v[cols*i + j] = (V > 0) ? (V > 255 ? 255 : V) : 0;
    }
}


void tidl_tb_yuvToRgb(
  uint8_t * y,
  uint8_t * u,
  uint8_t * v,
  uint8_t * r,
  uint8_t * g,
  uint8_t * b,
  uint16_t   cols,
  uint16_t   rows)
{
  int32_t Y, U, V;
  int32_t R, G, B;
  uint32_t i;
  uint32_t j;
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      Y = y[cols*i + j] - 16;
      U = u[cols*i + j] - 128;
      V = v[cols*i + j] - 128;
      R = (Y + 1.140*V);
      G = (Y - 0.395*U - 0.581*V);
      B = (Y + 2.032*U);


      r[cols*i + j] = (R > 0) ? (R > 255 ? 255 : R) : 0;
      g[cols*i + j] = (G > 0) ? (G > 255 ? 255 : G) : 0;
      b[cols*i + j] = (B > 0) ? (B > 255 ? 255 : B) : 0;
    }
}
const char * category_string[] =
{
  "Bg",
  "person",
  "trafficsign",
  "vehicle"
};
void
APP_drawBoldBoxRGB
(
  uint8_t * r,
  uint8_t * g,
  uint8_t * b,
  uint32_t  rgb24bit,
  int16_t  width,
  int16_t  height,
  int16_t  x1,
  int16_t  y1,
  int16_t  x2,
  int16_t  y2,
  uint32_t lineThickNess
  )
{
  int32_t  xmin ;
  int32_t  ymin ;
  int32_t  xmax ;
  int32_t  ymax ;
  int32_t i;
  for (i = 0; i < lineThickNess; i++)
  {
    xmin = clip_offst(x1 + i, width);
    ymin = clip_offst(y1 + i, height);
    xmax = clip_offst(x2 - i, width);
    ymax = clip_offst(y2 - i, height);
    if((xmax>xmin) && (ymax>ymin))
    APP_drawBoxRGB(r, g, b, rgb24bit, width, xmin, ymin, xmax, ymax);
  }


}

float TIDL_findMax(
  void   * ptr,
  int32_t  elementType,
  int32_t  bufSize)
{
  int32_t i;
  float data, max = -FLT_MAX;
  for (i = 0; i < bufSize; i++)
  {
    if(elementType ==  TIDL_UnsignedChar)
    {
      data = ((uint8_t*)ptr)[i];
    }
    else if(elementType ==  TIDL_SignedChar)
    {
      data = ((int8_t*)ptr)[i];
    }
    else if(elementType ==  TIDL_UnsignedShort)
    {
      data = ((uint16_t*)ptr)[i];
    }
    else if(elementType ==  TIDL_SignedShort)
    {
      data = ((int16_t*)ptr)[i];
    }
    else if(elementType == TIDL_SinglePrecFloat)
    {
      data = ((float*)ptr)[i];
    }
    else if(elementType == TIDL_SignedWord)
    {
      data = ((int32_t*)ptr)[i];
    }
    else
    {
      printf(" Unsupported elementType in tidl_tb_postProc\n");
    }
    if(data > max)
    {
      max = data;
    }
  }
  return max;
}

void TIDL_refclassifyAccuracyFloat(
  float   * prob,
  float     min,
  int32_t  idx,
  int32_t  bufSize)
{
  float max = min;
  int32_t crrMax;
  int32_t i, j, maxIdx = 0;
  for (i = 0; i < bufSize; i++)
  {
    if (max < prob[i])
    {
      maxIdx = i;
      max = prob[i];
    }
  }

  crrMax = maxIdx;
  if (idx == maxIdx)
  {
    passTestCount++;
    top5passTestCount++;
  }
  else
  {
    for (j = 0; j < 4; j++)
    {
      max = min;
      prob[maxIdx] = min;
      maxIdx = 0;
      for (i = 0; i < bufSize; i++)
      {
        if (max < prob[i])
        {
          maxIdx = i;
          max = prob[i];
        }
      }
      if (idx == maxIdx)
      {
        top5passTestCount++;
        break;
      }
    }
  }
  tidl_tb_printf(0, "\n A : %5d, %5.4f, %5.4f, %5d", idx, ((1.0*passTestCount) / totalTestCount), ((1.0*top5passTestCount) / totalTestCount), crrMax);
}
void TIDL_refclassifyAccuracyUint8(
  uint8_t   * prob,
  uint8_t     min,
  int32_t  idx,
  int32_t  bufSize)
{
  uint8_t max = min;
  int32_t crrMax;
  int32_t i, j, maxIdx = 0;
  for (i = 0; i < bufSize; i++)
  {
    if (max < prob[i])
    {
      maxIdx = i;
      max = prob[i];
    }
  }

  crrMax = maxIdx;
  if (idx == maxIdx)
  {
    passTestCount++;
    top5passTestCount++;
  }
  else
  {
    for (j = 0; j < 4; j++)
    {
      max = min;
      prob[maxIdx] = min;
      maxIdx = 0;
      for (i = 0; i < bufSize; i++)
      {
        if (max < prob[i])
        {
          maxIdx = i;
          max = prob[i];
        }
      }
      if (idx == maxIdx)
      {
        top5passTestCount++;
        break;
      }
    }
  }
  tidl_tb_printf(0, "\n A : %5d, %5.4f, %5.4f, %5d", idx, ((1.0*passTestCount) / totalTestCount), ((1.0*top5passTestCount) / totalTestCount), crrMax);
}
void TIDL_refclassifyAccuracyInt8(
  int8_t   * prob,
  int8_t     min,
  int32_t  idx,
  int32_t  bufSize)
{
  int8_t max = min;
  int32_t crrMax;
  int32_t i, j, maxIdx = 0;
  for (i = 0; i < bufSize; i++)
  {
    if (max < prob[i])
    {
      maxIdx = i;
      max = prob[i];
    }
  }

  crrMax = maxIdx;
  if (idx == maxIdx)
  {
    passTestCount++;
    top5passTestCount++;
  }
  else
  {
    for (j = 0; j < 4; j++)
    {
      max = min;
      prob[maxIdx] = min;
      maxIdx = 0;
      for (i = 0; i < bufSize; i++)
      {
        if (max < prob[i])
        {
          maxIdx = i;
          max = prob[i];
        }
      }
      if (idx == maxIdx)
      {
        top5passTestCount++;
        break;
      }
    }
  }
  tidl_tb_printf(0, "\n A : %5d, %5.4f, %5.4f, %5d", idx, ((1.0*passTestCount) / totalTestCount), ((1.0*top5passTestCount) / totalTestCount), crrMax);
}
void TIDL_refclassifyAccuracyInt16(
  int16_t   * prob,
  int16_t     min,
  int32_t  idx,
  int32_t  bufSize)
{
  int16_t max = min;
  int32_t crrMax;
  int32_t i, j, maxIdx = 0;
  for (i = 0; i < bufSize; i++)
  {
    if (max < prob[i])
    {
      maxIdx = i;
      max = prob[i];
    }
  }

  crrMax = maxIdx;
  if (idx == maxIdx)
  {
    passTestCount++;
    top5passTestCount++;
  }
  else
  {
    for (j = 0; j < 4; j++)
    {
      max = min;
      prob[maxIdx] = min;
      maxIdx = 0;
      for (i = 0; i < bufSize; i++)
      {
        if (max < prob[i])
        {
          maxIdx = i;
          max = prob[i];
        }
      }
      if (idx == maxIdx)
      {
        top5passTestCount++;
        break;
      }
    }
  }
  tidl_tb_printf(0, "\n A : %5d, %5.4f, %5.4f, %5d", idx, ((1.0*passTestCount) / totalTestCount), ((1.0*top5passTestCount) / totalTestCount), crrMax);
}


int32_t tidl_tb_postProc(int32_t width, int32_t height, int32_t n, int32_t frameCount, void * fPtr, int32_t elementType)
{
  int32_t i,j;
  int32_t idx;
  char imageFileName[500];
  char gtName[500];
  char baseFileName[500];

  if (gParams.postProcType == 1)
  {
    getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf/gIOParams.numVirtualCores-1, imageFileName);
    getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf/gIOParams.numVirtualCores, gtName);

    idx = atoi(gtName);
    totalTestCount++;

    if (elementType == TIDL_SinglePrecFloat)
    {
      TIDL_refclassifyAccuracyFloat((float *)fPtr, 0, idx, (width*height*n));
    }
    else if (elementType == TIDL_SignedChar)
    {
      TIDL_refclassifyAccuracyInt8((int8_t *)fPtr, (int8_t )-128, idx, (width*height*n));
    }
    else if (elementType == TIDL_UnsignedChar)
    {
      TIDL_refclassifyAccuracyUint8((uint8_t *)fPtr, (uint8_t)0, idx, (width*height*n));
    }
    else if (elementType == TIDL_SignedShort)
    {
      TIDL_refclassifyAccuracyInt16((int16_t *)fPtr, (int16_t )(0x8000), idx, (width*height*n));
    }
    else
    {
      printf(" Unsupported elementType in tidl_tb_postProc\n");
    }

  }
  else if (gParams.postProcType == 2)
  {
    TIDL_ODLayerHeaderInfo * objDetHeader = (TIDL_ODLayerHeaderInfo *)fPtr;
    TIDL_ODLayerObjInfo * bboxPtr = (TIDL_ODLayerObjInfo*)(((char *)fPtr) + (int32_t)objDetHeader->objInfoOffset);
    TIDL_ODLayerObjectPose *objectPosePtr ;
    // TIDL_ODLayerRawObjInfo *rawDataPtr;    
    char detTextFileName[500];
    FILE * fp;
    int32_t kpIdx;

    if (gParams.inFileFormat == 2)
    {
      getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf/gIOParams.numVirtualCores-1, imageFileName);
      getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf/gIOParams.numVirtualCores, gtName);
    }
    else if (gParams.inFileFormat == 0)
    {
      strcpy(imageFileName, (char*)gParams.inData);
    }
    else
    {
      printf("Unsuported post-proc \n");
    }
    uint8_t *r;
    uint8_t *g;
    uint8_t *b;
    uint8_t inExt[4];
    int32_t bmpFile = 0;
    strcpy((char *)inExt, (const char *)&imageFileName[strlen((const char *)imageFileName) - 3]);
    if(strcmp((const char *)inExt,"bmp") == 0  || (strcmp((const char *)inExt,"BMP") == 0))
    {
      bmpFile = 1;
    }
#if (!HOST_EMULATION)
    if(bmpFile == 0)
    {
      printf("%s file read not supported. Only BMP file read is supported in target \n", inExt);
      return (-1);
    }
#endif
    if(bmpFile == 1)
    {
      readInBMPImage((uint8_t *)imageFileName, &r, &g, &b, &width, &height);
    }
    else
    {
#if USE_OPENCV
      readInImage((uint8_t *)imageFileName, &r, &g, &b, &width, &height);
#endif
    }
    int32_t bidx;
    strcpy(baseFileName, (const char *)getFileNameFromPath(imageFileName));
    sprintf(imageFileName, "%s_%s_%06d", (const char *)gParams.outData, baseFileName, frameCount);
    sprintf(detTextFileName, "%s_%s_%06d.txt", (const char *)gParams.outData, baseFileName, frameCount);

#if USE_OPENCV
    strcat(imageFileName, "_tidl_post_proc2.png");
#else
    strcat(imageFileName, "_tidl_post_proc2.bmp");
#endif
    fp = fopen(detTextFileName, "w+");
    if (fp == NULL)
    {
      printf("Could not open %s file for writing \n", detTextFileName);
    }
    int32_t pitch = objDetHeader->objInfoSize;
    for (bidx = 0; bidx < objDetHeader->numDetObjects; bidx++)
    {
        uint32_t  rgb24bit = 0;
        int32_t   label = bboxPtr->label;
        int32_t   xmin ; 
        int32_t   ymin ; 
        int32_t   xmax ; 
        int32_t   ymax ; 
        xmin = clip_offst((bboxPtr->xmin * width), width);
        ymin = clip_offst((bboxPtr->ymin * height), height);
        xmax = clip_offst((bboxPtr->xmax * width), width);
        ymax = clip_offst((bboxPtr->ymax * height), height);
        fprintf(fp, "%5d %s %f %f %f %f %f ",
        ((int32_t)bboxPtr->label), gtName, bboxPtr->score, (bboxPtr->xmin * width), (bboxPtr->ymin * height), (bboxPtr->xmax * width), (bboxPtr->ymax * height));
        label = label % 21;
        rgb24bit = ((IdtoColorMap[label][0] << 16) |
          (IdtoColorMap[label][1] << 8) |
          (IdtoColorMap[label][2] << 0));
        APP_drawBoldBoxRGB(r, g, b, rgb24bit, width, height, xmin, ymin, xmax, ymax,4);
        for (kpIdx = 0; kpIdx < objDetHeader->odNumKeyPoints; kpIdx++)
        {
          int32_t   kpX = clip_offst((bboxPtr->keyPoints[kpIdx].x * width), width);
          int32_t   kpY = clip_offst((bboxPtr->keyPoints[kpIdx].y * height), height);
          int32_t   xmin = clip_offst(kpX - 2, width);
          int32_t   ymin = clip_offst(kpY - 2, height);
          int32_t   xmax = clip_offst(kpX + 2, width);
          int32_t   ymax = clip_offst(kpY + 2, height);
          fprintf(fp, "%f %f %f ", (bboxPtr->keyPoints[kpIdx].x * width), (bboxPtr->keyPoints[kpIdx].y * height), (bboxPtr->keyPoints[kpIdx].kpt_confidence));

          rgb24bit = ((255 << 16) |(255 << 8) | (0 << 0));
          APP_drawBoldBoxRGB(r, g, b, rgb24bit, width, height, xmin, ymin, xmax, ymax,4);
        }
        if (TIDL_GetObjDetectionFormatState(objDetHeader->odObjectType, TIDL_DetectObjectPose))
        {
          objectPosePtr = (TIDL_ODLayerObjectPose*) (((uint8_t *)bboxPtr) + pitch - sizeof(TIDL_ODLayerObjectPose));
          fprintf(fp, "%f %f %f %f %f %f %f %f %f", (objectPosePtr->r11), (objectPosePtr->r21), (objectPosePtr->r31),
                                                    (objectPosePtr->r12), (objectPosePtr->r22), (objectPosePtr->r32),
                                                    (objectPosePtr->tx), (objectPosePtr->ty), (objectPosePtr->tz)
                                                    );
        }
        fprintf(fp, "\n");

      bboxPtr = (TIDL_ODLayerObjInfo*) (((uint8_t *)bboxPtr) + pitch);
    }
#if USE_OPENCV
    writeOutImage((uint8_t *)imageFileName, r, g, b, width, height);
#else
    writeOutBMPImage((uint8_t *)imageFileName, r, g, b, width, height);
#endif

    fclose(fp);
    free(r);
    free(g);
    free(b);

  }
  else if ((gParams.postProcType == 3) || (gParams.postProcType == 4))
  {
    if ((elementType != TIDL_UnsignedChar) && (elementType != TIDL_UnsignedShort) && (elementType != TIDL_SinglePrecFloat) && (elementType != TIDL_SignedWord) && (elementType != TIDL_BFloat16))
    {
      printf(" Unsupported elementType %d in tidl_tb_postProc\n", elementType);
    }
    uint8_t * segPtr    = (uint8_t *)fPtr;
    uint16_t * segPtr16 = (uint16_t *)fPtr;
    int32_t * segPtr32 = (int32_t *)fPtr;
    float * segPtrSP = (float *)fPtr;
    int32_t segValue;
    float score;
    float Meanscore = 0;
    float IdxScale = 0;
    int32_t Max = TIDL_findMax(fPtr,elementType,(width*height*n));
    if ( Max > 0 )
    {
      IdxScale = ((MAX_SEG_COLOR_IDX - 1) / Max);
    }
    if (gParams.inFileFormat == 2)
    {
      getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf/gIOParams.numVirtualCores-1, imageFileName);
      getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf/gIOParams.numVirtualCores+gParams.postProcDataId, gtName);
    }
    else if (gParams.inFileFormat == 0)
    {
      strcpy(imageFileName, (char*)gParams.inData);
    }
    else
    {
      printf("Unsuported pre-proc \n");
    }
    uint8_t *r;
    uint8_t *g;
    uint8_t *b;
    uint8_t *y;
    uint8_t *u;
    uint8_t *v;
    y = (uint8_t*)malloc(width*height);
    u = (uint8_t*)malloc(width*height);
    v = (uint8_t*)malloc(width*height);
    uint8_t inExt[4];
    int32_t bmpFile = 0;
    strcpy((char *)inExt, (const char *)&imageFileName[strlen((const char *)imageFileName) - 3]);
    if(strcmp((const char *)inExt,"bmp") == 0  || (strcmp((const char *)inExt,"BMP") == 0))
    {
      bmpFile = 1;
    }
#if (!HOST_EMULATION)
    if(bmpFile == 0)
    {
      printf("%s file read not supported. Only BMP file read is supported in target \n", inExt);
      return (-1);
    }
#endif
    if(bmpFile == 1)
    {
      readandResizeInBMPImage((uint8_t *)imageFileName, &r, &g, &b, width, height);
    }
    else
    {
#if USE_OPENCV
      readandResizeInImage((uint8_t *)imageFileName, &r, &g, &b, width, height);
#endif
    }
    tidl_tb_rgbToY(r, g, b, y, width, height);
    if (gParams.postProcType == 4)
    {
      uint8_t* gtData = (uint8_t*)malloc(width*height);
      uint8_t inExt[4];
      int32_t bmpFile = 0;
      strcpy((char *)inExt, (const char *)&gtName[strlen((const char *)gtName) - 3]);
      if(strcmp((const char *)inExt,"bmp") == 0  || (strcmp((const char *)inExt,"BMP") == 0))
      {
        bmpFile = 1;
      }
#if (!HOST_EMULATION)
      if(bmpFile == 0)
      {
        printf("%s file read not supported. Only BMP file read is supported in target \n", inExt);
        return (-1);
      }
#endif
      if(bmpFile == 1)
      {
        readandResizeBMPsegGt((uint8_t *)gtName, gtData, width, height);
      }
      else
      {
#if USE_OPENCV
        readandResizesegGt((uint8_t *)gtName, gtData, width, height);
#endif
      }


      for (i = 0; i < height*width; i++)
      {
        if (elementType == TIDL_UnsignedChar)
        {
          segValue = segPtr[i];
        }
        else if (elementType == TIDL_UnsignedShort)
        {
          segValue = segPtr16[i];
        }
        else if (elementType == TIDL_SinglePrecFloat)
        {
          segValue = (int)segPtrSP[i];
        }
        else if (elementType == TIDL_SignedWord)
        {
          segValue = (int)segPtr32[i];
        }

        if ((segValue < gParams.totNumClasses) && (gtData[i] < gParams.totNumClasses))
        {
          confusion_matrix[(gtData[i])][segValue]++;
        }
      }
      /* Visualize GT data*/
      //memcpy(segPtr, gtData, height*width);
      free(gtData);


      int32_t numNonZeroScores = 0 ;
      tidl_tb_printf(0, " A ");
      int32_t label;
      for (label = 0; label < gParams.totNumClasses; label++)
      {
        int64_t ture_pos = confusion_matrix[label][label];
        int64_t label_count = 0;
        for (j = 0; j < gParams.totNumClasses; j++)
        {
          label_count += confusion_matrix[label][j];
        }
        int64_t label_observed = 0;
        for (j = 0; j < gParams.totNumClasses; j++)
        {
          label_observed += confusion_matrix[j][label];
        }
        float den = label_count + label_observed - ture_pos;
        score = ((den > 0) ? (ture_pos / den) : 0);

        tidl_tb_printf(0, " : %3d : %4.4f ", label, score);
        if (score > 0)
        {
          Meanscore += score;
          numNonZeroScores++ ;
        }
      }
      if (numNonZeroScores)
      {
        printf("Mean : %4.4f ", (Meanscore / numNonZeroScores));
      }
      else
      {
        printf("Mean : %4.4f ", 0.0);
      }
    }

    for (j = 0; j < height; j++)
    {
      for (i = 0; i < width; i++)
      {
        if ((elementType == TIDL_UnsignedChar) || (elementType == TIDL_SignedChar))
        {
          segValue = segPtr[j*width + i];
        }
        else if ((elementType == TIDL_UnsignedShort) || (elementType == TIDL_SignedShort))
        {
          segValue = segPtr16[j*width + i];
        }
        else if ((elementType == TIDL_SinglePrecFloat))
        {
          segValue = segPtrSP[j*width + i];
        }
        else if ((elementType == TIDL_SignedWord))
        {
          segValue = segPtr32[j*width + i];
        }
        else
        {
          segValue = 0;
        }
        if (segValue > Max) segValue = Max;

        r[j*width + i] = segIdtoColorMap[(int)(segValue * IdxScale)][0];
        g[j*width + i] = segIdtoColorMap[(int)(segValue * IdxScale)][1];
        b[j*width + i] = segIdtoColorMap[(int)(segValue * IdxScale)][2];
      }
    }
    tidl_tb_rgbToUV(r, g, b, u, v, width, height);
    //tidl_tb_rgbToY(r, g, b, y, width, height);
    tidl_tb_yuvToRgb(y, u, v, r, g, b, width, height);
    strcpy(baseFileName, (const char *)getFileNameFromPath(imageFileName));
    sprintf(imageFileName, "%s_%s_%06d", (const char *)gParams.outData, baseFileName, frameCount);
#if USE_OPENCV
    strcat(imageFileName, "_tidl_post_proc3.png");
    writeOutImage((uint8_t *)imageFileName, r, g, b, width, height);
#else
    strcat(imageFileName, "_tidl_post_proc3.bmp");
    writeOutBMPImage((uint8_t *)imageFileName, r, g, b, width, height);
#endif
    free(r);
    free(g);
    free(b);
    free(u);
    free(v);
    free(y);

  }
  else if ((gParams.postProcType == 5) || (gParams.postProcType == 6)) /*for 3D OD pointPillars and fastBEV*/
  {
    TIDL_ODLayerHeaderInfo * objDetHeader = (TIDL_ODLayerHeaderInfo *)fPtr;
    TIDL_3DODLayerObjInfo * box3dPtr = (TIDL_3DODLayerObjInfo*)(((char *)fPtr) + (int32_t)objDetHeader->objInfoOffset);
    char dirName[500];
    char detTextFileName[500];
    FILE * fp;

    if (gParams.inFileFormat == 5)
    {
      getFileNameAndIdxFromList((char*)gParams.inData, frameCount, gIOParams.numInputBuf-1, imageFileName);
    }
    else if (gParams.inFileFormat == 0)
    {
      strcpy(imageFileName, (char*)gParams.inData);
    }
    else
    {
      printf("Unsuported post-proc \n");
    }

    strcpy(baseFileName, (const char *)getFileNameFromPath(imageFileName));

    strcpy((char *)dirName, (char*)gParams.outData);
    getDirFromPath(dirName);

    sprintf(detTextFileName, "%s//%s.txt", (const char *)dirName, baseFileName);

    fp = fopen(detTextFileName, "w+");
    if (fp == NULL)
    {
      printf("Could not open %s file for writing \n", detTextFileName);
    }
    int32_t bidx;
    if (gParams.postProcType == 5)  /*for 3D OD pointPillars*/
    {
      for (bidx = 0; bidx < objDetHeader->numDetObjects; bidx++)
      {
        fprintf(fp, "%3d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",
          ((int32_t)box3dPtr->label), box3dPtr->score,
          box3dPtr->x, box3dPtr->y, box3dPtr->z,
          box3dPtr->w, box3dPtr->l, box3dPtr->h, box3dPtr->yaw);
          box3dPtr++;
          
      }
    }
    else  /*for 3D OD fastBEV*/
    {
      for (bidx = 0; bidx < objDetHeader->numDetObjects; bidx++)
      {
        fprintf(fp, "%3d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",
          ((int32_t)box3dPtr->label), box3dPtr->score,
          box3dPtr->x, box3dPtr->y, box3dPtr->z,
          box3dPtr->w, box3dPtr->l, box3dPtr->h, box3dPtr->yaw, box3dPtr->pitch, box3dPtr->roll);
          box3dPtr++;

      }
    }

    fclose(fp);
  }

  return 0;
}




