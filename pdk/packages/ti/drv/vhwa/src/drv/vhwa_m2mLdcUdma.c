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
 *  \file vhwa_m2mLdcUdma.c
 *
 *  \brief Utility APIs for UDMA configuration
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mLdcPriv.h"



/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


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
 * \param   instObj             Instance object.
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   complRing           Return ring to be set in the TR header
 *
 **/
static void vhwaM2mLdcSetupPacketInfo(Vhwa_M2mLdcChParams *chPrms,
                                      uint32_t complRingNum);

/**
 * \brief   Local Function to initialize transfer recoder for all
 *          TRs of the given channel object. It traverse through all
 *          enabled regions, uses the icnts, dims calculated parameters
 *          channels params and region params and initializes TR
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void vhwaM2mLdcSetupRxTransferRecord(Vhwa_M2mLdcChParams *chPrms,
                                            CSL_UdmapTR9 *pTr);

/**
 * \brief   Local Function to set the output buffer address in the TR
 *          for the given channel. Buffer start address is stored in chPrms
 *          and offset for each region TR is pre-calculated, it uses
 *          them to set the start address in TR.
 *          This API does not take care of cache operation, this should be
 *          taken care by caller.
 *
 * \param   ldcCfg              LDC Configuration, used to check if
 *                              multi-region mode is enabled or not.
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTr                 Starting TR for this channel.
 *
 **/
static void vhwaM2mLdcSetTRAddress(const Ldc_Config *ldcCfg,
    Vhwa_M2mLdcChParams *chPrms, CSL_UdmapTR9 *pTr);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/**
 * \brief Ring memory, one ring for each output channel.
 */
uint8_t gLdcRingMem[VHWA_M2M_LDC_MAX_DMA_CH]
    [VHWA_M2M_LDC_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

#if !defined(MCU_PLUS_SDK)
/**
 * \brief Completion Ring memory, one ring for each output channel.
 */
uint8_t gLdcCompRingMem[VHWA_M2M_LDC_MAX_DMA_CH]
    [VHWA_M2M_LDC_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**
 * \brief Teardown Completion Ring memory, one ring for each output channel.
 */
int8_t gLdcTdCompRingMem[VHWA_M2M_LDC_MAX_DMA_CH]
    [VHWA_M2M_LDC_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#endif
/**
 * \brief TRPD memory for each handle and for each channel in handle
 */
uint8_t gLdcRxTprdMem[VHWA_M2M_LDC_MAX_HANDLES]
    [VHWA_M2M_LDC_MAX_DMA_CH][VHWA_M2M_LDC_UDMA_TRPD_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/* Below set of functions uses UDMA LLD to allocate, configure, start and     */
/* stop channels.                                                             */
/* ========================================================================== */

/* Function to initialize and allocate UTC/UDMA(External) Channels */
int32_t Vhwa_m2mLdcUdmaInit(Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcInitParams *initPrms)
{
    int32_t                 status = UDMA_SOK;
    uint32_t                cnt;
    uint32_t                chType;
    Udma_ChHandle           chHndl;
    Udma_ChPrms             chPrms;
    Udma_ChUtcPrms          utcPrms;

    /* Check for Null pointer,
       Internal function so checking with assert */
    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != initPrms));

    for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_DMA_CH; cnt ++)
    {
        chHndl = &instObj->utcChObj[cnt];
        instObj->utcChHndl[cnt] = chHndl;

        /* Initialize channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);

        chPrms.chNum                = VHWA_LDC_UTC_CH_START + cnt;
        chPrms.utcId                = VHWA_M2M_LDC_UTC_ID;
        chPrms.fqRingPrms.ringMem   = &gLdcRingMem[cnt][0U];
        #if defined (MCU_PLUS_SDK)
        chPrms.cqRingPrms.ringMem   = NULL;
        chPrms.tdCqRingPrms.ringMem = NULL;
        #else
        chPrms.cqRingPrms.ringMem   = &gLdcCompRingMem[cnt][0];
        chPrms.tdCqRingPrms.ringMem = &gLdcTdCompRingMem[cnt][0U];
        #endif
        chPrms.fqRingPrms.elemCnt   = VHWA_M2M_LDC_UDMA_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = VHWA_M2M_LDC_UDMA_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = VHWA_M2M_LDC_UDMA_RING_ENTRIES;

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
            GT_0trace(VhwaLdcTrace, GT_ERR, "UDMA channel open failed!!\n");
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

            #if defined(MCU_PLUS_SDK)
            utcPrms.chanType = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
            utcPrms.druOwner = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
            #else
            utcPrms.chanType = (uint8_t)CSL_UDMAP_CHAN_TYPE_REF_TR_RING;
            utcPrms.busOrderId = 1;
            utcPrms.busPriority = 3;
            #endif

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
                GT_0trace(VhwaLdcTrace, GT_ERR,
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
    This failure case is out of scope for the imaging test framework.
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

int32_t Vhwa_m2mLdcUdmaDeInit(const Vhwa_M2mLdcInstObj *instObj)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    Udma_ChHandle       chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != instObj));

    for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_DMA_CH; cnt ++)
    {
        chHndl = instObj->utcChHndl[cnt];

        /* Close UDMA Channel */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        status = Udma_chClose(chHndl);
        if(UDMA_SOK != status)
        {
            GT_0trace(VhwaLdcTrace, GT_ERR,
                        "[Error] UDMA channel close failed!!\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Convert UDMA status to FVID2 status,
       so that the caller of this function always uses FVID2 status only */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    //REVIEW
    if(UDMA_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_SOK;
    }
    
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
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

int32_t Vhwa_m2mLdcStartCh(const Vhwa_M2mLdcInstObj *instObj)
{
    int32_t         status = FVID2_EBADARGS;
    uint32_t        cnt;
    Udma_ChHandle   chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != instObj));

    /* By default, all channels are enabled */
    for (cnt = 0u; cnt < VHWA_M2M_LDC_MAX_DMA_CH; cnt ++)
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
            GT_0trace(VhwaLdcTrace, GT_ERR,
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
    This failure case is out of scope for the imaging test framework.
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

int32_t Vhwa_m2mLdcStopCh(const Vhwa_M2mLdcInstObj *instObj)
{
    int32_t         status = FVID2_EBADARGS;
    uint32_t        cnt;
    Udma_ChHandle   chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != instObj));

    /* By default, all channels are disabled, on the last handle close */
    for (cnt = 0u; cnt < VHWA_M2M_LDC_MAX_DMA_CH; cnt ++)
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
            GT_0trace(VhwaLdcTrace, GT_ERR,
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
    This failure case is out of scope for the imaging test framework.
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

int32_t Vhwa_m2mLdcSubmitRing(Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj)
{
    int32_t              status = UDMA_SOK;
    uint32_t             chCnt;
    Vhwa_M2mLdcChParams *chPrms = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != hObj));

    /* Enable the channel which are required */
    for (chCnt = 0; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            /* Submit TRPD to ring */
            #if defined (MCU_PLUS_SDK)
            status = Udma_chRingQueueRaw(instObj->utcChHndl[chCnt],
                                            chPrms->rxTrMem,
                                            1U);
            #else
            status = Udma_ringQueueRaw(instObj->fqRingHndl[chCnt],
                (uint64_t) chPrms->rxTrMem);
            #endif

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
                GT_0trace(VhwaLdcTrace, GT_ERR, "Ring queue failed\n");
            }
            /* LDRA_JUSTIFY_END */
            #if defined (MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (UDMA_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                status = Udma_chRingRingDbRaw(instObj->utcChHndl[chCnt], 1U);
            }
            #endif /* SOC_AM62AX */
        }
    }

    return (status);
}
#if !defined (MCU_PLUS_SDK)
int32_t Vhwa_m2mLdcPopRings(Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj)
{
    int32_t              status = FVID2_SOK;
    uint32_t             chCnt;
    uint64_t             ringPopVal;
    Vhwa_M2mLdcChParams *chPrms = NULL;
    #if !defined(MCU_PLUS_SDK)
    uint32_t repeatCnt;
    Vhwa_M2mLdcChParams *chPrms2 = NULL;
    Vhwa_M2mLdcHandleObj *hObj2 = NULL;
    #endif

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != hObj));

    /* Enable the channel which are required */
    for (chCnt = 0u; chCnt < VHWA_M2M_LDC_MAX_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            #if defined (MCU_PLUS_SDK)
            status = Udma_chRingDeQueueRaw(instObj->utcChHndl[chCnt], 1U, &ringPopVal);
            if(UDMA_SOK != status)
            {
                GT_1trace(VhwaLdcTrace, GT_ERR,
                    "UDMA Failed to pop Ring for ch%d!!\n", chCnt);
                break;
            }
            if (UDMA_SOK == status)
            {
                status = Udma_chRingRingRvrDbRaw(instObj->utcChHndl[chCnt], 1U);
            }

            #else
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
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
                repeatCnt ++;
            } while (repeatCnt < VHWA_LDC_MSC_MAX_WAIT_LOOP_CNT);
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((repeatCnt < VHWA_LDC_MSC_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal == (uint32_t)chPrms->rxTrMem))
            /* LDRA_JUSTIFY_END */
            {
                status = FVID2_SOK;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((repeatCnt < VHWA_LDC_MSC_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal != (uint32_t)chPrms->rxTrMem))
            /* LDRA_JUSTIFY_END */
            {
                uint32_t hcnt = 0U;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                for (hcnt = 0U; hcnt < VHWA_M2M_LDC_MAX_HANDLES; hcnt ++)
                /* LDRA_JUSTIFY_END */
                {
                    hObj2 = Vhwa_m2mLdcGetHandleObj(hcnt);
                    chPrms2 = &hObj2->chPrms[chCnt];

                    if (((uint32_t)UTRUE == hObj2->isUsed) && 
                    ((uint32_t)ringPopVal == (uint32_t)chPrms2->rxTrMem))
                    {
                        status = FVID2_SOK;
                        break;
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested. 
                <justification end> */
                if(hcnt >= VHWA_M2M_LDC_MAX_HANDLES)
                {
                    status = FVID2_EFAIL;
                    GT_1trace(VhwaLdcTrace, GT_ERR, "UDMA Ring dequeue mismatch for ch%d!!\n", chCnt);
                }
                /* LDRA_JUSTIFY_END */
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                if (repeatCnt >= VHWA_LDC_MSC_MAX_WAIT_LOOP_CNT)
                {
                    status = FVID2_EFAIL;

                    GT_1trace(VhwaLdcTrace, GT_ERR, "UDMA Ring dequeue failed for ch%d!!\n", chCnt);
                }
            }
            /* LDRA_JUSTIFY_END */
            #endif
        }
    }

    return (status);
}
#endif
int32_t Vhwa_m2mLdcAllocUdmaMem(Vhwa_M2mLdcHandleObj *hObj)
{
    int32_t     status = FVID2_SOK;
    uint32_t    hIdx;
    uint32_t    chCnt = 0u;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != hObj));

    hIdx = hObj->hIdx;

    for (chCnt = 0u; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
    {
        hObj->chPrms[chCnt].rxTrMem = &gLdcRxTprdMem[hIdx][chCnt][0U];
        Fvid2Utils_memset(hObj->chPrms[chCnt].rxTrMem, 0x0,
            VHWA_M2M_LDC_UDMA_TRPD_SIZE);
    }

    return (status);
}


void Vhwa_m2mLdcSetTrDesc(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t               chCnt;
    uint8_t               *pTrMem;
    CSL_UdmapTR9          *pTr9;
    uint32_t               packetInfoSize;
    Vhwa_M2mLdcChParams   *chPrms;

    packetInfoSize = sizeof(CSL_UdmapTR15);

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != hObj));

    for (chCnt = 0u; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            pTrMem = chPrms->rxTrMem;
            pTr9 = (CSL_UdmapTR9 *)((uint32_t)pTrMem + packetInfoSize);

            /* Invalidate cache before changing contents */
            VhwaAl_Cache_Inv(pTrMem, VHWA_M2M_LDC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

            /* Setup Packet Info for the Tx Channel */
            vhwaM2mLdcSetupPacketInfo(chPrms,
                instObj->complRingNum[chCnt]);

            /* Setup Transfer Record */
            vhwaM2mLdcSetupRxTransferRecord(chPrms, pTr9);

            /* Write back contents into memory after changing */
            VhwaAl_Cache_Wb(pTrMem, VHWA_M2M_LDC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);
        }
    }
}

void Vhwa_m2mLdcSetOutputAddress(Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t             packetInfoSize, chCnt;
    uint8_t             *pTrMem;
    CSL_UdmapTR9        *pTr9;
    Vhwa_M2mLdcChParams *chPrms = NULL;

    packetInfoSize = sizeof(CSL_UdmapTR15);

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != hObj));

    /* Enable the channel which are required */
    for (chCnt = 0; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            pTrMem = chPrms->rxTrMem;
            pTr9 = (CSL_UdmapTR9 *)((uint32_t)pTrMem + packetInfoSize);

            /* Invalidate cache before changing contents */
            VhwaAl_Cache_Inv(pTrMem, VHWA_M2M_LDC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

            /* Set the addresses in TRs */
            vhwaM2mLdcSetTRAddress(&hObj->ldcCfg, chPrms, pTr9);

            *(uint32_t *)chPrms->rxTrRespMem = 0xFFFFFFFFU;

            /* Write back contents into memory after changing */
            VhwaAl_Cache_Wb(pTrMem, VHWA_M2M_LDC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);
        }
    }
}


/* ========================================================================== */
/*                             Local Functions                                */
/* ========================================================================== */

static void vhwaM2mLdcSetupPacketInfo(Vhwa_M2mLdcChParams *chPrms,
                                      uint32_t complRingNum)
{
    uint32_t             numTr;
    uint32_t             descType = CSL_UDMAP_CPPI5_PD_DESCINFO_DTYPE_VAL_TR;
    uint32_t             packetInfoSize;
    CSL_UdmapCppi5TRPD  *pTrpd = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != chPrms));

    packetInfoSize = sizeof(CSL_UdmapTR15);
    pTrpd = (CSL_UdmapCppi5TRPD *) chPrms->rxTrMem;
    numTr = chPrms->totalTRs;

    /* Calculate start address for the TR response,
     * This is dependent on the number of TRs
     * Added 1 here for the TR header. */
    chPrms->rxTrRespMem = (uint8_t *)((uint32_t)chPrms->rxTrMem +
                                        (packetInfoSize * (numTr + 1U)));

    CSL_udmapCppi5SetDescType(pTrpd, descType);
    CSL_udmapCppi5TrSetReload(pTrpd, UFALSE, 0U);
    CSL_udmapCppi5SetPktLen(pTrpd, descType, numTr);
    /* Flow ID and Packet ID */
    CSL_udmapCppi5SetIds(pTrpd, descType, 0U, VHWA_LDC_FLOW_ID);
    CSL_udmapCppi5SetSrcTag(pTrpd, 0x0025);             /* Not used */
    CSL_udmapCppi5SetDstTag(pTrpd, 0x1234);             /* Not used */
    CSL_udmapCppi5TrSetEntryStride(pTrpd,
        CSL_UDMAP_CPPI5_TRPD_PKTINFO_RECSIZE_VAL_64B);
    CSL_udmapCppi5SetReturnPolicy(pTrpd, descType,
        /* Don't care for TR */
        CSL_UDMAP_CPPI5_PD_PKTINFO2_RETPOLICY_VAL_ENTIRE_PKT,
        /* Early return */
        UFALSE, CSL_UDMAP_CPPI5_PD_PKTINFO2_RETPUSHPOLICY_VAL_TO_TAIL,
        complRingNum);
}

static void vhwaM2mLdcSetTRAddress(const Ldc_Config *ldcCfg,
    Vhwa_M2mLdcChParams *chPrms, CSL_UdmapTR9 *pTr)
{
    uint32_t                 trCnt;
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    uint64_t                 startAddr = 0u;
    CSL_UdmapTR9            *pTrLocal;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != ldcCfg));
    GT_assert(VhwaLdcTrace, (NULL != chPrms));
    GT_assert(VhwaLdcTrace, (NULL != pTr));

    pTrLocal = pTr;

    startAddr = chPrms->startAddr;
    if ((uint32_t)UFALSE == ldcCfg->enableMultiRegions)
    {
        /* Region00 is used in case multi regions are disabed */
        regPrms = &chPrms->regPrms[0u][0u];
        for (trCnt = 0u; trCnt < regPrms->numTr; trCnt ++)
        {
            pTrLocal->daddr = startAddr + regPrms->bufAddrOff[trCnt];
            pTrLocal ++;
        }
    }
    else
    {
        for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
        {
            for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
            {
                regPrms = &chPrms->regPrms[vCnt][hCnt];
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == regPrms->isEnabled)
                /* LDRA_JUSTIFY_END */
                {
                    for (trCnt = 0u; trCnt < regPrms->numTr; trCnt ++)
                    {
                        pTrLocal->daddr = startAddr + regPrms->bufAddrOff[trCnt];

                        /* It is ok to increament TR as TRs are arranged
                         * row wise ie first TR is for the region00,
                         * followed by TR for region01, then for region02,
                         * then for region 10 and so on.. */
                        pTrLocal ++;
                    }
                }
            }
        }
    }
}

static void vhwaM2mLdcSetupRxTransferRecord(Vhwa_M2mLdcChParams *chPrms,
                                            CSL_UdmapTR9 *pTr)
{
    uint32_t                 trCnt;
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    uint32_t                 blockHeight;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;
    CSL_UdmapTR9            *pTrLocal;

    /* Check for Null pointer */
    GT_assert(VhwaLdcTrace, (NULL != chPrms));
    GT_assert(VhwaLdcTrace, (NULL != pTr));

    pTrLocal = pTr;

    /* For each region in the channel, if it is enabled,
       initialize TR for the transfer */
    for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
    {
        for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
        {
            regPrms = &chPrms->regPrms[vCnt][hCnt];

            /* Block Height for the first TR is same as original block height,
               But it could be different for the subsequent TRs */
            blockHeight = regPrms->blockHeight;

            for (trCnt = 0u; ((uint32_t)UTRUE == regPrms->isEnabled) &&
                    (trCnt < regPrms->numTr); trCnt ++)
            {
                pTrLocal->flags =
                    CSL_FMK(UDMAP_TR_FLAGS_TYPE,
                        CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING) |
                    CSL_FMK(UDMAP_TR_FLAGS_STATIC, UFALSE) |
                    CSL_FMK(UDMAP_TR_FLAGS_EOL, (uint32_t)UFALSE) |   /* NA */
                    CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE,
                        CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT2_DEC) |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
                        CSL_UDMAP_TR_FLAGS_TRIGGER_LOCAL_EVENT) |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
                        CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT2_DEC) |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
                        CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
                        CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
                    /* CmdId: This will come back in TR response */
                    CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x25U) |
                    CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
                    CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
                    CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

                pTrLocal->fmtflags = (uint32_t)0x0U |
                    CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
                        CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
                    CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
                        CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
                    CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

                pTrLocal->icnt0 = (uint16_t)regPrms->blockWidthInBytes;
                pTrLocal->icnt1 = (uint16_t)blockHeight;
                pTrLocal->icnt2 = (uint16_t)regPrms->sicnt2[trCnt];
                pTrLocal->icnt3 = (uint16_t)regPrms->sicnt3[trCnt];

                pTrLocal->dim1 = (int32_t)chPrms->maxSl2BlockPitch;
                pTrLocal->dim2 = (int32_t)chPrms->maxSl2BlockPitch *
                                (int32_t)regPrms->blockHeight;
                pTrLocal->dim3 = 0;

                pTrLocal->dicnt0 = (uint16_t)regPrms->blockWidthInBytes;
                pTrLocal->dicnt1 = (uint16_t)blockHeight;
                pTrLocal->dicnt2 = (uint16_t)regPrms->dicnt2[trCnt];
                pTrLocal->dicnt3 = (uint16_t)regPrms->dicnt3[trCnt];

                pTrLocal->ddim1 = (int32_t)chPrms->pitch;
                pTrLocal->ddim2 = (int32_t)regPrms->blockWidthInBytes;
                pTrLocal->ddim3 = pTrLocal->ddim1 * (int32_t)blockHeight;

                pTrLocal->addr  = regPrms->sl2Addr[trCnt];
                pTrLocal->daddr = regPrms->sl2Addr[trCnt];

                pTrLocal ++;

                blockHeight = regPrms->height % regPrms->blockHeight;
            }
        }
    }
}


uint32_t Vhwa_m2mLdcCalcHorzSizeInBytes(uint32_t width, uint32_t ccsf,
    uint32_t dataFmt)
{
    uint32_t sizeInBytes, tWidth;

    if ((FVID2_DF_YUV422I_UYVY == (Fvid2_DataFormat)dataFmt) ||
        (FVID2_DF_YUV422I_YUYV == (Fvid2_DataFormat)dataFmt))
    {
        tWidth = width * 2U;
    }
    else
    {
        tWidth = width;
    }

    sizeInBytes = Vhwa_calcHorzSizeInBytes(tWidth, ccsf);

    return (sizeInBytes);
}

