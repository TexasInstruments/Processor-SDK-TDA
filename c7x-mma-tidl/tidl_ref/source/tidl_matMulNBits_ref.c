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

/**
 *  \file tidl_matMulNBits_ref.c
 *
 *  \brief This file defines the reference kernel for the MatMulNBits operator.
 *
 *  MatMulNBits computes Y = A * dequant(B)^T (+ bias), where B is an
 *  block-wise quantized weight matrix (4 or 8 bits) kept packed and
 *  dequantized on-the-fly:
 *      dequant_w[n,k] = (unpack(B,n,k) - zeroPoint(n,block)) * scale(n,block)
 *  with block = k / block_size.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <math.h>
#include "tidl_matMulNBits_ref.h"

/**
 * @brief  Dequantize a single packed weight element B[n,k].
 *
 * @param B            : base pointer to packed weights (uint8)
 * @param zp           : base pointer to zero points (uint8 packed / float), or NULL
 * @param scales       : base pointer to scales (float)
 * @param n            : output-feature index  [0, N)
 * @param k            : reduction index        [0, K)
 * @param kBlocks      : number of blocks along K (ceil(K/blockSize))
 * @param blockSize    : quantization block size
 * @param bits         : bit-width (4 or 8)
 * @param zeroPointType: 0 = absent (default 2^(bits-1)), 1 = packed uint8, 2 = float
 *
 * @return dequantized float weight value
 */
static float TIDL_matMulNBitsDequantWeight(const uint8_t *B,
                                           const void    *zp,
                                           const float   *scales,
                                           int32_t        n,
                                           int32_t        k,
                                           int32_t        kBlocks,
                                           int32_t        blockSize,
                                           int32_t        bits,
                                           int32_t        zeroPointType)
{
    int32_t block      = k / blockSize;
    int32_t offInBlock = k % blockSize;
    int32_t blobSize   = (blockSize * bits) / 8; /* bytes per (n,block) blob */
    int32_t blobOff    = ((n * kBlocks) + block) * blobSize;
    int32_t q;
    float   qZp;

    if (bits == 4)
    {
        uint8_t byteVal = B[blobOff + (offInBlock >> 1)];
        q = (offInBlock & 1) ? ((byteVal >> 4) & 0xF) : (byteVal & 0xF);
    }
    else /* bits == 8 */
    {
        q = B[blobOff + offInBlock];
    }

    if (zeroPointType == 1)
    {
        /* Packed uint8 zero points: rowBytes = ceil(kBlocks * bits / 8). */
        const uint8_t *zpu = (const uint8_t *)zp;
        if (bits == 4)
        {
            int32_t rowBytes = ((kBlocks * bits) + 7) / 8;
            uint8_t byteVal  = zpu[(n * rowBytes) + (block >> 1)];
            qZp = (float)((block & 1) ? ((byteVal >> 4) & 0xF) : (byteVal & 0xF));
        }
        else /* bits == 8 */
        {
            qZp = (float)zpu[(n * kBlocks) + block];
        }
    }
    else if (zeroPointType == 2)
    {
        /* Unpacked float zero points. */
        const float *zpf = (const float *)zp;
        qZp = zpf[(n * kBlocks) + block];
    }
    else
    {
        /* Absent: default symmetric zero point. */
        qZp = (float)(1 << (bits - 1));
    }

    return ((float)q - qZp) * scales[(n * kBlocks) + block];
}

int32_t TIDL_MatMulNBitsRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
    int32_t status = IALG_EOK;
    (void)layerIdx;

    sTIDL_Network_t            *net           = commonParams->net;
    sTIDL_DataParams_t         *inDataParams  = &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
    sTIDL_DataParams_t         *outDataParams = &tidlLayer->outData;
    sTIDL_MatMulNBitsParams_t  *params        = &tidlLayer->layerParams.matMulNBitsParams;

    const float32_tidl *inPtr  = (const float32_tidl *)inPtrs[0];
    float32_tidl       *outPtr = (float32_tidl *)outPtrs[0];

    const uint8_t *B      = (const uint8_t *)get_int8_t_pointer((int8_t *)net, params->weights);
    const float   *scales = (const float *)get_int8_t_pointer((int8_t *)net, params->scales);
    const void    *zp     = (params->zeroPointType != 0) ?
                              (const void *)get_int8_t_pointer((int8_t *)net, params->zeroPoints) : NULL;
    const float   *bias   = (params->isBias != 0) ?
                              (const float *)get_int8_t_pointer((int8_t *)net, params->bias) : NULL;

    int32_t K             = params->K;
    int32_t N             = params->N;
    int32_t kBlocks       = params->kBlocks;
    int32_t blockSize     = params->blockSize;
    int32_t bits          = params->bits;
    int32_t zeroPointType = params->zeroPointType;

    if (inDataParams->elementType != TIDL_SinglePrecFloat)
    {
        tidl_printf(0, "TIDL_MatMulNBits supports only float input !!!\n");
        return TIDL_ERR_FAILURE;
    }

    for (int32_t batchIdx = 0; batchIdx < inDataParams->dimValues[TIDL_DIM_BATCH]; batchIdx++)
    {
        for (int32_t dim1Idx = 0; dim1Idx < inDataParams->dimValues[TIDL_DIM_DIM1]; dim1Idx++)
        {
            for (int32_t dim2Idx = 0; dim2Idx < inDataParams->dimValues[TIDL_DIM_DIM2]; dim2Idx++)
            {
                for (int32_t chIdx = 0; chIdx < inDataParams->dimValues[TIDL_DIM_NUMCH]; chIdx++)
                {
                    for (int32_t heightIdx = 0; heightIdx < inDataParams->dimValues[TIDL_DIM_HEIGHT]; heightIdx++)
                    {
                        int32_t inRowOffset =
                            (batchIdx  * inDataParams->pitch[TIDL_ROI_PITCH]) +
                            (dim1Idx   * inDataParams->pitch[TIDL_DIM1_PITCH]) +
                            (dim2Idx   * inDataParams->pitch[TIDL_DIM2_PITCH]) +
                            (chIdx     * inDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                            (heightIdx * inDataParams->pitch[TIDL_LINE_PITCH]);

                        int32_t outRowOffset =
                            (batchIdx  * outDataParams->pitch[TIDL_ROI_PITCH]) +
                            (dim1Idx   * outDataParams->pitch[TIDL_DIM1_PITCH]) +
                            (dim2Idx   * outDataParams->pitch[TIDL_DIM2_PITCH]) +
                            (chIdx     * outDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                            (heightIdx * outDataParams->pitch[TIDL_LINE_PITCH]);

                        for (int32_t n = 0; n < N; n++)
                        {
                            float acc = (bias != NULL) ? bias[n] : 0.0f;
                            for (int32_t k = 0; k < K; k++)
                            {
                                float w = TIDL_matMulNBitsDequantWeight(B, zp, scales, n, k,
                                                                        kBlocks, blockSize, bits,
                                                                        zeroPointType);
                                acc += (float)inPtr[inRowOffset + k] * w;
                            }
                            outPtr[outRowOffset + n] = acc;
                        }
                    }
                }
            }
        }
    }

    return status;
}
