/*
*
* Copyright (c) 2021 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <TI/j7_tvm.h>
#include <tivx_utils_file_rd_wr.h>
#include <tivx_utils_graph_perf.h>
#include <tivx_utils_tidl_trace.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <float.h>
#include <math.h>
#include "tvm_rt_ovx_utils.h"

#include "itvm_rt.h"

extern char* strdup(const char* var);

/*! \brief Memory size for storing trace data */
#define TIVX_TVM_TRACE_DATA_SIZE  (2 * 1024 * 1024)

#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

/*! \brief This structure is internal handle for TVM RT API wrapper */
typedef struct
{
  /*! Struct to an application object */
  TVMAppObj gAppObj;
  /*! TVM RT create time configuration parametes */
  sTVMRT_Params_t rtPrms;
} IntHandle_t;


static uint32_t   tvmrt_debuglevel = 0;

static int init_tvm_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms);
static int deinit_tvm_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms);
static vx_status create_graph_tvm_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms);
static vx_status verify_graph_tvm_tiovx(TVMAppObj *obj);
static vx_status run_graph_tvmrt_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms,
                                sTVMRT_Tensor_t *in[], sTVMRT_Tensor_t *out[]);
static void delete_graph_tvm_tiovx(TVMAppObj *obj);

/*! \brief Print debug message if TVM_RT_DEBUG env var is set */
static int debug_printf(const char *fmt, ...)
{
  va_list ap;
  int ret = 0;

  if(tvmrt_debuglevel)
  {
    va_start(ap, fmt);
    ret = vprintf(fmt, ap);
    va_end(ap);
  }

  return ret;
}

static void get_time_u64(uint64_t *t)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    *t = (uint64_t)ts.tv_sec * (uint64_t)1000000000ull + (uint64_t)ts.tv_nsec;
}

static void tvmrt_init()
{
  char *debug_str = getenv("TVM_RT_DEBUG");
  if(!debug_str) 
  {
    tvmrt_debuglevel = 0;
  }
  else
  {
    tvmrt_debuglevel = atoi(debug_str);
  }
}

/*!
 * \brief Create TVMRT runtime
 * \param prms TVMRT configuration parameters
 * \return A handle to created TVMRT runtime instance
 */
int32_t
TVMRT_create(sTVMRT_Params_t *prms, void **handle)
{
  int32_t status = VX_SUCCESS;

  tidl_rt_ovx_Init();
  tvmrt_init();

  APP_ASSERT(prms != NULL);

  IntHandle_t *rtHandle = (IntHandle_t*)malloc(sizeof(IntHandle_t));
  if (rtHandle==NULL)
  {
    printf("TVM_RT_OVX: ERROR: Unable to allocate memory for TVMRT handle\n");
    return VX_FAILURE;
  }

  TVMAppObj *obj = &(rtHandle->gAppObj);

  debug_printf("TVMRT_create(): vxCreateContex()...\n");
  obj->context = vxCreateContext();
  APP_ASSERT_VALID_REF(obj->context);

  obj->traceBaseName = prms->traceBaseName ? strdup(prms->traceBaseName) : NULL;

  status = init_tvm_tiovx(obj, prms);

  if (VX_SUCCESS == status)
  {
    status = create_graph_tvm_tiovx(obj, prms);
  } else {
    printf("TVM_RT_OVX: ERROR: Init function failed\n");
  }

  if (VX_SUCCESS == status)
  {
    status = verify_graph_tvm_tiovx(obj);
  } else {
    printf("TVM_RT_OVX: ERROR: Create OpenVX graph failed\n");
  }

  if (VX_SUCCESS != status)
  {
    printf("TVM_RT_OVX: ERROR: Verify OpenVX graph failed\n");
  }

  rtHandle->rtPrms = *prms;
  *handle = rtHandle;
  return status;
}

/*!
 * \brief Invoke TVMRT runtime
 * \param handle Handle to the TVMRT runtime instance
 * \param in Array of input tensors
 * \param out Array of output tensors
 * \return vx status (0 for success)
 */
int32_t
TVMRT_invoke(void *handle, sTVMRT_Tensor_t *in[], sTVMRT_Tensor_t *out[])
{
  int32_t status = VX_SUCCESS;
  IntHandle_t * rtHandle = (IntHandle_t*)handle;

  status = run_graph_tvmrt_tiovx(&(rtHandle->gAppObj), &(rtHandle->rtPrms),
                                 in, out);

  return status;
}

/*!
 * \brief Deactivate TVMRT runtime
 * \param handle Handle to the TVMRT runtime instance
 * \return vx status (0 for success)
 */
int32_t
TVMRT_deactivate(void *handle)
{
    int32_t status = VX_SUCCESS;
    return status;
}

/*!
 * \brief Delete TVMRT runtime
 * \param handle Handle to the TVMRT runtime instance
 * \return vx status (0 for success)
 */
int32_t
TVMRT_delete(void *handle)
{
  int32_t status = VX_SUCCESS;
  IntHandle_t * rtHandle = (IntHandle_t*)handle;
  debug_printf("TVMRT_delete(): ...\n");

  delete_graph_tvm_tiovx(&(rtHandle->gAppObj));
  status = deinit_tvm_tiovx(&(rtHandle->gAppObj), &(rtHandle->rtPrms));

  if (status == VX_SUCCESS)
  {
    vxReleaseContext(&rtHandle->gAppObj.context);

    free(rtHandle->gAppObj.traceBaseName);
    free(handle);
    tidl_rt_ovx_DeInit(0);
  }

  debug_printf("TVMRT_delete(): ... Done\n");
  return status;
}

/*!
 * \brief Set default values for TVMRT configuration parameters
 * \param prms TVMRT configuration parameters
 * \return vx status (0 for success)
 */
int32_t
TVMRT_setParamsDefault(sTVMRT_Params_t *prms)
{
  int32_t status                  = 0;

  memset(prms, 0, sizeof(sTVMRT_Params_t));
  prms->TVMVprintf               = vprintf;
  prms->maxPreEmptDelay          = FLT_MAX;
  prms->targetPriority           = 0;
  prms->tvm_rt_trace_node        = -1;
  prms->tvm_rt_trace_size        = TIVX_TVM_TRACE_DATA_SIZE;
  prms->coreNum                  = 1;
  prms->stats                    = NULL;

  debug_printf("TVM_RT_OVX: Set default TVMRT params done\n");
  return status;
}

/*!
 * \brief Create VX parameters for the VX TVM kernel
 * \param obj App object
 * \param prms TVMRT configuration params
 * \return vx status
 */
static vx_status
init_tvm_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms)
{
  vx_status status = VX_SUCCESS;
  vx_map_id  map_id;

  uint32_t num_input_tensors  = prms->num_input_tensors;
  uint32_t num_output_tensors = prms->num_output_tensors;

  debug_printf("TVM_RT_OVX: Init ... \n");

  /* Create a vx_userdata object and map the TVMRT configuration parameters */
  obj->config = vxCreateUserDataObject(obj->context, "tivxTVMJ7Params",
                                       sizeof(tivxTVMJ7Params), NULL);
  status = vxGetStatus((vx_reference)obj->config);
  APP_ASSERT_VALID_REF(obj->config);

  if (VX_SUCCESS == status)
  {
    tivxTVMJ7Params *tvmParams;
    status = vxMapUserDataObject(obj->config, 0, sizeof(tivxTVMJ7Params),
                                 &map_id, (void **)&tvmParams,
                                 VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (VX_SUCCESS == status)
    {
      if (NULL == tvmParams)
      {
        printf("TVM_RT_OVX: ERROR: Map of config object failed\n");
        return VX_FAILURE;
      }

      APP_ASSERT(num_input_tensors <= TIVX_KERNEL_TVM_MAX_INPUT_TENSORS);
      APP_ASSERT(num_input_tensors + num_output_tensors <=
                 TIVX_KERNEL_TVM_MAX_TOTAL_TENSORS);
      APP_ASSERT(sizeof(tvmParams->input_names_offset) >=
                 sizeof(prms->input_names_offset));
      APP_ASSERT(sizeof(tvmParams->input_names) >=
                 sizeof(prms->input_names));
      APP_ASSERT(sizeof(sTVMRT_TensorParams_t) ==
                 sizeof(tivxTVMTensorParams));
      APP_ASSERT(sizeof(tvmParams->tensors_params) >=
                 sizeof(prms->tensors_params));

      tvmParams->num_input_tensors  = num_input_tensors;
      tvmParams->num_output_tensors = num_output_tensors;
      memcpy(tvmParams->input_names_offset, prms->input_names_offset,
             sizeof(prms->input_names_offset));
      memcpy(tvmParams->input_names, prms->input_names,
             sizeof(prms->input_names_offset));
      memcpy(tvmParams->tensors_params, prms->tensors_params,
             sizeof(prms->tensors_params));

      tvmParams->optimize_ivision_activation = 1;
      tvmParams->rt_info.tvm_rt_debug_level = prms->tvm_rt_debug_level;
      tvmParams->rt_info.tidl_trace_log_level = prms->traceLogLevel;
      tvmParams->rt_info.tidl_trace_write_level = prms->traceWriteLevel;
      tvmParams->rt_info.max_preempt_delay = prms->maxPreEmptDelay;
      tvmParams->rt_info.tvm_rt_trace_node = prms->tvm_rt_trace_node;
      tvmParams->rt_info.tvm_rt_target_priority = prms->targetPriority;
      tvmParams->rt_info.tvm_rt_core_num = prms->coreNum;
      vxUnmapUserDataObject(obj->config, map_id);
    }
  }

  /* Create a vx_userdata object and map the TVM C7x deployable module */
  obj->deploy_mod = vxCreateUserDataObject(obj->context, "tivxTVMJ7DeployMod",
                                           prms->deploy_mod_size, NULL);
  status = vxGetStatus((vx_reference)obj->deploy_mod);
  APP_ASSERT_VALID_REF(obj->deploy_mod);
  if (VX_SUCCESS == status)
  {
    void *deploy_mod;
    status = vxMapUserDataObject(obj->deploy_mod, 0, prms->deploy_mod_size,
                                 &map_id, (void **)&deploy_mod,
                                 VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (VX_SUCCESS == status)
    {
      if (NULL == deploy_mod)
      {
        printf("TVM_RT_OVX: ERROR: Map of deploy_mod object failed\n");
        return VX_FAILURE;
      }
      memcpy(deploy_mod, prms->deploy_mod, prms->deploy_mod_size);
      vxUnmapUserDataObject(obj->deploy_mod, map_id);
    }
  }

  /* Create a vx_tensor objects for input/output tensors */
  obj->input_tensors = malloc(num_input_tensors * sizeof(vx_tensor));
  obj->output_tensors = malloc(num_output_tensors * sizeof(vx_tensor));
  APP_ASSERT(obj->input_tensors != NULL && obj->output_tensors != NULL);

  obj->kernel = tivxAddKernelTVM(obj->context,
                                 num_input_tensors, num_output_tensors);
  APP_ASSERT_VALID_REF(obj->kernel);

  /* Save a copy of number of input/output tensors required as per config */
  obj->num_input_tensors  = num_input_tensors;
  obj->num_output_tensors = num_output_tensors;

  if(prms->tvm_rt_debug_level > 1)
  {
    obj->traceData = vxCreateUserDataObject(obj->context, "TVM_traceData",
                                            prms->tvm_rt_trace_size, NULL);
    APP_ASSERT_VALID_REF(obj->traceData);
  }

  debug_printf("TVM_RT_OVX: Init done.\n");
  return status;
}

/*!
 * \brief Deinit vx TVM kernel
 * \param obj App object
 * \return vx status
 */
static int
deinit_tvm_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms)
{
  uint32_t    id;
  vx_status status = VX_SUCCESS;
  debug_printf("TVM_RT_OVX: DeInit ... \n");

  vxReleaseUserDataObject(&obj->config);
  vxReleaseUserDataObject(&obj->deploy_mod);
  if(prms->tvm_rt_debug_level > 1)
  {
    vxReleaseUserDataObject(&obj->traceData);
  }
  for (id = 0; id < obj->num_input_tensors; id++)
  {
    vxReleaseTensor(&obj->input_tensors[id]);
  }
  for (id = 0; id < obj->num_output_tensors; id++)
  {
    vxReleaseTensor(&obj->output_tensors[id]);
  }

  free(obj->input_tensors);
  free(obj->output_tensors);

  //TODO: Host Emulation only: tivxTVMUnloadKernels(obj->context);

  debug_printf("TVM_RT_OVX: DeInit... Done.\n");
  return status;
}

/*!
 * \brief Delete VX graph for TVM kernel
 * \param obj App object
 * \return vx status
 */
static void
delete_graph_tvm_tiovx(TVMAppObj *obj)
{
  debug_printf("TVM_RT_OVX: Delete TVM graph ... \n");

  vxReleaseNode(&obj->tvm_node);
  vxRemoveKernel(obj->kernel);
  vxReleaseGraph(&obj->graph);

  debug_printf("TVM_RT_OVX: Delete TVM graph ... Done.\n");
}

/*!
 * \brief Create VX graph for calling the VX TVM kernel
 * \param obj App object
 * \param prms TVMRT configuration params
 * \return vx status
 */
static vx_status
create_graph_tvm_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms)
{
  vx_status status = VX_SUCCESS;

  vx_reference params[TIVX_KERNEL_TVM_IN_FIRST_TENSOR +
                      TIVX_KERNEL_TVM_MAX_TOTAL_TENSORS];
  vx_size tensor_sizes[1];
  uint32_t i;

  debug_printf("TVM_RT_OVX: Creating graph ... \n");

  /* Create OpenVx Graph */
  obj->graph = vxCreateGraph(obj->context);
  APP_ASSERT_VALID_REF(obj->graph);
  vxSetReferenceName((vx_reference)obj->graph, "TVM C7x deployable module");

  /* Create array of input tensors */
  for (i = 0; i < obj->num_input_tensors; i++)
  {
    tensor_sizes[0] = prms->tensors_params[i].size_in_bytes;
    obj->input_tensors[i] = vxCreateTensor(obj->context, 1, tensor_sizes,
                                           VX_TYPE_UINT8, 0);
  }

  /* Create array of output tensors */
  for (i = 0; i < obj->num_output_tensors; i++)
  {
    tensor_sizes[0] = prms->tensors_params[obj->num_input_tensors + i
                                          ].size_in_bytes;
    obj->output_tensors[i] = vxCreateTensor(obj->context, 1, tensor_sizes,
                                            VX_TYPE_UINT8, 0);
  }

  /* The 1st param MUST be config */
  params[0] = (vx_reference)obj->config;

  /* The 2nd param MUST be deployable module */
  params[1] = (vx_reference)obj->deploy_mod;

  if(prms->tvm_rt_debug_level > 1)
  {
    params[2] = (vx_reference)obj->traceData;
  }
  else
  {
    params[2] = (vx_reference)NULL; //The 3rd param MUST be NULL if trace data dump is not enabled
  }
  /* Create TVM Node */
  obj->tvm_node = tivxTVMNode(obj->graph, obj->kernel, params,
                              obj->input_tensors, obj->output_tensors);
  /* TODO need to check enum is available or not */
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

#if defined(SOC_J784S4)
  if((prms->coreNum < 1) || (prms->coreNum > 4))
  {
    printf("TVM_RT_OVX: ERROR: Invalid core number specified - %d , please specify 1,2,3 or 4 as core number\n", prms->coreNum);
    return VX_ERROR_INVALID_VALUE;
  }
#elif defined (SOC_J722S) || defined (SOC_TDA54) || defined (SOC_J742S2)
  if((prms->coreNum < 1) || (prms->coreNum > 2))
  {
    printf("TVM_RT_OVX: ERROR: Invalid core number specified - %d , please specify 1,2 as core number\n", prms->coreNum);
    return VX_ERROR_INVALID_VALUE;
  }
#else
  if(prms->coreNum != 1)
  {
    printf("TVM_RT_OVX: ERROR: Invalid core number specified - %d , expected core number is 1\n", prms->coreNum);
    return VX_ERROR_INVALID_VALUE;
  }
#endif
  vxSetNodeTarget(obj->tvm_node, VX_TARGET_STRING,
                  priors[8 * (prms->coreNum - 1) + prms->targetPriority]);
  APP_ASSERT_VALID_REF(obj->tvm_node);

  /* Set names for diferent OpenVX objects */
  vxSetReferenceName((vx_reference)obj->config, "Config");
  vxSetReferenceName((vx_reference)obj->deploy_mod, "DeployMod");

  for(i = 0; i < obj->num_input_tensors; i++) {
    char tensor_name[] = "InputTensor_";
    char ref_name[64];
    snprintf(ref_name, 64, "%s%d", tensor_name, i);
    vxSetReferenceName((vx_reference)obj->input_tensors[i], ref_name);
  }

  for(i = 0; i < obj->num_output_tensors; i++) {
    char tensor_name[] = "OutputTensor_";
    char ref_name[64];
    snprintf(ref_name, 64, "%s%d", tensor_name, i);
    vxSetReferenceName((vx_reference)obj->output_tensors[i], ref_name);
  }

  vxSetReferenceName((vx_reference)obj->kernel, "TVMKernel");
  vxSetReferenceName((vx_reference)obj->tvm_node, "TVMNode");

  debug_printf("TVM_RT_OVX: Creating graph ... Done.\n");

  return status;
}

/*!
 * \brief Verify VX graph for calling the VX TVM kernel
 * \param obj App object
 * \return vx status
 */
static vx_status
verify_graph_tvm_tiovx(TVMAppObj *obj)
{
  vx_status status = VX_SUCCESS;

  /* Verify the TVM Graph */
  status = vxVerifyGraph(obj->graph);
  if(status!=VX_SUCCESS)
  {
    printf("TVM_RT_OVX: ERROR: Verifying TVM graph ... Failed !!!\n");
    return status;
  }
  debug_printf("TVM_RT_OVX: Verifying TVM graph ... Done.\n");

  return status;
}

/*!
 * \brief Real function for invoking the TVMRT runtime
 * \param obj App object
 * \param prms TVMRT configuration parameters
 * \param in Array of input tensors
 * \param out Array of output tensors
 * \return vx status
 */
static vx_status
run_graph_tvmrt_tiovx(TVMAppObj *obj, sTVMRT_Params_t *prms,
                      sTVMRT_Tensor_t *in[], sTVMRT_Tensor_t *out[])
{

  vx_status status = VX_SUCCESS;
  //vx_char   *trace_filename = NULL;
  vx_map_id map_id;
  vx_size   starts[1];
  vx_size   ends[1];
  vx_size   strides[1];
  void     *ptr, *trace_ptr;
  int32_t  trace_written_size;

  debug_printf("TVM_RT_OVX: Copying inputs ... \n");
  if(prms->stats != NULL) get_time_u64(&prms->stats->cpIn_time_start);
  for (int32_t i = 0; i < obj->num_input_tensors; i++)
  {
    starts[0]  = 0;
    ends[0] = prms->tensors_params[i].size_in_bytes;
    strides[0] = sizeof(vx_uint8);
    debug_printf("TVM_RT_OVX:   [%d] start=%d, end=%d\n", i, starts[0],ends[0]);
    status = tivxMapTensorPatch(obj->input_tensors[i], 1, starts, ends,
            &map_id, strides, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    memcpy(ptr, in[i]->data, ends[0]);
    tivxUnmapTensorPatch(obj->input_tensors[i], map_id);
  }
  if(prms->stats != NULL) get_time_u64(&prms->stats->cpIn_time_end);

  debug_printf("TVM_RT_OVX: Running Graph ... \n");
  if(prms->stats != NULL) get_time_u64(&prms->stats->proc_time_start);
  status = vxProcessGraph(obj->graph);
  if(prms->stats != NULL) get_time_u64(&prms->stats->proc_time_end);

  debug_printf("TVM_RT_OVX: Copying outputs ... \n");
  if(prms->stats != NULL) get_time_u64(&prms->stats->cpOut_time_start);
  for (int32_t i = 0; i < obj->num_output_tensors; i++)
  {
    starts[0]  = 0;
    ends[0] = prms->tensors_params[obj->num_input_tensors+i].size_in_bytes;
    strides[0] = sizeof(vx_uint8);
    debug_printf("TVM_RT_OVX:   [%d] start=%d, end=%d\n", i, starts[0],ends[0]);
    status = tivxMapTensorPatch(obj->output_tensors[i], 1, starts, ends,
            &map_id, strides, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    memcpy(out[i]->data, ptr, ends[0]);
    tivxUnmapTensorPatch(obj->output_tensors[i], map_id);
  }
  if(prms->stats != NULL) get_time_u64(&prms->stats->cpOut_time_end);

  if(status!=VX_SUCCESS)
  {
    printf("ERROR: Running TVM graph ... Failed !!!\n");
  }
  debug_printf("TVM_RT_OVX: Ran TVM graph ... Done.\n");

  if(prms->tvm_rt_debug_level > 1)
  {
    status = vxMapUserDataObject(obj->traceData, 0, prms->tvm_rt_trace_size, &map_id,
                                 (void **)&trace_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
    if (status == VX_SUCCESS)
    {
      memcpy(&trace_written_size, trace_ptr, sizeof(int32_t));
      FILE *f_trace = fopen("tvm_c7x.trace", "wb");
      if (f_trace != NULL)
      {
        fwrite(trace_ptr, trace_written_size, 1, f_trace);
        fclose(f_trace);
        debug_printf("Saved %d bytes in trace file tvm_c7x.trace\n", trace_written_size);
      }
      else
      {
        printf("ERROR: Cannot write trace to tvm_c7x.trace\n");
      }
      vxUnmapUserDataObject(obj->traceData, map_id);
    }
    else
    {
      printf("ERROR: Failed to map trace userdata object\n");
    }
  }

  return status;
}

