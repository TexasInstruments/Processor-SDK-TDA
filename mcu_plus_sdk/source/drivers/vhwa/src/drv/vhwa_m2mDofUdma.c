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
 *  \file vhwa_m2mDofUdma.c
 *
 *  \brief Utility APIs for UDMA configuration
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mDofPriv.h"


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
static void vhwaM2mDofSetupPacketInfo(Vhwa_M2mDofChParams *chPrms,
                                      uint32_t complRingNum);

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
static void vhwaM2mDofSetupCurImgTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl);

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
static void vhwaM2mDofSetupRefImgTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl);

/**
 * \brief   Local Function to initialize transfer recoder for Temporeal predictor
 *          TR of the given channel object.
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void vhwaM2mDofSetupTempPreTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl);

/**
 * \brief   Local Function to initialize transfer recoder for pyramidal predictor
 *          TR of the given channel object.
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void vhwaM2mDofSetupPyrPreTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl);

/**
 * \brief   Local Function to initialize transfer recoder for SOF
 *          TR of the given channel object.
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void vhwaM2mDofSetupSofTr(Vhwa_M2mDofHandleObj *hObj,
                                 CSL_UdmapTR9 *pTr,
                                 uint32_t pyrLvl);

/**
 * \brief   Local Function to initialize transfer recoder for Output
 *          TR of the given channel object.
 *
 * \param   hObj                Handle Object
 * \param   chPrms              Channel Parameters, from which TR start
 *                              address, number of TR etc information is used
 * \param   pTR                 Start address of the TR for this channel
 *
 **/
static void vhwaM2mDofSetupOutputTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**
 * \brief Ring memory, one ring for each output channel.
 */
static uint8_t gDofRingMem[VHWA_M2M_DOF_MAX_DMA_CH]
    [VHWA_M2M_DOF_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#if !defined(MCU_PLUS_SDK)
/**
 * \brief Completion Ring memory, one ring for each output channel.
 */
static uint8_t gDofCompRingMem[VHWA_M2M_DOF_MAX_DMA_CH]
    [VHWA_M2M_DOF_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**
 * \brief Completion Ring memory, one ring for each output channel.
 */
static uint8_t gDofTdCompRingMem[VHWA_M2M_DOF_MAX_DMA_CH]
    [VHWA_M2M_DOF_UDMA_RING_MEM_SIZE]
    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
#endif

/**
 * \brief Teardown Completion Ring memory, one ring for each output channel.
 */
static uint8_t gDofTxRxTprdMem[VHWA_M2M_DOF_MAX_HANDLES]
    [DOF_MAX_PYR_LVL_SUPPORTED][VHWA_M2M_DOF_MAX_DMA_CH]
    [VHWA_M2M_DOF_UDMA_TRPD_SIZE]
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
int32_t Vhwa_m2mDofUdmaInit(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofInitParams *initPrms)
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
    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != initPrms));

    utcChId = VHWA_DOF_UTC_CH_START;

    for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_DMA_CH; cnt ++)
    {
        chHndl = &instObj->utcChObj[cnt];
        instObj->utcChHndl[cnt] = chHndl;

        /* Initialize channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);

        chPrms.chNum                = utcChId;
        chPrms.utcId                = VHWA_M2M_DOF_UTC_ID;
        chPrms.fqRingPrms.ringMem   = &gDofRingMem[cnt][0U];
        #if defined(MCU_PLUS_SDK)
        chPrms.cqRingPrms.ringMem   = NULL;
        chPrms.tdCqRingPrms.ringMem = NULL;
        #else
        chPrms.cqRingPrms.ringMem   = &gDofCompRingMem[cnt][0];
        chPrms.tdCqRingPrms.ringMem = &gDofTdCompRingMem[cnt][0U];
        #endif
        chPrms.fqRingPrms.elemCnt   = VHWA_M2M_DOF_UDMA_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = VHWA_M2M_DOF_UDMA_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = VHWA_M2M_DOF_UDMA_RING_ENTRIES;

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
            GT_0trace(VhwaDofTrace, GT_ERR, "UDMA channel open failed!!\n");
        }
        /* LDRA_JUSTIFY_END */

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
            /* Intialize UTC channel Paramenters */
            UdmaChUtcPrms_init(&utcPrms);


            #if defined (MCU_PLUS_SDK)
            utcPrms.chanType = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
            utcPrms.druOwner = (uint8_t)CSL_BCDMA_CHAN_TYPE_REF_TR_RING;
            #else
            utcPrms.chanType = (uint8_t)CSL_UDMAP_CHAN_TYPE_REF_TR_RING;
            utcPrms.busOrderId = 3;
            utcPrms.busPriority = 4;
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
                GT_0trace(VhwaDofTrace, GT_ERR,
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

int32_t Vhwa_m2mDofUdmaDeInit(const Vhwa_M2mDofInstObj *instObj)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    Udma_ChHandle       chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != instObj));

    for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_DMA_CH; cnt ++)
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
            GT_0trace(VhwaDofTrace, GT_ERR,
                        "[Error] UDMA channel close failed!!\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Convert UDMA status to FVID2 status,
       so that the caller of this function always uses FVID2 status only */
    /* LDRA_JUSTIFY_START_DISABLED
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(UDMA_SOK == status)
    {
        status = FVID2_SOK;
    }
    /* LDRA_JUSTIFY_END_DISABLED */
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

int32_t Vhwa_m2mDofStartCh(const Vhwa_M2mDofInstObj *instObj)
{
    int32_t         status = FVID2_EBADARGS;
    uint32_t        cnt;
    Udma_ChHandle   chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != instObj));

    /* By default, all channels are enabled */
    for (cnt = 0u; cnt < VHWA_M2M_DOF_MAX_DMA_CH; cnt ++)
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
            GT_0trace(VhwaDofTrace, GT_ERR,
                "UDMA channel enable failed!!\n");
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

int32_t Vhwa_m2mDofStopCh(const Vhwa_M2mDofInstObj *instObj)
{
    int32_t         status = FVID2_EBADARGS;
    uint32_t        cnt;
    Udma_ChHandle   chHndl;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != instObj));

    /* By default, all channels are disabled, on the last handle close */
    for (cnt = 0u; cnt < VHWA_M2M_DOF_MAX_DMA_CH; cnt ++)
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
            GT_0trace(VhwaDofTrace, GT_ERR,
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

int32_t Vhwa_m2mDofSubmitRing(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj)
{
    int32_t              status = UDMA_SOK;
    uint32_t             chCnt;
    Vhwa_M2mDofChParams *chPrms = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != hObj));

    /* Enable the channel which are required */
    for (chCnt = 0; chCnt < VHWA_M2M_DOF_MAX_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[hObj->curPyrLvl][chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            /* Submit TRPD to ring */
             #if defined (MCU_PLUS_SDK)
            status = Udma_chRingQueueRaw(instObj->utcChHndl[chCnt],
                                            chPrms->trMem,
                                            1U);
            #else
            status = Udma_ringQueueRaw(instObj->fqRingHndl[chCnt],
                (uint64_t) chPrms->trMem);
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
                GT_0trace(VhwaDofTrace, GT_ERR, "Ring queue failed\n");
            }
            /* LDRA_JUSTIFY_END */
            #if defined (MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (UDMA_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                status = Udma_chRingRingDbRaw(instObj->utcChHndl[chCnt], 1U);
            }
            #endif /* SOC_J722S*/
        }
    }

    return (status);
}

#if !defined (MCU_PLUS_SDK)
int32_t Vhwa_m2mDofPopRings(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, uint32_t pyrLvl)
{
    int32_t              status = FVID2_SOK;
    uint32_t             chCnt;
    uint64_t             ringPopVal;
    Vhwa_M2mDofChParams *chPrms = NULL;
#if !defined (MCU_PLUS_SDK)
    uint32_t repeatCnt;
    Vhwa_M2mDofChParams *chPrms2 = NULL;
    Vhwa_M2mDofHandleObj *hObj2 = NULL;
#endif

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != hObj));

    /* Enable the channel which are required */
    for (chCnt = 0u; chCnt < VHWA_M2M_DOF_MAX_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[pyrLvl][chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            #if defined (MCU_PLUS_SDK)
            status = Udma_chRingDeQueueRaw(instObj->utcChHndl[chCnt], 1U, &ringPopVal);
            if(UDMA_SOK != status)
            {
                GT_1trace(VhwaDofTrace, GT_ERR,
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
            } while (repeatCnt < VHWA_DOF_MAX_WAIT_LOOP_CNT);
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((repeatCnt < VHWA_DOF_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal == (uint32_t)chPrms->trMem))
            /* LDRA_JUSTIFY_END */
            {
                status = FVID2_SOK;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The test framework and test apps cannot reach this portion since triggering out of order interrupts and 
                                             UDMA interrupts out of order is not possible from the current test configs.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((repeatCnt < VHWA_DOF_MAX_WAIT_LOOP_CNT) &&
                ((uint32_t)ringPopVal != (uint32_t)chPrms->trMem))
            {
                uint32_t hcnt = 0U;
                for (hcnt = 0U; hcnt < VHWA_M2M_DOF_MAX_HANDLES; hcnt ++)
                {
                    hObj2 = Vhwa_m2mDofGetHandleObj(hcnt);
                    chPrms2 = &hObj2->chPrms[pyrLvl][chCnt];

                    if (((uint32_t)UTRUE == hObj2->isUsed) && 
                    ((uint32_t)ringPopVal == (uint32_t)chPrms2->trMem))
                    {
                        status = FVID2_SOK;
                        break;
                    }
                }
                if(hcnt >= VHWA_M2M_DOF_MAX_HANDLES)
                {
                    status = FVID2_EFAIL;
                    GT_1trace(VhwaDofTrace, GT_ERR, "UDMA Ring dequeue mismatch for ch%d!!\n", chCnt);
                }
            }
            else
            {
                if (repeatCnt >= VHWA_DOF_MAX_WAIT_LOOP_CNT)
                {
                    status = FVID2_EFAIL;

                    GT_1trace(VhwaDofTrace, GT_ERR, "UDMA Ring dequeue failed for ch%d!!\n", chCnt);
                }
            }
            /* LDRA_JUSTIFY_END */
            #endif
        }
    }

    return (status);
}
#endif

int32_t Vhwa_m2mDofAllocUdmaMem(Vhwa_M2mDofHandleObj *hObj)
{
    int32_t     status = FVID2_SOK;
    uint32_t    hIdx;
    uint32_t    chCnt = 0u;
    uint32_t    pyrCnt = 0u;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));

    hIdx = hObj->hIdx;

    for (chCnt = 0u; chCnt < VHWA_M2M_DOF_MAX_DMA_CH; chCnt ++)
    {
        for(pyrCnt = 0; pyrCnt < DOF_MAX_PYR_LVL_SUPPORTED; pyrCnt++)
        {
            hObj->chPrms[pyrCnt][chCnt].trMem =
                                    &gDofTxRxTprdMem[hIdx][pyrCnt][chCnt][0U];
            Fvid2Utils_memset(hObj->chPrms[pyrCnt][chCnt].trMem, 0x0,
                VHWA_M2M_DOF_UDMA_TRPD_SIZE);
        }
    }

    return (status);
}


void Vhwa_m2mDofSetTrDesc(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj)
{
    uint32_t               pyrCnt, chCnt;
    CSL_UdmapTR9          *pTr9;
    uint32_t               packetInfoSize;
    Vhwa_M2mDofChParams   *chPrms;

    packetInfoSize = sizeof(CSL_UdmapTR15);

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != hObj));

    for(pyrCnt = 0; pyrCnt < hObj->dofPrms.tPrmdLvl; pyrCnt++)
    {
        for (chCnt = 0u; chCnt < VHWA_M2M_DOF_MAX_DMA_CH; chCnt ++)
        {
            chPrms = &hObj->chPrms[pyrCnt][chCnt];
            if((uint32_t)UTRUE == chPrms->isEnabled)
            {
                pTr9 = (CSL_UdmapTR9 *)((uint32_t)chPrms->trMem +
                                                    packetInfoSize);

                /* Invalidate cache before changing contents */
                VhwaAl_Cache_Inv(chPrms->trMem, VHWA_M2M_DOF_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

                /* Setup Packet Info for the Tx Channel */
                vhwaM2mDofSetupPacketInfo(chPrms, instObj->complRingNum[chCnt]);

                if((DOF_INPUT_REFERENCE_IMG == chCnt) ||
                   (DOF_INPUT_FOCO_REF_IMG == chCnt))
                {
                    /* Setup Transfer Record */
                    vhwaM2mDofSetupRefImgTr(hObj, pTr9, pyrCnt);
                }
                else if((DOF_INPUT_CURRENT_IMG == chCnt) ||
                        (DOF_INPUT_FOCO_CURR_IMG == chCnt))
                {
                    vhwaM2mDofSetupCurImgTr(hObj, pTr9, pyrCnt);
                }
                else if(DOF_INPUT_TEMPORAL_PRED == chCnt)
                {
                    vhwaM2mDofSetupTempPreTr(hObj, pTr9, pyrCnt);
                }
                else if(DOF_INPUT_PYRAMID_PRED == chCnt)
                {
                    vhwaM2mDofSetupPyrPreTr(hObj, pTr9, pyrCnt);
                }
                else if(DOF_INPUT_SOF == chCnt)
                {
                    vhwaM2mDofSetupSofTr(hObj, pTr9, pyrCnt);
                }
                else
                {
                    /* DOF_OUTPUT == chCnt */
                    vhwaM2mDofSetupOutputTr(hObj, pTr9, pyrCnt);
                }

                /* Write back contents into memory after changing */
                VhwaAl_Cache_Wb(chPrms->trMem, VHWA_M2M_DOF_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);
            }
        }
    }
}

void vhwaM2mDofSetTRAddress(Vhwa_M2mDofHandleObj *hObj,
                            const Fvid2_FrameList *inFrmList,
                            uint64_t outAddr)
{
    uint32_t              packetInfoSize, chCnt;
    CSL_UdmapTR9         *pTr;
    Vhwa_M2mDofChParams   *chPrms;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != inFrmList));

    packetInfoSize = sizeof(CSL_UdmapTR15);

    /* Set address for current pyramid level */
    for (chCnt = 0u; chCnt < VHWA_M2M_DOF_MAX_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[hObj->curPyrLvl][chCnt];
        if((uint32_t)UTRUE == chPrms->isEnabled)
        {
            pTr = (CSL_UdmapTR9 *)(&chPrms->trMem[packetInfoSize]);
            VhwaAl_Cache_Inv(pTr, packetInfoSize, CacheP_TYPE_L1D);

            if(DOF_OUTPUT == chCnt)
            {
                pTr->daddr = outAddr;
            }
            else if(DOF_INPUT_FOCO_REF_IMG == chCnt)
            {
                pTr->addr = inFrmList->frames[DOF_INPUT_REFERENCE_IMG]->addr[0];
            }
            else if(DOF_INPUT_FOCO_CURR_IMG == chCnt)
            {
                pTr->addr = inFrmList->frames[DOF_INPUT_CURRENT_IMG]->addr[0];
            }
            else
            {
                pTr->addr = inFrmList->frames[chCnt]->addr[0];
            }
            VhwaAl_Cache_Wb(pTr, packetInfoSize, CacheP_TYPE_L1D);
        }
    }
}

/* ========================================================================== */
/*                             Local Functions                                */
/* ========================================================================== */

static void vhwaM2mDofSetupPacketInfo(Vhwa_M2mDofChParams *chPrms,
                                      uint32_t complRingNum)
{
    uint32_t             numTr;
    uint32_t             descType = CSL_UDMAP_CPPI5_PD_DESCINFO_DTYPE_VAL_TR;
    uint32_t             packetInfoSize;
    CSL_UdmapCppi5TRPD   *pTrpd = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != chPrms));

    packetInfoSize = sizeof(CSL_UdmapTR15);
    pTrpd = (CSL_UdmapCppi5TRPD *) chPrms->trMem;
    numTr = 1u;

    /* Calculate start address for the TR response,
     * This is dependent on the number of TRs
     * Added 1 here for the TR header. */
    chPrms->trRespMem = (uint8_t *)((uint32_t)chPrms->trMem +
                        (packetInfoSize * (numTr + 1U)));

    CSL_udmapCppi5SetDescType(pTrpd, descType);
    CSL_udmapCppi5TrSetReload(pTrpd, UFALSE, 0U);
    CSL_udmapCppi5SetPktLen(pTrpd, descType, numTr);
    /* Flow ID and Packet ID */
    CSL_udmapCppi5SetIds(pTrpd, descType, 0U, VHWA_DOF_FLOW_ID);
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

static void vhwaM2mDofSetupCurImgTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl)
{
    uint16_t icnt1, icnt2;
    uint32_t ddim1;
    uint32_t widthInBytes, imageHeight;
    uint32_t  sl2BufHeight;
    Dof_Config *dofCfg;
    Vhwa_M2mDofPrms *dofPrms;
    Fvid2_Format  *fmt;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != pTr));

    dofCfg = &hObj->dofCfg[pyrLvl];
    dofPrms = &hObj->dofPrms;
    fmt = &dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG];

    /* Calculate the buffer height in SL2 */
    sl2BufHeight = 6u+8u+2u+hObj->sl2Prms.sl2NumLines[DOF_INPUT_CURRENT_IMG];

    /* Get image width in bytes */
    widthInBytes = Vhwa_calcHorzSizeInBytes(dofCfg->width,
       fmt->ccsFormat);

    imageHeight = dofCfg->height;

    icnt1 = 2u;
    icnt2 = (uint16_t)(imageHeight / 2u);

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
    if(hObj->isFocoUsed[pyrLvl] == UTRUE)
    {
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
        pTr->dim1     = (int32_t)fmt->pitch[0U];
        pTr->dim2     = 0;
        pTr->dim3     = 0;

        pTr->dicnt0   = (uint16_t)widthInBytes;
        pTr->dicnt1   = DOF_FOCO_SL2_DEPTH;
        pTr->dicnt2   = (uint16_t)Vhwa_ceil(imageHeight,pTr->dicnt1);
        pTr->dicnt3   = 1U;

        ddim1 = (widthInBytes + 63u) & 0xFFFFFFC0u;

        pTr->ddim1    = (int32_t)ddim1;
        pTr->ddim2    = 0;
        pTr->ddim3    = 0;
        pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_FOCO_CURR_IMG];
    }
    else
    {
        pTr->flags =
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
                CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT2_DEC) |
            /* CmdId: This will come back in TR response */
            CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x25U) |
            CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

        pTr->icnt0    = (uint16_t)widthInBytes;
        pTr->icnt1    = icnt1;
        pTr->icnt2    = icnt2;
        pTr->icnt3    = 1U;
        pTr->dim1     = (int32_t)fmt->pitch[0U];
        pTr->dim2     = pTr->dim1 * (int32_t)pTr->icnt1;
        pTr->dim3     = 0;

        pTr->dicnt0   = (uint16_t)widthInBytes;
        pTr->dicnt1   = pTr->icnt1;

        pTr->dicnt2   = (uint16_t)(sl2BufHeight/pTr->dicnt1);

        pTr->dicnt3   = (uint16_t)Vhwa_ceil(imageHeight,
                                ((uint32_t)pTr->dicnt2 * (uint32_t)pTr->dicnt1));

        pTr->ddim1    = (int32_t)hObj->sl2Prms.sl2RefCurPitch;
        pTr->ddim2    = pTr->ddim1 * (int32_t)pTr->dicnt1;
        pTr->ddim3    = 0;
        pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_CURRENT_IMG];
    }

    pTr->addr     = 0x0U;
    pTr->fmtflags = 0x0U |
            CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
                CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
            CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
                CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
            CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

}

static void vhwaM2mDofSetupRefImgTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl)
{
    uint16_t icnt1, icnt2;
    uint32_t ddim1;
    uint32_t widthInBytes, imageHeight;
    uint32_t top, bottom, current, sl2BufHeight;
    Dof_Config *dofCfg;
    Vhwa_M2mDofPrms *dofPrms;
    Fvid2_Format  *fmt;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != pTr));

    dofCfg = &hObj->dofCfg[pyrLvl];
    dofPrms = &hObj->dofPrms;
    fmt = &dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_REFERENCE_IMG];

    /* Calculate the buffer height in SL2 */
    top      = dofCfg->topSearchRange + 6u;
    bottom   = dofCfg->bottomSearchRange + 6u + 2u + 1u;
    current  = 2u;
    if((top % 2u) != 0u)
    {
        top++;
    }
    if((bottom % 2u) != 0u)
    {
        bottom++;
    }

    sl2BufHeight = top + bottom + current +
                   hObj->sl2Prms.sl2NumLines[DOF_INPUT_REFERENCE_IMG];

    widthInBytes = Vhwa_calcHorzSizeInBytes(dofCfg->width,
       fmt->ccsFormat);

    imageHeight = dofCfg->height;

    icnt1 = 2u;
    icnt2 = (uint16_t)(imageHeight / 2u);

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
    if(hObj->isFocoUsed[pyrLvl] == UTRUE)
    {
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
            CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
                CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
            CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
                CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
            /* CmdId: This will come back in TR response */
            CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x26U) |
            CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

        pTr->icnt0    = (uint16_t)widthInBytes;
        pTr->icnt1    = (uint16_t)imageHeight;
        pTr->icnt2    = 1U;
        pTr->icnt3    = 1U;
        pTr->dim1     = (int32_t)fmt->pitch[0U];
        pTr->dim2     = 0;
        pTr->dim3     = 0;

        pTr->dicnt0   = (uint16_t)widthInBytes;
        pTr->dicnt1   = DOF_FOCO_SL2_DEPTH;
        pTr->dicnt2   = (uint16_t)Vhwa_ceil(imageHeight,pTr->dicnt1);

        /* Logic to validate if the image height is less wrt search range */
        pTr->dicnt3   = 1U;

        ddim1 = (widthInBytes + 63u) & 0xFFFFFFC0u;

        pTr->ddim1    = (int32_t)ddim1;
        pTr->ddim2    = 0;
        pTr->ddim3    = 0;
        pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_FOCO_REF_IMG];
    }
    else
    {
        pTr->flags =
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
                CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT2_DEC) |
            /* CmdId: This will come back in TR response */
            CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x26U) |
            CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
            CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

        pTr->icnt0    = (uint16_t)widthInBytes;
        pTr->icnt1    = icnt1;
        pTr->icnt2    = icnt2;
        pTr->icnt3    = 1U;
        pTr->dim1     = (int32_t)fmt->pitch[0U];
        pTr->dim2     = pTr->dim1 * (int32_t)pTr->icnt1;
        pTr->dim3     = 0;

        pTr->dicnt0   = (uint16_t)widthInBytes;
        pTr->dicnt1   = pTr->icnt1;
        pTr->dicnt2   = (uint16_t)(sl2BufHeight/pTr->icnt1);

        /* Logic to validate if the image height is less wrt search range */
        pTr->dicnt3   = (uint16_t)Vhwa_ceil(imageHeight,
                                (uint32_t)pTr->dicnt2 * (uint32_t)pTr->dicnt1);

        pTr->ddim1    = (int32_t)hObj->sl2Prms.sl2RefCurPitch;
        pTr->ddim2    = pTr->ddim1 * (int32_t)pTr->dicnt1;
        pTr->ddim3    = 0;
        pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_REFERENCE_IMG];
    }
    pTr->addr     = 0x0U;
    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);
}

static void vhwaM2mDofSetupTempPreTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl)
{
    uint16_t icnt1;
    uint32_t widthInBytes, imageHeight;
    uint32_t  sl2Pitch;
    Dof_Config *dofCfg;
    Vhwa_M2mDofPrms *dofPrms;
    Fvid2_Format  *fmt;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != pTr));

    dofCfg = &hObj->dofCfg[pyrLvl];
    dofPrms = &hObj->dofPrms;
    fmt = &dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_TEMPORAL_PRED];

    widthInBytes = dofCfg->width * 4U;
    imageHeight = dofCfg->height;

    sl2Pitch = (widthInBytes + 63u) & 0xFFFFFFC0u;

    icnt1 = (uint16_t)(imageHeight / 2u);

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
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
        /* CmdId: This will come back in TR response */
        CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x27U) |
        CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

    pTr->icnt0    = (uint16_t)widthInBytes;
    pTr->icnt1    = icnt1;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = (int32_t)fmt->pitch[0U] * (int32_t)2;
    /* Alternate lines are fetched */

    pTr->dim2     = 0;
    pTr->dim3     = 0;
    pTr->addr     = 0x0U;
    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

    pTr->dicnt0   = (uint16_t)widthInBytes;
    pTr->dicnt1   = DOF_TEMPORAL_PRED_SL2_DEPTH;
    pTr->dicnt2   = (uint16_t)Vhwa_ceil(imageHeight,
                                        (DOF_TEMPORAL_PRED_SL2_DEPTH * 2u));
    pTr->dicnt3   = 1;

    pTr->ddim1    = (int32_t)sl2Pitch;
    pTr->ddim2    = 0;
    pTr->ddim3    = 0;

    pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_TEMPORAL_PRED];

}

static void vhwaM2mDofSetupPyrPreTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl)
{
    uint16_t icnt1;
    uint32_t widthInBytes, imageHeight;
    uint32_t  sl2Pitch;
    Dof_Config *dofCfg;
    Vhwa_M2mDofPrms *dofPrms;
    Fvid2_Format  *fmt;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != pTr));

    dofCfg = &hObj->dofCfg[pyrLvl];
    dofPrms = &hObj->dofPrms;
    fmt = &dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_PYRAMID_PRED];

    widthInBytes = dofCfg->width;

    sl2Pitch = (widthInBytes + 63u) & 0xFFFFFFC0u;

    imageHeight = dofCfg->height;

    icnt1 = (uint16_t)(imageHeight / 2U); /* previous frame height is half of current frame */

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
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC) |
        /* CmdId: This will come back in TR response */
        CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x28U) |
        CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

    pTr->icnt0    = (uint16_t)widthInBytes;
    pTr->icnt1    = icnt1;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = (int32_t)fmt->pitch[0U];
    pTr->dim2     = 0;
    pTr->dim3     = 0;
    pTr->addr     = 0x0U;
    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_SRC_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);

    pTr->dicnt0   = (uint16_t)widthInBytes;
    pTr->dicnt1   = DOF_PYRAM_PRED_SL2_DEPTH;
    pTr->dicnt2   = (uint16_t)Vhwa_ceil(icnt1, DOF_PYRAM_PRED_SL2_DEPTH);

    pTr->dicnt3   = 1;
    pTr->ddim1    = (int32_t)sl2Pitch;
    pTr->ddim2    = 0;
    pTr->ddim3    = 0;
    pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_PYRAMID_PRED];

}

static void vhwaM2mDofSetupSofTr(Vhwa_M2mDofHandleObj *hObj,
                                 CSL_UdmapTR9 *pTr,
                                 uint32_t pyrLvl)
{
    uint32_t widthInBytes, imageHeight;
    uint32_t  sl2Pitch;
    Dof_Config *dofCfg;
    Vhwa_M2mDofPrms *dofPrms;
    Fvid2_Format  *fmt;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != pTr));

    dofCfg = &hObj->dofCfg[pyrLvl];
    dofPrms = &hObj->dofPrms;
    fmt = &dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_SOF];

    widthInBytes = Vhwa_ceil(dofCfg->width, 8);

    sl2Pitch = (widthInBytes + 63u) & 0xFFFFFFC0u;

    imageHeight = dofCfg->height;

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
        CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x27U) |
        CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

    pTr->icnt0    = (uint16_t)widthInBytes;
    pTr->icnt1    = 2u;
    pTr->icnt2    = (uint16_t)(imageHeight/2u);
    pTr->icnt3    = 1;
    pTr->dim1     = (int32_t)fmt->pitch[0U];
    pTr->dim2     = pTr->dim1 * (int32_t)pTr->icnt1;
    pTr->dim3     = 0;
    pTr->addr     = 0x0U;
    pTr->fmtflags = 0x00000000U;

    pTr->dicnt0   = (uint16_t)widthInBytes;
    pTr->dicnt1   = 2;
    pTr->dicnt2   = (uint16_t)(DOF_SOF_SL2_DEPTH/2u);
    pTr->dicnt3   = (uint16_t)Vhwa_ceil(imageHeight,
                                (uint32_t)pTr->dicnt2 * (uint32_t)pTr->dicnt1);

    pTr->ddim1    = (int32_t)sl2Pitch;
    pTr->ddim2    = pTr->ddim1 * (int32_t)pTr->dicnt1;
    pTr->ddim3    = 0;
    pTr->daddr    = hObj->sl2Prms.sl2Addr[DOF_INPUT_SOF];

}

static void vhwaM2mDofSetupOutputTr(Vhwa_M2mDofHandleObj *hObj,
                                    CSL_UdmapTR9 *pTr,
                                    uint32_t pyrLvl)
{
    uint16_t icnt1;
    uint32_t widthInBytes, imageHeight;
    uint32_t  sl2Pitch;
    Dof_Config *dofCfg;
    Vhwa_M2mDofPrms *dofPrms;
    Fvid2_Format  *fmt;

    /* Check for Null pointer */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != pTr));

    dofCfg = &hObj->dofCfg[pyrLvl];
    dofPrms = &hObj->dofPrms;
    fmt = &dofPrms->inOutImgFmt[pyrLvl][DOF_OUTPUT];

    if(dofCfg->lkConfidanceScore == UTRUE)
    {
        if(dofCfg->enableSof == UTRUE)
        {
            widthInBytes = 4U * dofCfg->maxMVsInSof;
        }
        else
        {
            widthInBytes = dofCfg->width * 4U;
        }
    }
    else
    {
        if(dofCfg->enableSof == UTRUE)
        {
            widthInBytes = 2u * dofCfg->maxMVsInSof;
        }
        else
        {
            widthInBytes = dofCfg->width * 2U;
        }
    }

    sl2Pitch = (widthInBytes + 63u) & 0xFFFFFFC0u;

    imageHeight = dofCfg->height;

    icnt1 = (uint16_t)hObj->sl2Prms.sl2NumLines[DOF_OUTPUT];

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
            CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT2_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
            CSL_UDMAP_TR_FLAGS_TRIGGER_LOCAL_EVENT) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT2_DEC) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
        CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT2_DEC) |
        /* CmdId: This will come back in TR response */
        CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x30U) |
        CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
        CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

    /* Update Output TR in case of SOF */

    pTr->icnt0    = (uint16_t)widthInBytes;
    pTr->icnt1    = 2u;
    pTr->icnt2    = icnt1/2u;
    if(dofCfg->enableSof == UTRUE)
    {
        pTr->icnt3 = (uint16_t)Vhwa_ceil(dofPrms->flowVectorHeight,
                                (uint32_t)pTr->icnt1 * (uint32_t)pTr->icnt2);
    }
    else
    {
        pTr->icnt3 = (uint16_t)Vhwa_ceil(imageHeight,
                                 (uint32_t)pTr->icnt1 * (uint32_t)pTr->icnt2);
    }
    pTr->dim1     = (int32_t)sl2Pitch;
    pTr->dim2     = pTr->dim1 * (int32_t)pTr->icnt1;
    pTr->dim3     = 0;

    pTr->dicnt0   = (uint16_t)widthInBytes;
    pTr->dicnt1   = 2u;
    if(dofCfg->enableSof == UTRUE)
    {
        pTr->dicnt2   = (uint16_t)(dofPrms->flowVectorHeight/2u);
    }
    else
    {
        pTr->dicnt2   = (uint16_t)(imageHeight/2u);
    }
    pTr->dicnt3   = 1u;
    pTr->ddim1    = (int32_t)fmt->pitch[0];
    pTr->ddim2    = pTr->ddim1 * (int32_t)pTr->dicnt1;
    pTr->ddim3    = 0;

    pTr->addr     = hObj->sl2Prms.sl2Addr[DOF_OUTPUT];
    pTr->fmtflags = 0x0U |
        CSL_FMK(UDMAP_TR_FMTFLAGS_AMODE,
            CSL_UDMAP_TR_FMTFLAGS_AMODE_LINEAR)                         |
        CSL_FMK(UDMAP_TR_FMTFLAGS_DIR,
            CSL_UDMAP_TR_FMTFLAGS_DIR_DST_USES_AMODE)                   |
        CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);;

    pTr->daddr    = 0x0U;

}

