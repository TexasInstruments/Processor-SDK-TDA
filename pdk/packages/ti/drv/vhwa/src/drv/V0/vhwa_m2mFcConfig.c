/**
 *   Copyright (c) Texas Instruments Incorporated 2021
 *   All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

/**
 *  \file vhwa_m2mFcConfig.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mFcDrvPriv.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define VHWA_GRAPH_CTRL_INDEX_INVALID      (0xFFu)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static uint32_t Vhwa_FcIspareNeeded(const Vhwa_M2mFcHandleObj *hObj,
                                    uint64_t startPort);
static uint32_t Vhwa_FcGetVissCtrlIdx(uint32_t outIdx,
                                        uint32_t *prodId);
static void Vhwa_m2mFcSetupVissCtrl(Vhwa_M2mFcHandleObj *hObj,
                                        uint32_t vissCtrlProdIdx,
                                        uint32_t vissOutProdIdx,
                                        uint32_t consCfgProdId,
                                        uint32_t consId);
static int32_t Vhwa_m2mFcDrvGetMscOutIdx(uint64_t startPort,
                                         uint32_t *outIdx);
static int32_t Vhwa_m2mFcSetupMscCtrl(Vhwa_M2mFcHandleObj *hObj,
                                    uint32_t mscCtrlProdIdx,
                                    uint32_t mscOutProdIdx,
                                    uint32_t consCfgProdId,
                                    uint32_t curNode,
                                    uint32_t nextNode);
static void Vhwa_FcGetMscConsId(uint64_t port, uint32_t *consId);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static uint32_t Vhwa_FcIspareNeeded(const Vhwa_M2mFcHandleObj *hObj,
                                    uint64_t startPort)
{
    uint32_t retVal = UFALSE;
    uint32_t idx;

    for (idx = 0; idx < hObj->graphObj.numEdges; idx++)
    {
        if(startPort == hObj->graphObj.graphEdgeObj[idx].startPort)
        {
            if(VHWA_FC_PORT_DDR ==
                hObj->graphObj.graphEdgeObj[idx].endPort)
            {
                retVal = UTRUE;
                break;
            }
        }
    }
    return retVal;
}

static void Vhwa_FcGetMscConsId(uint64_t port, uint32_t *consId)

{
    switch (port)
    {
        case VHWA_FC_PORT_MSC0_IN_0:
        {
            *consId = CSL_HTS_CONS_IDX_MSC0_CH0;
            break;
        }
        #if !defined (VHWA_VPAC_IP_REV_VPAC)
        case VHWA_FC_PORT_MSC0_IN_1:
        {
            *consId = CSL_HTS_CONS_IDX_MSC0_CH1;
            break;
        }
        #endif
        case VHWA_FC_PORT_MSC1_IN_0:
        {
            *consId = CSL_HTS_CONS_IDX_MSC1_CH0;
            break;
        }
        #if !defined (VHWA_VPAC_IP_REV_VPAC)
        case VHWA_FC_PORT_MSC1_IN_1:
        {
            *consId = CSL_HTS_CONS_IDX_MSC1_CH1;
            break;
        }
        #endif
        default:
        {
            break;
        }
    }

}

static uint32_t Vhwa_FcGetVissCtrlIdx(uint32_t outIdx,
                                        uint32_t *prodId)
{
    uint32_t ctrlIdx;

    ctrlIdx = VHWA_GRAPH_CTRL_INDEX_INVALID;

    switch(outIdx)
    {
        case 0:
        {
            ctrlIdx = 0u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_Y12;
            break;
        }
        case 1:
        {
            ctrlIdx = 1u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_UV12;
            break;
        }
        case 2:
        {
            ctrlIdx = 2u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_Y8;
            break;
        }
        case 3:
        {

            ctrlIdx = 3u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_UV8;
            break;
        }
        case 4:
        {
            ctrlIdx = 4u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_S8;
            break;
        }
        case 5:
        {
            ctrlIdx = 5u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_H3A;
            break;
        }
        case 6:
        {
            ctrlIdx = 6u;
            *prodId = CSL_HTS_PROD_IDX_VISS0_SP;
            break;
        }        
        default:
        {
            /*Do Nothing*/
            break;
        }
    }

    return ctrlIdx;
}

static void Vhwa_m2mFcSetupVissCtrl(Vhwa_M2mFcHandleObj *hObj,
                                        uint32_t vissCtrlProdIdx,
                                        uint32_t vissOutProdIdx,
                                        uint32_t consCfgProdId,
                                        uint32_t consId)
{
    uint32_t                instCnt, chCnt;
    hObj->vissFcConPrms.ctrlIdx = vissCtrlProdIdx;

    if(CSL_HTS_CONS_IDX_MSC0_CH0 == consId)
    {
        instCnt = 0u;
        chCnt = 0u;
    }
    #if !defined (VHWA_VPAC_IP_REV_VPAC)
    else if(CSL_HTS_CONS_IDX_MSC0_CH1 == consId)
    {
        instCnt = 0u;
        chCnt = 1u;
    }
    #endif
    else if(CSL_HTS_CONS_IDX_MSC1_CH0 == consId)
    {
        instCnt = 1u;
        chCnt = 0u;
    }
    #if !defined (VHWA_VPAC_IP_REV_VPAC)
    else if(CSL_HTS_CONS_IDX_MSC1_CH1 == consId)
    {
        instCnt = 1u;
        chCnt = 1u;
    }
    #endif
    else
    {
        /* Do Nothing*/
    }

    #if defined (VHWA_VPAC_IP_REV_VPAC)
    if(consId == CSL_HTS_CONS_IDX_MSC0_CH0)
    #else
    if((consId == CSL_HTS_CONS_IDX_MSC0_CH0) || (consId == CSL_HTS_CONS_IDX_MSC0_CH1))
    #endif
    {
        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].enable = UFALSE;
        hObj->msc0FcGetPrms.htsCfg->consCfg[chCnt].prodId = consCfgProdId;

        hObj->vissFcConPrms.outputIdx = vissOutProdIdx;
        hObj->vissFcConPrms.sl2Addr = hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2Addr[instCnt][chCnt];
        hObj->vissFcConPrms.sl2Depth = hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2BuffDepth[instCnt][chCnt];
        hObj->vissFcConPrms.sl2Pitch = hObj->msc0FcGetPrms.inSl2Pitch[instCnt];

        hObj->vissFcConPrms.htsConsId = consId;

        hObj->vissFcConPrms.htsThreshold =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].threshold;;
        hObj->vissFcConPrms.htsDmaProdPreLoad =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].cntPreLoad;;
        hObj->vissFcConPrms.htsDmaProdPostLoad =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].cntPostLoad;;
        hObj->vissFcConPrms.htsDmaProdCntDec =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].countDec;

     hObj->vissFcConPrms.paEnable =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.enable;
     hObj->vissFcConPrms.paEnableBufCtrl =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.enableBufCtrl;

     hObj->vissFcConPrms.paPsMaxCnt =
                            hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.psMaxCnt;
     hObj->vissFcConPrms.paCsMaxCnt =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.csMaxCnt;

     hObj->vissFcConPrms.paEnableDecCtrl =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.enableDecCtrl;
     hObj->vissFcConPrms.paCountDec =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.countDec;
    }
    #if defined (VHWA_VPAC_IP_REV_VPAC)
    else if(consId == CSL_HTS_CONS_IDX_MSC1_CH0)
    #else
    else if((consId == CSL_HTS_CONS_IDX_MSC1_CH0) || (consId == CSL_HTS_CONS_IDX_MSC1_CH1))
    #endif
    {
        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].enable = UFALSE;
        hObj->msc1FcGetPrms.htsCfg->consCfg[chCnt].prodId = consCfgProdId;

        hObj->vissFcConPrms.outputIdx = vissOutProdIdx;
        hObj->vissFcConPrms.sl2Addr = hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2Addr[instCnt][chCnt];
        hObj->vissFcConPrms.sl2Depth = hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2BuffDepth[instCnt][chCnt];
        hObj->vissFcConPrms.sl2Pitch = hObj->msc1FcGetPrms.inSl2Pitch[instCnt];

        hObj->vissFcConPrms.htsConsId = consId;

        hObj->vissFcConPrms.htsThreshold =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].threshold;
        hObj->vissFcConPrms.htsDmaProdPreLoad =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].cntPreLoad;
        hObj->vissFcConPrms.htsDmaProdPostLoad =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].cntPostLoad;
        hObj->vissFcConPrms.htsDmaProdCntDec =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].countDec;

     hObj->vissFcConPrms.paEnable =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.enable;
     hObj->vissFcConPrms.paEnableBufCtrl =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.enableBufCtrl;

     hObj->vissFcConPrms.paPsMaxCnt =
                            hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.psMaxCnt;
     hObj->vissFcConPrms.paCsMaxCnt =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.csMaxCnt;

     hObj->vissFcConPrms.paEnableDecCtrl =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.enableDecCtrl;
     hObj->vissFcConPrms.paCountDec =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[chCnt].paCfg.countDec;
    }
    else
    {
        /* Do Nothing*/
    }
}

int32_t Vhwa_m2mFcDrvConfigVissSch(Vhwa_M2mFcHandleObj *hObj,
                                 const Vhwa_M2mFcGraphNodeInfo *node)
{
    int32_t status = FVID2_SOK;
    uint32_t idx, idx3;
    uint32_t vissOutIdx, vissCtrlIdx, edgeFound, consCfgProdId;
    uint32_t consId;

    /* Check for outnode node, config sch based on next node */
    for (idx = 0; idx < (node->outputNodeSet.numNodes); idx++)
    {
        if (FVID2_SOK != status)
        {
            break;
        }
        if ((VHWA_FC_NODE_MSC0 == node->outputNodeSet.node[idx]->nodeId) ||
            (VHWA_FC_NODE_MSC1 == node->outputNodeSet.node[idx]->nodeId))
        {      
            for (idx3 = 0; idx3 < VHWA_GRAPH_MAX_NUM_CONNECTION; idx3++)
            {
                if(UTRUE == node->outputNodeSet.isEnabled[idx][idx3])
                {
                    edgeFound = UTRUE;

                    if(VHWA_FC_PORT_VISS_OUT_Y12 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        vissOutIdx = 0;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_UV12 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        vissOutIdx = 1;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_Y8 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        vissOutIdx = 2;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_UV8 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        vissOutIdx = 3;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_S8 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        vissOutIdx = 4;
                    }
                    else
                    {
                        edgeFound = UFALSE;
                    }
                    if(UTRUE == edgeFound)
                    {
                        vissCtrlIdx = Vhwa_FcGetVissCtrlIdx(vissOutIdx,
                                                                   &consCfgProdId);
                        if(VHWA_GRAPH_CTRL_INDEX_INVALID != vissCtrlIdx)
                        {
                            Vhwa_FcGetMscConsId(
                                    node->outputNodeSet.edgeInfo[idx][idx3]->endPort,
                                    &consId);
                            
                            Vhwa_m2mFcSetupVissCtrl(hObj, vissCtrlIdx,
                                            vissOutIdx, consCfgProdId,
                                            consId);

                            status = Fvid2_control(hObj->modHdls.vissHandle,
                                        IOCTL_VHWA_VISS_FC_CONN_PARAMS,
                                        &hObj->vissFcConPrms, NULL);
                        }
                        else
                        {
                            status = FVID2_EBADARGS;
                        }
                    }
                }
            }
        }
        else if (VHWA_FC_NODE_DDR == node->outputNodeSet.node[idx]->nodeId)
        {
            /* Check if no DDR output disable DMA consumers */
            hObj->vissFcPrms.dmaConsEnable[0] = UFALSE;
            hObj->vissFcPrms.dmaConsEnable[1] = UFALSE;
            hObj->vissFcPrms.dmaConsEnable[2] = UFALSE;
            hObj->vissFcPrms.dmaConsEnable[3] = UFALSE;
            hObj->vissFcPrms.dmaConsEnable[4] = UFALSE;

            for (idx3 = 0; idx3 < VHWA_GRAPH_MAX_NUM_CONNECTION; idx3++)
            {
                if(UTRUE == node->outputNodeSet.isEnabled[idx][idx3])
                {
                    if(VHWA_FC_PORT_VISS_OUT_Y12 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        hObj->vissFcPrms.dmaConsEnable[0] = UTRUE;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_UV12 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        hObj->vissFcPrms.dmaConsEnable[1] = UTRUE;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_Y8 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        hObj->vissFcPrms.dmaConsEnable[2] = UTRUE;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_UV8 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        hObj->vissFcPrms.dmaConsEnable[3] = UTRUE;
                    }
                    else if(VHWA_FC_PORT_VISS_OUT_S8 ==
                        node->outputNodeSet.edgeInfo[idx][idx3]->startPort)
                    {
                        hObj->vissFcPrms.dmaConsEnable[4] = UTRUE;
                    }
                    else
                    {
                      status = FVID2_EBADARGS;
                    }
                }
            }
        }
        else
        {
           status = FVID2_EBADARGS;
        }
    }
    /* H3A is not controlled by Graph */
    hObj->vissFcPrms.dmaConsEnable[5] = UTRUE;
    return status;
}

static int32_t Vhwa_m2mFcDrvGetMscOutIdx(uint64_t startPort,
                                         uint32_t *outIdx)

{
    int32_t status = FVID2_SOK;

    switch (startPort)
    {
        case VHWA_FC_PORT_MSC0_OUT_0:
        case VHWA_FC_PORT_MSC1_OUT_0:
        {
            *outIdx = 0;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_1:
        case VHWA_FC_PORT_MSC1_OUT_1:
        {
            *outIdx = 1;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_2:
        case VHWA_FC_PORT_MSC1_OUT_2:
        {
            *outIdx = 2;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_3:
        case VHWA_FC_PORT_MSC1_OUT_3:
        {
            *outIdx = 3;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_4:
        case VHWA_FC_PORT_MSC1_OUT_4:
        {
            *outIdx = 4;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_5:
        case VHWA_FC_PORT_MSC1_OUT_5:
        {
            *outIdx = 5;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_6:
        case VHWA_FC_PORT_MSC1_OUT_6:
        {
            *outIdx = 6;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_7:
        case VHWA_FC_PORT_MSC1_OUT_7:
        {
            *outIdx = 7;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_8:
        case VHWA_FC_PORT_MSC1_OUT_8:
        {
            *outIdx = 8;
            break;
        }
        case VHWA_FC_PORT_MSC0_OUT_9:
        case VHWA_FC_PORT_MSC1_OUT_9:
        {
            *outIdx = 9;
            break;
        }
        default:
        {
            status = FVID2_EFAIL;
            break;
        }
    }

    return status;
}

static int32_t Vhwa_m2mFcSetupMscCtrl(Vhwa_M2mFcHandleObj *hObj,
                                    uint32_t mscCtrlProdIdx,
                                    uint32_t mscOutProdIdx,
                                    uint32_t consCfgProdId,
                                    uint32_t curNode,
                                    uint32_t nextNode)
{
    int32_t status = FVID2_SOK;

    if((VHWA_FC_NODE_MSC0 == nextNode) && (VHWA_FC_NODE_MSC1 == curNode))
    {
        hObj->msc1FcConPrms.htsConsId = CSL_HTS_CONS_IDX_MSC0_CH0;

        hObj->msc1FcConPrms.outputIdx = mscOutProdIdx;
        hObj->msc1FcConPrms.ctrlIdx = mscCtrlProdIdx;
        hObj->msc1FcConPrms.sl2Depth =
                        hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2BuffDepth[0][0];
        hObj->msc1FcConPrms.sl2Addr =
                        hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2Addr[0][0];
        hObj->msc1FcConPrms.sl2Pitch = hObj->msc0FcGetPrms.inSl2Pitch[0];

        hObj->msc1FcConPrms.htsThreshold =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[0U].threshold;
        hObj->msc1FcConPrms.htsDmaProdPreLoad =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[0U].cntPreLoad;
        hObj->msc1FcConPrms.htsDmaProdPostLoad =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[0U].cntPostLoad;
        hObj->msc1FcConPrms.htsDmaProdCntDec =
                        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[0U].countDec;

        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[0U].enable = UFALSE;
        hObj->msc1FcGetPrms.htsCfg->consCfg[0U].prodId = consCfgProdId;

        status = Fvid2_control(hObj->modHdls.msc1Handle,
                            IOCTL_VHWA_MSC_FC_CONN_PARAMS,
                            &hObj->msc1FcConPrms, NULL);
    }
    else if ((VHWA_FC_NODE_MSC1 == nextNode) && (VHWA_FC_NODE_MSC0 == curNode))
    {
        hObj->msc0FcConPrms.htsConsId = CSL_HTS_CONS_IDX_MSC1_CH0;

        hObj->msc0FcConPrms.outputIdx = mscOutProdIdx;
        hObj->msc0FcConPrms.ctrlIdx = mscCtrlProdIdx;
        hObj->msc0FcConPrms.sl2Depth =
                        hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2BuffDepth[1][0];
        hObj->msc0FcConPrms.sl2Addr =
                        hObj->modInfo.sl2ResObj.mscSl2Prms.inSl2Addr[1][0];
        hObj->msc0FcConPrms.sl2Pitch = hObj->msc1FcGetPrms.inSl2Pitch[0];

        hObj->msc0FcConPrms.htsThreshold =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[0U].threshold;
        hObj->msc0FcConPrms.htsDmaProdPreLoad =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[0U].cntPreLoad;
        hObj->msc0FcConPrms.htsDmaProdPostLoad =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[0U].cntPostLoad;
        hObj->msc0FcConPrms.htsDmaProdCntDec =
                        hObj->msc1FcGetPrms.htsCfg->dmaProdCfg[0U].countDec;

        hObj->msc0FcGetPrms.htsCfg->dmaProdCfg[0U].enable = UFALSE;
        hObj->msc0FcGetPrms.htsCfg->consCfg[0U].prodId = consCfgProdId;

        status = Fvid2_control(hObj->modHdls.msc0Handle,
                            IOCTL_VHWA_MSC_FC_CONN_PARAMS,
                            &hObj->msc0FcConPrms, NULL);

    }
    else
    {
        status = FVID2_EFAIL;
    }

    return status;
}

int32_t Vhwa_m2mFcDrvConfigMscSch(Vhwa_M2mFcHandleObj *hObj,
                                    const Vhwa_M2mFcGraphNodeInfo *node)
{
    int32_t status = FVID2_SOK;
    uint32_t idx, idx2;
    uint32_t mscOutIdx, mscCtrlIdx, edgeFound, consCfgProdId = 0U;
    uint32_t mscSpareSoc = VHWA_MSC_SPARE_SOCKET;
    Vhwa_M2mMscFcUpdatePrms *pFcPrms = NULL;
    Vhwa_M2mMscFcConPrms *pFcConPrms = NULL;



    if (VHWA_FC_NODE_MSC0 == node->nodeId)
    {
        pFcPrms = &hObj->msc0FcPrms;
        pFcConPrms = &hObj->msc0FcConPrms;
        consCfgProdId = CSL_HTS_PROD_IDX_MSC0_SP;
    }
    else if(VHWA_FC_NODE_MSC1 == node->nodeId)
    {
        pFcPrms = &hObj->msc1FcPrms;
        pFcConPrms = &hObj->msc1FcConPrms;
        consCfgProdId = CSL_HTS_PROD_IDX_MSC1_SP;
    }
    else
    {
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK == status)
    {
        for (idx = 0;
                (idx < (node->inputNodeSet.numNodes)) && (FVID2_SOK == status);
                idx++)
        {
            if (VHWA_FC_NODE_DDR == node->inputNodeSet.node[idx]->nodeId)
            {
                pFcPrms->inDmaEnable[idx] = UTRUE;
            }
        }
        for (idx = 0; idx < (node->outputNodeSet.numNodes); idx++)
        {
            if (FVID2_SOK != status)
            {
                break;
            }
            if ((VHWA_FC_NODE_MSC0 == node->outputNodeSet.node[idx]->nodeId) ||
                (VHWA_FC_NODE_MSC1 == node->outputNodeSet.node[idx]->nodeId))
            {
                edgeFound = UFALSE;

                for (idx2 = 0; idx2 < VHWA_GRAPH_MAX_NUM_CONNECTION; idx2++)
                {
                    if(UTRUE == node->outputNodeSet.isEnabled[idx][idx2])
                    {
                        status = Vhwa_m2mFcDrvGetMscOutIdx(
                                node->outputNodeSet.edgeInfo[idx][idx2]->startPort,
                                &mscOutIdx);
                        if(FVID2_SOK == status)
                        {
                            edgeFound = UTRUE;
                        }
                        else
                        {
                            edgeFound = UFALSE;
                        }
                    }

                    if(UTRUE == edgeFound)
                    {
                        if (mscSpareSoc > 0u)
                        {
                            mscCtrlIdx = 10u;

                            pFcConPrms->enableMaskSel = UTRUE;
                            pFcConPrms->maskSel = mscOutIdx;
                            mscSpareSoc--;
                        }
                        else
                        {
                            status = FVID2_EBADARGS;
                            mscCtrlIdx = VHWA_GRAPH_CTRL_INDEX_INVALID;
                        }

                        if(VHWA_GRAPH_CTRL_INDEX_INVALID != mscCtrlIdx)
                        {
                            status = Vhwa_m2mFcSetupMscCtrl(hObj, mscCtrlIdx,
                                            mscOutIdx, consCfgProdId,
                                            node->nodeId,
                                            node->outputNodeSet.node[idx]->nodeId);
                        }

                        break;
                    }
                }
            }
            else if (VHWA_FC_NODE_DDR == node->outputNodeSet.node[idx]->nodeId)
            {
                /* Disable all DMA consumers */
                for (idx2 = 0; idx2 < MSC_MAX_OUTPUT; idx2++)
                {
                    pFcPrms->outDmaEnable[idx2] = UFALSE;
                }

                for (idx2 = 0; idx2 < VHWA_GRAPH_MAX_NUM_CONNECTION; idx2++)
                {
                    if(UTRUE == node->outputNodeSet.isEnabled[idx][idx2])
                    {
                        status = Vhwa_m2mFcDrvGetMscOutIdx(
                                node->outputNodeSet.edgeInfo[idx][idx2]->startPort,
                                &mscOutIdx);
                        if(FVID2_SOK == status)
                        {
                            pFcPrms->outDmaEnable[mscOutIdx] = UTRUE;
                        }
                    }
                }
            }
            else
            {
                status = FVID2_EBADARGS;
            }
        }
    }

    return status;
}
