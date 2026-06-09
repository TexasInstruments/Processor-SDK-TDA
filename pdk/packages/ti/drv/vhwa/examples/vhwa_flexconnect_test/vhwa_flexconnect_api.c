/*
*  Copyright (c) Texas Instruments Incorporated 2019
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
*  \file vhwa_fc_api.c
*
*  \brief VHWA VISS APIs
*/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <include/vhwa_m2mFlexConnect.h>

#include <examples/include/vhwa_viss_test_api.h>
#include <examples/include/vhwa_msc_api.h>
#include <examples/include/vhwa_flexconnect_api.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Offset for Each Input buffer in DDR */
#define APP_MSC_IN_BUFF_OFFSET      (0x800000u)
/* Offset for Each Output buffer in DDR */
#define APP_MSC_OUT_BUFF_OFFSET     (0x800000u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

int32_t  gSpCoeffSets[][MSC_MAX_SP_COEFF_SET][MSC_MAX_TAP] = {
    {
        {16, 48, 128, 48, 16},
        {32, 32, 128, 32, 32}
    },
};

int32_t  gMpCoeffSets[][MSC_MAX_MP_COEFF_SET][MSC_MAX_TAP * 32U] = {
    #include "examples/vhwa_msc_test/coeff.txt"
};

Msc_Coeff gCoefTbl[] = {
    {
        {gSpCoeffSets[0][0], gSpCoeffSets[0][1]},
        {gMpCoeffSets[0][0], gMpCoeffSets[0][1],
         gMpCoeffSets[0][2], gMpCoeffSets[0][3]}
    }
};

uint32_t gPerfNum[10];
uint32_t gPerfCnt = 0u;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
int32_t AppFc_CompareCrc(AppFc_TestParams *tPrms, uint32_t hIdx);
#endif

static int32_t AppFc_MscAllocBuffers(App_MscTestCfg *tConfig, uint32_t hndlIdx,
    uint64_t srcBuf, uint32_t *srcBufSize, uint32_t srcOffset,
    uint64_t dstBuf, uint32_t *dstBufSize, uint32_t dstOffset, uint32_t threadId);

static int32_t AppFc_VissAllocBuffers(AppViss_TestConfig *tConfig, uint32_t hidx,
               uint64_t srcBuf, uint32_t *inFrameSize,
               uint64_t dstBuf, uint32_t *outFrameSize);

static int32_t AppFc_VissSetAllConfig(AppViss_TestConfig *tCfg, uint32_t hidx);

static int32_t AppFc_MscSetCoeff(Msc_Coeff * coefTbl, uint32_t hndlIdx,
                                    uint32_t threadId);

static int32_t AppFc_MscSetParams(App_MscTestCfg *tCfg, uint32_t hndlIdx,
                                uint32_t threadId);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

AppFc_TestObject  gAppFcTestObject[VHWA_FC_DRV_MAX_HANDLES];
#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
AppCrc_hdlPrms gFcCrcChHandle;
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t AppFc_Init(Udma_DrvHandle udmaDrvHndl)
{
    int32_t                     status;
    Vhwa_M2mFcSl2AllocPrms      sl2AllocPrms;
    Vhwa_M2mFcInitPrms          fcInitPrms;
    Vhwa_M2mMscInitParams       mscInitPrms;
    Vhwa_M2mVissInitParams      vissInitPrms;

    Vhwa_m2mMscInitParamsInit(&mscInitPrms);

    mscInitPrms.drvHandle = udmaDrvHndl;

    status = Vhwa_m2mMscInit(&mscInitPrms);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" FC_TEST_APP: MSC Init Failed\n");
    }
    if(FVID2_SOK == status)
    {
        /* Initialize VISS Init parameters */
        Vhwa_m2mVissInitParamsInit(&vissInitPrms);

        /* Set UDMA driver handle */
        vissInitPrms.udmaDrvHndl = udmaDrvHndl;

        /* enable config through UDMA */
        vissInitPrms.configThroughUdmaFlag = UFALSE;

        status = Vhwa_m2mVissInit(&vissInitPrms);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" FC_TEST_APP: VISS Init Failed\n");
        }
    }

    if(FVID2_SOK == status)
    {
        /* Initialize Fc Init parameters */
        Vhwa_m2mFcInitPrmsInit(&fcInitPrms);

        /* Set UDMA driver handle */
        fcInitPrms.udmaDrvHndl = udmaDrvHndl;

        status = Vhwa_m2mFcInit(&fcInitPrms);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" FC_TEST_APP: FC Init Failed\n");
        }
        else
        {
            Vhwa_m2mFcDrvSl2AllocPrmsInit(&sl2AllocPrms);

            status = Vhwa_m2mFcAllocSl2(&sl2AllocPrms);
            if (FVID2_SOK != status)
            {
                Vhwa_print(" FC_TEST_APP: SL2 Alloc Failed !!!\n");
            }
        }
        Fvid2Utils_memset(gAppFcTestObject, 0x0,
                sizeof(AppFc_TestObject)*VHWA_FC_DRV_MAX_HANDLES);
    }

    return (status);
}

void AppFc_deInit(Udma_DrvHandle udmaDrvHndl)
{
    int32_t         status;

    Vhwa_m2mFcDeInit();

    status = Udma_deinit(udmaDrvHndl);
    if(UDMA_SOK != status)
    {
        Vhwa_print("[Error] UDMA deinit failed!!\n");
    }

    Fvid2_deInit(NULL);
}


int32_t AppFcFrameComplCb(Fvid2_Handle handle, void *appData)
{
    AppFc_TestObject *tObj = (AppFc_TestObject *)appData;

    if (NULL != tObj)
    {
        VhwaAl_SemaphoreP_post(tObj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

int32_t AppFc_Create(AppFc_TestParams *tPrms, uint32_t hidx)
{
    int32_t              status;
    uint32_t             instId =  VHWA_M2M_VPAC_0_FC_DRV_INST_ID_0;
    AppFc_TestObject   *tObj = &gAppFcTestObject[hidx];

     #if defined(MCU_PLUS_SDK)
     #if defined (VHWA_FREERTOS_SUPPORT)
     status = SemaphoreP_constructBinary(&tObj->waitForProcessCmpl, 0);
     #else
     status = SemaphoreP_constructMutex(&tObj->waitForProcessCmpl);
     #endif
    #else    
    SemaphoreP_Params   params;

    SemaphoreP_Params_init(&params);
    params.mode = SemaphoreP_Mode_BINARY;
    tObj->waitForProcessCmpl = SemaphoreP_create(0U, &params);
    if (NULL != tObj->waitForProcessCmpl)
    #endif
    #if defined(MCU_PLUS_SDK)
    if (SYSTEMP_SUCCESS == status)
    #endif
    {
        tObj->cbPrms.cbFxn   = AppFcFrameComplCb;
        tObj->cbPrms.appData = tObj;

        #if defined VHWA_PDK

        if(tPrms->isPerformanceTest)
        {
            tObj->createArgs.getTimeStamp = Vhwa_getTime;
        }
        #endif

#if defined VHWA_PDK
#if (VPAC_TEST_INSTANCE == 0)
        instId = VHWA_M2M_VPAC_0_FC_DRV_INST_ID_0;
#endif
#if (VPAC_TEST_INSTANCE == 1)
        instId = VHWA_M2M_VPAC_1_FC_DRV_INST_ID_0;
#endif
#endif

        tObj->handle = Fvid2_create(FVID2_VHWA_M2M_FC_DRV_ID,
            instId, (void *)&tObj->createArgs,
            NULL, &tObj->cbPrms);
    }
    else
    {
        Vhwa_print (" Could not Create Semaphore %d \n", hidx);
    }

    if (NULL != tObj->handle)
    {
        status = FVID2_SOK;
    }
    else
    {
        Vhwa_print (" Could not Create Handlle%d \n", hidx);
        status = FVID2_EFAIL;
    }

    return (status);
}


void AppFc_Delete(AppFc_TestParams *tPrms, uint32_t hidx)
{
    int32_t                 status;
    AppFc_TestObject   *tObj = &gAppFcTestObject[hidx];

    /* Set Flex-connect path */
    status = Fvid2_control(tObj->handle, IOCTL_VHWA_FC_DELETE_GRAPH,
                            NULL, NULL);
    if(FVID2_SOK != status)
    {
        Vhwa_print (" Failed to Delete Graph \n");
    }

    if (NULL != tObj->handle)
    {
        Fvid2_delete(tObj->handle, NULL);
        tObj->handle = NULL;
    }

    #if defined VHWA_PDK
    if (NULL != tObj->waitForProcessCmpl)
    {
        SemaphoreP_delete(tObj->waitForProcessCmpl);
        tObj->waitForProcessCmpl = NULL;
    }
    #else
    SemaphoreP_destruct(&tObj->waitForProcessCmpl);
    #endif
}

int32_t AppFc_SetAllConfig(AppFc_TestParams *tPrms, uint32_t hidx)
{
    int32_t                 status;
    AppFc_TestObject     *tObj = &gAppFcTestObject[hidx];

    /* Set Flex-connect path */
    status = Fvid2_control(tObj->handle, IOCTL_VHWA_FC_SET_GRAPH,
                            tPrms->testCfg[hidx].fcPathInfo, NULL);


    /* Configure Fc */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(tObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }

    /* Set HWA config */
    if(tPrms->isVissEnabled == UTRUE && FVID2_SOK == status)
    {
        status = AppFc_VissSetAllConfig(tPrms->testCfg[hidx].vissTestCfg, hidx);
    }

    if(tPrms->isMsc0Enabled == UTRUE)
    {
        AppFc_MscSetCoeff(&gCoefTbl[0], hidx, VPAC_MSC_INST_ID_0);
    }
    else if(tPrms->isMsc1Enabled == UTRUE)
    {
        AppFc_MscSetCoeff(&gCoefTbl[0], hidx, VPAC_MSC_INST_ID_1);
    }

    if(tPrms->isMsc0Enabled == UTRUE && FVID2_SOK == status)
    {
        status = AppFc_MscSetParams(tPrms->testCfg[hidx].msc0TestCfg, hidx,
                                        VPAC_MSC_INST_ID_0);
    }

    if(tPrms->isMsc1Enabled == UTRUE && FVID2_SOK == status)
    {
        status = AppFc_MscSetParams(tPrms->testCfg[hidx].msc1TestCfg, hidx,
                                        VPAC_MSC_INST_ID_1);
    }

    /* Configure Fc */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(tObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }

    return (status);
}

int32_t AppFc_AllocBuffers(AppFc_TestParams *tPrms, uint32_t hidx,
    uint64_t srcBuf, uint32_t *inFrameSize,
    uint64_t dstBuf, uint32_t *outFrameSize)
{
    AppViss_TestConfig *vissConfig = NULL;
    uint32_t inFrmSize = 0;
    uint32_t outFrmSize = 0;

    if(tPrms->isVissEnabled == UTRUE)
    {
        vissConfig = tPrms->testCfg[hidx].vissTestCfg;
        AppFc_VissAllocBuffers(vissConfig, hidx, srcBuf, &inFrmSize,
                            dstBuf, &outFrmSize);

        srcBuf += inFrmSize;
        dstBuf += outFrmSize;
        inFrmSize = 0;
        outFrmSize = 0;
    }
    if(tPrms->isMsc0Enabled == UTRUE)
    {
        AppFc_MscAllocBuffers(tPrms->testCfg[hidx].msc0TestCfg, hidx,
                        srcBuf, &inFrmSize, APP_MSC_IN_BUFF_OFFSET,
                        dstBuf, &outFrmSize, APP_MSC_OUT_BUFF_OFFSET,
                        VPAC_MSC_INST_ID_0);

        srcBuf += inFrmSize;
        dstBuf += outFrmSize;
        inFrmSize = 0;
        outFrmSize = 0;
    }

    if(tPrms->isMsc1Enabled == UTRUE)
    {
        AppFc_MscAllocBuffers(tPrms->testCfg[hidx].msc1TestCfg, hidx,
                        srcBuf, &inFrmSize, APP_MSC_IN_BUFF_OFFSET,
                        dstBuf, &outFrmSize, APP_MSC_OUT_BUFF_OFFSET,
                        VPAC_MSC_INST_ID_1);

        srcBuf += inFrmSize;
        dstBuf += outFrmSize;
        inFrmSize = 0;
        outFrmSize = 0;
    }

    return (FVID2_SOK);
}

static int32_t AppFc_VissAllocBuffers(AppViss_TestConfig *tConfig, uint32_t hidx,
    uint64_t srcBuf, uint32_t *inFrameSize,
    uint64_t dstBuf, uint32_t *outFrameSize)
{
    uint32_t                cnt;
    uint32_t                numIns;
    uint32_t                chromaOffset = 0;
    uint32_t                frameEnd = 0;
    AppViss_Cfg            *vissCfg = NULL;
    Vhwa_M2mVissParams     *vissPrms = NULL;
    Fvid2_Format           *outFmt = NULL;
    Fvid2_Format           *inFmt = NULL;
    AppViss_TestObject     *tObj = &(gAppFcTestObject[hidx].vissTestObj);

    vissCfg = tConfig->vissCfg;
    vissPrms = &tConfig->vissPrms;

    if (VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT == vissPrms->inputMode)
    {
        numIns = 1;
    }
    else if (VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE == vissPrms->inputMode)
    {
        numIns = 2;
    }
    else
    {
        numIns = 3;
    }

    inFmt    = &vissPrms->inFmt;
    frameEnd = inFmt->pitch[0] * inFmt->height;

    #ifdef VHWA_RUN_FROM_MSMC
    frameEnd = 16*1024 + 16;
    #endif

    for (cnt = 0u; cnt < numIns; cnt ++)
    {
        tObj->inFrm[cnt].addr[0U] = srcBuf;

        /* Move Buffer Index */
        srcBuf += frameEnd;
        *inFrameSize += frameEnd;

        Vhwa_print (" VISS: Input%d: 0x%llx\n", cnt,
            tObj->inFrm[cnt].addr[0U]);
    }

    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
    {
        if (UFALSE == vissPrms->outPrms[cnt].enable)
        {
            continue;
        }

        outFmt = &vissPrms->outPrms[cnt].fmt;

        chromaOffset = 0u;
        switch (outFmt->dataFormat)
        {
            case FVID2_DF_YUV422I_UYVY:
            case FVID2_DF_YUV422I_YUYV:
                frameEnd = outFmt->pitch[0] * outFmt->height;
                break;
            case FVID2_DF_RGB24_888_PLANAR:
                chromaOffset = outFmt->pitch[0] * outFmt->height;
                frameEnd = chromaOffset * 3;
                break;
            case FVID2_DF_GREY:
            case FVID2_DF_SATURATION:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                        frameEnd = (outFmt->pitch[0] * outFmt->height);
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_GREY/SATUR : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;

            case FVID2_DF_YUV420SP_UV:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = outFmt->pitch[0] * outFmt->height;
                        frameEnd = ((outFmt->pitch[0] * outFmt->height) * 3) / 2;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_YUV420SP_UV : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            case FVID2_DF_YUV422SP_UV:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = outFmt->pitch[0] * outFmt->height;
                        frameEnd = outFmt->pitch[0] * outFmt->height * 2u;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_YUV422SP_UV : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            case FVID2_DF_RAW:
                if (VHWA_M2M_VISS_OUT_H3A_IDX == cnt)
                {
                    uint32_t wins;

                    wins = (vissCfg->h3aCfg->aewbCfg.winCfg.horzCount *
                        vissCfg->h3aCfg->aewbCfg.winCfg.vertCount);

                    dstBuf += (wins * 32) + ((wins/8) *16);
                    *outFrameSize += (wins * 32) + ((wins/8) *16);
                    if (wins%8 != 0)
                    {
                        dstBuf += 16;
                        *outFrameSize += 16;
                    }
                }
                break;
            case FVID2_DF_LUMA_ONLY:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = 0u;
                        frameEnd = outFmt->pitch[0] * outFmt->height;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_LUMA_ONLY : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            
            case FVID2_DF_CHROMA_ONLY:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = 0u;
                        frameEnd = outFmt->pitch[0] * outFmt->height;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_CHROMA_ONLY : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            default:
                Vhwa_print("Error : Unsupported format 0x%x \n",
                    outFmt->dataFormat);
                return FVID2_EFAIL;
        }

        tObj->outFrm[cnt].addr[0U] = dstBuf;
        tObj->outFrm[cnt].addr[1U] = (dstBuf + chromaOffset);
        tObj->outFrm[cnt].addr[2U] = (dstBuf + 2*chromaOffset);

        /* Move Buffer Index */
        dstBuf += frameEnd;
        *outFrameSize += frameEnd;

        Vhwa_print(" VISS: Output%d:", cnt);
        Vhwa_print(" 0x%x 0x%x 0x%x\n",
            (uint32_t)tObj->outFrm[cnt].addr[0U],
            (uint32_t)tObj->outFrm[cnt].addr[1U],
            (uint32_t)tObj->outFrm[cnt].addr[2U]);
    }

    return (FVID2_SOK);
}


static int32_t AppFc_MscAllocBuffers(App_MscTestCfg *tConfig, uint32_t hndlIdx,
    uint64_t srcBuf, uint32_t *srcBufSize, uint32_t srcOffset,
    uint64_t dstBuf, uint32_t *dstBufSize, uint32_t dstOffset, uint32_t threadId)
{
    uint32_t  cnt;
    App_MscTestCfg *tCfg = tConfig;
    App_MscTestObj *appObj = NULL;

    if(VPAC_MSC_INST_ID_0 == threadId)
    {
        appObj = &gAppFcTestObject[hndlIdx].msc0TestObj;
    }
    else
    {
        appObj = &gAppFcTestObject[hndlIdx].msc1TestObj;
    }

    /* Change to allocation based on image size after bring up */
    *srcBufSize = 0u;

    appObj->inFrm.addr[0U] = srcBuf;
    appObj->inFrm.addr[1U] = (srcBuf +
                    (tCfg->inFrm.inPitch * tCfg->inFrm.inHeight));

    srcBuf += srcOffset;
    *srcBufSize += srcOffset;

    Vhwa_print(" MSC Input BufferAddress 0x%x, 0x%x\n",
            (uint32_t)appObj->inFrm.addr[0U],
            (uint32_t)appObj->inFrm.addr[1U]);

    *dstBufSize = 0u;
    for(cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        if(tCfg->mscCfgPrms[cnt].enable == UTRUE)
        {
            GT_assert(VhwaMscTrace, (((tCfg->outFrm[cnt].outPitch *
                        tCfg->outFrm[cnt].outHeight * 3) / 2) <= dstOffset));

            appObj->outFrm[cnt].addr[0U] = dstBuf;

            if(FVID2_DF_YUV420SP_UV == tCfg->outFrm[cnt].outDataFmt)
            {
                appObj->outFrm[cnt].addr[1U] = (dstBuf +
                        (tCfg->outFrm[cnt].outPitch * tCfg->outFrm[cnt].outHeight));

            }
            *dstBufSize += dstOffset;
            dstBuf += dstOffset;

            Vhwa_print(" MSC Output BufferAddress 0x%x, 0x%x\n",
                    (uint32_t)appObj->outFrm[cnt].addr[0U],
                    (uint32_t)appObj->outFrm[cnt].addr[1U]);
        }
    }

    return (FVID2_SOK);
}

int32_t AppFc_CrcInit(Udma_DrvHandle udmaDrvHndl)
{
    int32_t status = FVID2_SOK;


    return status;
}

int32_t AppFc_CrcDeinit(Udma_DrvHandle udmaDrvHndl)
{
    int32_t status = FVID2_SOK;

    return status;
}

int32_t AppFc_CompareCrc(AppFc_TestParams *tPrms, uint32_t hIdx)
{
    int32_t             status = FVID2_SOK;


    return (status);

}

void AppFc_PrepareRequest(AppFc_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    Fvid2_FrameList    *inFrmList;
    Fvid2_FrameList    *outFrmList;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    inFrmList = &tObj->inFrmList;
    outFrmList = &tObj->outFrmList;

    if(tPrms->isVissEnabled == UTRUE && FVID2_SOK == status)
    {
        for(cnt = 0; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt++)
        {
            inFrmList->frames[VHWA_FC_VISS_SRC_BUFF_IDX_START + cnt] =
                                                &tObj->vissTestObj.inFrm[cnt];
        }
        for(cnt = 0; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt++)
        {
            outFrmList->frames[VHWA_FC_VISS_DST_BUFF_IDX_START + cnt] =
                                                &tObj->vissTestObj.outFrm[cnt];
        }
    }
    if(tPrms->isMsc0Enabled == UTRUE && FVID2_SOK == status)
    {
        inFrmList->frames[VHWA_FC_MSC0_SRC_BUFF_IDX_START + cnt] =
                                                    &tObj->msc0TestObj.inFrm;

        for(cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            outFrmList->frames[VHWA_FC_MSC0_DST_BUFF_IDX_START + cnt] =
                                                &tObj->msc0TestObj.outFrm[cnt];
        }
    }

    if(tPrms->isMsc1Enabled == UTRUE && FVID2_SOK == status)
    {
        inFrmList->frames[VHWA_FC_MSC1_SRC_BUFF_IDX_START + cnt] =
                                                    &tObj->msc1TestObj.inFrm;

        for(cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            outFrmList->frames[VHWA_FC_MSC1_DST_BUFF_IDX_START + cnt] =
                                                &tObj->msc1TestObj.outFrm[cnt];
        }
    }

    inFrmList->numFrames = VHWA_FC_TOTAL_SRC_BUFF;
    outFrmList->numFrames = VHWA_FC_TOTAL_DST_BUFF;
}

int32_t AppFc_SubmitRequest(AppFc_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status;
    Fvid2_FrameList    *inFrmList;
    Fvid2_FrameList    *outFrmList;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    inFrmList = &tObj->inFrmList;
    outFrmList = &tObj->outFrmList;

    /* Submit Request*/
    status = Fvid2_processRequest(tObj->handle, inFrmList,
        outFrmList, FVID2_TIMEOUT_FOREVER);
    if (FVID2_SOK != status)
    {
        Vhwa_print
            (" FC: Fvid2_processRequest returned %x for Handle # %d\n",
            status, hidx);
    }

    return (status);
}

int32_t AppFc_WaitForCompRequest(AppFc_TestParams *tPrms, uint32_t hidx)
{
    int32_t          status = FVID2_SOK;
    Fvid2_FrameList *inFrmList;
    Fvid2_FrameList *outFrmList;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    VhwaAl_SemaphoreP_pend(tObj->waitForProcessCmpl, SYSTEMP_WAIT_FOREVER);

    inFrmList = &tObj->inFrmList;
    outFrmList = &tObj->outFrmList;

    status = Fvid2_getProcessedRequest(tObj->handle,
        inFrmList, outFrmList, 0);
    if (FVID2_SOK != status)
    {
        Vhwa_print (" Failed Handle Cnt %d; status = %x\n",
            hidx, status);
        return (status);
    }
    else
    {
        if (tPrms->isPerformanceTest)
        {
            status = Fvid2_control(tObj->handle,
                                   IOCTL_VHWA_FC_GET_PERFORMANCE,
                                   &gPerfNum[gPerfCnt], NULL);
            if(FVID2_SOK == status)
            {
                Vhwa_print (" FC Performance %d\n", gPerfNum[gPerfCnt]);
            }
            gPerfCnt++;
            if (gPerfCnt >= 10)
            {
                gPerfCnt = 0;
            }
        }
    }

    return (status);
}

static int32_t AppFc_VissSetParams(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    status = Fvid2_control(tObj->handle, IOCTL_VHWA_M2M_VISS_SET_PARAMS,
        (void *)&tCfg->vissPrms, NULL);

    return (status);
}

static int32_t AppFc_VissSetRfeConfig(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Rfe_Control         rfeCtrl;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    vissCfg = tCfg->vissCfg;

    if (NULL != vissCfg->lPwlCfg)
    {
        /* PWL for Long Input */
        rfeCtrl.module  = RFE_MODULE_PWL1;
        rfeCtrl.pwl1Cfg = vissCfg->lPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->sPwlCfg)
    {
        /* PWL for Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL2;
        rfeCtrl.pwl2Cfg = vissCfg->sPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->vsPwlCfg)
    {
        /* PWL for Very Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL3;
        rfeCtrl.pwl3Cfg = vissCfg->vsPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->lLutCfg)
    {
        /* PWL Lut for Long Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT1;
        rfeCtrl.decomp1Cfg      = vissCfg->lLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->sLutCfg)
    {
        /* PWL Lut for Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT2;
        rfeCtrl.decomp2Cfg      = vissCfg->sLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->vsLutCfg)
    {
        /* PWL Lut for Very Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT3;
        rfeCtrl.decomp3Cfg      = vissCfg->vsLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->wdr1Cfg)
    {
        /* WDR Merge 1 block configuration */
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA1;
        rfeCtrl.wdrMergeMa1  = vissCfg->wdr1Cfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->wdr2Cfg)
    {
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA2;
        rfeCtrl.wdrMergeMa2  = vissCfg->wdr2Cfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->comp20To16LutCfg)
    {
        /* Set Companding Lut to convert from 20 to 16bits */
        rfeCtrl.module       = RFE_MODULE_COMP_LUT;
        rfeCtrl.compCfg      = vissCfg->comp20To16LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->dpcOtf)
    {
        rfeCtrl.module       = RFE_MODULE_DPC_OTF;
        rfeCtrl.dpcOtfCfg    = vissCfg->dpcOtf;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->dpcLut)
    {
        rfeCtrl.module       = RFE_MODULE_DPC_LUT;
        rfeCtrl.dpcLutCfg    = vissCfg->dpcLut;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->lscCfg)
    {
        rfeCtrl.module      = RFE_MODULE_LSC;
        rfeCtrl.lscConfig   = vissCfg->lscCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->wbCfg)
    {
        rfeCtrl.module       = RFE_MODULE_GAIN_OFST;
        rfeCtrl.wbConfig     = vissCfg->wbCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    return (status);
}

static int32_t AppFc_VissSetFcpConfig(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    Fcp_Control         fcpCtrl;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    vissCfg = tCfg->vissCfg;
    vissPrms = &tCfg->vissPrms;

    if (NULL != vissCfg->comp16To12LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_COMPANDING;
        fcpCtrl.inComp = vissCfg->comp16To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->cfaCfg)
    {
        fcpCtrl.module          = FCP_MODULE_CFA;
        fcpCtrl.cfa             = vissCfg->cfaCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->ccm)
    {
        fcpCtrl.module      = FCP_MODULE_CCM;
        fcpCtrl.ccm         = vissCfg->ccm;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->gamma)
    {
        fcpCtrl.module = FCP_MODULE_GAMMA;
        fcpCtrl.gamma  = vissCfg->gamma;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->rgb2Hsv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2HSV;
        fcpCtrl.rgb2Hsv         = vissCfg->rgb2Hsv;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->rgb2yuv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2YUV;
        fcpCtrl.rgb2Yuv         = vissCfg->rgb2yuv;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->yuvSatLutCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_YUV_SAT_LUT;
        fcpCtrl.yuvSatLut               = vissCfg->yuvSatLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->histCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_HISTOGRAM;
        fcpCtrl.hist                    = vissCfg->histCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (((VHWA_M2M_VISS_EE_ON_FCP0_LUMA12 == vissPrms->edgeEnhancerMode) ||
         (VHWA_M2M_VISS_EE_ON_FCP0_LUMA8 == vissPrms->edgeEnhancerMode)) &&
        (NULL != vissCfg->eeCfg))
    {
        if (VHWA_M2M_VISS_EE_ON_FCP0_LUMA12 == vissPrms->edgeEnhancerMode)
        {
            vissCfg->eeCfg->bypassY12 = UFALSE;
            vissCfg->eeCfg->eeForY12OrY8 = 0u;
        }
        else
        {
            vissCfg->eeCfg->bypassY12 = UTRUE;
        }
        if (VHWA_M2M_VISS_EE_ON_FCP0_LUMA8 == vissPrms->edgeEnhancerMode)
        {
            vissCfg->eeCfg->bypassY8 = UFALSE;
            vissCfg->eeCfg->eeForY12OrY8 = 1u;
            vissCfg->eeCfg->leftShift = 2u;
            vissCfg->eeCfg->rightShift = 2u;
        }
        else
        {
            vissCfg->eeCfg->bypassY8 = UTRUE;
        }

        fcpCtrl.module              = FCP_MODULE_EE;
        fcpCtrl.eeCfg               = vissCfg->eeCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    return (status);
}

static int32_t AppFc_VissSetGlbceConfig(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    Glbce_Control       glbceCtrl;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    vissCfg = tCfg->vissCfg;
    vissPrms = &tCfg->vissPrms;

    if (UTRUE == vissPrms->enableGlbce)
    {
        if (NULL != vissCfg->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.glbceCfg = vissCfg->glbceCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->fwdPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_FWD_PERCEPT;
            glbceCtrl.fwdPrcptCfg = vissCfg->fwdPrcpCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->revPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_REV_PERCEPT;
            glbceCtrl.revPrcptCfg = vissCfg->revPrcpCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }
    }

    return (status);
}

static int32_t AppFc_VissSetNsf4Config(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    vissCfg = tCfg->vissCfg;
    vissPrms = &tCfg->vissPrms;

    if ((UTRUE == vissPrms->enableNsf4) && (NULL != vissCfg->nsf4Cfg))
    {
        status = Fvid2_control(tObj->handle, IOCTL_NSF4_SET_CONFIG,
            (void *)vissCfg->nsf4Cfg, NULL);
    }

    return (status);
}

static int32_t AppFc_VissSetH3aConfig(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    Rfe_Control         rfeCtrl;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppFc_TestObject *tObj = &gAppFcTestObject[hidx];

    vissCfg = tCfg->vissCfg;
    vissPrms = &tCfg->vissPrms;

    if ((uint32_t)UTRUE == vissPrms->outPrms[VHWA_M2M_VISS_OUT_H3A_IDX].enable)
    {
        if (NULL != vissCfg->rfeH3aInCfg)
        {
            /* H3A Input Selection and Lut configuration */
            rfeCtrl.module   = RFE_MODULE_H3A;
            rfeCtrl.h3aInCfg = vissCfg->rfeH3aInCfg;

            status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
                (void *)&rfeCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->h3aLutCfg)
        {
            /* H3A Input Selection and Lut configuration */
            rfeCtrl.module   = RFE_MODULE_H3A_LUT;
            rfeCtrl.h3aLutCfg = vissCfg->h3aLutCfg;

            status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
                (void *)&rfeCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->h3aCfg)
        {
            status = Fvid2_control(tObj->handle, IOCTL_H3A_SET_CONFIG,
                (void *)vissCfg->h3aCfg, NULL);
        }
    }

    return (status);
}

static int32_t AppFc_VissSetAllConfig(AppViss_TestConfig *tCfg, uint32_t hidx)
{
    int32_t             status = FVID2_EBADARGS;

    if (NULL != tCfg)
    {
        status = AppFc_VissSetParams(tCfg, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: SetParams Failed \n");
            status = FVID2_EFAIL;
            return status;
        }

        status = AppFc_VissSetRfeConfig(tCfg, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set RFE Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppFc_VissSetFcpConfig(tCfg, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set FCP Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppFc_VissSetGlbceConfig(tCfg, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set GLBCE Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppFc_VissSetNsf4Config(tCfg, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set NSF4 Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppFc_VissSetH3aConfig(tCfg, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set H3A Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
    }

    return status;
}

static int32_t AppFc_MscSetCoeff(Msc_Coeff * coefTbl, uint32_t hndlIdx,
                                    uint32_t threadId)
{
    int32_t status;
    AppFc_TestObject *appObj = &gAppFcTestObject[hndlIdx];
    Vhwa_M2mFcMscArgs           mscArgs;

    mscArgs.mscInstId = threadId;
    mscArgs.cmdArgs   = (Ptr) coefTbl;

    status = Fvid2_control(appObj->handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
        &mscArgs, NULL);

    return (status);
}

static int32_t AppFc_MscSetParams(App_MscTestCfg *tCfg, uint32_t hndlIdx,
                                uint32_t threadId)
{
    int32_t status, cnt;
    AppFc_TestObject *appObj = &gAppFcTestObject[hndlIdx];
    Vhwa_M2mMscParams *mscPrms;
    Msc_ScConfig *scCfg;
    Vhwa_M2mFcMscArgs           mscArgs;

    if(VPAC_MSC_INST_ID_0 == threadId)
    {
        mscPrms = &appObj->msc0TestObj.mscPrms[hndlIdx];
    }
    else
    {
        mscPrms = &appObj->msc1TestObj.mscPrms[hndlIdx];
    }

    mscPrms->loopBack = tCfg->loopBack;

    mscPrms->isEnableSimulProcessing = tCfg->isEnableSimulProcessing;
    mscPrms->enableLineSkip = tCfg->inFrm.skipInputLine;
    mscPrms->inFmt.width = tCfg->inFrm.inWidth;
    mscPrms->inFmt.height = tCfg->inFrm.inHeight;
    mscPrms->inFmt.dataFormat = tCfg->inFrm.inDataFmt;
    mscPrms->inFmt.ccsFormat = tCfg->inFrm.inCcsf;
    mscPrms->inFmt.pitch[0U] = tCfg->inFrm.inPitch;
    mscPrms->mscCfg.tapSel = tCfg->inFrm.tapSel;

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
    mscPrms->secChPrms.enable = tCfg->inSecFrm.enable;
    mscPrms->secChPrms.pitch = tCfg->inSecFrm.inPitch;
    mscPrms->secChPrms.ccsf = tCfg->inSecFrm.inCcsf;
#endif

    for(cnt = 0; cnt < MSC_MAX_OUTPUT;cnt++)
    {
        scCfg = &mscPrms->mscCfg.scCfg[cnt];
        if(tCfg->mscCfgPrms[cnt].enable == UTRUE)
        {
            scCfg->enable = UTRUE;

            mscPrms->outFmt[cnt].width = tCfg->outFrm[cnt].outWidth;
            mscPrms->outFmt[cnt].height = tCfg->outFrm[cnt].outHeight;
            mscPrms->outFmt[cnt].dataFormat = tCfg->outFrm[cnt].outDataFmt;
            mscPrms->outFmt[cnt].ccsFormat = tCfg->outFrm[cnt].outCcsf;
            mscPrms->outFmt[cnt].pitch[0U] = tCfg->outFrm[cnt].outPitch;

            scCfg->inRoi.cropStartX = tCfg->mscCfgPrms[cnt].inPos.startX;
            scCfg->inRoi.cropStartY = tCfg->mscCfgPrms[cnt].inPos.startY;
            scCfg->inRoi.cropWidth = tCfg->outFrm[cnt].roiWidth;
            scCfg->inRoi.cropHeight = tCfg->outFrm[cnt].roiHeight;
            scCfg->outWidth = tCfg->outFrm[cnt].outWidth;
            scCfg->outHeight = tCfg->outFrm[cnt].outHeight;
            scCfg->horzAccInit = tCfg->mscCfgPrms[cnt].horzAccInit;
            scCfg->vertAccInit = tCfg->mscCfgPrms[cnt].vertAccInit;
            scCfg->filtMode = tCfg->mscCfgPrms[cnt].filtMode;
            scCfg->phaseMode = tCfg->mscCfgPrms[cnt].phaseMode;
            scCfg->hsSpCoeffSel = tCfg->mscCfgPrms[cnt].hsSpCoeffSel;
            scCfg->vsSpCoeffSel = tCfg->mscCfgPrms[cnt].vsSpCoeffSel;
            scCfg->hsMpCoeffSel = tCfg->mscCfgPrms[cnt].hsMpCoeffSel;
            scCfg->vsMpCoeffSel = tCfg->mscCfgPrms[cnt].vsMpCoeffSel;
            scCfg->coeffShift = tCfg->mscCfgPrms[cnt].coeffShift;
            scCfg->isSignedData = tCfg->mscCfgPrms[cnt].isSignedData;
            scCfg->isEnableFiltSatMode = tCfg->mscCfgPrms[cnt].isEnableFiltSatMode;
        }
        else
        {
            scCfg->enable = UFALSE;
        }
    }

    mscPrms->loopBack = tCfg->loopBack;

    mscArgs.mscInstId = threadId;
    mscArgs.cmdArgs   = (Ptr) mscPrms;

    status = Fvid2_control(appObj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                            &mscArgs, NULL);

    return (status);
}

