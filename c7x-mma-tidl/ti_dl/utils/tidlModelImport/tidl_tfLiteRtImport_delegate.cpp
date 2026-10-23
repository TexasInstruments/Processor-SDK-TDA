#include <set>
#include <memory>
#include <vector>
#include <map>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <dlfcn.h>
#include <cmath>
#include <float.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <iostream>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/context_util.h"
#include "../tfliteImport/schema_generated.h"
#include "flatbuffers/flexbuffers.h"

#include "itidl_ti.h"
#include "itidl_rt.h"
#include "tidl_tfLiteRtImport.h"
#include "tidl_import_config.h"
#include "tidl_import_api.h"
#include "tidl_runtimes_import_common.h"
#include "tidl_runtimes_infer_common.h"
#include "tidl_tflrt_common.h"
#include "tidl_import_common_tidlrt_osrt.h"
#include "tidl_import_diag.h"

#undef __MODULE__
#define __MODULE__ "[PARSER]"

#define MAX_NUM_TIDL_SUBGRAPHS (16)
#define TENSOR_FLOAT (0)
#define TENSOR_UINT8 (1)
#define TENSOR_INT8 (2)
#define MAX_NUM_DETECTION_LAYER (4)
#define TIDL_MAX_TF_SSD_LAYERS  (MAX_NUM_DETECTION_LAYER)

extern uint8_t tidl_OdOutputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_STRING_SIZE];

namespace tflite {
namespace tfl_delegate {

class tidlDelegate;

/** This function has variables used across tflite RT code - interface options,
 * dynamic loading variables etc */
class TIDL_TfLiteDelegateData {
public:
    std::vector<tidlDelegate *> valid_subgraphs;
    std::vector<tidlDelegate *> invalid_subgraphs;
    std::vector<tidlDelegate *> pending_subgraphs;

    std::vector<std::string> odPostProcHeadNames;
    std::vector<std::string> odBackboneNodeNames;
    std::vector<std::vector<int>> supported_node_groups;
    std::vector<int> supported_nodes;

    sTIDL_tidlRtDynamicLoading_t infer_ops;

    TIDL_osrtOptions osrt_options;
};

/** Main Delegate class used for representing Tflite TIDL delegate */
class tidlDelegate {
public:
    // Define explicit constructor
    tidlDelegate();

    // Any initialization code needed
    TfLiteStatus Init(TfLiteContext* context, const TfLiteDelegateParams* params);

    // Any preparation work needed (e.g. allocate buffers)
    TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node);

    // Actual running of the delegate subgraph.
    TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node);

    //Destructor
    ~tidlDelegate();

    std::vector<int> nodes_;
    int32_t currNumInTensors_;
    float32_tidl *inTensorsQuantFactor_;
    float32_tidl * inTensorsMin_;
    float32_tidl * inTensorsMax_;
    int32_t currFrameIdx_;
    void *subGraphPtr_;
    void * gParams_;
    int32_t subgraphId_;
    TIDL_TfLiteDelegateData *data_;
    sTIDL_tidlRtParams_t tidlRtParamsObj;

};

tidlDelegate::tidlDelegate()
{
    currNumInTensors_ = 0;
    currFrameIdx_ = 0;
    subGraphPtr_ = NULL;
    gParams_ = NULL;
    inTensorsQuantFactor_ = new float32_tidl[TIDL_MAX_ALG_IN_BUFS];
    inTensorsMin_ = new float32_tidl[TIDL_MAX_ALG_IN_BUFS];
    inTensorsMax_ = new float32_tidl[TIDL_MAX_ALG_IN_BUFS];
    tidlRtParamsObj.ioBufDesc = NULL;
    tidlRtParamsObj.netPtr = NULL;
    tidlRtParamsObj.rtHandle = NULL;
    tidlRtParamsObj.rtInList = NULL;
    tidlRtParamsObj.rtOutList = NULL;
    tidlRtParamsObj.stats = NULL;
}

template <class Tin, class Tout>
static TfLiteStatus TIDL_tfliteRtDataFormatHwc2chw(Tout* dst, Tin* src, int32_t n, int32_t c, int32_t h, int32_t w, float src_scale, float dst_scale, int32_t zero)
{
    int32_t i0 , i1, i2, i3;
    float out;
    for (i0 = 0; i0 < n; i0++){
        for (i1 = 0; i1 < c; i1++)
        {
            for (i2 = 0; i2 < h; i2++)
            {
                for (i3 = 0; i3 < w; i3++)
                {
                    out = (((src[i0 * c * h * w + i1 + i2*w*c + i3*c] - zero)*src_scale)*(1/dst_scale));
                    dst[ i0 * c * h * w + i1 * h * w + i2 * w + i3] = (Tout)out;
                }
            }
        }
    }
    return kTfLiteOk;
}

/** Write inputs for each subgraph corresponding to all input frames in float mode in bin file
  * and also store corresponding quantFactors for quantization information */
static TfLiteStatus TIDL_writeQuantizedInput(TfLiteContext* context, TfLiteNode* node, char * inputName,
                                             int32_t *numpInputs, float ** inTensorsMin, float ** inTensorsMax, float ** quantScale, int32_t * tensorType,
                                             int32_t inferenceMode)
{
    FILE* fp = fopen(inputName, "ab");

    int32_t w[16];
    int32_t h[16];
    int32_t c[16];
    int32_t n[16];
    float * inputMin = *inTensorsMin;
    float * inputMax = *inTensorsMax;

    if (fp == NULL)
    {
        TIDL_GLOBAL_REPORT_ERROR("Could not open file to save the input tensors ");
        return kTfLiteError;
    }

    int32_t currInIdx = 0;
    int32_t numBatches = 1;

    for (auto tensor_index : TfLiteIntArrayView(node->inputs))
    {
        if (tensor_index == kTfLiteOptionalTensor)
        {
            continue;
        }
        TfLiteTensor* tensor = &context->tensors[tensor_index];
        if(tensor->allocation_type == kTfLiteArenaRw)
        {
            if(inferenceMode == TIDL_inferenceModeHighThroughput)
            {
                numBatches = 1;
            }
            else
            {
                numBatches = tensor->dims->data[0];
            }
            w[currInIdx] = tensor->dims->data[2];
            h[currInIdx] = tensor->dims->data[1];
            c[currInIdx] = tensor->dims->data[3];
            n[currInIdx] = numBatches;

            size_t tensorSize = w[currInIdx] * h[currInIdx] * c[currInIdx] * n[currInIdx];

            float * pInputData = (float *)malloc(tensorSize*(32/8));
            if(pInputData == NULL)
            {
                return kTfLiteError;
            }
            if(tensor->type == kTfLiteUInt8)
            {
                *tensorType = TENSOR_UINT8;
                TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)tensor->quantization.params;

                float scale = 1.0;
                TIDL_tfliteRtDataFormatHwc2chw(
                        pInputData, (uint8_t*)(tensor->data.uint8),
                        numBatches, tensor->dims->data[3], tensor->dims->data[1],
                        tensor->dims->data[2], quantization->scale->data[0], scale,
                        quantization->zero_point->data[0]);
                fwrite(pInputData, 1, tensorSize*(32/8), fp);
                scale = (float)ceil(log((double)quantization->scale->data[0]) / log((double)2));
                scale = pow(2.0, (double)scale);
                (*quantScale)[currInIdx] = 1 / scale;
            }
            else if(tensor->type == kTfLiteInt8)
            {
                *tensorType = TENSOR_INT8;
                TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)tensor->quantization.params;

                float scale = 1.0;
                TIDL_tfliteRtDataFormatHwc2chw(
                        pInputData, (int8_t*)(tensor->data.int8),
                        numBatches, tensor->dims->data[3], tensor->dims->data[1],
                        tensor->dims->data[2], quantization->scale->data[0], scale,
                        quantization->zero_point->data[0]);
                fwrite(pInputData, 1, tensorSize*(32/8), fp);
                scale = (float)ceil(log((double)quantization->scale->data[0]) / log((double)2));
                scale = pow(2.0, (double)scale);
                (*quantScale)[currInIdx] = 1 / scale;
            }
            else if(tensor->type == kTfLiteFloat32)
            {
                *tensorType = TENSOR_FLOAT;

                float min, max;
                TIDL_runtimesFindRange((float *)(tensor->data.f), numBatches, 1, 1, tensor->dims->data[3],
                        tensor->dims->data[1], tensor->dims->data[2], 1.0, 0, min, max);

                float scale = 1.0;
                TIDL_tfliteRtDataFormatHwc2chw(
                        (float *)pInputData, (float *)(tensor->data.f),
                        numBatches, tensor->dims->data[3], tensor->dims->data[1],
                        tensor->dims->data[2], 1.0, 1 / scale, 0);
                fwrite(pInputData, 1, tensorSize * (32 / 8), fp);

                inputMin[currInIdx] = (inputMin[currInIdx] < min) ? inputMin[currInIdx] : min;
                inputMax[currInIdx] = (inputMax[currInIdx] > max) ? inputMax[currInIdx] : max;
            }
            else
            {
                TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported Tensor->type %d", tensor->type);
                return kTfLiteError;
            }
            currInIdx++;
            free(pInputData);
        }
    }
    fclose(fp);
    *numpInputs = currInIdx;
    return kTfLiteOk;

}

/** Get input scales from models for quant models. These scales will be used when data convert layer is not present in network */
static TfLiteStatus TIDL_getInTensorScalesForQuantModels(TfLiteContext* context, TfLiteNode* node, float ** quantScale, int32_t ** zp)
{
    int32_t currInIdx = 0;

    for (auto tensor_index : TfLiteIntArrayView(node->inputs))
    {
        if (tensor_index == kTfLiteOptionalTensor)
        {
            continue;
        }
        TfLiteTensor* tensor = &context->tensors[tensor_index];
        if(tensor->allocation_type == kTfLiteArenaRw)
        {
            if((tensor->type == kTfLiteUInt8) || (tensor->type == kTfLiteInt8))
            {
                TfLiteAffineQuantization* quantization = (TfLiteAffineQuantization*)tensor->quantization.params;
                (*quantScale)[currInIdx] = quantization->scale->data[0];
                (*zp)[currInIdx] = (int32_t)quantization->zero_point->data[0];
            }
            else if(tensor->type == kTfLiteFloat32)
            {
                (*quantScale)[currInIdx] = 1.0;
                (*zp)[currInIdx] = (int32_t)0;
            }
            else
            {
                /* TODO : Add support for float input correctly when we support "quantize" layer */
                TIDL_GLOBAL_REPORT_UNSUPPORTED("Input type %d not supported for quant models ", tensor->type);
                return kTfLiteError;
            }
            currInIdx++;
        }
    }
    return kTfLiteOk;
}
/** First save the inputs to the subgraph corresponding to "m_calibration_frames" number of frames.
  * For the last frame, given all the inputs are now available, run postProcessNet, which runs calibration on the subgraph
  * After the final calibrated and quantized subgraph is saved in net.bin and io.bin files, copy these files from
  * tempDir to the artifacts folder for inference
  */
static bool TIDL_subgraphImport(TfLiteContext* context, TfLiteNode* node, tidlDelegate *subGraphDelegate,
        int32_t * currNumInTensors, int32_t currFrameIdx, float ** inQuantFactorCalibTensors, float ** inTensorMin, float ** inTensorMax)
{
    bool res = false;
    float * inQuantFactorAllTensors = *inQuantFactorCalibTensors;
    TIDL_TfLiteDelegateData *data = subGraphDelegate->data_;
    int32_t tensorType = TENSOR_FLOAT;

    if(data->osrt_options.m_quantization_scale_type == TIDL_QuantStyleAsymNP2_TFL)
    {
        if(currFrameIdx == 1)
        {
            /** This quantization style does not require saving subgraph inputs for calibration
             * So only do scale collection and proceed to import backend part
             */
            int32_t *zp = (int32_t *)malloc(TIDL_MAX_ALG_IN_BUFS * sizeof(int32_t));
            if(zp == NULL)
            {
                TIDL_GLOBAL_REPORT_ERROR("Can't allocate memory : Input zero points set to default value of 0 ");
                return false;
            }
            else
            {
                memset(zp, 0, TIDL_MAX_ALG_IN_BUFS * sizeof(int32_t));
            }
            char subgraphName[10];
            sprintf(subgraphName, "%d", subGraphDelegate->subgraphId_);
            TIDL_getInTensorScalesForQuantModels(context, node, &inQuantFactorAllTensors, &zp);
            int32_t rtStatus = TIDL_runtimesPostProcessNet(&data->osrt_options, data->osrt_options.m_calibration_frames, data->osrt_options.m_num_param_bits,
                                                subGraphDelegate->subGraphPtr_, subGraphDelegate->gParams_, inQuantFactorAllTensors, zp, (const char*)subgraphName);
            if(rtStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                return false;
            }
            if(zp !=NULL) free(zp);
            res = true;
        }
    }
    else
    {
        if(currFrameIdx <= data->osrt_options.m_calibration_frames) //need to copy input of subgraphs only before calibration is done
        {
            std::string inputName;
            inputName = data->osrt_options.m_temp_folder + "/" + std::to_string(subGraphDelegate->subgraphId_) +  "_calib_raw_data.bin";

            int32_t numParamBits = data->osrt_options.m_num_param_bits;
            int32_t inferenceMode = data->osrt_options.m_inference_mode;
            int32_t numInputTensors = 0;

            TfLiteStatus status = TIDL_writeQuantizedInput(context, node, const_cast<char *>(inputName.c_str()),
                                    &numInputTensors, inTensorMin, inTensorMax, &inQuantFactorAllTensors, &tensorType, inferenceMode);
            if(status == kTfLiteError)
            {
                return false;
            }

            /* OK, here we write the model in quant, but
            * the quant models will not be used in the
            * next invokes. create() has already been
            * called with float models
            */
            if(currFrameIdx == data->osrt_options.m_calibration_frames) //Have all inputs available now, run calibration
            {
                if(data->osrt_options.m_num_param_bits == 32)
                {
                    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n ************ Frame index %d : Running float inference **************** \n", currFrameIdx);
                }
                else
                {
                    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n ************ Frame index %d : Running fixed point mode for calibration **************** \n", currFrameIdx);
                }
                for (int i = 0; i < numInputTensors; i++)
                {
                    if(tensorType == TENSOR_FLOAT)
                    {
                        inQuantFactorAllTensors[i] =TIDL_findMaxQuantizationScale((*inTensorMin)[i], (*inTensorMax)[i], numParamBits);
                    }
                }
                char subgraphName[10];
                sprintf(subgraphName, "%d", subGraphDelegate->subgraphId_);
                int32_t rtStatus = TIDL_runtimesPostProcessNet(&data->osrt_options, data->osrt_options.m_calibration_frames, data->osrt_options.m_num_param_bits,
                                            subGraphDelegate->subGraphPtr_, subGraphDelegate->gParams_, inQuantFactorAllTensors, NULL, (const char *)subgraphName);
                if(rtStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
                {
                    return false;
                }

                res = true;
            }
            else
            {
                TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n ************ Frame index %d : Running float inference **************** \n", currFrameIdx);
            }
        }
    }

    return res;
}

TfLiteStatus tidlDelegate::Init(TfLiteContext* context, const TfLiteDelegateParams* params)
{
    TIDL_TfLiteDelegateData* data = reinterpret_cast<TIDL_TfLiteDelegateData *>(params->delegate->data_);
    data_ = data;
    int32_t status = 0;

    /* Init is called for each subgraph - add current subgraph to valid_subgraphs and  pending_subgraphs lists */
    data_->valid_subgraphs.push_back(this);
    data_->pending_subgraphs.push_back(this);

    /* If number of subgraphs exceeds "max_num_subgraphs" specified in compilation options, not need to create TIOVX graph for exeution,
    hence bypass the TIDL RT create stage and return with success status */
    if(data_->valid_subgraphs.size() > data_->osrt_options.m_num_tidl_subgraphs)
    {
        return kTfLiteOk;
    }

    subgraphId_ = TIDL_getOneOutputTensorIdx(context, params);

    // Check if subgraph contains OD post processing part
    bool isSubgraphOD = false;

    std::vector<int> nodesInSubgraph;

    for (auto node_index : TfLiteIntArrayView(params->nodes_to_replace))
    {
        nodesInSubgraph.push_back(node_index);
        TfLiteNode* node;
        TfLiteRegistration* registration;
        TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(context, node_index, &node, &registration));

        if((registration->builtin_code == kTfLiteBuiltinCustom) || (registration->builtin_code == kTfLiteBuiltinNonMaxSuppressionV4)
           || (registration->builtin_code == kTfLiteBuiltinNonMaxSuppressionV5))
        {
            isSubgraphOD = true;
        }
    }
    data->supported_node_groups.push_back(nodesInSubgraph);

    /* Check for the case where heads appear as part of distinct subgraphs and hence optimization logic fails */
    if(isSubgraphOD && data->osrt_options.m_meta_arch_type == TIDL_metaArchTfliteRtSsdDetectionLayer)
    {
        std::vector<int> numAspectRatios;
        int reduce_boxes_in_lowest_layer, score_convertor;
        std::vector<string> odHeads;
        bool isSupported = TIDL_isSsdOdPostProcStructureSupported(context, params->nodes_to_replace, odHeads, &data->osrt_options,
                                             numAspectRatios, reduce_boxes_in_lowest_layer, score_convertor);
        if(!isSupported)
        {
            TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("This network cannot be optimized for C7x post processing due to SSD heads being part of different subgraphs. \
 Please specify \"deny_list:layer_type\":\"32\" as part of runtime options and retry compilation");
        }
    }

    status = TIDL_tfliteRtImportInit(context, params, subgraphId_, &data->osrt_options, data->odPostProcHeadNames, isSubgraphOD);
    if(status == -1)
    {
        return kTfLiteError;
    }

    for (auto node_index : TfLiteIntArrayView(params->nodes_to_replace))
    {
        TfLiteNode* node;
        TfLiteRegistration* registration;
        TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(
                    context, node_index, &node, &registration));
        nodes_.push_back(node_index);

        if(((data->odBackboneNodeNames.size() == 0)  //non OD network
          || (std::find(data->odBackboneNodeNames.begin(), data->odBackboneNodeNames.end(), context->tensors[node_index].name) != data->odBackboneNodeNames.end())))
        {
            //Map all nodes for non OD network. For OD network, map nodes only if they are part of backbone, do not map the post proc nodes
            status = TIDL_tfliteRtImportAndLinkNode(registration, context, params, node, data->osrt_options.osrtDebugPrintLevel,  &data->osrt_options.odUserParams);
            if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                return kTfLiteError;
            }
        }
    }

    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization for subgraph_%d started]" , subgraphId_);
    status = TIDL_runtimesOptimizeNet(data->osrt_options.osrtDebugPrintLevel);
    if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
        return kTfLiteError;
    }
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization for subgraph_%d completed]" , subgraphId_);

    TIDL_saveTidlSubGraph(&subGraphPtr_);

    TIDL_saveGparams(&gParams_);

    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
        /* save in float mode in Init and do an RT create
        * all intermediate tensors will be done in float
        * mode
        */
        std::vector<float32_tidl> qfloat(TIDL_MAX_ALG_IN_BUFS);
        //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n ************** Frame index 1 : Running float import ************* \n");
        char subgraphName[10];
        sprintf(subgraphName, "%d", subgraphId_);

        status = TIDL_runtimesPostProcessNet(&data->osrt_options, 1, 32,  subGraphPtr_, gParams_, qfloat.data(), NULL, (const char*)subgraphName);
        if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
            return kTfLiteError;
        }

        sTIDL_tidlRtParams_t * subgraphParams = &this->tidlRtParamsObj;

        /* Setting options used in TIDL_subgraphRtCreate function */
        sTIDL_subgraphRtCreateOptions_t* subgraphRtCreateOptions = (sTIDL_subgraphRtCreateOptions_t*)malloc(sizeof(sTIDL_subgraphRtCreateOptions_t));
        strcpy(subgraphRtCreateOptions->netPath, data_->osrt_options.m_temp_folder.c_str());
        subgraphRtCreateOptions->debugLevel = data_->osrt_options.m_debug_level;
        subgraphRtCreateOptions->osrtDebugPrintLevel = data_->osrt_options.osrtDebugPrintLevel;
        subgraphRtCreateOptions->maxPreEmptDelay = FLT_MAX;
        subgraphRtCreateOptions->targetPriority = 0;
        subgraphRtCreateOptions->coreNum = 1;
        subgraphRtCreateOptions->coreStartIdx = 1;
        subgraphRtCreateOptions->inferenceMode = TIDL_inferenceModeDefault;
        subgraphRtCreateOptions->flowCtrl = TIDL_FLOW_CTRL_COMPILE;
        strcpy(subgraphRtCreateOptions->tempBufferDir, data_->osrt_options.m_temp_buffer_dir.c_str());

        /* Force inference mode to default for floating point compilation pass in OSRT. Fixed point pass indirectly calls invoke through standalone 
        PC_dsp_test_dl_algo.out exe instead of calling the RT executable directly, so this is not required for fixed point pass */
        status = TIDL_subgraphRtCreate(subgraphRtCreateOptions, &data->infer_ops, std::to_string(subgraphId_).c_str(), subgraphParams);
        
        if(subgraphRtCreateOptions != NULL)
        {
            free(subgraphRtCreateOptions);
        }
        if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
            return kTfLiteError;
        }
    }
    return kTfLiteOk;
}

TfLiteStatus tidlDelegate::Prepare(TfLiteContext* context, TfLiteNode* node)
{
    return kTfLiteOk;
}

TfLiteStatus tidlDelegate::Invoke(TfLiteContext* context, TfLiteNode* node)
{
    TIDL_GLOBAL_REPORT_INFO(data_->osrt_options.osrtDebugPrintLevel, "%s %s %d ", __FILE__, __func__, __LINE__);
    TfLiteStatus status = kTfLiteOk;
    int outTensorIdx;
    currFrameIdx_++;
    std::fill_n(inTensorsMin_, TIDL_MAX_ALG_IN_BUFS, FLT_MAX);
    std::fill_n(inTensorsMax_, TIDL_MAX_ALG_IN_BUFS, -FLT_MAX);

    /** Invoke is called to run inference on each subgraph
     * In the first subgraph's invoke call, check for the "valid_subgraphs" populated during init - if number of subgraphs in
     * "valid_subgraphs" is greater than user specified "max_num_subgraphs", then move the additional subgraphs from
     * "valid_subgraphs" to "invalid_subgraphs". Also remove them from "pending_subgraphs"
    */
    while(data_->valid_subgraphs.size() > data_->osrt_options.m_num_tidl_subgraphs)
    {
        /* Move last subgraph from valid_subgraphs to invalid_subgraphs */
        data_->invalid_subgraphs.insert(data_->invalid_subgraphs.end(), data_->valid_subgraphs[data_->valid_subgraphs.size() - 1]);

        data_->pending_subgraphs.pop_back();
        data_->valid_subgraphs.pop_back();
    }

    /* Do subgraph output collection and import only if current subgraph is valid */
    if(std::find(data_->valid_subgraphs.begin(), data_->valid_subgraphs.end(), this) != data_->valid_subgraphs.end())
    {
        /** RT invoke, this will always run in float mode as the RT create was done in Init with float models
         * Used to generate subgraph outputs to be collected for running calibration
        */
        if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
        {
            if(data_->osrt_options.m_num_param_bits != 32)
            {
                /* Floating point pass for subgraph input collection not required in case of execution with numParamBits = 32 */
                TfLiteStatus rtStatus = TIDL_subgraphRtInvoke(data_->osrt_options.osrtDebugPrintLevel, context, node, &this->tidlRtParamsObj, &this->data_->infer_ops);
                if(rtStatus == kTfLiteError)
                {
                    return kTfLiteError;
                }
            }
        }

        /* Run import for each subgraph */
        if(TIDL_subgraphImport(context, node, this, &currNumInTensors_, currFrameIdx_, &inTensorsQuantFactor_,
                &inTensorsMin_, &inTensorsMax_))
        {
            data_->pending_subgraphs.pop_back();

            /* subgraph import complete, copy generated net and io bin files from tempDir to artifacts */
            TIDL_copyFile(std::to_string(subgraphId_) + "_tidl_net.bin", data_->osrt_options.m_artifacts_folder, data_->osrt_options.m_temp_folder);
            TIDL_copyFile(std::to_string(subgraphId_) + "_tidl_io_1.bin", data_->osrt_options.m_artifacts_folder, data_->osrt_options.m_temp_folder);

            if(data_->pending_subgraphs.empty())  /* all subgraphs import complete, write out nodes list */
            {
                if(! data_->invalid_subgraphs.empty())
                {
                    TIDL_GLOBAL_REPORT_WARNING("%d subgraphs generated, but acccelerator supports only %d subgraphs, hence additional subgraphs will be delegated to ARM", data_->valid_subgraphs.size() + data_->invalid_subgraphs.size(), MAX_NUM_TIDL_SUBGRAPHS);
                }
                std::set<int> delete_nodes;
                for (auto it : data_->invalid_subgraphs)
                    delete_nodes.insert(it->nodes_.begin(), it->nodes_.end());

                /** Num subgraphs > max_num_subgraphs case :
                 * Iterate over list of supported nodes - if current node is in the list to be deleted, find the
                 * corresponding supported node group and delete the entire group from supported list
                */
                for (auto node_index : data_->supported_nodes)
                {
                    if(delete_nodes.find(node_index) != delete_nodes.end())
                    {
                        for(int i = 0; i < data_->supported_node_groups.size(); i++)
                        {
                            for(auto& node : data_->supported_node_groups[i])
                            {
                                if(node == node_index)
                                {
                                    data_->supported_node_groups.erase(std::find(data_->supported_node_groups.begin(), data_->supported_node_groups.end(), data_->supported_node_groups[i]));
                                    i--;
                                    break;
                                }
                            }
                        }
                    }
                }

                /* Populate allowed nodes list to be used for inference */
                std::string allow_fname = data_->osrt_options.m_artifacts_folder + "/allowedNode.txt";
                FILE *fp = fopen(allow_fname.c_str(), "w");
                if(fp == NULL)
                {
                    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not open %s", allow_fname.c_str());
                    return kTfLiteError;
                }

                int32_t numSuportedNodes = 0;
                fprintf(fp, "%ld\n", data_->supported_node_groups.size());
                for(int i = 0; i < data_->supported_node_groups.size(); i++)
                {
                    std::vector<int> subgraph = data_->supported_node_groups[i];
                    fprintf(fp, "%ld\n", subgraph.size());
                    for(int j = 0; j < subgraph.size(); j++)
                    {
                    fprintf(fp, "%d\n", subgraph[j]);
                    numSuportedNodes++;
                    }
                }
                fclose(fp);
                TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n Final number of subgraphs:%d , %d nodes delegated to accelerator \n", data_->supported_node_groups.size(), numSuportedNodes);

                //concat the quant params prototxt files of each subgraph into the user given path
                if(gParams.isQuantParamsToBeExported)
                {
                    int32_t exportStatus = TIDL_exportQuantParamsIntoUserPath(&gParams);
                    if(exportStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
                    {
                        TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
                    }
                }

                TIDL_runGraphvizToolRuntimes(data_->osrt_options.m_tidl_tools_path, data_->osrt_options.m_artifacts_folder, data_->osrt_options.m_debug_level);
            }
        }
    }
    TIDL_GLOBAL_REPORT_INFO(data_->osrt_options.osrtDebugPrintLevel, "%s %s %d ", __FILE__, __func__, __LINE__);

    return status;
}

tidlDelegate::~tidlDelegate() {
    if(subGraphPtr_) delete static_cast<sTIDL_OrgNetwork_t*>(subGraphPtr_); /* Not allocated memory in constructor so check if not NULL and then delete*/
    if(gParams_) delete static_cast<tidl_import_config*>(gParams_); /* Not allocated memory in constructor so check if not NULL and then delete*/
    if(gParams.quantizationStyle != TIDL_QuantStyleAsymNP2_TFL)
    {
        TIDL_subgraphRtDelete(data_->osrt_options.osrtDebugPrintLevel, &data_->infer_ops, &this->tidlRtParamsObj);
    }
    /* Following are allocated memory in constructor, so delete in destructor*/
    delete [] inTensorsQuantFactor_;
    delete [] inTensorsMin_;
    delete [] inTensorsMax_;
}

/*

 * Create the TfLiteRegistration for the Kernel node which will replace
 * the subgraph in the main TfLite graph.
 */
static TfLiteRegistration GetTIDLNodeRegistration() {
    /*
     * This is the registration for the Delegate Node that gets added to
     * the TFLite graph instead of the subGraph it replaces.
     * It is treated as a an OP node. But in our case
     * Init will initialize the delegate
     * Invoke will run the delegate graph.
     * Prepare for preparing the delegate.
     * Free for any cleaning needed by the delegate.
     */

    TfLiteRegistration kernel_registration = {0};
    kernel_registration.builtin_code = kTfLiteBuiltinDelegate;
    kernel_registration.custom_name = "tidlDelegate";
    kernel_registration.free = [](TfLiteContext* context, void* buffer) -> void {
        delete reinterpret_cast<tidlDelegate*>(buffer);
    };

    kernel_registration.init = [](TfLiteContext* context, const char* buffer, size_t) -> void* {
        const TfLiteDelegateParams* params = reinterpret_cast<const TfLiteDelegateParams*>(buffer);
        tidlDelegate* delegate = new tidlDelegate;
        if (delegate->Init(context, params) != kTfLiteOk)
            return nullptr;
        return delegate;
    };

    kernel_registration.invoke = [](TfLiteContext* context,
            TfLiteNode* node) -> TfLiteStatus {
        tidlDelegate* kernel = reinterpret_cast<tidlDelegate*>(node->user_data);
        return kernel->Invoke(context, node);
    };

    kernel_registration.prepare = [](TfLiteContext* context,
            TfLiteNode* node) -> TfLiteStatus {
        tidlDelegate* kernel = reinterpret_cast<tidlDelegate*>(node->user_data);
        return kernel->Prepare(context, node);
    };

    kernel_registration.get_custom_data = [](TfLiteContext* context,
            TfLiteNode* node, const char *op_name,
            char **node_name, void **node_data) -> TfLiteStatus {
        return kTfLiteError;
    };


    return kernel_registration;
}


std::vector<std::vector<int>> fusedCombinations = {{kTfLiteBuiltinPack, kTfLiteBuiltinPack, kTfLiteBuiltinReshape},
                                                   /*{kTfLiteBuiltinReshape, kTfLiteBuiltinTranspose, kTfLiteBuiltinReshape}*/};


/* Logic for allowlisting possible fusable combinations of layers:
*  Maintain vector fusedCombinations of possible combinations
*  Maintain vector activeChecks of form {{fusedCombinations Index, current check location in fusedCombinations Index}, {supported nodes in this combination till now}}
*  If a layer matches the beginning of a combination, say i, then push {{i, 1}, {current node index}} in activeChecks; if i not already in activeChecks
*  If activeChecks not empty, then check if next location in index i matches next layer - if yes, increment location by 1 and push node in supported list; if
doesn't match, remove the vector from activeChecks
*  On reaching end of a combination, push nodes from activeChecks to vector supportedNodes to be returned; and remove the vector from activeChecks
*/
std::vector<int> TIDL_tfliteAllowlistFusedLayers(TfLiteIntArray * plan, TfLiteContext* context)
{
    std::vector<int> supportedNodes = {};
    TfLiteNode* node;
    TfLiteRegistration* registration;
    std::vector<std::pair<std::vector<int>, std::vector<int>>> activeChecks = {};

    bool checking = false;

    for (int node_index : TfLiteIntArrayView(plan))
    {
        context->GetNodeAndRegistration(context, node_index, &node, &registration);
        if(activeChecks.size() > 0)
        {
            for(int i = 0; i < activeChecks.size(); i++)
            {
                if(registration->builtin_code == fusedCombinations[activeChecks[i].first[0]][activeChecks[i].first[1]])
                {
                    activeChecks[i].first[1] += 1;
                    activeChecks[i].second.push_back(node_index);
                    if(activeChecks[i].first[1] == fusedCombinations[activeChecks[i].first[0]].size()) //we have checked all layers in combination
                    {
                        supportedNodes.insert(supportedNodes.end(), activeChecks[i].second.begin(), activeChecks[i].second.end());
                        activeChecks.erase(activeChecks.begin() + i);
                    }
                }
                else
                {
                    activeChecks.erase(activeChecks.begin() + i);
                }
            }
        }
        for(int i = 0; i < fusedCombinations.size(); i++)
        {
            if(registration->builtin_code == fusedCombinations[i][0])
            {
                checking = false;
                for(int j = 0; j < activeChecks.size(); j++) //check if i already in activeChecks
                {
                    if(i == activeChecks[j].first[0])
                    {
                        checking = true;
                        break;
                    }
                }
                if(!checking)
                {
                    std::vector<int> map = {i,1};
                    std::vector<int> nodes = {node_index};
                    activeChecks.push_back(std::make_pair(map, nodes));
                }
            }
        }
    }
    return supportedNodes;
}

extern "C"
{

/** Get string op name from op code */
std::string TIDL_getOpNameByRegistration(const TfLiteRegistration& registration) {
  auto op = registration.builtin_code;
  std::string result =
      EnumNameBuiltinOperator(static_cast<BuiltinOperator>(op));
  if ((op == kTfLiteBuiltinCustom || op == kTfLiteBuiltinDelegate) &&
      registration.custom_name) {
    result += "_" + std::string(registration.custom_name);
  }
  return result;
}

/* Generates and prints the parse table */
void TIDL_generateParseTable(TfLiteContext* context, TfLiteIntArray* plan, int32_t numSuportedNodes, int32_t numOffloadSubGraphCreated)
{
  std::string supportedNodes = std::to_string(numSuportedNodes);
  std::string offloadSubGraph = std::to_string(numOffloadSubGraphCreated);
  std::string unsupportedNodes = std::to_string(plan->size - numSuportedNodes);
  std::vector<std::vector<std::string>> denylistData = {};

  TfLiteNode* node;
  TfLiteRegistration* registration;
  if(!gDiags.gDiagList.empty())
  {
    for (int node_index : TfLiteIntArrayView(plan))
    {
        if(gDiags.gDiagList[node_index].getKind() == TIDL_ModelDiagnostic::DK_NotSupported ||
        gDiags.gDiagList[node_index].getKind() == TIDL_ModelDiagnostic::DK_Error ||
        gDiags.gDiagList[node_index].getKind() == TIDL_ModelDiagnostic::DK_NotVerified)
        {
        context->GetNodeAndRegistration(context, node_index, &node, &registration);

        TfLiteTensor* tensor = &context->tensors[node->outputs->data[0]]; //tflite nodes don't have "name" attribute, so identify using node output tensor name

        std::string nodeName = tensor->name;
        std::string opType = TIDL_getOpNameByRegistration(*registration);

        // Parse the reason from diags string
        std::string diag = gDiags.gDiagList[node_index].getString();
        diag = diag.substr(diag.rfind(':') + 1);
        diag = diag.substr(0,diag.find("--"));
        diag.erase(diag.find_last_not_of(' ')+1);
        diag.erase(0, diag.find_first_not_of(' '));

        denylistData.push_back({opType,nodeName,diag});
        }
    }
  }
  TIDL_printParseTable(supportedNodes, offloadSubGraph, unsupportedNodes, denylistData);
}

/** Store all information needed for graph visualization like input/output adjacent nodes, node names, etc and
  * write to graphvizInfo.txt, which will be used by graphviz executable for visualization */
void TIDL_getGraphVisualizationInfo(TfLiteContext* context, TfLiteIntArray* plan, std::string artifactsFolder, std::vector<std::vector<int>> outputAdjacencyList, std::vector<std::vector<int>> inputAdjacencyList)
{
  sTIDL_runtimesVisualisationInfo_t * visInfo = new sTIDL_runtimesVisualisationInfo_t[plan->size];

  TfLiteNode* node;
  TfLiteRegistration* registration;
  for (int node_index : TfLiteIntArrayView(plan))
  {
    context->GetNodeAndRegistration(context, node_index, &node, &registration);

    TfLiteTensor* tensor = &context->tensors[node->outputs->data[0]]; //tflite nodes don't have "name" attribute, so identify using node output tensor name
    visInfo[node_index].nodeModelIdx = node_index;
    visInfo[node_index].nodeName = tensor->name;
    visInfo[node_index].outputAdjNodes = outputAdjacencyList[node_index];
    visInfo[node_index].inputAdjNodes = inputAdjacencyList[node_index];
    visInfo[node_index].opType = TIDL_getOpNameByRegistration(*registration);
    visInfo[node_index].diagnosticInfo = gDiags.gDiagList[node_index].getString();
  }

  std::ofstream outfile;
  std::string graphvizFileName = artifactsFolder + "/tempDir/graphvizInfo.txt";
  outfile.open(graphvizFileName);
  if(outfile.is_open())
  {
    outfile << std::to_string(plan->size) <<std::endl;
    for(int i = 0; i < plan->size; i++)
    {
      outfile << std::to_string(visInfo[i].nodeModelIdx) << " " << visInfo[i].nodeName << " " << visInfo[i].opType << " ";
      outfile << "outputAdjNodes " << visInfo[i].outputAdjNodes.size() << " ";
      if(visInfo[i].outputAdjNodes.size() == 0)
      {
        context->GetNodeAndRegistration(context, TfLiteIntArrayView(plan)[i], &node, &registration);
        std::string output_name = context->tensors[node->outputs->data[0]].name;
        outfile << output_name << " " ;
      }
      for(auto& adjNode : visInfo[i].outputAdjNodes)
      {
        outfile << adjNode << " ";
      }
      outfile << "inputAdjNodes " << visInfo[i].inputAdjNodes.size() << " ";
      if(visInfo[i].inputAdjNodes.size() == 0)
      {
        context->GetNodeAndRegistration(context, TfLiteIntArrayView(plan)[i], &node, &registration);
        std::string input_name = context->tensors[node->inputs->data[0]].name;
        outfile << input_name << " " ;
      }
      for(auto& adjNode : visInfo[i].inputAdjNodes)
      {
        outfile << adjNode << " ";
      }
      outfile << "diagInfo " << visInfo[i].diagnosticInfo;
      outfile << std::endl;
    }
    outfile.close();
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Cannot open %s -- graph visualisation will not work", graphvizFileName.c_str());
  }

  delete [] visInfo;
}

static TfLiteStatus DelegatePrepare(TfLiteContext* context, TfLiteDelegate* delegate) {

    TfLiteStatus ret;
    TfLiteNode* node;
    TfLiteRegistration* registration;

    TIDL_TfLiteDelegateData* data = reinterpret_cast<TIDL_TfLiteDelegateData *>(delegate->data_);

    gDiags.gDiagList.clear();
    data->valid_subgraphs = {};
    data->pending_subgraphs = {};
    data->supported_nodes = {};
    data->supported_node_groups = {};

    int32_t status = TIDL_runtimesGparamsInit(&data->osrt_options, TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT);
    if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
        return kTfLiteError;
    }

    TfLiteIntArray* plan;
    TF_LITE_ENSURE_STATUS(context->GetExecutionPlan(context, &plan));

    bool isObjectDetectionNetwork = false;
    bool hasDetectionPostprocLayer = false;

    TIDL_printCompilationStages();
    TIDL_printVersionTable(TIDL_TOOLS_VERSION,data->osrt_options.m_c7x_firmware_version);

    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Parsing Started]");

    for (int node_index : TfLiteIntArrayView(plan))
    {
        TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(context, node_index, &node, &registration));
        if((registration->builtin_code == kTfLiteBuiltinCustom) || (registration->builtin_code == kTfLiteBuiltinNonMaxSuppressionV4)
           || (registration->builtin_code == kTfLiteBuiltinNonMaxSuppressionV5)||(strcmp(data->osrt_options.m_model_type.c_str(),"OD") == 0))
        {
            isObjectDetectionNetwork = true;
        }
        if((registration->builtin_code == kTfLiteBuiltinCustom) && (data->osrt_options.m_meta_layers_names_list.empty()))
        {
            /* Tflite detection post process layer present, do not use meta arch parameters (these will be derived from tflite detections post process layer),
            create new meta arch type to identify tflite OD models with detection post processing layer */
            hasDetectionPostprocLayer = true;
            data->osrt_options.m_meta_arch_type = TIDL_metaArchTfliteRtSsdDetectionLayer;
        }
    }

    if(! isObjectDetectionNetwork)
    {
        TIDL_GLOBAL_REPORT_WARNING("Network not identified as Object Detection network : (1) Ignore if network is not Object Detection network (2) If network is Object Detection network, please specify \"model_type\":\"OD\" as part of OSRT compilation options");
    }

    std::vector<int> odBackboneNodeIds = {};
    data->odBackboneNodeNames = {};
    std::vector<std::vector<int>> outputAdjacencyList = {{}};
    std::vector<std::vector<int>> inputAdjacencyList = {{}};
    outputAdjacencyList = TIDL_createOutputAdjacencyList(context, plan);
    inputAdjacencyList = TIDL_createInputAdjacencyList(context, plan);

    int32_t numOutputs = 0;
    for(auto &nodeOutputList : outputAdjacencyList) /* output list for each node */
    {
        if(nodeOutputList.size() == 0)
        {
            numOutputs++;
        }
    }

    if(isObjectDetectionNetwork)
    {
        bool odCheckStatus;
        odCheckStatus = TIDL_OdOptionChecks(&data->osrt_options, {TIDL_metaArchTFSSD,TIDL_metaArchTIDLRetinaNet}, hasDetectionPostprocLayer);
        if(! odCheckStatus)
        {
            delete data;
            TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
        }

        std::vector<string> odHeads;
        std::vector<int> postProcInputIds;
        std::vector<int> numAspectRatios;
        int reduce_boxes_in_lowest_layer = 0;
        int score_convertor = 0;

        /* Check if tflite OD post processing assumptions are followed when meta architecture file is not provided */
        if(data->osrt_options.m_meta_arch_type == TIDL_metaArchTfliteRtSsdDetectionLayer)
        {
            bool postprocessingSupported = TIDL_isSsdOdPostProcStructureSupported(context, plan, odHeads, &data->osrt_options,
                                             numAspectRatios, reduce_boxes_in_lowest_layer, score_convertor);

            if(postprocessingSupported)  /* Check successful - continue with meta architecture mapping flow */
            {
                data->odPostProcHeadNames = odHeads;
            }
            else
            {
                /* No meta arch flow and set detection layer in deny list */
                data->osrt_options.m_meta_arch_type = -1;
                TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Delegating detection post process layer to ARM since post processing optimization not supported ");
                data->osrt_options.m_deny_list_layer_type.push_back(std::to_string(kTfLiteBuiltinCustom));
            }
        }
        else
        {
            data->odPostProcHeadNames = TIDL_readMetaArchInfo(data->osrt_options.m_meta_layers_names_list, numOutputs, data->osrt_options.m_meta_arch_type);
        }

        /* Need to populate tidl_OdOutputNames to correctly add output data layers and form corresponding connections*/
        for(int node_index : TfLiteIntArrayView(plan))
        {
            context->GetNodeAndRegistration(context, node_index, &node, &registration);
            if(registration->builtin_code == kTfLiteBuiltinCustom)
            {
                for (int i = 0; i<node->outputs->size; i++)
                {
                    TfLiteTensor* tensor = &context->tensors[node->outputs->data[i]];
                    strcpy((char *)tidl_OdOutputNames[i],tensor->name);
                }
            }
        }

#ifdef CREATE_METAARCH
        /* Code to create OD meta architecture file and populate it using tflite detection post process layer information */
        if(data->osrt_options.m_meta_layers_names_list.empty())
        {
            string pathForMeta = data->osrt_options.m_artifacts_folder + "/tempDir/metaFile.prototxt";
            for(int node_index : TfLiteIntArrayView(plan))
            {
                context->GetNodeAndRegistration(context, node_index, &node, &registration);
                if(registration->builtin_code == kTfLiteBuiltinCustom)
                {
                    numTFMetaLayers++;
                    TfLiteTensor* tensor = &context->tensors[node->inputs->data[2]];
                }
            }
            for(int node_index : TfLiteIntArrayView(plan))
            {
                if(node_index == 0)
                {
                    context->GetNodeAndRegistration(context, node_index, &node, &registration);
                    TfLiteTensor* tensor = &context->tensors[node->inputs->data[0]];
                    height = tensor->dims->data[1];
                    width = tensor->dims->data[2];
                }
                context->GetNodeAndRegistration(context, node_index, &node, &registration);
                if(registration->builtin_code == kTfLiteBuiltinCustom)
                {
                    TIDL_createMetaarchFile(registration,node,context,&data->osrt_options.odUserParams,numTFMetaLayers,numAspectRatios,OD_heads,height,width,reduce_boxes_in_lowest_layer,score_convertor,pathForMeta);
                }
            }
            data->osrt_options.m_meta_layers_names_list = pathForMeta;
            data->osrt_options.m_meta_arch_type = TIDL_metaArchTFSSD;
        }
#endif

        postProcInputIds = TIDL_getPostProcInputIds(context, plan, data->odPostProcHeadNames);

        std::vector<bool> visited;
        visited.assign(plan->size, false);

        // Run DFS on the graph with OD "heads" as root and traversal towards the network input to get nodes of the backbone network
        odBackboneNodeIds =  TIDL_callNodeTraversal(inputAdjacencyList, postProcInputIds, plan->size);

        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Size of odBackboneNodeIds = %d ", odBackboneNodeIds.size());
        for(int i = 0; i < odBackboneNodeIds.size(); i++)
        {
          data->odBackboneNodeNames.push_back(context->tensors[odBackboneNodeIds[i]].name); //convert node indices to corresponding names since indices change after graph partition
        }

        if(data->osrt_options.m_meta_arch_type == TIDL_metaArchTfliteRtSsdDetectionLayer)
        {
            context->GetNodeAndRegistration(context, 0, &node, &registration); /* TODO : implement better way to check for network inputs */
            TfLiteTensor* tensor = &context->tensors[node->inputs->data[0]];
            int height = tensor->dims->data[1];
            int width =tensor->dims->data[2];
            for(int node_index : TfLiteIntArrayView(plan))
            {
                context->GetNodeAndRegistration(context, node_index, &node, &registration);
                if(registration->builtin_code == kTfLiteBuiltinCustom)
                {
                    TIDL_mapDetectionLayerParamsToSsdConfig(registration,node,context,&data->osrt_options.odUserParams,numAspectRatios,odHeads,height,width,reduce_boxes_in_lowest_layer,score_convertor);
                }
            }
        }
    }
    else
    {
        data->odBackboneNodeNames = {};
        data->odPostProcHeadNames = {};
        data->osrt_options.m_meta_arch_type = -1;
        data->osrt_options.m_meta_layers_names_list = "";
    }


    int num_components = 1;
    int last_index = -1;
    bool isSupported = false;
    std::vector<int> supportedFusedNodes = TIDL_tfliteAllowlistFusedLayers(plan, context);

    for (int node_index : TfLiteIntArrayView(plan))
    {
        TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(
                    context, node_index, &node, &registration));

        if(data->osrt_options.m_quantization_scale_type == TIDL_QuantStyleAsymNP2_TFL)
        {
            if((registration->builtin_code == kTfLiteBuiltinSoftmax) || (registration->builtin_code == kTfLiteBuiltinAveragePool2d) /* || (registration->builtin_code == kTfLiteBuiltinMul)*/)
            {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Layer delegated to ARM since this layer type is currently not supported for tflite asymmetric quantization");
                continue;
            }
        }

        isSupported = false;
        for(int i = 0; i < supportedFusedNodes.size(); i++)
        {
            if(supportedFusedNodes[i] == node_index)
            {
                isSupported = true;
                TfLiteTensor* tensor = &context->tensors[node->outputs->data[0]];
                TIDL_LOG_INFO(gDiags.gDiagList, "Layer %s, tflite layer code - %d -- supported via fusion, individually might not be supported", tensor->name, registration->builtin_code);
                if (data->osrt_options.osrtDebugPrintLevel)
                {
                    gDiags.reportLastModeDiag();
                }
                break;
            }
        }

        if(!isSupported)
        {
            isSupported = TIDL_tfliteAllowlistNode(registration, node, node_index, context, &data->osrt_options, isObjectDetectionNetwork, odBackboneNodeIds);
        }


        if (isSupported)
        {
            if (last_index != -1 && node_index != last_index + 1)
            {
                ++num_components;  //TODO: Primitive logic to check number of subgraphs, need to have a more robust logic
            }
            data->supported_nodes.push_back(node_index);
            last_index = node_index;
        }
    }

    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nTotal Nodes = %d", plan->size);

    TIDL_generateParseTable(context, plan, data->supported_nodes.size(), num_components);

    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Parsing Completed]");

    TfLiteRegistration TIDL_kernel_registration = GetTIDLNodeRegistration();

    TfLiteIntArray *nodes = static_cast<TfLiteIntArray *>(malloc(sizeof(*nodes) + sizeof(nodes->data[0]) * data->supported_nodes.size()));
    nodes->size = data->supported_nodes.size();
    std::copy(data->supported_nodes.begin(), data->supported_nodes.end(), &nodes->data[0]);
    ret = context->ReplaceNodeSubsetsWithDelegateKernels(context, TIDL_kernel_registration, nodes, delegate);
    free(nodes);

    TIDL_getGraphVisualizationInfo(context, plan, data->osrt_options.m_artifacts_folder, outputAdjacencyList, inputAdjacencyList);

    return ret;
}

TfLiteDelegate* tflite_plugin_create_delegate(char** options_keys,
                                              char** options_values,
                                              size_t num_options,
                                              void (*error_handler)(const char*)) {
    TIDL_TfLiteDelegateData *data = new TIDL_TfLiteDelegateData;

    data->infer_ops.lib = dlopen("libvx_tidl_rt.so", RTLD_LAZY | RTLD_GLOBAL);

    if(!data->infer_ops.lib)
    {
        delete data;

        // TODO: make a function to take va_list arguments
        if (error_handler) error_handler("could not load library libvx_tidl_rt.so");
        return nullptr;
    }
    try
    {
        TIDL_rtLoadSymbols(&data->infer_ops);

    } catch (std::string &e)
    {
        delete data;

        if (error_handler) error_handler("could not load symbol from library libvx_tidl_rt.so");
        return nullptr;
    }

    data->osrt_options.odUserParams.confidence_threshold = -1;
    data->osrt_options.odUserParams.nms_threshold = -1;
    data->osrt_options.odUserParams.top_k = -1;
    data->osrt_options.odUserParams.keep_top_k = -1;

    int32_t status;
    for (uint32_t idx = 0; idx < num_options; idx++)
    {
        status = false;

        status = TIDL_readInterfaceOptions(&data->osrt_options, options_keys[idx], options_values[idx], TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT);
        if(! status)
        {
            delete data;
            return NULL;
        }
    }
    TIDL_derivedInterfaceOptions(&data->osrt_options);

    status = TIDL_checkInterfaceOptions(&data->osrt_options);
    if(!status)
    {
        delete data;
        return NULL;
    }

    TIDL_printInterfaceOptions(&data->osrt_options);

    // TODO: any unparsed arguments?

    TfLiteDelegate* delegate = new TfLiteDelegate;
    delegate->data_ = static_cast<void*>(data);
    delegate->flags = kTfLiteDelegateFlagsNone;
    delegate->Prepare = DelegatePrepare;
    delegate->CopyFromBufferHandle = nullptr;
    delegate->CopyToBufferHandle = nullptr;
    delegate->FreeBufferHandle = nullptr;
    return delegate;
}

void tflite_plugin_destroy_delegate(TfLiteDelegate* delegate) {
    TIDL_TfLiteDelegateData *data = nullptr;

    if(delegate)
        data = static_cast<TIDL_TfLiteDelegateData *>(delegate->data_);

    delete delegate;
    delete data;
}

}
}
}
