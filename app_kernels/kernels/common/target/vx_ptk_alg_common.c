/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "VX/vx.h"
#include <vx_ptk_alg_common.h>

static void tivxPtkAlgCleanup(
       tivx_ptk_alg_if_cntxt  * algCntxt)
{
    PTK_Api_MemoryRsp     * rsp;
    int32_t                 r;
    int32_t                 i;

    for (r = 0; r < algCntxt->numValidRsp; r++ )
    {
        rsp = &algCntxt->memRsp[r];

        for (i = 0; i < rsp->numBlks; i++ )
        {
            /* Release the allocated memory blocks. */
            tivxMemFree(rsp->blks[i].mem,
                        rsp->blks[i].size,
                        TIVX_MEM_EXTERNAL);
        }

    } /* for (r = 0; r < algCntxt->numValidRsp; r++ ) */

    /* Release the local algorithm context. */
    tivxMemFree(algCntxt,
                sizeof(tivx_ptk_alg_if_cntxt),
                TIVX_MEM_EXTERNAL);

    return;
}

tivx_ptk_alg_if_cntxt * tivxPtkAlgCommonCreate(
       tivx_target_kernel_instance kernel,
       PTK_Api_MemoryReq         * memReq,
       uint32_t                    numReq)
{
    tivx_ptk_alg_if_cntxt * algCntxt;
    PTK_Api_MemoryReq     * req;
    PTK_Api_MemoryRsp     * rsp;
    int32_t                 r;
    int32_t                 i;

    if (numReq > VX_PTK_ALG_MAX_NUM_REQ)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "numReq cannot exceed %d.\n",
                 VX_PTK_ALG_MAX_NUM_REQ);

        return NULL;
    }

    algCntxt = tivxMemAlloc(sizeof(tivx_ptk_alg_if_cntxt), TIVX_MEM_EXTERNAL);

    if (NULL == algCntxt)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Failed to allocate memory block of size %d bytes\n",
                 sizeof(tivx_ptk_alg_if_cntxt));

        return NULL;
    }

    algCntxt->numValidRsp = 0;

    tivxSetTargetKernelInstanceContext(kernel,
                                       algCntxt,
                                       sizeof(tivx_ptk_alg_if_cntxt));

    for (r = 0; r < numReq; r++ )
    {
        req = &memReq[r];
        rsp = &algCntxt->memRsp[r];

        rsp->numBlks = 0;
        algCntxt->numValidRsp++;

        for (i = 0; i < req->numBlks; i++ )
        {
            void  * mem;

            mem = tivxMemAlloc(req->blks[i].size, TIVX_MEM_EXTERNAL);

            if (NULL == mem)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "Failed to allocate memory block of size %d bytes\n",
                         req->blks[i].size);

                tivxPtkAlgCleanup(algCntxt);

                return NULL;

            } /* if (NULL == mem) */

            rsp->numBlks++;
            rsp->blks[i].size    = req->blks[i].size;
            rsp->blks[i].memType = PTK_Api_MemoryType_LVL4;
            rsp->blks[i].mem     = mem;

        } /* for (i = 0; i < req->numBlks; i++ ) */

    } /* for (r = 0; r < numReq; r++ ) */

    return algCntxt;
}

vx_status tivxPtkAlgCommonDelete(
       tivx_target_kernel_instance kernel)
{
    tivx_ptk_alg_if_cntxt * algCntxt = NULL;
    vx_status               status = VX_SUCCESS;
    uint32_t                size = 0;

    /*free all memory */
    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&algCntxt, &size);

    if ((VX_SUCCESS == status) &&
        (NULL != algCntxt)     &&
        (sizeof(tivx_ptk_alg_if_cntxt) == size))
    {
        PTK_Api_MemoryRsp * rsp;
        int32_t             i;
        int32_t             j;

        for (i = 0; i < algCntxt->numValidRsp; i++ )
        {
            rsp = &algCntxt->memRsp[i];

            for (j = 0; j < rsp->numBlks; j++ )
            {
                tivxMemFree(rsp->blks[j].mem,
                            rsp->blks[j].size,
                            TIVX_MEM_EXTERNAL);

            } /* for (j = 0; j < rsp->numBlks; j++ ) */

        } /* for (i = 0; i < algCntxt->numValidRsp; i++ ) */

        tivxMemFree(algCntxt,
                    sizeof(tivx_ptk_alg_if_cntxt),
                    TIVX_MEM_EXTERNAL);
    }
    else
    {
        status = VX_FAILURE;
    }

    return status;
}

