/*
*
* Copyright (c) 2022-2026 Texas Instruments Incorporated
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

#include "vx_dma_transfers.h"

vx_status dma_create(DMAObj *dmaObj, vx_size transfer_type, vx_uint32 dma_ch)
{
    vx_status status = VX_SUCCESS;

    dmaObj->transfer_type = transfer_type;

    memset(&dmaObj->tfrPrms, 0, sizeof(app_udma_copy_nd_prms_t));

    dmaObj->icnt1_next = 0;
    dmaObj->icnt2_next = 0;
    dmaObj->icnt3_next = 0;

    dmaObj->dma_ch = dma_ch;

    if(transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        dmaObj->udmaChHdl = appUdmaCopyNDGetHandle(dma_ch);
#endif
    }
    else
    {
        dmaObj->udmaChHdl = NULL;
    }

    return status;
}

vx_status dma_transfer_trigger(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDTrigger(dmaObj->udmaChHdl);
#endif
    }
    else
    {
        app_udma_copy_nd_prms_t *tfrPrms;
        vx_uint32 icnt1, icnt2, icnt3;
        vx_uint32 num_bytes = 1;

        tfrPrms = (app_udma_copy_nd_prms_t *)&dmaObj->tfrPrms;

        /* This is for case where for every trigger ICNT0 * ICNT1 bytes get transferred */
        icnt3 = dmaObj->icnt3_next;
        icnt2 = dmaObj->icnt2_next;
        icnt1 = dmaObj->icnt1_next;

        /* As C66 is a 32b processor, address will be truncated to use only lower 32b address */
        /* So user is responsible for providing correct address */
#ifndef x86_64
        vx_uint8 *pSrcNext = (vx_uint8 *)((uint32_t)tfrPrms->src_addr + (icnt3 * tfrPrms->dim3) + (icnt2 * tfrPrms->dim2));
        vx_uint8 *pDstNext = (vx_uint8 *)((uint32_t)tfrPrms->dest_addr + (icnt3 * tfrPrms->ddim3) + (icnt2 * tfrPrms->ddim2));
#else
        vx_uint8 *pSrcNext = (vx_uint8 *)(tfrPrms->src_addr + (icnt3 * tfrPrms->dim3) + (icnt2 * tfrPrms->dim2));
        vx_uint8 *pDstNext = (vx_uint8 *)(tfrPrms->dest_addr + (icnt3 * tfrPrms->ddim3) + (icnt2 * tfrPrms->ddim2));
#endif

        if((tfrPrms->eltype == 1) || (tfrPrms->eltype == 0))
        {
            /* Indicate 1 byte per element for transferring 8bit data */
            num_bytes = 1;
        }
        else if(tfrPrms->eltype == 2)
        {
            /* Indicate 2 bytes per element for transferring 16bit data */
            num_bytes = 2;
        }
        else if(tfrPrms->eltype == 3)
        {
            /* Indicate 3 bytes per element for transferring 24bit data */
            num_bytes = 3;
        }
        else if(tfrPrms->eltype == 4)
        {
            /* Indicate 4 bytes per element for transferring 32bit data */
            num_bytes = 4;
        }

        for(icnt1 = 0; icnt1 < tfrPrms->icnt1; icnt1++)
        {
            memcpy(pDstNext, pSrcNext, (tfrPrms->icnt0 * num_bytes));

            pSrcNext += tfrPrms->dim1;
            pDstNext += tfrPrms->ddim1;
        }

        icnt2++;

        if(icnt2 == tfrPrms->icnt2)
        {
            icnt2 = 0;
            icnt3++;
        }

        dmaObj->icnt3_next = icnt3;
        dmaObj->icnt2_next = icnt2;
    }

    return status;
}

vx_status dma_transfer_wait(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDWait(dmaObj->udmaChHdl);
#endif
    }

    return status;
}

vx_status dma_init(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDInit(dmaObj->udmaChHdl, &dmaObj->tfrPrms);
#endif
    }
    else
    {
        dmaObj->icnt1_next = 0;
        dmaObj->icnt2_next = 0;
        dmaObj->icnt3_next = 0;
    }

    return status;
}

vx_status dma_deinit(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDDeinit(dmaObj->udmaChHdl);
#endif
    }
    else
    {
        dmaObj->icnt1_next = 0;
        dmaObj->icnt2_next = 0;
        dmaObj->icnt3_next = 0;
    }
    return status;
}

vx_status dma_delete(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    dmaObj->udmaChHdl = NULL;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        int32_t retVal = appUdmaCopyNDReleaseHandle(dmaObj->dma_ch);
        if(retVal != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to release DMA handle %d\n", dmaObj->dma_ch);
            status = VX_FAILURE;
        }
#endif
    }

    return status;
}
