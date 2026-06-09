/**
 *   Copyright (c) Texas Instruments Incorporated 2018
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
 *  \file vhwa_m2mMscUdma.c
 *
 *  \brief Utility APIs for UDMA configuration
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mMscPriv.h"



/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief FLOW ID for MSC UDMA Channels */
#define VHWA_M2M_MSC_FLOW_ID            (0x3FFFU)

/** \brief Number of ring entries - these many requests can be
 *         submitted at max */
#define VHWA_MSC_UDMA_RING_ENTRIES      (16u)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define VHWA_MSC_UDMA_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define VHWA_MSC_UDMA_RING_MEM_SIZE     (VHWA_MSC_UDMA_RING_ENTRIES * \
                                         VHWA_MSC_UDMA_RING_ENTRY_SIZE)

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
 * \param   flowId              Flow ID, Currently not used
 * \param   returnRing          Return ring ID
 *
 **/
static void Vhwa_mscSetupPacketInfo(uint8_t *pTrPdMem,
                                    uint32_t flowId,
                                    uint32_t returnRing);

/**
 * \brief   Local Function to initialize transfer recoder for input channel
 *          of the given channel object. It traverse through all
 *          enabled regions, uses the icnts, dims calculated parameters
 *          channels params and initializes TR
 *
 * \param   hObj                Handle Object
 * \param   comObj              Common Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTrPdMem            Start address of the TR for this channel
 * \param   compCnt             component count
 *
 **/
static void Vhwa_mscSetupTxTr(Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscCommonObj *comObj,
                              const Vhwa_M2mMscInChParams *chPrms,
                              uint8_t *pTrPdMem,
                              uint32_t compCnt,
                              uint32_t chCnt);

/**
 * \brief   Local Function to initialize transfer recoder for output channel
 *          of the given channel object. It traverse through all
 *          enabled regions, uses the icnts, dims calculated parameters
 *          channels params and initializes TR
 *
 * \param   hObj                Handle Object
 * \param   comObj              Common Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   cnCnt               output channel count
 * \param   pTrPdMem            Start address of the TR for this channel
 * \param   compCnt             component count
 *
 **/
static void Vhwa_mscSetupRxTr(Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscCommonObj *comObj,
                              const Vhwa_M2mMscOutChParams *chPrms,
                              uint32_t chCnt,
                              uint8_t *pTrPdMem,
                              uint32_t compCnt);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/** Varibale for UDMA memories */
static uint8_t gMscRingMem[VHWA_M2M_MSC_MAX_DMA_CH]
         [VHWA_MSC_UDMA_RING_MEM_SIZE]
         __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#if !defined(MCU_PLUS_SDK)
static uint8_t gMscCompRingMem[VHWA_M2M_MSC_MAX_DMA_CH][VHWA_MSC_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

static uint8_t gMscTdCompRingMem[VHWA_M2M_MSC_MAX_DMA_CH][VHWA_MSC_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#endif
static uint8_t gMscRxTprdMem[VHWA_M2M_MSC_MAX_INST][VHWA_M2M_MSC_MAX_HANDLES]
    [MSC_MAX_OUTPUT][VHWA_M2M_MSC_MAX_COMP]
    [VHWA_MSC_UDMA_TRPD_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

static uint8_t gMscTxTprdMem[VHWA_M2M_MSC_MAX_INST][VHWA_M2M_MSC_MAX_HANDLES]
    [VHWA_M2M_MSC_MAX_IN_CHANNEL][VHWA_M2M_MSC_MAX_COMP]
    [VHWA_MSC_UDMA_TRPD_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mMscUdmaChInit(Vhwa_M2mMscCommonObj *comObj)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            cnt, cnt2, tCnt;
    uint32_t            utcChId;
    uint32_t            chType;
    Udma_ChHandle       chHandle;
    Udma_ChPrms         chPrms;
    Udma_ChUtcPrms      utcPrms;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != comObj));

    utcChId = VHWA_MSC_UTC_CH_START;

    tCnt = 0u;

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        for (cnt2 = 0U; cnt2 < VHWA_M2M_MSC_MAX_IN_CHANNEL; cnt2 ++)
        {
            chHandle = &comObj->inChObj[cnt][cnt2];
            comObj->inChHandle[cnt][cnt2] = chHandle;

            /* Initialize channel parameters */
            chType = UDMA_CH_TYPE_UTC;
            UdmaChPrms_init(&chPrms, chType);
            chPrms.utcId                = VHWA_M2M_MSC_UTC_ID;
            chPrms.chNum                = utcChId;
            chPrms.fqRingPrms.ringMem   = &gMscRingMem[tCnt][0U];
            #if defined (MCU_PLUS_SDK)
            chPrms.cqRingPrms.ringMem   = NULL;
            chPrms.tdCqRingPrms.ringMem = NULL;
            #else
            chPrms.cqRingPrms.ringMem   = &gMscCompRingMem[tCnt][0];
            chPrms.tdCqRingPrms.ringMem = &gMscTdCompRingMem[tCnt][0U];
            #endif
            chPrms.fqRingPrms.elemCnt   = VHWA_MSC_UDMA_RING_ENTRIES;
            chPrms.cqRingPrms.elemCnt   = VHWA_MSC_UDMA_RING_ENTRIES;
            chPrms.tdCqRingPrms.elemCnt = VHWA_MSC_UDMA_RING_ENTRIES;

            /* Open channel */
            retVal = Udma_chOpen(comObj->mscInitPrms.drvHandle, chHandle,
                                    chType, &chPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (UDMA_SOK != retVal)
            {
                GT_0trace(VhwaMscTrace, GT_ERR, "UDMA channel open failed!!\n");
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
            if (UDMA_SOK == retVal)
            /* LDRA_JUSTIFY_END */

            {
                /* Intialize UTC channel Paramenters */
                UdmaChUtcPrms_init(&utcPrms);
                #if defined (MCU_PLUS_SDK)
                utcPrms.chanType = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
                utcPrms.druOwner = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
                #else
                utcPrms.chanType = (uint8_t)CSL_UDMAP_CHAN_TYPE_REF_TR_RING;
                utcPrms.busOrderId = 2;
                utcPrms.busPriority = 3;
                #endif


                /* Configure UTC channel */
                retVal = Udma_chConfigUtc(chHandle, &utcPrms);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(UDMA_SOK != retVal)
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                                "UDMA UTC channel config failed!!\n");
                }
                /* LDRA_JUSTIFY_END */

            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (UDMA_SOK != retVal)
            {
                break;
            }
            /* LDRA_JUSTIFY_END */

            comObj->inComplRingNum[cnt][cnt2] = Udma_chGetCqRingNum(chHandle);

            utcChId ++;
            tCnt++;
        }
    }

    for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
    {
        tCnt = cnt + (VHWA_M2M_MSC_MAX_IN_CHANNEL * VHWA_M2M_MSC_MAX_INST);
        chHandle = &comObj->outChObj[cnt];
        comObj->outChHandle[cnt] = chHandle;

        /* Init channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.utcId                = VHWA_M2M_MSC_UTC_ID;
        chPrms.chNum                = utcChId;
        chPrms.fqRingPrms.ringMem   = &gMscRingMem[tCnt][0U];
        #if defined (MCU_PLUS_SDK)
        chPrms.cqRingPrms.ringMem   = NULL;
        chPrms.tdCqRingPrms.ringMem = NULL;
        #else
        chPrms.cqRingPrms.ringMem   = &gMscCompRingMem[tCnt][0];
        chPrms.tdCqRingPrms.ringMem = &gMscTdCompRingMem[tCnt][0U];
        #endif
        chPrms.fqRingPrms.elemCnt   = VHWA_MSC_UDMA_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = VHWA_MSC_UDMA_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = VHWA_MSC_UDMA_RING_ENTRIES;

        /* Open channel */
        retVal = Udma_chOpen(comObj->mscInitPrms.drvHandle, chHandle,
                                chType, &chPrms);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if (UDMA_SOK != retVal)
        {
            GT_0trace(VhwaMscTrace, GT_ERR, "UDMA channel open failed!!\n");
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        <justification end> */ 
        if (UDMA_SOK == retVal)
        /* LDRA_JUSTIFY_END */

        {
            /* Intialize UTC channel Paramenters */
            UdmaChUtcPrms_init(&utcPrms);
            #if defined (MCU_PLUS_SDK)
            utcPrms.chanType = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
            utcPrms.druOwner = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
            #else
            utcPrms.chanType = (uint8_t)CSL_UDMAP_CHAN_TYPE_REF_TR_RING;
            utcPrms.busOrderId = 2;
            utcPrms.busPriority = 3;
            #endif

            /* Configure UTC channel */
            retVal = Udma_chConfigUtc(chHandle, &utcPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (UDMA_SOK != retVal)
            {
                GT_0trace(VhwaMscTrace, GT_ERR,
                            "UDMA UTC channel config failed!!\n");
            }
             /* LDRA_JUSTIFY_END */

        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (UDMA_SOK != retVal)
        {
            break;
        }
        /* LDRA_JUSTIFY_END */


        comObj->outComplRingNum[cnt] = Udma_chGetCqRingNum(chHandle);

        utcChId ++;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UDMA_SOK == retVal)
    {
        retVal = FVID2_SOK;
    }
    /* LDRA_JUSTIFY_END */
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
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */


    return retVal;
}

int32_t Vhwa_m2mMscUdmaChDeInit(const Vhwa_M2mMscCommonObj *comObj)
{
    int32_t             retVal = FVID2_SOK;
    uint32_t            cnt, cnt2;
    Udma_ChHandle       chHandle;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != comObj));

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        for (cnt2 = 0U; cnt2 < VHWA_M2M_MSC_MAX_IN_CHANNEL; cnt2 ++)
        {
            chHandle = comObj->inChHandle[cnt][cnt2];

            /* Close UDMA Channel */
            retVal += Udma_chClose(chHandle);
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (UDMA_SOK != retVal)
            {
                GT_0trace(VhwaMscTrace, GT_ERR,
                            "[Error] UDMA channel close failed!!\n");
            }
            /* LDRA_JUSTIFY_END */

        }
    }

    for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
    {
        chHandle = comObj->outChHandle[cnt];

        /* Close UDMA Channel */
        retVal += Udma_chClose(chHandle);
        /* LDRA_JUSTIFY_START
        <metric start>statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if (UDMA_SOK != retVal)
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                        "[Error] UDMA channel close failed!!\n");
        }
        /* LDRA_JUSTIFY_END */

    }

    return retVal;
}

void Vhwa_mscM2mSetTrDesc(Vhwa_M2mMscHandleObj *hObj,
                          const Vhwa_M2mMscCommonObj *comObj)
{
    uint32_t               chCnt;
    uint32_t               compCnt;
    Vhwa_M2mMscInChParams  *inChPrms;
    Vhwa_M2mMscOutChParams *outChPrms;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));

    /* Setup the Input TR */
    for (chCnt = 0U; (chCnt < hObj->numInChUsed); chCnt ++)
    {
        inChPrms = &hObj->inChPrms[chCnt];

        for (compCnt = 0u; compCnt < hObj->numIter; compCnt ++)
        {
            VhwaAl_Cache_Inv(inChPrms->trMem[compCnt], VHWA_MSC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

            /* Setup Packet Info for the Channel */
            Vhwa_mscSetupPacketInfo(inChPrms->trMem[compCnt],
                VHWA_M2M_MSC_FLOW_ID, comObj->inComplRingNum[hObj->instId][chCnt]);

            /* Setup TR for TX channel */
            Vhwa_mscSetupTxTr(hObj, comObj, inChPrms, inChPrms->trMem[compCnt],
                              compCnt, chCnt);

            VhwaAl_Cache_Wb(inChPrms->trMem[compCnt], VHWA_MSC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);
        }
    }

    /* Setup the Output Scalar TR's */
    for (chCnt = 0u; chCnt < MSC_MAX_OUTPUT; chCnt ++)
    {
        outChPrms = &hObj->outChPrms[chCnt];

        if (UTRUE == outChPrms->buffEnable)
        {
            for (compCnt = 0u; compCnt < hObj->numIter; compCnt ++)
            {
                VhwaAl_Cache_Inv(outChPrms->trMem[compCnt], VHWA_MSC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

                /* Setup Packet Info for the Channel */
                Vhwa_mscSetupPacketInfo(outChPrms->trMem[compCnt],
                    VHWA_M2M_MSC_FLOW_ID, comObj->outComplRingNum[chCnt]);

                /* Setup TR for RX channel */
                Vhwa_mscSetupRxTr(hObj, comObj, outChPrms, chCnt,
                                  outChPrms->trMem[compCnt], compCnt);

                VhwaAl_Cache_Wb(outChPrms->trMem[compCnt], VHWA_MSC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);
            }
        }
    }

    return;
}

int32_t Vhwa_m2mMscStartCh(const Vhwa_M2mMscInstObj *instObj,
    const Vhwa_M2mMscCommonObj *comObj)
{
    int32_t         retVal = UDMA_SOK;
    uint32_t        cnt, cnt2;
    Udma_ChHandle   chHandle;
     /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == comObj))
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "NULL pointer\n");
        retVal = UDMA_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        for (cnt2 = 0U; cnt2 < VHWA_M2M_MSC_MAX_IN_CHANNEL; cnt2 ++)
        {
            if(UDMA_SOK != retVal)
            {
                break;
            }
            chHandle = comObj->inChHandle[cnt][cnt2];
            /* UDMA Channel enable */
            retVal = Udma_chEnable(chHandle);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(UDMA_SOK != retVal)
            {
                GT_1trace(VhwaMscTrace, GT_ERR,
                    "UDMA channel enable failed for input%d !!\n", cnt);
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
    {
        if(UDMA_SOK != retVal)
        {
            break;
        }
        chHandle = comObj->outChHandle[cnt];
        /* UDMA Channel enable */
        retVal = Udma_chEnable(chHandle);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UDMA_SOK != retVal)
        {
            GT_1trace(VhwaMscTrace, GT_ERR,
                "UDMA channel enable failed for output%d !!\n", cnt);
        }
        /* LDRA_JUSTIFY_END */

    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UDMA_SOK != retVal)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "UDMA channel enable failed!!\n");
        retVal = FVID2_EFAIL;
    }
    else
    /* LDRA_JUSTIFY_END */

    {
        retVal = FVID2_SOK;
    }

    return (retVal);
}

int32_t Vhwa_m2mMscStopCh(const Vhwa_M2mMscCommonObj *comObj)
{
    int32_t         retVal = UDMA_SOK;
    uint32_t        cnt, cnt2;
    Udma_ChHandle   chHandle;
     /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == comObj)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "NULL pointer\n");
        retVal = UDMA_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */


    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        for (cnt2 = 0U; cnt2 < VHWA_M2M_MSC_MAX_IN_CHANNEL; cnt2 ++)
        {
            if(UDMA_SOK != retVal)
            {
                break;
            }
            chHandle = comObj->inChHandle[cnt][cnt2];
            /* UDMA Channel disable */
            retVal = Udma_chDisable(chHandle,
                                    UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
            if(UDMA_SOK != retVal)
            {
                GT_1trace(VhwaMscTrace, GT_ERR,
                    "UDMA channel disable failed for input%d !!\n", cnt);
            }
        }
    }

    for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
    {
        if(UDMA_SOK != retVal)
        {
            break;
        }
        chHandle = comObj->outChHandle[cnt];
        /* UDMA Channel disable */
        retVal = Udma_chDisable(chHandle,
                                UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
        if(UDMA_SOK != retVal)
        {
            GT_1trace(VhwaMscTrace, GT_ERR,
                "UDMA channel disable failed for output%d !!\n", cnt);
        }
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
    if (UDMA_SOK != retVal)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "UDMA channel disable failed!!\n");
    }
    /* LDRA_JUSTIFY_END */


    return retVal;
}

int32_t Vhwa_mscM2mSubmitRing(const Vhwa_M2mMscInstObj *instObj,
                              const Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscCommonObj *comObj,
                              uint32_t compCnt)
{
    int32_t retVal = UDMA_SOK;
    uint32_t cnt;
    Udma_ChHandle chHandle;
     /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == hObj) || (NULL == comObj))

    {
        GT_0trace(VhwaMscTrace, GT_ERR, "NULL pointer\n");
        retVal = UDMA_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

     /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (hObj != NULL) && (comObj != NULL) )
    /* LDRA_JUSTIFY_END */

    {
        for (cnt = 0U; cnt < hObj->numInChUsed; cnt ++)
        {
             /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start> Rationale:
            The test framework and test apps cannot reach this portion
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((UDMA_SOK == retVal) &&
                (((UFALSE == hObj->fcStatus.isFlexConnect) &&
                (UTRUE == hObj->inChPrms[cnt].buffEnable)) ||
                ((UTRUE == hObj->fcStatus.isFlexConnect) &&
                (UTRUE == hObj->fcStatus.inDmaEnable[cnt]))))
            /* LDRA_JUSTIFY_END */

            {
                /* Submit TRPD to ring */
                chHandle = comObj->inChHandle[hObj->instId][cnt];
            #if defined (MCU_PLUS_SDK)
            retVal = Udma_chRingQueueRaw(chHandle,
                            hObj->inChPrms[cnt].trMem[compCnt],
                            1U);
            #else
                retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle),
                                        (uint64_t) hObj->inChPrms[cnt].trMem[compCnt]);
            #endif

            #if defined (MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (UDMA_SOK == retVal)
            /* LDRA_JUSTIFY_END */
            {
                retVal = Udma_chRingRingDbRaw(chHandle, 1U);
            }
            #endif /* SOC_AM62AX */
            }
        }

        for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
        {
            if(UDMA_SOK != retVal)
            {
                break;
            }
            if (((UTRUE == hObj->outChPrms[cnt].buffEnable) &&
                (UFALSE == hObj->fcStatus.isFlexConnect)) ||
                ((UTRUE == hObj->fcStatus.isFlexConnect) &&
                (UTRUE == hObj->fcStatus.outDmaEnable[cnt])))
            {
                /* Submit TRPD to ring */
                chHandle = comObj->outChHandle[cnt];
            #if defined (MCU_PLUS_SDK)
            retVal = Udma_chRingQueueRaw(chHandle,
                        hObj->outChPrms[cnt].trMem[compCnt],
                        1U);
            #else
                retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle),
                            (uint64_t) hObj->outChPrms[cnt].trMem[compCnt]);
            #endif
            #if defined (MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (UDMA_SOK == retVal)
            /* LDRA_JUSTIFY_END */

            {
                retVal = Udma_chRingRingDbRaw(chHandle, 1U);
            }
            #endif /* SOC_AM62AX */
            }
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UDMA_SOK != retVal)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "Ring queue failed\n");
    }
    /* LDRA_JUSTIFY_END */


    return (retVal);
}

int32_t Vhwa_m2mMscAllocUdmaMem(Vhwa_M2mMscHandleObj *hObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t chCnt, compCnt;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));

    for (chCnt = 0U; chCnt < VHWA_M2M_MSC_MAX_IN_CHANNEL; chCnt ++)
    {
        for (compCnt = 0u; compCnt < VHWA_M2M_MSC_MAX_COMP; compCnt ++)
        {
            /* Alloc TR memory for input channel */
            hObj->inChPrms[chCnt].trMem[compCnt] =
                &gMscTxTprdMem[hObj->instId][hObj->idx][chCnt][compCnt][0U];
        }
    }

    for (chCnt = 0u; chCnt < MSC_MAX_OUTPUT; chCnt ++)
    {
        for (compCnt = 0u; compCnt < VHWA_M2M_MSC_MAX_COMP; compCnt ++)
        {
            /* Alloc TR memory for output channel */
            hObj->outChPrms[chCnt].trMem[compCnt] =
                &gMscRxTprdMem[hObj->instId][hObj->idx][chCnt][compCnt][0U];
        }
    }

    return (retVal);
}

void Vhwa_m2mMscSetAddress(const Vhwa_M2mMscHandleObj *hObj,
                           const Fvid2_FrameList *inFrmList,
                           const Fvid2_FrameList *outFrmList)
{
    uint32_t              packetInfoSize, chCnt, compCnt;
    CSL_UdmapTR9         *pTr;

    packetInfoSize = sizeof(CSL_UdmapTR15);

    for (chCnt = 0U; (chCnt < hObj->numInChUsed); chCnt ++)
    {
         /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
        if ((UFALSE == hObj->fcStatus.isFlexConnect) ||
            ((UTRUE == hObj->fcStatus.isFlexConnect) &&
             (UTRUE == hObj->fcStatus.inDmaEnable[chCnt])))
            /* LDRA_JUSTIFY_END */

        {
            for (compCnt = 0u; compCnt < hObj->numIter; compCnt ++)
            {
            /* Set source address for TX channel */
            pTr = (CSL_UdmapTR9 *)(&hObj->inChPrms[chCnt].trMem[compCnt][packetInfoSize]);

            VhwaAl_Cache_Inv(pTr, packetInfoSize, CacheP_TYPE_L1D);
            pTr->addr = (uint64_t)inFrmList->frames[0]->addr[(chCnt*hObj->numIter)+compCnt];
            VhwaAl_Cache_Wb(pTr, packetInfoSize, CacheP_TYPE_L1D);
            }
        }
    }

    for (chCnt = 0; chCnt < MSC_MAX_OUTPUT; chCnt ++)
    {
        if (((UTRUE == hObj->outChPrms[chCnt].buffEnable) &&
             (UFALSE == hObj->fcStatus.isFlexConnect)) ||
            ((UTRUE == hObj->fcStatus.isFlexConnect) &&
             (UTRUE == hObj->fcStatus.outDmaEnable[chCnt])))
        {
            for (compCnt = 0u; compCnt < hObj->numIter; compCnt ++)
            {
                pTr = (CSL_UdmapTR9 *)(&hObj->outChPrms[chCnt].trMem[compCnt][packetInfoSize]);

                VhwaAl_Cache_Inv(pTr, packetInfoSize, CacheP_TYPE_L1D);
                /* Set source address for RX channel */
                pTr->daddr = (uint64_t)outFrmList->frames[chCnt]->addr[compCnt];
                VhwaAl_Cache_Wb(pTr, packetInfoSize, CacheP_TYPE_L1D);
            }
        }
    }
}

int32_t Vhwa_m2mMscPopRings(Vhwa_M2mMscHandleObj *hObj,
                            const Vhwa_M2mMscCommonObj *comObj,
                            uint32_t compCnt)
{
    int32_t retVal = FVID2_SOK;
    uint32_t chCnt;
    uint64_t ringPopVal;
    Udma_ChHandle chHandle;
#if !defined (MCU_PLUS_SDK)
    uint32_t repeatCnt;
    Vhwa_M2mMscInChParams *inChPrms = NULL;
    Vhwa_M2mMscInChParams *inchPrms2 = NULL;
    Vhwa_M2mMscOutChParams *outChPrms = NULL;
    Vhwa_M2mMscOutChParams *outchPrms2 = NULL;
    Vhwa_M2mMscHandleObj *hObj2 = NULL;
#endif
    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));


    /* Dequeue from completion ring for TX channel */
    for (chCnt = 0U; chCnt < hObj->numInChUsed; chCnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((UFALSE == hObj->fcStatus.isFlexConnect) &&
              (UTRUE == hObj->inChPrms[chCnt].buffEnable)) ||
            ((UTRUE == hObj->fcStatus.isFlexConnect) &&
             (UTRUE == hObj->fcStatus.inDmaEnable[chCnt])))
        /* LDRA_JUSTIFY_END */
        {
            chHandle = comObj->inChHandle[hObj->instId][chCnt];
            #if defined (MCU_PLUS_SDK)
                retVal = Udma_chRingDeQueueRaw(chHandle, 1U, &ringPopVal);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(UDMA_SOK != retVal)
            {
                GT_1trace(VhwaMscTrace, GT_ERR,
                    "UDMA In Channel Failed to pop Ring for in ch%d!!\n", chCnt);
                break;
            }
            if (UDMA_SOK == retVal)
            /* LDRA_JUSTIFY_END */
            {
                retVal = Udma_chRingRingRvrDbRaw(chHandle, 1U);
            }

            #else
            repeatCnt = 0u;
            inChPrms = &hObj->inChPrms[chCnt];

            do
            {
                retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle),
                            &ringPopVal);
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(UDMA_SOK == retVal)
                /* LDRA_JUSTIFY_END */
                {
                    break;
                }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
                repeatCnt ++;
            } while (repeatCnt < VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT);
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((repeatCnt < VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal == (uint32_t)inChPrms->trMem[compCnt]))
            /* LDRA_JUSTIFY_END */
            {
                retVal = FVID2_SOK;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((repeatCnt < VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT) &&
                    ((uint32_t)ringPopVal != (uint32_t)inChPrms->trMem))
            {
                uint32_t hcnt = 0U;
                for (hcnt = 0U; hcnt < VHWA_M2M_MSC_MAX_HANDLES; hcnt ++)
                {
                    for(uint32_t icnt = 0U; icnt < VHWA_M2M_MSC_MAX_INST; icnt ++)
                    {
                        hObj2 = Vhwa_m2mMscGetHandleObj(icnt, hcnt);
                        inchPrms2 = &hObj2->inChPrms[chCnt];

                        if (((uint32_t)UTRUE == hObj2->isUsed) && 
                        ((uint32_t)ringPopVal == (uint32_t)inchPrms2->trMem))
                        {
                            retVal = FVID2_SOK;
                            break;
                        }
                    }
                    if (((uint32_t)UTRUE == hObj2->isUsed) && 
                    ((uint32_t)ringPopVal == (uint32_t)inchPrms2->trMem))
                    {
                        retVal = FVID2_SOK;
                        break;
                    }
                }
                if(hcnt >= VHWA_M2M_MSC_MAX_HANDLES)
                {
                    retVal = FVID2_EFAIL;
                    GT_1trace(VhwaMscTrace, GT_ERR, "UDMA Input Ring dequeue mismatch for ch%d!!\n", chCnt);
                }
            }
            else
            {
                if (repeatCnt >= VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT)
                {
                    retVal = FVID2_EFAIL;                    
                    
                    GT_1trace(VhwaMscTrace, GT_ERR, "UDMA Input Ring dequeue failed for ch%d!!\n", chCnt);
                }
            }
            /* LDRA_JUSTIFY_END */
            #endif
        }
    }

    for (chCnt = 0; chCnt < MSC_MAX_OUTPUT; chCnt ++)
    {
            if (((UTRUE == hObj->outChPrms[chCnt].buffEnable) &&
                (UFALSE == hObj->fcStatus.isFlexConnect)) ||
                ((UTRUE == hObj->fcStatus.isFlexConnect) &&
                (UTRUE == hObj->fcStatus.outDmaEnable[chCnt])))
        {
            chHandle = comObj->outChHandle[chCnt];
            #if defined (MCU_PLUS_SDK)
            retVal = Udma_chRingDeQueueRaw(chHandle, 1U, &ringPopVal);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(UDMA_SOK != retVal)
            {
                GT_1trace(VhwaMscTrace, GT_ERR,
                    "UDMA Out channel Failed to pop Ring for in ch%d!!\n", chCnt);
                break;
            }
            if (UDMA_SOK == retVal)
            /* LDRA_JUSTIFY_END */
            {
                retVal = Udma_chRingRingRvrDbRaw(chHandle, 1U);
            }

            #else
            repeatCnt = 0u;
            outChPrms = &hObj->outChPrms[chCnt];

            do
            {
                /* Dequeue from completion ring for RX channel */
                retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle),
                            &ringPopVal);
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(UDMA_SOK == retVal)
                /* LDRA_JUSTIFY_END */
                {
                    break;
                }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
                repeatCnt ++;
            } while (repeatCnt < VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT);
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((repeatCnt < VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal == (uint32_t)outChPrms->trMem[compCnt]))
            /* LDRA_JUSTIFY_END */
            {
                retVal = FVID2_SOK;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((repeatCnt < VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal != (uint32_t)outChPrms->trMem))
            {
                uint32_t hcnt = 0U;
                for (hcnt = 0U; hcnt < VHWA_M2M_MSC_MAX_HANDLES; hcnt ++)
                {
                    for(uint32_t icnt = 0U; icnt < VHWA_M2M_MSC_MAX_INST; icnt ++)
                    {
                        hObj2 = Vhwa_m2mMscGetHandleObj(icnt, hcnt);
                        outchPrms2 = &hObj2->outChPrms[chCnt];

                        if (((uint32_t)UTRUE == hObj2->isUsed) && 
                        ((uint32_t)ringPopVal == (uint32_t)outchPrms2->trMem))
                        {
                            retVal = FVID2_SOK;
                            break;
                        }
                    }
                    if (((uint32_t)UTRUE == hObj2->isUsed) && 
                    ((uint32_t)ringPopVal == (uint32_t)outchPrms2->trMem))
                    {
                        retVal = FVID2_SOK;
                        break;
                    }
                }
                if(hcnt >= VHWA_M2M_MSC_MAX_HANDLES)
                {
                    retVal = FVID2_EFAIL;
                    GT_1trace(VhwaMscTrace, GT_ERR, "UDMA Output Ring dequeue mismatch for ch%d!!\n", chCnt);
                }
            }
            else
            {
                if (repeatCnt >= VHWA_M2M_MSC_MAX_WAIT_LOOP_CNT)
                {
                    retVal = FVID2_EFAIL;

                    GT_1trace(VhwaMscTrace, GT_ERR, "UDMA Output Ring dequeue failed for ch%d!!\n", chCnt);
                }
            }
            /* LDRA_JUSTIFY_END */
            #endif
        }
    }

    return (retVal);
}

/* ========================================================================== */
/*                             Local Functions                                */
/* ========================================================================== */

static void Vhwa_mscSetupPacketInfo(uint8_t *pTrPdMem, uint32_t flowId,
    uint32_t returnRing)
{
    CSL_UdmapCppi5TRPD *pTrpd;
    uint32_t descType = CSL_UDMAP_CPPI5_PD_DESCINFO_DTYPE_VAL_TR;

    /* Check for Null Pointer */
    GT_assert(VhwaMscTrace, (NULL != pTrPdMem));

    pTrpd = (CSL_UdmapCppi5TRPD *) pTrPdMem;
    CSL_udmapCppi5SetDescType(pTrpd, descType);
    CSL_udmapCppi5TrSetReload(pTrpd, UFALSE, 0U);
    CSL_udmapCppi5SetPktLen(pTrpd, descType, VHWA_MSC_UDMA_NUM_TR_DESC);
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


static void Vhwa_mscSetupTxTr(Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscCommonObj *comObj,
                              const Vhwa_M2mMscInChParams *chPrms,
                              uint8_t *pTrPdMem,
                              uint32_t compCnt,
                              uint32_t chCnt)
{
    uint32_t icnt1, icnt2;
    uint32_t widthInBytes, imageHeight;
    int32_t  srcPitch, dstPitch;
    Vhwa_M2mMscParams *mscPrms = NULL;
    CSL_UdmapTR9 *pTr = NULL;

    /* Check for Null Pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));
    GT_assert(VhwaMscTrace, (NULL != chPrms));
    GT_assert(VhwaMscTrace, (NULL != pTrPdMem));

    pTr = (CSL_UdmapTR9 *) &(pTrPdMem[sizeof(CSL_UdmapTR15)]);

    mscPrms = &hObj->mscPrms;

    /* Get image width in bytes */
    widthInBytes = Vhwa_m2mMscCalcHorzSizeInBytes(hObj->cslMscCfg[compCnt].inWidth,
                                            chPrms->ccsf,
                                            mscPrms->inFmt.dataFormat);
    /* Use Calculated height */
    imageHeight = hObj->cslMscCfg[compCnt].inHeight;
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot access this portion. 
    The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
    Effect on this unit: None, as control can never reach this condition.
    <justification end> */
    if((1u == comObj->socInfo.isDualChannel) && (1u == hObj->mscPrms.isEnableSimulProcessing))
    /* LDRA_JUSTIFY_END */

    {
        imageHeight = chPrms->height;
    }
    if((1u == chCnt) && (UTRUE == mscPrms->secChPrms.enable))
    {
        srcPitch = (int32_t)mscPrms->secChPrms.pitch;
    }
    else
    {
        srcPitch = (int32_t)mscPrms->inFmt.pitch[0];
    }

    dstPitch = (int32_t)chPrms->sl2Pitch;

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
    pTr->flags = CSL_FMK(UDMAP_TR_FLAGS_TYPE,
            CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING) |
        CSL_FMK(UDMAP_TR_FLAGS_STATIC, UFALSE) |
        CSL_FMK(UDMAP_TR_FLAGS_EOL, (uint32_t)UFALSE) |
        CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE,
            CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT1_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
            CSL_UDMAP_TR_FLAGS_TRIGGER_LOCAL_EVENT) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
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
    /* Source address will be overwritten before submitting */
    pTr->addr     = 0x0U;

    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

    icnt1 = comObj->sl2Prms.inSl2BuffDepth[hObj->instId][chCnt];
    icnt2 = imageHeight / icnt1;

    /* Increment if total TR transfer rows are less then total image row */
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
    pTr->daddr    = comObj->sl2Prms.inSl2Addr[hObj->instId][chCnt];
}


static void Vhwa_mscSetupRxTr(Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscCommonObj *comObj,
                              const Vhwa_M2mMscOutChParams *chPrms,
                              uint32_t chCnt,
                              uint8_t *pTrPdMem,
                              uint32_t compCnt)
{
    uint32_t icnt1, icnt2;
    uint32_t widthInBytes, imageHeight;
    int32_t  srcPitch, dstPitch;
    Msc_Config *mscCfg = NULL;
    CSL_UdmapTR9 *pTr = NULL;

    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));
    GT_assert(VhwaMscTrace, (NULL != chPrms));
    GT_assert(VhwaMscTrace, (NULL != pTrPdMem));

    mscCfg = &hObj->cslMscCfg[compCnt].mscCfg;


    pTr = (CSL_UdmapTR9 *)&(pTrPdMem[sizeof(CSL_UdmapTR15)]);

    /* Get image width in bytes */
    widthInBytes = Vhwa_m2mMscCalcHorzSizeInBytes(mscCfg->scCfg[chCnt].outWidth,
                                        hObj->mscPrms.outFmt[chCnt].ccsFormat,
                                        hObj->mscPrms.outFmt[chCnt].dataFormat);

    /* Use calculated height */
    imageHeight = mscCfg->scCfg[chCnt].outHeight;
    srcPitch = (int32_t)chPrms->sl2Pitch;
    dstPitch = (int32_t)hObj->mscPrms.outFmt[chCnt].pitch[0];

    icnt1 = comObj->sl2Prms.outSl2BuffDepth[chCnt];
    icnt2 = imageHeight / icnt1;

    /* Increment if total TR transfer rows are less then total image row */
    if ((icnt2 * icnt1) != imageHeight)
    {
        icnt2 ++;
    }

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
    pTr->addr     = comObj->sl2Prms.outSl2Addr[chCnt];

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
    /* Destination address will be overwritten before submitting */
    pTr->daddr    = 0x0U;

}

uint32_t Vhwa_m2mMscCalcHorzSizeInBytes(uint32_t width, uint32_t ccsf,
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
