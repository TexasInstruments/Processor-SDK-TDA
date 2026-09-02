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

#include "tidlrt_ep.h"


int32_t validateOptions(TIDL_TidlRtEPInferOptions *options)
{
    // Check existence of net and io bin
    if (!TIDL_checkIsPath((char *)options->m_net_bin_path))
    {
        printf("ERROR - %s is not a file\n", (char *)options->m_net_bin_path);
        return -1;
    }
    else if (!TIDL_checkIsPath((char *)options->m_io_bin_path))
    {
        printf("ERROR - %s is not a file\n", (char *)options->m_io_bin_path);
        return -1;
    }

    // Check target priority
    if(options->m_target_priority < 0 || options->m_target_priority > 7)
    {        
        printf("ERROR - Invalid target priority %d given. Allowed options are between 0-7\n", options->m_target_priority);
        return -1;
    }

    // Check flow control is set correctly
    if(options->m_flow_ctrl != 0 && options->m_flow_ctrl != 1 &&
       options->m_flow_ctrl != 12 && options->m_flow_ctrl != -1)
    {
        printf("WARNING - Invalid flow control %d given. Allowed options are 0, 1 & 12. It will be ignored\n", options->m_flow_ctrl);
        options->m_flow_ctrl = -1;
    }

#ifdef x86_64
    // Check temp_buffer_dir in case of x86_64
    if(!TIDL_checkIsDir((char *)options->m_temp_buffer_dir)) 
    {
        printf("WARNING - Directory to redirect temporary buffers %s does not exist. Defaultiong to /dev/shm\n",(char *)options->m_temp_buffer_dir);
        strcpy(options->m_temp_buffer_dir, "/dev/shm");
    }
    if(strlen((char *)(options->m_temp_buffer_dir)) > 64U)
    {
        printf("WARNING - Directory to redirect temporary buffers %s exceeds the maximum allowed path length of 64, Defaultiong to /dev/shm\n", (char *)options->m_temp_buffer_dir);
        strcpy(options->m_temp_buffer_dir ,"/dev/shm");
    }
#endif

    return 0;
}

extern "C"
{
void TIDL_setDefaultOptions(TIDL_TidlRtEPInferOptions *options)
{
    strcpy(options->m_net_bin_path, "");
    strcpy(options->m_io_bin_path, "");
    strcpy(options->m_tidl_tools_path, "");
    options->m_debug_level = 0;
    options->m_target_priority = 0;
    options->m_max_pre_empt_delay = FLT_MAX;
    options->m_core_number = 1;
    options->m_core_start_idx = 1;
    options->m_flow_ctrl = -1;
    strcpy(options->m_temp_buffer_dir, "/dev/shm");
    options->m_trace_base_name[0] = '\0';
}

int32_t TIDL_createInferSession(TIDL_TidlRtEPInferState *state)
{
    int32_t status = 0;

    // Initialize member variables
    state->m_tidlrt_params.ioBufDesc = NULL;
    state->m_tidlrt_params.netPtr = NULL;
    state->m_tidlrt_params.rtHandle = NULL;
    state->m_tidlrt_params.rtInList = NULL;
    state->m_tidlrt_params.rtOutList = NULL;
    state->m_tidlrt_params.stats = NULL;

    TIDL_TidlRtEPInferOptions *options = &state->m_options;

    // Load libraries
    std::string lib_path;
    std::string tidl_tools_path(options->m_tidl_tools_path);
    if (tidl_tools_path == "")
    {
        lib_path = "libvx_tidl_rt.so";
        state->m_infer_ops.lib = dlopen(lib_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    }
    else
    {
        lib_path = tidl_tools_path + "/libvx_tidl_rt.so";
        state->m_infer_ops.lib = dlopen(lib_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if(!state->m_infer_ops.lib)
        {
            lib_path = "libvx_tidl_rt.so";
            state->m_infer_ops.lib = dlopen(lib_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        }
    }

    if(!state->m_infer_ops.lib)
    {
        printf("ERROR - %s\n", dlerror());
        return -1;
    }

    TIDL_rtLoadSymbols(&state->m_infer_ops);

    
    status = validateOptions(options);
    if(status != 0)
    {
        return status;
    }

    sTIDL_subgraphRtCreateOptions_t* subgraphRtCreateOptions = (sTIDL_subgraphRtCreateOptions_t*) malloc(sizeof(sTIDL_subgraphRtCreateOptions_t));
    strcpy(subgraphRtCreateOptions->netBinPath, options->m_net_bin_path);
    strcpy(subgraphRtCreateOptions->ioBinPath, options->m_io_bin_path);
    subgraphRtCreateOptions->debugLevel = options->m_debug_level;
    subgraphRtCreateOptions->maxPreEmptDelay = options->m_max_pre_empt_delay;
    subgraphRtCreateOptions->targetPriority = options->m_target_priority;
    subgraphRtCreateOptions->coreNum = options->m_core_number;
    subgraphRtCreateOptions->coreStartIdx = options->m_core_start_idx;
    subgraphRtCreateOptions->inferenceMode = -1;
    subgraphRtCreateOptions->flowCtrl = options->m_flow_ctrl;
#ifdef x86_64
    strcpy(subgraphRtCreateOptions->tempBufferDir, options->m_temp_buffer_dir);
#endif
    strncpy(subgraphRtCreateOptions->traceBaseName, options->m_trace_base_name, TIDLRT_STRING_SIZE - 1);
    subgraphRtCreateOptions->traceBaseName[TIDLRT_STRING_SIZE - 1] = '\0';

    //Create the session
    status = TIDL_subgraphRtCreate(subgraphRtCreateOptions, &state->m_infer_ops, NULL, &state->m_tidlrt_params);

    if(subgraphRtCreateOptions != NULL)
    {
        free(subgraphRtCreateOptions);
    }

    return status;
}

int32_t TIDL_runInferSession(TIDL_TidlRtEPInferState *state, sTIDLRT_Tensor_t *in[], sTIDLRT_Tensor_t *out[])
{
    int32_t status = 0;
    void *handle;
    sTIDL_IOBufDesc_t *ioBufDesc;
    int32_t numInputBuf;
    int32_t numOutputBuf;

    if (state == nullptr)
    {
        return -1;
    }

    handle = (void *)state->m_tidlrt_params.rtHandle;
    if (handle == nullptr)
    {
        printf("ERROR - Session handle is not created\n");
        return -1;
    }

    ioBufDesc = (sTIDL_IOBufDesc_t *)state->m_tidlrt_params.ioBufDesc;
    if (handle == nullptr)
    {
        printf("ERROR - Session iobuf descriptor is null\n");
        return -1;
    }

    numInputBuf = ioBufDesc->numInputBuf;
    numOutputBuf = ioBufDesc->numOutputBuf;

    for (int32_t i = 0; i < numInputBuf; i++)
    {
        if (in[i] == NULL)
        {
            printf("ERROR - Input buffer at %d is NULL\n", i);
            return -1;
        }
    }

    for (int32_t i = 0; i < numOutputBuf; i++)
    {
        if (out[i] == NULL)
        {
            printf("ERROR - Output buffer at %d is NULL\n", i);
            return -1;
        }
    }

    status = state->m_infer_ops.TIDLRT_invoke(handle, in, out);

    return status;
}

int32_t TIDL_deleteInferSession(TIDL_TidlRtEPInferState *state)
{
    int32_t status = 0;

    if (state == nullptr)
    {
        return -1;
    }

    status = TIDL_subgraphRtDelete(0, &state->m_infer_ops, &state->m_tidlrt_params);

    return status;
}
} //extern C