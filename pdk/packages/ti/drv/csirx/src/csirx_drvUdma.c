/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2019
 *  All rights reserved.
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
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \file csirx_drvUdma.c
 *
 *  \brief File containing the CSIRX driver UDMA initialization and
 *         configuration code.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <ti/drv/csirx/src/csirx_drvUdma.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/*
 * UDMA Memories
 */
static uint8_t gCsirxRingMem[CSIRX_INSTANCE_ID_MAX][CSIRX_NUM_CH][CSIRX_DRV_RING_MEM_SIZE_ALIGN]
                            __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#if defined (SOC_J721E)
static uint8_t gCsirxCompRingMem[CSIRX_INSTANCE_ID_MAX][CSIRX_NUM_CH][CSIRX_DRV_RING_MEM_SIZE_ALIGN]
                            __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gCsirxTdCompRingMem[CSIRX_INSTANCE_ID_MAX][CSIRX_NUM_CH][CSIRX_DRV_RING_MEM_SIZE_ALIGN]
                            __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#endif
static uint8_t gCsirxUdmaTprdMem
        [CSIRX_INSTANCE_ID_MAX][CSIRX_NUM_CH][CSIRX_CAPT_QUEUE_DEPTH_PER_CH][CSIRX_DRV_TRPD_SIZE_ALIGN]
                            __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
extern CsirxDrv_CommonObj gCsirxCommonObj;

#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
extern uint32_t gTrSubmit[CSIRX_DRV_TR_LOG_CNT][4];
extern uint32_t gTrSubmitCnt;
extern uint32_t gTrOut[CSIRX_DRV_TR_LOG_CNT][4];
extern uint32_t gTrOutCnt;
#endif
/* ========================================================================== */
/*                  Internal/Private Function Declarations                   */
/* ========================================================================== */
static uint16_t CsirxDrv_getIcnt0(const Csirx_ChCfg *chCfg);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
int32_t CsirxDrv_getDMACfgParams(CsirxDrv_ChObj *chObj)
{
    int32_t retVal = FVID2_SOK;
    Csirx_CreateParams *createParams;
    uint32_t chType;

    createParams = &chObj->instObj->createParams;

    /* Assign UDMA Rx channel params from channel configurations */
    chObj->rxChParams = &chObj->chCfg->rxChParams;
    /* Init UDMA channel parameters */
    chType = UDMA_CH_TYPE_RX;
    UdmaChPrms_init(&chObj->chParams, chType);
    chObj->trpdMem = &gCsirxUdmaTprdMem[chObj->instObj->drvInstId][chObj->psilThreadId][0U][0U];
    chObj->rxFqRingMem = &gCsirxRingMem[chObj->instObj->drvInstId][chObj->psilThreadId][0U];
#if defined (SOC_J721E)
    chObj->rxCqRingMem = &gCsirxCompRingMem[chObj->instObj->drvInstId][chObj->psilThreadId][0U];
    chObj->rxTdCqRingMem = &gCsirxTdCompRingMem[chObj->instObj->drvInstId][chObj->psilThreadId][0U];
#endif
    chObj->chParams.peerChNum            = (chObj->instObj->psilThreadIdStart +
                                            chObj->psilThreadId);
    chObj->chParams.fqRingPrms.ringMem   = chObj->rxFqRingMem;
    chObj->chParams.fqRingPrms.orderId   = chObj->chCfg->ringOrderId;
    chObj->chParams.fqRingPrms.elemCnt   = CSIRX_CAPT_QUEUE_DEPTH_PER_CH;
#if defined (SOC_J721E)
    chObj->chParams.cqRingPrms.ringMem   = chObj->rxCqRingMem;
    chObj->chParams.tdCqRingPrms.ringMem = chObj->rxTdCqRingMem;
    /* Initialize ring order ID */
    chObj->chParams.cqRingPrms.orderId   = chObj->chCfg->ringOrderId;
    chObj->chParams.tdCqRingPrms.orderId = chObj->chCfg->ringOrderId;
    chObj->chParams.cqRingPrms.elemCnt   = CSIRX_CAPT_QUEUE_DEPTH_PER_CH;
    chObj->chParams.tdCqRingPrms.elemCnt = CSIRX_CAPT_QUEUE_DEPTH_PER_CH;
#endif

    /* Get the PSI-L Threads/DMA channel configuration parameters */
    chObj->dmaCfgParams.chNum = chObj->psilThreadId;
    chObj->dmaCfgParams.virtCh = chObj->chCfg->vcNum;
    chObj->dmaCfgParams.dataType = chObj->chCfg->inCsiDataType;
    chObj->dmaCfgParams.dstTag = CSIRX_DRV_PSI_DEST_TAG;
    chObj->dmaCfgParams.srcTag = CSIRX_DRV_PSI_SRC_TAG;
    chObj->dmaCfgParams.psFlags = 0U;
    chObj->dmaCfgParams.pktType = 0U;

    chObj->dmaCfgParams.dualPkgEnable = UFALSE;
    /* Storage mode for YUV422 data format */
    if (FVID2_DF_YUV422I_UYVY == chObj->chCfg->outFmt.dataFormat)
    {
        chObj->dmaCfgParams.yuv422Mode = CSL_CSIRX_YUV422_MODE_YVYU;
    }
    else if (FVID2_DF_YUV422I_YUYV == chObj->chCfg->outFmt.dataFormat)
    {
        chObj->dmaCfgParams.yuv422Mode = CSL_CSIRX_YUV422_MODE_VYUY;
    }
    else if (FVID2_DF_YUV422I_YVYU == chObj->chCfg->outFmt.dataFormat)
    {
        chObj->dmaCfgParams.yuv422Mode = CSL_CSIRX_YUV422_MODE_UYVY;
    }
    else if (FVID2_DF_YUV422I_VYUY == chObj->chCfg->outFmt.dataFormat)
    {
        chObj->dmaCfgParams.yuv422Mode = CSL_CSIRX_YUV422_MODE_YUYV;
    }
    else
    {
        /* If input format is not YUV422, this parameter values is not used */
        chObj->dmaCfgParams.yuv422Mode = CSL_CSIRX_YUV422_MODE_UYVY;
    }
    chObj->dmaCfgParams.pck12CfgEnable = UFALSE;
    /* Change 12 bit packed mode only if DT(inCsiDataType) is
       FVID2_CSI2_DF_RAW12 */
    if (FVID2_CSI2_DF_RAW12 == chObj->chCfg->inCsiDataType)
    {
        if (FVID2_CCSF_BITS12_PACKED == chObj->chCfg->outFmt.ccsFormat)
        {
            chObj->dmaCfgParams.pck12CfgEnable = UTRUE;
        }
        else if (FVID2_CCSF_BITS12_UNPACKED16 == chObj->chCfg->outFmt.ccsFormat)
        {
            chObj->dmaCfgParams.pck12CfgEnable = UFALSE;
        }
        else
        {
            /* If input format is not RAW12, this parameter values is not used */
            chObj->dmaCfgParams.pck12CfgEnable = UFALSE;
        }
    }
    if ((FVID2_CSI2_DF_YUV422_8B == chObj->chCfg->inCsiDataType) && 
        (1U == createParams->instCfg.numPixelsStrm0))
    {
        chObj->dmaCfgParams.dualPkgEnable = UTRUE;
    }
    if (FVID2_CSI2_DF_RAW20 == chObj->chCfg->inCsiDataType)
    {
        chObj->dmaCfgParams.dataSizeShift =
                            CSL_CSIRX_DMA_DATA_SIZE_SHIFT_32BITS;
    }
    else if (FVID2_CCSF_BITS12_PACKED == chObj->chCfg->outFmt.ccsFormat)
    {
        chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_8BITS;
    }
    else if ((FVID2_CSI2_DF_YUV420_10B == chObj->chCfg->inCsiDataType) ||
             (FVID2_CSI2_DF_YUV422_10B == chObj->chCfg->inCsiDataType) ||
             (FVID2_CSI2_DF_RAW10      == chObj->chCfg->inCsiDataType) ||
             (FVID2_CSI2_DF_RAW12      == chObj->chCfg->inCsiDataType) ||
             (FVID2_CSI2_DF_RAW14      == chObj->chCfg->inCsiDataType) ||
             (FVID2_CSI2_DF_RAW16      == chObj->chCfg->inCsiDataType))
    {
        if (0U == createParams->instCfg.numPixelsStrm0)
        {
            chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_16BITS;
        }
        else if(1U == createParams->instCfg.numPixelsStrm0)
        {
            chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_32BITS;
        }
        else
        {
            /* Do nothing as above two conditions should be hit
               and check has done before entering this function */
            retVal = FVID2_EFAIL;
        }
    }
    else if (FVID2_CSI2_DF_YUV422_8B == chObj->chCfg->inCsiDataType)
    {
        if ((0U == createParams->instCfg.numPixelsStrm0) || (1U == createParams->instCfg.numPixelsStrm0))
        {
            chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_8BITS;
        }
        else
        {
            /* Do nothing as above two conditions should be hit
               and check has done before entering this function */
            retVal = FVID2_EFAIL;
        }
    }
    else if (FVID2_CSI2_DF_RAW8 == chObj->chCfg->inCsiDataType)
    {

        if (0U == createParams->instCfg.numPixelsStrm0)
        {
            chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_8BITS;
        }
        else if(1U == createParams->instCfg.numPixelsStrm0)
        {
            chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_16BITS;
            chObj->dmaCfgParams.dualPkgEnable = 1U;
        }
        else if(2U == createParams->instCfg.numPixelsStrm0)
        {
            chObj->dmaCfgParams.dataSizeShift =
                                CSL_CSIRX_DMA_DATA_SIZE_SHIFT_32BITS;
            chObj->dmaCfgParams.dualPkgEnable = 1U;
        }
        else
        {
            /* Do nothing as above two conditions should be hit
               and check has done before entering this function */
            retVal = FVID2_EFAIL;
        }
    }
    else
    {
        chObj->dmaCfgParams.dataSizeShift =
                            CSL_CSIRX_DMA_DATA_SIZE_SHIFT_8BITS;
    }

    return retVal;
}

int32_t CsirxDrv_setChUdmaParams(CsirxDrv_ChObj *chObj)
{
    int32_t retVal = FVID2_SOK, tempRetVal = UDMA_SOK;
    uint32_t chType;
    Udma_EventHandle eventHandle;
    Udma_EventPrms eventParams;
    CsirxDrv_CommonObj *captObj;

    captObj = chObj->instObj->commonObjRef;
    if (CSIRX_CH_TYPE_CAPT == chObj->chCfg->chType)
    {
        /* Program UDMA channels only for capture channels */
        chType = UDMA_CH_TYPE_RX;
        /* Open UDMA channel */
        tempRetVal = Udma_chOpen(captObj->initParams.drvHandle,
                                 &chObj->rxChObj,
                                 chType,
                                 &chObj->chParams);
        if(UDMA_SOK != tempRetVal)
        {
            retVal = CsirxDrv_udmaToFvid2ErrorMap(tempRetVal);
            GT_0trace(CsirxTrace,
                      GT_ERR,
                      "[ERROR]UDMA channel open FAILED!!!\r\n");
        }
        if (FVID2_SOK == retVal)
        {
            /* Configure UDMA Rx Channel */
            chObj->rxChParams->configDefaultFlow = UFALSE;
            tempRetVal = Udma_chConfigRx(&chObj->rxChObj,
                                         chObj->rxChParams);
            if(UDMA_SOK != tempRetVal)
            {
                retVal = CsirxDrv_udmaToFvid2ErrorMap(tempRetVal);
                GT_0trace(CsirxTrace,
                      GT_ERR,
                      "[ERROR]UDMA RX channel config FAILED!!!\r\n");
            }
        }
        if (FVID2_SOK == retVal)
        {
            /* UDMA Master event is registered only once. Subsequent Event
             * registrations use the same Master event */
            if(UDMA_INIT_DONE != captObj->masterEvent.eventInitDone)
            {
                  UdmaEventPrms_init(&eventParams);
                  eventParams.eventType         = UDMA_EVENT_TYPE_MASTER;
                  eventParams.appData           = captObj;
                  retVal = Udma_eventRegister(captObj->initParams.drvHandle,
                                              &captObj->masterEvent,
                                              &eventParams);
                  if(UDMA_SOK != retVal)
                  {
                      retVal = CsirxDrv_udmaToFvid2ErrorMap(retVal);
                  }
            }
            if (FVID2_SOK == retVal)
            {
                  /* Register callbacks here */
                  /* Register ring completion callback */
                  eventHandle = &chObj->cqEventObj;
                  UdmaEventPrms_init(&eventParams);
                  eventParams.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
                  eventParams.eventMode         = UDMA_EVENT_MODE_SHARED;
                  eventParams.chHandle          = &chObj->rxChObj;
                  eventParams.masterEventHandle = &captObj->masterEvent;
                  eventParams.eventCb           = &CsirxDrv_udmaCQEventCb;
                  eventParams.appData           = chObj;
                  tempRetVal = Udma_eventRegister(captObj->initParams.drvHandle,
                                                  eventHandle,
                                                  &eventParams);
            }
            if(UDMA_SOK != tempRetVal)
            {
                retVal = CsirxDrv_udmaToFvid2ErrorMap(tempRetVal);
                GT_0trace(CsirxTrace,
                      GT_ERR,
                      "[ERROR]UDMA CQ event register FAILED!!!\r\n");
            }
        }

#if defined (SOC_J721E)
        if(UDMA_SOK == retVal)
        {
            /* Register teardown ring completion callback */
            eventHandle = &chObj->tdcEventObj;
            UdmaEventPrms_init(&eventParams);
            eventParams.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
            eventParams.eventMode         = UDMA_EVENT_MODE_SHARED;
            eventParams.chHandle          = &chObj->rxChObj;
            eventParams.masterEventHandle = &captObj->masterEvent;
            eventParams.eventCb           = &CsirxDrv_udmaTDCEventCb;
            eventParams.appData           = chObj;
            tempRetVal = Udma_eventRegister(captObj->initParams.drvHandle,
                                            eventHandle,
                                            &eventParams);
            if(UDMA_SOK != tempRetVal)
            {
                retVal = CsirxDrv_udmaToFvid2ErrorMap(tempRetVal);
                GT_0trace(CsirxTrace,
                    GT_ERR,
                    "[ERROR]UDMA Tear Down Q event register FAILED!!!\r\n");
            }
        }
#endif
    }
    if (FVID2_SOK == retVal)
    {
        /* Set channel status to created */
        chObj->status = CSIRX_DRV_CH_STATE_CREATED;
    }

    return retVal;
}

int32_t CsirxDrv_clearUdmaParams(CsirxDrv_ChObj *chObj)
{
    int32_t retVal = FVID2_SOK, tempRetVal = UDMA_SOK;
    Udma_EventHandle eventHandle;

    if (CSIRX_CH_TYPE_CAPT == chObj->chCfg->chType)
    {
        /* Unregister ring completion callback */
        eventHandle = &chObj->cqEventObj;
        tempRetVal += Udma_eventUnRegister(eventHandle);
#if defined (SOC_J721E)
        /* Unregister teardown ring completion callback */
        eventHandle = &chObj->tdcEventObj;
        tempRetVal += Udma_eventUnRegister(eventHandle);
#endif

        /* Close UDMA channel */
        tempRetVal += Udma_chClose(&chObj->rxChObj);
    }

    if (UDMA_SOK != tempRetVal)
    {
        retVal = CsirxDrv_udmaToFvid2ErrorMap(tempRetVal);
    }

    return retVal;
}

int32_t CsirxDrv_udmaRxTrpdInit(Udma_ChHandle rxChHandle,
                                uint8_t *pTrpdMem,
                                const uint32_t *destBuf,
                                const Csirx_ChCfg *chCfg,
                                uint32_t chIdx)
{
    int32_t retVal = FVID2_SOK;
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) pTrpdMem;
    uint32_t descType = (uint32_t)CSL_UDMAP_CPPI5_PD_DESCINFO_DTYPE_VAL_TR;
    uint32_t cqRingNum = Udma_chGetCqRingNum(rxChHandle);
    CSL_UdmapTR1 *pTr = UdmaUtils_getTrpdTr1Pointer((uint8_t *)pTrpd, 0U);
    uint32_t *pTrResp =
                (uint32_t *) (pTrpdMem + (sizeof(CSL_UdmapTR15) *
                                         (1U + 1U)));

    /* Setup descriptor */
    CSL_udmapCppi5SetDescType(pTrpd, descType);

    CSL_udmapCppi5TrSetReload(pTrpd, UFALSE, 0U);
    /* Since each TRPD will always have only one TR */
    CSL_udmapCppi5SetPktLen(pTrpd, descType, 1U);
    /* Flow ID and Packet ID */
    CSL_udmapCppi5SetIds(pTrpd, descType, 0U, (uint32_t)UDMA_DEFAULT_FLOW_ID);
    CSL_udmapCppi5SetSrcTag(pTrpd, 0x0044);             /* Not used */
    CSL_udmapCppi5SetDstTag(pTrpd, 0x0416);             /* Not used */
    CSL_udmapCppi5TrSetEntryStride(pTrpd,
        CSL_UDMAP_CPPI5_TRPD_PKTINFO_RECSIZE_VAL_64B);
    CSL_udmapCppi5SetReturnPolicy(pTrpd,
        descType,
        /* Don't care for TR */
        CSL_UDMAP_CPPI5_PD_PKTINFO2_RETPOLICY_VAL_ENTIRE_PKT,
        CSL_UDMAP_CPPI5_PD_PKTINFO2_EARLYRET_VAL_NO,
        CSL_UDMAP_CPPI5_PD_PKTINFO2_RETPUSHPOLICY_VAL_TO_TAIL,
        cqRingNum);

    /* Initialize TR packet: Start */
    /* Setup TR */
    /* Bit fields::
       Bit No.->Info
       3:0->Type: 2D transfer
       7:6->Event Size: Event is only generated with the TR is complete
       9:8->Trigger0: No Trigger
       10:11->Trigger0 Type:
                    The second inner most loop (ICNT1) will be decremented by 1.
       13:12->Trigger1: No Trigger
       15:14->Trigger1 Type:
                    The second inner most loop (ICNT1) will be decremented by 1.
       23:16->Command ID: The Command ID for the TR
       31:24->Conf Flags: Configuration Specific Flags
       */
    pTr->flags = 0U;
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TYPE,
                            CSL_UDMAP_TR_FLAGS_TYPE_2D_DATA_MOVE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_STATIC, (uint32_t)UFALSE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_EOL, (uint32_t)UFALSE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE,
                            CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, chIdx);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)UFALSE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)UFALSE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)UFALSE);
    pTr->icnt0    = CsirxDrv_getIcnt0(chCfg);
    pTr->icnt1    = (uint16_t)(chCfg->outFmt.height);
    pTr->dim1     = (int32_t)(chCfg->outFmt.pitch[0U]);
    /* Destination address does not needed to be programmed,
       this will overwritten in Queue */
    pTr->addr     = (uint64_t) destBuf;

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;
    /* Initialize TR packet: End */
    /* Writeback cache */
    CsirxDrv_cacheWb(pTrpdMem,
                     CSIRX_DRV_TRPD_SIZE_ALIGN);
    /* check if line size is greater than pitch provided, pitch should be
       equal to or more than line size in bytes */
    if ((pTr->icnt0 > (uint32_t)pTr->dim1) || (0U == pTr->icnt0))
    {
        retVal = FVID2_EINVALID_PARAMS;
    }

    return retVal;
}

void CsirxDrv_udmaCQEventCb(Udma_EventHandle eventHandle,
                            uint32_t eventType,
                            void *appData)
{
    int32_t retVal = UDMA_SOK, tempVal = UDMA_SOK;
    CsirxDrv_ChObj *chObj;
    CsirxDrv_QueObj *qObj;
    CsirxDrv_BufManObj *bmObj;
    uint64_t pDesc = 0;
    uint32_t *pTrResp;
    uint32_t pTrRespVal, pTrRespStsTypeVal, pTrRespInfoVal;
    uint32_t curQCnt, appCb = UFALSE, cookie, chIdx;
    uint64_t timeStamp;
    CSL_UdmapTR1 *pTr;

    if(NULL == eventHandle)
	{
		/*To remove unused variable warning*/
	}

    chObj = (CsirxDrv_ChObj *) appData;
    GT_assert(CsirxTrace, (NULL != chObj));

    /* Take out completed TRPD from UDMA CQ Ring */
    if(UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    {
        /* Disable HW interrupts here */
        cookie = HwiP_disable();
        bmObj = &chObj->bufManObj;
        /* Response received in completion queue, empty up full CQ Ring */
        while (UDMA_SOK == retVal)
        {
            retVal = Udma_ringDequeueRaw(
                         Udma_chGetCqRingHandle(&chObj->rxChObj), &pDesc);
            if(UDMA_SOK == retVal)
            {
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
                {
                    gTrOut[gTrOutCnt][0U] = (uint32_t)pDesc;
                    gTrOut[gTrOutCnt][1U] = chObj->instObj->drvInstId;
                    gTrOut[gTrOutCnt][2U] = chObj->chId;
                    if (NULL != gCsirxCommonObj.getTimeStamp)
                    {
                        gTrOut[gTrOutCnt][3U] =
                                        (uint32_t)gCsirxCommonObj.getTimeStamp(NULL);;
                    }
                    else
                    {
                        gTrOut[gTrOutCnt][3U] = 0U;
                    }
                    gTrOutCnt = (gTrOutCnt + 1U) % CSIRX_DRV_TR_LOG_CNT;
                    CsirxDrv_cacheWb(&gTrOut[0U],
                                     sizeof(gTrOut));
                }
#endif
                if (NULL != gCsirxCommonObj.getTimeStamp)
                {
                    timeStamp = gCsirxCommonObj.getTimeStamp(NULL);
                }
                else
                {
                    timeStamp = 0U;
                }
                /* De-queue from curQ */
                qObj = (CsirxDrv_QueObj *) Fvid2Utils_dequeue(bmObj->curQ);
                GT_assert(CsirxTrace, (NULL != qObj));
				if(NULL != qObj)
				{
					GT_assert(CsirxTrace,
                          (chObj->chId == qObj->chObj->chId));
					/* Check returned descriptor pointer */
					GT_assert(CsirxTrace, (pDesc == ((uint64_t) qObj->trpd)));
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
					gCsirxCommonObj.frmChId[gCsirxCommonObj.frmCnt] =
													qObj->chObj->chId;
					gCsirxCommonObj.frmInstId[gCsirxCommonObj.frmCnt] =
											qObj->chObj->instObj->drvInstId;
					if (NULL != gCsirxCommonObj.getTimeStamp)
					{
						gCsirxCommonObj.frmTS[gCsirxCommonObj.frmCnt] =
												(uint32_t)(timeStamp / 1000U);
					}
#endif
                    pTr = UdmaUtils_getTrpdTr1Pointer((uint8_t *)qObj->trpd, 0U);
                    if (pTr->addr == (uint64_t)(chObj->instObj->createParams.frameDropBuf))
                    {
                        /* Mark qBoj as Normal Frame which can be used next time */
                        qObj->type = CSIRX_DRV_Q_OBJ_TYPE_NORMAL;
                        /* This is frame drop buffer so no need to issue CB */
                        appCb = UFALSE;
                        /* Invalidate cache */
                        CsirxDrv_cacheInv(qObj->trpd,
                                          CSIRX_DRV_TRPD_SIZE_ALIGN);
                        /* de-queued TRPD was for frame submitted
                           by application */
                        /* Check for truncated/elongated frame */
                        pTrResp = (uint32_t *) ((uint8_t *)(qObj->trpd) +
                                               (sizeof(CSL_UdmapTR15) *
                                               (1U + 1U)));
                        /* NO need to put it to done queue, put it to FreeQ */
                        Fvid2Utils_queue(bmObj->freeQ, &qObj->qElem, qObj);
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
                        gCsirxCommonObj.trResp[gCsirxCommonObj.frmCnt] =
                                                CSL_REG32_RD(pTrResp);
                        gCsirxCommonObj.frmType[gCsirxCommonObj.frmCnt] = 0U;
#endif
                    }
                    else
                    {
                        /* This is frame provided by application so issue CB */
                        appCb = UTRUE;
                        /*GT_assert(CsirxTrace, (qObj->frm != NULL));*/
                        qObj->frm->timeStamp64 = timeStamp;
                        /* Invalidate cache */
                        CsirxDrv_cacheInv(qObj->trpd,
                                          CSIRX_DRV_TRPD_SIZE_ALIGN);
                        /* de-queued TRPD was for frame submitted
                           by application */
                        /* Check for truncated/elongated frame */
                        pTrResp = (uint32_t *) ((uint8_t *)(qObj->trpd) +
                                               (sizeof(CSL_UdmapTR15) *
                                               (1U + 1U)));

                        pTrRespVal = CSL_REG32_RD(pTrResp);
                        pTrRespStsTypeVal = CSL_REG32_FEXT(pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
                        pTrRespInfoVal = CSL_FEXT(pTrRespVal, UDMAP_TR_RESPONSE_STATUS_INFO);
                        /* check TR response */
                        if (CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE != pTrRespStsTypeVal)
                        {

                            if ((CSIRX_DRV_CH_STATE_STOPPING == chObj->status) ||
                                (CSIRX_DRV_CH_STATE_STOPPED  == chObj->status) ||
                                (CSL_UDMAP_TR_RESPONSE_STATUS_ABORTED_ERR == pTrRespStsTypeVal))
                            {
                                /* Aborted frame due to forced teardown at
                                   Udma_chDisable() */
                                qObj->frm->status = FVID2_FRAME_STATUS_ABORTED;
                            }
                            else if (CSL_UDMAP_TR_RESPONSE_STATUS_SUBMISSION_ERR == pTrRespStsTypeVal)
                            { 
                                /* Submission error */
                                qObj->frm->status = FVID2_FRAME_STATUS_SUBMISSION_ERROR; 
                            }
                            else if (CSL_UDMAP_TR_RESPONSE_STATUS_TRANSFER_EXCEP_ERR ==
                                      pTrRespStsTypeVal)
                            {
                                /* Transfer exception*/
                                if(CSL_UDMAP_TR_RESPONSE_STATUS_TRANSFER_EXCEP_SHORT_PACKET ==
                                    pTrRespInfoVal)
                                    /* \ref CSL_ErrType_t */
                                {
                                    /* Received frame is truncated */
                                    qObj->frm->status = FVID2_FRAME_STATUS_TRUNCATED;
                                    chObj->instObj->status.errorFrameCount[chObj->chCfg->chId]++;
                                }
                                else if (CSL_UDMAP_TR_RESPONSE_STATUS_TRANSFER_EXCEP_LONG_PACKET ==
                                          pTrRespInfoVal)
                                          /* \ref CSL_ErrType_t */
                                {
                                    /* Received frame is elongated */
                                    qObj->frm->status = FVID2_FRAME_STATUS_ELONGATED;
                                    chObj->instObj->status.errorFrameCount[chObj->chCfg->chId]++;
                                }
                                else
                                {
                                    /* Do nothing */
                                }
                            }
                            else
                            {
                                /* Other UDMA TR errors */
                                qObj->frm->status = FVID2_FRAME_STATUS_ERROR;
                            }
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
                            gCsirxCommonObj.frmStatus[gCsirxCommonObj.frmCnt] =
                                                        qObj->frm->status;
#endif
                        }
                        else
                        {
                            /* Received frame has no error */
                            qObj->frm->status = FVID2_FRAME_STATUS_COMPLETED;
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
                            gCsirxCommonObj.frmStatus[gCsirxCommonObj.frmCnt] =
                                                            qObj->frm->status;
#endif
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
                    gCsirxCommonObj.trResp[gCsirxCommonObj.frmCnt] =
                                                CSL_REG32_RD(pTrResp);
                    gCsirxCommonObj.frmType[gCsirxCommonObj.frmCnt] = 1U;
#endif
                        }
                        /* Return the frame to done queue */
                        Fvid2Utils_queue(
                            chObj->bufManObj.doneQ,
                            &qObj->qElem,
                            qObj);
                    }
#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
					uint32_t oldFrmCnt;

					oldFrmCnt = gCsirxCommonObj.frmCnt;
					gCsirxCommonObj.frmCnt = (gCsirxCommonObj.frmCnt + 1U) %
											  CSIRX_DRV_LOG_CNT;
					if (oldFrmCnt > gCsirxCommonObj.frmCnt)
					{
						gCsirxCommonObj.frmCntOverflow = UTRUE;
					}
#endif
				}
            }
        }
        /** Enable HW interrupts here */
        HwiP_restore(cookie);

        if (CSIRX_DRV_CH_STATE_RUNNING == chObj->status)
        {
            /* Application call back */
            if (UTRUE == appCb)
            {
                if(NULL != chObj->virtContext->fdmCbParams.fdmCbFxn)
                {
                    (void)chObj->virtContext->fdmCbParams.fdmCbFxn(
                                    chObj->virtContext->fdmCbParams.fdmData);
                }
            }
        }

        /* Disable HW interrupts here */
        cookie = HwiP_disable();
        if (UDMA_ETIMEOUT == retVal)
        {
            /* this will happen if there in no TR in the Ring of UDMA channel */
            retVal = FVID2_SOK;
        }
        if (FVID2_SOK == retVal)
        {
            if (CSIRX_DRV_CH_STATE_RUNNING == chObj->status)
            {
                bmObj = &chObj->bufManObj;
                /* Check if one more frame is queued by application,
                   if not then submit TR with frame drop buffer */
                curQCnt = Fvid2Utils_getNumQElem(bmObj->curQ);
                if (1U >= curQCnt)
                {
                    while (CSIRX_DRV_FRAME_DROP_TRPD_NUM > curQCnt)
                    {
                        /* Allocate a free queue object from the pool */
                        qObj =
                         (CsirxDrv_QueObj *) Fvid2Utils_dequeue(bmObj->freeQ);
                        if (NULL == qObj)
                        {
                            GT_0trace(
                                CsirxTrace, GT_ERR,
                                "ALLOC: Q object allocation failed\r\n");
                            retVal = FVID2_EALLOC;
                        }
                        if(FVID2_SOK == retVal)
                        {
                            /* Copy the frame to the driver's queue object */
                            qObj->frm = NULL;
                            /* Re-program dim0 and destination buffer */
                            pTr = UdmaUtils_getTrpdTr1Pointer((uint8_t *)qObj->trpd, 0U);
                            /* Keep over-writing frameDrop buffer with each incoming line */
                            pTr->dim1 = 0;
                            pTr->addr =
                                (uint64_t)(chObj->instObj->createParams.frameDropBuf);
                            /* Mark qBoj as Frame Drop */
                            qObj->type = CSIRX_DRV_Q_OBJ_TYPE_FD;
                            /* Submit TR to FQ ring for UDMA channel */
                            /* Writeback cache */
                            CsirxDrv_cacheWb(qObj->trpd,
                                             CSIRX_DRV_TRPD_SIZE_ALIGN);
                            /* Submit TRPD to channel */
    #if (CSIRX_DRV_ENABLE_DEBUG == 1U)
                            {
                                gTrSubmit[gTrSubmitCnt][0U] =
                                            (uint32_t)qObj->trpd;
                                gTrSubmit[gTrSubmitCnt][1U] =
                                            chObj->instObj->drvInstId;
                                gTrSubmit[gTrSubmitCnt][2U] =
                                            qObj->chObj->chId;;
                                if (NULL != gCsirxCommonObj.getTimeStamp)
                                {
                                    gTrSubmit[gTrSubmitCnt][3U] =
                                            (uint32_t)gCsirxCommonObj.getTimeStamp(NULL);
                                }
                                else
                                {
                                    gTrSubmit[gTrSubmitCnt][3U] = 0U;
                                }
                                gTrSubmitCnt =
                                    (gTrSubmitCnt + 1U) % CSIRX_DRV_TR_LOG_CNT;
                                CsirxDrv_cacheWb(&gTrSubmit[0U],
                                                 sizeof(gTrSubmit));
                            }
    #endif
                            tempVal = Udma_ringQueueRaw(
                                            Udma_chGetFqRingHandle(&chObj->rxChObj),
                                            (uint64_t)qObj->trpd);
                            if(UDMA_SOK != tempVal)
                            {
                                GT_0trace(
                                    CsirxTrace, GT_ERR,
                                    "Queue: Submitting of TR failed\r\n");
                                /* Queue back the Object which was
                                   de-queued earlier from freeQ */
                                Fvid2Utils_queue(bmObj->freeQ, &qObj->qElem, qObj);
                                retVal = CsirxDrv_udmaToFvid2ErrorMap(tempVal);
                            }
                            else
                            {
                                /* Add the queue object in driver's current queue */
                                Fvid2Utils_queue(bmObj->curQ, &qObj->qElem, qObj);
                                /* update frame drop count here */
                                chIdx = chObj->chId;
                                chObj->instObj->status.dropCount[chIdx]++;
                            }
                        }
                        curQCnt = Fvid2Utils_getNumQElem(bmObj->curQ);
                    }
                }
            }
        }
        /** Enable HW interrupts here */
        HwiP_restore(cookie);
    }
    else
    {
        chObj->instObj->status.spuriousUdmaIntrCount++;
    }
}

#if defined (SOC_J721E)
void CsirxDrv_udmaTDCEventCb(Udma_EventHandle eventHandle,
                             uint32_t eventType,
                             void *appData)
{
    int32_t retVal = UDMA_SOK;
    CsirxDrv_ChObj *chObj;
    CSL_UdmapTdResponse tdResp;

    if(NULL == eventHandle)
	{
		//To remove unused variable warning
	}

    chObj = (CsirxDrv_ChObj *) appData;
    GT_assert(CsirxTrace, (NULL != chObj));
    /* Take out completed TRPD from UDMA TDC Ring */
    if(UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
    {
        /* Response received in Teardown completion queue */
        retVal = Udma_chDequeueTdResponse(&chObj->rxChObj, &tdResp);
        if(UDMA_SOK != retVal)
        {
            /* [Error] No TD response after callback!! */
            GT_0trace(
                        CsirxTrace, GT_ERR,
                        "UDMA Tear Down Failed!!!\r\n");
        }
    }
    else
    {
        chObj->instObj->status.spuriousUdmaIntrCount++;
    }
}
#endif

void CsirxDrv_cacheWb(const void *addr, uint32_t size)
{
    uint32_t    isCacheCoherent = Udma_isCacheCoherent();

    if(UTRUE != isCacheCoherent)
    {
        CacheP_wb(addr, size);
    }

    return;
}

void CsirxDrv_cacheInv(const void * addr, uint32_t size)
{
    uint32_t    isCacheCoherent = Udma_isCacheCoherent();

    if(UTRUE != isCacheCoherent)
    {
        CacheP_Inv(addr, size);
    }

    return;
}

uint8_t* CsirxDrv_getTrpdMemAddr(CsirxDrv_ChObj *chObj, uint32_t qCnt)
{
    return (&gCsirxUdmaTprdMem[chObj->instObj->drvInstId][chObj->chId][qCnt][0U]);
}

int32_t CsirxDrv_udmaToFvid2ErrorMap(int32_t udmaErr)
{
     int32_t retVal;
  
     switch(udmaErr)
     {
         case UDMA_SOK :
            retVal = FVID2_SOK;
            break;
         case UDMA_EFAIL :
            retVal = FVID2_EFAIL;
            break;
         case UDMA_EBADARGS : 
            retVal = FVID2_EBADARGS;
            break;
         case UDMA_EINVALID_PARAMS : 
            retVal = FVID2_EINVALID_PARAMS;
            break;
         case UDMA_ETIMEOUT :
            retVal = FVID2_ETIMEOUT;
            break;
         case UDMA_EALLOC :
            retVal = FVID2_EALLOC;
            break;
         default :
            retVal = FVID2_EFAIL;
            break;
     }
     return retVal;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */
static uint16_t CsirxDrv_getIcnt0(const Csirx_ChCfg *chCfg)
{
    uint16_t icnt0 = 0U;
    uint32_t bpp  = 0U;
    uint32_t tempicnt0 = 0U;

    bpp = CsirxDrv_getStorageBpp(chCfg->inCsiDataType);

    if ((FVID2_CSI2_DF_RAW12      == chCfg->inCsiDataType) &&
        (FVID2_CCSF_BITS12_PACKED == chCfg->outFmt.ccsFormat))
    {
        /* over-writing bpp as this is packed format and it is valid only for RAW12 */
        bpp = CSIRX_BITS_PER_PIXEL_12_BITS;
        /* icnt0 expressed in bits */
        tempicnt0 = (chCfg->outFmt.width * bpp);
        /* icnt0 currently in bits, rounding off to next word boundary */
        icnt0 = (uint16_t)((tempicnt0 + 31U) / 32U);
        /* icnt0 in bytes */
        icnt0 = icnt0 * 4U;
    }
    else
    {
        /* icnt0 expressed in bits */
        tempicnt0 = (chCfg->outFmt.width * bpp);
        /* icnt0 in bytes */
        icnt0 = (uint16_t)(tempicnt0 / 8U);
    }

    return icnt0;
}
