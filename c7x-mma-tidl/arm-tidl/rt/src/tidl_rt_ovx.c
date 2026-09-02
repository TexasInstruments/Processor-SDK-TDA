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

//#undef _POSIX_C_SOURCE
//#define _POSIX_C_SOURCE 200809L
#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include <TI/tivx_task.h>
#include <TI/j7_tidl.h>
#include <tivx_utils_file_rd_wr.h>
#include <tivx_utils_graph_perf.h>
#include <tivx_utils_tidl_trace.h>
#include <TI/dl_kernels.h>

#include <VX/vx_khr_safe_casts.h>

#if ! defined (HOST_EMULATION)
#include "utils/perf_stats/include/app_perf_stats.h"
#endif

#ifdef HOST_EMULATION
#include "utils/mem/include/app_mem.h"
#include <signal.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include "itidl_rt.h"
#include "tidl_rt_ovx_utils.h"
#include "tidl_rt_ovx_debug_utils.h"
#include "tidl_rt_profile.h"
#include "tidl_rt_force_negative_test.h"
#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   /* for nanosleep */
int nanosleep(const struct timespec *req,
              struct timespec *rem);
int clock_gettime (clockid_t __clock_id, struct timespec *__tp);
int pthread_mutex_lock (pthread_mutex_t *__mutex);
int pthread_mutex_unlock (pthread_mutex_t *__mutex);
#endif
extern char* strdup(const char *ptr);

#define TIVX_TIDL_TRACE_DATA_SIZE  (64 * 1024 * 1024)

#if defined TIDL_COVERAGE_DEAD_CODE
#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));
#endif

#define ABS_FLT(a) ((a) > 0)?(a):(-(a))
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) > (B) ? (B) : (A))

#define OFFSET_TO_MULTI_SUBGRAPH_NET_MAGIC_NUMBER (1)
#define OFFSET_TO_MULTI_SUBGRAPH_IO_MAGIC_NUMBER (sizeof(sTIDL_IOBufDesc_t))

#define TIDL_NODE_ERROR_EVENT (1U)

static uint32_t         tidlrt_debuglevel = 0;
static uint32_t         tidlrt_handles_state[TIDLRT_MAX_HANDLES] = {0};
static IntHandle_t      tidlrt_handles_pool[TIDLRT_MAX_HANDLES];
static pthread_mutex_t  tidlrt_handles_lock = PTHREAD_MUTEX_INITIALIZER;

#if defined (HOST_EMULATION)
static int8_t sigsev_deinit = 0;
#endif

static void get_time_u64(uint64_t *t);
int32_t tidlrt_free_handle(void * ptr);
void tidlrt_printf(const char *fmt, ...);
int32_t getTidlRtFlowCtrl(void);
int32_t getAVXKernelEnv(void);
void * tidlrt_alloc_handle(void);

void * tidlrt_alloc_handle(void)
{
    void * ptr = NULL;
    (void)pthread_mutex_lock(&tidlrt_handles_lock);
    for (int32_t i = 0; i <TIDLRT_MAX_HANDLES; i++)
    {
        if(tidlrt_handles_state[i] == 0U)
        {
            tidlrt_handles_state[i] = 1;
            ptr = (void*)&tidlrt_handles_pool[i];
            break;
        }
    }
    (void)pthread_mutex_unlock(&tidlrt_handles_lock);
    return ptr;

}

int32_t tidlrt_free_handle(void * ptr)
{
    int32_t ret = VX_FAILURE;
    if(ptr != NULL)
    {
        for (int32_t i = 0; i <TIDLRT_MAX_HANDLES; i++)
        {
            if(&tidlrt_handles_pool[i] == ptr)
            {
                tidlrt_handles_state[i] = 0;
                ret = VX_SUCCESS;
                break;
            }
        }
    }
    if(ret == (int32_t)VX_FAILURE)
    {
        tidlrt_printf("TIDL_RT_OVX_ERR: NOT a Valid TIDLRT handle Memory \n");
    }

    return ret;

}


static vx_user_data_object mapConfig(AppObj *obj, vx_context context, const sTIDLRT_Params_t *prms, uint32_t *num_input_tensors, uint32_t *num_output_tensors, int32_t * inferenceMode);
static vx_user_data_object mapNetwork(AppObj *obj, void * netPtr, int32_t capacity);
static int init_tidl_tiovx (AppObj *obj, sTIDLRT_Params_t *prms);
static int32_t deinit_tidl_tiovx(AppObj *obj, const sTIDLRT_Params_t *prms);
static vx_status create_graph_tidl_tiovx(AppObj *obj);
static void delete_graph_tidl_tiovx(AppObj *obj);
static vx_status run_graph_tidlrt_tiovx(AppObj *obj, sTIDLRT_Tensor_t *in[], sTIDLRT_Tensor_t *out[], sTIDLRT_PerfStats_t *stats);
static vx_status verify_graph_tidl_tiovx(AppObj *obj);
static vx_user_data_object setCreateParams(AppObj *obj, sTIDLRT_Params_t *prms, int32_t inferenceMode);
static vx_user_data_object setInArgs(AppObj *obj);
static vx_user_data_object setOutArgs(vx_context context);

static vx_status addParam(vx_reference params[], vx_reference obj, uint32_t *num_params);

static vx_status createInputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors);
static vx_status createOutputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *output_tensors);

#if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
/*
    createObjArray packs num_tensors vx_tensors into num_array_elements vx_obj_array
    While packing it ensures it packs according to HighThroughPut mode, so that extracting tensors will become easier in MPU_1    
*/
static vx_object_array createObjArray(vx_context context, vx_tensor *tensors, uint32_t num_tensors);
#endif

static vx_size getTensorDataType(vx_int32 tidl_type);
static uint32_t getElementSize(uint32_t data_type);

static uint32_t get_tidlrt_id(uint32_t id, char *name, sTIDLRT_Tensor_t *tensors[], uint32_t num_tensors);

static vx_status map_cp_in_tidlrt_tensor_tiovx(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors, void *input_buffer, sTIDLRT_Tensor_t *in[]);
static vx_status memset_out_tensor_tidlrt_tiovx(AppObj *obj, vx_user_data_object config, vx_tensor *output_tensors, sTIDLRT_Tensor_t *out[]);
static vx_status map_cp_out_tensor_tidlrt_tiovx(AppObj *obj, vx_user_data_object config, vx_tensor *output_tensors, sTIDLRT_Tensor_t *out[]);
//static vx_status cp_data_in_tidlrt_tensor_tiovx_slow(AppObj *obj, sTIDLRT_Tensor_t *in[], void *input_buffer, uint32_t id, uint32_t tidlrt_id);
//static vx_status cp_data_out_tensor_tidlrt_tiovx_slow(AppObj *obj, sTIDLRT_Tensor_t *out[], void *output_buffer, uint32_t id, uint32_t elementSize, vx_size data_type, float scale, uint32_t tidlrt_id);
static vx_status set_in_tidlrt_tensor_refs(AppObj *obj, vx_tensor *input_tensors, sTIDLRT_Tensor_t *in[]);
static vx_status set_out_tidlrt_tensor_refs(AppObj *obj, vx_tensor *output_tensors, sTIDLRT_Tensor_t *out[]);
static vx_status allocate_intermediate_tensors(IntHandle_t *rtHandle);
static vx_status tidlrt_check_assert_status(vx_reference ref);


static vx_status tidlrt_check_assert_status(vx_reference ref)
{
    vx_status status = (vx_status)VX_FAILURE;
    if(vxGetStatus((ref)) == (vx_status)VX_SUCCESS)
    {
        status = VX_SUCCESS;
    }

    return status;
}

static vx_status tidlrt_force_shuffle_kernel_params(const AppObj *obj)
{
    AppObj dummyObj = *obj;
    vx_user_data_object *dummyObjData[5] = {&dummyObj.network,
                                            &dummyObj.createParams,
                                            &dummyObj.inArgs,
                                            &dummyObj.outArgs,
                                            &dummyObj.traceData};

    dummyObj.network = NULL;
    (void)create_graph_tidl_tiovx(&dummyObj);
    (void)verify_graph_tidl_tiovx(&dummyObj);
    dummyObj = *obj;

    dummyObj.traceData = NULL;
    (void)create_graph_tidl_tiovx(&dummyObj);
    (void)verify_graph_tidl_tiovx(&dummyObj);
    dummyObj = *obj;

    for (int32_t i = 0; i < 5; i++)
    {
        vx_user_data_object tempObj;
        tempObj = vxCreateUserDataObject(dummyObj.context, "DUMMY", 1, NULL);
        *dummyObjData[i] = tempObj;
        (void)create_graph_tidl_tiovx(&dummyObj);
        (void)verify_graph_tidl_tiovx(&dummyObj);
        (void)vxReleaseNode(&dummyObj.tidl_node);
        (void)vxReleaseGraph(&dummyObj.graph);
        (void)vxReleaseUserDataObject(&tempObj);
        dummyObj = *obj;
    }

    (void)vxRemoveKernel(dummyObj.kernel);
    (void)vxReleaseContext(&dummyObj.context);

    return (vx_status)VX_FAILURE;
}

void tidlrt_printf(const char *fmt, ...)
{
    va_list ap;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
    Users aren't expected to set TIDL_RT_DEBUG env variable,
    only for debug purposes.
    TIDL_LDRA_TAG: TIDL_LDRA_TAG_TIDL_RT_DEBUG_ENV_VAR
    Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
    However, due to the stated rationale, this is not covered in production code.
    <justification end> */
    if(tidlrt_debuglevel == 0U)
    /* LDRA_JUSTIFY_END */
    {

    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
    Users aren't expected to set TIDL_RT_DEBUG env variable,
    only for debug purposes.
    TIDL_LDRA_TAG: TIDL_LDRA_TAG_TIDL_RT_DEBUG_ENV_VAR
    Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
    However, due to the stated rationale, this is not covered in production code.
    <justification end> */
    else
    {
        va_start(ap, fmt);
        (void)vprintf(fmt, ap);
        va_end(ap);
    }
    /* LDRA_JUSTIFY_END */
}

/** TIDL_LDRA_TAG_TIDL_RT_DEBUG_ENV_VAR */
static void __attribute__((constructor)) lib_init(void)
{
    char *debug_str;

    debug_str = getenv("TIDL_RT_DEBUG");
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
    Users aren't expected to set TIDL_RT_DEBUG env variable,
    only for debug purposes.
    Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
    However, due to the stated rationale, this is not covered in production code.
    <justification end> */
    if(!debug_str)
    /* LDRA_JUSTIFY_END */
    {
        tidlrt_debuglevel = 0;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
    Users aren't expected to set TIDL_RT_DEBUG env variable,
    only for debug purposes.
    Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
    However, due to the stated rationale, this is not covered in production code.
    <justification end> */
    else
    {
        char *endptr = NULL;
        unsigned long val = strtoul(debug_str, &endptr, 10);
        if ((debug_str != endptr) && (*endptr == '\0'))
        {
            tidlrt_debuglevel = (uint32_t)val;
        }
    }
    /* LDRA_JUSTIFY_END */
}

int32_t getTidlRtFlowCtrl(void)
{
    int32_t flowCtrl;
    #if defined(x86_64) || defined(SOC_TDA54)
        /* TODO [TIDL-14527]: Run ref flow by default on TDA54 until silicon is available.
         * Revert SOC_TDA54 condition when TDA54 silicon comes. */
        flowCtrl = 1;
    #else
        flowCtrl = 0;
    #endif

    return flowCtrl;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t getAVXKernelEnv.* <function end>
<justification start> The function is for utilty only, in case
TIDLRT_setParamsDefault is directly called as an API without the Testbench,
otherwise the Testbench's utility getAVXKernelEnv is called, and not this one.
<justification end> */
int32_t getAVXKernelEnv(void)
{
int32_t env = 0;
#ifdef x86_64
    char *str;
    str = getenv("TIDL_RT_AVX_REF");
    if(!str)
    {
        env = 0x00000020;
    }
    else
    {
        char *endptr = NULL;
        long val = strtol(str, &endptr, 10);
        if (str != endptr && *endptr == '\0')
        {
            env = (int32_t)val;
            if((env != 0) && (env != 1) && (env != 3))
            {
                tidlrt_printf("TIDL_RT_AVX_REF is expected to be either 0 or 1 or 3. -  %d is not supported. Setting it to zero\n", env);
                env = 0;
            }
            if(env != 0)
            {
                uint32_t shiftedEnv = (uint32_t)env << (uint32_t)5;
                env = (int32_t)shiftedEnv;
            }
        }
   }
#endif
    return env;
}

int32_t TIDLRT_setParamsDefault(sTIDLRT_Params_t *prms)
{
    int32_t status                  = 0;
    prms->netPtr                    = NULL;
    prms->ioBufDescPtr              = NULL;
    prms->net_capacity              = 0;
    prms->io_capacity               = 0;
    prms->dumpNetInitBackupData     = 0;
    prms->releaseIOTensorsAtCreate  = 0;
    prms->computeChecksum           = 0;

    int32_t flowCtrl_1              = getTidlRtFlowCtrl();
    int32_t flowCtrl_2              = getAVXKernelEnv();
    prms->flowCtrl                  = ((uint32_t) flowCtrl_1) | ((uint32_t) flowCtrl_2);

    prms->quantRangeExpansionFactor = 1;
    prms->quantRangeUpdateFactor    = 0;

    prms->targetPriority            = 0;
    prms->maxPreEmptDelay           = FLT_MAX;

    prms->coreNum                   = 1;
    prms->coreStartIdx              = 1;

    prms->forceNegativeTest         = 0;
    prms->enableCodeCoverage        = 0;

    prms->traceLogLevel             = 0;
    prms->traceWriteLevel           = 0;
    (void)strcpy(prms->traceBaseName, "/tmp/tidl_trace");
    (void)strcpy(prms->tempBufferDir, "/dev/shm");
    prms->stats                     = NULL;

    prms->TIDLVprintf               = vprintf;
    prms->TIDLWriteBinToFile        = NULL;
    prms->TIDLReadBinFromFile       = NULL;

    tidlrt_printf("TIDL_RT_OVX: Set default TIDLRT params done\n");
    return status;
}

int32_t TIDLRT_setTensorDefault(sTIDLRT_Tensor_t *tensor)
{
    int32_t status                  = 0;
    //tensor->name[]                = {0};
    tensor->elementType             = 0;
    tensor->numDim                  = 0;
    tensor->ptr                     = NULL;
    tensor->dataOffset              = 0;
    tensor->layout                  = 0;
    tensor->zeroPoint               = 0;
    tensor->scale                   = 1.0;
    tensor->memType                 = 0;
    tensor->pitch[TIDL_ROI_PITCH] = -1;
    tensor->pitch[TIDL_DIM1_PITCH] = -1;
    tensor->pitch[TIDL_DIM2_PITCH] = -1;
    tensor->pitch[TIDL_CHANNEL_PITCH] = -1;
    tensor->pitch[TIDL_LINE_PITCH] = -1;
    tensor->padValues[0] = 0;
    tensor->padValues[1] = 0;
    tensor->padValues[2] = 0;
    tensor->padValues[3] = 0;
    tensor->dimValues[TIDL_DIM_WIDTH]   = -1;
    tensor->dimValues[TIDL_DIM_HEIGHT]  = -1;
    tensor->dimValues[TIDL_DIM_NUMCH]   = -1;
    tensor->dimValues[TIDL_DIM_DIM2]   = -1;
    tensor->dimValues[TIDL_DIM_DIM1]   = -1;
    tensor->dimValues[TIDL_DIM_BATCH]   = -1;
    tensor->bufferSize = -1;

    tidlrt_printf("TIDL_RT_OVX: Set default TIDLRT tensor done\n");
    return status;

}

static void get_time_u64(uint64_t *t)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    *t = ((uint64_t)ts.tv_sec * (uint64_t)1000000000ULL) + (uint64_t)ts.tv_nsec;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static void segfault_handler.* <function end>
<justification start> The function is for segmentation fault handling only,
not encountered in real test case. It is just graceful exit mechanism.
<justification end> */
#if defined (HOST_EMULATION)
static void segfault_handler(int code)
{
    if (sigsev_deinit == 0)
    {
        tidlrt_printf("Segfault in TIDL-RT!. Calling appMemDeinit\n");
        (void)tidl_rt_ovx_DeInit(1);
        sigsev_deinit = 1;
    }
}
#endif

int32_t TIDLRT_create(sTIDLRT_Params_t *prms, void **handle)
{
    int32_t status = VX_SUCCESS;
    IntHandle_t *rtHandle = NULL;
    IntHandle_t *prevRtHandle = NULL;
    int32_t numSubgraphs = 1;
    int32_t netNumSubgraphs = 1;
    int32_t ioNumSubgraphs = 1;
    int32_t netPtrOffset[TIDLRT_MAX_HANDLES] = {0};
    int32_t netCapacity[TIDLRT_MAX_HANDLES];
    int32_t ioPtrOffset[TIDLRT_MAX_HANDLES] = {0};
    int32_t ioCapacity[TIDLRT_MAX_HANDLES];

    for (int32_t i = 0; i < TIDLRT_MAX_HANDLES; i = i + 1)
    {
        netCapacity[i] = prms->net_capacity;
    }
    for (int32_t i = 0; i < TIDLRT_MAX_HANDLES; i = i + 1)
    {
        ioCapacity[i] = prms->io_capacity;
    }

    void *origNetPtr = prms->netPtr;
    void *origIOPtr = prms->ioBufDescPtr;
    // Check if it is multiple subgraphs and get the offsets
    /**
     * Expected format for custom net.bin containing multiple subgraphs
     *
     * |<TIDL_NET_VERSION> <MAGIC_NUMBER> <NUM SUBGRAPHS> <SUBGRAPH_0 OFFSET>  |
     * |<SUBGRAPH_0 NET CAPACITY> ..... <SUBGRAPH_0 NET> <SUBGRAPH_1 NET> .....|
     */
    if(*(((uint32_t *)prms->netPtr) + OFFSET_TO_MULTI_SUBGRAPH_NET_MAGIC_NUMBER) == TIDL_NET_MULTI_SUBGRAPH_MAGIC_NUMBER)
    {
        prms->netPtr = (uint32_t *)prms->netPtr + OFFSET_TO_MULTI_SUBGRAPH_NET_MAGIC_NUMBER;
        uint32_t *headerPtr = (uint32_t *)prms->netPtr + 1;
        netNumSubgraphs = *headerPtr;
        headerPtr++;
        for (int32_t i = 0; i < netNumSubgraphs; i++)
        {
            netPtrOffset[i] = *headerPtr;
            headerPtr++;
            netCapacity[i] = *headerPtr;
            headerPtr++;
            headerPtr++; // Reserved
            headerPtr++; // Reserved
        }
    }

    if(prms->io_capacity > (int64_t)OFFSET_TO_MULTI_SUBGRAPH_IO_MAGIC_NUMBER)
    {
        /**
         * Expected format for custom io.bin containing multiple subgraphs
         *
         * |<COMBINED IOBUF> <MAGIC_NUMBER> <NUM SUBGRAPHS> <SUBGRAPH_0 OFFSET>          |
         * |<SUBGRAPH_0 IOBUF CAPACITY> ..... <SUBGRAPH_0 IOBUF> <SUBGRAPH_1 IOBUF> .....|
         */
        if(*((uint32_t *)(prms->ioBufDescPtr + OFFSET_TO_MULTI_SUBGRAPH_IO_MAGIC_NUMBER)) == TIDL_NET_MULTI_SUBGRAPH_MAGIC_NUMBER)
        {
            prms->ioBufDescPtr = prms->ioBufDescPtr + OFFSET_TO_MULTI_SUBGRAPH_IO_MAGIC_NUMBER;
            uint32_t *headerPtr = (uint32_t *)prms->ioBufDescPtr + 1;
            ioNumSubgraphs = *headerPtr;
            headerPtr++;
            for (int32_t i = 0; i < ioNumSubgraphs; i++)
            {
                ioPtrOffset[i] = *headerPtr;
                headerPtr++;
                ioCapacity[i] = *headerPtr;
                headerPtr++;
                headerPtr++; // Reserved
                headerPtr++; // Reserved
            }
        }
    }

    if (netNumSubgraphs != ioNumSubgraphs)
    {
        tidlrt_printf("TIDL_RT_OVX: ERROR: Number of subgraphs in net and io file differ\n");
        status = (int32_t)VX_FAILURE;
    }

    numSubgraphs = netNumSubgraphs;

    if(prms->stats != NULL)
    {
        get_time_u64(&prms->stats->proc_time_start);
    }

    for (int32_t i = 0; ((status != (int32_t)VX_FAILURE) && (i < numSubgraphs)); i++)
    {
        AppObj *obj = NULL;

        prms->netPtr = (void *)prms->netPtr + netPtrOffset[i];
        prms->net_capacity = netCapacity[i];

        prms->ioBufDescPtr = (void *)prms->ioBufDescPtr + ioPtrOffset[i];
        prms->io_capacity = ioCapacity[i];
            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TIOVX_ALLOC_FAILURE
             * forces tidlrt_alloc_handle returns null rtHandle
             */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TIOVX_ALLOC_FAILURE)
            {
              for (int32_t j = 0; j <TIDLRT_MAX_HANDLES; j++)
              {
                tidlrt_handles_state[j] = 1;
              }
            }
            /*
             * FORCE NEGATIVE TEST END
             */
        rtHandle = (IntHandle_t*)tidlrt_alloc_handle();

        // Create AppObj
        if(rtHandle != NULL)
        {
            rtHandle->next = NULL;
            if(prevRtHandle != NULL)
            {
                prevRtHandle->next = rtHandle;
            }

            obj = &(rtHandle->gAppObj);

            // Copy values from user provided params
            obj->traceLogLevel = prms->traceLogLevel;
            obj->traceWriteLevel = prms->traceWriteLevel;
            if(prms->traceBaseName[0] != '\0')
            {
                if(strcmp(prms->traceSubgraphName, "") != 0)
                {
                    char traceBaseNameTmp[(TIDLRT_STRING_SIZE * 2) + 2];
                    (void)snprintf(traceBaseNameTmp, ((TIDLRT_STRING_SIZE * 2) + 2), "%s_%s_", prms->traceBaseName, prms->traceSubgraphName);
                    (void)strncpy(prms->traceBaseName, traceBaseNameTmp, TIDLRT_STRING_SIZE);
                }
                (void)strncpy(obj->traceBaseName, prms->traceBaseName, TIDLRT_STRING_SIZE);
            }
            else
            {
                (void)strncpy(obj->traceBaseName, "/tmp/tidl_trace", TIDLRT_STRING_SIZE);
                (void)strncpy(prms->traceBaseName, "/tmp/tidl_trace", TIDLRT_STRING_SIZE);
            }
            obj->maxPreEmptDelay = prms->maxPreEmptDelay;
            obj->targetPriority = prms->targetPriority;
            obj->coreNum = prms->coreNum;
            obj->coreStartIdx = prms->coreStartIdx;

            init_rt_profile(&obj->profilePoints, obj->traceLogLevel);

            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_TIDLRT_CREATE);

            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            tidlrt_printf("TIDL_RT_OVX: ERROR: Unable to allocate memory for TIDL RT handle\n");
            status = (vx_status)VX_FAILURE;
        }

        // App Init
        if(((int32_t)VX_SUCCESS == status) && (i == 0))
        {
            // Call appInit only for first iteration
            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_TIDL_RT_OVX_INIT);
            #if defined (HOST_EMULATION) && !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY)
            if(((sTIDL_Network_t*)prms->netPtr)->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
            {
                /* Set path to redirect temporary buffers */
                status = appMemSetFdPath((char*)prms->tempBufferDir, (uint32_t)(strlen((char *)(prms->tempBufferDir))));
                if(status != 0)
                {
                    tidlrt_printf("TIDL_RT_OVX: WARNING: appMemSetFdPath failed, temporary buffers will be directed to /dev/shm\n");
                }
            }
            #endif

            // Init
            status = tidl_rt_ovx_Init();

#if defined (HOST_EMULATION)
            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TIOVX_INIT_FAILURE
             * forces appInit again to check if it is skipped in multiple calls
             * and also forces appDeInit multiple times to check if it is
             * happens properly
             */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TIOVX_INIT_FAILURE)
            {
                // Deinit is handled by tb by default
                (void)tidl_rt_ovx_Init();
                (void)tidl_rt_ovx_DeInit(0);
                (void)tidl_rt_ovx_DeInit(0);
                (void)tidl_rt_ovx_DeInit(0);
                status = (int32_t)VX_FAILURE;
            }
            /*
             * FORCE NEGATIVE TEST END
             */
#endif
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if((int32_t)VX_SUCCESS != status)
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: appInit failed\n");
            }
            /* LDRA_JUSTIFY_END */
            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_TIDL_RT_OVX_INIT);
        }

        // Create context
        if ((int32_t)VX_SUCCESS == status)
        {
            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_VXCREATECONTEXT);

            obj->context = vxCreateContext();

            /*
            * FORCE NEGATIVE TEST START
            *
            * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CONTEXT_FAILURE
            * forces release of the context leading to VX_FAILURE in
            * tidlrt_check_assert_status
            */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CONTEXT_FAILURE)
            {
                (void)vxReleaseContext(&obj->context);
            }
            /*
            * FORCE NEGATIVE TEST END
            */

            status = tidlrt_check_assert_status(vxCastRefFromContext(obj->context));
            if((int32_t)VX_SUCCESS != status)
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Create context failed\n");
            }

            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_VXCREATECONTEXT);
        }

        // Init TIDL
        if ((int32_t)VX_SUCCESS == status)
        {
            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_INIT_TIDL_TIOVX);

            status = init_tidl_tiovx(obj,prms);
            if((int32_t)VX_SUCCESS != status)
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Init TIDL failed\n");
            }

            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_INIT_TIDL_TIOVX);
        }

        // Create Graph
        if ((int32_t)VX_SUCCESS == status)
        {

            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_CREATE_GRAPH_TIDL_TIOVX);

            /*
            * FORCE NEGATIVE TEST START
            *
            * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_SHUFFLE_KERNEL_PARAMETER_FAILURE
            * forces shuffling of params in the user kernel leading to mismatch
            * kernel params chronology
            */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_SHUFFLE_KERNEL_PARAMETER_FAILURE)
            {
                status = tidlrt_force_shuffle_kernel_params(obj);
                status = (int32_t)VX_FAILURE;
            }
            /*
            * FORCE NEGATIVE TEST END
            */

            /*
            * FORCE NEGATIVE TEST START
            *
            * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CREATE_NODE_AND_GRAPH_FAILURE
            * forces release of the user kernel and eventually the creation of node
            * and the graph, resulting in VX_FAILURE during create_graph_tidl_tiovx
            */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CREATE_GRAPH_FAILURE)
            {
                (void)vxReleaseContext(&obj->context);
            }
            /*
            * FORCE NEGATIVE TEST END
            */

            /*
            * FORCE NEGATIVE TEST START
            *
            * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CREATE_NODE_AND_GRAPH_FAILURE
            * forces release of the user kernel and eventually the creation of node
            * and the graph, resulting in VX_FAILURE during create_graph_tidl_tiovx
            */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CREATE_NODE_AND_GRAPH_FAILURE)
            {
                (void)vxReleaseKernel(&obj->kernel);
            }
            /*
            * FORCE NEGATIVE TEST END
            */

            status = create_graph_tidl_tiovx(obj);
            if((int32_t)VX_SUCCESS != status)
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Create OpenVX graph failed\n");
            }

            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_CREATE_GRAPH_TIDL_TIOVX);
        }

        // Verify Graph
        if ((int32_t)VX_SUCCESS == status)
        {
            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_VERIFY_GRAPH_TIDL_TIOVX);

            /*
            * FORCE NEGATIVE TEST START
            *
            * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_VERIFY_FAILURE
            * forces release of graph leading verify_graph_tidl_tiovx to fail
            */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_VERIFY_FAILURE)
            {
                (void)vxReleaseGraph(&obj->graph);
            }
            /*
            * FORCE NEGATIVE TEST END
            */

            status = verify_graph_tidl_tiovx(obj);
            if((int32_t)VX_SUCCESS != status)
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Verify OpenVX graph failed\n");
            }

            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_VERIFY_GRAPH_TIDL_TIOVX);
        }

        /*
         * Release  network user data object as part of init optimization
         * since it is not needed after Create call is complete
         */
        if (((int32_t)VX_SUCCESS == status) && (((sTIDL_Network_t *)(prms->netPtr))->netInitBackupDataOffset[0] == 0))
        {
            void       *ref1Addr[1]; /* Single user data object */
            uint32_t    size[1];
            uint32_t    numPlanes;

            /*
            * FORCE NEGATIVE TEST START
            *
            * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXPORT_NETWORK_FAILURE
            * forces release of network user data obj and leading to VX_FAILURE
            * in tivxReferenceExportHandle
            */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXPORT_NETWORK_FAILURE)
            {
                (void)vxReleaseUserDataObject(&obj->network);
            }
            /*
            * FORCE NEGATIVE TEST END
            */

            status = tivxReferenceExportHandle(vxCastRefFromUserDataObject(obj->network),
                                                            ref1Addr,
                                                            size,
                                                            1,
                                                            &numPlanes);
            if (status == (int32_t)VX_SUCCESS)
            {
                /*
                * FORCE NEGATIVE TEST START
                *
                * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_FREE_NET_FAILURE
                * forces ref1Addr[0] to be an invalid pointer (prms here)
                * leading to VX_FAILURE in tivxMemFree
                */
                if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_FREE_NET_FAILURE)
                {
                    ref1Addr[0]=prms;
                }
                /*
                * FORCE NEGATIVE TEST END
                */

                status = tivxMemFree(ref1Addr[0], size[0], (vx_enum)TIVX_MEM_EXTERNAL);
                ref1Addr[0] = NULL;

                if (status == (int32_t)VX_SUCCESS)
                {
                    /*
                    * FORCE NEGATIVE TEST START
                    *
                    * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_NET_BUF_FAILURE
                    * forces release of network user data obj and leading to
                    * VX_FAILURE in tivxReferenceImportHandle
                    */
                    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_NET_BUF_FAILURE)
                    {
                        (void)vxReleaseUserDataObject(&obj->network);
                    }
                    /*
                    * FORCE NEGATIVE TEST END
                    */

                    status = tivxReferenceImportHandle(vxCastRefFromUserDataObject(obj->network),
                                                            (const void **)ref1Addr,
                                                            size,
                                                            numPlanes);

                    if (status == (int32_t)VX_SUCCESS)
                    {
                        /*
                        * FORCE NEGATIVE TEST START
                        *
                        * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_VX_NET_FAILURE
                        * forces release of network user data obj and leading
                        * to VX_FAILURE on further realease of same object
                        */

                        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_VX_NET_FAILURE)
                        {
                            (void)vxReleaseUserDataObject(&obj->network);
                        }
                        /*
                        * FORCE NEGATIVE TEST END
                        */

                        status = vxReleaseUserDataObject(&obj->network);
                        if (status != (int32_t)VX_SUCCESS)
                        {
                            tidlrt_printf("TIDL_RT_OVX: ERROR! Unable to release Network user data object! \n");
                        }

                        obj->network = NULL;
                        tidlrt_printf("TIDL_RT_OVX: INFO: TIOVX Network buffer is copied to C7x heap and freed in shared memory!\n");
                    }
                    else
                    {
                        tidlrt_printf("TIDL_RT_OVX: Unable to import NULL handle! network buffer not released! \n");
                    }
                }
                else
                {
                    tidlrt_printf("TIDL_RT_OVX: Unable to free network handle! network buffer not released! \n");
                }
            }
            else
            {
                tidlrt_printf("TIDL_RT_OVX: Unable to export network handle! network buffer not released! \n");
            }

            status = (int32_t)VX_SUCCESS;
        }

        // Release input/output tensors at create if enables
        if(((int32_t)VX_SUCCESS == status) && (prms->releaseIOTensorsAtCreate == 1))
        {
            void       *ref1Addr[1]; /* Single user data object */
            uint32_t    size[1];
            uint32_t    numPlanes;
            uint32_t    id;

            /* Release the input and output tensor buffers allocated during graph verify stage */
            for(id = 0; id < obj->num_input_tensors; id++)
            {
                /*
                * FORCE NEGATIVE TEST START
                *
                * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXPORT_TENSOR_BUFFER_FAILURE
                * forces release of tensor leading to VX_FAILURE during
                * tivxReferenceExportHandle
                */
                if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXPORT_TENSOR_BUFFER_FAILURE)
                {
                    (void)vxReleaseTensor(&obj->input_tensors[id]);
                }
                /*
                * FORCE NEGATIVE TEST END
                */

                status = tivxReferenceExportHandle(vxCastRefFromTensor(obj->input_tensors[id]),
                                                    ref1Addr,
                                                    size,
                                                    MAX_TENSOR_DIMS,
                                                    &numPlanes);

                tidlrt_printf("TIDL_RT_OVX: input tensor buffer ref1Addr[0] = %p %d\n", ref1Addr[0], size[0]);

                if (status == (int32_t)VX_SUCCESS)
                {
                    /*
                    * FORCE NEGATIVE TEST START
                    *
                    * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_FREE_TENSOR_BUF_FAILURE
                    * forces ref1Addr[0] to be an invalid pointer (prms here)
                    * leading to VX_FAILURE in tivxMemFree
                    */
                    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_FREE_TENSOR_BUF_FAILURE)
                    {
                        ref1Addr[0]=prms;
                    }
                    /*
                    * FORCE NEGATIVE TEST END
                    */

                    status = tivxMemFree(ref1Addr[0], size[0], (vx_enum)TIVX_MEM_EXTERNAL);

                    if (status == (int32_t)VX_SUCCESS)
                    {

                        /*
                        * FORCE NEGATIVE TEST START
                        *
                        * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TENSOR_BUF_FAILURE
                        * forces release of input tensor buffer leading to
                        * VX_FAILURE during tivxReferenceImportHandle
                        */
                        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TENSOR_BUF_FAILURE)
                        {
                            (void)vxReleaseTensor(&obj->input_tensors[id]);
                        }
                        /*
                        * FORCE NEGATIVE TEST END
                        */

                        ref1Addr[0] = NULL;
                        status = tivxReferenceImportHandle(vxCastRefFromTensor(obj->input_tensors[id]),
                                                            (const void **)ref1Addr,
                                                            size,
                                                            numPlanes);
                        if (status == (int32_t)VX_SUCCESS)
                        {
                            tidlrt_printf("TIDL_RT_OVX: INFO: Input buffer allocated during graph verify is released in TIDLRT_create()!\n");
                        }
                        else
                        {
                            tidlrt_printf("TIDL_RT_OVX: Unable to import NULL handle! input buffer[%d] not released! \n", id);
                        }
                    }
                    else
                    {
                        tidlrt_printf("TIDL_RT_OVX: Unable to free input buffer handle! input buffer[%d] not released! \n", id);
                    }
                }
                else
                {
                    tidlrt_printf("TIDL_RT_OVX: Unable to export input buffer handle! input buffer[%d] not released! \n", id);
                }
            }

            for(id = 0; id < obj->num_output_tensors; id++)
            {
                /*
                * FORCE NEGATIVE TEST START
                *
                * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXPORT_TENSOR_BUFFER_FAILURE
                * forces release of tensor leading to VX_FAILURE in
                * tivxReferenceExportHandle
                */
                if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXPORT_TENSOR_BUFFER_FAILURE)
                {
                    (void)vxReleaseTensor(&obj->output_tensors[id]);
                }
                /*
                * FORCE NEGATIVE TEST END
                */

                status = tivxReferenceExportHandle(vxCastRefFromTensor(obj->output_tensors[id]),
                                                        ref1Addr,
                                                        size,
                                                        MAX_TENSOR_DIMS,
                                                        &numPlanes);

                tidlrt_printf("TIDL_RT_OVX: output tensor buffer ref1Addr[0] = %p %d\n", ref1Addr[0], size[0]);

                if (status == (int32_t)VX_SUCCESS)
                {
                    /*
                    * FORCE NEGATIVE TEST START
                    *
                    * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_FREE_TENSOR_BUF_FAILURE
                    * forces ref1Addr[0] to be an invalid pointer (prms here)
                    * leading to VX_FAILURE in tivxMemFree
                    */
                    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_FREE_TENSOR_BUF_FAILURE)
                    {
                        ref1Addr[0]=prms;
                    }
                    /*
                    * FORCE NEGATIVE TEST END
                    */

                    status = tivxMemFree(ref1Addr[0], size[0], (vx_enum)TIVX_MEM_EXTERNAL);

                    if (status == (int32_t)VX_SUCCESS)
                    {
                        /*
                        * FORCE NEGATIVE TEST START
                        *
                        * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TENSOR_BUF_FAILURE
                        * forces release of output tensor buffer leading to
                        * VX_FAILURE in tivxReferenceImportHandle
                        */
                        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TENSOR_BUF_FAILURE)
                        {
                            (void)vxReleaseTensor(&obj->output_tensors[id]);
                        }
                        /*
                        * FORCE NEGATIVE TEST END
                        */
                        ref1Addr[0] = NULL;
                        status = tivxReferenceImportHandle(vxCastRefFromTensor(obj->output_tensors[id]),
                                                        (const void **)ref1Addr,
                                                        size,
                                                        numPlanes);
                        if (status == (int32_t)VX_SUCCESS)
                        {
                            tidlrt_printf("TIDL_RT_OVX: INFO: Output buffer allocated during graph verify is released in TIDLRT_create()!\n");
                        }
                        else
                        {
                            tidlrt_printf("TIDL_RT_OVX: Unable to import NULL handle! output buffer[%d] not released! \n", id);
                        }
                    }
                    else
                    {
                        tidlrt_printf("TIDL_RT_OVX: Unable to free output buffer handle! output buffer[%d] not released! \n", id);
                    }
                }
                else
                {
                    tidlrt_printf("TIDL_RT_OVX: Unable to export output buffer handle! output buffer[%d] not released! \n", id);
                }
            }

            status = (int32_t)VX_SUCCESS;
        }

        if((int32_t)VX_SUCCESS == status)
        {
#if !defined (HOST_EMULATION)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
            Users aren't expected to set TIDL_RT_DEBUG env variable,
            only for debug purposes.
            TIDL_LDRA_TAG: TIDL_LDRA_TAG_TIDL_RT_DEBUG_ENV_VAR
            Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
            However, due to the stated rationale, this is not covered in production code.
            <justification end> */
            if(tidlrt_debuglevel == 3U)
            {
                tidlrt_printf("******** Printing stats at the end of TIDLRT_create call ****** \n");
                #ifndef SOC_TDA54
                    (void)appPerfStatsPrintAll();
                #endif
            }
            /* LDRA_JUSTIFY_END */
#endif
            rtHandle->rtPrms = *prms;
            prevRtHandle = rtHandle;

            // Set handle to first rtHandle
            if(i == 0)
            {
                *handle = rtHandle;
            }

            rtHandle->numIntermediateIn = 0;
            rtHandle->numIntermediateOut = 0;
            for (int32_t j = 0; j < TIDL_MAX_INTERMEDIATE_TENSOR; j++)
            {
                rtHandle->intermediateInTensorAllocated[j] = 0;
                rtHandle->intermediateOutTensorAllocated[j] = 0;
            }

            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_TIDLRT_CREATE);
            printProfileInfo(&obj->profilePoints);
        }
        else
        {
            if(i == 0)
            {
                /*
                 * Setting handle even if status fails so the caller can still
                 * call TIDLRT_deactivate and TIDLRT_delete functions to delete
                 * any allocated buffers
                 */
                *handle = rtHandle;
            }
        }
    }

    // Allocate intermediate output tensor if multiple subgraphs
    if(((int32_t)VX_SUCCESS == status) && (numSubgraphs > 1))
    {
        status = allocate_intermediate_tensors((IntHandle_t *)*handle);
        if((int32_t)VX_SUCCESS != status)
        {
            tidlrt_printf("TIDL_RT_OVX: ERROR: Intermediate tensor allocation failed\n");
        }
    }

    if(prms->stats != NULL)
    {
        get_time_u64(&prms->stats->proc_time_end);
    }

    prms->netPtr = origNetPtr;
    prms->ioBufDescPtr = origIOPtr;

    return status;
}

int32_t TIDLRT_invoke(void *handle, sTIDLRT_Tensor_t *in[], sTIDLRT_Tensor_t *out[])
{
#if defined (HOST_EMULATION)
    (void)signal(SIGSEGV, segfault_handler);
#endif
    int32_t status = (int32_t) VX_SUCCESS;
    IntHandle_t * rtHandle = (IntHandle_t*) handle;

    if(rtHandle != NULL)
    {
        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PROCESS_FAILURE
         * forces release of graph leading to VX_FAILURE during graph processing
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PROCESS_FAILURE)
        {
            (void)vxReleaseGraph(&rtHandle->gAppObj.graph);
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_INPUT_TENSORS
         * forces release of all input tensors leading to failure in
         * setting and copying of input tensors
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_INPUT_TENSORS)
        {
            for(uint32_t i = 0; i < rtHandle->gAppObj.num_input_tensors; i++)
            {
                (void)vxReleaseTensor(&rtHandle->gAppObj.input_tensors[i]);
                rtHandle->gAppObj.input_tensors[i] = NULL;
            }
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_OUTPUT_TENSORS
         * forces release of all output tensors leading to setting and memcpy
         * of output tensors to fail
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_OUTPUT_TENSORS)
        {
            for(uint32_t i = 0; i < rtHandle->gAppObj.num_output_tensors; i++)
            {
                (void)vxReleaseTensor(&rtHandle->gAppObj.output_tensors[i]);
                rtHandle->gAppObj.output_tensors[i] = NULL;
            }
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OUTARGS_NULL
         * forces release of outArgs object leads to failure of outArgs for
         * debug trace level processing
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OUTARGS_NULL)
        {
            (void)vxReleaseUserDataObject(&rtHandle->gAppObj.outArgs);
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_BASE_NAME_NULL
         * forces traceBaseName to be "" for covering branches which avoid
         * writing to the perf file in TIDLRT_LogMetaData in
         * map_cp_out_tensor_tidlrt_tiovx after processing graph
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_BASE_NAME_NULL)
        {
            AppObj *obj = &(rtHandle->gAppObj);
            (void)strcpy(obj->traceBaseName,"");
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        if(rtHandle->next == NULL)
        {
            status = run_graph_tidlrt_tiovx(&(rtHandle->gAppObj), in, out, rtHandle->rtPrms.stats);
        }
        else
        {
            sTIDLRT_Tensor_t *in_tensor[TIDL_MAX_ALG_IN_BUFS];
            sTIDLRT_Tensor_t *out_tensor[TIDL_MAX_ALG_OUT_BUFS];
            int32_t i_count = 0;
            int32_t o_count = 0;
            /** TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_002 */
            while((rtHandle != NULL) && (status == (int32_t)VX_SUCCESS))
            {
                AppObj *obj = &(rtHandle->gAppObj);
                sTIDL_IOBufDesc_t *ioBufDesc = &(obj->ioBufDesc);

                for (int32_t i = 0; i < ioBufDesc->numInputBuf; i++)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
                    TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
                    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                    This does not affect runtime behavior or safety.
                    <justification end> */
                    if (i >= TIDL_MAX_INTERMEDIATE_TENSOR)
                    {
                        tidlrt_printf("TIDL_RT_OVX: ERROR: Not enough intermediate input tensors\n");
                        status = (int32_t)VX_FAILURE;
                        break;
                    }
                    /* LDRA_JUSTIFY_END */

                    // If intermediate input is not allocated, pick from in
                    if(rtHandle->intermediateInTensorAllocated[i] == 0)
                    {
                        int32_t id = i_count;
                        in_tensor[i] = in[id];
                        i_count++;
                    }
                    else
                    {
                        in_tensor[i] = &(rtHandle->intermediateInTensor[i]);
                    }
                }

                for (int32_t i = 0; i < ioBufDesc->numOutputBuf; i++)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
                    TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
                    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                    This does not affect runtime behavior or safety.
                    <justification end> */
                    if (i >= TIDL_MAX_INTERMEDIATE_TENSOR)
                    {
                        tidlrt_printf("TIDL_RT_OVX: ERROR: Not enough intermediate output tensors\n");
                        status = (int32_t)VX_FAILURE;
                        break;
                    }
                    /* LDRA_JUSTIFY_END */

                    // If intermediate output is not alocated, pick from out
                    if(rtHandle->intermediateOutTensorAllocated[i] == 0)
                    {
                        int32_t id = o_count;
                        out_tensor[i] = out[id];
                        o_count++;
                    }
                    else
                    {
                        out_tensor[i] = &(rtHandle->intermediateOutTensor[i]);
                    }
                }

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
                TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
                TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_002
                Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                This does not affect runtime behavior or safety.
                <justification end> */
                if (status == (int32_t)VX_SUCCESS)
                {
                    status = run_graph_tidlrt_tiovx(obj, in_tensor, out_tensor, rtHandle->rtPrms.stats);
                    rtHandle = (IntHandle_t*)rtHandle->next;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }
    else
    {
        status = (int32_t)VX_FAILURE;
    }

    return status;
}



int32_t TIDLRT_delete(void *handle)
{
    int32_t status = VX_SUCCESS;
    IntHandle_t * rtHandle = (IntHandle_t*)handle;
    tidlrt_printf("TIDL_RT_OVX: Delete ... \n");

    while(rtHandle != NULL)
    {
        for(int32_t i = 0; i < rtHandle->gAppObj.ioBufDesc.numOutputBuf; i++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
            This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
            TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
            Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
            This does not affect runtime behavior or safety.
            <justification end> */
            if (((IntHandle_t*)rtHandle->next != NULL) && (i >= TIDL_MAX_INTERMEDIATE_TENSOR))
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Not enough intermediate output tensors\n");
                break;
            }
            /* LDRA_JUSTIFY_END */

            if(rtHandle->intermediateOutTensorAllocated[i] == 1)
            {
                sTIDLRT_Tensor_t *t = &(rtHandle->intermediateOutTensor[i]);
                TIDLRT_freeSharedMem(t->ptr);
            }
        }

        delete_graph_tidl_tiovx(&(rtHandle->gAppObj));

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_LOAD_UNLOAD_KERNELS
         * forces multiple unloads of kernels for null checks and non-multiple
         * unloading of kernels
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_LOAD_UNLOAD_KERNELS)
        {
            tivxTIDLUnLoadKernels(NULL);
            tivxTIDLUnLoadKernels(rtHandle->gAppObj.context);
            tivxTIDLUnLoadKernels(rtHandle->gAppObj.context);
            tivxTIDLUnLoadKernels(rtHandle->gAppObj.context);
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_TENSORS_AFTER_PROCESS
         * forces release of all tensors after graph processing leading
         * to failure in release of tensors in deinit_tidl_tiovx
         */
        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_TENSORS_AFTER_PROCESS)
        {
            rtHandle->gAppObj.network = vxCreateUserDataObject(rtHandle->gAppObj.context, "TIDL_Network", 1, NULL);
            for(uint32_t i = 0; i < rtHandle->gAppObj.num_input_tensors; i++)
            {
                (void)vxReleaseTensor(&rtHandle->gAppObj.input_tensors[i]);
                rtHandle->gAppObj.input_tensors[i] = NULL;
            }
            for(uint32_t i = 0; i < rtHandle->gAppObj.num_output_tensors; i++)
            {
                (void)vxReleaseTensor(&rtHandle->gAppObj.output_tensors[i]);
                rtHandle->gAppObj.output_tensors[i] = NULL;
            }
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        status = deinit_tidl_tiovx(&(rtHandle->gAppObj), &rtHandle->rtPrms);

        if(status == (int32_t)VX_SUCCESS)
        {
            (void)vxReleaseContext(&rtHandle->gAppObj.context);

            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_HANDLE_FAILURE
             * forces tidlrt_free_handle to check for NULL and invalid pointers
             */
            if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_HANDLE_FAILURE)
            {
                // Passing NULL pointer
                (void)tidlrt_free_handle(NULL);

                // Passing random invalid pointer
                (void)tidlrt_free_handle(&rtHandle->rtPrms);
            }
            /*
             * FORCE NEGATIVE TEST END
             */

            (void)tidlrt_free_handle((void *)rtHandle);
        }
        rtHandle = (IntHandle_t*)rtHandle->next;
    }

    if(status == (int32_t)VX_SUCCESS)
    {
        (void)tidl_rt_ovx_DeInit(0);
    }

    tidlrt_printf("TIDL_RT_OVX: Delete ... Done \n");
    return status;
}

int32_t TIDLRT_deactivate(void *handle)
{
    int32_t status = VX_SUCCESS;
    return status;
}

static vx_status init_tidl_tiovx(AppObj *obj,sTIDLRT_Params_t *prms)
{

    vx_status status = VX_SUCCESS;

    uint32_t num_input_tensors = 0;
    uint32_t num_output_tensors = 0;
    int32_t inferenceMode = TIDL_inferenceModeDefault;

    tidlrt_printf("TIDL_RT_OVX: Init ... \n");

    TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_TIVX_TIDL_LOAD_KERNEL); //rtHandle not passed here...

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_LOAD_UNLOAD_KERNELS
     * forces null check and non-multiple reloads of kernels in
     * tivxTIDLLoadKernels
     */
    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_LOAD_UNLOAD_KERNELS)
    {
        tivxTIDLLoadKernels(NULL);
        tivxTIDLLoadKernels(obj->context);
    }
    /*
     * FORCE NEGATIVE TEST END
     */

    tivxTIDLLoadKernels(obj->context);
    TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_TIVX_TIDL_LOAD_KERNEL);

    /* Create a vx_array object and map the config data*/
    TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_MAPCONFIG);

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CONFIG_FAILURE
     * forces release of context leading to VX_FAILURE during creation of
     * config user data object in mapConfig
     */
    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CONFIG_FAILURE)
    {
        obj->kernel = tivxAddKernelTIDL(obj->context, 0, 0);
        (void)vxReleaseContext(&obj->context);
    }
    if (prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CAPACITY_ZERO)
    {
        prms->io_capacity = 0;
    }
    /*
     * FORCE NEGATIVE TEST END
     */
    obj->config = mapConfig(obj, obj->context, prms, &num_input_tensors, &num_output_tensors, &inferenceMode);
    status = tidlrt_check_assert_status(vxCastRefFromUserDataObject(obj->config));
    if(status == (int32_t)VX_SUCCESS)
    {
        TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_MAPCONFIG);

        TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_TIVXADDKERNELTIDL);

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_KERNEL_FAILURE
         * forces creation of kernel with invalid parameters and releases
         * context in valid case leading to VX_FAILURE in tivxAddKernelTIDL
         */
        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_KERNEL_FAILURE)
        {
            (void)vxReleaseContext(&obj->context);
        }
        /*
         * FORCE NEGATIVE TEST END
         */
        
        #if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
        if((num_input_tensors + num_output_tensors) > (TIVX_KERNEL_MAX_PARAMS - TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS)) 
        {
            obj->kernel = NULL;
            obj->is_tidl_node_tensor_list = true;
        }
        else
        #endif
        {
            obj->input_tensor_list = NULL;
            obj->output_tensor_list = NULL;
            obj->is_tidl_node_tensor_list = false;
            obj->kernel = tivxAddKernelTIDL(obj->context, num_input_tensors, num_output_tensors);
            status = tidlrt_check_assert_status(vxCastRefFromKernel(obj->kernel));
        }


        if(status == (int32_t)VX_SUCCESS)
        {
            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_TIVXADDKERNELTIDL);

            if(obj->kernel!=NULL)
            {
                tidlrt_printf("Host kernel - %p \n", (vx_kernel *)&obj->kernel);
            }

            /* Save a copy of number of input/output tensors required as per config */
            obj->num_input_tensors  = num_input_tensors;
            obj->num_output_tensors = num_output_tensors;

            /* Create a vx_tensor object and map the network data */

            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_NETWORK_FAILURE
             * forces release of context leading to VX_FAILURE during
             * creation of network user data object in mapNetwork
             */
            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_NETWORK_FAILURE)
            {
                (void)vxReleaseContext(&obj->context);
            }
            /*
             * FORCE NEGATIVE TEST END
             */

            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_MAPNETWORK);
            obj->network = mapNetwork(obj, prms->netPtr, prms->net_capacity);
            status = tidlrt_check_assert_status(vxCastRefFromUserDataObject(obj->network));
            if(status == (int32_t)VX_SUCCESS)
            {
                TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_MAPNETWORK);
                /*
                 * FORCE NEGATIVE TEST START
                 *
                 * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CREATE_PARAMS_FAILURE
                 * forces release of context leading to VX_FAILURE
                 * during creation of createParams user data object in
                 * setCreateParams
                 */
                if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CREATE_PARAMS_FAILURE)
                {
                    (void)vxReleaseContext(&obj->context);
                }
                /*
                 * FORCE NEGATIVE TEST END
                 */

                TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_SETCREATEPARAMS);
                obj->createParams = setCreateParams(obj, prms, inferenceMode);
                TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_SETCREATEPARAMS);
                status = tidlrt_check_assert_status(vxCastRefFromUserDataObject(obj->createParams));
                if(status == (int32_t)VX_SUCCESS)
                {
                    TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_SETARGS);

                    /*
                     * FORCE NEGATIVE TEST START
                     *
                     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INARGS_FAILURE
                     * forces release of context leading to VX_FAILURE
                     * during creation of inArgs user data object in
                     * setInArgs
                     */
                    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INARGS_FAILURE)
                    {
                        (void)vxReleaseContext(&obj->context);
                    }
                    /*
                     * FORCE NEGATIVE TEST END
                     */

                    obj->inArgs = setInArgs(obj);
                    status = tidlrt_check_assert_status(vxCastRefFromUserDataObject(obj->inArgs));
                    if(status == (int32_t)VX_SUCCESS)
                    {

                        /*
                         * FORCE NEGATIVE TEST START
                         *
                         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OUTARGS_FAILURE
                         * forces release of context leading to VX_FAILURE
                         * during creation of outArgs user data object in
                         * setInArgs
                         */
                        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OUTARGS_FAILURE)
                        {
                            (void)vxReleaseContext(&obj->context);
                        }
                        /*
                         * FORCE NEGATIVE TEST END
                         */

                        obj->outArgs = setOutArgs(obj->context);
                        status = tidlrt_check_assert_status(vxCastRefFromUserDataObject(obj->outArgs));
                        if(status == (int32_t)VX_SUCCESS)
                        {
                            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_SETARGS);
                            if(obj->traceWriteLevel != 0U)
                            {
                                TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_VX_CREATE_USER_DATA_OBJECT);

                                size_t traceDataObjSize = TIVX_TIDL_TRACE_DATA_SIZE;

                                /*
                                 * FORCE NEGATIVE TEST START
                                 *
                                 * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_DATA_FAILURE
                                 * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OUTARGS_FAILURE
                                 * forces release of context leading to VX_FAILURE
                                 * during creation of traceData user data object
                                 * in vxCreateUserDataObject
                                 */
                                if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_DATA_FAILURE)
                                {
                                    (void)vxReleaseContext(&obj->context);
                                }
                                /*
                                 * FORCE NEGATIVE TEST END
                                 */

                                /*
                                 * FORCE NEGATIVE TEST START
                                 *
                                 * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_DATA_SIZE_1
                                 * forces creation of traceData object with an
                                 * invalid size leading to failure in post process
                                 * while writing to trace data object
                                 */
                                if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_DATA_SIZE_1)
                                {
                                    traceDataObjSize = 1;
                                }
                                /*
                                 * FORCE NEGATIVE TEST END
                                 */

                                obj->traceData = vxCreateUserDataObject(obj->context, "TIDL_traceData", traceDataObjSize, NULL);

                                status = tidlrt_check_assert_status(vxCastRefFromUserDataObject(obj->traceData));
                            }
                            else
                            {
                                obj->traceData = NULL;
                            }

                            if(status == (int32_t)VX_SUCCESS)
                            {
                                TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_VX_CREATE_USER_DATA_OBJECT);
                                tidlrt_printf("TIDL_RT_OVX: Init done.\n");
                            }
                        }
                    }
                }
            }
        }
    }

    return status;
}

static int32_t deinit_tidl_tiovx(AppObj *obj, const sTIDLRT_Params_t *prms)
{
    uint32_t id;
    void       *ref1Addr[MAX_TENSOR_DIMS];
    uint32_t    size[MAX_TENSOR_DIMS];
    uint32_t    numPlanes;
    vx_status status = VX_SUCCESS;
    tidlrt_printf("TIDL_RT_OVX: DeInit ... \n");

    if(obj->traceWriteLevel != 0U)
    {
        (void)vxReleaseUserDataObject(&obj->traceData);
    }

    (void)vxReleaseUserDataObject(&obj->config);
    (void)vxReleaseUserDataObject(&obj->outArgs);
    (void)vxReleaseUserDataObject(&obj->inArgs);
    (void)vxReleaseUserDataObject(&obj->createParams);
    if (obj->network != NULL)
    {
        (void)vxReleaseUserDataObject(&obj->network);
    }

    /* Release input object array */
    if(obj->input_tensor_list != NULL) {
        (void)vxReleaseObjectArray(&obj->input_tensor_list);
    }
    
    /* Release output object array */
    if(obj->output_tensor_list != NULL) {
        (void)vxReleaseObjectArray(&obj->output_tensor_list);
    }

    for(id = 0; id < obj->num_input_tensors; id++) {
        if(obj->shared_input_tensors[id] == 1) {
            
            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INCORRECT_CONFIG_CHECKSUM
             * forces checksum for config to be incorrect
             */
            if (prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_TENSOR_BEFORE_DEINIT)
            {
                (void)vxReleaseTensor(&obj->input_tensors[id]);
            }
            /*
             * FORCE NEGATIVE TEST END
             */

            status = tivxReferenceExportHandle(vxCastRefFromTensor(obj->input_tensors[id]),
                                                    ref1Addr,
                                                    size,
                                                    MAX_TENSOR_DIMS,
                                                    &numPlanes);
            if(status == (int32_t)VX_SUCCESS){
                ref1Addr[0] = NULL;
                status = tivxReferenceImportHandle(vxCastRefFromTensor(obj->input_tensors[id]),
                                                (const void **)ref1Addr,
                                                size,
                                                numPlanes);
            }
        }
        (void)vxReleaseTensor(&obj->input_tensors[id]);
    }

    for(id = 0; id < obj->num_output_tensors; id++) {
         if(obj->shared_output_tensors[id] == 1) {

            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INCORRECT_CONFIG_CHECKSUM
             * forces checksum for config to be incorrect
             */
            if (prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_RELEASE_TENSOR_BEFORE_DEINIT)
            {
                (void)vxReleaseTensor(&obj->output_tensors[id]);
            }
            /*
             * FORCE NEGATIVE TEST END
             */

            status = tivxReferenceExportHandle(vxCastRefFromTensor(obj->output_tensors[id]),
                                                    ref1Addr,
                                                    size,
                                                    MAX_TENSOR_DIMS,
                                                    &numPlanes);
            if(status == (int32_t)VX_SUCCESS){
                ref1Addr[0] = NULL;
                status = tivxReferenceImportHandle(vxCastRefFromTensor(obj->output_tensors[id]),
                                                (const void **)ref1Addr,
                                                size,
                                                numPlanes);
            }
         }
         (void)vxReleaseTensor(&obj->output_tensors[id]);
    }

    tivxTIDLUnLoadKernels(obj->context);

    tidlrt_printf("TIDL_RT_OVX: DeInit... Done.\n");
    return status;
}

static void delete_graph_tidl_tiovx(AppObj *obj)
{
    tidlrt_printf("TIDL_RT_OVX: Delete TIDL graph ... \n");

    (void)vxDisableEvents(obj->context);
    (void)vxReleaseNode(&obj->tidl_node);
    if(obj->kernel != NULL)
    {
        (void)vxRemoveKernel(obj->kernel);
    }
    (void)vxReleaseGraph(&obj->graph);

    tidlrt_printf("TIDL_RT_OVX: Delete TIDL graph ... Done.\n");
}

static vx_user_data_object mapConfig(AppObj *obj, vx_context context, const sTIDLRT_Params_t *prms, uint32_t *num_input_tensors, uint32_t *num_output_tensors, int32_t * inferenceMode)
{
    vx_status status = VX_SUCCESS;

    tivxTIDLJ7Params  *tidlParams = NULL;
    sTIDL_IOBufDesc_t *ioBufDesc = NULL;
    vx_user_data_object   config = NULL;
    vx_map_id map_id;
    int32_t capacity = prms->io_capacity;
    void *ioBufDescPtr = prms->ioBufDescPtr;

    tidlrt_printf("TIDL_RT_OVX: Mapping config file ...\n");
    
    if( (uint32_t)capacity != sizeof(sTIDL_IOBufDesc_t) )
    {
        tidlrt_printf("TIDL_RT_OVX: ERROR: Config file size (%d bytes) does not match size of sTIDL_IOBufDesc_t (%d bytes)\n", capacity, (vx_uint32)sizeof(sTIDL_IOBufDesc_t));
        status = (vx_status)VX_FAILURE;
    }

    if ((int32_t)VX_SUCCESS == status)
    {
        /* Create a user struct type for handling config data*/
        config = vxCreateUserDataObject(context, "tivxTIDLJ7Params", sizeof(tivxTIDLJ7Params), NULL );
        status = vxGetStatus(vxCastRefFromUserDataObject(config));
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((int32_t)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id,
                (void **)&tidlParams, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        vxMapUserDataObject fails if vx_user_data_object provided is NULL.
        The vx_user_data_object provided here is config which is already
        checked for NULL in vxGetStatus
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((int32_t)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            tidlParams will never be null if the status above is VX_SUCCESS
            i.e config has been successfully mapped to tidlParams
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(tidlParams == NULL)
            {
				tidlrt_printf("TIDL_RT_OVX: ERROR: Map of config object failed\n");
				config = NULL;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            tidlParams will never be null if the status above is VX_SUCCESS
            i.e config has been successfully mapped to tidlParams
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
            /* LDRA_JUSTIFY_END */
                tivx_tidl_j7_params_init(tidlParams);
                tidlParams->optimize_ivision_activation = 1;
                ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

                (void)memcpy((void *)&obj->ioBufDesc, (void *)ioBufDescPtr, (size_t)capacity); //PC-- can memcpy be avoid here?
                *num_input_tensors  = obj->ioBufDesc.numInputBuf;
                *num_output_tensors = obj->ioBufDesc.numOutputBuf;
                *inferenceMode = obj->ioBufDesc.inferenceMode;

                (void)memcpy((void *)ioBufDesc, (void *)ioBufDescPtr, (size_t)capacity); //PC-- can memcpy be avoid here?

                // Compute checksum if enabled
                if(prms->computeChecksum != 0)
                {
                    sTIDL_Network_t *pNet = (sTIDL_Network_t *)prms->netPtr;
                    uint8_t *pPerfInfo = (uint8_t *)prms->netPtr + pNet->dataFlowInfo;
                    tidlParams->compute_config_checksum = 1;
                    tidlParams->compute_network_checksum = 1;
                    tivx_tidl_j7_get_qc((uint8_t *)&tidlParams->ioBufDesc, (uint8_t *)tidlParams->config_checksum, (int32_t)(sizeof(sTIDL_IOBufDesc_t)));
                    tivx_tidl_j7_get_qc((uint8_t *)pPerfInfo, (uint8_t *)tidlParams->network_checksum, (int32_t)(prms->net_capacity - pNet->dataFlowInfo));

                    /*
                     * FORCE NEGATIVE TEST START
                     *
                     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INCORRECT_CONFIG_CHECKSUM
                     * forces checksum for config to be incorrect
                     */
                    if (prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INCORRECT_CONFIG_CHECKSUM)
                    {
                        (void)memset(tidlParams->config_checksum, 0, TIVX_TIDL_J7_CHECKSUM_SIZE);
                    }
                    /*
                     * FORCE NEGATIVE TEST END
                     */

                    /*
                     * FORCE NEGATIVE TEST START
                     *
                     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INCORRECT_NETWORK_CHECKSUM
                     * forces checksum for network to be incorrect
                     */
                    if (prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INCORRECT_NETWORK_CHECKSUM)
                    {
                        (void)memset(tidlParams->network_checksum, 0, TIVX_TIDL_J7_CHECKSUM_SIZE);
                    }
                    /*
                     * FORCE NEGATIVE TEST END
                     */
                }

                /*
                * FORCE NEGATIVE TEST START
                *
                * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXCEED_MAX_KERNEL_PARAMS_FAILURE
                * forces total kernel parameters to exceed TIVX_KERNEL_MAX_PARAMS
                * which is checked in tivxTIDLNode
                */
                if (prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_EXCEED_MAX_KERNEL_PARAMS_FAILURE)
                {
                    ioBufDesc->numInputBuf = (int32_t)TIVX_KERNEL_MAX_PARAMS;
                    ioBufDesc->numVirtualCores = 1;
                }
                /*
                * FORCE NEGATIVE TEST END
                */

                (void)vxUnmapUserDataObject(config, map_id);

                tidlrt_printf("TIDL_RT_OVX: Mapping config file ... Done. %d bytes\n", (uint32_t)capacity);
                tidlrt_printf("TIDL_RT_OVX: Tensors, input = %d, output = %d\n", *num_input_tensors, *num_output_tensors);
            }
        }
    }

    return config;
}

static vx_user_data_object mapNetwork(AppObj* obj, void * netPtr, int32_t capacity)
{
    vx_status status;
    vx_user_data_object  network;
    vx_map_id  map_id;
    void *network_buffer = NULL;
    vx_context context = obj->context;

    tidlrt_printf("TIDL_RT_OVX: Mapping network file\n");

    TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_VX_CREATE_USER_DATA_OBJECT);
    network = vxCreateUserDataObject(context, "TIDL_network", capacity, NULL );
    TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_VX_CREATE_USER_DATA_OBJECT);

    status = vxGetStatus(vxCastRefFromUserDataObject(network));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((int32_t)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_VX_MAP_USER_DATA_OBJECT);
        status = vxMapUserDataObject(network, 0, capacity, &map_id,
                (void **)&network_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);
        TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_VX_MAP_USER_DATA_OBJECT);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        vxMapUserDataObject fails if vx_user_data_object provided is NULL.
        The vx_user_data_object provided here is network which is already
        checked for NULL in vxGetStatus
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((int32_t)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            network_buffer will never be null if the status above is VX_SUCCESS
            i.e network has been successfully mapped to network_buffer.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(network_buffer != NULL)
            {
            /* LDRA_JUSTIFY_END */
                TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_MEMCOPY_NETWORK_BUFFER);
                (void)memcpy((void *)network_buffer, (void *)netPtr, capacity); //PC-- can memcpy be avoid here?
                TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_MEMCOPY_NETWORK_BUFFER);

            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            network_buffer will never be null if the status above is VX_SUCCESS
            i.e network has been successfully mapped to network_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Unable copy netPtr in mapped network_buffer\n");
            }
            /* LDRA_JUSTIFY_END */

            TIDLRT_profileStart(&obj->profilePoints, TIDLRT_PROFILE_VX_UNMAP_USER_DATA_OBJECT);
            (void)vxUnmapUserDataObject(network, map_id);
            TIDLRT_profileEnd(&obj->profilePoints, TIDLRT_PROFILE_VX_UNMAP_USER_DATA_OBJECT);

        }
    }

    tidlrt_printf("TIDL_RT_OVX: Mapping network file... Done %d bytes\n", (uint32_t)capacity);

    return network;
}

static vx_user_data_object setCreateParams(AppObj *obj, sTIDLRT_Params_t *prms, int32_t inferenceMode)
{
    vx_status status;

    vx_user_data_object  createParams;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    capacity = sizeof(TIDL_CreateParams);
    createParams = vxCreateUserDataObject(obj->context, "TIDL_CreateParams", capacity, NULL );

    status = vxGetStatus(vxCastRefFromUserDataObject(createParams));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((int32_t)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = vxMapUserDataObject(createParams, 0, capacity, &map_id,
                (void **)&createParams_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        vxMapUserDataObject fails if vx_user_data_object provided is NULL.
        The vx_user_data_object provided here is createParams which is already
        checked for NULL in vxGetStatus
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((int32_t)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            createParams_buffer will never be null if the status above is
            VX_SUCCESS i.e createParams has been successfully mapped to
            createParams_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(createParams_buffer != NULL)
            {
            /* LDRA_JUSTIFY_END */
                TIDL_CreateParams *params = createParams_buffer;
                (void)TIDL_createParamsInit(params);
                params->isInbufsPaded                 = 1;
                params->traceLogLevel                 = obj->traceLogLevel;
                params->traceWriteLevel               = obj->traceWriteLevel;
                params->maxPreEmptDelay               = obj->maxPreEmptDelay;
                params->targetPriority                = obj->targetPriority;
                params->coreId                        = obj->coreNum - 1U;
                params->coreStartIdx                  = obj->coreStartIdx - 1;
                uint32_t reservedCtrl                 = prms->flowCtrl >> 16U;
                prms->flowCtrl                        = (prms->flowCtrl & 0xFFFFU);
                params->flowCtrl                      = prms->flowCtrl;
                params->reservedCtrl                  = reservedCtrl;
                (void)strncpy(params->traceBaseName, prms->traceBaseName, TIDL_STRING_SIZE);
                params->dumpNetInitBackupData         = prms->dumpNetInitBackupData;
                params->quantRangeExpansionFactor     = prms->quantRangeExpansionFactor;
                params->quantRangeUpdateFactor        = prms->quantRangeUpdateFactor ;
                params->forceNegativeTest             = prms->forceNegativeTest;
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            createParams_buffer will never be null if the status above is
            VX_SUCCESS i.e createParams has been successfully mapped to
            createParams_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf("TIDL_RT_OVX: ERROR: Unable to allocate memory for create time params! %d bytes\n", capacity);
            }
            /* LDRA_JUSTIFY_END */

            (void)vxUnmapUserDataObject(createParams, map_id);
        }
    }

    return createParams;
}

static vx_user_data_object setInArgs(AppObj *obj)
{
    vx_status status;

    vx_user_data_object  inArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *inArgs_buffer = NULL;

    capacity = sizeof(TIDL_InArgs);
    inArgs = vxCreateUserDataObject(obj->context, "TIDL_InArgs", capacity, NULL );

    status = vxGetStatus(vxCastRefFromUserDataObject(inArgs));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((int32_t)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = vxMapUserDataObject(inArgs, 0, capacity, &map_id,
                (void **)&inArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        vxMapUserDataObject fails if vx_user_data_object provided is NULL.
        The vx_user_data_object provided here is inArgs which is already
        checked for NULL in vxGetStatus
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((int32_t)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            inArgs_buffer will never be null if the status above is VX_SUCCESS
            i.e inArgs has been successfully mapped to inArgs_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(inArgs_buffer != NULL)
            {
            /* LDRA_JUSTIFY_END */
                TIDL_InArgs *prms = inArgs_buffer;
                prms->iVisionInArgs.size         = sizeof(TIDL_InArgs);
                prms->iVisionInArgs.subFrameInfo = 0;
                if(obj->traceLogLevel != 0U)
                {
                    prms->enableLayerPerfTraces = 1;
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            inArgs_buffer will never be null if the status above is VX_SUCCESS
            i.e inArgs has been successfully mapped to inArgs_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf("TIDL_RT_OVX: Unable to allocate memory for inArgs! %d bytes\n", capacity);
            }
            /* LDRA_JUSTIFY_END */

            (void)vxUnmapUserDataObject(inArgs, map_id);
        }
    }
    return inArgs;
}

static vx_user_data_object setOutArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  outArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *outArgs_buffer = NULL;

    capacity = sizeof(TIDL_outArgs);
    outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );

    status = vxGetStatus(vxCastRefFromUserDataObject(outArgs));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((int32_t)VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(outArgs, 0, capacity, &map_id,
                (void **)&outArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        vxMapUserDataObject fails if vx_user_data_object provided is NULL.
        The vx_user_data_object provided here is outArgs which is already
        checked for NULL in vxGetStatus
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((int32_t)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            outArgs_buffer will never be null if the status above is VX_SUCCESS
            i.e outArgs has been successfully mapped to outArgs_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(outArgs_buffer != NULL)
            {
            /* LDRA_JUSTIFY_END */
                TIDL_outArgs *prms = outArgs_buffer;
                prms->iVisionOutArgs.size         = sizeof(TIDL_outArgs);
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            outArgs_buffer will never be null if the status above is VX_SUCCESS
            i.e outArgs has been successfully mapped to outArgs_buffer
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf("TIDL_RT_OVX: Unable to allocate memory for outArgs! %d bytes\n", capacity);
            }
            /* LDRA_JUSTIFY_END */

            (void)vxUnmapUserDataObject(outArgs, map_id);
        }
    }
    /* LDRA_JUSTIFY_END */

    return outArgs;
}

static vx_status addParam(vx_reference params[], vx_reference obj, uint32_t *num_params)
{
    vx_status status = VX_FAILURE;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    this is a utility function and is expected to be used to only to add
    appParams for tivxTIDLNode, hence only 6 params are added at max which is
    less than TIDL_MAX_PARAMS (16)
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if((*num_params < TIDL_MAX_PARAMS))
    {
        params[*num_params] = obj;
        *num_params = *num_params + 1U;
        status = VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_END */

    return status;

}

static vx_status create_graph_tidl_tiovx(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    char tidlrt_string[TIDLRT_STRING_SIZE + 32];
    vx_reference params[TIDL_MAX_PARAMS];
    uint32_t i, num_params = 0;

    tidlrt_printf("TIDL_RT_OVX: Creating graph ... \n");

    /* Create OpenVx Graph */
    obj->graph = vxCreateGraph(obj->context);
    status = tidlrt_check_assert_status(vxCastRefFromGraph(obj->graph));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if(status == (int32_t)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        #ifndef BUILD_WITH_OPENACC
        (void)snprintf(tidlrt_string, (TIDLRT_STRING_SIZE + 32), "TIDL subgraph %s", (char*)obj->ioBufDesc.outDataName[0]);
        #endif

        (void)vxSetReferenceName(vxCastRefFromGraph(obj->graph), tidlrt_string);

        /* Create array of input tensors */
        status = createInputTensors(obj, obj->context, obj->config, obj->input_tensors);

        /* Create array of output tensors */
        status |= createOutputTensors(obj, obj->context, obj->config, obj->output_tensors);

        /* The 1st param MUST be config */
        status |= addParam(params, vxCastRefFromUserDataObject(obj->config), &num_params);

        /* The 2nd param MUST be network */
        status |= addParam(params, vxCastRefFromUserDataObject(obj->network), &num_params);

        /* The 3rd param MUST be create params */
        status |= addParam(params, vxCastRefFromUserDataObject(obj->createParams), &num_params);

        /* The 4th param MUST be inArgs */
        status |= addParam(params, vxCastRefFromUserDataObject(obj->inArgs), &num_params);

        /* The 5th param MUST be outArgs */
        status |= addParam(params, vxCastRefFromUserDataObject(obj->outArgs), &num_params);

        /* The 6th param MUST be traceData or NULL if trace writing is disabled */
        if(obj->traceWriteLevel != 0U)
        {
            status |= addParam(params, vxCastRefFromUserDataObject(obj->traceData), &num_params);
        }
        else
        {
            status |= addParam(params, (vx_reference)NULL, &num_params);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        addParam fails if num_params > TIDL_MAX_PARAMS (16).
        We are only adding 6 params before this check
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if(status == (int32_t)VX_SUCCESS)
        /* LDRA_JUSTIFY_END */
        {
            /* Create TIDL Node */
            #if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
            if (obj->is_tidl_node_tensor_list)
            {
                /* Packing Input tensors in vx_object_array */
                obj->input_tensor_list = createObjArray(obj->context, obj->input_tensors, obj->num_input_tensors);

                /* Packing Output tensors in vx_object_array */
                obj->output_tensor_list = createObjArray(obj->context, obj->output_tensors, obj->num_output_tensors);

                obj->tidl_node = tivxTIDLNodeV2(
                                                obj->graph,
                                                obj->config,
                                                obj->network,
                                                obj->createParams,
                                                obj->inArgs,
                                                obj->outArgs,
                                                obj->traceData,
                                                obj->input_tensor_list,
                                                obj->output_tensor_list);
            }
            else
            #endif
            {
                /* Creating TIDL-node with older way i.e. using vx_tensors*/
                obj->tidl_node = tivxTIDLNode(obj->graph, obj->kernel, params, obj->input_tensors, obj->output_tensors);
            }

            const char* priors[] = {
                    TIVX_TARGET_DSP_C7_1_PRI_1,
                    TIVX_TARGET_DSP_C7_1_PRI_2,
                    TIVX_TARGET_DSP_C7_1_PRI_3,
                    TIVX_TARGET_DSP_C7_1_PRI_4,
                    TIVX_TARGET_DSP_C7_1_PRI_5,
                    TIVX_TARGET_DSP_C7_1_PRI_6,
                    TIVX_TARGET_DSP_C7_1_PRI_7,
                    TIVX_TARGET_DSP_C7_1_PRI_8,
#if defined(SOC_J784S4) || defined (SOC_J722S) || defined (SOC_TDA54) || defined (SOC_J742S2)
                    TIVX_TARGET_DSP_C7_2_PRI_1,
                    TIVX_TARGET_DSP_C7_2_PRI_2,
                    TIVX_TARGET_DSP_C7_2_PRI_3,
                    TIVX_TARGET_DSP_C7_2_PRI_4,
                    TIVX_TARGET_DSP_C7_2_PRI_5,
                    TIVX_TARGET_DSP_C7_2_PRI_6,
                    TIVX_TARGET_DSP_C7_2_PRI_7,
                    TIVX_TARGET_DSP_C7_2_PRI_8,
#endif
#ifdef SOC_J784S4
                    TIVX_TARGET_DSP_C7_3_PRI_1,
                    TIVX_TARGET_DSP_C7_3_PRI_2,
                    TIVX_TARGET_DSP_C7_3_PRI_3,
                    TIVX_TARGET_DSP_C7_3_PRI_4,
                    TIVX_TARGET_DSP_C7_3_PRI_5,
                    TIVX_TARGET_DSP_C7_3_PRI_6,
                    TIVX_TARGET_DSP_C7_3_PRI_7,
                    TIVX_TARGET_DSP_C7_3_PRI_8,
                    TIVX_TARGET_DSP_C7_4_PRI_1,
                    TIVX_TARGET_DSP_C7_4_PRI_2,
                    TIVX_TARGET_DSP_C7_4_PRI_3,
                    TIVX_TARGET_DSP_C7_4_PRI_4,
                    TIVX_TARGET_DSP_C7_4_PRI_5,
                    TIVX_TARGET_DSP_C7_4_PRI_6,
                    TIVX_TARGET_DSP_C7_4_PRI_7,
                    TIVX_TARGET_DSP_C7_4_PRI_8,
#endif
            };


#if defined(SOC_J784S4) || defined (SOC_J722S) || defined (SOC_TDA54) || defined (SOC_J742S2)
            const char* mpuTargets[] = {
                TIVX_TARGET_MPU_1,
                TIVX_TARGET_MPU_2,
                TIVX_TARGET_MPU_3,
            };
            if((obj->coreNum < 1U) || (obj->coreNum > (uint32_t)TIDL_MAX_NUM_CORES))
            {
                tidlrt_printf("TIDL_RT_OVX: WARNING: Invalid core number specified - %u , please specify 1 <= coreNum <= %u. Defaulting to 1.\n", obj->coreNum, TIDL_MAX_NUM_CORES);
                obj->coreNum = 1;
            }
            if((obj->coreStartIdx < 1) || (obj->coreStartIdx > (int32_t)(TIDL_MAX_NUM_CORES - 1U)))
            {
                tidlrt_printf("TIDL_RT_OVX: WARNING: Invalid coreStartIdx specified - %d , please specify 0 <= coreStartIdx <= %u. Defaulting to 1.\n", obj->coreStartIdx, (TIDL_MAX_NUM_CORES - 1U));
                obj->coreStartIdx = 1;
            }
            if(obj->ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeDefault)
            {
                if(obj->coreStartIdx != 1)
                {
                    tidlrt_printf("TIDL_RT_OVX: WARNING : coreStartIdx is not applicable for inferenceMode = 0, please use coreNum to specify desired core for inference \n");
                }
            }
            if((obj->ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeHighThroughput) || (obj->ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
            {
                (void)vxSetNodeTarget(obj->tidl_node, (vx_enum)VX_TARGET_STRING, mpuTargets[0]); /* setting target to mpu_1 */
            }
            else /* default mode - set DSP targets */
            {
                (void)vxSetNodeTarget(obj->tidl_node, (vx_enum)VX_TARGET_STRING, priors[(8U * (obj->coreNum - 1U)) + (uint32_t)(obj->targetPriority)]);
            }
#else
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(obj->coreNum != 1U)
            {
                tidlrt_printf("TIDL_RT_OVX: WARNING: Invalid core number specified - %d , expected core number is 1. Defaulting to 1.\n", obj->coreNum);
                obj->coreNum = 1;
            }
            if(obj->coreStartIdx != 1)
            {
                tidlrt_printf("TIDL_RT_OVX: WARNING: Invalid coreStartIdx specified - %d , expected coreStartIdx is 1. Defaulting to 1.\n", obj->coreStartIdx);
                obj->coreStartIdx = 1;
            }
            /* LDRA_JUSTIFY_END */
            (void)vxSetNodeTarget(obj->tidl_node, (vx_enum)VX_TARGET_STRING, priors[(8U * (obj->coreNum - 1U)) + (uint32_t)obj->targetPriority]);
#endif
            (void)vxEnableEvents(obj->context);
            (void)vxRegisterEvent((vx_reference)obj->tidl_node, VX_EVENT_NODE_ERROR, 0, TIDL_NODE_ERROR_EVENT);

            status = tidlrt_check_assert_status(vxCastRefFromNode(obj->tidl_node));
            if(status == (int32_t)VX_SUCCESS)
            {
                /* Set names for diferent OpenVX objects */
                (void)vxSetReferenceName(vxCastRefFromUserDataObject(obj->config), "Config");
                (void)vxSetReferenceName(vxCastRefFromUserDataObject(obj->network), "Network");
                (void)vxSetReferenceName(vxCastRefFromUserDataObject(obj->createParams), "CreateParams");
                (void)vxSetReferenceName(vxCastRefFromUserDataObject(obj->inArgs), "InArgs");
                (void)vxSetReferenceName(vxCastRefFromUserDataObject(obj->outArgs), "OutArgs");

                for(i = 0; i < obj->num_input_tensors; i++) {
                    char tensor_name[] = "InputTensor_";
                    char ref_name[64];
                    (void)snprintf(ref_name, 64, "%s%d", tensor_name, i);
                    (void)vxSetReferenceName(vxCastRefFromTensor(obj->input_tensors[i]), ref_name);
                }

                for(i = 0; i < obj->num_output_tensors; i++) {
                    char tensor_name[] = "OutputTensor_";
                    char ref_name[64];
                    (void)snprintf(ref_name, 64, "%s%d", tensor_name, i);
                    (void)vxSetReferenceName(vxCastRefFromTensor(obj->output_tensors[i]), ref_name);
                }
                if (obj->kernel!=NULL)
                {
                    (void)vxSetReferenceName(vxCastRefFromKernel(obj->kernel), "TIDLKernel");
                }
                (void)vxSetReferenceName(vxCastRefFromNode(obj->tidl_node), "TIDLNode");


                tidlrt_printf("TIDL_RT_OVX: Creating graph ... Done.\n");
                obj->firstProcessCall = 1;
            }
        }
    }
    return status;
}


static vx_status verify_graph_tidl_tiovx(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Verify the TIDL Graph */

    status = vxVerifyGraph(obj->graph);
    if(status!= (int32_t)VX_SUCCESS)
    {
        tidlrt_printf("TIDL_RT_OVX: ERROR: Verifying TIDL graph ... Failed !!!\n");
        status = VX_FAILURE;
    }
    if (status != (int32_t)VX_FAILURE)
    {
        tidlrt_printf("TIDL_RT_OVX: Verifying TIDL graph ... Done.\n");
    }

    return status;
}

/*
 * tidlrt_update_inargs_dim_values - Synchronise TIDL_InArgs.inDimValues with
 * the caller-supplied input tensor dimValues before each vxProcessGraph() call.
 * 
 * setInArgs() creates the TIDL_InArgs user-data-object once at init time.
 * The caller may change the active batch/height/width on
 * every inference call (dynamic-shape use-case).  The firmware reads
 * inDimValues to know the actual live dimensions of each input buffer
 * during the TIDL_Process call
 */
static vx_status tidlrt_update_inargs_dim_values(AppObj *obj, sTIDLRT_Tensor_t *in[])
{
    vx_status   status = VX_SUCCESS;
    vx_map_id   map_id;
    void       *inArgs_buffer = NULL;
    vx_uint32   capacity = sizeof(TIDL_InArgs);

    status = vxMapUserDataObject(obj->inArgs, 0, capacity, &map_id,
            (void **)&inArgs_buffer, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    vxMapUserDataObject fails if vx_user_data_object provided is NULL.
    The vx_user_data_object provided here is obj->inArgs which is already
    validated during setInArgs() at init time.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((int32_t)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        inArgs_buffer will never be null if the status above is VX_SUCCESS
        i.e inArgs has been successfully mapped to inArgs_buffer
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if (inArgs_buffer != NULL)
        /* LDRA_JUSTIFY_END */
        {
            TIDL_InArgs *inArgs = (TIDL_InArgs *)inArgs_buffer;
            sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

            for (uint32_t id = 0U; id < (uint32_t)ioBufDesc->numInputBuf; id++)
            {
                uint32_t tidlrt_id = get_tidlrt_id(id,
                        (char *)ioBufDesc->inDataName[id],
                        in,
                        (uint32_t)ioBufDesc->numInputBuf);

                for (uint32_t d = 0U; d < (uint32_t)TIDL_DIM_MAX; d++)
                {
                    inArgs->inDimValues[id][d] = (int32_t)in[tidlrt_id]->dimValues[d];
                }
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        inArgs_buffer will never be null if the status above is VX_SUCCESS
        i.e inArgs has been successfully mapped to inArgs_buffer
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        else
        {
            tidlrt_printf("TIDL_RT_OVX: ERROR: tidlrt_update_inargs_dim_values: inArgs_buffer is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        (void)vxUnmapUserDataObject(obj->inArgs, map_id);
    }

    return status;
}

static vx_status run_graph_tidlrt_tiovx(AppObj *obj, sTIDLRT_Tensor_t *in[], sTIDLRT_Tensor_t *out[],
		sTIDLRT_PerfStats_t *stats)
{

    vx_status           status = VX_SUCCESS;
    void                *input_buffer = NULL;
    vx_status           event_status = VX_SUCCESS;
    vx_event_t          event;
    struct timespec     event_wait_time;

    event_wait_time.tv_sec = (time_t)0;
    event_wait_time.tv_nsec = (long)500; //500ns

    tidlrt_printf("TIDL_RT_OVX: Running Graph ... \n");

    if(stats != NULL)
    {
        get_time_u64(&stats->cpIn_time_start);
        (void)set_in_tidlrt_tensor_refs(obj, obj->input_tensors, in);
        (void)set_out_tidlrt_tensor_refs(obj, obj->output_tensors, out);

        status = map_cp_in_tidlrt_tensor_tiovx(obj, obj->context, obj->config, obj->input_tensors, input_buffer, in);
        if(status==(int32_t)VX_SUCCESS)
        {
            status = memset_out_tensor_tidlrt_tiovx(obj, obj->config, obj->output_tensors, out);
        }
        get_time_u64(&stats->cpIn_time_end);
        tidlrt_printf("TIDL_RT_OVX: memset_out_tensor_tidlrt_tiovx  ... Done.\n");

        if(status==(int32_t)VX_SUCCESS)
        {
            status = tidlrt_update_inargs_dim_values(obj, in);
        }
        tidlrt_printf("TIDL_RT_OVX: update_inargs_dim_values  ... Done.\n");

        if(status==(int32_t)VX_SUCCESS)
        {
            get_time_u64(&stats->proc_time_start);
            status = vxProcessGraph(obj->graph);
            get_time_u64(&stats->proc_time_end);

            tidlrt_printf("TIDL_RT_OVX: vxProcessGraph  ... Done.\n");
        }

        if (status == (int32_t)VX_SUCCESS)
        {
            /*
             * Check for NODE_ERROR event after some timeout using vxWaitEvent
             * in non-blocking mode.
             * 
             * NOTE: 
             * As of SDK 11.1 if vxProcessGraph is success, this results in a 
             * stary print which can be ignored successfully.
             * "VX_ZONE_ERROR: [tivxQueueGet:401] no elements found in que"
             * 
             */
             /*MISRA Rule 17.3: Reviewed clock_gettime is defined in time.h with _POSIX_C_SOURCE 200809L*/
            (void)nanosleep(&event_wait_time, NULL);
            event_status = vxWaitEvent(obj->context, &event, vx_true_e);
            if((event_status == (vx_status)VX_SUCCESS) &&
               (event.type == (vx_status)VX_EVENT_NODE_ERROR) &&
               (event.app_value == TIDL_NODE_ERROR_EVENT))
            {
                status = (int32_t)VX_FAILURE;
                tidlrt_printf("TIDL_RT_OVX: VX_EVENT_NODE_ERROR caught after TIDL_process() call.\n");
            }
        }

        if(status==(int32_t)VX_SUCCESS)
        {
            get_time_u64(&stats->cpOut_time_start);
            status = map_cp_out_tensor_tidlrt_tiovx(obj, obj->config, obj->output_tensors, out);
            get_time_u64(&stats->cpOut_time_end);
            tidlrt_printf("TIDL_RT_OVX: map_cp_out_tensor_tidlrt_tiovx  ... Done.\n");
        }
    }
    else
    {
        (void)set_in_tidlrt_tensor_refs(obj, obj->input_tensors, in);
        (void)set_out_tidlrt_tensor_refs(obj, obj->output_tensors, out);
        status = map_cp_in_tidlrt_tensor_tiovx(obj, obj->context, obj->config, obj->input_tensors, input_buffer, in);
        if(status==(int32_t)VX_SUCCESS)
        {
            status = memset_out_tensor_tidlrt_tiovx(obj, obj->config, obj->output_tensors, out);
        }

        if(status==(int32_t)VX_SUCCESS)
        {
            status = tidlrt_update_inargs_dim_values(obj, in);
        }

        if(status==(int32_t)VX_SUCCESS)
        {
            status = vxProcessGraph(obj->graph);
        }

        if (status == (int32_t)VX_SUCCESS)
        {
            /*
             * Check for NODE_ERROR event after some timeout using vxWaitEvent
             * in non-blocking mode.
             * 
             * NOTE: 
             * As of SDK 11.1 if vxProcessGraph is success, this results in a 
             * stary print which can be ignored successfully.
             * "VX_ZONE_ERROR: [tivxQueueGet:401] no elements found in que"
             * 
             */
             /*MISRA Rule 17.3: Reviewed clock_gettime is defined in time.h with _POSIX_C_SOURCE 200809L*/
            (void)nanosleep(&event_wait_time, NULL);
            event_status = vxWaitEvent(obj->context, &event, vx_true_e);
            if((event_status == (vx_status)VX_SUCCESS) &&
               (event.type == (vx_status)VX_EVENT_NODE_ERROR) &&
               (event.app_value == TIDL_NODE_ERROR_EVENT))
            {
                status = (int32_t)VX_FAILURE;
                tidlrt_printf("TIDL_RT_OVX: VX_EVENT_NODE_ERROR caught after TIDL_process() call.\n");
            }
        }

        if(status==(int32_t)VX_SUCCESS)
        {
            status = map_cp_out_tensor_tidlrt_tiovx(obj, obj->config, obj->output_tensors, out);
        }
    }

    if(status!=(int32_t)VX_SUCCESS)
    {
        tidlrt_printf("ERROR: Running TIDL graph ... Failed !!!\n");
    }

    tidlrt_printf("TIDL_RT_OVX: Ran TIDL graph ... Done.\n");

    #if defined TIDL_COVERAGE_DEAD_CODE
    // if(obj->traceWriteLevel) {
        // tivx_utils_tidl_trace_write(obj->traceData, obj->traceBaseName);
    // }
    #endif

    obj->firstProcessCall = 0;

    return status;
}

static vx_size getTensorDataType(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if(tidl_type == TIDL_SignedChar)
    {
        openvx_type = VX_TYPE_INT8;
    }
    else if(tidl_type == TIDL_UnsignedShort || tidl_type == TIDL_BFloat16)
    {
        openvx_type = VX_TYPE_UINT16;
    }
    else if(tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if(tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if(tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if(tidl_type == TIDL_UnsignedDoubleWord)
    {
        openvx_type = VX_TYPE_UINT64;
    }
    else if(tidl_type == TIDL_SignedDoubleWord)
    {
        openvx_type = VX_TYPE_INT64;
    }
    else if(tidl_type == TIDL_Bool)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This else block is for MISRA-C purposes
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    else if(tidl_type == TIDL_SinglePrecFloat)
    /* LDRA_JUSTIFY_END */
    {
        openvx_type = VX_TYPE_FLOAT32;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This else block is for MISRA-C purposes
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    else
    {
        /*Do Nothing*/
    }
    /* LDRA_JUSTIFY_END */
    return openvx_type;
}

static uint32_t getElementSize(uint32_t data_type)
{
    uint32_t elementSize=1;
    if((data_type == (uint32_t)VX_TYPE_INT8) || (data_type == (uint32_t)VX_TYPE_UINT8))
    {
        elementSize = sizeof(vx_int8);
    }
    else if((data_type == (uint32_t)VX_TYPE_INT16) || (data_type == (uint32_t)VX_TYPE_UINT16))
    {
        elementSize = sizeof(vx_int16);
    }
    #ifdef HOST_EMULATION
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
    This else if block is uncovered due to unsupported datatype (64-bit). This code is maintained for future use.
    Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
    This does not impact functional correctness or safety.
    <justification end> */
    else if((data_type == (uint32_t)VX_TYPE_INT32) || (data_type == (uint32_t)VX_TYPE_UINT32) || (data_type == (uint32_t)VX_TYPE_FLOAT32))
    /* LDRA_JUSTIFY_END */
    {
        elementSize = sizeof(vx_int32);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
    This else if block is uncovered due to unsupported datatype (64-bit). This code is maintained for future use.
    Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
    This does not impact functional correctness or safety.
    <justification end> */
    else if((data_type == (uint32_t)VX_TYPE_INT64) || (data_type == (uint32_t)VX_TYPE_UINT64))
    {
        elementSize = sizeof(vx_int64);
    }
    /* LDRA_JUSTIFY_END */
    #endif
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This else block is for MISRA-C purposes
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    else
    {
        /*Do Nothing*/
    }
    /* LDRA_JUSTIFY_END */
    return elementSize;
}

static vx_status createInputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors)
{
    vx_status status = VX_SUCCESS;
    int32_t input_buf_size = 0;
    uint32_t capacity;
    uint32_t id;

    vx_map_id map_id_input;

    vx_size    start[MAX_TENSOR_DIMS];
    vx_size    input_strides[MAX_TENSOR_DIMS];
    vx_size    input_sizes[MAX_TENSOR_DIMS];

    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    void *input_buffer;

    for(id = 0; id < (uint32_t)ioBufDesc->numInputBuf; id++) {

        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = (ioBufDesc->inNumChannels[id] + ioBufDesc->inPadCh[id] + 1) * ioBufDesc->inDIM1[id]* ioBufDesc->inDIM2[id];
        input_sizes[3] = ioBufDesc->inNumBatches[id];

        vx_size data_type = getTensorDataType(ioBufDesc->inElementType[id]);

        capacity = input_sizes[0] * input_sizes[1] * input_sizes[2] * input_sizes[3];

        start[3] = 0;
        start[2] = start[3];
        start[1] = start[2];
        start[0] = start[1];

        input_strides[0] = sizeof(vx_int8);

        if((data_type == (uint32_t)VX_TYPE_INT8) || (data_type == (uint32_t)VX_TYPE_UINT8))
        {
            input_strides[0] = sizeof(vx_int8);
        }
        else if((data_type == (uint32_t)VX_TYPE_INT16) || (data_type == (uint32_t)VX_TYPE_UINT16))
        {
            input_strides[0] = sizeof(vx_int16);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        No test case is expected to have VX_TYPE_UINT64 datatype
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else if((data_type == (uint32_t)VX_TYPE_INT32) || (data_type == (uint32_t)VX_TYPE_UINT32) || (data_type == (uint32_t)VX_TYPE_FLOAT32))
        {
        /* LDRA_JUSTIFY_END */
            input_strides[0] = sizeof(vx_int32);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        No test case is expected to have VX_TYPE_UINT64 datatype
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else if((data_type == (uint32_t)VX_TYPE_INT64) || (data_type == (uint32_t)VX_TYPE_UINT64))
        {
            input_strides[0] = sizeof(vx_int64);
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        No test case is expected to hit this code path since all tidl datatypes
        are covered in getTensorDataType
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        else
        {
            /*Do Nothing*/
        }
        /* LDRA_JUSTIFY_END */

        input_strides[1] = input_sizes[0] * input_strides[0];
        input_strides[2] = input_sizes[1] * input_strides[1];
        input_strides[3] = input_sizes[2] * input_strides[2];

        tidlrt_printf("TIDL_RT_OVX: input_sizes[0] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)input_strides[1], ioBufDesc->inWidth[id], ioBufDesc->inPadL[id], ioBufDesc->inPadR[id]);
        tidlrt_printf("TIDL_RT_OVX: input_sizes[1] = %d, dim = %d padT = %d padB = %d\n", (uint32_t)input_strides[2], ioBufDesc->inHeight[id], ioBufDesc->inPadT[id], ioBufDesc->inPadB[id]);
        tidlrt_printf("TIDL_RT_OVX: input_sizes[2] = %d, dim = %d \n", (uint32_t)input_sizes[2], ioBufDesc->inNumChannels[id]);
        tidlrt_printf("TIDL_RT_OVX: input_sizes[3] = %d, dim = %d \n", (uint32_t)input_sizes[3], ioBufDesc->inNumBatches[id]);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Users are never expected to pass invalid data type during Inference
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if(data_type != (uint32_t)VX_TYPE_INVALID)
        /* LDRA_JUSTIFY_END */
        {
            input_tensors[id] = vxCreateTensor(context, 4, input_sizes, data_type, 0);
            status = tivxMapTensorPatch(input_tensors[id], 4, start, input_sizes, &map_id_input, input_strides, &input_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Safe programming and cant be true in real usecase
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(status == (int32_t)VX_SUCCESS)
            /* LDRA_JUSTIFY_END */
            {
                /* Reset the input buffer, this will take care of padding requirement for TIDL */
                (void)memset(input_buffer, 0, (capacity*input_strides[0]));
                input_buf_size += ioBufDesc->inNumBatches[id]*ioBufDesc->inNumChannels[id]*ioBufDesc->inWidth[id]*ioBufDesc->inHeight[id];
                tidlrt_printf("TIDL_RT_OVX: input_buffer = %p %d\n", input_buffer, capacity);
                (void)tivxUnmapTensorPatch(input_tensors[id], map_id_input);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Safe programming and cant be true in real usecase
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf("TIDL_RT_OVX: Error in tivxMapTensorPatch\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Users are never expected to pass invalid data type during Inference
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        else
        {
            status = VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Safe programming and cant be true in real usecase
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if(status != (int32_t)VX_SUCCESS)
        {
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

#if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
static vx_object_array createObjArray(vx_context context, vx_tensor *tensors, uint32_t num_tensors)
{
    uint32_t tensorIndex = 0U ;
    vx_reference refList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    for(tensorIndex = 0; tensorIndex < num_tensors ; tensorIndex++){
        refList[tensorIndex] = (vx_reference)tensors[tensorIndex];
    }
    return tivxCreateObjectArrayFromList(context, refList, num_tensors);
}
#endif

static vx_status createOutputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *output_tensors)
{
    vx_status status = VX_SUCCESS;
    vx_size output_sizes[MAX_TENSOR_DIMS];

    uint32_t id;

    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    for(id = 0; id < (uint32_t)ioBufDesc->numOutputBuf; id++) {

        output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
        output_sizes[2] = (ioBufDesc->outNumChannels[id] + ioBufDesc->outPadCh[id]) * ioBufDesc->outDIM1[id] * ioBufDesc->outDIM2[id];
        output_sizes[3] = ioBufDesc->outNumBatches[id];

        vx_size data_type = getTensorDataType(ioBufDesc->outElementType[id]);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        No test case is expected to hit this code path since all tidl datatypes
        are covered in getTensorDataType
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if(data_type != (uint32_t)VX_TYPE_INVALID)
        /* LDRA_JUSTIFY_END */
        {
            output_tensors[id] = vxCreateTensor(context, 4, output_sizes, data_type, 0);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        No test case is expected to hit this code path since all tidl datatypes
        are covered in getTensorDataType
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        else
        {
            status = VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        No test case is expected to hit this code path since all tidl datatypes
        are covered in getTensorDataType
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if(status != (int32_t)VX_SUCCESS)
        {
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE
static inline uint32_t uclamp(float f, uint32_t min_val, uint32_t max_val)
{
    uint32_t val = (uint32_t) f;
    val = (val < min_val) ? min_val : val;
    val = (val > max_val) ? max_val : val;
    return val;
}

static inline int32_t clamp(float f, int32_t min_val, int32_t max_val)
{
    int32_t val = (int32_t) f;
    val = (val < min_val) ? min_val : val;
    val = (val > max_val) ? max_val : val;
    return val;
}

static inline uint64_t uclamp_64(float f, uint64_t min_val, uint64_t max_val)
{
    uint64_t val = (uint64_t) f;
    val = (val < min_val) ? min_val : val;
    val = (val > max_val) ? max_val : val;
    return val;
}

static inline int64_t clamp_64(float f, int64_t min_val, int64_t max_val)
{
    int64_t val = (int64_t) f;
    val = (val < min_val) ? min_val : val;
    val = (val > max_val) ? max_val : val;
    return val;
}

static inline uint8_t sat_uint8(float f)
{
    return uclamp(f, 0, 255);
}

static inline int8_t sat_int8(float f)
{
    return clamp(f, -128, 127);
}

static inline uint16_t sat_uint16(float f)
{
    return uclamp(f, 0, 65535);
}

static inline int16_t sat_int16(float f)
{
    return clamp(f, -32768, 32767);
}

static inline uint32_t sat_uint32(float f)
{
    return uclamp_64(f, 0ULL, 4294967295ULL);
}

static inline int32_t sat_int32(float f)
{
    return clamp_64(f, -2147483648LL, 2147483647LL);
}
#endif

static uint32_t get_tidlrt_id(uint32_t id, char *name,
        sTIDLRT_Tensor_t *tensors[], uint32_t num_tensors)
{
    uint32_t id_status = 0U;
    for (uint32_t tidlrt_id = 0; tidlrt_id < num_tensors; tidlrt_id++)
    {
        if (strncmp((char *) tensors[tidlrt_id]->name, name, TIDL_STRING_SIZE)
                == 0)
        {
            id_status = tidlrt_id;
            tidlrt_id = num_tensors;
        }
        else
        {
            id_status = id ;
        }
    }
    return id_status;
}

static vx_status set_in_tidlrt_tensor_refs(AppObj *obj, vx_tensor *input_tensors, sTIDLRT_Tensor_t *in[])
{
    vx_status status = VX_SUCCESS;
    uint32_t id;
    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;
    uint32_t numPlanes;
    void       *ref1Addr[MAX_TENSOR_DIMS];
    uint32_t    size[MAX_TENSOR_DIMS];

    for (id = 0;  id < (uint32_t)ioBufDesc->numInputBuf; id++)
    {
        obj->shared_input_tensors[id] = 0;

        uint32_t tidlrt_id = get_tidlrt_id(id, (char *) ioBufDesc->inDataName[id], in, ioBufDesc->numInputBuf);

        if((is_tidlrt_in_tensor_same(ioBufDesc, in, id, tidlrt_id) != 0) && (in[tidlrt_id]->memType == TIDLRT_MEM_SHARED))
        {
            status = tivxReferenceExportHandle(vxCastRefFromTensor(input_tensors[id]),
                                                    ref1Addr,
                                                    size,
                                                    MAX_TENSOR_DIMS,
                                                    &numPlanes);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            numPlanes for vx_tensor type is always 1 as defined by
            tivxReferenceExportHandle function.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(numPlanes == 1U)
            {
            /* LDRA_JUSTIFY_END */
                if((obj->firstProcessCall != 0U) && (ref1Addr[0] != NULL))
                {
                    (void)tivxMemFree(ref1Addr[0], size[0], (vx_enum)TIVX_MEM_EXTERNAL);
                }
                ref1Addr[0] = in[tidlrt_id]->ptr;

                status = tivxReferenceImportHandle(vxCastRefFromTensor(input_tensors[id]),
                                                (const void **)ref1Addr,
                                                size,
                                                numPlanes);
                (void)tivxMemBufferUnmap(ref1Addr[0], size[0], (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE);
                tidlrt_printf(" INFO: USER supplied input pointer is swapped! \n ");
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            numPlanes for vx_tensor type is always 1 as defined by
            tivxReferenceExportHandle function.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf(" WARN: Num Planes expected to be 1\n ");
            }
            /* LDRA_JUSTIFY_END */

            obj->shared_input_tensors[id] = 1;
        }
        else
        {
            tidlrt_printf(" INFO: Input tensor cannot be swapped. Either tensor params is not same or not allocated in shared memory.\n ");
        }
    }

    return status;
}

static vx_status set_out_tidlrt_tensor_refs(AppObj *obj, vx_tensor *output_tensors, sTIDLRT_Tensor_t *out[])
{
    vx_status status = VX_SUCCESS;
    uint32_t id;
    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;
    uint32_t numPlanes;
    void       *ref1Addr[MAX_TENSOR_DIMS];
    uint32_t    size[MAX_TENSOR_DIMS];

    for (id = 0;  id < (uint32_t)ioBufDesc->numOutputBuf; id++)
    {
        obj->shared_output_tensors[id] = 0;

        uint32_t tidlrt_id = get_tidlrt_id(id, (char *) ioBufDesc->outDataName[id], out, ioBufDesc->numOutputBuf);

        if((is_tidlrt_out_tensor_same(ioBufDesc, out, id, tidlrt_id) != 0) && (out[tidlrt_id]->memType == TIDLRT_MEM_SHARED))
        {
            status = tivxReferenceExportHandle(vxCastRefFromTensor(output_tensors[id]),
                                                    ref1Addr,
                                                    size,
                                                    MAX_TENSOR_DIMS,
                                                    &numPlanes);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            numPlanes for vx_tensor type is always 1 as defined by
            tivxReferenceExportHandle function.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(numPlanes == 1U)
            {
            /* LDRA_JUSTIFY_END */
                if((obj->firstProcessCall != 0U) && (ref1Addr[0] != NULL))
                {
                    (void)tivxMemFree(ref1Addr[0], size[0], (vx_enum)TIVX_MEM_EXTERNAL);
                }
                ref1Addr[0] = out[tidlrt_id]->ptr;

                status = tivxReferenceImportHandle(vxCastRefFromTensor(output_tensors[id]),
                                                (const void **)ref1Addr,
                                                size,
                                                numPlanes);
                (void)tivxMemBufferMap(ref1Addr[0], size[0], (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE);
                tidlrt_printf(" USER supplied output pointer is swapped! \n ");
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            numPlanes for vx_tensor type is always 1 as defined by
            tivxReferenceExportHandle function.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf(" Wrning: Num Planes expected to be 1\n ");
            }
            /* LDRA_JUSTIFY_END */
            obj->shared_output_tensors[id] = 1;
        }
        else
        {
            tidlrt_printf(" INFO: Ouput tensor cannot be swapped. Either tensor params is not same or not allocated in shared memory.\n ");
        }
    }
    return status;
}

static vx_status map_cp_in_tidlrt_tensor_tiovx(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors, void *input_buffer, sTIDLRT_Tensor_t *in[])
{
    vx_status status = VX_SUCCESS;
    int32_t input_buf_size = 0;
    uint32_t capacity;
    uint32_t id, tidlrt_id;
    void *pinputBuffer = input_buffer;

    vx_map_id map_id_input;

    vx_size    start[MAX_TENSOR_DIMS];
    vx_size    input_strides[MAX_TENSOR_DIMS];
    vx_size    input_sizes[MAX_TENSOR_DIMS];

    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    for(id = 0; id < (uint32_t)ioBufDesc->numInputBuf; id++)
    {
        tidlrt_id = get_tidlrt_id(id, (char *) ioBufDesc->inDataName[id], in, ioBufDesc->numInputBuf);

        if((in[tidlrt_id]->memType != TIDLRT_MEM_SHARED) || (is_tidlrt_in_tensor_same(ioBufDesc, in, id, tidlrt_id) == 0))
        {
            vx_size data_type = getTensorDataType(ioBufDesc->inElementType[id]);

            input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
            input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
            input_sizes[2] = (ioBufDesc->inNumChannels[id] + ioBufDesc->inPadCh[id] + 1) * ioBufDesc->inDIM1[id]* ioBufDesc->inDIM2[id];
            input_sizes[3] = ioBufDesc->inNumBatches[id];


            capacity = input_sizes[0] * input_sizes[1] * input_sizes[2] * input_sizes[3];

            start[3] = 0;
            start[2] = start[3];
            start[1] = start[2];
            start[0] = start[1];

            input_strides[0] = sizeof(vx_int8);

            if((data_type == (uint32_t)VX_TYPE_INT8) || (data_type == (uint32_t)VX_TYPE_UINT8))
            {
                input_strides[0] = sizeof(vx_int8);
            }
            else if((data_type == (uint32_t)VX_TYPE_INT16) || (data_type == (uint32_t)VX_TYPE_UINT16))
            {
                input_strides[0] = sizeof(vx_int16);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
            No test case is expected to have VX_TYPE_UINT64 datatype.
            Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
            This does not impact functional correctness or safety.
            <justification end> */
            else if((data_type == (uint32_t)VX_TYPE_INT32) || (data_type == (uint32_t)VX_TYPE_UINT32) || (data_type == (uint32_t)VX_TYPE_FLOAT32))
            {
            /* LDRA_JUSTIFY_END */
                input_strides[0] = sizeof(vx_int32);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
            No test case is expected to have VX_TYPE_UINT64 datatype.
            Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
            This does not impact functional correctness or safety.
            <justification end> */
            else if((data_type == (uint32_t)VX_TYPE_INT64) || (data_type == (uint32_t)VX_TYPE_UINT64))
            {
                input_strides[0] = sizeof(vx_int64);
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
            No test case is expected to have VX_TYPE_UINT64 datatype.
            Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
            This does not impact functional correctness or safety.
            <justification end> */
            else
            {
                /*Do Nothing*/
            }
            /* LDRA_JUSTIFY_END */

            input_strides[1] = input_sizes[0] * input_strides[0];
            input_strides[2] = input_sizes[1] * input_strides[1];
            input_strides[3] = input_sizes[2] * input_strides[2];

            tidlrt_printf("TIDL_RT_OVX: input_sizes[0] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)input_strides[1], ioBufDesc->inWidth[id], ioBufDesc->inPadL[id], ioBufDesc->inPadR[id]);
            tidlrt_printf("TIDL_RT_OVX: input_sizes[1] = %d, dim = %d padT = %d padB = %d\n", (uint32_t)input_strides[2], ioBufDesc->inHeight[id], ioBufDesc->inPadT[id], ioBufDesc->inPadB[id]);
            tidlrt_printf("TIDL_RT_OVX: input_sizes[2] = %d, dim = %d \n", (uint32_t)input_sizes[2], ioBufDesc->inNumChannels[id]);
            tidlrt_printf("TIDL_RT_OVX: input_sizes[3] = %d, dim = %d \n", (uint32_t)input_sizes[3], ioBufDesc->inNumBatches[id]);

            status = tivxMapTensorPatch(input_tensors[id], 4, start, input_sizes, &map_id_input, input_strides, &pinputBuffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

            /* Reset the input buffer, this will take care of padding requirement for TIDL */
            input_buf_size += ioBufDesc->inNumBatches[id]*ioBufDesc->inNumChannels[id]*ioBufDesc->inWidth[id]*ioBufDesc->inHeight[id] * ioBufDesc->inDIM1[id]* ioBufDesc->inDIM2[id];

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
            Users aren't expected to set TIDL_RT_DEBUG env variable,
            only for debug purposes.
            TIDL_LDRA_TAG: TIDL_LDRA_TAG_TIDL_RT_DEBUG_ENV_VAR
            Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
            However, due to the stated rationale, this is not covered in production code.
            <justification end> */
            if (tidlrt_debuglevel > 0U)
            {
                char *tidl_name = (char *) ioBufDesc->inDataName[id];
                char *ext_name  = (char *) in[tidlrt_id]->name;
                if (strncmp(tidl_name, ext_name, TIDL_STRING_SIZE) != 0)
                {
                    tidlrt_printf("TIDL_RT_OVX: input %d name MISMATCH: %s != %s\n",
                                id, tidl_name, ext_name);
                }

            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Safe programming and cant be true in real usecase
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if(status == (int32_t)VX_SUCCESS)
            /* LDRA_JUSTIFY_END */
            {
                status = cp_data_in_tidlrt_tensor_tiovx(obj, in, pinputBuffer, id, tidlrt_id);
                tidlrt_printf("TIDL_RT_OVX: input_buffer = %p %d\n", pinputBuffer, capacity);
                (void)tivxUnmapTensorPatch(input_tensors[id], map_id_input);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Safe programming and cant be true in real usecase
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
                tidlrt_printf("TIDL_RT_OVX: Error in tivxMapTensorPatch\n");
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return status;
}

static vx_status memset_out_tensor_tidlrt_tiovx(AppObj *obj, vx_user_data_object config, vx_tensor *output_tensors, sTIDLRT_Tensor_t *out[])
{
    vx_status status = VX_SUCCESS;
    uint32_t id;
    vx_size output_sizes[MAX_TENSOR_DIMS];
    sTIDL_IOBufDesc_t *ioBufDesc;
    void *output_buffer;
    vx_map_id map_id_output;

    ioBufDesc = &obj->ioBufDesc;
    for(id = 0; id < (uint32_t)ioBufDesc->numOutputBuf; id++)
    {
        uint32_t tidlrt_id = get_tidlrt_id(id, (char *) ioBufDesc->outDataName[id], out, ioBufDesc->numOutputBuf);

        if((out[tidlrt_id]->memType != TIDLRT_MEM_SHARED) || (is_tidlrt_out_tensor_same(ioBufDesc, out, id, tidlrt_id) == 0))
        {
            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[id]);
            output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
            output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
            output_sizes[2] = ioBufDesc->outNumChannels[id] + ioBufDesc->outPadCh[id];
            output_sizes[3] = ioBufDesc->outNumBatches[id];

            status = vxGetStatus(vxCastRefFromTensor(output_tensors[id]));

            if ((int32_t)VX_SUCCESS == status)
            {
                vx_size output_strides[MAX_TENSOR_DIMS];
                vx_size start[MAX_TENSOR_DIMS];

                start[3] = 0;
                start[2] = start[3];
                start[1] = start[2];
                start[0] = start[1];

                output_strides[0] = sizeof(vx_int8);

                if((data_type == (uint32_t)VX_TYPE_INT8) ||
                        (data_type == (uint32_t)VX_TYPE_UINT8))
                {
                    output_strides[0] = sizeof(vx_int8);
                }
                else if((data_type == (uint32_t)VX_TYPE_INT16) ||
                        (data_type == (uint32_t)VX_TYPE_UINT16))
                {
                    output_strides[0] = sizeof(vx_int16);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                All datatypes are already covered in getTensorDataType
                Added the else block for MISRA-C purposes
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                else if((data_type == (uint32_t)VX_TYPE_FLOAT32))
                {
                /* LDRA_JUSTIFY_END */
                    output_strides[0] = sizeof(vx_float32);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                All datatypes are already covered in getTensorDataType
                Added the else block for MISRA-C purposes
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                else
                {
                    /*Do Nothing*/
                }
                /* LDRA_JUSTIFY_END */

                output_strides[1] = output_sizes[0] * output_strides[0];
                output_strides[2] = output_sizes[1] * output_strides[1];
                output_strides[3] = output_sizes[2] * output_strides[2];

                status = tivxMapTensorPatch(output_tensors[id], 4, start, output_sizes, &map_id_output, output_strides, &output_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                Safe programming and cant be true in real usecase
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                if (status == (int32_t)VX_SUCCESS)
                /* LDRA_JUSTIFY_END */
                {
                    int32_t outWidth  = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id];
                    int32_t outHeight = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];;
                    int32_t elementSizeBytes  = output_strides[0];

                    tidlrt_printf("TIDL_RT_OVX: output_buffer = %p %d\n", output_buffer);

                    if((outWidth * outHeight * elementSizeBytes) <  (16*1024))
                    {
                        (void)memset( output_buffer, 0, outWidth * outHeight * elementSizeBytes);
                    }
                    (void)tivxUnmapTensorPatch(output_tensors[id], map_id_output);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                Safe programming and cant be true in real usecase
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                else
                {
                    tidlrt_printf("TIDL_RT_OVX: Error in tivxMapTensorPatch\n");
                }
                /* LDRA_JUSTIFY_END */
            }
        }
        else
        {
           tidlrt_printf("TIDL_RT_OVX: Shared Mem is used for Input Buff\n");
        }
    }

    return status;
}

static vx_status map_cp_out_tensor_tidlrt_tiovx(AppObj *obj, vx_user_data_object config, vx_tensor *output_tensors, sTIDLRT_Tensor_t *out[])
{
    vx_status status = VX_SUCCESS;
    uint32_t id, tidlrt_id, d;
    uint32_t elementSize =0;
    vx_size output_sizes[MAX_TENSOR_DIMS];
    sTIDL_IOBufDesc_t *ioBufDesc;
    void *output_buffer;
    vx_map_id map_id_output;
    vx_map_id  map_id_out_args;
    vx_uint32  capacity;
    void *outArgs_buffer = NULL;
    TIDL_outArgs *outArgs = NULL;

    ioBufDesc = &obj->ioBufDesc;

    capacity = sizeof(TIDL_outArgs);

    status = vxMapUserDataObject(obj->outArgs, 0, capacity, &map_id_out_args,
            (void **)&outArgs_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if ((int32_t)VX_SUCCESS == status)
    {
        outArgs = outArgs_buffer;
        if(obj->traceLogLevel != 0U)
        {
            TIDLRT_LogMetaData(outArgs, obj->traceBaseName);
        }
    }
    else
    {
        tidlrt_printf("TIDL_RT_OVX: Unable to read outArgs! %d bytes\n", capacity);
    }



    for(id = 0; id < (uint32_t)ioBufDesc->numOutputBuf; id++)
    {
        vx_size data_type = getTensorDataType(ioBufDesc->outElementType[id]);
        status = vxGetStatus(vxCastRefFromTensor(obj->output_tensors[id]));

        if ((int32_t)VX_SUCCESS == status)
        {
            elementSize = getElementSize(data_type);
        }
    }

    for(id = 0; id < (uint32_t)ioBufDesc->numOutputBuf; id++)
    {
        tidlrt_id = get_tidlrt_id(id, (char *) ioBufDesc->outDataName[id], out, ioBufDesc->numOutputBuf);

        uint8_t isOutputInSharedMem = (out[tidlrt_id]->memType == TIDLRT_MEM_SHARED) ? 1U : 0U;
        uint8_t isOutputTensorSame = (uint8_t)(is_tidlrt_out_tensor_same(ioBufDesc, out, id, tidlrt_id));

        /*
         * Propagate the runtime-computed output dimensions back into the
         * caller-supplied output tensors in case of dynamic outputs
         */
        if((outArgs != NULL) && (out[tidlrt_id]->isDynamic == 1U))
        {
            for (d = 0U; d < (uint32_t)TIDL_DIM_MAX; d++)
            {
                out[tidlrt_id]->dimValues[d] = outArgs->outDimValues[id][d];
            }
            out[tidlrt_id]->pitch[TIDL_LINE_PITCH]    = outArgs->outPitchValues[id][TIDL_LINE_PITCH];
            out[tidlrt_id]->pitch[TIDL_CHANNEL_PITCH] = outArgs->outPitchValues[id][TIDL_CHANNEL_PITCH];
            out[tidlrt_id]->pitch[TIDL_DIM2_PITCH]    = outArgs->outPitchValues[id][TIDL_DIM2_PITCH];
            out[tidlrt_id]->pitch[TIDL_DIM1_PITCH]    = outArgs->outPitchValues[id][TIDL_DIM1_PITCH];
            out[tidlrt_id]->pitch[TIDL_ROI_PITCH]     = outArgs->outPitchValues[id][TIDL_ROI_PITCH];
        }

        if((isOutputInSharedMem == 0U) || (isOutputTensorSame == 0U))
        {
            vx_size data_type = getTensorDataType(ioBufDesc->outElementType[id]);

            output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
            output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
            output_sizes[2] = (ioBufDesc->outNumChannels[id] + ioBufDesc->outPadCh[id]) * ioBufDesc->outDIM1[id] * ioBufDesc->outDIM2[id];
            output_sizes[3] = ioBufDesc->outNumBatches[id];

            status = vxGetStatus(vxCastRefFromTensor(output_tensors[id]));

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            After inference i.e. vxProcessGraph the output tensor are not NULL
            and are valid. Refer to run_graph_tidlrt_tiovx function.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if ((int32_t)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                vx_size output_strides[MAX_TENSOR_DIMS];
                vx_size start[MAX_TENSOR_DIMS];

                start[3] = 0;
                start[2] = start[3];
                start[1] = start[2];
                start[0] = start[1];

                output_strides[0] = sizeof(vx_int8);

                if((data_type == (uint32_t)VX_TYPE_INT8) ||
                        (data_type == (uint32_t)VX_TYPE_UINT8))
                {
                    output_strides[0] = sizeof(vx_int8);
                }
                else if((data_type == (uint32_t)VX_TYPE_INT16) ||
                        (data_type == (uint32_t)VX_TYPE_UINT16))
                {
                    output_strides[0] = sizeof(vx_int16);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                All datatypes are already covered in getTensorDataType
                Added the else block for MISRA-C purposes
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                else if((data_type == (uint32_t)VX_TYPE_FLOAT32))
                {
                /* LDRA_JUSTIFY_END */
                    output_strides[0] = sizeof(vx_float32);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                All datatypes are already covered in getTensorDataType
                Added the else block for MISRA-C purposes
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                else
                {
                    /*Do Nothing*/
                }
                /* LDRA_JUSTIFY_END */

                output_strides[1] = output_sizes[0] * output_strides[0];
                output_strides[2] = output_sizes[1] * output_strides[1];
                output_strides[3] = output_sizes[2] * output_strides[2];

                status = tivxMapTensorPatch(output_tensors[id], 4, start, output_sizes, &map_id_output, output_strides, &output_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                Safe programming and cant be true in real usecase
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                if (status == (int32_t)VX_SUCCESS)
                /* LDRA_JUSTIFY_END */
                {
                    if (outArgs != NULL)
                    {
                        status = cp_data_out_tensor_tidlrt_tiovx(obj, out, output_buffer, id, elementSize, data_type, outArgs->scale[id], tidlrt_id);

                        tidlrt_printf("TIDL_RT_OVX: elementSize = %d, OutElementType = %d scale[%d] = %f, rtelemType = %d, rtZp = %d, rtScale = %f\n",
                            elementSize, data_type, id, outArgs->scale[id],
                            out[tidlrt_id]->elementType, out[tidlrt_id]->zeroPoint,
                            out[tidlrt_id]->scale);
                    }
                    (void)tivxUnmapTensorPatch(output_tensors[id], map_id_output);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                Safe programming and cant be true in real usecase
                Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                else
                {
                    tidlrt_printf("TIDL_RT_OVX: Error in tivxMapTensorPatch\n");
                }
                /* LDRA_JUSTIFY_END */
            }

            tidlrt_printf("TIDL_RT_OVX: output_sizes[0] = %d, outWidht = %d padL = %d padR = %d\n", (uint32_t)output_sizes[0], ioBufDesc->outWidth[id], ioBufDesc->outPadL[id], ioBufDesc->outPadR[id]);
            tidlrt_printf("TIDL_RT_OVX: output_sizes[1] = %d, outHeight = %d padT = %d padB = %d\n", (uint32_t)output_sizes[1], ioBufDesc->outHeight[id], ioBufDesc->outPadT[id], ioBufDesc->outPadB[id]);
            tidlrt_printf("TIDL_RT_OVX: output_sizes[2] = %d, outNumChannels = %d \n", (uint32_t)output_sizes[2], ioBufDesc->outNumChannels[id]);
            tidlrt_printf("TIDL_RT_OVX: output_sizes[3] = %d, outNumBatches = %d \n", (uint32_t)output_sizes[3], ioBufDesc->outNumBatches[id]);
        }
        else
        {
            tidlrt_printf("TIDL_RT_OVX: Shared Mem is used for Output Buff\n");
        }
    }
    if(outArgs != NULL)
    {
        (void)vxUnmapUserDataObject(obj->outArgs, map_id_out_args);
    }

    return status;
}

static vx_status allocate_intermediate_tensors(IntHandle_t *rtHandle)
{
    vx_status status = VX_SUCCESS;
    int32_t totalHandles = 0;
    IntHandle_t *tmpHandle = rtHandle;
    IntHandle_t *tmpHandles[TIDLRT_MAX_HANDLES];
    int32_t numUniqueOutBufNeeded = 0;
    AppObj *obj = NULL;
    sTIDL_IOBufDesc_t *ioBufDesc = NULL;
    AppObj *tmpObj1 = NULL;
    sTIDL_IOBufDesc_t *tmpIoBufDesc1 = NULL;
    AppObj *tmpObj2 = NULL;
    sTIDL_IOBufDesc_t *tmpIoBufDesc2 = NULL;
    int32_t outBufSizes[TIDLRT_MAX_HANDLES * TIDL_MAX_ALG_OUT_BUFS] = {0};
    int32_t outBufSizesIdx = 0;
    void *outBufPool[TIDLRT_MAX_HANDLES * TIDL_MAX_ALG_OUT_BUFS];
    int8_t outBufPoolUsed[TIDLRT_MAX_HANDLES * TIDL_MAX_ALG_OUT_BUFS];
    void *tempOutBufPool[TIDLRT_MAX_HANDLES * TIDL_MAX_ALG_OUT_BUFS];

    while (tmpHandle != NULL)
    {
        tmpHandles[totalHandles] = tmpHandle;
        tmpHandle = (IntHandle_t*) tmpHandle->next;
        totalHandles++;
    }

    /*
     * Get total number of unique output buffers n
    */
    for(int32_t i = 0; i < totalHandles; i++)
    {
        int32_t totalAllocatedOutBuf = 0;
        obj = &(tmpHandles[i]->gAppObj);
        ioBufDesc = &(obj->ioBufDesc);

        /* Get number of output buffers that needs to be allocated for this subgraph */
        for(int32_t j = 0; j < ioBufDesc->numOutputBuf; j++)
        {
            int32_t needAllocation = false;
            for (int32_t k = 0; k < totalHandles; k++)
            {
                tmpObj1 = &(tmpHandles[k]->gAppObj);
                tmpIoBufDesc1 = &(tmpObj1->ioBufDesc);
                for (int32_t p = 0; p < tmpIoBufDesc1->numInputBuf; p++)
                {
                    if (strcmp((char *)ioBufDesc->outDataName[j], (char *)tmpIoBufDesc1->inDataName[p]) == 0)
                    {
                        vx_size data_type = getTensorDataType(ioBufDesc->outElementType[j]);
                        int32_t size = ioBufDesc->outBufSize[j] * (int32_t)getElementSize(data_type);
                        outBufSizes[outBufSizesIdx] = size;
                        outBufSizesIdx++;
                        totalAllocatedOutBuf++;
                        needAllocation = true;
                        break;
                    }
                }
                if (needAllocation != 0)
                {
                    break;
                }
            }
        }

        /*
         * Get number of output buffers that are produced before this subgraph but
         * but consumed after this. This indicates that it cannot be reused at this
         * output junction.
         */
        for(int32_t j = 0; j < i; j++)
        {
            tmpObj1 = &(tmpHandles[j]->gAppObj);
            tmpIoBufDesc1 = &(tmpObj1->ioBufDesc);
            for (int32_t p = 0; p < tmpIoBufDesc1->numOutputBuf; p++)
            {
                int32_t foundFutureConsumer = 0;
                for (int32_t k = i+1; k < totalHandles; k++)
                {
                    tmpObj2 = &(tmpHandles[k]->gAppObj);
                    tmpIoBufDesc2 = &(tmpObj2->ioBufDesc);
                    for (int32_t q = 0; q < tmpIoBufDesc2->numInputBuf; q++)
                    {
                        if (strcmp((char *)tmpIoBufDesc1->outDataName[p], (char *)tmpIoBufDesc2->inDataName[q]) == 0)
                        {
                            totalAllocatedOutBuf++;
                            foundFutureConsumer = 1;
                            break;
                        }
                    }
                    if (foundFutureConsumer == 1)
                    {
                        break;
                    }
                }
            }
        }
        if (totalAllocatedOutBuf > numUniqueOutBufNeeded)
        {
            numUniqueOutBufNeeded = totalAllocatedOutBuf;
        }
    }


    // Sort outBufSizes array to get largest numUniqueOutBufNeeded buffers
    for (int32_t i = 0; i < (outBufSizesIdx - 1); i++)
    {
        for (int32_t j = 0; j < (outBufSizesIdx - i - 1); j++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
            Standard bubble sort algorithm
            Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
            This does not impact functional correctness or safety.
            <justification end> */
            if (outBufSizes[j] < outBufSizes[j + 1])
            {
                int32_t temp = outBufSizes[j];
                outBufSizes[j] = outBufSizes[j + 1];
                outBufSizes[j + 1] = temp;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_SHARED_TENSORS
     * forces TIDLRT_allocSharedMem to allocate more than TIDLRT_MAX_MEM_RECS
     * leading to failure in outBufPool allocation
     */
    if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_SHARED_TENSORS)
    {
        for(int32_t i = 0; i < TIDLRT_MAX_MEM_RECS; i++)
        {
            void *ptr = TIDLRT_allocSharedMem(64, 1);
            tempOutBufPool[i] = ptr;
        }
    }
    /*
     * FORCE NEGATIVE TEST END
     */

    // Allocate the bufferpool
    for (int32_t i = 0; i < numUniqueOutBufNeeded; i++)
    {
        outBufPool[i] = (void *)TIDLRT_allocSharedMem(64, outBufSizes[i]);
        if(outBufPool[i] == NULL)
        {
            tidlrt_printf("TIDL_RT_OVX: ERROR: Unable to allocate intermediate output shared memory of size %d\n",outBufSizes[i]);
            status = VX_FAILURE;
        }
    }

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_SHARED_TENSORS
     * forces freeing of shared memory allocated by TIDLRT_allocSharedMem
     * in above TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_SHARED_TENSORS case
     */
    if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_SHARED_TENSORS)
    {
        for(int32_t i = 0; i < TIDLRT_MAX_MEM_RECS; i++)
        {
            TIDLRT_freeSharedMem(tempOutBufPool[i]);
        }
    }
    /*
     * FORCE NEGATIVE TEST END
     */

    if (status == (vx_status)VX_SUCCESS)
    {
        for(int32_t i = 0; i < totalHandles; i++)
        {
            int32_t priorityList[TIDL_MAX_ALG_OUT_BUFS];
            int32_t priorityListIdx = 0;
            obj = &(tmpHandles[i]->gAppObj);
            ioBufDesc = &(obj->ioBufDesc);

            // Free the bufferpool
            for (int32_t j = 0; j < numUniqueOutBufNeeded; j++)
            {
                outBufPoolUsed[j] = 0;
            }

            for (int32_t j = 0; j < TIDL_MAX_ALG_OUT_BUFS; j++)
            {
                priorityList[j] = 0;
            }

            /* Check if buffer pool is used in the future, if yes then mark it busy */
            for(int32_t j = 0; j < i; j++)
            {
                tmpObj1 = &(tmpHandles[j]->gAppObj);
                tmpIoBufDesc1 = &(tmpObj1->ioBufDesc);
                for (int32_t p = 0; p < tmpIoBufDesc1->numOutputBuf; p++)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
                    TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
                    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                    This does not affect runtime behavior or safety.
                    <justification end> */
                    if (p >= TIDL_MAX_INTERMEDIATE_TENSOR)
                    {
                        tidlrt_printf("TIDL_RT_OVX: ERROR: Not enough intermediate output tensors\n");
                        status = VX_FAILURE;
                        break;
                    }
                    /* LDRA_JUSTIFY_END */
                    int32_t foundFutureConsumer = 0;
                    sTIDLRT_Tensor_t *out_tensor = &(tmpHandles[j]->intermediateOutTensor[p]);
                    for (int32_t k = i+1; k < totalHandles; k++)
                    {
                        tmpObj2 = &(tmpHandles[k]->gAppObj);
                        tmpIoBufDesc2 = &(tmpObj2->ioBufDesc);
                        for (int32_t q = 0; q < tmpIoBufDesc2->numInputBuf; q++)
                        {
                            if (strcmp((char *)tmpIoBufDesc1->outDataName[p], (char *)tmpIoBufDesc2->inDataName[q]) == 0)
                            {
                                foundFutureConsumer = 1;
                                break;
                            }
                        }
                        if (foundFutureConsumer == 1)
                        {
                            break;
                        }
                    }
                    if (foundFutureConsumer == 1)
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                        This check is to ensure that the we loop over complete outBufPool
                        to check for output_tensor. out_tensor is already present
                        in outBufPool, hence the loop will never reach termination
                        by the condition k < numUniqueOutBufNeeded.
                        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                        <justification end> */
                        for (int32_t k = 0; k < numUniqueOutBufNeeded; k++)
                        /* LDRA_JUSTIFY_END */
                        {
                            if(outBufPool[k] == out_tensor->ptr)
                            {
                                outBufPoolUsed[k] = 1;
                                break;
                            }
                        }
                    }
                }

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
                TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
                Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                This does not affect runtime behavior or safety.
                <justification end> */
                if(status == (vx_status)VX_FAILURE)
                {
                    break;
                }
                /* LDRA_JUSTIFY_END */
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
            This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
            TIDL_LDRA_TAG : TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001
            Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
            This does not affect runtime behavior or safety.
            <justification end> */
            if (status == (vx_status)VX_SUCCESS)
            /* LDRA_JUSTIFY_END */
            {
                // Give allocation priority to buffers with future consumers
                for(int32_t j = 0; j < ioBufDesc->numOutputBuf; j++)
                {
                    int32_t foundFutureConsumer = 0;
                    for (int32_t p = i+2; p < totalHandles; p++)
                    {
                        tmpObj1 = &(tmpHandles[p]->gAppObj);
                        tmpIoBufDesc1 = &(tmpObj1->ioBufDesc);
                        for (int32_t q = 0; q < tmpIoBufDesc1->numInputBuf; q++)
                        {
                            if (strcmp((char *)ioBufDesc->outDataName[j], (char *)tmpIoBufDesc1->inDataName[q]) == 0)
                            {
                                foundFutureConsumer = 1;
                                break;
                            }
                        }
                        if (foundFutureConsumer == 1)
                        {
                            break;
                        }
                    }
                    if(foundFutureConsumer != 0)
                    {
                        priorityList[priorityListIdx] = j;
                        priorityListIdx++;
                    }
                }

                // Populate rest of the output tensors in the priority list
                for(int32_t j = 0; j < ioBufDesc->numOutputBuf; j++)
                {
                    int32_t found = 0;
                    for(int32_t k = 0; k < priorityListIdx; k++)
                    {
                        if(priorityList[k] == j)
                        {
                            found = 1;
                            break;
                        }
                    }
                    if(found == 0)
                    {
                        priorityList[priorityListIdx] = j;
                        priorityListIdx++;
                    }
                }

                // Allocate if the output is input to another subgraph
                for(int32_t id = 0; ((id < priorityListIdx) && (status != (vx_status)VX_FAILURE)); id++)
                {
                    int32_t j = priorityList[id];
                    int32_t allocate = false;

                    /* TIDL_LDRA_TAG_TIDL_RT_PRIOR_CHECK_001 */
                    if (j >= TIDL_MAX_INTERMEDIATE_TENSOR)
                    {
                        tidlrt_printf("TIDL_RT_OVX: ERROR: Not enough intermediate output tensors\n");
                        status = VX_FAILURE;
                        break;
                    }

                    sTIDLRT_Tensor_t *out_tensor = &(tmpHandles[i]->intermediateOutTensor[j]);
                    sTIDLRT_Tensor_t *in_tensor_next = NULL;

                    for (int32_t p = 0; p < totalHandles; p++)
                    {
                        tmpObj1 = &(tmpHandles[p]->gAppObj);
                        tmpIoBufDesc1 = &(tmpObj1->ioBufDesc);
                        for (int32_t q = 0; q < tmpIoBufDesc1->numInputBuf; q++)
                        {
                            if (strcmp((char *)ioBufDesc->outDataName[j], (char *)tmpIoBufDesc1->inDataName[q]) == 0)
                            {
                                if (q >= TIDL_MAX_INTERMEDIATE_TENSOR)
                                {
                                    tidlrt_printf("TIDL_RT_OVX: ERROR: Not enough intermediate input tensors\n");
                                    status = VX_FAILURE;
                                }
                                else
                                {
                                    allocate = true;
                                    in_tensor_next = &(tmpHandles[p]->intermediateInTensor[q]);
                                    tmpHandles[p]->intermediateInTensorAllocated[q] = 1;
                                    tmpHandles[p]->numIntermediateIn++;
                                }
                                break;
                            }
                        }

                        if((status == (vx_status)VX_FAILURE) || (allocate != 0))
                        {
                            break;
                        }
                    }

                    if ((status != (vx_status)VX_FAILURE) && (allocate != 0))
                    {
                        (void)TIDLRT_setTensorDefault(out_tensor);
                        out_tensor->bufferSize  = ioBufDesc->outBufSize[j];
                        out_tensor->elementType = ioBufDesc->outElementType[j];
                        out_tensor->scale = ioBufDesc->outTensorScale[j];
                        out_tensor->zeroPoint = ioBufDesc->outZeroPoint[j];
                        out_tensor->layout = ioBufDesc->outLayout[j];
                        out_tensor->pitch[TIDL_LINE_PITCH]    = ioBufDesc->outPadL[j] + ioBufDesc->outWidth[j] + ioBufDesc->outPadR[j];
                        out_tensor->pitch[TIDL_CHANNEL_PITCH] = ioBufDesc->outChannelPitch[j];
                        out_tensor->pitch[TIDL_DIM2_PITCH]    = out_tensor->pitch[TIDL_CHANNEL_PITCH] * ioBufDesc->outNumChannels[j];
                        out_tensor->pitch[TIDL_DIM1_PITCH]    = out_tensor->pitch[TIDL_DIM2_PITCH] * ioBufDesc->outDIM2[j];
                        out_tensor->pitch[TIDL_ROI_PITCH]     = out_tensor->pitch[TIDL_DIM1_PITCH] * ioBufDesc->outDIM1[j];
                        out_tensor->padValues[0] = ioBufDesc->outPadL[j];
                        out_tensor->padValues[1] = ioBufDesc->outPadR[j];
                        out_tensor->padValues[2] = ioBufDesc->outPadT[j];
                        out_tensor->padValues[3] = ioBufDesc->outPadB[j];
                        out_tensor->dimValues[TIDL_DIM_WIDTH] = ioBufDesc->outWidth[j];
                        out_tensor->dimValues[TIDL_DIM_HEIGHT] = ioBufDesc->outHeight[j];
                        out_tensor->dimValues[TIDL_DIM_NUMCH] = ioBufDesc->outNumChannels[j];
                        out_tensor->dimValues[TIDL_DIM_DIM2] = ioBufDesc->outDIM2[j];
                        out_tensor->dimValues[TIDL_DIM_DIM1] = ioBufDesc->outDIM1[j];
                        out_tensor->dimValues[TIDL_DIM_BATCH] = ioBufDesc->outNumBatches[j];
                        out_tensor->isDynamic = (uint8_t)ioBufDesc->outIsDynamic[j];
                        (void)strcpy((char*)out_tensor->name,(char*)ioBufDesc->outDataName[j]);
                        out_tensor->memType = TIDLRT_MEM_SHARED;

                        // Get appropriate buffer from the pool
                        vx_size data_type = getTensorDataType(out_tensor->elementType);
                        int32_t size = out_tensor->bufferSize * (int32_t)getElementSize(data_type);
                        out_tensor->ptr = NULL;

                        /*
                        * FORCE NEGATIVE TEST START
                        *
                        * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INTERMEDIATE_ALLOC_FAILURE
                        * forces depletion of the outBufPool[] leading to VX_FAILURE
                        * during ptr allocation of out tensor from the pool
                        */
                        if(rtHandle->rtPrms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INTERMEDIATE_ALLOC_FAILURE)
                        {
                            for (int32_t k = 0; k < numUniqueOutBufNeeded; k++)
                            {
                                outBufPoolUsed[k] = 1;
                            }
                        }
                        /*
                        * FORCE NEGATIVE TEST END
                        */

                        for (int32_t k = numUniqueOutBufNeeded - 1; k >= 0; k--)
                        {
                            if((outBufPoolUsed[k] == 0) && (outBufSizes[k] >= size))
                            {
                                out_tensor->ptr = outBufPool[k];
                                outBufPoolUsed[k] = 1;
                                break;
                            }
                        }

                        if (out_tensor->ptr != NULL)
                        {
                            tmpHandles[i]->intermediateOutTensorAllocated[j] = 1;
                            tmpHandles[i]->numIntermediateOut++;
                            // Set input tensor of next subgraph to output tensor of this subgraph
                            *in_tensor_next = *out_tensor;
                        }
                        else
                        {
                            tidlrt_printf("TIDL_RT_OVX: ERROR: Unable to allocate from buffer pool for output tensor (%d) in subgraph (%d)\n",j,i);
                            status = VX_FAILURE;
                        }
                    }
                }
            }

            if(status == (vx_status)VX_FAILURE)
            {
                break;
            }

        }
    }

    return status;
}
#ifdef CODE_COVERAGE_ENABLED_FOR_TIDL
int32_t TIDLRT_control(sTIDLRT_Params_t *prms, void *handle)
{
    int32_t status = VX_SUCCESS;
    IntHandle_t * rtHandle = (IntHandle_t*)handle;
    status |= tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CODE_COVERAGE_UPLOAD, NULL, 0u);
    return status;
}
int32_t TIDLRT_controlCallTest(sTIDLRT_Params_t *prms, void *handle)
{
    int32_t status = VX_SUCCESS;
    IntHandle_t * rtHandle = (IntHandle_t*)handle;
    vx_reference refs[2];
    vx_user_data_object  controlGetArgs, controlSetArgs;

    vx_context innerContext = vxCreateContext();

    controlGetArgs = vxCreateUserDataObject(innerContext, "TIDL_controlGetArgs", sizeof(TIDL_controlGetArgs), NULL);
    controlSetArgs = vxCreateUserDataObject(innerContext, "TIDL_controlSetArgs", sizeof(TIDL_controlSetArgs), NULL);

    refs[0] = NULL;
    
    status = tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CMD_GET_C7X_PTRS, refs, 1u);

    refs[0] = (vx_reference)(controlGetArgs);
    
    status = tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CMD_GET_C7X_PTRS, refs, 1u);

    refs[0] = NULL;

    status |= tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CMD_SET_C7X_PTRS, refs, 1u);
   
    refs[0] = (vx_reference)controlSetArgs;
    memcpy(controlSetArgs, controlGetArgs, sizeof(vx_context));

    status |= tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CMD_SET_C7X_PTRS, refs, 1u);

    status |= tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CMD_INIT, NULL, 0u);

    status |= tivxNodeSendCommand(rtHandle->gAppObj.tidl_node, 0, TIVX_TIDL_CODE_COVERAGE_UPLOAD, NULL, 0u);
    return status;
}
#endif
