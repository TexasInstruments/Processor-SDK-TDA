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



/** @file tidl_image_read_write.c
*
*  @brief  This file contains image read related
*       test code for TI DL test application
*
*
*  @date  Jul 2019
*
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform_common.h"

#pragma pack(2)
typedef struct
{
  uint8_t  headerName[14];
  uint32_t headerSize;
  int32_t  width;
  int32_t  height;
  uint16_t numColPlanes;
  uint16_t numBitsPerPel;
  uint32_t cmpMethod;
  uint32_t imageSize;
  int32_t horPPM;
  int32_t verPPM;
  int32_t numCol;
  int32_t numImpCol;
}tidl_bmpHeader; 

#define TIDL_BMP_HEADER_INFO_SIZE (54)
#define TIDL_BMP_OFST_NAME    (0 )
#define TIDL_BMP_OFST_SIZE    (14)
#define TIDL_BMP_OFST_WIDTH   (18)
#define TIDL_BMP_OFST_HEIGHT  (22)
#define TIDL_BMP_OFST_PLANES  (26)
#define TIDL_BMP_OFST_BPP     (28)
#define TIDL_BMP_OFST_CMPTYP  (30)
#define TIDL_BMP_OFST_IMGSIZE (34)
#define TIDL_BMP_OFST_HORPPM  (38)
#define TIDL_BMP_OFST_VERPPM  (42)
#define TIDL_BMP_OFST_NUMCOL  (46)
#define TIDL_BMP_OFST_IMPCOL  (50)
uint8_t bmHeader[TIDL_BMP_HEADER_INFO_SIZE];
uint8_t bmHeaderNmae[] = {66,77,54,48,42,0,0,0,0,0,54,0,0,0};


uint8_t * tidl_loadBMP(const char *filepath, int32_t * width, int32_t * height)
{
    TI_FILE *f = FOPEN(filepath, "rb");
    int32_t fReadStatus;
    if (f)
    {
      fReadStatus = FREAD(bmHeader, sizeof(uint8_t), sizeof(tidl_bmpHeader), f);
      *width = *(int32_t *)(bmHeader + TIDL_BMP_OFST_WIDTH);
      *height = *(int32_t*)(bmHeader + TIDL_BMP_OFST_HEIGHT);
      unsigned int size = *width * *height * 3;
      uint8_t * pixels = (uint8_t *)malloc(size);
      fReadStatus = FREAD(pixels, sizeof(unsigned char), size, f);
      FCLOSE(f);
      (void)fReadStatus;
      return pixels;
   }  
   return NULL;
}

int32_t tidl_saveBMP(const char *filepath, uint8_t * pixels, int32_t width, int32_t height)
{
    TI_FILE *f = FOPEN(filepath, "wb+");
    if (f)
    {
      memcpy(bmHeader, bmHeaderNmae, TIDL_BMP_OFST_SIZE);
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_WIDTH) = width;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_HEIGHT) = height;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_IMGSIZE) = width * height * 3;;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_NUMCOL) = 0;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_IMPCOL) = 0;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_CMPTYP) = 0;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_HORPPM) = 3780;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_VERPPM) = 3780;
      *(int32_t*)(bmHeader + TIDL_BMP_OFST_SIZE) = 40;
      *(int16_t*)(bmHeader + TIDL_BMP_OFST_PLANES) = 1;
      *(int16_t*)(bmHeader + TIDL_BMP_OFST_BPP) = 24;
      
      FWRITE(bmHeader, sizeof(uint8_t), TIDL_BMP_HEADER_INFO_SIZE, f);
      unsigned int size = width * height  * 3;
      FWRITE(pixels, sizeof(unsigned char), size, f);
      FCLOSE(f);
      return 0;
   }  
   return -1;
}

int32_t tidl_imageInterToPlanar(uint8_t * pixels, int32_t width, int32_t height, int32_t outFormat)
{
  unsigned int size = width * height  * 3;
  uint8_t * ptr = (uint8_t *)malloc(size);
  int32_t i, j, idx = 0;
  size = width * height;
  if(outFormat == 0)
  {
    for (j = height - 1; j >= 0; j--)
    {
      for (i = 0; i < width; i++)
      {
        ptr[0 * size + j * width + i] = pixels[idx++];
        ptr[1 * size + j * width + i] = pixels[idx++];
        ptr[2 * size + j * width + i] = pixels[idx++];
      }
    }
  }
  else
  {
    for (j = height - 1; j >= 0; j--)
    {
      for (i = 0; i < width; i++)
      {
        ptr[2 * size + j * width + i] = pixels[idx++];
        ptr[1 * size + j * width + i] = pixels[idx++];
        ptr[0 * size + j * width + i] = pixels[idx++];
      }
    }
  }
  memcpy(pixels, ptr, size * 3);
  free(ptr);
  return 0;
}

int32_t tidl_imagePlanarToInter(uint8_t * pixels, int32_t width, int32_t height, int32_t outFormat)
{
  unsigned int size = width * height  * 3;
  uint8_t * ptr = (uint8_t *)malloc(size);
  int32_t i, j, idx = 0;
  size = width * height;
  if(outFormat == 0)
  {
    for (j = height - 1; j >= 0; j--)
    {
      for (i = 0; i < width; i++)
      {
        ptr[idx++] = pixels[0 * size + j * width + i];
        ptr[idx++] = pixels[1 * size + j * width + i];
        ptr[idx++] = pixels[2 * size + j * width + i];
      }
    }
  }
  else
  {
    for (j = height - 1; j >= 0; j--)
    {
      for (i = 0; i < width; i++)
      {
        ptr[idx++] = pixels[2 * size + j * width + i];
        ptr[idx++] = pixels[1 * size + j * width + i];
        ptr[idx++] = pixels[0 * size + j * width + i];
      }
    }
  }
  memcpy(pixels, ptr, size * 3);
  free(ptr);
  return 0;
}
int32_t tidl_imageCrop(uint8_t * pixels, uint8_t * ptr, int32_t width, int32_t height, int32_t startX, int32_t startY, int32_t outWidth, int32_t outHeight, int32_t numChs)
{
  int32_t size, outSize ;
  int32_t i, j, k;
  outSize = outWidth * outHeight;
  size    = width * height;
  for (k = 0; k < numChs; k++)
  {
    for (j = 0; j < outHeight; j++)
    {
      for (i = 0; i < outWidth; i++)
      {
        ptr[k * outSize + j * outWidth + i] = pixels[k * size + (j + startY)  * width + i + startX];
      }
    }
  }
  return 0;
}

void tidl_imageResize(uint8_t *pixels, uint8_t *ptr, int32_t inWidth, int32_t inHeight, int32_t outWidth, int32_t outHeight, int32_t mode, int32_t numChs)
{

  float wRatio = inWidth / (1.0 * outWidth);
  float hRatio = inHeight/ (1.0 * outHeight);
  int inChPitch = inWidth * inHeight;
  int outChPitch = outWidth * outHeight;
  int inPitch = inWidth;
  int outPitch = outWidth;


  int i = 0;
  int j = 0;
  int k = 0;
  volatile float hLoc, wLoc;
  int hIdx, wIdx, hNext, wNext;
  volatile float w00, w01, w10, w11;
  if (mode == 0)
  {
    for (i = 0; i < outHeight; i++)
    {
      hLoc = hRatio * (i + 0.5) - 0.5;
      hLoc = (hLoc < 0) ? 0 : hLoc;
      hIdx = hLoc;
      for (j = 0; j < outWidth; j++)
      {
        wLoc = wRatio * (j + 0.5) - 0.5;
        wLoc = (wLoc < 0) ? 0 : wLoc;
        wIdx = wLoc;
        int32_t inputOffset = inPitch * hIdx + wIdx;
        int32_t outputOffset = outPitch * (i) + (j);
        for (k = 0; k < numChs; k++)
        {
          *(ptr + k * outChPitch + outputOffset) = *(pixels + k * inChPitch + inputOffset);
        }
      }
    }
  }
  else if (mode > 0)
  {
    for (i = 0; i < outHeight; i++)
    {
      hLoc = hRatio * (i + 0.5f) - 0.5f;
      hLoc = (hLoc < 0) ? 0 : hLoc;
      hIdx = hLoc;
      hNext = (hIdx < (inHeight - 1)) ? 1 : 0;
      w11 = hLoc - hIdx;
      w10 = 1 - w11;
      for (j = 0; j < outWidth; j++)
      {
        wLoc = wRatio * (j + 0.5f) - 0.5f;
        wLoc = (wLoc < 0) ? 0 : wLoc;
        wIdx = wLoc;
        w01 = wLoc - wIdx;
        w00 = 1 - w01;
        int32_t inputOffset = inPitch * hIdx + wIdx;
        int32_t outputOffset = outPitch * (i) + (j);
        wNext = (wIdx < (inWidth - 1)) ? 1 : 0;

        for (k = 0; k < numChs; k++)
        {
          uint8_t i00 = *(pixels + k * inChPitch + inputOffset);
          uint8_t i01 = *(pixels + k * inChPitch + inputOffset + wNext);
          uint8_t i10 = *(pixels + k * inChPitch + inputOffset + hNext * inPitch);
          uint8_t i11 = *(pixels + k * inChPitch + inputOffset + hNext * inPitch + wNext);
          volatile float i0_int = (i00 * w00 + i01 * w01);
          volatile float i1_int = (i10 * w00 + i11 * w01);
                         i0_int = i0_int*w10;
                         i1_int = i1_int*w11;
          *(ptr + k * outChPitch + outputOffset) = (i0_int + i1_int);
        }
      }
    }
  }
}
