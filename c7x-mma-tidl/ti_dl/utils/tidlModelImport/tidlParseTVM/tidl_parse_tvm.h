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

#ifndef TIDL_PARSE_TVM_H_
#define TIDL_PARSE_TVM_H_

#include "ti_dl.h"
#include "tidl_import_config.h"
#include "tidl_import_common.h"
#include "tidl_import_api.h"
#include "tidl_import_diag.h"
#include "tidl_import_common_model_check.h"
#include "tidl_common_utils_infer_import.h"
#include "tidl_runtimes_infer_common.h"

#include <tvm/relay/expr.h>

using namespace std;
using namespace tvm;
using namespace tvm::relay;

#define S(T,I)  (I < sizeof(T)) ? T[I] : 0
// using the same mechanism as onnxrt parse mapping, but renaming it everything in terms of TVM 
#define OpNameStr(T) \
    S(T, 0),S(T, 1),S(T, 2),S(T, 3),S(T, 4),S(T, 5),S(T, 6),S(T, 7),S(T, 8),S(T, 9),\
    S(T,10),S(T,11),S(T,12),S(T,13),S(T,14),S(T,15),S(T,16),S(T,17),S(T,18),S(T,19),\
    S(T,20),S(T,21),S(T,22),S(T,23),S(T,24),S(T,25),S(T,26),S(T,27),S(T,28),S(T,29),\
    S(T,30),S(T,31),S(T,32),S(T,33),S(T,34),S(T,35),S(T,36),S(T,37),S(T,38),S(T,39),\
    S(T,40),S(T,41),S(T,42),S(T,43),S(T,44),S(T,45),S(T,46),S(T,47),S(T,48),S(T,49)

#define INPUT_NOT_REQUIRED (0)
#define INPUT_REQUIRED (1)

static int tidl_relay_debuglevel = 0;
#define RelayDebugPrint(...)  if (tidl_relay_debuglevel > 0) \
                                printf("RelayImportDebug: " __VA_ARGS__)
static int tidl_relay_max_batch_size = 1;

/// @brief Parses TVM layers
struct TidlParseTVM
{
    sTIDL_LayerPC_t layer;

    relay::Call call;
    int32_t index;

    int32_t status;

    /// @brief Constructor of TidlParseTVM
    TidlParseTVM(relay::Call call, int32_t index, sTIDL_LayerPC_t layer);

    /// @brief The parse function: fills in attributes into sTIDL_Layer_PC_t struct
    template <char... S>
    int32_t parse();

    int default_zp = 0;
    float default_scale = 1.0f;

    // Utility Functions
    int32_t saveAllowlistingMetaData(relay::Call call, int32_t numInputs=0);
    bool TIDL_RelayBroadcast(Expr tensor, Expr constant, int32_t &numCh);
    int32_t getTIDLDataTypeFromTVMDataType(int32_t tvmDataType);
    int32_t convertIndicesToInt32(sBuffer_t& weights);
    bool isLayerIONumSupported(sTIDL_LayerPC_t &layer);
};

int32_t TIDL_relayExprDims(Expr expr);

vector<int32_t> TIDL_relayNormalizeShape(Expr e);

bool TIDL_RelayChannelBroadcast(Expr tensor, Expr constant, int32_t &numCh);

int32_t balancePadding(sTIDL_LayerPC_t &layer, int32_t before, int32_t after);

float* NDArrtoFloat(const runtime::NDArray &ndArr, int nelem, int ndarr_zp=0, float ndarr_scale=1.0f);

long long int* NDArrtoInt64(const runtime::NDArray &ndArr, int nelem);

sBuffer_t TIDL_extractConstantTensorData(const relay::Call& call, int arg_index,
                                        const sTIDL_allowlistingMetaData& md, int const_index, int ndarr_zp=0, float ndarr_scale=1.0f);
// helper functions for convolution
template<typename Tvalue>
Tvalue* TIDL_relayGetConstantArr(const relay::Expr &expr, int32_t &len);

template<typename Tvalue>
Tvalue TIDL_relayGetConstantVal(const relay::Expr &expr);

int32_t TIDL_relayNormalizeAxis(int32_t axis, int32_t dims, int32_t *relay_axis=nullptr, int32_t mode=0);

std::string TIDL_relayGetOpName(relay::Call call);

#endif /*TIDL_PARSE_TVM_H_ */