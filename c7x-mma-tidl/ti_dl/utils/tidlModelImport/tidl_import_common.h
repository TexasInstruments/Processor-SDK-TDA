/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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

#ifndef TIDL_IMPORT_COMMONH_
#define TIDL_IMPORT_COMMONH_ 1

#include "perfsim.h"
#include <google/protobuf/message.h>
using ::google::protobuf::Message;

#define SAT_HI_UINT8   (255)
#define ALIGN_NET_DATA (64)
#define TF_PAD_SAME (0)
#define TF_PAD_VALID (1)
#define TIDL_BATCHPADMAXVALUE (8)
#define TIDL_MAX_BATCH_TO_PUSH (32)

/* LFMP constraint dimensions */
#define TIDL_MAX_WIDTH_VALUE (65536)

bool TIDL_readProtoFromTextFile(const char* fileName, Message* proto);

bool TIDL_readProtoFromBinaryFile(const char* fileName, Message* proto);

int32_t TIDL_getDataID(sTIDL_DataParams_t *data,
sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
int32_t            numLayer,
int8_t             *bufName);

int32_t TIDL_isDataBufUsed(int32_t           dataId,
sTIDL_Network_t   *pTIDLNetStructure,
int32_t           numLayer);
int32_t TIDL_isInputConv2D(sTIDL_OrgNetwork_t   *pOrgTIDLNetStruct,
int32_t              numLayer,
const char           *bufName);
void TIDL_UpdateInDataBuff(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
uint32_t numLayers, sTIDL_DataParams_t dataBuf);

template <typename T>
void TIDL_findRange(T * data, int32_t dataSize, float * minOut, float * maxOut, float scale);
int32_t TIDL_alignParamsWrite(FILE *fp, sBuffer_t * buf, uint32_t *totalParamSize, uint32_t numBytes);

void TIDL_asymAllocScalesPointers(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, uint32_t numLayers);
void TIDL_setDefaultKernelType(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, uint32_t numLayers);
void TIDL_allocAndCopyModelParams(sTIDL_OrgNetwork_t * orgTIDLNetStructureDst,sTIDL_OrgNetwork_t * orgTIDLNetStructureSrc, uint32_t numLayers);
void TIDL_copyModelParams(sTIDL_OrgNetwork_t * orgTIDLNetStructureDst,sTIDL_OrgNetwork_t * orgTIDLNetStructureSrc, uint32_t numLayers);
void TIDL_freeModelParams(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, uint32_t numLayers);
int32_t tidl_isLayerMatMul(sTIDL_LayerPC_t& layer);

int32_t TIDL_writeModel(sTIDL_Network_t * tIDLNetStructure,
                                            sTIDL_OrgNetwork_t * orgTIDLNetStructure,
                                            const char * name,
                                            uint32_t numLayers,
                                            tidl_import_config * configParams);

void  TIDL_fillDataBufPadRequirements(sTIDL_Network_t * tIDLNetStructure);
void * my_malloc(int32_t size);
void * my_calloc(int32_t num, int32_t size);
void my_free(void *ptr);
int32_t TIDL_writeInfo(const sTIDL_Network_t * tIDLNetStructure,
                                        const sTIDL_OrgNetwork_t * orgTIDLNetStructure,
                                        const char * name,
                                        uint32_t numLayers,
                                        uint32_t currLayersGroupId,
                                        const sPerfSim_t * perfSimInfo,
                                        const tidl_import_config * configParams);

#define NUM_WHGT_BITS   (gParams.numParamBits)
#define NUM_BIAS_BITS   (8)
#define PRINT_TENSOR_MINMAX (0)
#define LAYER_INFO_FILENAME "layer_info.txt"

#define TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED (-2)
#define TIDL_ALLOWLISTING_LAYER_CHECK_FAILED (-1)
#define TIDL_ALLOWLISTING_LAYER_CHECK_PASSED (0)

int32_t TIDL_getLayerIdx(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t numLayer, const char *bufName);
typedef struct {
  int32_t layerType;
  int32_t(*tidl_tfOutReshape)(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
    int32_t              layerIndex);
}sTIDL_tfOutReshapeMap_t;
extern sTIDL_tfOutReshapeMap_t sTIDL_tfOutReshapeTable[];

typedef struct {
  char           *layerName;
  char           *layerOpsString;
  uint32_t       NumOps;
} TIDL_TFLayerMapping_t;

#define TIDL_MAX_HEADS_TF_SSD   (16)
#define TIDL_MAX_AR_PER_HEAD    (16)

#define MAX_NUM_DETECTION_LAYER (4)

#define TIDL_MAX_TF_SSD_LAYERS  (MAX_NUM_DETECTION_LAYER)
#define TIDL_MAX_TF_FASTER_RCNN_LAYERS  (MAX_NUM_DETECTION_LAYER)

#define PRECISION_LAYER_DEFAULT          (-1)
#define PRECISION_LAYER_FEATURE_IN_16BIT (1)
#define PRECISION_LAYER_PARAMS_IN_16BIT  (0)
#define PRECISION_LAYER_FEATURE_IN_16BIT_INDUCED  (1)
typedef struct {
  int32_t  num_classes;
  int32_t  num_keypoints;
  int32_t  num_aspect_ratios;
  int32_t  num_layers;
  int32_t  max_detections_per_class;
  int32_t  max_total_detections;
  int32_t  score_converter;
  int32_t  encode_background_as_zeros;
  float    y_scale;
  float    x_scale;
  float    height_scale;
  float    width_scale;
  float    scales[TIDL_MAX_HEADS_TF_SSD + 1];
  float    aspect_ratios[TIDL_MAX_AR_PER_HEAD];
  float    interpolated_scale_aspect_ratio;
  float    reduce_boxes_in_lowest_layer;
  float    base_anchor_height;
  float    base_anchor_width;
  float    height_stride[TIDL_MAX_HEADS_TF_SSD];
  float    width_stride[TIDL_MAX_HEADS_TF_SSD];
  float    height_offset[TIDL_MAX_HEADS_TF_SSD];
  float    width_offset[TIDL_MAX_HEADS_TF_SSD];
  float    score_threshold;
  float    iou_threshold;
  float *   anchorInputsFromModel;
  std::vector<int> num_ars;
  std::vector<string> box_input;
  std::vector<string> class_input;
} TIDL_TFSSDConfig_t;


typedef struct {
  //:TODO: Rename macros to RCNN
  int32_t num_scales;
  int32_t num_aspect_ratios;
  int32_t max_proposals;
  float    scales[TIDL_MAX_HEADS_TF_SSD + 1];
  float    aspect_ratios[TIDL_MAX_AR_PER_HEAD];
  float    height_stride;/*Anchor stride in height dimension in pixels */
  float    width_stride;/*Anchor stride in width dimension in pixels.*/
  float    height_offset;/*Anchor height offset in pixels.*/
  float    width_offset;/*Anchor width offset in pixels */
  float    width;/*Anchor width in pixels */
  float    height;/*Anchor height in pixels */
  float    nms_score_threshold;
  float    nms_iou_threshold;
} TIDL_TFFasterRCNNFirstStageConfig_t;

typedef struct {
  int32_t max_total_detections;
  int32_t max_detections_per_class;
  float    nms_score_threshold;
  float    nms_iou_threshold;
} TIDL_TFFasterRCNNSecondStageConfig_t;


typedef struct {
  int32_t  number_of_stages;
  int32_t  num_classes;
  int32_t initial_crop_size;
  int32_t maxpool_kernel_size;
  int32_t maxpool_stride;
  TIDL_TFFasterRCNNFirstStageConfig_t firstStageConfig;
  TIDL_TFFasterRCNNSecondStageConfig_t secondStageConfig;
} TIDL_TFFasterRCNNConfig_t;

typedef enum {
  ADD_LAYER_BEFORE = 0,
  ADD_LAYER_AFTER
} TIDL_addLayer_opType_t;

typedef enum {
  MAKE_LAYERID_DATAID_SAME = 1,
}TIDL_dataIdLayerIdConstraint_t;

typedef enum {
    BASE_OP_SUB = 0,
    BASE_OP_ADD,
    BASE_OP_DIV,
    BASE_OP_MUL
} TIDL_base_operator_t;

typedef struct TensorDescriptor
{
  double scale;
  int32_t zp;
  int32_t element_type;
  int32_t dimValues[TIDL_DIM_MAX];
  char   *name;
} TensorDescriptor_t;

/**
 * @brief Object detection parameters structure
 *
 * This structure contains configuration parameters for object detection models
 * that are used to determine output tensor dimensions and post-processing behavior.
 * These parameters are read from meta architecture files and used
 * to derive output data layer shapes for object detection models in TIDL import
 * based on meta architecture/framework types (in function TIDL_getOdOutNodeInfo)
 */
typedef struct TIDL_odParams
{
  /** Object detection code type - specifies the detection algorithm variant
   *  (e.g., 11 for FastBEV in 3DOD models) */
  int32_t codeType;

  /** Sub-code type for specialized detection tasks
   *  (e.g., TIDL_ObjectPose for 6D pose estimation) */
  int32_t subCodeType;

  /** Maximum number of top detections to keep after NMS (Non-Maximum Suppression)
   *  This determines the output tensor height/size for detection results */
  int32_t keepTopK;

  /** Number of keypoints per detected object
   *  Used in keypoint detection models (e.g., pose estimation, facial landmarks) */
  int32_t num_keypoint;

  /** Flag indicating whether keypoint confidence scores are included
   *  0 = no confidence (2 params per keypoint: x, y)
   *  1 = with confidence (3 params per keypoint: x, y, confidence) */
  int32_t keypoint_confidence;

  /** Name of the object detection framework
   *  (e.g., "MMDetection", "YOLO", "SSD", "BEVFormer") */
  string tidl_OdFramework;
} TIDL_odParams_t;

/* Per-stage shape change for a single layer */
struct sTIDL_ShapeFoldChange {
  std::string                       stageName;
  std::vector<std::vector<int32_t>> before; /* [inBufIdx][0..TIDL_DIM_MAX-1] */
  std::vector<std::vector<int32_t>> after;  /* [inBufIdx][0..TIDL_DIM_MAX-1] */
};

/* Accumulates all stage changes for a single layer, keyed by shapeFoldUId */
struct sTIDL_ShapeFoldTracker {
  int32_t                            shapeFoldUId;
  std::vector<sTIDL_ShapeFoldChange> change;
};

/**
 * A map to tabulate the minimum dimensions required for functionallity
 * for each layer. This is used for batch inference while doing shape
 * inference on the TIDL net structure
*/
/* input shapes can be re-organized for batch interpretation
with no change in functionality */
#define LAYER_NUMDIM_INVARIANT    (-1)
/* transition/change happens from input to output shapes and
should not be touched for batch interpretation */
#define LAYER_NUMDIM_TRANSITION   (-2)
/* layers with strict no batch dimension manipulation */
#define LAYER_NUMDIM_RESTRICTED   (-99)

static std::unordered_map <int32_t, int32_t> TIDL_minFunctionalDimensionForBatchHandling = {
  /* input */
  {TIDL_DataLayer,              LAYER_NUMDIM_INVARIANT},
  {TIDL_ConstDataLayer,         LAYER_NUMDIM_INVARIANT},
  /* conv like */
  {TIDL_ConvolutionLayer,       3},
  {TIDL_Deconv2DLayer,          3},
  {TIDL_ColorConversionLayer,   3},
  {TIDL_DepthToSpaceLayer,      3},
  {TIDL_SpaceToDepthLayer,      3},
  {TIDL_GridSampleLayer,        3},
  /* with filters */
  {TIDL_PoolingLayer,           2},
  {TIDL_ResizeLayer,            2},
  {TIDL_RoiPoolingLayer,        2},
  {TIDL_ReduceLayer,            2},
  /* invariant */
  {TIDL_EltWiseLayer,           LAYER_NUMDIM_INVARIANT},
  {TIDL_InnerProductLayer,      LAYER_NUMDIM_INVARIANT},
  {TIDL_SoftMaxLayer,           LAYER_NUMDIM_INVARIANT},
  {TIDL_BatchNormLayer,         LAYER_NUMDIM_INVARIANT},
  {TIDL_BiasLayer,              LAYER_NUMDIM_INVARIANT},
  {TIDL_ScaleLayer,             LAYER_NUMDIM_INVARIANT},
  {TIDL_CropLayer,              LAYER_NUMDIM_INVARIANT},
  {TIDL_DropOutLayer,           LAYER_NUMDIM_INVARIANT},
  {TIDL_ArgOpLayer,            LAYER_NUMDIM_INVARIANT},
  {TIDL_DetectionOutputLayer,   LAYER_NUMDIM_INVARIANT},
  {TIDL_ShuffleChannelLayer,    LAYER_NUMDIM_INVARIANT},
  {TIDL_OdPostProcessingLayer,  LAYER_NUMDIM_INVARIANT},
  {TIDL_PadLayer,               LAYER_NUMDIM_INVARIANT},
  {TIDL_OdOutputReformatLayer,  LAYER_NUMDIM_INVARIANT},
  {TIDL_DataConvertLayer,       LAYER_NUMDIM_INVARIANT},
  {TIDL_CustomLayer,            LAYER_NUMDIM_INVARIANT},
  {TIDL_ScatterElementsLayer,   LAYER_NUMDIM_INVARIANT},
  {TIDL_GatherLayer,            LAYER_NUMDIM_INVARIANT},
  {TIDL_LayerNormLayer,         LAYER_NUMDIM_INVARIANT},
  {TIDL_DataConvertLayer,       LAYER_NUMDIM_INVARIANT},
  {TIDL_GatherElementsLayer,    LAYER_NUMDIM_INVARIANT},
  /* activations */
  {TIDL_SigmoidLayer,           LAYER_NUMDIM_INVARIANT},
  {TIDL_ReLULayer,              LAYER_NUMDIM_INVARIANT},
  {TIDL_PReLULayer,             LAYER_NUMDIM_INVARIANT},
  {TIDL_TanhLayer,              LAYER_NUMDIM_INVARIANT},
  {TIDL_HardSigmoidLayer,       LAYER_NUMDIM_INVARIANT},
  {TIDL_ELULayer,               LAYER_NUMDIM_INVARIANT},
  {TIDL_SoftPlusLayer,          LAYER_NUMDIM_INVARIANT},
  {TIDL_SoftSignLayer,          LAYER_NUMDIM_INVARIANT},
  /* transition of shape */
  {TIDL_SqueezeLayer,           LAYER_NUMDIM_TRANSITION},
  {TIDL_FlattenLayer,           LAYER_NUMDIM_TRANSITION},
  {TIDL_ConcatLayer,            LAYER_NUMDIM_TRANSITION},
  {TIDL_SplitLayer,             LAYER_NUMDIM_TRANSITION},
  {TIDL_SliceLayer,             LAYER_NUMDIM_TRANSITION},
  /* restricted */
  {TIDL_ReshapeLayer,           LAYER_NUMDIM_RESTRICTED},
  {TIDL_TransposeLayer,         LAYER_NUMDIM_RESTRICTED},
  {TIDL_BatchReshapeLayer,      LAYER_NUMDIM_RESTRICTED},
  {TIDL_UnsupportedLayer,       LAYER_NUMDIM_RESTRICTED},
};

static std::unordered_map <int32_t, int32_t> TIDL_minFunctionalDimensionForShapeFolding = {
  {TIDL_ConvolutionLayer,       3},
  {TIDL_Deconv2DLayer,          3},
  {TIDL_ArgOpLayer,             3},
  {TIDL_ColorConversionLayer,   3},
  {TIDL_GridSampleLayer,        3},
  /******/
  {TIDL_PoolingLayer,           2},
  {TIDL_ResizeLayer,            2},
  {TIDL_ReduceLayer,            2},
  {TIDL_CropLayer,              2},
  {TIDL_LayerNormLayer,         2},
  {TIDL_GatherLayer,            2},
  /******/
  {TIDL_InnerProductLayer,      1},
  {TIDL_SplitLayer,             1},
  {TIDL_SliceLayer,             1},
  /******/
  {TIDL_SoftMaxLayer,           0},
  {TIDL_EltWiseLayer,           0},
  {TIDL_BatchNormLayer,         0},
  {TIDL_DataConvertLayer,       0},
  {TIDL_SigmoidLayer,           0},
  {TIDL_ReLULayer,              0},
  {TIDL_TanhLayer,              0},
  {TIDL_HardSigmoidLayer,       0},
  {TIDL_ELULayer,               0},
  {TIDL_ConstDataLayer,         0},
  {TIDL_ConcatLayer,            0},
  {TIDL_ClipLayer,              0},
  {TIDL_MinimumLayer,           0},
  {TIDL_LeakyReluLayer,         0},
  {TIDL_SqrtLayer,              0},
  {TIDL_PowLayer,               0},
  {TIDL_AsinLayer,              0},
  {TIDL_AsinhLayer,             0},
  {TIDL_HardSwishLayer,         0},
  {TIDL_MishLayer,              0},
  {TIDL_LogLayer,               0},
  {TIDL_LogLayer,               0},
  {TIDL_AbsLayer,               0},
  {TIDL_FloorLayer,             0},
  {TIDL_ExpLayer,               0},
  {TIDL_SinLayer,               0},
  {TIDL_ErfLayer,               0},
  {TIDL_TransposeLayer,         0},
  {TIDL_ReshapeLayer,           0},
  {TIDL_SwishLayer,             0},
  {TIDL_SoftPlusLayer,          0},
  {TIDL_SoftSignLayer,          0},
  {TIDL_CeilLayer,              0},
  {TIDL_CeluLayer,              0},
  {TIDL_SeluLayer,              0},
  {TIDL_RoundLayer,             0},
  {TIDL_SignLayer,              0},
};

/*
 * These layers do not support non-singleton Dim1 and Dim2, hence
 * forcing these dimensions to batch
 * Remove from here if we add support for these in future
 */
static std::vector <int32_t> TIDL_forceToBatch = {
  TIDL_ConvolutionLayer,
  TIDL_Deconv2DLayer,
  TIDL_PoolingLayer,
  TIDL_ResizeLayer,
  TIDL_RoiPoolingLayer,
  TIDL_DeformableConvLayer
};

int32_t TIDL_updateOutElementTypeUnSupported(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeUnSigned(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeSinglePrecFloat(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeSigned(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeActType(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeCopyInDataElementType(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeConcat(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeGridSampleLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeSliceLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeEltWise(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeDataLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeDataConvertLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeConstDataLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateOutElementTypeSoftMaxLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
void tidl_initUpdateOutElementTypeMap(std::unordered_map<int32_t, std::function<int32_t(sTIDL_OrgNetwork_t*, int32_t)>> & sTIDL_updateOutElementTypeMap);
int32_t tidl_updateElementTypesWithNativeTypes(sTIDL_OrgNetwork_t &orgTIDLNetStructure);

int32_t TIDL_getOdOutNodeInfo(int32_t metaArchType, TIDL_odParams *odParams, int32_t* numOutNodes, TensorDescriptor_t **output_tensors);
int32_t tidl_configureTransposeLayer (sTIDL_LayerPC_t *layer, int32_t *permute);
int32_t tidl_getIncrementForTopKOutputBuffer (sTIDL_LayerPC_t& layer);
int32_t tidl_getAxisForTopKOutputBuffer (sTIDL_LayerPC_t& layer);
int32_t TIDL_tfOutReshapeDataLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeConvLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapePoolingLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeIdentity(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeTopKLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeTileLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeShapeLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSizeLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeEltwise(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeBN(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeLogicalOpLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeRelu(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapePRelu(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSigmoid(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeTanh(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeHardSigmoid(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeELU(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSoftmax(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeIPLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeDeConvLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeConcatLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSliceLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeCropLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeFlattenLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeUnsqueezeLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeArgOpLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapePadLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeDetOutLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeRoiPoolingLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeOdPostProcessingLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_optimizeSqueezeLayers(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_optimizeTransposeLayers(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_optimizeGridSampleLayers (sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_optimizeGlobalAveragePoolingLayers (sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_optimizeDCNv2Block(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t tidl_addTransposeForDCN(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t tidl_addTransposeForGridSample(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t tidl_fuseSoftMaxDataConvert(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, tidl_import_config * params, int32_t numLayers);
int32_t tidl_optimizeSoftmaxLayer(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, tidl_import_config * params, int32_t numLayers);
int32_t tidl_optimizeSoftmaxAxis(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, tidl_import_config * params, int32_t numLayers);
int32_t TIDL_tfOutReshapeOdOutputReformatLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeReduceLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeScatterElementsLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeLSTMLayer(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeGRULayer(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeRNNLayer(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeGatherNDLayer(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeGatherElementsLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeNonZeroLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_AddTfODOutputLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex, int32_t metaArchTtype);
int32_t tidl_AddOnnxODOutputLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex);
int32_t TIDL_tfOutReshapeReshapeLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeGridSampleLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeCastLayer (sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeDeformConvLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeGatherLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_supportUnitConvWithPad(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataId);
int32_t tidl_sortLayersInProcOrder(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, sTIDL_OrgNetwork_t  *ptempTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeConstDataLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeTransposeLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSqueeze(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeAttention(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_checkIfShapeUpdatedForLayer (sTIDL_OrgNetwork_t &orgTIDLNetStructure, sTIDL_LayerPC_t& layer);

int32_t TIDL_getTfOpParamMapId(const char  * name);
int32_t tidl_optimizeDeConvLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t* dataIndex);
int32_t tidl_isLayerInputConst (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, sTIDL_LayerPC_t* layer, int32_t inBufIdx);
int32_t tidl_linkInputTensors(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_linkOutputTensors(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_isAllInsAvailable(sTIDL_LayerPC_t  *orgLayer, sTIDL_OrgNetwork_t  *ptempTIDLNetStructure, int32_t layerIndex);
int32_t tidl_sortLayersInProcOrder(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, sTIDL_OrgNetwork_t  *ptempTIDLNetStructure, int32_t layerIndex);
int32_t tidl_removeMergedLayersFromNet(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, sTIDL_OrgNetwork_t  *ptempTIDLNetStructure);
int32_t tidl_upateAInDataId(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t layerIndex, int32_t oldId, int32_t currId);
int32_t tidl_sortDataIds(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure);
int32_t tidl_makeDataIdLayerIdSame(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_updateConstDataLayerRange(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_updateOutDataShape(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t startIdx, int32_t layerIndex, sTIDL_tfOutReshapeMap_t * sTIDL_tfOutReshapeTable);
int32_t tidl_getInLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId);
int32_t tidl_getOutLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId);
int32_t tidl_getOutLayer(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId);
int32_t tidl_getInLayer(sTIDL_Network_t  *pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId);
int32_t tidl_getOutLayer(sTIDL_Network_t  *pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId);
int32_t tidl_mergeFalttenLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeBiasLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_FindWrapAroundPadLayer (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_mergeColumnPadIntoConv (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_mergePadLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeBNLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeYuv444InConvLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t inDataFormat);
int32_t tidl_mergeReluLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeGeluLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeSiluLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeLogitLayer (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_convertVariableDivToBN (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_convertVariableSubToAddWithNeg (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_mergeLayerNormLayer (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_splitNormLayer(sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers, bool isInstanceNorm=false);
int32_t tidl_convertInstanceNormalizationToLayerNorm(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t layerIndex);
int32_t tidl_convertConv2DToIpLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, sTIDL_tfOutReshapeMap_t * sTIDL_tfOutReshapeTable,int32_t* dataIndex);
int32_t tidl_copyPCNetToDeviceNet(sTIDL_OrgNetwork_t  * pOrgTIDLNetStructure,
                                                                  sTIDL_Network_t  * pTIDLNetStructure,
                                                                  tidl_import_config * configParams,
                                                                  int32_t numLayers);
int32_t tidl_addOutDataLayer(sTIDL_Network_t  * tIDLNetStructure, int32_t tiLayerIndex);
int32_t tidl_splitOutDataLayer(sTIDL_OrgNetwork_t &orgTIDLNetStructure,
                                 int32_t &layerIndex);
void tidl_updateWeightElemSize(sTIDL_OrgNetwork_t  * pOrgTIDLNetStructure, tidl_import_config * params, int32_t numLayers);
void tidl_convertElementTypeGivenParambits(sTIDL_OrgNetwork_t  * pOrgTIDLNetStructure, int32_t numLayers, int32_t numParamBits);
int32_t tidl_fillInDataLayerShape(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, tidl_import_config * params, int32_t tiLayerIndex);
void TIDL_convertDeconv2DtoConv(sTIDL_OrgNetwork_t   &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_addInDataLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex);
int32_t tidl_addConstDataLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex, tidl_import_config *params);
int32_t tidl_createConstDataLayer (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, float* data, int32_t dataSize, int32_t layerIndex);
int32_t tidl_doesLayerSupportSpatialSplit(sTIDL_OrgNetwork_t& pOrgTIDLNetStructure, int32_t lyrIdx, int32_t numCores);
void tidl_updateScratchMemoryRequirement(sTIDL_OrgNetwork_t& pOrgTIDLNetStructure);
void tidl_updateMultiCoreState(sTIDL_OrgNetwork_t& pOrgTIDLNetStructure, tidl_import_config * params);
int32_t tidl_doesNetworkHasMatmulLayer(sTIDL_OrgNetwork_t& pOrgTIDLNetStructure);
int32_t tidl_addCropLayerForMultiCore(sTIDL_OrgNetwork_t& pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex, int32_t quantizationStyle, int32_t numCores);
template <class Tin>
int32_t tidl_convertNHWCtoNCHW(Tin* dst, Tin* src, int32_t n, int32_t c, int32_t h, int32_t w);
int32_t tidl_mergeReshapeLayers (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure);
int32_t tidl_mergeIdentitytLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_handleDuplicateDQLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeQDQLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, bool mergeQuant);
int32_t tidl_mergeInitializerDQLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_merge1x1MaxPoolingLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergePoolingLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeBatchToSpaceInDWConvLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_optimizePatchMergingBlock (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t& layerIndex);
int32_t tidl_fusePatchMergingBlock (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t& layerIndex);
int32_t tidl_recheckSliceStrideSupport (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t& layerIndex);
int32_t tidl_reduceSliceDims(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t& layerIndex);
int32_t tidl_duplicateSliceLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex);
int32_t tidl_mergeDetectionoutLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_removeConcatFlattenLayerInDetNet(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_removeConcatReshapeLogisticLayerInDetNet(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_removeDivideByOneLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeSplitLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeNoOpReshapeLayers (sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t startIdx, int32_t numLayers);
int32_t tidl_mergeConsecutiveSliceLayers (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_addPadLayerBeforeConv(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_addPadLayerBeforeConvForAsymmetricPadding(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);

int32_t TIDL_tfOutReshapeResize(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_addNormLayerToInData(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex, tidl_import_config * params);
int32_t tidl_addPadLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex, tidl_import_config * params);
int32_t TIDL_addLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIdx, int32_t newLayerType, int32_t* dataIndex, int32_t opType, std::vector<int32_t> nodeIndices = {});
int32_t TIDL_wrapWithReshape(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIdx, int32_t* dataIndex, int32_t* newDimVals, std::vector<int32_t> inpNodeIndices = {});
int32_t TIDL_wrapWithReshapeTranspose(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIdx, int32_t* dataIndex, int32_t* newDimVals, int32_t* perms, std::vector<int32_t> inpNodeIndices = {});
int32_t tidl_convertIpLayerInputShape(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_convertSqueezeToFlattenLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_convertPoolToIdentityLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_convertSoftMaxLayerInputShape(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tf_getLayreTypeMapIdx(char* layerName, TIDL_TFLayerMapping_t* TIDL_TFLayerMap, int32_t tblSize);
int32_t tf_isLayerType(char* layerName, int32_t  startLayer, sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, TIDL_TFLayerMapping_t* TIDL_TFLayerMap, int32_t tblSize);
int32_t tidl_convertRelUToBNLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_getStringsFromList(char *list, char * names, int strLen);
int32_t tidl_mergePreBNLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_getDimPadLayerWithStrideOffset(int32_t * padT, int32_t * padB, int32_t * padL, int32_t * padR,
                       const sTIDL_ConvParams_t * convParams,
                       int32_t convOutHeight, int32_t convOutWidth,
                       int32_t convInHeight, int32_t convInWidth, int32_t * strideOffsetMethod);
int32_t tidl_mergeFlattenLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_getElementType(int32_t sign);
int32_t tidl_getElementSizeInBits(int32_t elementType);
int32_t TIDL_doesLayerSupportMixedPrecision(sTIDL_LayerPC_t * layer);
int32_t TIDL_getDatElementSign(int32_t elementType);
const char* TIDL_getOutDataName(const sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t dataId);
const sTIDL_DataParams_t * TIDL_getOutDataInNet(const sTIDL_Network_t * tidlNet,
                                     int32_t dataId);
const sTIDL_DataParams_t *TIDL_getOutData(const sTIDL_OrgNetwork_t *pOrgTIDLNetStructure,
                                    int32_t dataId);
void getAbsPath(char *path, char *absPath);
bool TIDL_doesLayerSupportAsymTensors(sTIDL_LayerPC_t* layerType, sTIDL_OrgNetwork_t *pOrgTIDLNetStructure = NULL);
int32_t getNumberOfLinesIntheFile(char *fileName);
int32_t updatePadAndWriteModel(sTIDL_OrgNetwork_t *pTidlOrigNet,
                               sTIDL_Network_t *pTidlNet,
                               tidl_import_config *configParams);
void TIDL_copyTensorStats(sTIDL_OrgNetwork_t *pOrgTIDLNetStructureDst,
                          sTIDL_OrgNetwork_t *pOrgTIDLNetStructureSrc,
                          int32_t skip_input_datalayers);
int tidlRunQuantStatsTool(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure,
                          sTIDL_Network_t *pTIDLNetStructure,
                          tidl_import_config *params,
                          uint32_t numLayers);
void getDirFromPath(char *path);
char *getFileNameFromPath(char *path);
int32_t tidl_addMetaArchLayersToNet(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t *dataIndex, tidl_import_config *params, int32_t detLayerId);
int32_t tidl_addInDataLayerForUVData(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                     int32_t layerIndex,
                                     int32_t *dataIndex,
                                     tidl_import_config *params,
                                     int32_t *inputList,
                                     int32_t numInLayers);
int32_t tidl_addUVConversionLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                  int32_t layerIndex,
                                  int32_t *dataIndex,
                                  int32_t *uvLayerList);
int32_t tidl_addConcatLayerForYUVMerge(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                       int32_t layerIndex,
                                       int32_t *dataIndex,
                                       int32_t *inputList,
                                       int32_t (*outForInputList)[TIDL_NUM_OUT_BUFS],
                                       int32_t *uvLayerList,
                                       int32_t *newConcatLayerList,
                                       int32_t *numOutLayers,
                                       int32_t numInLayers);
int32_t tidl_addNormLayerToConcat(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                  int32_t layerIndex,
                                  int32_t *dataIndex,
                                  tidl_import_config *params,
                                  int32_t *concatLayerList,
                                  int32_t numConcatLayers);
int32_t tidl_addConvLayerAfterConcat(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                     int32_t layerIndex,
                                     int32_t *dataIndex,
                                     tidl_import_config *params,
                                     int32_t *concatLayerList,
                                     int32_t *newConvList,
                                     int32_t numConcatLayers);
int32_t tidl_fillRgbToYuvConvLayerList(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                       int32_t layerIndex,
                                       int32_t *dataIndex,
                                       tidl_import_config *params,
                                       int32_t *convLayerList);
int32_t tidl_addLayersForUVConversion(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t *dataIndex, tidl_import_config *params);
int32_t tidl_mergeYUVtoRGBConversion(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t *dataIndex, tidl_import_config *params);
int32_t tidl_addPadLayerAndConv1x1ForYUVtoRGB(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure,
                                              int32_t layerIndex,
                                              int32_t *dataIndex,
                                              tidl_import_config *params,
                                              int32_t *convLayerList,
                                              int32_t * padLayerList,
                                              int32_t * numYuvMergeConvLayers,
                                              int32_t * numYuvMergePadLayers);
void tidl_getInverse3X3Matrix(float32_tidl mat[3][3], float32_tidl matInv[3][3]);
int32_t TIDL_doesLayerSupportBatchStitching(sTIDL_LayerPC_t * layer);
int32_t tidl_addBatchReshapeLayerForBatchStitching(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, std::vector<int32_t> &currGroup, int32_t &dataIndex);
int32_t tidl_initAndUpdateBatchPadRequirements(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, std::vector<int32_t> &currGroup);

extern uint32_t TIDL_kernelReshape(float * param, uint32_t w, uint32_t h, uint32_t ci, uint32_t co);
extern uint32_t TIDL_kernelReshapeHWOI(float * param, uint32_t w, uint32_t h, uint32_t ci, uint32_t co);
extern uint32_t TIDL_kernelReshapeIOHW(float * param, uint32_t w, uint32_t h, uint32_t ci, uint32_t co);
extern int32_t tidl_FindFlattenLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
extern int32_t tidl_FindHardSigmoidLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
extern int32_t tidl_mergePackToNNResizeLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeClipLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_convert1x1s1DWSConvToBatchNorm (sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t numLayers);
int32_t TIDL_tfOutReshapeClip(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergeMinimumLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
void tidl_replaceInTensorName(sTIDL_OrgNetwork_t  *pOrgTIDLNetStructure, int32_t layerIndex, char * orgTensorName, char * newTensorName);
int32_t tidltb_isInDataBuff(const sTIDL_Network_t * pTIDLNetStructure, int32_t dataId, int32_t layersGroupId);
int32_t tidltb_isOutDataBuff(const sTIDL_Network_t *pTIDLNetStructure, int32_t dataId, int32_t layersGroupId);
void TIDL_caffeReorderDeconvFilters(sBuffer_t &buf, int32_t gno, int32_t gni, int32_t gr, int32_t k);
int32_t TIDL_tfOutReshapeDepthtoSpace(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSpacetoDepth(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeBatchtoSpace(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeSpacetoBatch(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeColorConversionLayer(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_tfOutReshapeBatchReshape(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);

#define TIDL_MAX_DATA_NAME (512)

extern char inDataNames[TIDL_MAX_ALG_IN_BUFS][TIDL_MAX_DATA_NAME];
extern char outDataNames[TIDL_MAX_ALG_OUT_BUFS][TIDL_MAX_DATA_NAME];
extern int32_t numNetInData;
extern int32_t numNetOutData;


extern sTIDL_tfOutReshapeMap_t sTIDL_OutReshapeTable[];
int32_t tidl_FindOnnxShuffleLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_FindMatMulBias(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_fuseLayerNormBetaGamma(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_identifyPatchEmbeddingBlock(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_FindFlattenLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_FindHardSigmoidLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_mergePackToNNResizeLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_FindOnnxShuffleLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);
int32_t tidl_splitResizeLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t &layerIndex, int32_t& dataIndex);
int32_t tidl_merge1x1ResizeLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex);

extern void TIDL_tfBNToScaleBias(float* scale, float* bias, uint32_t numCh, float* mean, float* var, float* gamma, float* beta, float eps);
extern int32_t tidl_convertBiasToBNLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
extern int32_t tidl_convertReshapeToFlatten(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);

int32_t tidl_handleTopKAxis (sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_handleTopKLayers (sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_handleRecurrentLayers (sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t numLayers, int32_t* dataIndex);
int32_t tidl_addDataConvertForScatterLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t* dataIndex);
int32_t tidl_addDataConvertLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t *dataIndex, tidl_import_config *params);
int32_t TIDL_handlePadforTransposeLayers (sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t TIDL_handleTransposeLayers(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t TIDL_convertTransposetoReshape(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers);
int32_t TIDL_find_index(int32_t *perm, int32_t val, int32_t size);
int32_t TIDL_convertTransposeToDataConvert(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t TIDL_removeTransposeFromNet(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t numLayers);
int32_t tidl_addDataConvertLayerForOTFsupport(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t *dataIndex, tidl_import_config *params, unordered_set<int32_t>& LayerList);
int32_t tidl_addCropLayerForValidPadSupport(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex, tidl_import_config * params);
int32_t TIDL_getConfigParamOutIndexFromLayerName(const char * layerName);
bool TIDL_checkIsDirectory(const char *path);
bool tidl_findReshapeinNetwork(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_sortDataIDsinPCNet(sTIDL_OrgNetwork_t*  orgTIDLNetStructure, int32_t makeDataIdLayerIdSame = 0);
int32_t tidl_mergeReshapeLayers(sTIDL_OrgNetwork_t& orgTIDLNetStructure);
void tidl_checkPadForDCLayers(sTIDL_Network_t  *tidlNet, sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t reshapeIdx, int32_t* removeDC1, int32_t* removeDC2);
int32_t TIDL_getCompilerDataSize(const sPerfSim_t * perfSimInfo, int32_t numLayers);
int32_t TIDL_getNetworkStructDataSize(const sTIDL_Network_t *pNet);
void tidl_poolPadParams(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex);

sBufParams_t * TIDL_getInBufParams(const sTIDL_Network_t *net,int32_t prevDataId, int32_t curdataId, int32_t currLayersGroupId);
sBufParams_t * TIDL_getOutBufParams(const sTIDL_Network_t *net, int32_t dataId, int32_t currLayersGroupId);
int TIDL_importBitDepthProtoTxt(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure,
                                        tidl_import_config *configParams);
int32_t  tidl_InnerProductBTranspose (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);

int32_t  tidl_convertFlattenToReshape (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
int32_t  tidl_convertUnsqueezeToReshape(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_removeTranposeBetweenDequantizeAndMatMul(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);

int32_t tidl_optimizePatchEmbedding(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t* dataIndex);
int32_t tidl_optimizeBatchNormLayers (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t factorDimensions(int32_t& d1, int32_t& d2, int32_t totDim);
int32_t tidl_fuseTransposeMatMul(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t* dataIndex);
int32_t tidl_fuseMSDAMulMatMul(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t* dataIndex);
int32_t tidl_doesConstNeedBroadcastSupport (sTIDL_LayerPC_t& SrcLayer, sTIDL_LayerPC_t& SrcConstLayer);
int32_t tidl_doesConstSatisfyBroadcastConditions (sTIDL_allowlistingMetaData& layerMetaData);
int32_t tidl_checkTensorHigherDimsOnes (vector<int32_t>& Tensor, int32_t boundaryDim);
int32_t tidl_checkTensorLowerDimsNotOnes (vector<int32_t>& Tensor, int32_t boundaryDim);
std::vector<int32_t> tidl_getOutLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t dataId); 
std::vector<int32_t> tidl_getInLayers_v2(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers, int32_t layerIdx); 
int32_t tidl_fuseTransposeLayers (sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t numLayers);
int32_t tidl_convertEltwiseToBNLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers, int32_t *dataIndex);
int32_t tidl_removeNodeAndLinkIONodes (sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t layerIdx);
int32_t tidl_checkIfShapesSame (int32_t* shapeOne, int32_t* shapeTwo);
int32_t tidl_transposeWeights (float* src_ptr, float* dst_ptr, int32_t channels, int32_t height, int32_t width);
int32_t tidl_transposeDataBuffer (float* src_ptr, float* dst_ptr, int32_t* dimValues);
int32_t tidl_containsSuffix(std::string str, std::string suffix);
int32_t tidl_addReshapeBeforeIP(sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);

#define TIDL_VERSION_SPLITS (4U)
std::vector<std::string> TIDL_splitString(std::string str, std::string delimiter);
bool TIDL_isSupportedInFirmwareVersion(const char *providedVersion, const char *queryVersion);
int32_t tidl_batchProcessing(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t &dataIndex);
int32_t tidl_detectBatchSupportedGroups (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, std::vector<std::pair<std::pair<int32_t, int32_t>, std::vector<int32_t>>> &batchSupportedBlocks, std::vector<std::vector<std::vector<int32_t>>> &batchGroupBoundaries, uint8_t updateShapeAfterDetection);
int32_t tidl_addReshapeAtBatchGroupBoundary(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, std::vector<std::pair<std::pair<int32_t, int32_t>, std::vector<int32_t>>> &batchSupportedBlocks, std::vector<std::vector<std::vector<int32_t>>> &batchGroupBoundaries, int32_t *dataIndex, sTIDL_tfOutReshapeMap_t * sTIDL_tfOutReshapeTable);
int32_t tidl_removeSingletonDimensions(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t layerIndex, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_mergeContiguousBroadcast(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t layerIndex, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_mergeHigherDimsToChannel(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t layerIndex, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_mergeHigherDimsInSpecialCase(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t layerIndex, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_mergeHigherDims(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t startIndex, int32_t stopIndex, int32_t stopDim, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_explodeWidthForLFMP(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t layerIndex, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_forcePushToBatch(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_pushToBatch(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers, std::map<int32_t, sTIDL_ShapeFoldTracker>& shapeFoldTrackers, const std::string& stageName, bool &stopShapeFolding);
int32_t tidl_mergeDuplicateOutputs(sTIDL_OrgNetwork_t &orgTIDLNetStructure);
int32_t getUserPrecisionFromDataId(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                   tidl_import_config * params, int32_t dataId);
int32_t tidl_convertGatherToLineGather(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers, int32_t* dataIndex);
int32_t tidl_convertGatherNDToLineGatherND(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers, int32_t* dataIndex);
int32_t tidl_replaceGatherWithReshape(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_convertGatherToSlice(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_convertGatherElementsAxisToWidthwiseAxis(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers, int32_t* dataIndex);
int32_t tidl_addDataConvertForGather(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_propagateCalibrationParams(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t * dataIndex);
int32_t tidl_removeInitializerQDQLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t *dataIndex, int32_t numLayers);
int32_t tidl_supportMultiChannelMatMulBias(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_supportMultiChannelMatMul(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_supportMultiChannelEltWise(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_satisfyHighThroughputBatchRequirement(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_changeOutElementType(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_optimizeDepthToSpaceLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers, int32_t *dataIndex);
int32_t tidl_convertReduceMeanReduceSumToMatMulLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t tidl_handleNegativePadValuesForPadLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t numLayers, int32_t* dataIndex);
int32_t tidl_addDataConvertLayerForMPConstraining(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t *dataIndex, tidl_import_config *params);
int32_t tidl_addDataConvertLayerForMPConstrainingV2(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t *dataIndex, tidl_import_config *params);
int32_t tidl_convertExpandToEltwiseSumLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t tidl_removeNoOpTileLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t addReshapeAcrossBatchNormLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t TIDL_zeroPadKernelToIncreaseSize(sBuffer_t &buf, int32_t orgKernelWidth, int32_t orgKernelHeight, int32_t newKernelWidth, int32_t newKernelHeight, bool isUpperPadded);
int32_t tidl_mapGemmMatMulToIP(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t* dataIndex, int32_t numLayers);
int32_t tidl_mergeSwishLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex);
int32_t tidl_addReshapeAcrossRMSNormLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& numLayers, int32_t* dataIndex);
int32_t tidl_convertGroupNormToInstanceNorm(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t* dataIndex);
int32_t tidl_removeNoOpConcatLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t& layerIndex, int32_t& dataIndex);
int32_t tidl_convertConcatAxisToChannel(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t* dataIndex);
int32_t tidl_mergeNoopCastLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers);
int32_t tidl_addCastLayersForINT64Outputs (sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
int32_t tidl_addCastLayersForINT64Inputs(sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex);
bool TIDL_isOutTensorFlat(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure, int32_t layerIndex);
bool TIDL_isOutputLayerType(int32_t layerType);
int32_t tidl_decidePTQLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure);
int32_t tidl_insertDataConvertsForNativeLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t* dataIndex);
bool tidl_isCalibrationInNativeTypes();
int32_t tidl_removeUnusedDCLayers(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure);
int32_t tidl_convertShapeAndSizeToConst(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t& layerIndex);
int32_t tidl_splitAttentionLayer(sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t& numLayers, int32_t* dataIndex);
int32_t TIDL_getNonPassThroughProducers(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t dataId, int32_t layerIdx, std::vector<int32_t> &producers);
int32_t tidl_mergeConvToConvLayer(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure);
int32_t tidl_fillGatherProp(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t numLayers);
#endif /*TIDL_IMPORT_COMMONH_ */
