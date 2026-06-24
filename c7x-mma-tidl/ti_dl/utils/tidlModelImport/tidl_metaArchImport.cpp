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
#include "tidl_meta_arch.pb.h"
#include "tidl_import_config.h"
#include "tidl_import_diag.h"

#define __MODULE__ ""

using namespace std;
using namespace tidl_meta_arch;
using ::google::protobuf::Message;
using ::google::protobuf::io::FileInputStream;
using ::google::protobuf::io::FileOutputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;
using ::google::protobuf::io::CodedOutputStream;

#include "tidl_import_common.h"

extern char outMetaLayerNames[TIDL_MAX_ALG_OUT_BUFS][TIDL_MAX_DATA_NAME];
sTIDL_DetectOutputParams_t tidl_OdPostProc[TIDL_MAX_TF_SSD_LAYERS];
float *tidlPriorPtrs[TIDL_MAX_TF_SSD_LAYERS] = {0};
extern int32_t numTFMetaLayers;
uint8_t tidl_OdBoxInputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_MAX_ALG_OUT_BUFS][TIDL_STRING_SIZE];
uint8_t tidl_OdClassInputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_MAX_ALG_OUT_BUFS][TIDL_STRING_SIZE];
uint8_t tidl_OdRegInput0Names[TIDL_MAX_TF_SSD_LAYERS][TIDL_MAX_ALG_OUT_BUFS][TIDL_STRING_SIZE];
uint8_t tidl_OdDirInputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_MAX_ALG_OUT_BUFS][TIDL_STRING_SIZE]; // used in 3d od
uint8_t tidl_OdOutputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_STRING_SIZE];
std::string tidl_OdFramework = "";


void TIDL_metaArchImportDetectionOutputParams(TIDLOdPostProc& detection_output_param,
sTIDL_DetectOutputParams_t & detectOutParams)
{
  int32_t  status;
  int32_t  id;
  int32_t  j, num_classes, num_priors;
  int32_t  num_loc_classes;
  int32_t  background_label_id;
  int32_t  code_type;
  float    confidence_threshold;
  float    nms_threshold, eta;
  int32_t  top_k, keep_top_k, top_k_all_class;
  bool     share_location, clip_detectOutParamsdetectOutParamsbbox;
  bool     variance_encoded_in_target;

  detectOutParams.metaArchType = TIDL_metaArchCaffeJacinto;

  detectOutParams.numKeypoints = detection_output_param.num_keypoint();
  detectOutParams.keypointConfidence = detection_output_param.keypoint_confidence();
  detectOutParams.numClasses   = detection_output_param.num_classes();
  detectOutParams.shareLocation = detection_output_param.share_location();
  detectOutParams.backgroundLabelId = detection_output_param.background_label_id();
  detectOutParams.codeType = detection_output_param.has_code_type() ? detection_output_param.code_type() : 0;
  detectOutParams.varianceEncoded =  detection_output_param.variance_encoded_in_target();
  detectOutParams.keepTopK = detection_output_param.keep_top_k();
  detectOutParams.confThreshold = detection_output_param.has_confidence_threshold() ? detection_output_param.confidence_threshold() : -FLT_MAX;
  detectOutParams.nmsThreshold = detection_output_param.nms_param().nms_threshold();
  detectOutParams.reserve2 = detection_output_param.nms_param().nms_type(); // reserve2:: nmsType (need to add the variable "nmsType" in arm-tidl)
  int32_t nms_type = detection_output_param.nms_param().nms_type();

  if((nms_type != UPRIGHT_BOX_OVERLAP_NMS) && (nms_type != CIRCULAR_BOX_DIST_NMS))
  {
    TIDL_GLOBAL_REPORT_WARNING("%d kind of NMS operation is not supported by TIDL at present, using upright box overlap based IOU",nms_type);
  }

  detectOutParams.eta = detection_output_param.nms_param().eta();

  if ((detection_output_param.code_type()==CODE_TYPE_CENTERNET) || (detection_output_param.code_type()==CODE_TYPE_FAST_BEV) || (detection_output_param.code_type()==CODE_TYPE_BEVFORMER)) // For CenterNet or Fast BEV or BEVFormer if NMS layer in not present then picking keep_top_k as top_k
  {
    detectOutParams.topK = detection_output_param.nms_param().has_top_k() ? detection_output_param.nms_param().top_k() : detectOutParams.keepTopK;
  }
  else
  {
    detectOutParams.topK = detection_output_param.nms_param().has_top_k() ? detection_output_param.nms_param().top_k() : -1;
  }

  detectOutParams.topKAllClasses = detection_output_param.nms_param().has_top_k_all_class() ? detection_output_param.nms_param().top_k_all_class() : -1;
  if (detection_output_param.code_type()==CODE_TYPE_BEVFORMER) // For BEVFormer if NMS layer in not present then picking topKAllClasses as top_k
  {
    detectOutParams.topKAllClasses =  detection_output_param.nms_param().has_top_k_all_class() ? detection_output_param.nms_param().top_k_all_class() : detectOutParams.keepTopK;
  }
  detectOutParams.subCodeType = detection_output_param.sub_code_type();

  if (detection_output_param.sub_code_type() == TIDL_ObjectPose)
  {
    detectOutParams.subCodeParams.cameraParams.fX = detection_output_param.camera_intrinsic_params().fx();
    detectOutParams.subCodeParams.cameraParams.fY = detection_output_param.camera_intrinsic_params().fy();
    detectOutParams.subCodeParams.cameraParams.pX = detection_output_param.camera_intrinsic_params().px();
    detectOutParams.subCodeParams.cameraParams.pY = detection_output_param.camera_intrinsic_params().py();
  }
}


int32_t TIDL_metaArchimportPriorBoxParams(PriorBoxParameter& prior_box_param,
float               **pPrioData,
int32_t              *priorDataSize,
int32_t               imWidth,
int32_t               imHeight)
{
  int32_t     status;
  int32_t id;
  int32_t j, img_height, img_width, num_priors;
  float step_h, step_w;

   int32_t aspect_ratios_size = prior_box_param.aspect_ratio_size();
  float  * aspect_ratios   = (float *)my_calloc((aspect_ratios_size*3), sizeof(float));
  bool flip = prior_box_param.flip();
  int ptr_offset = 0;
  aspect_ratios[ptr_offset++] = 1.0;
  for (int idx = 0; idx < aspect_ratios_size; idx++)
  {
   float ar = prior_box_param.aspect_ratio(idx);
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

  int32_t min_sizes_size = prior_box_param.min_size_size();
  float  * min_sizes   = (float *)my_malloc(min_sizes_size*sizeof(float));
  for (int idx = 0; idx < min_sizes_size; idx++)
  {
     min_sizes[idx] = prior_box_param.min_size(idx);
  }

  int32_t max_sizes_size = prior_box_param.max_size_size();
  float  * max_sizes   = (float *)my_malloc(max_sizes_size*sizeof(float));
  num_priors = aspect_ratios_size * min_sizes_size;
  for (int idx = 0; idx < max_sizes_size; idx++)
  {
     max_sizes[idx] = prior_box_param.max_size(idx);
   num_priors += 1;
  }
  float offset = prior_box_param.offset();
  bool  clip = prior_box_param.clip();

  if (prior_box_param.has_img_h() || prior_box_param.has_img_w())
  {
    img_height = prior_box_param.img_h();
    img_width = prior_box_param.img_w();
  }
  else if (prior_box_param.has_img_size())
  {
    int32_t img_size = prior_box_param.img_size();
    img_height = img_size;
    img_width = img_size;
  }
  else
  {
    img_height = imHeight;
    img_width  = imWidth;

    TIDL_GLOBAL_REPORT_WARNING("Image dimensions is not provided, Please provide it as part of prior_box_param in form of either (img_w & img_h) or img_size. Proceeding with img_w = %d and img_h = %d in prior box decoding",img_width,img_height);

  }

  if (prior_box_param.has_step_h() || prior_box_param.has_step_w())
  {
    step_h = prior_box_param.step_h();
    step_w = prior_box_param.step_w();
  }
  else if (prior_box_param.has_step())
  {
    float step = prior_box_param.step();
    step_h = step;
    step_w = step;
  }
  else // Step Not found prototext need to find from inout layer
  {
    TIDL_GLOBAL_REPORT_ERROR("Step size is not found in the prototext, provide it as part of prior_box_param, exiting");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    step_h = -1;
    step_w = -1;
  }

  int32_t num_keypoint;
  if (prior_box_param.has_num_keypoint())
  {
    num_keypoint = prior_box_param.num_keypoint();
  }
  else
  {
    num_keypoint = 0;
  }

  int32_t variance_size = prior_box_param.variance_size();
  float  * variance   = (float *)my_malloc((variance_size+1)*sizeof(float));
  if(variance_size)
  {
    for (int idx = 0; idx < variance_size; idx++)
    {
       variance[idx] = prior_box_param.variance(idx);
    }
    if (variance_size != (4 + 2 * num_keypoint))
    {
      TIDL_GLOBAL_REPORT_ERROR("variance_size is not matching with the expected size, exiting...");
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
  anchorBox->numAnchors = num_priors;
  anchorBox->numKeyPoints = num_keypoint;
  anchorBox->headWidth = img_width/step_w;
  anchorBox->headHeight = img_height/step_h;
  anchorBox->strideW = step_w/img_width;
  anchorBox->strideH = step_h/img_height;
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
      anchorBox->boxWidth[box_counter] = box_width/img_width;
      // ymin
      anchorBox->boxHeight[box_counter] = box_height/img_height;
      box_counter++;

      if (max_sizes_size > 0)
      {
        int max_size = max_sizes[s];
        float size  = min_size * max_size;
        // second prior: aspect_ratio = 1, size = sqrt(min_size * max_size)
        box_width = box_height = sqrt(size);
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

  *pPrioData = (float *)anchorBox;
  *priorDataSize = sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);
  my_free(aspect_ratios);
  my_free(min_sizes);
  my_free(max_sizes);
  my_free(variance);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

void TIDL_metaArchimportPriorBox3DODParams(PriorBox3DODParameter& prior_box_3dod_param,
float               **pPrioData,
int32_t              *priorDataSize,
sTIDL_DetectOutputParams_t & detectOutParams
)
{
  int32_t     status;
  int32_t id;
  int32_t j, img_height, img_width, num_priors;

  int32_t num_rotations = prior_box_3dod_param.rotation_size();
  int32_t num_scales = prior_box_3dod_param.anchor_length_size();

  sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(1, sizeof(sTIDL_AnchorBoxParams_t));
  anchorBox->numAnchors = num_rotations*num_scales;

  int32_t box_counter = 0;

  anchorBox->numKeyPoints = 0;

  if(prior_box_3dod_param.has_step_x())
  {
    anchorBox->strideW = prior_box_3dod_param.step_x();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "step_x is mandatory field and it should be provided as part of prior_box_3dod_param");
  }

  if(prior_box_3dod_param.has_step_y())
  {
    anchorBox->strideH = prior_box_3dod_param.step_y();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "step_y is mandatory field and it should be provided as part of prior_box_3dod_param");
  }

  if(prior_box_3dod_param.has_step_z())
  {
    anchorBox->strideL = prior_box_3dod_param.step_z();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "step_z is mandatory field and it should be provided as part of prior_box_3dod_param");
  }

  if(prior_box_3dod_param.has_offset_x())
  {
    anchorBox->offsetW = prior_box_3dod_param.offset_x();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "offset_x is mandatory field and it should be provided as part of prior_box_3dod_param");
  }

  if(prior_box_3dod_param.has_offset_y())
  {
    anchorBox->offsetH = prior_box_3dod_param.offset_y();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "offset_y is mandatory field and it should be provided as part of prior_box_3dod_param");
  }

  if(prior_box_3dod_param.has_offset_z())
  {
    anchorBox->offsetL = prior_box_3dod_param.offset_z();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "offset_z is mandatory field and it should be provided as part of prior_box_3dod_param");
  }

  if(prior_box_3dod_param.has_offset_dir())
  {
    anchorBox->offsetDir = prior_box_3dod_param.offset_dir();
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "offset_dir is mandatory field and it should be provided, -1.57 is used by default");
    anchorBox->offsetDir = -1.57;
  }

  for (int s = 0; s < num_scales; s++)
  {
    for(int r = 0; r < num_rotations; r++)
    {
      anchorBox->boxWidth[box_counter]  = prior_box_3dod_param.anchor_width(s);
      anchorBox->boxHeight[box_counter] = prior_box_3dod_param.anchor_height(s);
      anchorBox->boxLength[box_counter] = prior_box_3dod_param.anchor_length(s);
      anchorBox->boxAngle[box_counter] = prior_box_3dod_param.rotation(r);
      box_counter++;
    }
  }

  *pPrioData = (float *)anchorBox;
  *priorDataSize = sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);

}

#define TIDL_MA_MAX_SSD_HEAD (32)

int32_t tidl_metaArch_import(tidl_import_config * params)
{
  int32_t  i, j, k, l;
  TIDLMetaArch           tidlMetaArch;
  char defaultDetOpName[TIDL_STRING_SIZE];
  if (strcmp((char*)params->metaLayersNamesList, "") != 0)
  {
    numTFMetaLayers = tidl_getStringsFromList((char *)params->metaLayersNamesList, (char*)outMetaLayerNames, TIDL_MAX_DATA_NAME);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "No Meta Arch layer to parse");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  if(numTFMetaLayers > 1)
  {
    TIDL_GLOBAL_REPORT_ERROR("Only one metaArch file can be provided");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL Meta pipeLine (proto) file  : %s  ", (const char *)outMetaLayerNames[0]);
  bool success = TIDL_readProtoFromTextFile((const char *)outMetaLayerNames[0], &tidlMetaArch);
  if(!success){
    TIDL_GLOBAL_REPORT_ERROR("Reading prototext file failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  numTFMetaLayers = tidlMetaArch.caffe_ssd_size() +
                    tidlMetaArch.centernet_size() +
                    tidlMetaArch.tf_od_api_ssd_size() +
                    tidlMetaArch.tidl_faster_rcnn_size() +
                    tidlMetaArch.tidl_yolo_size() +
                    tidlMetaArch.tidl_retinanet_size() +
                    tidlMetaArch.tidl_3dod_size() +
                    tidlMetaArch.tidl_bevformer_size();

  int32_t numCurrOutputLayers = 0;
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.name().c_str());

  for (j = 0; j < tidlMetaArch.caffe_ssd_size(); j++)
  {
    sprintf(defaultDetOpName, "tidl_det_op_default_name_%d",j);

    if(tidlMetaArch.caffe_ssd(j).has_framework()) //Framework will be available as "MMDetection" for MMDetection networks in prototxt file
    {
      tidl_OdFramework = tidlMetaArch.caffe_ssd(j).framework();
    }
    else
    {
      tidl_OdFramework = "";
    }

    float *priorPtrs[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t priosSize[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t imWidth;
    int32_t imHeight;

    if(tidlMetaArch.caffe_ssd(j).has_in_width() && tidlMetaArch.caffe_ssd(j).has_in_height())
    {
      imWidth  = tidlMetaArch.caffe_ssd(j).in_width();
      imHeight = tidlMetaArch.caffe_ssd(j).in_height();
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("in_width or in_height not found in the Meta Arch proto text. These two are mandatory, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if(tidlMetaArch.caffe_ssd(j).output_size() >= 1)
    {
      // ideally it is assumed that tidlMetaArch.caffe_ssd(j).output_size() is one only
      for(k =0; k < tidlMetaArch.caffe_ssd(j).output_size(); k++)
      {
        strcpy((char *)tidl_OdOutputNames[numCurrOutputLayers + k],tidlMetaArch.caffe_ssd(j).output(k).c_str());
      }
      numCurrOutputLayers += tidlMetaArch.caffe_ssd(j).output_size();
    }
    else
    {
      strcpy((char *)tidl_OdOutputNames[j],defaultDetOpName);
    }

    for(k =0; k < tidlMetaArch.caffe_ssd(j).box_input_size(); k++)
    {
      strcpy((char *)tidl_OdBoxInputNames[j][k],tidlMetaArch.caffe_ssd(j).box_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdBoxInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.caffe_ssd(j).class_input_size(); k++)
    {
      strcpy((char *)tidl_OdClassInputNames[j][k],tidlMetaArch.caffe_ssd(j).class_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdClassInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.caffe_ssd(j).prior_box_param_size(); k++)
    {
      auto p = tidlMetaArch.caffe_ssd(j).prior_box_param(k);
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_metaArchimportPriorBoxParams(p,&priorPtrs[k],&priosSize[k], imWidth, imHeight), "");
    }

    // generic OD will have default information about prior box. as in yolov8 and centerpoint
    // if prior box is not provided then default values are populted
    if(tidlMetaArch.caffe_ssd(j).prior_box_param_size() == 0)
    {
      for(k =0; k < tidlMetaArch.caffe_ssd(j).box_input_size(); k++)
      {
        sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(1, sizeof(sTIDL_AnchorBoxParams_t));
        anchorBox->numAnchors = 1;
        anchorBox->numKeyPoints = 0;
        anchorBox->headWidth = -1;
        anchorBox->headHeight = -1;
        anchorBox->strideW = -1;
        anchorBox->strideH = -1;
        anchorBox->offsetH = 0.5;
        anchorBox->offsetW = 0.5;
        if( tidlMetaArch.caffe_ssd(j).detection_output_param().code_type()==CODE_TYPE_RTMDET)
        {
          anchorBox->offsetH = 0.0;
          anchorBox->offsetW = 0.0;
        }
        anchorBox->boxWidth[0] = 1.0f;
        anchorBox->boxHeight[0] = 1.0f;

        priorPtrs[k] = (float *)anchorBox;
        priosSize[k] = sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);

      }
    }

    auto d = tidlMetaArch.caffe_ssd(j).detection_output_param();
    TIDL_metaArchImportDetectionOutputParams(d,tidl_OdPostProc[j]);
    tidl_OdPostProc[j].numHeads = tidlMetaArch.caffe_ssd(j).box_input_size();
    tidl_OdPostProc[j].numSubHeads = 1;
    tidl_OdPostProc[j].imWidth  = imWidth;
    tidl_OdPostProc[j].imHeight = imHeight;

    int32_t totalPriorSize = 1;

    for(k =0; k < tidl_OdPostProc[j].numHeads; k++)
    {
      totalPriorSize+= priosSize[k];
    }

    tidlPriorPtrs[j] = (float*)my_malloc(tidl_OdPostProc[j].numHeads*sizeof(sTIDL_AnchorBoxParams_t));

    for(k =0; k < tidl_OdPostProc[j].numHeads; k++)
    {
      float *priorData = priorPtrs[k];
      memcpy((tidlPriorPtrs[j] + (k*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float))), priorData, sizeof(sTIDL_AnchorBoxParams_t));
      my_free(priorData);
    }
    tidl_OdPostProc[j].priorBoxSize = tidl_OdPostProc[j].numHeads*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);
    tidl_OdPostProc[j].scoreConverter = tidlMetaArch.caffe_ssd(j).has_score_converter() ? tidlMetaArch.caffe_ssd(j).score_converter() : SOFTMAX;
    tidl_OdPostProc[j].dataLayout = tidlMetaArch.caffe_ssd(j).data_layout();
    tidl_OdPostProc[j].metaArchType = params->metaArchType;
  }

  for (j = 0; j < tidlMetaArch.centernet_size(); j++)
  {
    sprintf(defaultDetOpName, "tidl_det_op_default_name_%d",j);

    if(tidlMetaArch.centernet(j).has_framework()) //Framework will be available as "MMDetection" for MMDetection networks in prototxt file
    {
      tidl_OdFramework = tidlMetaArch.centernet(j).framework();
    }
    else
    {
      tidl_OdFramework = "";
    }

    float *priorPtrs[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t priosSize[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t imWidth;
    int32_t imHeight;

    if(tidlMetaArch.centernet(j).has_in_width() && tidlMetaArch.centernet(j).has_in_height())
    {
      imWidth  = tidlMetaArch.centernet(j).in_width();
      imHeight = tidlMetaArch.centernet(j).in_height();
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("in_width or in_height not found in the Meta Arch proto text. These two are mandatory, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if(tidlMetaArch.centernet(j).output_size() >= 1)
    {
      // ideally it is assumed that tidlMetaArch.centernet(j).output_size() is one only
      for(k =0; k < tidlMetaArch.centernet(j).output_size(); k++)
      {
        strcpy((char *)tidl_OdOutputNames[numCurrOutputLayers + k],tidlMetaArch.centernet(j).output(k).c_str());
      }
      numCurrOutputLayers += tidlMetaArch.centernet(j).output_size();
    }
    else
    {
      strcpy((char *)tidl_OdOutputNames[j],defaultDetOpName);
    }

    for(k =0; k < tidlMetaArch.centernet(j).box_input_size(); k++)
    {
      strcpy((char *)tidl_OdBoxInputNames[j][k],tidlMetaArch.centernet(j).box_input(k).name().c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdBoxInputNames[j][k]);
    }

    tidl_OdPostProc[j].reserve0 =0;
    tidl_OdPostProc[j].reserve1 = tidlMetaArch.centernet(j).class_input(0).has_start_channel() ? tidlMetaArch.centernet(j).class_input(0).start_channel() : -1 ;

    for(k =0; k < tidlMetaArch.centernet(j).reg_input_size(); k++)
    {
      tidl_OdPostProc[j].reserve0 =1;
      strcpy((char *)tidl_OdRegInput0Names[j][k],tidlMetaArch.centernet(j).reg_input(k).name().c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdRegInput0Names[j][k]);
    }
    
  
    for(k =0; k < tidlMetaArch.centernet(j).class_input_size(); k++)
    { 
      
      strcpy((char *)tidl_OdClassInputNames[j][k],tidlMetaArch.centernet(j).class_input(k).name().c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdClassInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.centernet(j).prior_box_param_size(); k++)
    {
      auto p = tidlMetaArch.centernet(j).prior_box_param(k);
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_metaArchimportPriorBoxParams(p,&priorPtrs[k],&priosSize[k], imWidth, imHeight), "");
    }

    // generic OD will have default information about prior box. as in yolov8 and centerpoint
    // if prior box is not provided then default values are populted
    if(tidlMetaArch.centernet(j).prior_box_param_size() == 0)
    {
      
      // for(k =0; k < tidlMetaArch.centernet(j).box_input_size(); k++)
      for(k =0; k < tidlMetaArch.centernet(j).class_input_size(); k++)
      {
        sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(1, sizeof(sTIDL_AnchorBoxParams_t));
        anchorBox->numAnchors = 1;
        anchorBox->numKeyPoints = 0;
        anchorBox->headWidth = -1;
        anchorBox->headHeight = -1;
        anchorBox->strideW = -1;
        anchorBox->strideH = -1;
        anchorBox->offsetH = 0.5;
        anchorBox->offsetW = 0.5;
        anchorBox->boxWidth[0] = 1.0f;
        anchorBox->boxHeight[0] = 1.0f;

        priorPtrs[k] = (float *)anchorBox;
        priosSize[k] = sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);

      }
    }

    auto d = tidlMetaArch.centernet(j).detection_output_param();
    TIDL_metaArchImportDetectionOutputParams(d,tidl_OdPostProc[j]);
    tidl_OdPostProc[j].numHeads = tidlMetaArch.centernet(j).class_input_size();
    tidl_OdPostProc[j].numSubHeads = 1;
    tidl_OdPostProc[j].imWidth  = imWidth;
    tidl_OdPostProc[j].imHeight = imHeight;

    int32_t totalPriorSize = 1;

    for(k =0; k < tidl_OdPostProc[j].numHeads; k++)
    {
      totalPriorSize+= priosSize[k];
    }

    tidlPriorPtrs[j] = (float*)my_malloc(tidl_OdPostProc[j].numHeads*sizeof(sTIDL_AnchorBoxParams_t));

    for(k =0; k < tidl_OdPostProc[j].numHeads; k++)
    {
      float *priorData = priorPtrs[k];
      memcpy((tidlPriorPtrs[j] + (k*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float))), priorData, sizeof(sTIDL_AnchorBoxParams_t));
      my_free(priorData);
    }
    tidl_OdPostProc[j].priorBoxSize = tidl_OdPostProc[j].numHeads*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);
    tidl_OdPostProc[j].scoreConverter = tidlMetaArch.centernet(j).has_score_converter() ? tidlMetaArch.centernet(j).score_converter() : SOFTMAX;
    tidl_OdPostProc[j].dataLayout = tidlMetaArch.centernet(j).data_layout();
    tidl_OdPostProc[j].metaArchType = params->metaArchType;
  }

  for (j = 0; j < tidlMetaArch.tidl_retinanet_size(); j++)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_retinanet(j).name().c_str());

    sprintf(defaultDetOpName, "tidl_retina_det_op_default_name_%d",j);

    if(tidlMetaArch.tidl_retinanet(j).has_framework()) //Framework will be available as "MMDetection" for MMDetection networks in prototxt file
    {
      tidl_OdFramework = tidlMetaArch.tidl_retinanet(j).framework();
    }
    else
    {
      tidl_OdFramework = "";
    }

    float *priorPtrs[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t priosSize[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t imWidth;
    int32_t imHeight;

    if(tidlMetaArch.tidl_retinanet(j).has_in_width() && tidlMetaArch.tidl_retinanet(j).has_in_height())
    {
      imWidth  = tidlMetaArch.tidl_retinanet(j).in_width();
      imHeight = tidlMetaArch.tidl_retinanet(j).in_height();
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("in_width or in_height not found in the Meta Arch proto text. These two are mandatory, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if(tidlMetaArch.tidl_retinanet(j).output_size() >= 1)
    {
      // ideally it is assumed that tidlMetaArch.caffe_ssd(j).output_size() is one only
      for(k =0; k < tidlMetaArch.tidl_retinanet(j).output_size(); k++)
      {
        strcpy((char *)tidl_OdOutputNames[numCurrOutputLayers + k],tidlMetaArch.tidl_retinanet(j).output(k).c_str());
      }
      numCurrOutputLayers += tidlMetaArch.tidl_retinanet(j).output_size();
    }
    else
    {
      strcpy((char *)tidl_OdOutputNames[j],defaultDetOpName);
    }

    for(k =0; k < tidlMetaArch.tidl_retinanet(j).box_input_size(); k++)
    {
      strcpy((char *)tidl_OdBoxInputNames[j][k],tidlMetaArch.tidl_retinanet(j).box_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdBoxInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.tidl_retinanet(j).class_input_size(); k++)
    {
      strcpy((char *)tidl_OdClassInputNames[j][k],tidlMetaArch.tidl_retinanet(j).class_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdClassInputNames[j][k]);
    }
    if(tidlMetaArch.tidl_retinanet(j).box_input_size() !=
       tidlMetaArch.tidl_retinanet(j).class_input_size())
    {
      TIDL_GLOBAL_REPORT_ERROR("Number of class heads and box heads is not matching, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    int32_t numHeads = tidlMetaArch.tidl_retinanet(j).box_input_size();
    if (numHeads == 0)
    {
      TIDL_GLOBAL_REPORT_ERROR("Number of head has to be minimum one, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    float x_scale = tidlMetaArch.tidl_retinanet(j).x_scale();
    float y_scale = tidlMetaArch.tidl_retinanet(j).y_scale();
    float width_scale = tidlMetaArch.tidl_retinanet(j).width_scale();
    float height_scale = tidlMetaArch.tidl_retinanet(j).height_scale();

    float octave_base_scale  = tidlMetaArch.tidl_retinanet(j).anchor_param().octave_base_scale();
    int32_t scales_per_octave = tidlMetaArch.tidl_retinanet(j).anchor_param().scales_per_octave();
    int32_t num_aspect_ratio = tidlMetaArch.tidl_retinanet(j).anchor_param().aspect_ratio_size();
    if ((num_aspect_ratio == 0) || (scales_per_octave == 0))
    {
      TIDL_GLOBAL_REPORT_ERROR("Number of num_aspect_ratio and scales_per_octave has to be minimum one, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(numHeads, sizeof(sTIDL_AnchorBoxParams_t));
    for(k =0; k < numHeads; k++)
    {
      anchorBox[k].numKeyPoints = 0;
      anchorBox[k].headWidth = 0;
      anchorBox[k].headHeight = 0;
      anchorBox[k].strideW = 0;
      anchorBox[k].strideH = 0;
      anchorBox[k].offsetH = tidlMetaArch.tidl_retinanet(j).y_offset();
      anchorBox[k].offsetW = tidlMetaArch.tidl_retinanet(j).x_offset();
      anchorBox[k].boxScales[0] = 1/x_scale;
      anchorBox[k].boxScales[1] = 1/y_scale;
      anchorBox[k].boxScales[2] = 1/width_scale;
      anchorBox[k].boxScales[3] = 1/height_scale;
      anchorBox[k].numAnchors = scales_per_octave*num_aspect_ratio;
	  uint32_t anchorbox_order = tidlMetaArch.tidl_retinanet(j).anchorbox_order();
	  if (!(anchorbox_order))
	  {
        for (l = 0; l < num_aspect_ratio; l++)
        {
          for (i = 0; i < scales_per_octave; i++)
          {

            float ar = tidlMetaArch.tidl_retinanet(j).anchor_param().aspect_ratio(l);
            float size = octave_base_scale * pow(2.0, ((i*1.0) / scales_per_octave));
            anchorBox[k].boxWidth[l*scales_per_octave + i]  = (size/sqrt(ar))/imWidth;
            anchorBox[k].boxHeight[l*scales_per_octave + i] = (size*sqrt(ar))/imHeight;
          }
        }
      }
      else
      {
        for (i = 0; i < scales_per_octave; i++)
        {
          for (l = 0; l < num_aspect_ratio; l++)
          {
            float ar = tidlMetaArch.tidl_retinanet(j).anchor_param().aspect_ratio(l);
            float size = octave_base_scale * pow(2.0, ((i*1.0) / scales_per_octave));
            anchorBox[k].boxWidth[i*num_aspect_ratio + l]  = (size*sqrt(ar))/imWidth;
            anchorBox[k].boxHeight[i*num_aspect_ratio + l] = (size/sqrt(ar))/imHeight;
          }
        }
      }
    }
    auto d = tidlMetaArch.tidl_retinanet(j).detection_output_param();
    TIDL_metaArchImportDetectionOutputParams(d,tidl_OdPostProc[j]);
    tidl_OdPostProc[j].reserve0 =0;
    tidl_OdPostProc[j].reserve1 = -1;
    tidl_OdPostProc[j].numHeads = numHeads;
    tidl_OdPostProc[j].numSubHeads = 1;
    tidl_OdPostProc[j].imWidth  = imWidth;
    tidl_OdPostProc[j].imHeight = imHeight;
    tidl_OdPostProc[j].metaArchType = TIDL_metaArchTIDLRetinaNet;
    tidlPriorPtrs[0] = (float *)anchorBox;
    tidl_OdPostProc[j].priorBoxSize = tidl_OdPostProc[j].numHeads * sizeof(sTIDL_AnchorBoxParams_t) / sizeof(float);
    tidl_OdPostProc[j].scoreConverter = tidlMetaArch.tidl_retinanet(j).has_score_converter() ? tidlMetaArch.tidl_retinanet(j).score_converter() : SOFTMAX;
    tidl_OdPostProc[j].codeType = tidlMetaArch.tidl_retinanet(j).detection_output_param().code_type();
    tidl_OdPostProc[j].dataLayout = 0;
  }
  for (j = 0; j < tidlMetaArch.tidl_yolo_size(); j++)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_yolo(j).name().c_str());
    sprintf(defaultDetOpName, "tidl_yolo_det_op_default_name_%d",j);

    if(tidlMetaArch.tidl_yolo(j).has_framework()) //Framework will be available as "MMDetection" for MMDetection networks in prototxt file
    {
      tidl_OdFramework = tidlMetaArch.tidl_yolo(j).framework();
    }
    else
    {
      tidl_OdFramework = "";
    }

    float *priorPtrs[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t priosSize[TIDL_MA_MAX_SSD_HEAD] = {0};

    int32_t imWidth  = tidlMetaArch.tidl_yolo(j).in_width();
    int32_t imHeight = tidlMetaArch.tidl_yolo(j).in_height();

    if(tidlMetaArch.tidl_yolo(j).output_size() >= 1)
    {
      // ideally it is assumed that tidlMetaArch.caffe_ssd(j).output_size() is one only
      for(int k = 0; k < tidlMetaArch.tidl_yolo(j).output_size(); k++)
      {
        strcpy((char *)tidl_OdOutputNames[numCurrOutputLayers + k],tidlMetaArch.tidl_yolo(j).output(k).c_str());
      }
      numCurrOutputLayers += tidlMetaArch.tidl_yolo(j).output_size();
    }
    else
    {
      strcpy((char *)tidl_OdOutputNames[j],defaultDetOpName);
    }

    int32_t numHeads = tidlMetaArch.tidl_yolo(j).yolo_param_size();
    if(numHeads == 0)
    {
      TIDL_GLOBAL_REPORT_ERROR("Number of yolo head has to be minimum one, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(numHeads, sizeof(sTIDL_AnchorBoxParams_t));


    for(k =0; k < numHeads; k++)
    {
      strcpy((char *)tidl_OdBoxInputNames[j][k],tidlMetaArch.tidl_yolo(j).yolo_param(k).input().c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdBoxInputNames[j][k]);

      if(((tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_width_size() !=
          tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_height_size())
          || (tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_width_size() == 0)) && (params->metaArchType != TIDL_metaArchTIDLYoloV8))
      {
        TIDL_GLOBAL_REPORT_ERROR("yolo_param_anchor_width_size and yolo_param_anchor_height_size is not matching, exiting");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      anchorBox[k].numAnchors = tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_width_size();
      for (i = 0; i < tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_width_size(); i++)
      {
        anchorBox[k].boxWidth[i]  = tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_width(i);
        anchorBox[k].boxHeight[i] = tidlMetaArch.tidl_yolo(j).yolo_param(k).anchor_height(i);

        if(anchorBox[k].boxWidth[i] > 1)
        {
          anchorBox[k].boxWidth[i] = anchorBox[k].boxWidth[i] / imWidth;
        }
        if(anchorBox[k].boxHeight[i] > 1)
        {
          anchorBox[k].boxHeight[i] = anchorBox[k].boxHeight[i] / imHeight;
        }
      }
      anchorBox[k].numKeyPoints = tidlMetaArch.tidl_yolo(j).detection_output_param().num_keypoint();
    }
    auto d = tidlMetaArch.tidl_yolo(j).detection_output_param();
    TIDL_metaArchImportDetectionOutputParams(d,tidl_OdPostProc[j]);
    tidl_OdPostProc[j].numHeads = numHeads;
    tidl_OdPostProc[j].numSubHeads = 1;
    tidl_OdPostProc[j].reserve0 =  0;
    tidl_OdPostProc[j].reserve1 = -1;
    tidl_OdPostProc[j].imWidth  = imWidth;
    tidl_OdPostProc[j].imHeight = imHeight;
    tidl_OdPostProc[j].metaArchType = TIDL_metaArchTIDLYolo;
    tidlPriorPtrs[0] = (float *)anchorBox;
    tidl_OdPostProc[j].priorBoxSize = tidl_OdPostProc[j].numHeads * sizeof(sTIDL_AnchorBoxParams_t) / sizeof(float);
    tidl_OdPostProc[j].scoreConverter = SIGMOID; // import tool should parse this information from prototext file
    //tidl_OdPostProc[j].codeType = 4; //4 for yolov3 and 6 for yolov5
    tidl_OdPostProc[j].codeType = tidlMetaArch.tidl_yolo(j).detection_output_param().code_type();
    tidl_OdPostProc[j].dataLayout = 0;
  }

  for (j = 0; j < tidlMetaArch.tidl_3dod_size(); j++)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_3dod(j).name().c_str());
    sprintf(defaultDetOpName, "tidl_3dod_det_op_default_name_%d",j);

    float *priorPtrs[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t priosSize[TIDL_MA_MAX_SSD_HEAD] = {0};

    float min_x, min_y, min_z, max_x, max_y, max_z;

    if(tidlMetaArch.tidl_3dod(j).has_min_x() &&
       tidlMetaArch.tidl_3dod(j).has_max_x() &&
       tidlMetaArch.tidl_3dod(j).has_min_y() &&
       tidlMetaArch.tidl_3dod(j).has_max_y() &&
       tidlMetaArch.tidl_3dod(j).has_min_z() &&
       tidlMetaArch.tidl_3dod(j).has_max_z()
       )
    {
      min_x  = tidlMetaArch.tidl_3dod(j).min_x();
      max_x  = tidlMetaArch.tidl_3dod(j).max_x();
      min_y  = tidlMetaArch.tidl_3dod(j).min_y();
      max_y  = tidlMetaArch.tidl_3dod(j).max_y();
      min_z  = tidlMetaArch.tidl_3dod(j).min_z();
      max_z  = tidlMetaArch.tidl_3dod(j).max_z();
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("min_x/y/z or max_x/y/z not found in the Meta Arch proto text. These two are mandatory, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    tidl_OdPostProc[j].point3dRange[0][0] = min_x;
    tidl_OdPostProc[j].point3dRange[0][1] = max_x;
    tidl_OdPostProc[j].point3dRange[1][0] = min_y;
    tidl_OdPostProc[j].point3dRange[1][1] = max_y;
    tidl_OdPostProc[j].point3dRange[2][0] = min_z;
    tidl_OdPostProc[j].point3dRange[2][1] = max_z;

    tidl_OdPostProc[j].reserve0 =  1; /*3D OD also has extra head for dir of objects.*/
    tidl_OdPostProc[j].reserve1 = -1;

    if(tidlMetaArch.tidl_3dod(j).has_voxel_size_x())
    {
      tidl_OdPostProc[j].voxelSize[0] = tidlMetaArch.tidl_3dod(j).voxel_size_x();
    }

    if(tidlMetaArch.tidl_3dod(j).has_voxel_size_y())
    {
      tidl_OdPostProc[j].voxelSize[1] = tidlMetaArch.tidl_3dod(j).voxel_size_y();
    }

    if(tidlMetaArch.tidl_3dod(j).has_voxel_size_z())
    {
      tidl_OdPostProc[j].voxelSize[2] = tidlMetaArch.tidl_3dod(j).voxel_size_z();
    }

    if(tidlMetaArch.tidl_3dod(j).has_max_points_per_voxel())
    {
      tidl_OdPostProc[j].maxPointsPerVoxel = tidlMetaArch.tidl_3dod(j).max_points_per_voxel();
    }

    tidl_OdPostProc[j].numHeads = tidlMetaArch.tidl_3dod(j).box_input_size();
    tidl_OdPostProc[j].numSubHeads = tidlMetaArch.tidl_3dod(j).prior_box_3dod_param_size()/tidl_OdPostProc[j].numHeads;

    if((tidlMetaArch.tidl_3dod(j).box_input_size() != tidlMetaArch.tidl_3dod(j).class_input_size()) ||
       (tidlMetaArch.tidl_3dod(j).box_input_size() != tidlMetaArch.tidl_3dod(j).dir_input_size())
      )
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Number of box_input, class_input and dir_input should match ");
    }

    for(k =0; k < tidlMetaArch.tidl_3dod(j).box_input_size(); k++)
    {
      strcpy((char *)tidl_OdBoxInputNames[j][k],tidlMetaArch.tidl_3dod(j).box_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdBoxInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.tidl_3dod(j).class_input_size(); k++)
    {
      strcpy((char *)tidl_OdClassInputNames[j][k],tidlMetaArch.tidl_3dod(j).class_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdClassInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.tidl_3dod(j).dir_input_size(); k++)
    {
      strcpy((char *)tidl_OdDirInputNames[j][k],tidlMetaArch.tidl_3dod(j).dir_input(k).c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdDirInputNames[j][k]);
    }
    if(tidlMetaArch.tidl_3dod(j).output_size() >= 1)
    {
      // ideally it is assumed that tidlMetaArch.caffe_ssd(j).output_size() is one only
      for(k =0; k < tidlMetaArch.tidl_3dod(j).output_size(); k++)
      {
        strcpy((char *)tidl_OdOutputNames[numCurrOutputLayers + k],tidlMetaArch.tidl_3dod(j).output(k).c_str());
      }
      numCurrOutputLayers += tidlMetaArch.tidl_3dod(j).output_size();
    }
    else
    {
      strcpy((char *)tidl_OdOutputNames[j],defaultDetOpName);
    }

    for(k =0; k < tidlMetaArch.tidl_3dod(j).prior_box_3dod_param_size(); k++)
    {
      auto p = tidlMetaArch.tidl_3dod(j).prior_box_3dod_param(k);
      TIDL_metaArchimportPriorBox3DODParams(p,&priorPtrs[k],&priosSize[k],tidl_OdPostProc[j]);
    }

    auto d = tidlMetaArch.tidl_3dod(j).detection_output_param();
    TIDL_metaArchImportDetectionOutputParams(d,tidl_OdPostProc[j]);

    tidlPriorPtrs[j] = (float*)my_malloc(tidlMetaArch.tidl_3dod(j).prior_box_3dod_param_size()*sizeof(sTIDL_AnchorBoxParams_t));

    for(k =0; k < tidlMetaArch.tidl_3dod(j).prior_box_3dod_param_size(); k++)
    {
      float *priorData = priorPtrs[k];
      memcpy((tidlPriorPtrs[j] + (k*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float))), priorData, sizeof(sTIDL_AnchorBoxParams_t));
      my_free(priorData);
    }
    tidl_OdPostProc[j].priorBoxSize = tidlMetaArch.tidl_3dod(j).prior_box_3dod_param_size()*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);
    tidl_OdPostProc[j].scoreConverter = tidlMetaArch.tidl_3dod(j).has_score_converter() ? tidlMetaArch.tidl_3dod(j).score_converter() : SIGMOID;
    tidl_OdPostProc[j].dataLayout = tidlMetaArch.tidl_3dod(j).data_layout();
    tidl_OdPostProc[j].metaArchType = TIDL_metaArchTIDL3DOD;
  }
  for (j = 0; j < tidlMetaArch.tidl_bevformer_size(); j++)
  {
    sprintf(defaultDetOpName, "tidl_det_op_default_name_%d",j);

    if(tidlMetaArch.tidl_bevformer(j).has_framework()) //Framework will be available as "MMDetection" for MMDetection networks in prototxt file
    {
      tidl_OdFramework = tidlMetaArch.tidl_bevformer(j).framework();
    }
    else
    {
      tidl_OdFramework = "";
    }

    float *priorPtrs[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t priosSize[TIDL_MA_MAX_SSD_HEAD] = {0};
    int32_t imWidth;
    int32_t imHeight;

    if(tidlMetaArch.tidl_bevformer(j).has_in_width() && tidlMetaArch.tidl_bevformer(j).has_in_height())
    {
      imWidth  = tidlMetaArch.tidl_bevformer(j).in_width();
      imHeight = tidlMetaArch.tidl_bevformer(j).in_height();
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("in_width or in_height not found in the Meta Arch proto text. These two are mandatory, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    float min_x, min_y, min_z, max_x, max_y, max_z;

    if(tidlMetaArch.tidl_bevformer(j).has_min_x() &&
       tidlMetaArch.tidl_bevformer(j).has_max_x() &&
       tidlMetaArch.tidl_bevformer(j).has_min_y() &&
       tidlMetaArch.tidl_bevformer(j).has_max_y() &&
       tidlMetaArch.tidl_bevformer(j).has_min_z() &&
       tidlMetaArch.tidl_bevformer(j).has_max_z()
       )
    {
      min_x  = tidlMetaArch.tidl_bevformer(j).min_x();
      max_x  = tidlMetaArch.tidl_bevformer(j).max_x();
      min_y  = tidlMetaArch.tidl_bevformer(j).min_y();
      max_y  = tidlMetaArch.tidl_bevformer(j).max_y();
      min_z  = tidlMetaArch.tidl_bevformer(j).min_z();
      max_z  = tidlMetaArch.tidl_bevformer(j).max_z();
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("min_x/y/z or max_x/y/z not found in the Meta Arch proto text. These two are mandatory, exiting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    tidl_OdPostProc[j].point3dRange[0][0] = min_x;
    tidl_OdPostProc[j].point3dRange[0][1] = max_x;
    tidl_OdPostProc[j].point3dRange[1][0] = min_y;
    tidl_OdPostProc[j].point3dRange[1][1] = max_y;
    tidl_OdPostProc[j].point3dRange[2][0] = min_z;
    tidl_OdPostProc[j].point3dRange[2][1] = max_z;

    if(tidlMetaArch.tidl_bevformer(j).output_size() >= 1)
    {
      // ideally it is assumed that tidlMetaArch.tidl_bevformer(j).output_size() is one only
      for(k =0; k < tidlMetaArch.tidl_bevformer(j).output_size(); k++)
      {
        strcpy((char *)tidl_OdOutputNames[numCurrOutputLayers + k],tidlMetaArch.tidl_bevformer(j).output(k).c_str());
      }
      numCurrOutputLayers += tidlMetaArch.tidl_bevformer(j).output_size();
    }
    else
    {
      strcpy((char *)tidl_OdOutputNames[j],defaultDetOpName);
    }

    for(k =0; k < tidlMetaArch.tidl_bevformer(j).box_input_size(); k++)
    {
      strcpy((char *)tidl_OdBoxInputNames[j][k],tidlMetaArch.tidl_bevformer(j).box_input(k).name().c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdBoxInputNames[j][k]);
    }

    tidl_OdPostProc[j].reserve0 =0;
    tidl_OdPostProc[j].reserve1 = tidlMetaArch.tidl_bevformer(j).class_input(0).has_start_channel() ? tidlMetaArch.tidl_bevformer(j).class_input(0).start_channel() : -1 ;
    
    for(k =0; k < tidlMetaArch.tidl_bevformer(j).class_input_size(); k++)
    { 
      
      strcpy((char *)tidl_OdClassInputNames[j][k],tidlMetaArch.tidl_bevformer(j).class_input(k).name().c_str());
      strcat((char *)params->outDataNamesList,",");
      strcat((char *)params->outDataNamesList,(char *)tidl_OdClassInputNames[j][k]);
    }

    for(k =0; k < tidlMetaArch.tidl_bevformer(j).prior_box_param_size(); k++)
    {
      auto p = tidlMetaArch.tidl_bevformer(j).prior_box_param(k);
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_metaArchimportPriorBoxParams(p,&priorPtrs[k],&priosSize[k], imWidth, imHeight), "");
    }

    // generic OD will have default information about prior box. as in yolov8, centerpoint and bevformer
    // if prior box is not provided then default values are populted
    if(tidlMetaArch.tidl_bevformer(j).prior_box_param_size() == 0)
    {
      
      // for(k =0; k < tidlMetaArch.tidl_bevformer(j).box_input_size(); k++)
      for(k =0; k < tidlMetaArch.tidl_bevformer(j).class_input_size(); k++)
      {
        sTIDL_AnchorBoxParams_t* anchorBox = (sTIDL_AnchorBoxParams_t*)my_calloc(1, sizeof(sTIDL_AnchorBoxParams_t));
        anchorBox->numAnchors = 1;
        anchorBox->numKeyPoints = 0;
        anchorBox->headWidth = -1;
        anchorBox->headHeight = -1;
        anchorBox->strideW = -1;
        anchorBox->strideH = -1;
        anchorBox->offsetH = 0.5;
        anchorBox->offsetW = 0.5;
        anchorBox->boxWidth[0] = 1.0f;
        anchorBox->boxHeight[0] = 1.0f;

        priorPtrs[k] = (float *)anchorBox;
        priosSize[k] = sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);

      }
    }

    auto d = tidlMetaArch.tidl_bevformer(j).detection_output_param();
    TIDL_metaArchImportDetectionOutputParams(d,tidl_OdPostProc[j]);
    tidl_OdPostProc[j].numHeads = tidlMetaArch.tidl_bevformer(j).class_input_size();
    tidl_OdPostProc[j].numSubHeads = 1;
    tidl_OdPostProc[j].imWidth  = imWidth;
    tidl_OdPostProc[j].imHeight = imHeight;

    int32_t totalPriorSize = 1;

    for(k =0; k < tidl_OdPostProc[j].numHeads; k++)
    {
      totalPriorSize+= priosSize[k];
    }

    tidlPriorPtrs[j] = (float*)my_malloc(tidl_OdPostProc[j].numHeads*sizeof(sTIDL_AnchorBoxParams_t));

    for(k =0; k < tidl_OdPostProc[j].numHeads; k++)
    {
      float *priorData = priorPtrs[k];
      memcpy((tidlPriorPtrs[j] + (k*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float))), priorData, sizeof(sTIDL_AnchorBoxParams_t));
      my_free(priorData);
    }
    tidl_OdPostProc[j].priorBoxSize = tidl_OdPostProc[j].numHeads*sizeof(sTIDL_AnchorBoxParams_t)/sizeof(float);
    tidl_OdPostProc[j].scoreConverter = tidlMetaArch.tidl_bevformer(j).has_score_converter() ? tidlMetaArch.tidl_bevformer(j).score_converter() : SIGMOID;
    tidl_OdPostProc[j].dataLayout = tidlMetaArch.tidl_bevformer(j).data_layout();
    tidl_OdPostProc[j].metaArchType = params->metaArchType;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
