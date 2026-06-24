/*
* Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <ctime>
#include "tidlruntime_infer.h"
#include "tidlruntime_compile.h"

namespace py = pybind11;

/*
 * Global variable for performance data
 */
uint64_t run_start_ts;
uint64_t run_start_ddr_read;
uint64_t run_start_ddr_write;
uint64_t run_end_ts;
uint64_t run_end_ddr_read;
uint64_t run_end_ddr_write;

/*
 * Class to parse input details from IOBuf Descriptor
 */
class __attribute__((visibility("default"))) CompileInputDetail
{
    public:
        CompileInputDetail(std::string _name, std::vector<int32_t> _shape, int32_t _type)
        {
            name.assign(_name.c_str());
            shape = _shape;
            type = _type;
        }
    public:
        py::dict detail;
        std::vector<int32_t> shape;
        int32_t type;
        std::string name;
};

/*
 * Class to parse input details from IOBuf Descriptor
 */
class __attribute__((visibility("default"))) InferenceInputDetail
{
    public:
        InferenceInputDetail(sTIDL_IOBufDesc_t* ioBufDesc, int32_t i)
        {
            name.assign((char *)ioBufDesc->inDataName[i]);

            shape = {ioBufDesc->inNumBatches[i],
                     ioBufDesc->inDIM1[i],
                     ioBufDesc->inDIM2[i],
                     ioBufDesc->inNumChannels[i],
                     ioBufDesc->inHeight[i],
                     ioBufDesc->inWidth[i]};

            type = ioBufDesc->inElementType[i];

            pad = {ioBufDesc->inPadCh[i],
                   ioBufDesc->inPadT[i],
                   ioBufDesc->inPadB[i],
                   ioBufDesc->inPadL[i],
                   ioBufDesc->inPadR[i]};

            detail["inDataName"] = name;
            detail["inBufSize"] = ioBufDesc->inBufSize[i];

            detail["inWidth"] = ioBufDesc->inWidth[i];
            detail["inHeight"] = ioBufDesc->inHeight[i];
            detail["inNumChannels"] = ioBufDesc->inNumChannels[i];
            detail["inDIM2"] = ioBufDesc->inDIM2[i];
            detail["inDIM1"] = ioBufDesc->inDIM1[i];
            detail["inNumBatches"] = ioBufDesc->inNumBatches[i];

            detail["inPadL"] = ioBufDesc->inPadL[i];
            detail["inPadT"] = ioBufDesc->inPadT[i];
            detail["inPadR"] = ioBufDesc->inPadR[i];
            detail["inPadB"] = ioBufDesc->inPadB[i];
            detail["inPadCh"] = ioBufDesc->inPadCh[i];

            detail["inElementType"] = ioBufDesc->inElementType[i];
            detail["inLayout"] = ioBufDesc->inLayout[i];

            detail["inChannelPitch"] = ioBufDesc->inChannelPitch[i];

            detail["inDataFormat"] = ioBufDesc->inDataFormat[i];
            detail["inResizeType"] = ioBufDesc->inResizeType[i];
            detail["rawDataInElementType"] = ioBufDesc->rawDataInElementType[i];
            detail["inDataId"] = ioBufDesc->inDataId[i];
            detail["inTensorScale"] = ioBufDesc->inTensorScale[i];
            detail["inZeroPoint"] = ioBufDesc->inZeroPoint[i];
        }
    public:
        py::dict detail;
        std::vector<int32_t> shape;
        int32_t type;
        std::vector<int32_t> pad;
        std::string name;
};

/*
 * Class to parse output details from IOBuf Descriptor
 */
class __attribute__((visibility("default"))) InferenceOutputDetail
{
    public:
        InferenceOutputDetail(sTIDL_IOBufDesc_t* ioBufDesc, int32_t i)
        {
            name.assign((char *)ioBufDesc->outDataName[i]);

            shape = {ioBufDesc->outNumBatches[i],
                     ioBufDesc->outDIM1[i],
                     ioBufDesc->outDIM2[i],
                     ioBufDesc->outNumChannels[i],
                     ioBufDesc->outHeight[i],
                     ioBufDesc->outWidth[i]};

            type = ioBufDesc->outElementType[i];

            pad = {ioBufDesc->outPadCh[i],
                   ioBufDesc->outPadT[i],
                   ioBufDesc->outPadB[i],
                   ioBufDesc->outPadL[i],
                   ioBufDesc->outPadR[i]};

            detail["outDataName"] = name;
            detail["outBufSize"] = ioBufDesc->outBufSize[i];

            detail["outWidth"] = ioBufDesc->outWidth[i];
            detail["outHeight"] = ioBufDesc->outHeight[i];
            detail["outNumChannels"] = ioBufDesc->outNumChannels[i];
            detail["outDIM2"] = ioBufDesc->outDIM2[i];
            detail["outDIM1"] = ioBufDesc->outDIM1[i];
            detail["outNumBatches"] = ioBufDesc->outNumBatches[i];

            detail["outPadL"] = ioBufDesc->outPadL[i];
            detail["outPadT"] = ioBufDesc->outPadT[i];
            detail["outPadR"] = ioBufDesc->outPadR[i];
            detail["outPadB"] = ioBufDesc->outPadB[i];
            detail["outPadCh"] = ioBufDesc->outPadCh[i];

            detail["outElementType"] = ioBufDesc->outElementType[i];
            detail["outLayout"] = ioBufDesc->outLayout[i];

            detail["outChannelPitch"] = ioBufDesc->outChannelPitch[i];

            detail["outDataId"] = ioBufDesc->outDataId[i];
            detail["outTensorScale"] = ioBufDesc->outTensorScale[i];
            detail["outZeroPoint"] = ioBufDesc->outZeroPoint[i];
            detail["numValidTensorDims"] = ioBufDesc->numValidTensorDims[i];
        }
    public:
        py::dict detail;
        std::vector<int32_t> shape;
        int32_t type;
        std::vector<int32_t> pad;
        std::string name;
};

/*
 * Utility function that timestamp in nanoseconds
 *
 */
static void get_time_u64(uint64_t *t)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    *t = ((uint64_t)ts.tv_sec * (uint64_t)1000000000ULL) + (uint64_t)ts.tv_nsec;
}

/*
 * Utility function that timestamp in nanoseconds
 *
 */
static void resetPerfData()
{
    run_start_ts = 0;
    run_start_ddr_read = 0;
    run_start_ddr_write = 0;
    run_end_ts = 0;
    run_end_ddr_read = 0;
    run_end_ddr_write = 0;
}

/*
 * Utility function that returns the size in bytes TIDL datatype.
 *
 * Parameters:
 *   dtype: TIDL Datatype
 *
 * Returns:
 *   size of dtype in bytes
 */
int getElementSize(int dtype) 
{
    if(dtype == TIDL_UnsignedChar) return sizeof(uint8_t);
    if(dtype == TIDL_SignedChar) return sizeof(int8_t);
    if(dtype == TIDL_UnsignedShort) return sizeof(uint16_t);
    if(dtype == TIDL_SignedShort) return sizeof(int16_t);
    if(dtype == TIDL_SinglePrecFloat) return sizeof(float);
    if(dtype == TIDL_Bool) return sizeof(bool);
    if(dtype == TIDLRT_Uint32) return sizeof(uint32_t);
    if(dtype == TIDLRT_Int32) return sizeof(int32_t);
    if(dtype == TIDLRT_Uint64) return sizeof(uint64_t);
    if(dtype == TIDLRT_Int64) return sizeof(int64_t);
    if(dtype == TIDLRT_Bool) return sizeof(bool);

    throw std::runtime_error("[ERROR] Invalid TIDL Data type");
}

/*
 * Utility function that converts py:dtype to TIDL datatype.
 *
 * Parameters:
 *   np_array: py::array
 *
 * Returns:
 *   TIDL Datatype.
 */
int getArrayType(const py::array& np_array) 
{
    const py::dtype dtype = np_array.dtype();
    if (dtype.is(py::dtype::of<uint8_t>())) return TIDL_UnsignedChar;
    if (dtype.is(py::dtype::of<int8_t>())) return TIDL_SignedChar;
    if (dtype.is(py::dtype::of<uint16_t>())) return TIDL_UnsignedShort;
    if (dtype.is(py::dtype::of<int16_t>())) return TIDL_SignedShort;
    if (dtype.is(py::dtype::of<uint32_t>())) return TIDL_UnsignedWord;
    if (dtype.is(py::dtype::of<int32_t>())) return TIDL_SignedWord;
    if (dtype.is(py::dtype::of<float>())) return TIDL_SinglePrecFloat;
    if (dtype.is(py::dtype::of<uint64_t>())) return TIDL_UnsignedDoubleWord;
    if (dtype.is(py::dtype::of<int64_t>())) return TIDL_SignedDoubleWord;
    if (dtype.is(py::dtype::of<bool>())) return TIDL_Bool;

    return (-1);
}

/*
 * Utility function that check py:array shape against iobuf descriptor shape
 *
 * Parameters:
 *   input: py::array
 *   ioBufDesc: 
 *
 * Returns:
 *   TIDL Datatype.
 */
std::vector<int32_t> getArrayShape(const py::array& np_array) 
{
    py::buffer_info buf = np_array.request();
    std::vector<int32_t> shape(TIDL_DIM_MAX,1);

    if (buf.ndim > TIDL_DIM_MAX) 
    {
        throw std::runtime_error("[ERROR] Tensor cannot have more than 6 dimensions.");
    }
    
    for (int i = 0; i < buf.ndim; ++i) 
    {
        shape[TIDL_DIM_MAX - buf.ndim + i] = buf.shape[i];
    }

    return shape;
}

/*
 * Utility function to check if given input(py::array) is valid against iobuf
 * descriptor
 *
 * Parameters:
 *   name: Input tensor name
 *   np_array: py::array
 *   ioBufDesc: IOBuf Descriptor
 *   index: Input index in ioBufDesc
 *
 * Returns:
 *   Pair containing return type and error message
 */
std::pair<int,std::string> checkInputTensorValid(const std::string& name,
                                                 const py::array& np_array,
                                                 sTIDL_IOBufDesc_t* ioBufDesc,
                                                 int index) 
{
    // Check element type.
    int np_type = getArrayType(np_array);
    int tidl_type = ioBufDesc->inElementType[index];

    if(np_type != tidl_type)
    {
        return {-1, "Input tensor '" + name + "' has mismatched element type"};
    }

    // Check dimensions.
    std::vector<int32_t> arrayShape = getArrayShape(np_array);

    int xBatch = ioBufDesc->inNumBatches[index];
    int xDim1 = ioBufDesc->inDIM1[index];
    int xDim2 = ioBufDesc->inDIM2[index];
    int xChannel = ioBufDesc->inNumChannels[index] + ioBufDesc->inPadCh[index];
    int xHeight = ioBufDesc->inPadT[index] + ioBufDesc->inHeight[index] + ioBufDesc->inPadB[index];
    int xWidth = ioBufDesc->inPadL[index] + ioBufDesc->inWidth[index] + ioBufDesc->inPadR[index];

    if(arrayShape[0] != xBatch      ||
       arrayShape[1] != xDim1       ||
       arrayShape[2] != xDim2       ||
       arrayShape[3] != xChannel    ||
       arrayShape[4] != xHeight     ||
       arrayShape[5] != xWidth)
    {
        return {-1, "Input tensor '" + name + "' has mismatched shape"}; 
    }

    return {0, ""};

}

/*
 * Utility function to det the parameters for sTIDLRT_Tensor_t befor the Inference invoke
 *
 * Parameters:
 *   np_array: py::array
 *   ins: sTIDLRT_Tensor_t
 *   in: Array of sTIDLRT_Tensor_t pointers
 *   index: Index in ioBufDesc
 *   ioBufDesc: IOBuf descriptor
 *   isInput: Is input?
 *
 * Returns:
 *   Pair containing return type and error message
 */
std::pair<int,std::string> setRtTensorParameters(void *dataPtr,
                                                 sTIDLRT_Tensor_t *tensor,
                                                 sTIDLRT_Tensor_t *tensorArr[],
                                                 int32_t index,
                                                 sTIDL_IOBufDesc_t *ioBufDesc,  
                                                 int32_t isInput,
                                                 int32_t isInfer,
                                                 sTIDL_tidlRtDynamicLoading_t *infer_ops)
{
    int32_t status = 0;

    uint8_t* ptr = (uint8_t *)dataPtr;
    int32_t memType = 0;
    int32_t elementSize;
    int32_t numVirtualCores;
    int32_t numSuperBatches;
    int32_t numBatches;
    int32_t numIosPerCore;
    int32_t numElementsPerBatch;

    numVirtualCores = ioBufDesc->numVirtualCores;
    numSuperBatches = ioBufDesc->numSuperBatches;
    numBatches = (numVirtualCores * numSuperBatches);

    if(ptr == NULL)
    {
        status = -1;
        return {status, "Could not get pointer to numpy array"};
    }


    if(isInput == 1)
    {
        numIosPerCore = ioBufDesc->numInputBuf / numBatches;
        numElementsPerBatch = ioBufDesc->inDIM1[index] * ioBufDesc->inDIM2[index] * ioBufDesc->inNumChannels[index] * ioBufDesc->inHeight[index] * ioBufDesc->inWidth[index];
    }
    else
    {
        numIosPerCore = ioBufDesc->numOutputBuf / numBatches;
        numElementsPerBatch = ioBufDesc->outDIM1[index] * ioBufDesc->outDIM2[index] * ioBufDesc->outNumChannels[index] * ioBufDesc->outHeight[index] * ioBufDesc->outWidth[index];
    }
   

    if(isInfer && infer_ops->TIDLRT_isSharedMem(ptr))
    {
        memType = TIDLRT_MEM_SHARED;
    }

    for(int l = 0; l < numSuperBatches; l++)
    {
        for(int k = 0; k < numVirtualCores; k++)
        {
            int32_t idx = (l * numVirtualCores * numIosPerCore + k * numIosPerCore + index);

            tensorArr[idx] = &(tensor[idx]);

            infer_ops->TIDLRT_setTensorDefault(tensorArr[idx]);

            if(isInput == 1)
            {
                strcpy((char *)tensorArr[idx]->name, (char *)ioBufDesc->inDataName[index]);
                tensorArr[idx]->bufferSize  = ioBufDesc->inBufSize[index];
                tensorArr[idx]->elementType = ioBufDesc->inElementType[index];    
                tensorArr[idx]->scale = ioBufDesc->inTensorScale[index];
                tensorArr[idx]->zeroPoint = ioBufDesc->inZeroPoint[index];
                tensorArr[idx]->layout = ioBufDesc->inLayout[index];
                tensorArr[idx]->pitch[TIDL_LINE_PITCH]    = ioBufDesc->inPadL[index] + ioBufDesc->inWidth[index] + ioBufDesc->inPadR[index];
                tensorArr[idx]->pitch[TIDL_CHANNEL_PITCH] = ioBufDesc->inChannelPitch[index];
                tensorArr[idx]->pitch[TIDL_DIM2_PITCH]    = tensorArr[idx]->pitch[TIDL_CHANNEL_PITCH] * ioBufDesc->inNumChannels[index];
                tensorArr[idx]->pitch[TIDL_DIM1_PITCH]    = tensorArr[idx]->pitch[TIDL_DIM2_PITCH] * ioBufDesc->inDIM2[index];
                tensorArr[idx]->pitch[TIDL_ROI_PITCH]     = tensorArr[idx]->pitch[TIDL_DIM1_PITCH] * ioBufDesc->inDIM1[index];
                tensorArr[idx]->padValues[0] = ioBufDesc->inPadL[index];
                tensorArr[idx]->padValues[1] = ioBufDesc->inPadR[index];
                tensorArr[idx]->padValues[2] = ioBufDesc->inPadT[index];
                tensorArr[idx]->padValues[3] = ioBufDesc->inPadB[index];
                tensorArr[idx]->dimValues[TIDL_DIM_WIDTH]   = ioBufDesc->inWidth[index];
                tensorArr[idx]->dimValues[TIDL_DIM_HEIGHT]  = ioBufDesc->inHeight[index];
                tensorArr[idx]->dimValues[TIDL_DIM_NUMCH]   = ioBufDesc->inNumChannels[index];
                tensorArr[idx]->dimValues[TIDL_DIM_DIM2]  = ioBufDesc->inDIM2[index];
                tensorArr[idx]->dimValues[TIDL_DIM_DIM1]  = ioBufDesc->inDIM1[index];
                tensorArr[idx]->dimValues[TIDL_DIM_BATCH]   = ioBufDesc->inNumBatches[index];

                elementSize = getElementSize(ioBufDesc->inElementType[index]);
            }
            else
            {
                strcpy((char *)tensorArr[idx]->name, (char *)ioBufDesc->outDataName[index]);
                tensorArr[idx]->bufferSize  = ioBufDesc->outBufSize[index];
                tensorArr[idx]->elementType = ioBufDesc->outElementType[index];
                tensorArr[idx]->scale = ioBufDesc->outTensorScale[index];
                tensorArr[idx]->zeroPoint = ioBufDesc->outZeroPoint[index];
                tensorArr[idx]->layout = ioBufDesc->outLayout[index];
                tensorArr[idx]->pitch[TIDL_LINE_PITCH]    = ioBufDesc->outPadL[index] + ioBufDesc->outWidth[index] + ioBufDesc->outPadR[index];
                tensorArr[idx]->pitch[TIDL_CHANNEL_PITCH] = ioBufDesc->outChannelPitch[index];
                tensorArr[idx]->pitch[TIDL_DIM2_PITCH]    = tensorArr[idx]->pitch[TIDL_CHANNEL_PITCH] * ioBufDesc->outNumChannels[index];
                tensorArr[idx]->pitch[TIDL_DIM1_PITCH]    = tensorArr[idx]->pitch[TIDL_DIM2_PITCH] * ioBufDesc->outDIM2[index];
                tensorArr[idx]->pitch[TIDL_ROI_PITCH]     = tensorArr[idx]->pitch[TIDL_DIM1_PITCH] * ioBufDesc->outDIM1[index];
                tensorArr[idx]->padValues[0] = ioBufDesc->outPadL[index];
                tensorArr[idx]->padValues[1] = ioBufDesc->outPadR[index];
                tensorArr[idx]->padValues[2] = ioBufDesc->outPadT[index];
                tensorArr[idx]->padValues[3] = ioBufDesc->outPadB[index];
                tensorArr[idx]->dimValues[TIDL_DIM_WIDTH]   = ioBufDesc->outWidth[index];
                tensorArr[idx]->dimValues[TIDL_DIM_HEIGHT]  = ioBufDesc->outHeight[index];
                tensorArr[idx]->dimValues[TIDL_DIM_NUMCH]   = ioBufDesc->outNumChannels[index];
                tensorArr[idx]->dimValues[TIDL_DIM_DIM2]   = ioBufDesc->outDIM2[index];
                tensorArr[idx]->dimValues[TIDL_DIM_DIM1]   = ioBufDesc->outDIM1[index];
                tensorArr[idx]->dimValues[TIDL_DIM_BATCH]   = ioBufDesc->outNumBatches[index];

                elementSize = getElementSize(ioBufDesc->outElementType[index]);
            }
     
            tensorArr[idx]->ptr = ptr + l * numVirtualCores * numElementsPerBatch * elementSize + k * numElementsPerBatch * elementSize;

            if(memType == TIDLRT_MEM_SHARED)
            {
                tensorArr[idx]->memType = TIDLRT_MEM_SHARED;
            }
        }
    }
  
    return {status,""};
}


/*
 * This function returns the input details from iobuf descriptor.
 *
 * Parameters:
 *   sess: TIDLRT_InferenceSession
 *
 * Returns:
 *   List of input details.
 */
static std::vector<InferenceInputDetail> getInferenceInputDetails(const TIDLRT_InferenceSession &sess)
{
    std::vector<InferenceInputDetail> inputDetails = {};
    sTIDL_IOBufDesc_t* ioBufDesc;

    if(sess.m_session_created == 0)
    {
        return inputDetails;
    }

    ioBufDesc = (sTIDL_IOBufDesc_t *)sess.m_infer_state.m_tidlrt_params.ioBufDesc;
    if (ioBufDesc == NULL)
    {
        return inputDetails;
    }

    for (int32_t i = 0; i < ioBufDesc->numInputBuf; i++)
    {
        inputDetails.emplace_back(ioBufDesc,i);
    }

    return inputDetails;
}

/*
 * This function returns the output details from iobuf descriptor.
 *
 * Parameters:
 *   sess: TIDLRT_InferenceSession
 *
 * Returns:
 *   List of output details.
 */
static std::vector<InferenceOutputDetail> getInferenceOutputDetails(const TIDLRT_InferenceSession &sess)
{
    std::vector<InferenceOutputDetail> outputDetails = {};
    sTIDL_IOBufDesc_t* ioBufDesc;

    if(sess.m_session_created == 0)
    {
        return outputDetails;
    }

    ioBufDesc = (sTIDL_IOBufDesc_t *)sess.m_infer_state.m_tidlrt_params.ioBufDesc;
    if (ioBufDesc == NULL)
    {
        return outputDetails;
    }

    for (int32_t i = 0; i < ioBufDesc->numOutputBuf; i++)
    {
        outputDetails.emplace_back(ioBufDesc,i);
    }
    return outputDetails;
}

/*
 * This function returns the performance statistics
 *
 * Parameters:
 *   sess: TIDLRT_InferenceSession
 *
 * Returns:
 *   Dictionary of performance numbers
 */
static py::dict getPerformance(TIDLRT_InferenceSession &sess)
{

    py::dict perf;
    sTIDLRT_PerfStats_t *perfStats;

    // Set keys
    perf["ts:copy_in_start"] = 0;
    perf["ts:copy_in_end"] = 0;
    perf["ts:proc_start"] = 0;
    perf["ts:proc_end"] = 0;
    perf["ts:copy_out_start"] = 0;
    perf["ts:copy_out_end"] = 0;
    perf["ts:run_start"] = 0;
    perf["ts:run_end"] = 0;
    perf["ddr:read_start"] = 0;
    perf["ddr:read_end"] = 0;
    perf["ddr:write_start"] = 0;
    perf["ddr:write_end"] = 0;

    if(sess.m_session_created == 0)
    {
        return perf;
    }

    perfStats = (sTIDLRT_PerfStats_t*)sess.m_infer_state.m_tidlrt_params.stats;
    if (perfStats == NULL)
    {
        return perf;
    }

    perf["ts:copy_in_start"] = uint64_t(perfStats->cpIn_time_start);
    perf["ts:copy_in_end"] = uint64_t(perfStats->cpIn_time_end);
    perf["ts:proc_start"] = uint64_t(perfStats->proc_time_start);
    perf["ts:proc_end"] = uint64_t(perfStats->proc_time_end);
    perf["ts:copy_out_start"] = uint64_t(perfStats->cpOut_time_start);
    perf["ts:copy_out_end"] = uint64_t(perfStats->cpOut_time_end);

    perf["ts:run_start"] = uint64_t(run_start_ts);
    perf["ts:run_end"] = uint64_t(run_end_ts);
    perf["ddr:read_start"] = uint64_t(run_start_ddr_read);
    perf["ddr:read_end"] = uint64_t(run_end_ddr_read);
    perf["ddr:write_start"] = uint64_t(run_start_ddr_write);
    perf["ddr:write_end"] = uint64_t(run_end_ddr_write);

    return perf;
}

/*
 * This function creates an object of TIDLRT_InferenceSession.
 *
 * Parameters:
 *   options_dict: Inference options dictionary
 *
 * Returns:
 *   TIDLRT_InferenceSession.
 */
TIDLRT_InferenceSession* createInferenceSession(const py::dict &options_dict)
{
    resetPerfData();

    std::map<std::string,std::string> sessOpts;
    for (auto &item : options_dict)
    {
        std::string key = py::str(item.first);
        std::string value = py::str(item.second);
        sessOpts[key] = value;
    }

    return new TIDLRT_InferenceSession(sessOpts);
}

template <typename T>
py::array_t<T> createPyArray(void *data, std::vector<int32_t> &shape)
{
    std::vector<py::ssize_t> arrShape = {};
    std::vector<py::ssize_t> arrStrides = {};
    for (int32_t i = 0; i < (int32_t)shape.size(); i++)
    {
        arrShape.push_back(static_cast<py::ssize_t>(shape[i]));
    }


    for (int32_t i = 0; i < (int32_t)shape.size(); i++)
    {
        int32_t stride = sizeof(T);
        for (int32_t j = i+1; j < (int32_t)shape.size(); j++)
        {
            stride = stride * shape[j];
        }
        arrStrides.push_back(static_cast<py::ssize_t>(stride));
    }

    py::capsule freeDataCapsule(data, [](void *f)
    {
        free(f);
    });

    return py::array_t<T>(arrShape,arrStrides,(T*)data,freeDataCapsule);
}

static py::dict runInferenceSession(TIDLRT_InferenceSession &sess, const py::dict &inputs)
{

    int32_t status = 0 ;
    py::dict outputs;

    std::pair<int, std::string> err;

    sTIDLRT_Tensor_t *in[128];
    sTIDLRT_Tensor_t *out[128];
    void*             outDataPtr[128];

    sTIDLRT_Tensor_t *ins;
    sTIDLRT_Tensor_t *outs;

    sTIDL_IOBufDesc_t* ioBufDesc;

    if(sess.m_session_created == 0)
    {
        return outputs;
    }

    resetPerfData();
    sess.m_infer_state.m_infer_ops.TIDLRT_getDdrStats(&run_start_ddr_read, &run_start_ddr_write);
    get_time_u64(&run_start_ts);

    ins = (sTIDLRT_Tensor_t *)sess.m_infer_state.m_tidlrt_params.rtInList;
    outs = (sTIDLRT_Tensor_t *)sess.m_infer_state.m_tidlrt_params.rtOutList;

    ioBufDesc = (sTIDL_IOBufDesc_t *)sess.m_infer_state.m_tidlrt_params.ioBufDesc;
    if (ioBufDesc == NULL)
    {
        return outputs;
    }

    for (int32_t i = 0; i < ioBufDesc->numInputBuf; i++)
    {
        std::string name((char *)ioBufDesc->inDataName[i]);

        // Check if name exists in inputs dict, if not error out .
        if (!inputs.contains(name.c_str())) 
        {
            throw std::runtime_error("Input '" + name + "' not found in inputs");
        }

        // Get the numpy array from the inputs dictionary corresponding to the name.
        py::object obj = inputs[name.c_str()];
        if(!py::isinstance<py::array>(obj))
        {
            throw std::runtime_error("Input '" + name + "' is not a numpy array");
        }

        py::array np_array = py::cast<py::array>(obj);
        if(np_array.is_none())
        {
            throw std::runtime_error("Input '" + name + "' is None");
        }

        // Check validity of input tesnor
        err = checkInputTensorValid(name, np_array, ioBufDesc, i);
        if(err.first != 0)
        {
            throw std::runtime_error(err.second);
        }

        // Get pointer from input array and set then set the tensor parameters
        py::buffer_info buf_info = np_array.request();
        void *dataPtr = static_cast<void*>(buf_info.ptr);

        err = setRtTensorParameters(dataPtr, ins, in, i, ioBufDesc, 1, 1, &sess.m_infer_state.m_infer_ops);
        if(err.first != 0)
        {
            throw std::runtime_error(err.second);
        }
    }

    // Allocate output memory
    for (int32_t i = 0; i < ioBufDesc->numOutputBuf; i++)
    {
        void *dataPtr = NULL;
        int32_t elementType = ioBufDesc->outElementType[i];
        int32_t bufSize = ioBufDesc->outBufSize[i];
        int32_t totalSize =  bufSize * getElementSize(elementType);
        dataPtr = (void *)malloc(totalSize);
        if(dataPtr == NULL)
        {
            throw std::runtime_error("[ERROR] Could not allocate output buffer");
        }

        outDataPtr[i] = dataPtr;

        err = setRtTensorParameters(dataPtr, outs, out, i, ioBufDesc, 0, 1, &sess.m_infer_state.m_infer_ops);
        if(err.first != 0)
        {
            throw std::runtime_error(err.second);
        }
    }

    // Run inference
    status = sess.TIDLRT_InferenceSession_Run(in,out);
    if (status != 0)
    {
        throw std::runtime_error("Error while invoking the session");
    }

     // Wrap the allocated output memory in py_array_t
    for (int32_t i = 0; i < ioBufDesc->numOutputBuf; i++)
    {
        std::string name((char *)ioBufDesc->outDataName[i]);
        int batch = ioBufDesc->outNumBatches[i];
        int dim1 = ioBufDesc->outDIM1[i];
        int dim2 = ioBufDesc->outDIM2[i];
        int channel = ioBufDesc->outNumChannels[i] + ioBufDesc->outPadCh[i];
        int height = ioBufDesc->outPadT[i] + ioBufDesc->outHeight[i] + ioBufDesc->inPadB[i];
        int width = ioBufDesc->outPadL[i] + ioBufDesc->outWidth[i] + ioBufDesc->outPadR[i];

        std::vector<int> outShape = {batch,dim1,dim2,channel,height,width};

        int32_t elementType = ioBufDesc->outElementType[i];

        // // Allocate output array
        if (elementType == TIDL_SinglePrecFloat)
        {
            outputs[name.c_str()] = createPyArray<float>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_UnsignedChar)
        {
            outputs[name.c_str()] = createPyArray<uint8_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_SignedChar)
        {
            outputs[name.c_str()] = createPyArray<int8_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_UnsignedShort)
        {
            outputs[name.c_str()] = createPyArray<uint16_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_SignedShort)
        {
           outputs[name.c_str()] = createPyArray<int16_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_UnsignedWord)
        {
            outputs[name.c_str()] = createPyArray<uint32_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_SignedWord)
        {
           outputs[name.c_str()] = createPyArray<int32_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_UnsignedDoubleWord)
        {
            outputs[name.c_str()] = createPyArray<uint64_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_SignedDoubleWord)
        {
            outputs[name.c_str()] = createPyArray<int64_t>((void *)outDataPtr[i], outShape);
        }
        else if (elementType == TIDL_Bool)
        {
            outputs[name.c_str()] = createPyArray<bool>((void *)outDataPtr[i], outShape);
        }
        else
        {
            throw std::runtime_error("Invalid Output dataype");
        }
    }

    get_time_u64(&run_end_ts);
    sess.m_infer_state.m_infer_ops.TIDLRT_getDdrStats(&run_end_ddr_read, &run_end_ddr_write);

    return outputs;
}


static int32_t deleteInferenceSession(TIDLRT_InferenceSession &sess)
{
    resetPerfData();
    return sess.TIDLRT_InferenceSession_Delete();
}

static void dumpInferSessionOptions(TIDLRT_InferenceSession &sess)
{
    return sess.TIDLRT_InferenceSession_DumpOptions();
}


/*
 * This function creates an object of TIDLRT_CompileSession.
 *
 * Parameters:
 *   options_dict: Compile options dictionary
 *
 * Returns:
 *   TIDLRT_CompileSession.
 */
TIDLRT_CompileSession* createCompileSession(const py::dict &options_dict)
{
    std::map<std::string,std::string> sessOpts;
    for (auto &item : options_dict)
    {
        std::string key = py::str(item.first);
        std::string value = py::str(item.second);
        sessOpts[key] = value;
    }

    return new TIDLRT_CompileSession(sessOpts);
}

/*
 * This function returns the input details from compile parameters
 *
 * Parameters:
 *   sess: TIDLRT_CompileSession
 *
 * Returns:
 *   List of input details.
 */
static std::vector<CompileInputDetail> getCompileInputDetails(const TIDLRT_CompileSession &sess)
{
    std::vector<CompileInputDetail> inputDetails = {};
#ifdef BUILD_COMPILE
    if(sess.m_session_created == 0)
    {
        return inputDetails;
    }

    std::string inDataNamesList((char *)sess.m_compile_option.m_options.inDataNamesList);
    std::replace(inDataNamesList.begin(), inDataNamesList.end(), ',', ' ');
    std::stringstream ss(inDataNamesList);
    std::string token;

    int32_t i = 0;
    while (ss >> token) 
    {
        std::vector<int32_t> shape = {
            sess.m_compile_option.m_options.numBatches[i],
            sess.m_compile_option.m_options.inDIM1[i],
            sess.m_compile_option.m_options.inDIM2[i],
            sess.m_compile_option.m_options.inNumChannels[i],
            sess.m_compile_option.m_options.inHeight[i],
            sess.m_compile_option.m_options.inWidth[i]
        };

        int32_t type = sess.m_compile_option.m_options.inElementType[i];
        inputDetails.emplace_back(token,shape,type);
        i++;
    }
#endif /* BUILD_COMPILE */

    return inputDetails;
}

static int32_t runCompileSession(TIDLRT_CompileSession &sess, const py::dict &inputs = py::none())
{
    int32_t status = 0 ;

#ifdef BUILD_COMPILE
	if (inputs.is_none() || py::len(inputs) <= 0)
    {
        /* Create Compile Session without any numpy input to use inData as
         * as input for calibration
         */
        status = sess.TIDLRT_CompileSession_Run(nullptr);
    }

    else
    {
        calibTensor *tensorArray[128];
        calibTensor tensor[128];
        int idx = 0;

        std::string inDataNamesList((char *)sess.m_compile_option.m_options.inDataNamesList);
        std::replace(inDataNamesList.begin(), inDataNamesList.end(), ',', ' ');
        std::stringstream ss(inDataNamesList);
        std::string name;

        while (ss >> name) 
        {
            // Check if name exists in inputs dict, if not error out .
            if (!inputs.contains(name.c_str())) 
            {
                throw std::runtime_error("Input '" + name + "' not found in inputs");
            }

            // Get the numpy array from the inputs dictionary corresponding to the name.
            py::object obj = inputs[name.c_str()];
            if(!py::isinstance<py::array>(obj))
            {
                throw std::runtime_error("Input '" + name + "' is not a numpy array");
            }

            py::array array = py::cast<py::array>(obj);
            if(array.is_none())
            {
                throw std::runtime_error("Input '" + name + "' is None");
            }

            py::buffer_info buf_info = array.request();
            int type = getArrayType(array);
            std::vector<int32_t> arrayShape = getArrayShape(array);
            if (type == -1)
            {
                throw std::runtime_error("Invalid datatype for array specified");
            }

            tensor[idx].ptr = static_cast<void*>(buf_info.ptr);
            tensor[idx].elementType = type;
            tensor[idx].dimValues[TIDL_DIM_BATCH] = arrayShape[0];
            tensor[idx].dimValues[TIDL_DIM_DIM1] = arrayShape[1];
            tensor[idx].dimValues[TIDL_DIM_DIM2] = arrayShape[2];
            tensor[idx].dimValues[TIDL_DIM_NUMCH] = arrayShape[3];
            tensor[idx].dimValues[TIDL_DIM_HEIGHT] = arrayShape[4];
            tensor[idx].dimValues[TIDL_DIM_WIDTH] = arrayShape[5];
            tensorArray[idx] = &tensor[idx];
            idx++;
        }       

        status = sess.TIDLRT_CompileSession_Run(tensorArray, idx);
    }
#endif /* BUILD_COMPILE */

    return status;
}

static int32_t deleteCompileSession(TIDLRT_CompileSession &sess)
{
    return sess.TIDLRT_CompileSession_Delete();
}

static void dumpCompileSessionOptions(TIDLRT_CompileSession &sess)
{
    return sess.TIDLRT_CompileSession_DumpOptions();
}

PYBIND11_MODULE(tidlruntime_core, m) {
    // Compile session
    py::class_<TIDLRT_CompileSession>(m, "CompileSession", "TIDL-RT Compile Session class")
        .def(py::init(&createCompileSession), "Create TIDL-RT Compile Session")
        .def("run",&runCompileSession, "Run TIDL-RT Compile Session")
        .def("delete",&deleteCompileSession, "Delete TIDL-RT Compile Session")
        .def("dump_options",&dumpCompileSessionOptions, "Print TIDL-RT Compile Session options")
        .def("get_input_details",&getCompileInputDetails, "List of Input details");

    m.attr("TIDL_UnsignedChar")         = TIDL_UnsignedChar;
    m.attr("TIDL_SignedChar")           = TIDL_SignedChar;
    m.attr("TIDL_UnsignedShort")        = TIDL_UnsignedShort;
    m.attr("TIDL_SignedShort")          = TIDL_SignedShort;
    m.attr("TIDL_UnsignedWord")         = TIDL_UnsignedWord;
    m.attr("TIDL_SignedWord")           = TIDL_SignedWord;
    m.attr("TIDL_SinglePrecFloat")      = TIDL_SinglePrecFloat;
    m.attr("TIDL_UnsignedDoubleWord")   = TIDL_UnsignedDoubleWord;
    m.attr("TIDL_SignedDoubleWord")     = TIDL_SignedDoubleWord;
    m.attr("TIDL_Bool")                 = TIDL_Bool;

    py::class_<CompileInputDetail>(m, "CompileInputDetail", "Class for details of single input")
        .def_readonly("detail",&CompileInputDetail::detail, "Dictionary having all input details")
        .def_readonly("shape",&CompileInputDetail::shape, "Shape (excluding pad) of the input. [Batch, Dim1, Dim2, Channel, Height, Width]")
        .def_readonly("type",&CompileInputDetail::type, "Data type of the input in TIDL format")
        .def_readonly("name",&CompileInputDetail::name, "Name of the input");

    // Inference session
    py::class_<TIDLRT_InferenceSession>(m, "InferenceSession", "TIDL-RT Inference Session class")
        .def(py::init(&createInferenceSession), "Create TIDL-RT Inference Session")
        .def("run",&runInferenceSession, "Run TIDL-RT Inference Session")
        .def("delete",&deleteInferenceSession, "Delete TIDL-RT Inference Session")
        .def("dump_options",&dumpInferSessionOptions, "Print TIDL-RT Inference Session options")
        .def("get_input_details",&getInferenceInputDetails, "List of Input details")
        .def("get_output_details",&getInferenceOutputDetails, "List of Output details")
        .def("get_performance",&getPerformance, "Performance details of the run");

    m.attr("TIDL_UnsignedChar")         = TIDL_UnsignedChar;
    m.attr("TIDL_SignedChar")           = TIDL_SignedChar;
    m.attr("TIDL_UnsignedShort")        = TIDL_UnsignedShort;
    m.attr("TIDL_SignedShort")          = TIDL_SignedShort;
    m.attr("TIDL_UnsignedWord")         = TIDL_UnsignedWord;
    m.attr("TIDL_SignedWord")           = TIDL_SignedWord;
    m.attr("TIDL_SinglePrecFloat")      = TIDL_SinglePrecFloat;
    m.attr("TIDL_UnsignedDoubleWord")   = TIDL_UnsignedDoubleWord;
    m.attr("TIDL_SignedDoubleWord")     = TIDL_SignedDoubleWord;
    m.attr("TIDL_Bool")                 = TIDL_Bool;

    py::class_<InferenceInputDetail>(m, "InferenceInputDetail", "Class for details of single input")
        .def_readonly("detail",&InferenceInputDetail::detail, "Dictionary having all input details")
        .def_readonly("shape",&InferenceInputDetail::shape, "Shape (excluding pad) of the input. [Batch, Dim1, Dim2, Channel, Height, Width]")
        .def_readonly("pad",&InferenceInputDetail::pad, "Pads for the input. [Pad_Channel, Pad_Top, Pad_Bottom, Pad_Left, Pad_Right]")
        .def_readonly("type",&InferenceInputDetail::type, "Data type of the input in TIDL format")
        .def_readonly("name",&InferenceInputDetail::name, "Name of the input");

    py::class_<InferenceOutputDetail>(m, "InferenceOutputDetail", "Class for details of single output")
        .def_readonly("detail",&InferenceOutputDetail::detail, "Dictionary having all output details")
        .def_readonly("shape",&InferenceOutputDetail::shape, "Shape (excluding pad) of the output. [Batch, Dim1, Dim2, Channel, Height, Width]")
        .def_readonly("pad",&InferenceOutputDetail::pad, "Pads for the output. [Pad_Channel, Pad_Top, Pad_Bottom, Pad_Left, Pad_Right]")
        .def_readonly("type",&InferenceOutputDetail::type, "Data type of the output in TIDL format")
        .def_readonly("name",&InferenceOutputDetail::name, "Name of the output");
}