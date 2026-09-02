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
#include <float.h>
#include <cmath>

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tidl_import_config.h"
#include "tidl_custom_import.h"
#include "tidl_import_diag.h"

#define __MODULE__ "[PARSER]"

using namespace std;
using namespace tensorflow;
using ::google::protobuf::Message;
using ::google::protobuf::io::FileInputStream;
using ::google::protobuf::io::FileOutputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;
using ::google::protobuf::io::CodedOutputStream;

#include "tidl_import_common.h"

#define IS_SIGNED_DATA (1)
#define QUAN_STYLE2_ROUND (0.5)
extern sTIDL_OrgNetwork_t      orgTIDLNetStructure;
extern sTIDL_OrgNetwork_t      tempTIDLNetStructure;
extern sTIDL_Network_t         tIDLNetStructure;

#define ENABLE_BIN_PARSE_PRINT  (0)
int32_t gloab_data_format = -1;

// This version reshape from HWIO kernel/weights layout to OIHW
uint32_t TIDL_kernelReshape(float * param, uint32_t w, uint32_t h, uint32_t ci, uint32_t co)
{
  uint32_t i0, i1, i2, i3;
  float * tPtr = (float * )my_malloc(w*h*ci*co*sizeof(float));

	for(i0 = 0; i0 < co; i0++)
	{
	  for(i1 = 0; i1 < ci; i1++)
	  {
	    for(i2 = 0; i2 < h; i2++)
	    {
	      for(i3 = 0; i3 < w; i3++)
		  {
		    tPtr[i0*ci*h*w + i1*h*w + i2*w + i3] = param[i2*w*ci*co + i3*ci*co + i1*co + i0];
		  }
		}
	  }
	}
  memcpy(param,tPtr,w*h*ci*co*sizeof(float));
  free(tPtr);
  return 0;
}
// This version reshape from HWOI kernel/weights layout to OIHW
uint32_t TIDL_kernelReshapeHWOI(float *param, uint32_t w, uint32_t h,
                                uint32_t ci, uint32_t co)
{
  uint32_t i0, i1, i2, i3;
  float * tPtr = (float * )my_malloc(w*h*ci*co*sizeof(float));

  for(i0 = 0; i0 < co; i0++)
    for(i1 = 0; i1 < ci; i1++)
      for(i2 = 0; i2 < h; i2++)
        for(i3 = 0; i3 < w; i3++)
          tPtr[i0*ci*h*w + i1*h*w + i2*w + i3] =
                                     param[i2*w*co*ci + i3*co*ci + i0*ci + i1];

  memcpy(param,tPtr,w*h*ci*co*sizeof(float));
  free(tPtr);
  return 0;
}
// This version reshape from IOHW kernel/weights layout to OIHW
uint32_t TIDL_kernelReshapeIOHW(float *param, uint32_t w, uint32_t h,
                                uint32_t ci, uint32_t co)
{
  uint32_t i0, i1, i2, i3;
  float * tPtr = (float * )my_malloc(w*h*ci*co*sizeof(float));

  for(i0 = 0; i0 < co; i0++)
    for(i1 = 0; i1 < ci; i1++)
      for(i2 = 0; i2 < h; i2++)
        for(i3 = 0; i3 < w; i3++)
          tPtr[i0*ci*h*w + i1*h*w + i2*w + i3] =
                                     param[i1*co*h*w + i0*h*w + i2*w + i3];

  memcpy(param,tPtr,w*h*ci*co*sizeof(float));
  free(tPtr);
  return 0;
}
uint32_t TIDL_kernelScale(float * param, float * scale, uint32_t w, uint32_t h, uint32_t ci, uint32_t co)
{
  uint32_t i0, i1, i2, i3;
  for(i0 = 0; i0 < co; i0++)
  {
    for(i1 = 0; i1 < ci; i1++)
    {
      for(i2 = 0; i2 < h; i2++)
      {
        for(i3 = 0; i3 < w; i3++)
        {
          param[i2*w*ci*co + i3*ci*co + i1*co + i0] *= scale[i0];
        }
      }
    }
  }
  return 0;
}
uint32_t TIDL_depthWiseKernelScale(float * param, float * scale, uint32_t k, uint32_t c)
{
  uint32_t i0, i1;
  for(i0 = 0; i0 < c; i0++)
  {
    for(i1 = 0; i1 < k; i1++)
    {
      param[i1*c + i0] *= scale[i0];
    }
  }
  return 0;
}


TensorProto TIDL_getConstTensor(GraphDef& tfGraphDef, const string name)
{
  int i;

  for (i = 0; i < tfGraphDef.node_size(); i++)
  {
    if((strcmp(tfGraphDef.node(i).name().c_str(),name.c_str()) == 0))
    {
      if((strcmp(tfGraphDef.node(i).op().c_str(),"Const") == 0))
      {
        if(tfGraphDef.node(i).attr().at(std::string("value")).has_tensor())
        {
          auto & tensor = tfGraphDef.node(i).attr().at(std::string("value")).tensor();
          return(tensor);
        }
      }
      else if((strcmp(tfGraphDef.node(i).op().c_str(),"Identity") == 0))
      {
        return(TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(i).input(0)));
      }
    }
  }
  return (tensorflow::TensorProto::default_instance());
}


int32_t TIDL_hasAttr(const NodeDef& node, char * name)
{
  for (auto&& it = node.attr().begin(); it != node.attr().end();)
  {
    auto& map = it->first;
    if(strcmp(map.c_str(),name) == 0)
    {
      return (1);
    }
    it++;
  }
  return (0);
}

int32_t TIDL_getAttr_type(const NodeDef& node, char * name, int32_t * type)
{
  if(TIDL_hasAttr(node,name))
  {
    auto& value = node.attr().at(std::string(name));
    if(value.type() == DT_UINT8)
    {
      *type = TIDL_UnsignedChar;
    }
    else if(value.type() == DT_INT8)
    {
      *type = TIDL_SignedChar;
    }
    *type = TIDL_UnsignedChar;
    return (1);
  }
  return (0);
}

int32_t TIDL_getAttr_padding(const NodeDef& node, char * name, int32_t * padType)
{
  if(TIDL_hasAttr(node,name))
  {
    auto& value = node.attr().at(std::string(name));
    if(strcmp(value.s().c_str(),"SAME") == 0)
    {
      *padType = 0;
    }
    else if(strcmp(value.s().c_str(),"VALID") == 0)
    {
      *padType = 1;
    }
    else
    {
      *padType = -1;
      // using INFO diagnosis here
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Unsupported padding type ");
    }
    return (1);
  }
  return (0);
}
int32_t TIDL_getAttr_data_format(const NodeDef& node, char * name)
{
  if(TIDL_hasAttr(node,name))
  {
    auto& value = node.attr().at(std::string(name));
    if(strcmp(value.s().c_str(),"NHWC") == 0)
    {
      if(gloab_data_format == -1)
      {
        gloab_data_format = 0;
      }
      else if(gloab_data_format != 0)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "data_format is not common accross all the layers ");
      }
    }
    else if(strcmp(value.s().c_str(),"NCHW") == 0)
    {
      if(gloab_data_format == -1)
      {
        gloab_data_format = 1;
      }
      else if(gloab_data_format != 1)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "data_format is not common accross all the layers ");
      }
    }
    else
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Unsupported data_format ");
    }
    return (1);
  }
  return (0);
}
int32_t TIDL_getAttr_value(const NodeDef& node, char * name, int32_t * valuePtr, int32_t idx)
{
  if(TIDL_hasAttr(node,name))
  {
    auto& value = node.attr().at(std::string(name));
    *valuePtr  = (int32_t)value.list().i(idx);

    return (1);
  }
  return (0);
}

int32_t TIDL_getAttr_bool(const NodeDef& node, char * name, bool * valuePtr)
{
  if(TIDL_hasAttr(node,name))
  {
    auto& value = node.attr().at(std::string(name));
    *valuePtr  = (bool)value.b();

    return (1);
  }
  return (0);
}

int32_t TIDL_getAttr_float(const NodeDef& node, char * name, float * valuePtr, int32_t idx)
{
  if (TIDL_hasAttr(node, name))
  {
    auto& value = node.attr().at(std::string(name));
    if (value.has_list())
    {
      *valuePtr = value.list().f(idx);
    }
    else if(idx == 0)
    {
      *valuePtr = value.f();
    }
    else
    {
      return (0);
    }
    return (1);
  }
  return (0);
}



int32_t TIDL_tfCopyInputConstTensor(GraphDef& tfGraphDef, int32_t nIdx, int32_t inIdx, sBuffer_t &buf)
{
  TensorProto tensor = TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(nIdx).input(inIdx));
  float * tPtr = (float *)tensor.tensor_content().c_str();
  buf.bufSize = tensor.tensor_content().size() / sizeof(float);
  buf.ptr = (float *)my_malloc(buf.bufSize*sizeof(float));
  if ((tensor.dtype() == DT_FLOAT) || (tensor.dtype() == DT_INT32)) //DT_FLOAT
  {
    if(tensor.dtype() == DT_FLOAT)
    {
      buf.dataType = TIDL_SinglePrecFloat;
    }
    else if(tensor.dtype() == DT_INT32)
    {
      buf.dataType = TIDL_SignedWord;
    }
    memcpy(buf.ptr, tPtr, sizeof(float)*buf.bufSize);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Only float and DT_INT32 tensor is supported ");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
}

int32_t TIDL_tfMapPlaceHolderParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_DataLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs = -1;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  return 0;
}

int32_t TIDL_tfMapConvParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_ConvParams_t &convParams      = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.convParams;

  TIDLPCLayers.layerType = TIDL_ConvolutionLayer;
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  TIDLPCLayers.outData[0].elementType = tidl_getElementType(1);

  TensorProto tensor         = TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(i).input(1));
  convParams.numInChannels   = tensor.tensor_shape().dim(2).size();
  convParams.numOutChannels  = tensor.tensor_shape().dim(3).size();
  convParams.kernelW         = tensor.tensor_shape().dim(0).size();
  convParams.kernelH         = tensor.tensor_shape().dim(1).size();

  convParams.numGroups       = 1;
  convParams.dilationW       = 1;
  convParams.dilationH       = 1;
  convParams.strideW         = 1;
  convParams.strideH         = 1;
  convParams.padW            = 0;
  convParams.padH            = 0;
  convParams.padL            = 0;
  convParams.padR            = 0;
  convParams.padT            = 0;
  convParams.padB            = 0;
  convParams.enableBias      = 0;
  convParams.enablePooling   = 0;
  TIDL_getAttr_data_format(tfGraphDef.node(i), "data_format");
  if (gloab_data_format == 1)
  {
    idx1 = 3;
    idx2 = 2;
  }
  else
  {
    idx1 = 2;
    idx2 = 1;
  }
  TIDL_getAttr_value(tfGraphDef.node(i), "strides", &convParams.strideW, idx1);
  TIDL_getAttr_value(tfGraphDef.node(i), "strides", &convParams.strideH, idx2);
  TIDL_getAttr_value(tfGraphDef.node(i), "dilation_rate", &convParams.dilationW, idx1);
  TIDL_getAttr_value(tfGraphDef.node(i), "dilation_rate", &convParams.dilationH, idx2);

  TIDL_getAttr_padding(tfGraphDef.node(i), "padding", &padType);
  TIDLPCLayers.strideOffsetMethod = TIDL_StrideOffsetCenter;
  if (padType == TF_PAD_SAME)   /* SAME : Padding done to input */
  {
    convParams.padW = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    convParams.padH = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
    convParams.padL = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    convParams.padT = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
    convParams.padR = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    convParams.padB = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
  }
  else if (padType == TF_PAD_VALID)  /* VALID : No padding to input */
  {
    TIDLPCLayers.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    convParams.padL            = 0;
    convParams.padR            = 0;
    convParams.padT            = 0;
    convParams.padB            = 0;
  }

  if ((strcmp(tfGraphDef.node(i).op().c_str(), "Conv2D") == 0) ||
      (strcmp(tfGraphDef.node(i).op().c_str(), "Conv2DTranspose") == 0))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1, TIDLPCLayers.weights), "");
    TIDL_kernelReshape((float *)TIDLPCLayers.weights.ptr, convParams.kernelW, convParams.kernelH, convParams.numInChannels, convParams.numOutChannels);
  }

  return 0;
}


int32_t TIDL_tfMapDWConvParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_ConvParams_t &convParams = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.convParams;
  TIDL_tfMapConvParams(pOrgTIDLNetStructure, i, layerIndex, dataIndex, tfGraphDef);
  if (convParams.numOutChannels != 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "DW Convolution with depth multiplier > 1 is not suported now");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  convParams.numGroups      =
  convParams.numOutChannels =
  convParams.numInChannels;

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1, TIDLPCLayers.weights), "");
  TIDL_kernelReshape((float *)TIDLPCLayers.weights.ptr, convParams.kernelW, convParams.kernelH,
    convParams.numInChannels/ convParams.numGroups, convParams.numOutChannels);
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_tfMapDeconv2DParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDL_tfMapConvParams(pOrgTIDLNetStructure, i, layerIndex, dataIndex, tfGraphDef);
  TIDLPCLayers.layerType = TIDL_Deconv2DLayer;
  return 0;
}

int32_t TIDL_tfMapBiasAddParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_BiasLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1, pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].bias), "");
  return 0;
}

int32_t TIDL_tfMapAddParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_EltWiseLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSum;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs = tfGraphDef.node(i).input_size();

  return 0;
}

int32_t TIDL_tfMapMulParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ScaleLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  return 0;
}

void TIDL_tfBNToScaleBias(
  float    * scale,
  float    * bias,
  uint32_t  numCh,
  float * mean,
  float * var,
  float * gamma,
  float * beta,
  float eps
  )

{
  uint32_t j;
  if( mean == NULL ||  var == NULL || gamma == NULL || beta == NULL )
  {
    return;
  }
  for (j = 0; j < numCh; j++)
  {
    double m = mean[j];
    double v = var[j];
    double s = gamma[j];
    double b = beta[j];
    double inv_var = pow((eps + v), -0.5);
    scale[j] = (s)*inv_var;
    bias[j]  = (((-m)*s)*inv_var) + b;
  }
}
int32_t TIDL_tfGetNodeIdx(GraphDef &tfGraphDef, const char *bufName)
{
  int32_t i, nameLen, nodeIdx = -1;
  char nodeName[TIDL_MAX_DATA_NAME];
  char inDataName[TIDL_MAX_DATA_NAME];
  for (i = 0; i < tfGraphDef.node_size(); i++)
  {
    strcpy(nodeName, tfGraphDef.node(i).name().c_str());
    strcpy(inDataName, bufName);
    nameLen = strlen(nodeName);
    if (((strlen(inDataName) > nameLen)) && (inDataName[nameLen] == ':'))
    {
      inDataName[nameLen] = '\0';
    }
    if (strcmp(nodeName, inDataName) == 0)
    {
      nodeIdx = i;
      break;
    }
  }
  return nodeIdx;
}
int32_t TIDL_tfMapFusedBnParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  sBuffer_t gamma;
  sBuffer_t beta;
  sBuffer_t mean;
  sBuffer_t variance;
  float epsilon;

  int32_t dataSize;

  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_BatchNormLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;

  TIDL_getAttr_float(tfGraphDef.node(i), "epsilon", &epsilon, 0);
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1, gamma), "");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 2, beta), "");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 3, mean), "");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 4, variance), "");

  dataSize = gamma.bufSize;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weights.ptr     = my_malloc(dataSize*sizeof(float));
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weights.bufSize = dataSize;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].bias.ptr        = my_malloc(dataSize*sizeof(float));
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].bias.bufSize    = dataSize;

  TIDL_tfBNToScaleBias((float *)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weights.ptr,
                       (float *)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].bias.ptr, dataSize,
                       (float *)mean.ptr, (float *)variance.ptr, (float *)gamma.ptr, (float *)beta.ptr, epsilon
                      );
  my_free(mean.ptr);
  my_free(variance.ptr);
  my_free(gamma.ptr);
  my_free(beta.ptr);
  return 0;
}

int32_t TIDL_tfMapReluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ReLULayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].actParams.actType = TIDL_RelU;
  return 0;
}

int32_t TIDL_tfMapRelu6Params(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  TIDL_tfMapReluParams(pOrgTIDLNetStructure, i, layerIndex, dataIndex, tfGraphDef);
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].actParams.actType = TIDL_RelU6;
  return 0;
}

int32_t TIDL_tfMapSigmoidParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_SigmoidLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].actParams.actType = TIDL_Sigmoid;
  return 0;
}

int32_t TIDL_tfMapTanhParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t status = 0;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_TanhLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].actParams.actType = TIDL_Tanh;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  return status;
}

int32_t TIDL_tfMapELUParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t status = 0;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ELULayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].actParams.actType = TIDL_ELU;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  return status;
}

int32_t TIDL_tfMapHardSigmoidParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t status = 0;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_HardSigmoidLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].actParams.actType = TIDL_HardSigmoid;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  return status;
}

int32_t TIDL_tfMapMaxPoolParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_PoolingParams_t &poolParams = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.poolParams;
  sTIDL_PoolPCParams_t &poolPCParams = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.poolParams;

  TIDLPCLayers.layerType = TIDL_PoolingLayer;
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  poolParams.poolingType = TIDL_MaxPooling;
  TIDL_getAttr_data_format(tfGraphDef.node(i), "data_format");
  if (gloab_data_format == 1)
  {
    idx1 = 3;
    idx2 = 2;
  }
  else
  {
    idx1 = 2;
    idx2 = 1;
  }

  poolParams.useCeil = 0; // by default tf uses floor for reshape
  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padB = 0;
  poolParams.padT = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;
  poolPCParams.originalPadT = 0;
  poolPCParams.originalPadB = 0;
  poolPCParams.originalPadL = 0;
  poolPCParams.originalPadR = 0;
  TIDL_getAttr_value(tfGraphDef.node(i), "strides", &poolParams.strideW, idx1);
  TIDL_getAttr_value(tfGraphDef.node(i), "strides", &poolParams.strideH, idx2);
  TIDL_getAttr_value(tfGraphDef.node(i), "ksize",   &poolParams.kernelW, idx1);
  TIDL_getAttr_value(tfGraphDef.node(i), "ksize",   &poolParams.kernelH, idx2);

  TIDL_getAttr_padding(tfGraphDef.node(i), "padding", &padType);
  if (padType == 0)
  {
    /* Handles "SAME" pool */
    poolParams.padW = ((poolParams.kernelW - 1)) / 2;
    poolParams.padH = ((poolParams.kernelH - 1)) / 2;
    poolParams.padT = ((poolParams.kernelH - 1)) / 2;
    poolParams.padB = ((poolParams.kernelH - 1)) / 2;
    poolParams.padL = ((poolParams.kernelW - 1)) / 2;
    poolParams.padR = ((poolParams.kernelW - 1)) / 2;
    poolPCParams.originalPadT = poolParams.padT;
    poolPCParams.originalPadB = poolParams.padB;
    poolPCParams.originalPadL = poolParams.padL;
    poolPCParams.originalPadR = poolParams.padR;
  }

  return 0;
}

int32_t TIDL_tfMapAvgPoolParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{

  TIDL_tfMapMaxPoolParams(pOrgTIDLNetStructure, i, layerIndex, dataIndex, tfGraphDef);
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.poolParams.poolingType = TIDL_AveragePooling;
  return 0;
}

int32_t TIDL_tfMapArgmaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ArgOpLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.argOpParams.selectLastIndex = 0;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.argOpParams.selectLastIndex = 0;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.argOpParams.argOpType = TIDL_ArgMax;
  if(!TIDL_isSupportedInFirmwareVersion((char*)gParams.c7xFirmwareVersion, "11_00_07_00"))
  {
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.argOpParams.selectLastIndex = 1;
  }
  if (tfGraphDef.node(i).input_size() > 1)
  {
    TensorProto tensor = TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(i).input(1));
    auto& axis = tensor.int_val();
    if (((int)axis.Get(0) != 3) &&  ((int)axis.Get(0) != -1))
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "ArgMax/Min axis is not supported");
      /* needs review */
      return(-1);  // No error - this op is supported
    }
  }
  return 0;
}

int32_t TIDL_tfMapMeanParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  sTIDL_PoolingParams_t &poolParams = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.poolParams;

  TIDLPCLayers.layerType = TIDL_PoolingLayer;
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  poolParams.poolingType = TIDL_AveragePooling;
  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;
  poolParams.kernelW = 0;
  poolParams.kernelH = 0;
  return 0;
}


int32_t TIDL_tfMapConcatV2Params(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t idx;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ConcatLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.concatParams.axis = TIDL_DIM_NUMCH;
  if (TIDL_hasAttr(tfGraphDef.node(i), "N"))
  {
    auto& value = tfGraphDef.node(i).attr().at(std::string("N"));
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs = value.i();
  }
  else
  {
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs = 2;
  }
  TensorProto tensor = TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(i).input(pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs));
  auto& axis = tensor.int_val();

  if (gloab_data_format == 0) // NHWC
  {
    if (axis.Get(0) == 3)
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.concatParams.axis = TIDL_DIM_NUMCH;
    else if (axis.Get(0) == 2)
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.concatParams.axis = TIDL_DIM_WIDTH;
    else if (axis.Get(0) == 1)
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.concatParams.axis = TIDL_DIM_HEIGHT;
    else
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.concatParams.axis = TIDL_DIM_BATCH;
  }
  else // NCHW
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "gloab_data_format = %d is not supported with TF import Concat layer", gloab_data_format);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
int32_t TIDL_tfMapPadParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t idx,j;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_PadLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;

  TensorProto tensor = TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(i).input(1));
  auto& padTensor = tensor.int_val();

  int32_t * tPtr = (int32_t *)tensor.tensor_content().c_str();
  int32_t * padValStore = (int32_t *)my_malloc(tensor.tensor_content().size());
  memcpy(padValStore, tPtr, tensor.tensor_content().size());

  int32_t padT, padB, padL, padR;

  if (gloab_data_format == 0)
  {
    padL = padValStore[2 * 2 + 0];
    padT = padValStore[1 * 2 + 0];
    padR = padValStore[2 * 2 + 1];
    padB = padValStore[1 * 2 + 1];
  }
  else
  {
    padL = padValStore[3 * 2 + 0];
    padT = padValStore[2 * 2 + 0];
    padR = padValStore[3 * 2 + 1];
    padB = padValStore[2 * 2 + 1];
  }

  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.padLayerParams.padT = padT;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.padLayerParams.padB = padB;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.padLayerParams.padL = padL;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.padLayerParams.padR = padR;

  /*****  Assigning default values here, correct values are assigned at appropriate places in the code ****/
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.padLayerParams.padConstValue = 0;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.padLayerParams.padType = TIDL_PadZero;

  my_free(padValStore);
  return 0;
}

int32_t TIDL_tfMapSliceParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  int32_t j, slicePoints[TIDL_NUM_OUT_BUFS];
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_SliceLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;

  if (TIDL_hasAttr(tfGraphDef.node(i), "num_slice"))
  {
    auto& value = tfGraphDef.node(i).attr().at(std::string("num_slice"));

    if (value.has_list())
    {
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs = value.list().i_size();
      for (j = 0; j < value.list().i_size(); j++)
      {
        slicePoints[j] = value.list().i(j);
      }
    }
    else
    {
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs = value.i();
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.sliceParams.setSlicePoints = 1;
    }
  }
  else
  {
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs = 2;
    slicePoints[0] = ((pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inData[0].dimValues[TIDL_DIM_NUMCH] + 1) / 2);
    slicePoints[1] = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inData[0].dimValues[TIDL_DIM_NUMCH] - slicePoints[0];
  }
  for (j = 0; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; j++)
  {
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.sliceParams.slicePoints[j] = slicePoints[j];
  }

  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.sliceParams.axis = TIDL_DIM_NUMCH;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.sliceParams.stride = 1;

  return 0;
}

int32_t TIDL_tfMapSqueezeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_SqueezeLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;

  if (TIDL_hasAttr(tfGraphDef.node(i), "squeeze_dims"))
  {
    auto& value = tfGraphDef.node(i).attr().at(std::string("squeeze_dims"));

    if (value.has_list())
    {
      for (int32_t  j = 0; j < value.list().i_size(); j++)
      {
        if(value.list().i(j) < TIDL_DIM_MAX)
        {
          pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[value.list().i(j)] = 1;
        }
      }
    }
  }
  if(gloab_data_format == 0)
  {
    int32_t c,h,w;
    c = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[3];
    h = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[1];
    w = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[2];
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[TIDL_DIM_DIM1] = 0;
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[TIDL_DIM_DIM2] = 0;
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[TIDL_DIM_NUMCH]  = c;
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[TIDL_DIM_HEIGHT] = h;
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerPCParams.squeezeParams.axis[TIDL_DIM_WIDTH]  = w;
  }
  return 0;
}

int32_t TIDL_tfMapReshapeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.layerType = TIDL_ReshapeLayer;
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1,TIDLPCLayers.weights), "");
  int32_t * shape = (int32_t *)TIDLPCLayers.weights.ptr;
  //input will be in NHWC format, change accordingly
  if (TIDLPCLayers.weights.bufSize >= 3)
  {
    int32_t ch = shape[TIDL_DIM_WIDTH];
    shape[TIDL_DIM_WIDTH]   = shape[TIDL_DIM_HEIGHT];
    shape[TIDL_DIM_HEIGHT]  = shape[TIDL_DIM_NUMCH];
    shape[TIDL_DIM_NUMCH] = ch;
  }
  return 0;
}

int32_t TIDL_tfMapSoftmaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_SoftMaxLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.softMaxParams.axis = TIDL_DIM_WIDTH;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.softMaxParams.outTranspose = 0;
  return 0;
}

int32_t TIDL_tfMapShapeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ShapeLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  return 0;
}

int32_t TIDL_tfMapIdentityParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_IdentityLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  return 0;
}

int32_t TIDL_tfMapMatMulParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  sTIDL_InnerProductParams_t &innerProductParams = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.innerProductParams;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  /*Default:*/
  innerProductParams.numBChannels = 1;
  TIDLPCLayers.layerType = TIDL_InnerProductLayer;
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;

  TensorProto tensorB         = TIDL_getConstTensor(tfGraphDef, tfGraphDef.node(i).input(1));

  innerProductParams.numInCols   = tensorB.tensor_shape().dim(0).size();
  innerProductParams.numOutCols  = tensorB.tensor_shape().dim(1).size();

  innerProductParams.numInRows   = 1;
  innerProductParams.numBatches  = 1;

  bool transpose_a,transpose_b;

  if(TIDL_getAttr_bool(tfGraphDef.node(i), "transpose_a", &transpose_a) == 0)
  {
    innerProductParams.inputATranspose = 0;
  }
  else
  {
    innerProductParams.inputATranspose = transpose_a;
  }

  if(TIDL_getAttr_bool(tfGraphDef.node(i), "transpose_b", &transpose_b) == 0)
  {
    innerProductParams.inputBTranspose = 1;
  }
  else
  {
    innerProductParams.inputBTranspose = transpose_b;
  }

  innerProductParams.isBias          = 1;
  innerProductParams.constIdx        = -1;

  TIDLPCLayers.inData[0].dimValues[TIDL_DIM_BATCH]  = 1; // Is there better way to get batch, currently assuming 1
  TIDLPCLayers.inData[0].dimValues[TIDL_DIM_NUMCH]  = 1; // Is there better way to get channel, currently assuming 1
  TIDLPCLayers.inData[0].dimValues[TIDL_DIM_HEIGHT] = 1;
  TIDLPCLayers.inData[0].dimValues[TIDL_DIM_WIDTH]  = innerProductParams.numInCols;

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1,TIDLPCLayers.weights), "");
  if(innerProductParams.inputBTranspose)
  TIDL_kernelReshape((float *)TIDLPCLayers.weights.ptr, innerProductParams.numBatches, innerProductParams.numInRows, innerProductParams.numInCols, innerProductParams.numOutCols);

  /* Set default bias as zero, if next layer has bias it will get merged and this buffer will be used */
  TIDLPCLayers.bias.bufSize = innerProductParams.numOutCols;
  TIDLPCLayers.bias.ptr = (float *)my_malloc(TIDLPCLayers.bias.bufSize * sizeof(float));
  memset(TIDLPCLayers.bias.ptr, 0, (TIDLPCLayers.bias.bufSize * sizeof(float)));


  return 0;
}


int32_t TIDL_tfMapUnSuportedlayerParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&             tfGraphDef)
{
  return 0;
}

int32_t TIDL_tfMapResizeNearestNeighborParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&            tfGraphDef)
{
  int32_t idx, j;
  bool half_pixel_centers = true, align_corners = false;
  char upsampleMode[50];
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ResizeLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  int32_t axis;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs = 1;

  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.mode = TIDL_ResizeNearest;

  int32_t nodeIdx = TIDL_tfGetNodeIdx(tfGraphDef, tfGraphDef.node(i).input(1).c_str());
  sBuffer_t buf;
  if (nodeIdx != -1)
  {
    if (strcmp(tfGraphDef.node(nodeIdx).op().c_str(), "Const") == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, i, 1, buf), "");
      int32_t *ptr = (int32_t *)buf.ptr;
      ptr[0] = -ptr[0];
      ptr[1] = -ptr[1];
    }
    else if (strcmp(tfGraphDef.node(nodeIdx).op().c_str(), "Mul") == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyInputConstTensor(tfGraphDef, nodeIdx, 1, buf), "");
    }

    TIDL_getAttr_bool(tfGraphDef.node(i), "half_pixel_centers", &half_pixel_centers);
    TIDL_getAttr_bool(tfGraphDef.node(i), "align_corners", &align_corners);
    if(align_corners == true)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Align corners = TRUE is not supported");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if(half_pixel_centers == false)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Half pixel centers = FALSE is not supported");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  /* initialize resizeRatio by 1, later will be changed acc to ratio given */
  for(j = 0; j < TIDL_DIM_MAX; j++)
  {
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.resizeRatio[j] = 1.0;
  }
  if ((buf.ptr != NULL) && (buf.bufSize == 2))
  {
    int32_t *ptr = (int32_t *)buf.ptr;
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.resizeRatio[TIDL_DIM_HEIGHT] = ptr[0];
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.resizeRatio[TIDL_DIM_WIDTH] = ptr[1];
    my_free(ptr);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not find the right resize ratio const, assuming it as 2x ");
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.resizeRatio[TIDL_DIM_HEIGHT] = 2;
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.resizeRatio[TIDL_DIM_WIDTH] = 2;
  }

  return 0;
}

int32_t TIDL_tfMapResizeBilinearParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphDef&            tfGraphDef)
{
  TIDL_tfMapResizeNearestNeighborParams(pOrgTIDLNetStructure, i, layerIndex, dataIndex, tfGraphDef);
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.resizeParams.mode = TIDL_ResizeBilinear;
  return 0;
}

typedef struct {
  int8_t name[TIDL_STRING_SIZE];
  int32_t(*tidl_tfMapFunc)(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
    int32_t              i,
    int32_t              layerIndex,
    int32_t              *dataIndex,
    GraphDef&             tfGraphDef);
}sTIDL_tfOpParamMap_t;

sTIDL_tfOpParamMap_t tidl_TfOpParamMapTable[] =
{
  { "Placeholder",                     TIDL_tfMapPlaceHolderParams },       //  TIDL_DataLayer,
  { "Conv2D",                          TIDL_tfMapConvParams },       //  TIDL_ConvolutionLayer ,
  { "Conv2DTranspose",                 TIDL_tfMapDeconv2DParams },       //  TIDL_ConvolutionLayer ,
  { "DepthwiseConv2dNative",           TIDL_tfMapDWConvParams },       //  TIDL_ConvolutionLayer ,
  { "BiasAdd",                         TIDL_tfMapBiasAddParams },       //  TIDL_BiasLayer ,
  { "Add",                             TIDL_tfMapAddParams },       //  TIDL_EltWiseLayer ,
  { "Mul",                             TIDL_tfMapMulParams },       //  TIDL_EltWiseLayer ,
  { "FusedBatchNorm",                  TIDL_tfMapFusedBnParams },        //  TIDL_BatchNormLayer ,
  { "Relu",                            TIDL_tfMapReluParams },       //  TIDL_ReLULayer ,
  { "Relu6",                           TIDL_tfMapRelu6Params },       //  TIDL_ReLULayer ,
  { "Sigmoid",                         TIDL_tfMapSigmoidParams},       //  TIDL_SigmoidLayer ,
  { "Logistic",                        TIDL_tfMapSigmoidParams },
  { "Tanh",                            TIDL_tfMapTanhParams},       //  TIDL_TanhLayer ,
  { "HardSigmoid",                     TIDL_tfMapHardSigmoidParams},       //  TIDL_HardSigmoidLayer ,
  { "ELU",                             TIDL_tfMapELUParams},       //  TIDL_ELULayer ,
  { "MaxPool",                         TIDL_tfMapMaxPoolParams },       //  TIDL_PoolingLayer ,
  { "AvgPool",                         TIDL_tfMapAvgPoolParams },       //  TIDL_PoolingLayer ,
  { "ConcatV2",                        TIDL_tfMapConcatV2Params },       //  TIDL_ConcatLayer ,
  { "Slice",                           TIDL_tfMapSliceParams },       //  TIDL_SliceLayer ,
  { "Squeeze",                         TIDL_tfMapSqueezeParams },       //  TIDL_SqueezeLayer ,
  { "Reshape",                         TIDL_tfMapReshapeParams },       //  TIDL_ReshapeLayer ,
  { "Softmax",                         TIDL_tfMapSoftmaxParams },       //  TIDL_SoftMaxLayer ,
  { "Pad",                             TIDL_tfMapPadParams },       //  TIDL_SoftMaxLayer ,
  { "Mean",                            TIDL_tfMapMeanParams },       //  TIDL_SoftMaxLayer ,
  { "ResizeNearestNeighbor",           TIDL_tfMapResizeNearestNeighborParams },       //  TIDL_SoftMaxLayer ,
  { "ResizeBilinear",                  TIDL_tfMapResizeBilinearParams },       //  TIDL_SoftMaxLayer ,
  { "Shape",                           TIDL_tfMapShapeParams },       //  TIDL_ShapeLayer ,
  { "Identity",                        TIDL_tfMapIdentityParams},       //  TIDL_ShapeLayer ,
  { "ArgMax",                          TIDL_tfMapArgmaxParams },       //  TIDL_ArgMaxLayer ,
  { "MatMul",                          TIDL_tfMapMatMulParams }       //  TIDL_InnerProductLayer ,
};



int32_t TIDL_getTfOpParamMapId(const char  * name)
{
  int32_t i = -1;
  for (i = 0; i < sizeof(tidl_TfOpParamMapTable) / sizeof(sTIDL_tfOpParamMap_t); i++)
  {
    if ((strcmp(name, (const char *)tidl_TfOpParamMapTable[i].name) == 0))
    {
      return (i);
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

TIDL_TFLayerMapping_t TIDL_TFLayerMap[] =
{
  { (char*)"TIDL_TFSlimFlatten",            (char*)"TIDL_SqueezeLayerTIDL_ReshapeLayer", 2 },
  { (char*)"TIDL_TFPackPackReshape",        (char*)"TIDL_PackLayerTIDL_PackLayerTIDL_ReshapeLayer", 3 },
  { (char*)"TIDL_TFSlimShuffle",            (char*)"ResahpeSqueeze", 3 },
  { (char*)"TIDL_ReshapeLayer",             (char*)"TIDL_ReshapeLayerTIDL_InnerProductLayer", 2},
  { (char*)"TIDL_HardSigmoidLayer",         (char*)"TIDL_BatchNormLayerTIDL_BatchNormLayerTIDL_ClipLayerTIDL_ReLULayer", 4}
};

int32_t tidl_checkParamsForHardSigmoidLayerTFLite(sTIDL_LayerPC_t &TIDLPCLayers1, sTIDL_LayerPC_t &TIDLPCLayers2, sTIDL_LayerPC_t &TIDLPCLayers3, sTIDL_LayerPC_t &TIDLPCLayers4, float32_tidl *alpha, float32_tidl *beta)
{
  int32_t status = 1;

  float32_tidl mul_weight = ((float32_tidl*)TIDLPCLayers1.weights.ptr)[0];
  float32_tidl mul_bias = ((float32_tidl*)TIDLPCLayers1.bias.ptr)[0];
  for(int i=0; i<TIDLPCLayers1.weights.bufSize; i++)
  {
    if(mul_weight != ((float32_tidl*)TIDLPCLayers1.weights.ptr)[i])
    {
      return 0;
    }
    mul_weight = ((float32_tidl*)TIDLPCLayers1.weights.ptr)[i];

    mul_bias = ((float32_tidl*)TIDLPCLayers1.bias.ptr)[i];
    if(mul_bias!=(float)0)
    {
      return 0;
    }
  }

  float32_tidl add_weight = ((float32_tidl*)TIDLPCLayers2.weights.ptr)[0];
  float32_tidl add_bias = ((float32_tidl*)TIDLPCLayers2.bias.ptr)[0];
  for(int i=0; i<TIDLPCLayers2.bias.bufSize; i++)
  {
    if(add_bias != ((float32_tidl*)TIDLPCLayers2.bias.ptr)[i])
    {
      return 0;
    }
    add_bias = ((float32_tidl*)TIDLPCLayers2.bias.ptr)[i];

    add_weight = ((float32_tidl*)TIDLPCLayers2.weights.ptr)[i];
    if(add_weight!=(float)1)
    {
      return 0;
    }
  }

  if(add_bias!=0.5)
  {
    return 0;
  }

  if(TIDLPCLayers3.clipParams.clipMax != 1)
  {
    return 0;
  }

  *alpha = mul_weight;
  *beta = add_bias;

  return status;
}

int32_t tidl_FindHardSigmoidLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    if (tf_isLayerType("TIDL_HardSigmoidLayer", i1, pOrgTIDLNetStructure, TIDL_TFLayerMap, (sizeof(TIDL_TFLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
    {
      int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_HardSigmoidLayer", TIDL_TFLayerMap, (sizeof(TIDL_TFLayerMap) / sizeof(TIDL_TFLayerMapping_t)));

      sTIDL_LayerPC_t &TIDLPCLayers1 = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      int32_t nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
      if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        continue;
      sTIDL_LayerPC_t &TIDLPCLayers2 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];
      nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx].outData[0].dataId);
      if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        continue;
      sTIDL_LayerPC_t &TIDLPCLayers3 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];
      nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx].outData[0].dataId);
      if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        continue;
      sTIDL_LayerPC_t &TIDLPCLayers4 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];
      float32_tidl alpha_tflite, beta_tflite;

      int32_t isHardSigmoidLayer = tidl_checkParamsForHardSigmoidLayerTFLite(TIDLPCLayers1, TIDLPCLayers2, TIDLPCLayers3, TIDLPCLayers4, &alpha_tflite, &beta_tflite);
      if(isHardSigmoidLayer)
      {
        pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType = TIDL_HardSigmoidLayer;
        pOrgTIDLNetStructure.TIDLPCLayers[i1].actParams.actType = TIDL_HardSigmoid;
        pOrgTIDLNetStructure.TIDLPCLayers[i1].layerPCParams.hardSigmoidParams.alpha = alpha_tflite;
        pOrgTIDLNetStructure.TIDLPCLayers[i1].layerPCParams.hardSigmoidParams.beta = beta_tflite;
        pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0] = pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outData[0];
        strcpy((char *)pOrgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[0] , (char *)pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outDataNames[0]);
        pOrgTIDLNetStructure.TIDLPCLayers[i1].outConsumerCnt[0] = pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outConsumerCnt[0];
        for (i2 = 0; i2 < (TIDL_TFLayerMap[mapIdx].NumOps - 1); i2++)
        {
          pOrgTIDLNetStructure.TIDLPCLayers[i1 + i2 + 1].numInBufs = -1;
          pOrgTIDLNetStructure.TIDLPCLayers[i1 + i2 + 1].numOutBufs = -1;
        }
        sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure.TIDLPCLayers[i1];

        TIDLPCLayers.outData[0].dimValues[TIDL_DIM_BATCH] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_BATCH];
        TIDLPCLayers.outData[0].dimValues[TIDL_DIM_DIM1] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_DIM1];
        TIDLPCLayers.outData[0].dimValues[TIDL_DIM_DIM2] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_DIM2];
        TIDLPCLayers.outData[0].dimValues[TIDL_DIM_NUMCH] = 1;
        TIDLPCLayers.outData[0].dimValues[TIDL_DIM_HEIGHT] = 1;
        TIDLPCLayers.outData[0].dimValues[TIDL_DIM_WIDTH ] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_NUMCH]*
                                              TIDLPCLayers.inData[0].dimValues[TIDL_DIM_HEIGHT] *
                                              TIDLPCLayers.inData[0].dimValues[TIDL_DIM_WIDTH ];

        int32_t  idx = tidl_getOutLayer(pOrgTIDLNetStructure, layerIndex, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
        if (idx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        sTIDL_LayerPC_t &TIDLPCLayersout = pOrgTIDLNetStructure.TIDLPCLayers[idx];
        TIDLPCLayersout.inData[0] = TIDLPCLayers.outData[0];
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}


int32_t tidl_FindFlattenLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    if (tf_isLayerType("TIDL_TFSlimFlatten", i1, pOrgTIDLNetStructure, TIDL_TFLayerMap, (sizeof(TIDL_TFLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
    {
      int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_TFSlimFlatten", TIDL_TFLayerMap, (sizeof(TIDL_TFLayerMap) / sizeof(TIDL_TFLayerMapping_t)));
      /* check if (squeeze + reshape) can be converted to flatten */
      int64_t inDims = 1;
      for(i2 = TIDL_DIM_DIM1; i2 < TIDL_DIM_MAX; i2++)
      {
        inDims *= pOrgTIDLNetStructure.TIDLPCLayers[i1].inData[0].dimValues[i2];
      }
      if(inDims != pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outData[0].dimValues[TIDL_DIM_WIDTH])
      {
        continue;
      }
      pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType = TIDL_FlattenLayer;
      pOrgTIDLNetStructure.TIDLPCLayers[i1].layerPCParams.flattenParams.axis = TIDL_DIM_DIM1;
      pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0] = pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outData[0];
      strcpy((char *)pOrgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[0] , (char *)pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outDataNames[0]);
      pOrgTIDLNetStructure.TIDLPCLayers[i1].outConsumerCnt[0] = pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outConsumerCnt[0];
      for (i2 = 0; i2 < (TIDL_TFLayerMap[mapIdx].NumOps - 1); i2++)
      {
        pOrgTIDLNetStructure.TIDLPCLayers[i1 + i2 + 1].numInBufs = -1;
        pOrgTIDLNetStructure.TIDLPCLayers[i1 + i2 + 1].numOutBufs = -1;
      }
      sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure.TIDLPCLayers[i1];

      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_BATCH] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_BATCH];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_DIM1] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_DIM1];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_DIM2] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_DIM2];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_NUMCH] = 1;
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_HEIGHT] = 1;
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_WIDTH ] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_NUMCH]*
                                             TIDLPCLayers.inData[0].dimValues[TIDL_DIM_HEIGHT] *
                                             TIDLPCLayers.inData[0].dimValues[TIDL_DIM_WIDTH ];

      int32_t  idx = tidl_getOutLayer(pOrgTIDLNetStructure, layerIndex, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
      if (idx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &TIDLPCLayersout = pOrgTIDLNetStructure.TIDLPCLayers[idx];
      TIDLPCLayersout.inData[0] = TIDLPCLayers.outData[0];
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_mergePackToNNResizeLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    if (tf_isLayerType("TIDL_TFPackPackReshape", i1, pOrgTIDLNetStructure, TIDL_TFLayerMap, (sizeof(TIDL_TFLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
    {
      int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_TFPackPackReshape", TIDL_TFLayerMap, (sizeof(TIDL_TFLayerMap) / sizeof(TIDL_TFLayerMapping_t)));
      pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType = TIDL_ResizeLayer;
      pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0] = pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outData[0];
      strcpy((char *)pOrgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[0] , (char *)pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outDataNames[0]);
      pOrgTIDLNetStructure.TIDLPCLayers[i1].outConsumerCnt[0] = pOrgTIDLNetStructure.TIDLPCLayers[i1 + TIDL_TFLayerMap[mapIdx].NumOps - 1].outConsumerCnt[0];
      for (i2 = 0; i2 < (TIDL_TFLayerMap[mapIdx].NumOps - 1); i2++)
      {
        pOrgTIDLNetStructure.TIDLPCLayers[i1 + i2 + 1].numInBufs = -1;
        pOrgTIDLNetStructure.TIDLPCLayers[i1 + i2 + 1].numOutBufs = -1;
      }
      sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure.TIDLPCLayers[i1]; //pack layer to be converted to NN resize
      sTIDL_ResizeLayerParams_t& params = TIDLPCLayers.layerParams.resizeParams;
      /* initializing ratio to 1.0, will be changed acc to values in packParams */
      for(i3 = 0; i3 < TIDL_DIM_MAX; i3++)
      {
        params.resizeRatio[i3] = 1.0;
      }
      params.resizeRatio[TIDL_DIM_HEIGHT] = TIDLPCLayers.layerPCParams.packParams.valuesCount;
      params.resizeRatio[TIDL_DIM_WIDTH]  = TIDLPCLayers.layerPCParams.packParams.valuesCount;
      params.mode = TIDL_ResizeNearest;
      sTIDL_ResizePCParams_t& pcParams = TIDLPCLayers.layerPCParams.resizeParams;
      pcParams.antialias = 0;
      pcParams.keep_aspect_ratio_policy = 0;

      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_BATCH] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_BATCH];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_DIM1] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_DIM1];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_DIM2] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_DIM2];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_NUMCH] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_NUMCH];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_HEIGHT] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_HEIGHT] * params.resizeRatio[TIDL_DIM_HEIGHT];
      TIDLPCLayers.outData[0].dimValues[TIDL_DIM_WIDTH] = TIDLPCLayers.inData[0].dimValues[TIDL_DIM_WIDTH] * params.resizeRatio[TIDL_DIM_WIDTH];

      int32_t  idx = tidl_getOutLayer(pOrgTIDLNetStructure, layerIndex, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
      if (idx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &TIDLPCLayersout = pOrgTIDLNetStructure.TIDLPCLayers[idx];
      TIDLPCLayersout.inData[0] = TIDLPCLayers.outData[0];
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_findTfOutputNames(GraphDef&  tfGraphDef, char * outList)
{
  int i, j, k, l;
  char tensorName[TIDL_STRING_SIZE];
  char inTensorName[TIDL_STRING_SIZE];

  for (i = 0; i < tfGraphDef.node_size(); i++)
  {
    int outDataUsed = 0;
    strncpy((char *)tensorName, tfGraphDef.node(i).name().c_str(), TIDL_STRING_SIZE);
    for (k = 0; k < tfGraphDef.node_size(); k++)
    {
      for (l = 0; l < tfGraphDef.node(k).input_size(); l++)
      {
        strncpy((char *)inTensorName, tfGraphDef.node(k).input(l).c_str(), TIDL_STRING_SIZE);
        if (strcmp(tensorName, inTensorName) == 0)
        {
          outDataUsed = 1;
          break;
        }
        else if (inTensorName[strlen(tensorName)] == ':')
        {
          inTensorName[strlen(tensorName)] = '\0';
          if (strcmp(tensorName, inTensorName) == 0)
          {
           outDataUsed = 1;
           break;
          }
      }
      if (outDataUsed)
        break;
      }
      if (outDataUsed)
        break;
    }
    if (outDataUsed == 0)
    {
      strcat(outList, tensorName);
      strcat(outList, ",");
   }
  }
  return 0;
}

int32_t tidl_tfLayerFillTensorNames(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  GraphDef&             tfGraphDef)
{
  int32_t j;
  if(strlen(tfGraphDef.node(i).name().c_str()) < TIDL_STRING_SIZE)
  {
    strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].name, tfGraphDef.node(i).name().c_str());
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", tfGraphDef.node(i).name().c_str(), TIDL_STRING_SIZE);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  if (pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs > 0)
  {
    for (j = 0; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs; j++)
    {
      if(strlen(tfGraphDef.node(i).input(j).c_str()) < TIDL_STRING_SIZE)
      {
        strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inDataNames[j], tfGraphDef.node(i).input(j).c_str());
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", tfGraphDef.node(i).input(j).c_str(), TIDL_STRING_SIZE);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inData[j].dataId = -1;
    }
  }
  if (pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs > 0)
  {
    if(strlen(tfGraphDef.node(i).name().c_str()) < TIDL_STRING_SIZE)
    {
      strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[0], tfGraphDef.node(i).name().c_str());
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", tfGraphDef.node(i).name().c_str(), TIDL_STRING_SIZE);
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerLinked[0] = 0;
    for (j = 1; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; j++)
    {
      char numberStr[10];
      strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[j], tfGraphDef.node(i).name().c_str());
      strcat((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[j], ":");
      sprintf(numberStr, "%d",j);
      strcat((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[j], numberStr);
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerLinked[j] = 0;
    }
  }
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weightsElementSizeInBits = NUM_WHGT_BITS;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}



int32_t tidl_tfGetNewNodeToAdd(sTIDL_OrgNetwork_t   &orgTIDLNetStructure,
  int32_t              layerIndex,
  GraphDef&             tfGraphDef)
{
  int32_t i, j, nodeIdx = -1;

  for (i = 0; i < layerIndex; i++)
  {
    for (j = 0; j < orgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
    {
      if (TIDL_getLayerIdx(&orgTIDLNetStructure, layerIndex, (const char *)orgTIDLNetStructure.TIDLPCLayers[i].inDataNames[j]) == -1)
      {
        nodeIdx = TIDL_tfGetNodeIdx(tfGraphDef, (const char *)orgTIDLNetStructure.TIDLPCLayers[i].inDataNames[j]);
        if (nodeIdx != -1)
        {
          if (strcmp(tfGraphDef.node(nodeIdx).op().c_str(), "Const") == 0)
          {
            nodeIdx == -1;
          }
        }
        if (nodeIdx != -1)
        {
          break;
        }
      }
    }
    if (nodeIdx != -1)
    {
      break;
    }
  }
  return nodeIdx;
}
int32_t tidl_tfLayerUpdateConsumerCount(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure,
                                        int32_t i,
                                        int32_t layerIndex,
                                        GraphDef &tfGraphDef)
{
  int32_t i0, i1, i2;
  int32_t numCons = 0;
  for (i0 = 0; i0 < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; i0++)
  {

    for (i1 = 0; i1 < tfGraphDef.node_size(); i1++)
    {
      for (i2 = 0; i2 < tfGraphDef.node(i1).input_size(); i2++)
      {
        if (strcmp((const char *)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[i0], tfGraphDef.node(i1).input(i2).c_str()) == 0)
        {
          numCons++;
        }
      }
    }
    for (i2 = 0; i2 < numNetOutData; i2++)
    {
      if (strcmp((const char *)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[i0], outDataNames[i2]) == 0)
      {
        numCons++;
      }
    }

    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerCnt[i0] = numCons;
  }
  return 0;
}

int32_t tf_import(tidl_import_config * params, int32_t *totalData, int32_t* totalLayers)
{
  int32_t                    i,j;
  int32_t                    layerNum;
  int32_t                    inputSize;
  int32_t                    pad,stride;
  int32_t                    layerIndex;
  int32_t                    tiLayerIndex;
  int32_t                    dataIndex;
  const uint8_t             *name;
  const uint8_t             *inputName[10];
  const uint8_t             *outputName;
  GraphDef           tfGraphDef;
  int32_t                    dataSize;
  int32_t                    id;
  int paramSet  = 0;
  int conv2DRandParams = 0;
  string attrKey;
  int32_t inLayerId = 0;
  int32_t weightsElementSizeInBits;
  int32_t mapTblIdx = -1;

  string key = "value";

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TF Model (Proto) File  : %s ",(const char *)params->inputNetFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL Network File      : %s ", (const char *)params->outputNetFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL IO Info File      : %s ", (const char *)params->outputParamsFile);

  bool success = TIDL_readProtoFromBinaryFile((const char *)params->inputNetFile, &tfGraphDef);
  if(!success){
    TIDL_GLOBAL_REPORT_ERROR("Reading models params file failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  gloab_data_format = 0;
  layerIndex = 0;
  dataIndex  = 0;

  if (strcmp((char*)params->outDataNamesList, "") == 0)
  {
    char outDataNamesList[TIDL_MAX_ALG_OUT_BUFS * TIDL_STRING_SIZE] = "";
    tidl_findTfOutputNames(tfGraphDef, (char*)outDataNamesList);
    numNetOutData = tidl_getStringsFromList((char *)outDataNamesList, (char*)outDataNames, TIDL_MAX_DATA_NAME);
  }
  else
  {
    numNetOutData = tidl_getStringsFromList((char *)params->outDataNamesList, (char*)outDataNames, TIDL_MAX_DATA_NAME);
  }
  for (i = 0; i < numNetOutData; i++)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs =  1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = -1;
    strcpy((char*)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, outDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].inDataNames[0], outDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[0], outDataNames[i]);
    layerIndex++;
  }

  if (strcmp((char *)params->inDataNamesList, "") != 0)
  {
    numNetInData = tidl_getStringsFromList((char *)params->inDataNamesList, (char *)inDataNames, TIDL_MAX_DATA_NAME);
    for (i = 0; i < numNetInData; i++)
    {
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = -1;
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
      strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, inDataNames[i]);
      strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[0], inDataNames[i]);
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerCnt[0] = 1;
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerLinked[0] = 0;
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dataId = dataIndex++;
      tidl_tfLayerUpdateConsumerCount(&orgTIDLNetStructure, layerIndex, layerIndex, tfGraphDef);
      tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
      layerIndex++;
    }
  }

  int newNode = tidl_tfGetNewNodeToAdd(orgTIDLNetStructure, layerIndex, tfGraphDef);;
  while (newNode != -1)
  {
    int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].actParams.actType = TIDL_NoAct;
    
    if (params->enableCustomLayers)
    {
#ifdef TIDL_CUSTOM_LAYER_API_v1
      TIDL_MapCustomParams(&orgTIDLNetStructure,
                          newNode,
                          layerIndex,
                          &dataIndex,
                          &tfGraphDef,
                          NULL,
                          TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW);
#elif defined(TIDL_CUSTOM_LAYER_API_v2)         
      sTIDL_LayerPC_t* layer = &orgTIDLNetStructure.TIDLPCLayers[layerIndex];
      if(layer->weights.ptr == NULL)
      {
        status = TIDL_mapCustomParams_v2(layer,
                                      newNode,
                                      &tfGraphDef,
                                      NULL,
                                      TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW);
        
        if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          layer->outData[0].dataId = dataIndex++;
        }                
      }
#endif
    }

    if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      mapTblIdx = TIDL_getTfOpParamMapId(tfGraphDef.node(newNode).op().c_str());
      if (mapTblIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, " TF operator %s is not suported now, By passing", tfGraphDef.node(newNode).op().c_str());
        TIDL_tfMapUnSuportedlayerParams(&orgTIDLNetStructure, newNode, layerIndex, &dataIndex, tfGraphDef);
      }
      else
      {
        status = tidl_TfOpParamMapTable[mapTblIdx].tidl_tfMapFunc(&orgTIDLNetStructure, newNode, layerIndex, &dataIndex, tfGraphDef);
        if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          for(auto &diag: gDiags.gDiagList)
          {
            if(! diag.getString().empty())
            {
              gDiags.modelDiag(diag);
            }
          }
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
      }
    }

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_tfLayerFillTensorNames(&orgTIDLNetStructure, newNode, layerIndex, tfGraphDef),"");
    tidl_tfLayerUpdateConsumerCount(&orgTIDLNetStructure, newNode, layerIndex, tfGraphDef);
    tidl_linkInputTensors(&orgTIDLNetStructure, layerIndex);
    tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
    layerIndex++;
    newNode = tidl_tfGetNewNodeToAdd(orgTIDLNetStructure, layerIndex, tfGraphDef);
  }
  *totalData = dataIndex;
  *totalLayers = layerIndex;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

