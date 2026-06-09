/**
 *   Copyright (c) Texas Instruments Incorporated 2019
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
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 *  \file vhwa_m2mNfUdma.c
 *
 *  \brief Utility APIs for UDMA configuration
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mNfPriv.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define VHWA_M2M_NF_FLOW_ID     (0x3FFFU)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Local Function to initialize TR header for the
 *          given channel object. It initializes, number of TRs,
 *          completion Ring etc in the TR header.
 *
 * \param   pTrPdMem            Pointer to TR memory
 * \param   flowId              Flow ID
 * \param   returnRing          Return ring to be set in the TR header
 *
 **/
static void Vhwa_nfSetupPacketInfo(uint8_t *pTrPdMem, uint32_t flowId,
                                     uint32_t returnRing);

/**
 * \brief   Local Function to initialize transfer recoder for Current frame
 *          TR of the given channel object.
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void Vhwa_nfSetupTxTr(const Vhwa_M2mNfInstObj *instObj,
                               Vhwa_M2mNfHandleObj *hObj,
                               uint8_t *pTrPdMem,
                               uint32_t compCnt);

/**
 * \brief   Local Function to initialize transfer recoder for Reference
 *          TR of the given channel object.
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void Vhwa_nfSetupRxTr(const Vhwa_M2mNfInstObj *instObj,
                               Vhwa_M2mNfHandleObj *hObj,
                               uint8_t *pTrPdMem,
                               uint32_t compCnt);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**
 * \brief Ring memory, one ring for each output channel.
 */
static uint8_t gNfRingMem[VHWA_M2M_NF_MAX_DMA_CH]
    [VHWA_M2M_NF_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**
 * \brief Completion Ring memory, one ring for each output channel.
 */
static uint8_t gNfCompRingMem[VHWA_M2M_NF_MAX_DMA_CH]
    [VHWA_M2M_NF_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**
 * \brief Completion Ring memory, one ring for each output channel.
 */
static uint8_t gNfTdCompRingMem[VHWA_M2M_NF_MAX_DMA_CH]
    [VHWA_M2M_NF_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**
 * \brief Teardown Completion Ring memory, one ring for each output channel.
 */
static uint8_t gNfTxRxTprdMem[VHWA_M2M_NF_MAX_HANDLES]
    [VHWA_M2M_NF_MAX_DMA_CH][VHWA_M2M_NF_MAX_COMP][VHWA_M2M_NF_UDMA_TRPD_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**
 * \brief TRPD memory for each handle and for each channel in handle
 */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/* Below set of functions uses UDMA LLD to allocate, configure, start and     */
/* stop channels.                                                             */
/* ========================================================================== */

/* Function to initialize and allocate UTC/UDMA(External) Channels */
int32_t Vhwa_m2mNfUdmaInit(Vhwa_M2mNfInstObj *instObj,
                            Vhwa_M2mNfInitPrms *initPrms)
{
    int32_t                 status = UDMA_SOK;
    uint32_t                cnt;
    uint32_t                utcChId;
    uint32_t                chType;
    Udma_ChHandle           chHndl;
    Udma_ChPrms             chPrms;
    Udma_ChUtcPrms          utcPrms;

    /* Check for Null pointer,
       Internal function so checking with assert */
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != initPrms));

    utcChId = VHWA_NF_UTC_CH_START;

    for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_DMA_CH; cnt ++)
    {
        chHndl = &instObj->utcChObj[cnt];
        instObj->utcChHndl[cnt] = chHndl;

        /* Initialize channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);

        chPrms.chNum                = utcChId;
        chPrms.utcId                = VHWA_M2M_NF_UTC_ID;
        chPrms.fqRingPrms.ringMem   = &gNfRingMem[cnt][0U];
        chPrms.cqRingPrms.ringMem   = &gNfCompRingMem[cnt][0];
        chPrms.tdCqRingPrms.ringMem = &gNfTdCompRingMem[cnt][0U];
        chPrms.fqRingPrms.elemCnt   = VHWA_M2M_NF_UDMA_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = VHWA_M2M_NF_UDMA_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = VHWA_M2M_NF_UDMA_RING_ENTRIES;

        /* Open channel,
           since chNum is set to ANY in chPrms, it also allocates
           an external channel and completion channel */
        status = Udma_chOpen(initPrms->udmaDrvHndl, chHndl,
            chType, &chPrms);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */  
        if(UDMA_SOK != status)
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "UDMA channel open failed!!\n");
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if(UDMA_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Intialize UTC channel Paramenters */
            UdmaChUtcPrms_init(&utcPrms);

            utcPrms.chanType = (uint8_t)CSL_UDMAP_CHAN_TYPE_REF_TR_RING;

            /* Configure UTC channel,
               used to configure other channel specific parameters like
               priority, orderid, queue id etc */
            status = Udma_chConfigUtc(chHndl, &utcPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if(UDMA_SOK != status)
            {
                GT_0trace(VhwaNfTrace, GT_ERR,
                            "UDMA UTC channel config failed!!\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if(UDMA_SOK != status)
        {
            break;
        }
        /* LDRA_JUSTIFY_END */

        /* Locally store completion queue number, forward ring handle
           and completion ring handle */
        instObj->complRingNum[cnt] = Udma_chGetCqRingNum(chHndl);
        instObj->fqRingHndl[cnt] = Udma_chGetFqRingHandle(chHndl);
        instObj->cqRingHndl[cnt] = Udma_chGetCqRingHandle(chHndl);
        instObj->utcCh[cnt] = Udma_chGetNum(chHndl);

        utcChId++;
    }

    /* Convert UDMA status to FVID2 status,
       so that the caller of this function always uses FVID2 status only */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if(UDMA_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_SOK;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    else
    {
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

int32_t Vhwa_m2mNfUdmaDeInit(const Vhwa_M2mNfInstObj *instObj)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    Udma_ChHandle       chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_DMA_CH; cnt ++)
    {
        chHndl = instObj->utcChHndl[cnt];

        /* Close UDMA Channel */
        status = Udma_chClose(chHndl);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if(UDMA_SOK != status)
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                        "[Error] UDMA channel close failed!!\n");
        }
         /* LDRA_JUSTIFY_END */
    }

    /* Convert UDMA status to FVID2 status,
       so that the caller of this function always uses FVID2 status only */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if(UDMA_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_SOK;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    else
    {
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

int32_t Vhwa_m2mNfStartCh(const Vhwa_M2mNfInstObj *instObj)
{
    int32_t         status = FVID2_EBADARGS;
    uint32_t        cnt;
    Udma_ChHandle   chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    /* By default, all channels are enabled */
    for (cnt = 0u; cnt < VHWA_M2M_NF_MAX_DMA_CH; cnt ++)
    {
        chHndl = instObj->utcChHndl[cnt];

        /* UDMA Channel enable */
        status = Udma_chEnable(chHndl);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if (UDMA_SOK != status)
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "UDMA channel enable failed!!\n");
            status = FVID2_EFAIL;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Convert UDMA status to FVID2 status,
       so that the caller of this function always uses FVID2 status only */
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if(UDMA_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_SOK;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    else
    {
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

int32_t Vhwa_m2mNfStopCh(const Vhwa_M2mNfInstObj *instObj)
{
    int32_t         status = FVID2_EBADARGS;
    uint32_t        cnt;
    Udma_ChHandle   chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    /* By default, all channels are disabled, on the last handle close */
    for (cnt = 0u; cnt < VHWA_M2M_NF_MAX_DMA_CH; cnt ++)
    {
        chHndl = instObj->utcChHndl[cnt];

        /* UDMA Channel disable */
        status = Udma_chDisable(chHndl, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if(UDMA_SOK != status)
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "UDMA channel disable failed!!\n");
            status = FVID2_EFAIL;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Convert UDMA status to FVID2 status,
       so that the caller of this function always uses FVID2 status only */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(UDMA_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_SOK;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

int32_t Vhwa_m2mNfSubmitRing(Vhwa_M2mNfInstObj *instObj,
                             Vhwa_M2mNfHandleObj *hObj,
                             uint32_t itrCnt)
{
    int32_t              status = UDMA_SOK;
    uint32_t             chCnt;
    Vhwa_M2mNfChParams *chPrms = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != hObj));

    /* Enable the channel which are required */
    for (chCnt = 0; chCnt < VHWA_M2M_NF_MAX_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[chCnt];

        /* Submit TRPD to ring */
        status = Udma_ringQueueRaw(instObj->fqRingHndl[chCnt],
            (uint64_t) chPrms->trMem[itrCnt]);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if(UDMA_SOK != status)
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "Ring queue failed\n");
        }
         /* LDRA_JUSTIFY_END */
    }

    return (status);
}

int32_t Vhwa_m2mNfPopRings(Vhwa_M2mNfInstObj *instObj,
                           const Vhwa_M2mNfHandleObj *hObj,
                           uint32_t itrCnt)
{
    int32_t              status = FVID2_SOK;
    uint32_t             chCnt, repeatCnt;
    uint64_t             ringPopVal;
    Vhwa_M2mNfChParams  *chPrms2 = NULL;
    Vhwa_M2mNfHandleObj *hObj2 = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    /* Enable the channel which are required */
    for (chCnt = 0u; chCnt < VHWA_M2M_NF_MAX_DMA_CH; chCnt ++)
    {
        repeatCnt = 0u;

        do
        {
            /* Dequeue from completion ring for RX channel */
            status = Udma_ringDequeueRaw(instObj->cqRingHndl[chCnt],
                    &ringPopVal);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if(UDMA_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                break;
            }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
            repeatCnt ++;
        } while (repeatCnt < VHWA_M2M_NF_MAX_WAIT_LOOP_CNT);
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((repeatCnt < VHWA_M2M_NF_MAX_WAIT_LOOP_CNT) &&
            ((uint32_t)ringPopVal ==
                (uint32_t)hObj->chPrms[chCnt].trMem[itrCnt]))
        /* LDRA_JUSTIFY_END */
        {
            status = FVID2_SOK;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        else if ((repeatCnt < VHWA_M2M_NF_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal !=
                    (uint32_t)hObj->chPrms[chCnt].trMem[itrCnt]))
        {
            uint32_t hcnt = 0U;
            for (hcnt = 0U; hcnt < VHWA_M2M_NF_MAX_HANDLES; hcnt ++)
            {
                hObj2 = Vhwa_m2mNfGetHandleObj(hcnt);
                chPrms2 = &hObj2->chPrms[chCnt];

                if (((uint32_t)UTRUE == hObj2->isUsed) && 
                ((uint32_t)ringPopVal == (uint32_t)chPrms2->trMem[itrCnt]))
                {
                    status = FVID2_SOK;
                    break;
                }
            }
            if(hcnt >= VHWA_M2M_NF_MAX_HANDLES)
            {
                status = FVID2_EFAIL;
                GT_1trace(VhwaNfTrace, GT_ERR, "UDMA Ring dequeue mismatch for ch%d!!\n", chCnt);
            }
        }
        else
        {
            if (repeatCnt >= VHWA_M2M_NF_MAX_WAIT_LOOP_CNT)
            {
                status = FVID2_EFAIL;

                GT_1trace(VhwaNfTrace, GT_ERR, "UDMA Ring dequeue failed for ch%d!!\n", chCnt);
            }
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}

int32_t Vhwa_m2mNfAllocUdmaMem(Vhwa_M2mNfHandleObj *hObj)
{
    int32_t     status = FVID2_SOK;
    uint32_t    hIdx;
    uint32_t    chCnt, compCnt;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));

    hIdx = hObj->hIdx;

    for (chCnt = 0u; chCnt < VHWA_M2M_NF_MAX_DMA_CH; chCnt ++)
    {
        for (compCnt = 0u; compCnt < VHWA_M2M_NF_MAX_COMP; compCnt ++)
        {
            hObj->chPrms[chCnt].trMem[compCnt] =
                                    &gNfTxRxTprdMem[hIdx][chCnt][compCnt][0U];
        }
    }

    return (status);
}


void Vhwa_m2mNfSetTrDesc(const Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj)
{
    uint32_t               compCnt;
    Vhwa_M2mNfChParams    *chPrms;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != hObj));

    for (compCnt = 0u; compCnt < hObj->numIter; compCnt ++)
    {
        chPrms = &hObj->chPrms[0];

        /* Invalidate cache before changing contents */
        VhwaAl_Cache_Inv(chPrms->trMem[compCnt], VHWA_M2M_NF_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

        /* Setup Packet Info for the Tx Channel */
        Vhwa_nfSetupPacketInfo(chPrms->trMem[compCnt],
                VHWA_M2M_NF_FLOW_ID, instObj->complRingNum[0]);

        /* Setup Tx Transfer Record */
        Vhwa_nfSetupTxTr(instObj, hObj,
                            chPrms->trMem[compCnt], compCnt);

        /* Write back contents into memory after changing */
        VhwaAl_Cache_Wb(chPrms->trMem[compCnt], VHWA_M2M_NF_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

        chPrms = &hObj->chPrms[1];

        /* Invalidate cache before changing contents */
        VhwaAl_Cache_Inv(chPrms->trMem[compCnt], VHWA_M2M_NF_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

        /* Setup Packet Info for the Tx Channel */
        Vhwa_nfSetupPacketInfo(chPrms->trMem[compCnt],
                VHWA_M2M_NF_FLOW_ID, instObj->complRingNum[1]);

        /* Setup Rxs Transfer Record */
        Vhwa_nfSetupRxTr(instObj, hObj,
                            chPrms->trMem[compCnt], compCnt);

        /* Write back contents into memory after changing */
        VhwaAl_Cache_Wb(chPrms->trMem[compCnt], VHWA_M2M_NF_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);
    }
}

void Vhwa_m2mNfSetTRAddress(Vhwa_M2mNfHandleObj *hObj,
                            const Fvid2_FrameList *inFrmList,
                            const Fvid2_FrameList *outFrmList)
{
    uint32_t              packetInfoSize, compCnt;
    CSL_UdmapTR9         *pTr;
    Vhwa_M2mNfChParams   *chPrms;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != inFrmList));
    GT_assert(VhwaNfTrace, (NULL != outFrmList));

    packetInfoSize = sizeof(CSL_UdmapTR15);

    for (compCnt = 0u; compCnt < VHWA_M2M_NF_MAX_COMP; compCnt ++)
    {
        /* Set address for TX Channel */
        chPrms = &hObj->chPrms[0];

        pTr = (CSL_UdmapTR9 *)(&chPrms->trMem[compCnt][packetInfoSize]);

        VhwaAl_Cache_Inv(pTr, packetInfoSize, CacheP_TYPE_L1D);
        pTr->addr = (uint64_t)inFrmList->frames[0]->addr[compCnt];
        VhwaAl_Cache_Wb(pTr, packetInfoSize, CacheP_TYPE_L1D);

        /* Set address for RX channel */
        chPrms = &hObj->chPrms[1];

        pTr = (CSL_UdmapTR9 *)(&chPrms->trMem[compCnt][packetInfoSize]);
        VhwaAl_Cache_Inv(pTr, packetInfoSize, CacheP_TYPE_L1D);
        pTr->daddr = (uint64_t)outFrmList->frames[0]->addr[compCnt];
        VhwaAl_Cache_Wb(pTr, packetInfoSize, CacheP_TYPE_L1D);
    }
}

/* ========================================================================== */
/*                             Local Functions                                */
/* ========================================================================== */

static void Vhwa_nfSetupPacketInfo(uint8_t *pTrPdMem, uint32_t flowId,
                                     uint32_t returnRing)
{
    CSL_UdmapCppi5TRPD *pTrpd;
    uint32_t descType = CSL_UDMAP_CPPI5_PD_DESCINFO_DTYPE_VAL_TR;

    /* Check for Null Pointer */
    GT_assert(VhwaNfTrace, (NULL != pTrPdMem));

    pTrpd = (CSL_UdmapCppi5TRPD *) pTrPdMem;
    CSL_udmapCppi5SetDescType(pTrpd, descType);
    CSL_udmapCppi5TrSetReload(pTrpd, UFALSE, 0U);
    CSL_udmapCppi5SetPktLen(pTrpd, descType, VHWA_M2M_NF_UDMA_NUM_TR_DESC);
    /* Flow ID and Packet ID */
    CSL_udmapCppi5SetIds(pTrpd, descType, 0U, flowId);
    CSL_udmapCppi5SetSrcTag(pTrpd, 0x0025);
    CSL_udmapCppi5SetDstTag(pTrpd, 0x1234);
    CSL_udmapCppi5TrSetEntryStride(pTrpd,
        CSL_UDMAP_CPPI5_TRPD_PKTINFO_RECSIZE_VAL_64B);
    CSL_udmapCppi5SetReturnPolicy(pTrpd, descType,
        CSL_UDMAP_CPPI5_PD_PKTINFO2_RETPOLICY_VAL_ENTIRE_PKT,
        UFALSE, CSL_UDMAP_CPPI5_PD_PKTINFO2_RETPUSHPOLICY_VAL_TO_TAIL,
        returnRing);
}

static void Vhwa_nfSetupTxTr(const Vhwa_M2mNfInstObj *instObj,
                               Vhwa_M2mNfHandleObj *hObj,
                               uint8_t *pTrPdMem,
                               uint32_t compCnt)
{
    uint32_t icnt1, icnt2;
    uint32_t widthInBytes, imageHeight;
    int32_t  srcPitch, dstPitch;
    Vhwa_M2mNfConfig *nfCfg;
    CSL_UdmapTR9 *pTr = NULL;

    /* Check for Null Pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != pTrPdMem));

    pTr = (CSL_UdmapTR9 *) &(pTrPdMem[sizeof(CSL_UdmapTR15)]);

    nfCfg = &hObj->nfCfg[compCnt];

    widthInBytes = Vhwa_calcHorzSizeInBytes(nfCfg->inFmt.width,
                                            nfCfg->inFmt.ccsFormat);
    imageHeight = nfCfg->inFmt.height;
    srcPitch = (int32_t)nfCfg->inFmt.pitch[compCnt];
    dstPitch = (int32_t)instObj->sl2Prms.sl2Pitch[0];

    /* Initialize TR packet: Start */
    /* Setup TR */
    /* Bit fields::
       Bit No.->Info
       3:0->Type: 4D transfer
       7:6->Event Size: Event is generated every time ICNT1 is decremented by 1
       9:8->Trigger0: Local Event
       10:11->Trigger0 Type:
                    The second inner most loop (ICNT1) will be decremented by 1
       13:12->Trigger1: No Trigger
       15:14->Trigger1 Type:
                    The second inner most loop (ICNT1) will be decremented by 1
       23:16->Command ID: The Command ID will come back in TR response
       31:24->Conf Flags: Configuration Specific Flags
       */
    pTr->flags =
        CSL_FMK(UDMAP_TR_FLAGS_TYPE,
            CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING) |
        CSL_FMK(UDMAP_TR_FLAGS_STATIC, UFALSE) |
        CSL_FMK(UDMAP_TR_FLAGS_EOL, (uint32_t)UFALSE) |   /* NA */
        CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE,
            CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT1_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
            CSL_UDMAP_TR_FLAGS_TRIGGER_LOCAL_EVENT) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
                    /* TODO: Check this */
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
        /* CmdId: This will come back in TR response */
        CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x25U) |
        CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

    pTr->icnt0    = (uint16_t)widthInBytes;
    pTr->icnt1    = (uint16_t)imageHeight;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = srcPitch;
    pTr->dim2     = 0;
    pTr->dim3     = 0;
    pTr->addr     = 0x0U;

    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

    icnt1 = instObj->sl2Prms.sl2NumLines[0];
    icnt2 = imageHeight / icnt1;

    if ((icnt2 * icnt1) != imageHeight)
    {
        icnt2 ++;
    }

    pTr->dicnt0   = (uint16_t)widthInBytes;
    pTr->dicnt1   = (uint16_t)icnt1;
    pTr->dicnt2   = (uint16_t)icnt2;
    pTr->dicnt3   = 1u;

    pTr->ddim1    = dstPitch;
    pTr->ddim2    = 0;
    pTr->ddim3    = 0;
    pTr->daddr    = instObj->sl2Prms.sl2Addr[0];

}

static void Vhwa_nfSetupRxTr(const Vhwa_M2mNfInstObj *instObj,
                               Vhwa_M2mNfHandleObj *hObj,
                               uint8_t *pTrPdMem,
                               uint32_t compCnt)
{

    uint32_t icnt1, icnt2;
    uint32_t widthInBytes, imageHeight;
    int32_t  srcPitch, dstPitch;
    Vhwa_M2mNfConfig *nfCfg;
    CSL_UdmapTR9 *pTr = NULL;

    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != pTrPdMem));

    nfCfg = &hObj->nfCfg[compCnt];

    pTr = (CSL_UdmapTR9 *) &(pTrPdMem[sizeof(CSL_UdmapTR15)]);

    widthInBytes = Vhwa_calcHorzSizeInBytes(nfCfg->outFmt.width,
                                                nfCfg->outFmt.ccsFormat);
    imageHeight = nfCfg->outFmt.height;
    srcPitch = (int32_t)instObj->sl2Prms.sl2Pitch[1];
    dstPitch = (int32_t)nfCfg->outFmt.pitch[compCnt];

    icnt1 = instObj->sl2Prms.sl2NumLines[1];
    icnt2 = imageHeight / icnt1;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, it updates icnt2 value accordingly.
    <justification end> */
    if ((icnt2 * icnt1) != imageHeight)
    {
        icnt2 ++;
    }
    /* LDRA_JUSTIFY_END */

    /* Initialize TR packet: Start */
    /* Setup TR */
    /* Bit fields::
       Bit No.->Info
       3:0->Type: 4D transfer
       7:6->Event Size: Event is generated every time ICNT1 is decremented by 1
       9:8->Trigger0: Local Event
       10:11->Trigger0 Type:
                    The second inner most loop (ICNT1) will be decremented by 1
       13:12->Trigger1: No Trigger
       15:14->Trigger1 Type:
                    The second inner most loop (ICNT1) will be decremented by 1
       23:16->Command ID: The Command ID will come back in TR response
       31:24->Conf Flags: Configuration Specific Flags
       */
    pTr->flags =
        CSL_FMK(UDMAP_TR_FLAGS_TYPE,
            CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING) |
        CSL_FMK(UDMAP_TR_FLAGS_STATIC, UFALSE) |
        CSL_FMK(UDMAP_TR_FLAGS_EOL, (uint32_t)UFALSE) |
        CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE,
            CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT1_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
            CSL_UDMAP_TR_FLAGS_TRIGGER_LOCAL_EVENT) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x25U) |
        CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1u);

    pTr->icnt0    = (uint16_t)widthInBytes;
    pTr->icnt1    = (uint16_t)icnt1;
    pTr->icnt2    = (uint16_t)icnt2;
    pTr->icnt3    = 1U;
    pTr->dim1     = srcPitch;
    pTr->dim2     = 0;
    pTr->dim3     = 0;
    pTr->addr     = instObj->sl2Prms.sl2Addr[1];
    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_DST_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

    pTr->dicnt0   = (uint16_t)widthInBytes;
    pTr->dicnt1   = (uint16_t)imageHeight;
    pTr->dicnt2   = 1U;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = dstPitch;
    pTr->ddim2    = 0;
    pTr->ddim3    = 0;
    pTr->daddr    = 0x0U;

}
