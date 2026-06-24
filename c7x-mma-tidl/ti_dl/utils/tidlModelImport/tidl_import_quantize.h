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


#ifndef TIDL_QUANTIZE_INCLUDE_H_
#define TIDL_QUANTIZE_INCLUDE_H_

#include "ti_dl.h"
#include "tidl_import_config.h"
#include "tidl_import_common.h"
#include "tidl_import_api.h"
#include "tidl_custom_import.h"

#define STATS_COLLECTION_FLOAT (0)
#define STATS_COLLECTION_FIXED_POINT (1)

#define TIDL_MMAV2_SCALE_BITS (8U)
#define TIDL_MMAV2_MAX_SCALE  (1.0 * (1 << TIDL_MMAV2_SCALE_BITS) - 1)
#define TIDL_MMAV2_MAX_SCALE_16  (1.0 * (1 << TIDL_MMAV2_SCALE_BITS) - 1)

#define TIDL_MMAV2_MIN_SCALE_16  ((float)10E-15)
#define TIDL_MMAV2_MIN_SCALE_8  ((float)(2.14 * 10E-9))
#define TIDL_MMAV2_MAX_BIAS_BITS_8 (30U)
#define TIDL_MMAV2_MAX_BIAS_BITS_16 (53U)
#define TIDL_WEIGHT_SCALE_REDUCTION_FACTOR (2.0)
#define TIDL_MAX_SCALE_REPRESENTATION_ERROR ((float)10E-4)
#define TIDL_MAX_SCALE_UPDATE ((float)2.0 * 10E-3)
#define TIDL_SIGNED_ZP_THRESHOLD (-1 * 108)
#define MAX_INT_REP_8BIT ((float) (1 << 6))
#define MAX_INT_REP_16BIT ((float) (1 << 14))

int32_t TIDL_import_quantize(uint32_t layerIndex);

int32_t TIDL_quantStatsFixedOrFloat(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, sTIDL_Network_t * pTIDLNetStructure, 
                                  tidl_import_config * configParams, int32_t statsCollectionType);

int8_t TIDL_isLayerParamsToBeUpdated(int32_t layerType);
int32_t TIDL_increasePrecision(int32_t elementType);
int32_t TIDL_increaseWeightPrecision(sTIDL_LayerPC_t * layer, int32_t weightsElementSizeInBits);
int32_t TIDL_setUpdatedElementType(int32_t layerType, sTIDL_OrgNetwork_t* pOrgTIDLNetStructure, int32_t layerIndex);
int32_t TIDL_updateDataTypesFixedPoint(sTIDL_OrgNetwork_t  * pOrgTIDLNetStructure, tidl_import_config * params, int32_t numLayers,
    std::unordered_map<int32_t, std::function<int32_t(sTIDL_OrgNetwork_t*, int32_t)>>& sTIDL_updateOutElementTypeMap);
void TIDL_setDefaultWeightElementBits(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,tidl_import_config * params,int32_t numLayers);
void TIDL_convert8bitLayersTo16Bit(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,tidl_import_config * params,int32_t numLayers);
int32_t TIDL_asymUpdateNetworkWithConstraints(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                              tidl_import_config * params,
                              int32_t numLayers);

#endif /*TIDL_QUANTIZE_INCLUDE_H_ */

