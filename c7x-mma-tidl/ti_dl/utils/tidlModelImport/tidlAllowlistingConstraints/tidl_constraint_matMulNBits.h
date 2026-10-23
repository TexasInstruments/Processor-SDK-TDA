/*
*
* Copyright (c) {2015 - 2026} Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Userize") this software subject to the
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

#include "tidl_constraint.h"

const vector<TidlConstraint> tidlConstraintMatMulNBits =
{
    TIDL_CSTR(
        "Only 4-bit and 8-bit quantization is supported",
        "Only 4-bit and 8-bit quantization is supported",
        "Only 4-bit and 8-bit quantization is supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t bits = layer->layerParams.matMulNBitsParams.bits;
            if((bits != 4) && (bits != 8))
            {
                oss << "Only 4-bit and 8-bit quantization is supported, found bits = " << bits;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "block_size must be a power of 2 and >= 16",
        "block_size must be a power of 2 and >= 16",
        "block_size must be a power of 2 and >= 16",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t blockSize = layer->layerParams.matMulNBitsParams.blockSize;
            if((blockSize < 16) || ((blockSize & (blockSize - 1)) != 0))
            {
                oss << "block_size must be a power of 2 and >= 16, found block_size = " << blockSize;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "K and N must be positive",
        "K and N must be positive",
        "K and N must be positive",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t K = layer->layerParams.matMulNBitsParams.K;
            int32_t N = layer->layerParams.matMulNBitsParams.N;
            if((K <= 0) || (N <= 0))
            {
                oss << "K and N must be positive, found K = " << K << ", N = " << N;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
};
