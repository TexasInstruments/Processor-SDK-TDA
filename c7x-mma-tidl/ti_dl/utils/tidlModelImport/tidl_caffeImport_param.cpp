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

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
//#include <io.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <cmath>

#include "ti_dl.h"
#include "tidl_import_config.h"
#include "caffe.pb.h"
#include "tidl_import_diag.h"

#define __MODULE__ "[PARSER]"

using namespace std;
using namespace caffe;
using ::google::protobuf::Message;
using ::google::protobuf::io::FileInputStream;
using ::google::protobuf::io::FileOutputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;
using ::google::protobuf::io::CodedOutputStream;


#include "tidl_import_common.h"
#include "tidl_caffeImport_param.h"

extern sTIDL_OrgNetwork_t      orgTIDLNetStructure;
extern sTIDL_OrgNetwork_t      tempTIDLNetStructure;
extern sTIDL_Network_t         tIDLNetStructure;

void TIDL_tfBNToScaleBias(
  float    * scale,
  float    * bias,
  uint32_t  numCh,
  float * mean,
  float * var,
  float * gamma,
  float * beta,
  float eps
  );

int32_t tidl_caffeIndexStructure2Param(
  NetParameter& netStructure,
  NetParameter& netParams,
  int32_t structureIndex)
{
  int32_t netParamsIndex = -1;
  for(int j=0;j<netParams.layer_size();j++)
  {
    if(netParams.layer(j).name() == netStructure.layer(structureIndex).name())
      netParamsIndex = j;
  }

  if(netParamsIndex == -1)
    TIDL_GLOBAL_REPORT_ERROR("Params for %s layer %s exist in netStructure, but not in netParam",netStructure.layer(structureIndex).type().c_str(), netStructure.layer(structureIndex).name().c_str());

  return netParamsIndex;
}

void TIDL_caffeReorderDeconvFilters(sBuffer_t &buf, int32_t gno, int32_t gni, int32_t gr, int32_t k)
{
  int32_t g, j, i, no, ni;
  float*  src = (float*)buf.ptr;
  float*  dst = (float*)my_malloc(buf.bufSize* sizeof(float));
  no = gno/gr;
  ni = gni/gr;
  for(g =0; g < gr; g++)
  {
    for(j = 0; j < no; j++)
    {
      for(i = 0; i < ni; i++)
      {
        memcpy((dst + g*no*ni*k + j*ni*k + i*k), (src + g*no*ni*k + j*k + i*no*k), k*sizeof(float));
      }
    }
  }
  buf.ptr = dst;
  my_free(src);
}

int32_t TIDL_caffeCopyFloatInitializerTensor(NetParameter& netStructure, NetParameter& netParams, int32_t idxProto, int32_t idxBlobs, sBuffer_t &buf)
{
  int32_t idxParam = -1;
  int32_t dataSize = 0;
  float*  data;

  idxParam = tidl_caffeIndexStructure2Param(netStructure, netParams, idxProto);

  if((idxParam != -1) &&
     (netParams.layer(idxParam).blobs_size() > idxBlobs) &&
     (netParams.layer(idxParam).blobs(idxBlobs).data_size() > 0))
  {
    dataSize = netParams.layer(idxParam).blobs(idxBlobs).data_size();
    data = (float *)my_malloc(dataSize*sizeof(float));

    for (int i = 0; i < dataSize; i++)
    {
      data[i] = netParams.layer(idxParam).blobs(idxBlobs).data(i);
    }
  }
  else
  {
    dataSize = 0;
    data = NULL;
  }

  buf.ptr = data;
  buf.bufSize = dataSize;
  buf.dataType = TIDL_SinglePrecFloat;

  return 0;
}

static void TIDL_caffeConstructWeightBias(
  float    * conv2weights,
  float    * conv2dBias,
  uint32_t  kernerlSize,
  uint32_t  numCh,
  float * mean,
  float * var,
  float * scale,
  float * bias,
  float eps
  )

{
  kernerlSize /= numCh;
  uint32_t i, j;
  for (j = 0; j < numCh; j++)
  {
    float cb = conv2dBias[j];
    float m = mean[j];
    float v = var[j];
    float s = scale[j];
    float b = bias[j];
    double inv_var = pow((eps + v), -0.5);
    for (i = 0; i < kernerlSize; i++)
    {
      float w = conv2weights[j*kernerlSize + i];
      conv2weights[j*kernerlSize + i] = (w*s)*inv_var;
    }
    conv2dBias[j] = (((cb - m)*s)*inv_var) + b;
  }
}

int32_t TIDL_caffeMapConcatParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t idx;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_ConcatLayer;
  TIDLPCLayers.numInBufs = netStructure.layer(i).bottom_size();
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  TIDLPCLayers.layerParams.concatParams.axis = netStructure.layer(i).concat_param().axis();
  /*Implicit assumption that 4 dimensions exist for caffe:*/
  TIDLPCLayers.layerParams.concatParams.axis += (TIDL_DIM_MAX - 4);  
  if(TIDLPCLayers.layerParams.concatParams.axis < 0 || TIDLPCLayers.layerParams.concatParams.axis >= TIDL_DIM_MAX)
  {
    TIDL_GLOBAL_REPORT_ERROR("Concat Layer %s -- Unsupported axis configuration", netStructure.layer(i).name().c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  return 0;
}

int32_t TIDL_caffeMapPoolingParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_PoolingParams_t &poolParams = TIDLPCLayers.layerParams.poolParams;
  sTIDL_PoolPCParams_t &poolPCParams = TIDLPCLayers.layerPCParams.poolParams;
  poolPCParams.dilationH = 1;
  poolPCParams.dilationW = 1;

  TIDLPCLayers.layerType = TIDL_PoolingLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  poolParams.poolingType = TIDL_AveragePooling;
  poolParams.strideW = 2;
  poolParams.strideH = 2;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;
  poolPCParams.originalPadT = 0;
  poolPCParams.originalPadL = 0;
  poolPCParams.originalPadB = 0;
  poolPCParams.originalPadR = 0;

  poolParams.kernelW = 2;
  poolParams.kernelH = 2;
  poolParams.useCeil = 0;

  if ((netStructure.layer(i).pooling_param().has_ceil_mode() == false) ||
      (netStructure.layer(i).pooling_param().ceil_mode() == true))
  {
    poolParams.useCeil = 1;
  }
  else
  {
    poolParams.useCeil = 0;
  }

  if(netStructure.layer(i).pooling_param().has_kernel_h())
  {
    poolParams.kernelH = netStructure.layer(i).pooling_param().kernel_h();
  }

  if(netStructure.layer(i).pooling_param().has_kernel_w())
  {
    poolParams.kernelW = netStructure.layer(i).pooling_param().kernel_w();
  }

  if(netStructure.layer(i).pooling_param().has_stride_h())
  {
    poolParams.strideH = netStructure.layer(i).pooling_param().stride_h();
  }

  if(netStructure.layer(i).pooling_param().has_stride_w())
  {
    poolParams.strideW = netStructure.layer(i).pooling_param().stride_w();
  }

  if(netStructure.layer(i).pooling_param().has_stride())
  {
    poolParams.strideW = netStructure.layer(i).pooling_param().stride();
    poolParams.strideH = netStructure.layer(i).pooling_param().stride();
  }

  if(netStructure.layer(i).pooling_param().has_kernel_size())
  {
    poolParams.kernelW = netStructure.layer(i).pooling_param().kernel_size();
    poolParams.kernelH = netStructure.layer(i).pooling_param().kernel_size();
  }

  if(netStructure.layer(i).pooling_param().global_pooling())
  {
    poolParams.strideW = 0;
    poolParams.strideH = 0;
    poolParams.kernelW = 0;
    poolParams.kernelH = 0;
  }

  if(netStructure.layer(i).pooling_param().has_pad() > 0)
  {
    poolParams.padW = netStructure.layer(i).pooling_param().pad();
    poolParams.padH = netStructure.layer(i).pooling_param().pad();
    poolParams.padT = netStructure.layer(i).pooling_param().pad();
    poolParams.padB = netStructure.layer(i).pooling_param().pad();
    poolParams.padL = netStructure.layer(i).pooling_param().pad();
    poolParams.padR = netStructure.layer(i).pooling_param().pad();
    poolPCParams.originalPadT = poolParams.padT;
    poolPCParams.originalPadL = poolParams.padL;
    poolPCParams.originalPadB = poolParams.padB;
    poolPCParams.originalPadR = poolParams.padR;
  }

  if(netStructure.layer(i).pooling_param().pool() == TIDL_AveragePooling)
  {
    TIDLPCLayers.layerParams.poolParams.poolingType = TIDL_AveragePooling;
  }
  else if(netStructure.layer(i).pooling_param().pool() == TIDL_MaxPooling)
  {
    TIDLPCLayers.layerParams.poolParams.poolingType = TIDL_MaxPooling;
  }
  else
  {
    /* Unsupported Pooling Type */
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Unsupported Pooling Type -- %d", netStructure.layer(i).pooling_param().pool());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return 0;
}

int32_t TIDL_caffeMapConvParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_ConvParams_t &convParams = TIDLPCLayers.layerParams.convParams;

  TIDLPCLayers.layerType = TIDL_ConvolutionLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);
  TIDLPCLayers.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  convParams.numOutChannels = netStructure.layer(i).convolution_param().num_output();
  convParams.enableBias = netStructure.layer(i).convolution_param().bias_term();
  convParams.enablePooling = 0; // wait for pooling merge

  /*
   * Parsing Convolution Groups (Depthwise)
   */
  if(netStructure.layer(i).convolution_param().has_group())
  {
    convParams.numGroups = netStructure.layer(i).convolution_param().group();
  }
  else
  {
    convParams.numGroups = 1;
  }
  if(strcmp(netStructure.layer(i).type().c_str(), "ConvolutionDepthwise") == 0)
  {
    convParams.numGroups = netStructure.layer(i).convolution_param().num_output();
  }

  /*
   * Parsing Dilations
   */
  if(netStructure.layer(i).convolution_param().dilation_size() > 0)
  {
    convParams.dilationW = netStructure.layer(i).convolution_param().dilation(0);
    convParams.dilationH = netStructure.layer(i).convolution_param().dilation(0);
  }
  else
  {
    convParams.dilationW = 1;
    convParams.dilationH = 1;
  }

  /*
   * Parsing Strides
   */
  if(netStructure.layer(i).convolution_param().stride_size() > 0)
  {
    convParams.strideW = netStructure.layer(i).convolution_param().stride(0);
    convParams.strideH = netStructure.layer(i).convolution_param().stride(0);
  }
  else
  {
    convParams.strideW = 1;
    convParams.strideH = 1;
  }

  if(netStructure.layer(i).convolution_param().has_stride_h())
  {
    convParams.strideH = netStructure.layer(i).convolution_param().stride_h();
  }

  if(netStructure.layer(i).convolution_param().has_stride_w())
  {
    convParams.strideW = netStructure.layer(i).convolution_param().stride_w();
  }

  /*
   * Parsing Kernel size
   */
  if(netStructure.layer(i).convolution_param().kernel_size_size() > 0)
  {
    convParams.kernelH = netStructure.layer(i).convolution_param().kernel_size(0);
    convParams.kernelW = netStructure.layer(i).convolution_param().kernel_size(0);
  }
  else
  {
    convParams.kernelH = 0;
    convParams.kernelW = 0;
  }

  if(netStructure.layer(i).convolution_param().has_kernel_h())
  {
    convParams.kernelH = netStructure.layer(i).convolution_param().kernel_h();
  }

  if(netStructure.layer(i).convolution_param().has_kernel_w())
  {
    convParams.kernelW = netStructure.layer(i).convolution_param().kernel_w();
  }

  if(convParams.kernelH == 0 || convParams.kernelW == 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Conv Layer %s -- Kernel Size not specified", netStructure.layer(i).name().c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }


  /*
   * Parsing Padding size
   */
  if(netStructure.layer(i).convolution_param().pad_size() > 0)
  {
    convParams.padW = netStructure.layer(i).convolution_param().pad(0);
    convParams.padH = netStructure.layer(i).convolution_param().pad(0);
  }
  else
  {
    convParams.padW = 0;
    convParams.padH = 0;
  }

  if(netStructure.layer(i).convolution_param().has_pad_h())
  {
    convParams.padH = netStructure.layer(i).convolution_param().pad_h();
  }

  if(netStructure.layer(i).convolution_param().has_pad_w())
  {
    convParams.padW = netStructure.layer(i).convolution_param().pad_w();
  }
  convParams.padL = convParams.padW;
  convParams.padR = convParams.padW;
  convParams.padT = convParams.padH;
  convParams.padB = convParams.padH;
  /*
   * Import coeff from caffemodel
   */
  TIDL_caffeCopyFloatInitializerTensor(netStructure, netParams, i, 0, TIDLPCLayers.weights);
  if(convParams.enableBias)
  {
    TIDL_caffeCopyFloatInitializerTensor(netStructure, netParams, i, 1, TIDLPCLayers.bias);
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_caffeMapReluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_ReLULayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(0);
  TIDLPCLayers.actParams.actType = TIDL_RelU;

  return 0;
}

int32_t TIDL_caffeMapSigmoidParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_SigmoidLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(0);
  TIDLPCLayers.actParams.actType = TIDL_Sigmoid;

  return 0;
}

int32_t TIDL_caffeMapTanhParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t status = 0;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_TanhLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);
  TIDLPCLayers.actParams.actType = TIDL_Tanh;
  return status;
}

int32_t TIDL_caffeMapHardSigmoidParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t status = 0;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_HardSigmoidLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(0);
  TIDLPCLayers.actParams.actType = TIDL_HardSigmoid;
  return status;
}

int32_t TIDL_caffeMapELUParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t status = 0;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_ELULayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);
  TIDLPCLayers.actParams.actType = TIDL_ELU;
  return status;
}

int32_t TIDL_caffeMapPReLUParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t     dataSize;
  int32_t	    idxParam;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_PReLULayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);
  TIDLPCLayers.actParams.actType = TIDL_PRelU;

  idxParam = tidl_caffeIndexStructure2Param(netStructure, netParams, i);
  if (idxParam != -1)
  {
    dataSize = netParams.layer(idxParam).blobs(0).data_size();

    float * dataBias = (float*)my_malloc(dataSize*sizeof(float));
    float * dataWeigths = (float*)my_malloc(dataSize*sizeof(float));
    float * dataSlope = (float *)my_malloc(dataSize*sizeof(float));

    for (int j = 0; j < dataSize; j++)
    {
      dataBias[j] = 0;
      dataWeigths[j] = 1;
    }


    for (int j = 0; j < dataSize; j++)
    {
      int32_t channel_shared = netStructure.layer(i).prelu_param().channel_shared();
      if (channel_shared)
      {
        dataSlope[j] = netParams.layer(idxParam).blobs(0).data(0);
      }
      else
      {
        dataSlope[j] = netParams.layer(idxParam).blobs(0).data(j);
      }
    }

    TIDLPCLayers.bias.ptr = dataBias;
    TIDLPCLayers.bias.bufSize = dataSize;
    TIDLPCLayers.weights.ptr = dataWeigths;
    TIDLPCLayers.weights.bufSize = dataSize;
    TIDLPCLayers.slope.ptr = dataSlope;
    TIDLPCLayers.slope.bufSize = dataSize;
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not find %s params in caffemodel, will generate random params", (char*)TIDLPCLayers.name);

    TIDLPCLayers.bias.ptr = NULL;
    TIDLPCLayers.bias.bufSize = 0;
    TIDLPCLayers.weights.ptr = NULL;
    TIDLPCLayers.weights.bufSize = 0;
    TIDLPCLayers.slope.ptr = NULL;
    TIDLPCLayers.slope.bufSize = 0;
  }


  return 0;
}

int32_t TIDL_caffeMapDropoutParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_DropOutLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  return 0;
}

int32_t TIDL_caffeMapSoftmaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.layerType = TIDL_SoftMaxLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = TIDL_SinglePrecFloat;
  TIDLPCLayers.layerParams.softMaxParams.axis = TIDL_DIM_WIDTH;
  TIDLPCLayers.layerParams.softMaxParams.outTranspose = 0;
  return 0;
}

int32_t TIDL_caffeMapDeconvParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t idx, j;
  int32_t axis;
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_ConvParams_t &convParams = TIDLPCLayers.layerParams.convParams;

  TIDLPCLayers.layerType = TIDL_Deconv2DLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);
  TIDLPCLayers.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  convParams.numOutChannels = netStructure.layer(i).convolution_param().num_output();
  convParams.enableBias = netStructure.layer(i).convolution_param().bias_term();
  convParams.enablePooling = 0; // wait for pooling merge

  /*
   * Parsing Convolution Groups (Depthwise)
   */
  if(netStructure.layer(i).convolution_param().has_group())
  {
    convParams.numGroups = netStructure.layer(i).convolution_param().group();
  }
  else
  {
    convParams.numGroups = 1;
  }

  /*
   * Parsing Dilations
   */
  if(netStructure.layer(i).convolution_param().dilation_size() > 0)
  {
    convParams.dilationW = netStructure.layer(i).convolution_param().dilation(0);
    convParams.dilationH = netStructure.layer(i).convolution_param().dilation(0);
  }
  else
  {
    convParams.dilationW = 1;
    convParams.dilationH = 1;
  }

  /*
   * Parsing Strides
   */
  if(netStructure.layer(i).convolution_param().stride_size() > 0)
  {
    convParams.strideW = netStructure.layer(i).convolution_param().stride(0);
    convParams.strideH = netStructure.layer(i).convolution_param().stride(0);
  }
  else
  {
    convParams.strideW = 1;
    convParams.strideH = 1;
  }

  if(netStructure.layer(i).convolution_param().has_stride_h())
  {
    convParams.strideH = netStructure.layer(i).convolution_param().stride_h();
  }

  if(netStructure.layer(i).convolution_param().has_stride_w())
  {
    convParams.strideW = netStructure.layer(i).convolution_param().stride_w();
  }

  /*
   * Parsing Kernel size
   */
  if(netStructure.layer(i).convolution_param().kernel_size_size() > 0)
  {
    convParams.kernelH = netStructure.layer(i).convolution_param().kernel_size(0);
    convParams.kernelW = netStructure.layer(i).convolution_param().kernel_size(0);
  }
  else
  {
    convParams.kernelH = 0;
    convParams.kernelW = 0;
  }

  if(netStructure.layer(i).convolution_param().has_kernel_h())
  {
    convParams.kernelH = netStructure.layer(i).convolution_param().kernel_h();
  }

  if(netStructure.layer(i).convolution_param().has_kernel_w())
  {
    convParams.kernelW = netStructure.layer(i).convolution_param().kernel_w();
  }

  if(convParams.kernelH == 0 || convParams.kernelW == 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Conv Layer %s -- Kernel Size not specified", netStructure.layer(i).name().c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }


  /*
   * Parsing Padding size
   */
  if(netStructure.layer(i).convolution_param().pad_size() > 0)
  {
    convParams.padW = netStructure.layer(i).convolution_param().pad(0);
    convParams.padH = netStructure.layer(i).convolution_param().pad(0);
  }
  else
  {
    convParams.padW = 0;
    convParams.padH = 0;
  }

  if(netStructure.layer(i).convolution_param().has_pad_h())
  {
    convParams.padH = netStructure.layer(i).convolution_param().pad_h();
  }

  if(netStructure.layer(i).convolution_param().has_pad_w())
  {
    convParams.padW = netStructure.layer(i).convolution_param().pad_w();
  }
  convParams.padT            = convParams.padH;
  convParams.padL            = convParams.padW;
  convParams.padB            = convParams.padH;
  convParams.padR            = convParams.padW;

  /*
   * Import coeff from caffemodel
   */
  TIDL_caffeCopyFloatInitializerTensor(netStructure, netParams, i, 0, TIDLPCLayers.weights);
  if(convParams.enableBias)
  {
    TIDL_caffeCopyFloatInitializerTensor(netStructure, netParams, i, 1, TIDLPCLayers.bias);
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_caffeMapArgmaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_ArgOpParams_t &argOpParams = TIDLPCLayers.layerParams.argOpParams;

  TIDLPCLayers.layerType = TIDL_ArgOpLayer;
  TIDLPCLayers.layerParams.argOpParams.argOpType = TIDL_ArgMax;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(0);

  int32_t axis = 1;
  if(netStructure.layer(i).argmax_param().has_axis())
  {
    axis = netStructure.layer(i).argmax_param().axis();
  }

  if(axis != TIDL_DIM_NUMCH)
  {
    TIDL_GLOBAL_REPORT_WARNING("Argmax layer axis must be equal to channel dimension. Any other value is not supported by current version and will be overwritten to channel dimension");
    axis = TIDL_DIM_NUMCH;
  }

  /* TIDL default value for selectLastIndex is 1*/
  argOpParams.selectLastIndex = 1;
  return 0;
}

int32_t TIDL_caffeMapBiasParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t dataSize;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_BatchNormLayer;
  TIDLPCLayers.isConverted = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  int32_t netParamsIndex;
  netParamsIndex = tidl_caffeIndexStructure2Param(netStructure, netParams, i);

  if(netParamsIndex != -1)
  {
    dataSize = netParams.layer(netParamsIndex).blobs(0).data_size();

    float * bias = (float*)my_malloc(dataSize*sizeof(float));
    float * scale = (float*)my_malloc(dataSize*sizeof(float));

    for (int j = 0; j < dataSize; j++)
    {
      scale[j] = 1.0;
      bias[j] = netParams.layer(netParamsIndex).blobs(0).data(j);
    }

    TIDLPCLayers.bias.ptr = bias;
    TIDLPCLayers.bias.bufSize = dataSize;
    TIDLPCLayers.weights.ptr = scale;
    TIDLPCLayers.weights.bufSize = dataSize;
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Bias layer %s params cannot be found in caffemodel, will generate random params",
           netStructure.layer(i).name().c_str());

    TIDLPCLayers.bias.ptr = NULL;
    TIDLPCLayers.bias.bufSize = 0;
    TIDLPCLayers.weights.ptr = NULL;
    TIDLPCLayers.weights.bufSize = 0;
  }


  return 0;
}

int32_t TIDL_caffeMapShuffleParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_ShuffleChannelLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  if(netStructure.layer(i).shuffle_channel_param().has_group())
  {
    TIDLPCLayers.layerParams.shuffleLayerParams.numGroups = netStructure.layer(i).shuffle_channel_param().group();
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Shuffle Layer %s group param is not found in prototxt, cannot proceed", netStructure.layer(i).name().c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_caffeMapEltwiseParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_EltWiseLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = netStructure.layer(i).bottom_size();
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  if(netStructure.layer(i).eltwise_param().has_operation())
  {
    TIDLPCLayers.layerParams.eltWiseParams.eltWiseType = netStructure.layer(i).eltwise_param().operation();
  }
  else
  {
    TIDLPCLayers.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSum;
  }

  return 0;
}

int32_t TIDL_caffeMapScaleParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_BatchNormLayer;
  TIDLPCLayers.isConverted = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  int32_t netParamsIndex;
  netParamsIndex = tidl_caffeIndexStructure2Param(netStructure, netParams, i);

  if(netParamsIndex != -1)
  {
    int32_t dataSize;
    dataSize = netParams.layer(netParamsIndex).blobs(0).data_size();

    float * bias = (float*)my_malloc(dataSize*sizeof(float));
    float * scale = (float*)my_malloc(dataSize*sizeof(float));

    for (int j = 0; j < dataSize; j++)
    {
      scale[j] = netParams.layer(netParamsIndex).blobs(0).data(j);
      if (netParams.layer(netParamsIndex).blobs_size() == 2)
      {
        bias[j] = netParams.layer(netParamsIndex).blobs(1).data(j);
      }
      else
      {
        bias[j] = 0;
      }
    }

    TIDLPCLayers.bias.ptr = bias;
    TIDLPCLayers.bias.bufSize = dataSize;
    TIDLPCLayers.weights.ptr = scale;
    TIDLPCLayers.weights.bufSize = dataSize;
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Scale layer %s params cannot be found in caffemodel, will generate random params", netStructure.layer(i).name().c_str());

    TIDLPCLayers.bias.ptr = NULL;
    TIDLPCLayers.bias.bufSize = 0;
    TIDLPCLayers.weights.ptr = NULL;
    TIDLPCLayers.weights.bufSize = 0;
  }

  return 0;
}

int32_t TIDL_caffeMapBatchNormParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int j;
  float eps = 1e-05;
  int32_t dataSize;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_BatchNormLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);

  int32_t netParamsIndex;
  netParamsIndex = tidl_caffeIndexStructure2Param(netStructure, netParams, i);

  if(netParamsIndex != -1)
  {
    dataSize = netParams.layer(netParamsIndex).blobs(0).data_size();

    float * mean = (float*)my_malloc(dataSize*sizeof(float));
    float * var = (float*)my_malloc(dataSize*sizeof(float));
    float * scale = (float*)my_malloc(dataSize*sizeof(float));
    float * bias = (float*)my_malloc(dataSize*sizeof(float));
    float * gama = (float*)my_malloc(dataSize*sizeof(float));
    float * beta = (float*)my_malloc(dataSize*sizeof(float));
    TIDLPCLayers.bias.ptr = bias;
    TIDLPCLayers.bias.bufSize = dataSize;
    TIDLPCLayers.weights.ptr = scale;
    TIDLPCLayers.weights.bufSize = dataSize;

    TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);

    eps = netParams.layer(netParamsIndex).batch_norm_param().eps();
    if (netParams.layer(netParamsIndex).blobs_size() == 5)
    {

      // old format: 0 - scale , 1 - bias,  2 - mean , 3 - var, 4 - reserved
      // new format: 0 - mean  , 1 - var,  2 - reserved , 3- scale, 4 - bias
      if (netParams.layer(netParamsIndex).blobs(4).data_size() == 1)
      {
        for (j = 0; j < dataSize; j++)
        {
          mean[j] = netParams.layer(netParamsIndex).blobs(2).data(j);
          var[j] = netParams.layer(netParamsIndex).blobs(3).data(j);
          scale[j] = netParams.layer(netParamsIndex).blobs(0).data(j);
          bias[j] = netParams.layer(netParamsIndex).blobs(1).data(j);
        }
      }
      else
      {
        for (j = 0; j < dataSize; j++)
        {
          mean[j] = netParams.layer(netParamsIndex).blobs(0).data(j);
          var[j] = netParams.layer(netParamsIndex).blobs(1).data(j);
          scale[j] = netParams.layer(netParamsIndex).blobs(3).data(j);
          bias[j] = netParams.layer(netParamsIndex).blobs(4).data(j);
        }
      }
    }
    else if (netParams.layer(netParamsIndex).blobs_size() == 3)
    {
      float scale_c = netParams.layer(netParamsIndex).blobs(2).data(0);
      scale_c = scale_c == 0 ? 0 : (1 / scale_c);
      for (j = 0; j < dataSize; j++)
      {
        mean[j]  = netParams.layer(netParamsIndex).blobs(0).data(j);
        var[j]   = netParams.layer(netParamsIndex).blobs(1).data(j);
        mean[j] *= scale_c;
        var[j]  *= scale_c;
        scale[j] = 1;
        bias[j]  = 0;
      }
    }
    else
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Unsupported number of blobs = %d for Batchnorm layer %s. Only 3 and 5 are supported", netParams.layer(netParamsIndex).blobs_size(), netStructure.layer(i).name().c_str());
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    for(j = 0; j < dataSize; j++)
    {
      gama[j] = 1;
      beta[j] = 0;
    }

    float * dataBias = (float*)my_malloc(dataSize*sizeof(float));
    float * dataWeigths = (float*)my_malloc(dataSize*sizeof(float));
    for (j = 0; j < dataSize; j++)
    {
      dataBias[j] = 0;
      dataWeigths[j] = 1;
    }
    TIDLPCLayers.bias.ptr = dataBias;
    TIDLPCLayers.bias.bufSize = dataSize;
    TIDLPCLayers.weights.ptr = dataWeigths;
    TIDLPCLayers.weights.bufSize = dataSize;

    TIDL_caffeConstructWeightBias(
      (float *)TIDLPCLayers.weights.ptr,
      (float *)TIDLPCLayers.bias.ptr,
      TIDLPCLayers.weights.bufSize,
      TIDLPCLayers.bias.bufSize,
      mean, var, scale, bias, eps);

    my_free(mean);
    my_free(var);
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Batchnorm layer %s params cannot be found in caffemodel, will generate random params", netStructure.layer(i).name().c_str());

    TIDLPCLayers.bias.ptr = NULL;
    TIDLPCLayers.bias.bufSize = 0;
    TIDLPCLayers.weights.ptr = NULL;
    TIDLPCLayers.weights.bufSize = 0;
  }



  return 0;
}

int32_t TIDL_caffeMapInnerProductParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_InnerProductLayer;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);

  TIDLPCLayers.layerParams.innerProductParams.numOutCols = netStructure.layer(i).inner_product_param().num_output();
  TIDLPCLayers.layerParams.innerProductParams.numInRows  = 1;
  TIDLPCLayers.layerParams.innerProductParams.inputATranspose = 0;
  TIDLPCLayers.layerParams.innerProductParams.inputBTranspose = 1;
  TIDLPCLayers.layerParams.innerProductParams.isBias          = 1;
  TIDLPCLayers.layerParams.innerProductParams.constIdx        = -1;


  int32_t netParamsIndex;
  netParamsIndex = tidl_caffeIndexStructure2Param(netStructure, netParams, i);

  if(netParamsIndex != -1)
  {
    int32_t dataSize;
    dataSize = netParams.layer(netParamsIndex).blobs(0).data_size();
    TIDLPCLayers.layerParams.innerProductParams.numInCols =
      dataSize / TIDLPCLayers.layerParams.innerProductParams.numOutCols;

    TIDLPCLayers.inData[0].dimValues[TIDL_DIM_BATCH]  = 1; // Is there better way to get batch, currently assuming 1
    TIDLPCLayers.inData[0].dimValues[TIDL_DIM_NUMCH]  = 1; // Is there better way to get channel, currently assuming 1
    TIDLPCLayers.inData[0].dimValues[TIDL_DIM_HEIGHT] = 1;
    TIDLPCLayers.inData[0].dimValues[TIDL_DIM_WIDTH]  = TIDLPCLayers.layerParams.innerProductParams.numInCols;

    TIDLPCLayers.layerParams.innerProductParams.numBatches = 1;

    float  * data = (float *)my_malloc(dataSize*sizeof(float));
    float  * bias = (float *)my_malloc(dataSize*sizeof(float));
    for (int idx = 0; idx < dataSize; idx++)
    {
      data[idx] = netParams.layer(netParamsIndex).blobs(0).data(idx);
    }

    TIDLPCLayers.weights.ptr = data;
    TIDLPCLayers.weights.bufSize = dataSize;

    if (netStructure.layer(i).inner_product_param().bias_term())
    {
      dataSize = netParams.layer(netParamsIndex).blobs(1).data_size();
      for (int idx = 0; idx < dataSize; idx++)
      {
        bias[idx] = netParams.layer(netParamsIndex).blobs(1).data(idx);
      }
      TIDLPCLayers.bias.ptr = bias;
      TIDLPCLayers.bias.bufSize = dataSize;
    }
    else
    {
      TIDL_GLOBAL_REPORT_WARNING("Bias for Inner Product Layer %s not found", netStructure.layer(i).name().c_str());
      TIDLPCLayers.bias.ptr = NULL;
      TIDLPCLayers.bias.bufSize = 0;
    }
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Inner Product layer %s params cannot be found in caffemodel, will generate random params", netStructure.layer(i).name().c_str());

    TIDLPCLayers.weights.ptr = NULL;
    TIDLPCLayers.weights.bufSize = 0;
    TIDLPCLayers.bias.ptr = NULL;
    TIDLPCLayers.bias.bufSize = 0;
  }

  return 0;
}

int32_t TIDL_caffeMapSplitParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_SliceLayerParams_t &sliceParams = TIDLPCLayers.layerParams.sliceParams;

  TIDLPCLayers.layerType = TIDL_SplitLayer;
  TIDLPCLayers.numOutBufs = netStructure.layer(i).top_size();
  TIDLPCLayers.numInBufs = 1;
  sliceParams.stride = 1;

  for (int32_t j = 0; j < TIDLPCLayers.numOutBufs; j++)
  {
    TIDLPCLayers.outData[j].dataId = dataIndex++;
  }

  return 0;
}

int32_t TIDL_caffeMapSliceParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{

  int j;
  int32_t NumSlices= netStructure.layer(i).top_size();
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_SliceLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = NumSlices;
  TIDLPCLayers.layerParams.sliceParams.slicePoints[0] = 0;
  TIDLPCLayers.outData[0].dataId = dataIndex++;
  TIDLPCLayers.layerParams.sliceParams.stride = 1;

  TIDLPCLayers.layerParams.sliceParams.axis = netStructure.layer(i).slice_param().axis();

  /** Here model dimensions are assumed as 4 but it has to be updated to TIDL_DIM_MAX so difference is added to axis
   *  Best way is to read the dimensions from model - numDim and add (TIDL_DIM_MAX - numDim), but most of the models in TA have no shape inference
   *  Hence assuming 4 dimensions by default
  */
  TIDLPCLayers.layerParams.sliceParams.axis += (TIDL_DIM_MAX - 4);

  if(TIDLPCLayers.layerParams.sliceParams.axis == 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Slice layer %s unsupported axis = %d", netStructure.layer(i).name().c_str(), TIDLPCLayers.layerParams.sliceParams.axis);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  if (netStructure.layer(i).slice_param().slice_point_size())
  {
    if(NumSlices != (netStructure.layer(i).slice_param().slice_point_size()+1))
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Slice layer %s top_size and slice_point_size are not matching: %d vs %d", netStructure.layer(i).name().c_str(), (NumSlices+1), netStructure.layer(i).slice_param().slice_point_size());
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    for (j = 0; j < netStructure.layer(i).slice_param().slice_point_size(); j++)
    {
      TIDLPCLayers.layerParams.sliceParams.slicePoints[j + 1] = netStructure.layer(i).slice_param().slice_point(j);
      TIDLPCLayers.outData[j + 1].dataId = dataIndex++;
    }
  }
  else
  {
    TIDLPCLayers.layerPCParams.sliceParams.setSlicePoints = 1;
    for (j = 0; j < NumSlices; j++)
    {
      TIDLPCLayers.outData[j].dataId = dataIndex++;
    }
  }
  TIDLPCLayers.layerParams.sliceParams.slicePoints[NumSlices] = -1;
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_caffeMapCropParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_CropLayer;
  TIDLPCLayers.numInBufs = netStructure.layer(i).bottom_size();
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  if (netStructure.layer(i).crop_param().axis() != 2)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Unsupported crop axis %d for Crop layer %s", netStructure.layer(i).crop_param().axis(), netStructure.layer(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  else if(netStructure.layer(i).crop_param().offset_size() == 1)
  {
    TIDLPCLayers.layerParams.cropParams.offsetH = netStructure.layer(i).crop_param().offset(0);
    TIDLPCLayers.layerParams.cropParams.offsetW = netStructure.layer(i).crop_param().offset(0);
  }
  else if(netStructure.layer(i).crop_param().offset_size() == 2)
  {
    TIDLPCLayers.layerParams.cropParams.offsetH = netStructure.layer(i).crop_param().offset(0);
    TIDLPCLayers.layerParams.cropParams.offsetW = netStructure.layer(i).crop_param().offset(1);
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Crop Layer %s has offset = %d, Crop parameter should have 1 or 2 offset", netStructure.layer(i).name().c_str(), netStructure.layer(i).crop_param().offset_size());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }


  return 0;
}

int32_t TIDL_caffeMapFlattenParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_FlattenLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  return 0;
}

int32_t TIDL_caffeMapPermuteParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t& TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_PermuteLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  return 0;
}

int32_t TIDL_caffeMapPriorBoxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t i1;
  sTIDL_LayerPC_t& TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_PriorBoxLayer;
  TIDLPCLayers.numInBufs = netStructure.layer(i).bottom_size();
  TIDLPCLayers.numOutBufs = 1;

  for(i1 = 0; i1 < TIDLPCLayers.numOutBufs; i1++)
  {
    TIDLPCLayers.outData[i1].dataId = dataIndex++;
  }

  int32_t     status;
  int32_t id;
  int32_t j, img_height, img_width, num_priors;
  int32_t layer_height, layer_width;
  float step_h, step_w;


  int32_t aspect_ratios_size = netStructure.layer(i).prior_box_param().aspect_ratio_size();
  float  * aspect_ratios   = (float *)my_calloc((aspect_ratios_size*3), sizeof(float));
  bool flip = netStructure.layer(i).prior_box_param().flip();
  int ptr_offset = 0;
  aspect_ratios[ptr_offset++] = 1.0;
  for (int idx = 0; idx < aspect_ratios_size; idx++)
  {
    float ar = netStructure.layer(i).prior_box_param().aspect_ratio(idx);
    bool already_exist = false;
    for (int j = 0; j < ptr_offset; ++j)
    {
      if (fabs(ar - aspect_ratios[j]) < 1e-6)
      {
        already_exist = true;
        break;
        }
    }
    if (!already_exist)
    {
      aspect_ratios[ptr_offset++] = ar;
      if (flip)
      {
        aspect_ratios[ptr_offset++] = (1./ar);
      }
    }
  }
  aspect_ratios_size = ptr_offset;

  int32_t min_sizes_size = netStructure.layer(i).prior_box_param().min_size_size();
  float  * min_sizes   = (float *)my_malloc(min_sizes_size*sizeof(float));
  for (int idx = 0; idx < min_sizes_size; idx++)
  {
    min_sizes[idx] = netStructure.layer(i).prior_box_param().min_size(idx);
  }

  int32_t max_sizes_size = netStructure.layer(i).prior_box_param().max_size_size();
  float  * max_sizes   = (float *)my_malloc(max_sizes_size*sizeof(float));
  num_priors = aspect_ratios_size * min_sizes_size;
  for (int idx = 0; idx < max_sizes_size; idx++)
  {
    max_sizes[idx] = netStructure.layer(i).prior_box_param().max_size(idx);
    num_priors += 1;
  }
  float offset = netStructure.layer(i).prior_box_param().offset();
  bool  clip = netStructure.layer(i).prior_box_param().clip();

  /* First input points to Conv layer output data */
  layer_height = -1;// Reshape in tidl_import_common.cpp
  layer_width  = -1;// Reshape in tidl_import_common.cpp

  if (netStructure.layer(i).prior_box_param().has_img_h() || netStructure.layer(i).prior_box_param().has_img_w())
  {
    img_height = netStructure.layer(i).prior_box_param().img_h();
    img_width = netStructure.layer(i).prior_box_param().img_w();
  }
  else if (netStructure.layer(i).prior_box_param().has_img_size())
  {
    int32_t img_size = netStructure.layer(i).prior_box_param().img_size();
    img_height = img_size;
    img_width = img_size;
  }
  else if (netStructure.input_shape_size() != 0)
  {
    /* Second input points to input data */
    img_height =  netStructure.input_shape(0).dim(2);// Reshape in tidl_import_common.cpp mergeDetectionOut
    img_width  =  netStructure.input_shape(0).dim(3);// Reshape in tidl_import_common.cpp mergeDetectionOut

    TIDL_GLOBAL_REPORT_WARNING("Image dimensions is not provided, Please provide it as part of prior_box_param in form of either (img_w & img_h) or img_size. Proceeding with img_w = %d and img_h = %d in prior box decoding", img_width, img_height);

  }
  else
  {
    int32_t j;
    for(j = 0; j < i; j++)
    { /*Assuming that input layer would have come till now*/
      if(netStructure.layer(j).has_input_param())
      {
        break;
      }
    }
    img_height =  netStructure.layer(j).input_param().shape(0).dim(2);// Reshape in tidl_import_common.cpp mergeDetectionOut
    img_width  =  netStructure.layer(j).input_param().shape(0).dim(3);// Reshape in tidl_import_common.cpp mergeDetectionOut

    TIDL_GLOBAL_REPORT_WARNING("Image dimensions is not provided, Please provide it as part of prior_box_param in form of either (img_w & img_h) or img_size. Proceeding with img_w = %d and img_h = %d in prior box decoding", img_width, img_height);

  }

  if (netStructure.layer(i).prior_box_param().has_step_h() || netStructure.layer(i).prior_box_param().has_step_w())
  {
    step_h = netStructure.layer(i).prior_box_param().step_h();
    step_w = netStructure.layer(i).prior_box_param().step_w();
  }
  else if (netStructure.layer(i).prior_box_param().has_step())
  {
    float step = netStructure.layer(i).prior_box_param().step();
    step_h = step;
    step_w = step;
  }
  else // Step Not found in the net structure decoding from base size
  {
    step_h = -1;//((float)img_height)/layer_height;
    step_w = -1;//((float)img_width)/layer_width;
  }

  int32_t num_keypoint;
  if (netStructure.layer(i).prior_box_param().has_num_keypoint())
  {
    num_keypoint = netStructure.layer(i).prior_box_param().num_keypoint();
  }
  else
  {
    num_keypoint = 0;
  }

  int32_t variance_size = netStructure.layer(i).prior_box_param().variance_size();
  float  * variance   = (float *)my_malloc((variance_size+1)*sizeof(float));
  if(variance_size)
  {
    for (int idx = 0; idx < variance_size; idx++)
    {
       variance[idx] = netStructure.layer(i).prior_box_param().variance(idx);
    }
    if (variance_size != (4 + 2 * num_keypoint))
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "variance_size = %d is not matching with the expected size = %d", variance_size, (4 + 2 * num_keypoint));
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  else
  {
    // Set default to 0.1.
    variance_size = 1;
    variance[0] = 1.0;
  }

  sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(1, sizeof(sTIDL_AnchorBoxParams_t));

  // set the variance.
  anchorBox->numAnchors = num_priors;
  anchorBox->numKeyPoints = num_keypoint;
  anchorBox->headWidth = layer_width;
  anchorBox->headHeight = layer_height;
  anchorBox->strideW = step_w;
  anchorBox->strideH = step_h;
  anchorBox->offsetH = offset;
  anchorBox->offsetW = offset;

  for (int j = 0; j < 4; ++j)
  {
    if (variance_size == 1)
    {
      anchorBox->boxScales[j] =variance[0];
    }
    else
    {
      anchorBox->boxScales[j] =variance[j];
    }
  }

  for (int j = 0; j < num_keypoint; ++j)
  {
    if (variance_size == 1)
    {
      anchorBox->kpScales[j*2] = variance[0];
      anchorBox->kpScales[j*2 + 1] = variance[0];
    }
    else
    {
      anchorBox->kpScales[j*2] =variance[4 + j*2];
      anchorBox->kpScales[j*2 + 1] =variance[4 + j*2 + 1];
    }
  }

  float box_width, box_height;
  int32_t box_counter = 0;
  for (int s = 0; s < min_sizes_size; ++s)
  {
      float min_size = min_sizes[s];
      // first prior: aspect_ratio = 1, size = min_size
      box_width = box_height = min_size;
      // xmin
      anchorBox->boxWidth[box_counter] = box_width/img_width;
      // ymin
      anchorBox->boxHeight[box_counter] = box_height/img_height;
      box_counter++;

      if (max_sizes_size > 0)
      {
        //CHECK_EQ(min_sizes_.size(), max_sizes_.size());
        int max_size = max_sizes[s];
        float size  = min_size * max_size;
        // second prior: aspect_ratio = 1, size = sqrt(min_size * max_size)
        box_width = box_height = sqrt(size);
        // xmin
        anchorBox->boxWidth[box_counter] = box_width/img_width;
        // ymin
        anchorBox->boxHeight[box_counter] = box_height/img_height;
        box_counter++;
      }

      // rest of priors
      for (int r = 0; r < aspect_ratios_size; ++r)
      {
        float ar = aspect_ratios[r];
        if (fabs(ar - 1.) < 1e-6)
        {
          continue;
        }
        box_width = min_size * sqrt(ar);
        box_height = min_size / sqrt(ar);
        // xmin
        anchorBox->boxWidth[box_counter] = box_width/img_width;
        // ymin
        anchorBox->boxHeight[box_counter] = box_height/img_height;
        box_counter++;
      }
    }

  TIDLPCLayers.priorBox.ptr = anchorBox;
  TIDLPCLayers.priorBox.bufSize = sizeof(sTIDL_AnchorBoxParams_t);
  TIDLPCLayers.layerParams.detectOutParams.priorBoxSize = sizeof(sTIDL_AnchorBoxParams_t);


  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_caffeMapReshapeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  sTIDL_LayerPC_t& TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_ReshapeLayer;
  TIDLPCLayers.numInBufs = 1;
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  int32_t netParamsIndex;
  netParamsIndex = tidl_caffeIndexStructure2Param(netStructure, netParams, i);

  if(netParamsIndex != -1)
  {
    int32_t dataSize;
    dataSize = netParams.layer(netParamsIndex).blobs(0).data_size();

    float  * data = (float *)my_malloc(dataSize*sizeof(float));
    for (int idx = 0; idx < dataSize; idx++)
    {
      data[idx] = netParams.layer(netParamsIndex).blobs(0).data(idx);
    }

    TIDLPCLayers.weights.ptr = data;
    TIDLPCLayers.weights.bufSize = dataSize;
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Reshape layer %s params cannot be found in caffemodel, will generate random params", netStructure.layer(i).name().c_str());
    TIDLPCLayers.weights.ptr = NULL;
    TIDLPCLayers.weights.bufSize = 0;
  }
  return 0;
}

int32_t TIDL_caffeMapDetectionOutputParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t  status;
  int32_t  id;
  int32_t  j, num_classes, num_priors;
  int32_t  num_loc_classes;
  int32_t  background_label_id;
  int32_t  code_type, nms_type;
  float    confidence_threshold;
  float    nms_threshold, eta;
  int32_t  top_k, keep_top_k;
  bool     share_location, clip_bbox;
  bool     variance_encoded_in_target;
  sTIDL_LayerPC_t& TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_DetectionOutputLayer;
  TIDLPCLayers.numInBufs = netStructure.layer(i).bottom_size();
  TIDLPCLayers.numOutBufs = 1;
  TIDLPCLayers.outData[0].dataId = dataIndex++;

  if (netStructure.layer(i).detection_output_param().has_num_keypoint())
  {
    TIDLPCLayers.layerParams.detectOutParams.numKeypoints =
        netStructure.layer(i).detection_output_param().num_keypoint();
  }
  else
  {
    TIDLPCLayers.layerParams.detectOutParams.numKeypoints = 0;
  }

  TIDLPCLayers.layerParams.detectOutParams.metaArchType = TIDL_metaArchCaffeJacinto;

  num_classes = netStructure.layer(i).detection_output_param().num_classes();
  TIDLPCLayers.layerParams.detectOutParams.numClasses = num_classes;

  share_location = netStructure.layer(i).detection_output_param().share_location();
  TIDLPCLayers.layerParams.detectOutParams.shareLocation = share_location;

  background_label_id = netStructure.layer(i).detection_output_param().background_label_id();
  TIDLPCLayers.layerParams.detectOutParams.backgroundLabelId = background_label_id;

  code_type = netStructure.layer(i).detection_output_param().code_type();
  TIDLPCLayers.layerParams.detectOutParams.codeType = code_type;

  variance_encoded_in_target =
      netStructure.layer(i).detection_output_param().variance_encoded_in_target();
  TIDLPCLayers.layerParams.detectOutParams.varianceEncoded = variance_encoded_in_target;

  keep_top_k = netStructure.layer(i).detection_output_param().keep_top_k();
  TIDLPCLayers.layerParams.detectOutParams.keepTopK = keep_top_k;

  confidence_threshold = netStructure.layer(i).detection_output_param().has_confidence_threshold() ?
      netStructure.layer(i).detection_output_param().confidence_threshold() : -FLT_MAX;
  TIDLPCLayers.layerParams.detectOutParams.confThreshold = confidence_threshold;

  // Parameters used in nms.
  nms_type = netStructure.layer(i).detection_output_param().nms_param().nms_type();
  TIDLPCLayers.layerParams.detectOutParams.reserve2 = nms_type; // reserve2:: nmsType (need to add the variable "nmsType" in arm-tidl)

  nms_threshold = netStructure.layer(i).detection_output_param().nms_param().nms_threshold();
  TIDLPCLayers.layerParams.detectOutParams.nmsThreshold = nms_threshold;

  eta = netStructure.layer(i).detection_output_param().nms_param().eta();
  TIDLPCLayers.layerParams.detectOutParams.eta = eta;

  top_k = -1;
  if (netStructure.layer(i).detection_output_param().nms_param().has_top_k())
  {
    top_k = netStructure.layer(i).detection_output_param().nms_param().top_k();
  }
  TIDLPCLayers.layerParams.detectOutParams.topK = top_k;

  TIDLPCLayers.layerParams.detectOutParams.topKAllClasses = -1;

  /*Checking if revious two layer has softmax or sigmoid layer then set the scoreConverter appropriately*/
  if(netStructure.layer(i-1).has_softmax_param() || netStructure.layer(i-2).has_softmax_param())
  {
    TIDLPCLayers.layerParams.detectOutParams.scoreConverter = TIDL_scoreConverterSOFTMAX;
  }
  else if(netStructure.layer(i-1).has_sigmoid_param() || netStructure.layer(i-2).has_sigmoid_param())
  {
    TIDLPCLayers.layerParams.detectOutParams.scoreConverter = TIDL_scoreConverterSIGMOID;
  }
  else
  {
    TIDLPCLayers.layerParams.detectOutParams.scoreConverter = TIDL_scoreConverterSOFTMAX;
  }
  TIDLPCLayers.layerParams.detectOutParams.dataLayout = 0;

  return 0;
}

int32_t TIDL_caffeMapUnSuportedlayerParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t&             layerIndex,
  int32_t&             dataIndex,
  NetParameter&        netStructure,
  NetParameter&        netParams)
{
  int32_t i1;
  sTIDL_LayerPC_t& TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];

  TIDLPCLayers.layerType = TIDL_UnsupportedLayer;
  TIDLPCLayers.numInBufs = netStructure.layer(i).bottom_size();
  TIDLPCLayers.numOutBufs = netStructure.layer(i).top_size();

  for(i1 = 0; i1 < TIDLPCLayers.numOutBufs; i1++)
  {
    TIDLPCLayers.outData[i1].dataId = dataIndex++;
  }

  return 0;
}
