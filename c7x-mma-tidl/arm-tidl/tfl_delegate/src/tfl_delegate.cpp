/*
* Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
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

#include "itidl_ti.h"
#include "itidl_rt.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "float.h"
#include "dlfcn.h"
#include "assert.h"
#include <string>
#include <vector>
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/context_util.h"

#include "tidl_runtimes_infer_common.h"
#include "tidl_tflrt_common.h"

static int tidlrt_perfstats = 0;
static void __attribute__((constructor)) lib_init()
{
	char *perf_str;

	perf_str = getenv("TIDL_RT_PERFSTATS");
	if(!perf_str)
		tidlrt_perfstats = 0;
	else
		tidlrt_perfstats = atoi(perf_str);
}

namespace tflite {
namespace tfl_delegate {

struct  TfLiteTIDLDelegateOptions {
  int debug_level;
  sTIDL_tidlRtParams_t tidlRtParamsObj;
  char artifacts_folder[TIDLRT_STRING_SIZE];
  sTIDL_tidlRtDynamicLoading_t infer_ops; //rtLibLoadParams;
  int osrtDebugPrintLevel;
  /* default value of target_priority and max_pre_empt_delay is
     set in tflite_plugin_create_delegate() func call */
  /* Int value of priority for selcting the target name:set to 0-7
     0(highest prior)-7(lowest prior)*/ 
  int target_priority;
  /* Maximum Tolerated delay for TIDL pre-emption in milliSecond */
  float max_pre_empt_delay;
  /* C7x core number to be used for inference */
  int core_number;
  /* Core id to start processing from */
  int core_start_idx;
  int flow_ctrl = -1;
  char temp_buffer_dir[TIDLRT_STRING_SIZE];
  char trace_base_name[TIDLRT_STRING_SIZE];
};

constexpr int kMaxTIDLGraphs = 32;

class tidlDelegate {
public:
  // Any initialization code needed
  TfLiteStatus Init(TfLiteContext* context, const TfLiteDelegateParams* params);
  // Any preparation work needed (e.g. allocate buffers)
  TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node);
  // Actual running of the delegate subgraph.
  TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node);
  // ... Add any other methods needed.
  TfLiteStatus CustomData(TfLiteContext* context, TfLiteNode* node, const char *op_,
          char **node_name, void **node_data);
  //Destructor
  ~tidlDelegate();

private:
  TfLiteTIDLDelegateOptions options_;
  const char *subGraphName_;
};

TfLiteStatus tidlDelegate::Init(TfLiteContext* context, const TfLiteDelegateParams* params) 
{
  int status = 0;

  TfLiteTIDLDelegateOptions* options_ptr = nullptr;
  if (params != nullptr && params->delegate != nullptr) 
  {
    options_ptr = reinterpret_cast<TfLiteTIDLDelegateOptions*>(params->delegate->data_);
    if (options_ptr != nullptr) options_ = *options_ptr;
  }
  TIDL_osrtDebugPrint(options_.osrtDebugPrintLevel, "\n ****** In tidlDelegate::Init ****** \n");

  int32_t subgraphId = TIDL_getOneOutputTensorIdx(context, params);

  asprintf(const_cast<char **>(&subGraphName_), "%d", subgraphId);
  
  sTIDL_tidlRtParams_t * subgraphParams = &options_.tidlRtParamsObj;

  /* Setting options used in TIDL_subgraphRtCreate function */
  sTIDL_subgraphRtCreateOptions_t* subgraphRtCreateOptions = (sTIDL_subgraphRtCreateOptions_t*)malloc(sizeof(sTIDL_subgraphRtCreateOptions_t));
  strcpy(subgraphRtCreateOptions->netPath, options_.artifacts_folder);
  subgraphRtCreateOptions->debugLevel = options_.debug_level;
  subgraphRtCreateOptions->osrtDebugPrintLevel = options_.osrtDebugPrintLevel;
  subgraphRtCreateOptions->maxPreEmptDelay = options_.max_pre_empt_delay;
  subgraphRtCreateOptions->targetPriority = options_.target_priority;
  subgraphRtCreateOptions->coreNum = options_.core_number;
  subgraphRtCreateOptions->coreStartIdx = options_.core_start_idx;
  subgraphRtCreateOptions->inferenceMode = -1;
  subgraphRtCreateOptions->flowCtrl = options_.flow_ctrl;
  strcpy(subgraphRtCreateOptions->tempBufferDir, options_.temp_buffer_dir);
  strncpy(subgraphRtCreateOptions->traceBaseName, options_.trace_base_name, TIDLRT_STRING_SIZE - 1);
  subgraphRtCreateOptions->traceBaseName[TIDLRT_STRING_SIZE - 1] = '\0';

  status = TIDL_subgraphRtCreate(subgraphRtCreateOptions, &options_.infer_ops, std::to_string(subgraphId).c_str(), subgraphParams);

  if(subgraphRtCreateOptions != NULL)
  {
    free(subgraphRtCreateOptions);
  }

  return kTfLiteOk;
}

TfLiteStatus tidlDelegate::Prepare(TfLiteContext* context, TfLiteNode* node)
{
  TIDL_osrtDebugPrint(options_.osrtDebugPrintLevel,"\n ****** tidlDelegate::Prepare ****** \n");

  for (auto tensor_index : TfLiteIntArrayView(node->outputs)) 
  {
    if (tensor_index == kTfLiteOptionalTensor) 
    {
      continue;
    }
    TfLiteTensor* tensor = &context->tensors[tensor_index];

    if(tensor->allocation_type == kTfLiteArenaRw)
      TIDL_osrtDebugPrint(options_.osrtDebugPrintLevel, " Outputs Tensor name and id -  %s, %d\n", tensor->name, tensor_index);
  }

  return kTfLiteOk;
}

TfLiteStatus tidlDelegate::Invoke(TfLiteContext* context, TfLiteNode* node)
{
  TIDL_osrtDebugPrint(options_.osrtDebugPrintLevel, "\n ****** tidlDelegate::Invoke ****** \n");
  TfLiteStatus status = kTfLiteOk;

  int outTensorIdx = 0;
  status = TIDL_subgraphRtInvoke(options_.osrtDebugPrintLevel, context, node, &options_.tidlRtParamsObj, &options_.infer_ops, 1);

  return status;
}

tidlDelegate::~tidlDelegate()
{
  TIDL_osrtDebugPrint(options_.osrtDebugPrintLevel, "************ in ~tidlDelegate ************ \n ");
  TIDL_subgraphRtDelete(options_.osrtDebugPrintLevel, &options_.infer_ops, &options_.tidlRtParamsObj);
}

// Create the TfLiteRegistration for the Kernel node which will replace
// the subgraph in the main TfLite graph.
TfLiteRegistration GetTIDLNodeRegistration() {
  // This is the registration for the Delegate Node that gets added to
  // the TFLite graph instead of the subGraph it replaces.
  // It is treated as a an OP node. But in our case
  // Init will initialize the delegate
  // Invoke will run the delegate graph.
  // Prepare for preparing the delegate.
  // Free for any cleaning needed by the delegate.

  TfLiteRegistration kernel_registration = {0};
  kernel_registration.builtin_code = kTfLiteBuiltinDelegate;
  kernel_registration.custom_name = "tidlDelegate";
  kernel_registration.free = [](TfLiteContext* context, void* buffer) -> void {
    delete reinterpret_cast<tidlDelegate*>(buffer);
  };

  kernel_registration.init = [](TfLiteContext* context, const char* buffer,
                                   size_t) -> void* {
    // In the node init phase, initialize MyDelegate instance
    const TfLiteDelegateParams* params =
        reinterpret_cast<const TfLiteDelegateParams*>(buffer);
    tidlDelegate* delegate = new tidlDelegate;
    if (delegate->Init(context, params) != kTfLiteOk) {
      return nullptr;
    }
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
    tidlDelegate* kernel = reinterpret_cast<tidlDelegate*>(node->user_data);
    return kernel->CustomData(context, node, op_name, node_name, node_data);
  };

  return kernel_registration;
}

TfLiteStatus tidlDelegate::CustomData(TfLiteContext* context, TfLiteNode* node, const char *op_,
        char **node_name, void **node_data)
{
    std::string op = std::string(op_);

    if(op == "ddr_stats") 
    {
        std::pair<uint64_t, uint64_t> *v = new std::pair<uint64_t, uint64_t>;

        options_.infer_ops.TIDLRT_getDdrStats(&v->first, &v->second);

        *node_data = static_cast<void *>(v);
    } 
    else if(op == "perf_stats") 
    {
        if(!tidlrt_perfstats)  return kTfLiteError;

        std::vector<uint64_t> *v = new std::vector<uint64_t>();

        sTIDLRT_PerfStats_t * stats = (sTIDLRT_PerfStats_t *)options_.tidlRtParamsObj.stats;

        v->push_back(uint64_t(stats->cpIn_time_start));
        v->push_back(uint64_t(stats->cpIn_time_end));
        v->push_back(uint64_t(stats->proc_time_start));
        v->push_back(uint64_t(stats->proc_time_end));
        v->push_back(uint64_t(stats->cpOut_time_start));
        v->push_back(uint64_t(stats->cpOut_time_end));

        *node_data = static_cast<void *>(v);
    } else {
        return kTfLiteError;
    }

    *node_name = const_cast<char *>(subGraphName_);
    return kTfLiteOk;
}

TfLiteStatus DelegatePrepareInfer(TfLiteContext* context, TfLiteDelegate* delegate) 
{
  TfLiteTIDLDelegateOptions* options_ptr = reinterpret_cast<TfLiteTIDLDelegateOptions*>(delegate->data_);
  
  TIDL_osrtDebugPrint(options_ptr->osrtDebugPrintLevel, "\n ****** In DelegatePrepare ****** \n");
  
  //Get list of supported nodes available from compilation
  std::vector<std::vector<int>> supportedNodeGroups = TIDL_readAllowedNodesList(options_ptr->artifacts_folder);
  
  std::vector<int> supported_nodes(1);  // Reserve 1 element, since we need first element to be size, will be updated later.
  for(int i = 0; i < supportedNodeGroups.size(); i++)
  {
    supported_nodes.insert(supported_nodes.end(), supportedNodeGroups[i].begin(), supportedNodeGroups[i].end());
  }

  TfLiteIntArray* plan;
  TF_LITE_ENSURE_STATUS(context->GetExecutionPlan(context, &plan));

  // Set first element to the number of nodes to replace.
  supported_nodes[0] = supported_nodes.size() - 1;
  printf("\n Number of subgraphs:%ld , %d nodes delegated out of %d nodes \n \n", supportedNodeGroups.size(), supported_nodes[0], plan->size);

  TfLiteRegistration TIDL_kernel_registration = GetTIDLNodeRegistration();

  return context->ReplaceNodeSubsetsWithDelegateKernels(context, TIDL_kernel_registration, reinterpret_cast<TfLiteIntArray*>(supported_nodes.data()), delegate);

  return kTfLiteOk;
}

TfLiteDelegate* TfLiteTIDLDelegateCreate(TfLiteTIDLDelegateOptions* options)
{
  TfLiteDelegate* delegate = new TfLiteDelegate;

  options->infer_ops.lib = dlopen("libvx_tidl_rt.so", RTLD_LAZY | RTLD_GLOBAL);
  assert(options->infer_ops.lib);
  
  TIDL_rtLoadSymbols(&options->infer_ops);

  delegate->data_ = (void*)options;
  delegate->flags = kTfLiteDelegateFlagsNone;
  delegate->Prepare = DelegatePrepareInfer;
  delegate->CopyFromBufferHandle = nullptr;
  delegate->CopyToBufferHandle = nullptr;
  delegate->FreeBufferHandle = nullptr;
  return delegate;
}

extern "C"{
TfLiteDelegate* tflite_plugin_create_delegate(char** options_keys,
                                              char** options_values,
                                              size_t num_options,
                                              void (*error_handler)(const char*)) 
{
  tfl_delegate::TfLiteTIDLDelegateOptions *options =
    (tfl_delegate::TfLiteTIDLDelegateOptions*)malloc(sizeof(tfl_delegate::TfLiteTIDLDelegateOptions));

  strcpy(options->artifacts_folder, "");
  /* setting teh default values*/
  options->debug_level = 0;
  options->target_priority = 0;
  options->max_pre_empt_delay = FLT_MAX;
  options->core_number = 1;
  options->core_start_idx = 1;
  options->flow_ctrl = -1;
  strcpy(options->temp_buffer_dir, "/dev/shm");
  options->trace_base_name[0] = '\0';

  char deny_list[512];

  for (uint32_t idx = 0; idx < num_options; idx++)
  {
    if (strcmp("artifacts_folder", options_keys[idx]) == 0)
    {
        strcpy(options->artifacts_folder, options_values[idx]);
    }
    if (strcmp("debug_level", options_keys[idx]) == 0)
    {
        sscanf(options_values[idx], "%d", &options->debug_level);
    }
    if (strcmp("priority", options_keys[idx]) == 0)
    {
        sscanf(options_values[idx], "%d",  &options->target_priority);
    }
    if (strcmp("max_pre_empt_delay", options_keys[idx]) == 0)
    {
        sscanf(options_values[idx], "%f", &options->max_pre_empt_delay);
    }
    if (strcmp("core_number", options_keys[idx]) == 0)
    {
        sscanf(options_values[idx], "%d",  &options->core_number);
    }
    if (strcmp("core_start_idx", options_keys[idx]) == 0)
    {
        sscanf(options_values[idx], "%d",  &options->core_start_idx);
    }
    if (strcmp("advanced_options:flow_ctrl", options_keys[idx]) == 0)
    {
        sscanf(options_values[idx], "%d",  &options->flow_ctrl);
        if(options->flow_ctrl != 0 && options->flow_ctrl != 1 && options->flow_ctrl != 12)
        {
          printf("WARNING : Invalid flow control %d given. Available options are 0, 1 & 12. It will be ignored \n", options->flow_ctrl);
          options->flow_ctrl = -1;
        }
    }
    if (strcmp("advanced_options:trace_base_name", options_keys[idx]) == 0)
    {
        strncpy(options->trace_base_name, options_values[idx], TIDLRT_STRING_SIZE - 1);
        options->trace_base_name[TIDLRT_STRING_SIZE - 1] = '\0';
    }
    #ifdef x86_64
    if (strcmp("advanced_options:temp_buffer_dir", options_keys[idx]) == 0)
    {
        strcpy(options->temp_buffer_dir, options_values[idx]);
        if(!TIDL_checkIsDir(options->temp_buffer_dir)) 
        {
          printf("WARNING : Directory to redirect temporary buffers (temp_buffer_dir - %s) does not exist, /dev/shm will be used instead \n", options->temp_buffer_dir);
          strcpy(options->temp_buffer_dir, "/dev/shm");
        }
        if(strlen(options->temp_buffer_dir) > 64U)
        {
          printf("WARNING : Directory to redirect temporary buffers (temp_buffer_dir - %s) exceeds the maximum path length (64), /dev/shm will be used instead\n", options->temp_buffer_dir);
          strcpy(options->temp_buffer_dir, "/dev/shm");
        }
    }
    #endif
  }
  if(options->target_priority < 0 || options->target_priority > 7)
  {
    printf("ERROR : Please provide priority betweeen 0-7 provided:%d... exiting \n",options->target_priority);
    exit(-1);
  }

  if(strcmp(options->artifacts_folder, "") == 0)
  {
    printf("ERROR : Please provide artifacts folders path ... exiting \n");
    exit(-1);
  }

  strcat(options->artifacts_folder, "/");

  options->osrtDebugPrintLevel = (options->debug_level == 0) ? 0 : 1;

  if(options->trace_base_name[0] != '\0')
  {
    TIDL_osrtDebugPrint(options->osrtDebugPrintLevel, "trace_base_name      = %s \n", options->trace_base_name);
  }

  TfLiteDelegate *tidl_delegate = TfLiteTIDLDelegateCreate(options);

  return tidl_delegate;

}

void tflite_plugin_destroy_delegate(TfLiteDelegate* delegate) 
{
  free(delegate->data_);
  delete delegate;
}

}
}  //namespace tfl_delegate
}  // namespace tflite


