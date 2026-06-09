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

#ifndef TIDL_ONNXRT_COMMON_H_
#define TIDL_ONNXRT_COMMON_H_

#include "core/session/onnxruntime_c_api.h"
#include "onnx/onnx-ml.proto3.pb.h"
#include "core/providers/tidl/tidl_execution_provider_common.h"
#include "tidl_runtimes_infer_common.h"
#include "itidl_ti.h"

using namespace onnx;
using namespace std;

int32_t TIDL_ortGetType(int64_t ortType, int32_t * type, int32_t * size);
void TIDL_ortUpdateType(int32_t * type);
int32_t TIDL_printOnnxGraphProperties(GraphProto&   onnxGraph);
int32_t TIDL_subgraphRtInvoke(int32_t osrtDebugPrintLevel, OnnxTIDLSubGraphParams * subgraphParams, sTIDL_tidlRtDynamicLoading_t * infer_ops, int32_t isInfer = 0);

extern "C"
{

static map<string, pair<pair<string,string>,pair<string,string>>> TIDL_onnxCompatibilityMap = {
    // {ONNX Version,   {{Min. TIDL Tools Version, Max. TIDL Tools Version},
    //                   {Min. C7x Firmware Version, Max. C7x Firmware Version}}
    // },

    {"1.14.0+10000000",  {{"09_02_06_00", "10_00_00_00"},
                          {"09_02_06_00", "10_00_00_00"}}
    },
    {"1.14.0+10000005",  {{"10_00_00_05", "10_01_00_00"},
                          {"10_00_00_05", "10_01_00_00"}}
    },
    {"1.15.0",           {{"10_01_00_01", "11_02_16_00"},
                          {"10_01_00_01", "11_02_16_00"}}
    },
};
int32_t TIDL_getOutputShape(void * ioBufDescVPtr, int8_t onnxName[], std::vector<int64_t> &shape);
int32_t TIDLEP_getSubGraphStats(OnnxTIDLSubGraphParams * state_subGraph, char **node_name, void **node_data);
}

#endif //TIDL_ONNXRT_COMMON_H_
