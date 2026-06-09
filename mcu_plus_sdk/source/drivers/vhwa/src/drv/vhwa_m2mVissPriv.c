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
 */

/**
 *  \file vhwa_m2mVissApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mVissPriv.h"

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

static int32_t vhwaM2mVissInitYuv422OutChPrms(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId);
static int32_t vhwaM2mVissInitYuv420OutChPrms(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId,
    uint32_t mode);
static int32_t vhwaM2mVissInitOutChPrmsForRGB888(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId,
    uint32_t mode);
static int32_t vhwaM2mVissInitOutChPrmsForSinglePlane(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId);
static int32_t vhwaM2mVissInitOutChPrmsForH3a(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId);

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
static int32_t vhwaM2mVissInitOutChPrmsFor2PI(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId);
static int32_t vhwaM2mVissInitYuv422SpOutChPrms(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId,
    uint32_t mode);
#endif
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
static int32_t vhwaM2mVissInitOutChPrmsForIrOutput(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId);
static int32_t vhwaM2mVissInitOutChPrmsForIrOutputOnBayer(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mVissSetInChParams(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                cnt;
    uint32_t                sl2MemSize = 0;
    uint64_t                sl2Addr;
    Fvid2_Format           *fmt = NULL;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;

    /* Check for Null Pointer */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));

    sl2Prms = &instObj->sl2AllocPrms;
    sl2Addr = instObj->sl2Addr;
    fmt     = &vsPrms->inFmt;

    if ((vsPrms->inputMode < VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT) ||
        (vsPrms->inputMode >= VHWA_M2M_VISS_MODE_MAX))
    {
        status = FVID2_EINVALID_PARAMS;
    }

    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt ++)
    {
        chPrms = &hObj->inChPrms[cnt];

        chPrms->isEnabled = (uint32_t)UFALSE;

        /* Width and Height and other common parameters are used from
         * inFmt only */
        chPrms->widthInBytes = Vhwa_calcHorzSizeInBytes(fmt->width,
            fmt->ccsFormat);
        chPrms->width        = fmt->width;
        chPrms->height       = fmt->height;
        chPrms->ccsf         = (Fvid2_ColorCompStorageFmt)fmt->ccsFormat;
        chPrms->dataFmt      = fmt->dataFormat;
        chPrms->imgPitch     = fmt->pitch[cnt];
        chPrms->sl2Pitch     = Vhwa_calcSl2Pitch(chPrms->widthInBytes);
        chPrms->sl2Depth     = sl2Prms->inDepth;
        chPrms->extraBlanking = 0U;
        chPrms->isLumaCh     = (uint32_t)UFALSE;
        chPrms->frmIdx       = cnt;
        chPrms->frmAddrIdx   = 0U;

        chPrms->sl2Addr      = sl2Addr;
        chPrms->bufAddr      = sl2Addr;

        /* Sl2 memory required for this input channel */
        sl2MemSize           = chPrms->sl2Depth * chPrms->sl2Pitch;

        if (VHWA_M2M_VISS_INPUT_IDX0 == cnt)
        {
            /* Ch0 is always enabled, no matter what viss mode is */
            chPrms->isEnabled   = (uint32_t)UTRUE;

            sl2Addr            += sl2MemSize;
            hObj->totalSl2Size += sl2MemSize;
        }
        else if (VHWA_M2M_VISS_INPUT_IDX1 == cnt)
        {
            if ((VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE == vsPrms->inputMode) ||
                (VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE == vsPrms->inputMode))
            {
                /* Ch1 is required for both two or three merge mode */
                chPrms->isEnabled = (uint32_t)UTRUE;

                sl2Addr            += sl2MemSize;
                hObj->totalSl2Size += sl2MemSize;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else if (VHWA_M2M_VISS_INPUT_IDX2 == cnt)
/* LDRA_JUSTIFY_END */
        {
            if (VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE == vsPrms->inputMode)
            {
                /* Ch3 is required only for three merge mode */
                chPrms->isEnabled = (uint32_t)UTRUE;

                sl2Addr            += sl2MemSize;
                hObj->totalSl2Size += sl2MemSize;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            chPrms->isEnabled = (uint32_t)UFALSE;
        }
/* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (instObj->sl2Size < hObj->totalSl2Size)
        {
            GT_1trace(VhwaVissTrace, GT_ERR,
                "Insufficient Internal Memory for Input%d !!\n", cnt);
            status = FVID2_EFAIL;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}


int32_t Vhwa_m2mVissSetOutChParams(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms)
{
    int32_t                     status = FVID2_SOK;
    uint32_t                    cnt;
    Vhwa_M2mVissOutputParams   *outPrms = NULL;
    Fvid2_Format               *outFmt = NULL;

    /* Check for Null Pointer */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));

    /* First mark all channels/outputs disabled */
    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
    {
        /* By Default All channels are disabled */
        hObj->outChPrms[cnt].isEnabled = (uint32_t)UFALSE;
    }

    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
    {
        if (FVID2_SOK != status)
        {
            break;
        }
        outPrms = &vsPrms->outPrms[cnt];
        outFmt  = &outPrms->fmt;

        if ((uint32_t)UTRUE == outPrms->enable)
        {
            #if defined (VHWA_VPAC_IP_REV_VPAC3L)
            if (VHWA_VISS_IROUT_ENABLED == outPrms->isIrOut)
            {
                if ((FVID2_DF_RAW08 == outFmt->dataFormat) || (FVID2_DF_RAW12 == outFmt->dataFormat) || (FVID2_DF_RAW16 == outFmt->dataFormat))
                {
                    if (((FVID2_CCSF_BITS8_PACKED == outFmt->ccsFormat) ||
                         (FVID2_CCSF_BITS12_PACKED == outFmt->ccsFormat)) ||
                        (FVID2_CCSF_BITS12_UNPACKED16 == outFmt->ccsFormat) ||
                        (FVID2_CCSF_BITS16_PACKED == outFmt->ccsFormat))
                    {
                        status = vhwaM2mVissInitOutChPrmsForIrOutput(
                                                    instObj, hObj, vsPrms, cnt);

                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set IR Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "IR Data format not yet supported for  Output%d !!\n", cnt);
                    }
                }
                else
                {
                    GT_1trace(VhwaVissTrace, GT_ERR,
                            "For IR Data dataFormat should be FVID2_DF_RAW for Output%d !!\n", cnt);
                }
                /* Check if the Sl2 size is sufficient */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested
                <justification end> */
                if (hObj->totalSl2Size > instObj->sl2Size)
                {
                    GT_1trace(VhwaVissTrace, GT_ERR,
                        "Insufficient Internal Memory for Output%d !!\n", cnt);
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
            }
            else
            #endif
            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */
            switch (outFmt->dataFormat)
            {
/* LDRA_JUSTIFY_END */
                case FVID2_DF_GREY:
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The test framework and test apps cannot reach this portion.
                    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                    Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach here.
                    <justification end> */
                    if ((VHWA_M2M_VISS_OUT_GREY_8B_IDX == cnt) ||
                        (VHWA_M2M_VISS_OUT_GREY_12B_IDX == cnt))
                    /* LDRA_JUSTIFY_END */
                    {
                        status = vhwaM2mVissInitOutChPrmsForSinglePlane(
                            instObj, hObj, vsPrms, cnt);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set Grey Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported Grey Output on %d !!\n", cnt);
                    }
                    break;
                }
                case FVID2_DF_SATURATION:
                {
                    if (VHWA_M2M_VISS_OUT_SATURA_8B_IDX == cnt)
                    {
                        status = vhwaM2mVissInitOutChPrmsForSinglePlane(
                            instObj, hObj, vsPrms, cnt);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set Saturation Output%d !!\n",
                                cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported Saturation Output on %d !!\n",
                            cnt);
                    }
                    break;
                }
                case FVID2_DF_RAW:
                {
                    if (VHWA_M2M_VISS_OUT_H3A_IDX == cnt)
                    {
                        status = vhwaM2mVissInitOutChPrmsForH3a(
                            instObj, hObj, vsPrms, cnt);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set H3A Output%d !!\n",
                                cnt);
                        }
/* LDRA_JUSTIFY_END */
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The test framework and test apps cannot reach this portion.
                    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                    Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach here.
                    <justification end> */
                    else if ((VHWA_M2M_VISS_OUT_CFA_C1_12B_IDX == cnt) ||
                             (VHWA_M2M_VISS_OUT_CFA_C2_12B_IDX == cnt) ||
                             (VHWA_M2M_VISS_OUT_CFA_C3_12B_IDX == cnt) ||
                             (VHWA_M2M_VISS_OUT_CFA_C4_12B_IDX == cnt))
                    /* LDRA_JUSTIFY_END */
                    {
                        status = vhwaM2mVissInitOutChPrmsForSinglePlane(
                            instObj, hObj, vsPrms, cnt);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set RAW Output%d !!\n",
                                cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported H3A Output on %d !!\n",
                            cnt);
                    }
                    break;
                }
                case FVID2_DF_YUV420SP_UV:
                {
                    if ((VHWA_M2M_VISS_OUT_YUV420_12B_IDX == cnt) ||
                        (VHWA_M2M_VISS_OUT_YUV420_8B_IDX == cnt))
                    {
                        /* Two Channels are required for YUV420 output,
                         * Set the channel parameters */
                        status = vhwaM2mVissInitYuv420OutChPrms(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_LUMA_MODE |
                                VHWA_M2M_VISS_CHROMA_MODE);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set YUV420 Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported YUV420 Output on %d !!\n", cnt);
                    }
                    break;
                }
                case FVID2_DF_RGB24_888_PLANAR:
                {
                    if (VHWA_M2M_VISS_OUT_RGB888_8B_IDX == cnt)
                    {
                        /* Three Channels are required for RGB output,
                         * Set the channel parameters */
                        status = vhwaM2mVissInitOutChPrmsForRGB888(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_RED_COLOR_MODE |
                                VHWA_M2M_VISS_GREEN_COLOR_MODE |
                                VHWA_M2M_VISS_BLUE_COLOR_MODE);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set RGB888 Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported RGB888 Output on %d !!\n", cnt);
                    }
                    break;
                }
            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                case FVID2_DF_YUV422SP_UV:
                case FVID2_DF_YUV422SP_VU:
                {
                    if ((VHWA_M2M_VISS_OUT_YUV422_12B_IDX == cnt) ||
                        (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == cnt))
                    {
                        /* Two Channels are required for YUV420 output,
                         * Set the channel parameters */
                        status = vhwaM2mVissInitYuv422SpOutChPrms(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_LUMA_MODE |
                                VHWA_M2M_VISS_CHROMA_MODE);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set YUV422SP Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported YUV422SP Output on %d !!\n", cnt);
                    }
                    break;
                }
            #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
                case FVID2_DF_YUV422I_UYVY:
                case FVID2_DF_YUV422I_YUYV:
                {
                    if ((VHWA_M2M_VISS_OUT_YUV422_8B_IDX == cnt) ||
                        (VHWA_M2M_VISS_OUT_YUV422_12B_IDX == cnt))
                    {
                        /* Only one channel requied for YUV422 output */
                        status = vhwaM2mVissInitYuv422OutChPrms(
                            instObj, hObj, vsPrms, cnt);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set YUV422 Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported YUV422 Output on %d !!\n", cnt);
                    }
                    break;
                }
            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                //REVIEW
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                case FVID2_DF_RGI:
                {
                    if (VHWA_M2M_VISS_OUT_RGI_8B_IDX == cnt)
                    {
                        /* Only one channel requied for RGI output */
                        status = vhwaM2mVissInitOutChPrmsFor2PI(
                            instObj, hObj, vsPrms, cnt);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set RGI Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported RGI Output on %d !!\n", cnt);
                    }
                    break;
                }
                /* LDRA_JUSTIFY_END */
            #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
                case FVID2_DF_LUMA_ONLY:
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((VHWA_M2M_VISS_OUT_LUMA_12B_IDX == cnt) ||
                        (VHWA_M2M_VISS_OUT_LUMA_8B_IDX == cnt))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* Two Channels are required for YUV420 output,
                         * Set the channel parameters */
                        status = vhwaM2mVissInitYuv420OutChPrms(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_LUMA_MODE);
                         /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start> 
                        Rationale: The test framework and test apps cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != status)
                        /* LDRA_JUSTIFY_END */
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set Luma Output%d !!\n", cnt);
                        }
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported Luma Output on %d !!\n", cnt);
                    }
                    /* LDRA_JUSTIFY_END */
                    break;
                }
                case FVID2_DF_CHROMA_ONLY:
                {
                    if ((VHWA_M2M_VISS_OUT_CHROMA_12B_IDX == cnt) ||
                        (VHWA_M2M_VISS_OUT_CHROMA_8B_IDX == cnt))
                    {
                        /* Two Channels are required for YUV420 output,
                         * Set the channel parameters */
                        status = vhwaM2mVissInitYuv420OutChPrms(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_CHROMA_MODE);
                        //REVIEW
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start> 
                        Rationale: The test framework and test apps cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set Chroma Output%d !!\n", cnt);
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported Chroma Output on %d !!\n", cnt);
                    }
                    break;
                }
                case FVID2_DF_RED:
                {
                    if (VHWA_M2M_VISS_OUT_RED_8B_IDX == cnt)
                    {
                        status = vhwaM2mVissInitOutChPrmsForRGB888(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_RED_COLOR_MODE);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set RED Color Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported RED Color Output on %d !!\n", cnt);
                    }
                    break;
                }
                case FVID2_DF_GREEN:
                {
                    if (VHWA_M2M_VISS_OUT_GREEN_8B_IDX == cnt)
                    {
                        status = vhwaM2mVissInitOutChPrmsForRGB888(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_GREEN_COLOR_MODE);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set GREEN Color Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported GREEN Color Output on %d !!\n", cnt);
                    }
                    break;
                }
                case FVID2_DF_BLUE:
                {
                    if (VHWA_M2M_VISS_OUT_BLUE_8B_IDX == cnt)
                    {
                        status = vhwaM2mVissInitOutChPrmsForRGB888(
                            instObj, hObj, vsPrms, cnt,
                                VHWA_M2M_VISS_BLUE_COLOR_MODE);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set BLUE Color Output%d !!\n", cnt);
                        }
                    }
                    else
                    {
                        status = FVID2_EINVALID_PARAMS;
                        GT_1trace(VhwaVissTrace, GT_ERR,
                            "Not supported BLUE Color Output on %d !!\n", cnt);
                    }
                    break;
                }
                #if defined (VHWA_VPAC_IP_REV_VPAC3L)
                /*IR downstream*/
                case FVID2_DF_RAW08:
                case FVID2_DF_RAW12:
                {
                    status = vhwaM2mVissInitOutChPrmsForIrOutputOnBayer(
                            instObj, hObj, vsPrms, cnt);
                        if (FVID2_SOK != status)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR,
                                "Failed to set IR downstream Output%d !!\n",
                                cnt);
                        }
                }
                #endif
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */
                default:
                {
                    /* Nothing to do here */
                    break;
                }
/* LDRA_JUSTIFY_END */
            }
            }
            /* Check if the Sl2 size is sufficient */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            The handleObject sl2size requirement cannot be higher than the instanceObject in the current test configurations therefore the control cannot reach here.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (hObj->totalSl2Size > instObj->sl2Size)
            {
                GT_1trace(VhwaVissTrace, GT_ERR,
                    "Insufficient Internal Memory for Output%d !!\n", cnt);
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return (status);
}

/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */

static int32_t vhwaM2mVissInitYuv422OutChPrms(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    CslFcp_OutputSelect    *fcpOutCfg = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    chPrms  = &hObj->outChPrms[outId];
    sl2Prms = &instObj->sl2AllocPrms;

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

    fcpOutCfg = &hObj->fcpOutCfg;
    chPrms->maxVertBlanking = hObj->maxLseVertBlanking;
    chPrms->thr = hObj->thr;

#if defined VHWA_VPAC_IP_REV_VPAC3
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        fcpOutCfg = &hObj->fcp2OutCfg;
        chPrms->maxVertBlanking = hObj->maxLseVertBlankingMV;
        chPrms->thr = hObj->thr;
    }
#endif

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
    if ((VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId) ||
        (VHWA_M2M_VISS_OUT_YUV422_12B_IDX == outId))
/* LDRA_JUSTIFY_END */
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        if ((uint32_t)UFALSE == chPrms->isEnabled)
/* LDRA_JUSTIFY_END */
        {
            chPrms->isEnabled       = (uint32_t)UTRUE;

            /* For YUV422, min bytes is width * 2*/
            chPrms->widthInBytes    = Vhwa_calcHorzSizeInBytes(
                (fmt->width * 2U), outFmt->ccsFormat);
            chPrms->width           = fmt->width;
            chPrms->height          = fmt->height;
            chPrms->ccsf            =
                (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
            chPrms->dataFmt         = (uint32_t)outFmt->dataFormat;
            /* For YUV422, pitch is at 0th index */
            chPrms->imgPitch        = outFmt->pitch[0U];
            chPrms->sl2Pitch        =
                Vhwa_calcSl2Pitch(chPrms->widthInBytes);
            chPrms->sl2Depth        = sl2Prms->outDepth[outId];
            chPrms->isLumaCh        = UTRUE;
            chPrms->frmIdx          = outId;
            chPrms->frmAddrIdx      = 0U;

            chPrms->sl2Addr         = sl2Addr;

            /* For Luma channel, extra 2 lines of blanking required if
             * edge enhancer is enabled */
        #if defined VHWA_VPAC_IP_REV_VPAC
            if ((VHWA_M2M_VISS_OUT_YUV420_12B_IDX == outId) &&
                (VHWA_M2M_VISS_EE_ON_FCP0_LUMA12 == vsPrms->edgeEnhancerMode))
            {
                chPrms->extraBlanking = 0x2u;
            }
            else if ((VHWA_M2M_VISS_OUT_YUV420_8B_IDX == outId) &&
                     (VHWA_M2M_VISS_EE_ON_FCP0_LUMA8 == vsPrms->edgeEnhancerMode))
            {
                chPrms->extraBlanking = 0x2u;
            }
        #elif defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if ((FVID2_CCSF_BITS8_PACKED == chPrms->ccsf) &&
                (((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP0_LUMA8))
                  && (VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline))
                 || ((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP1_LUMA8))
                  && (VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline))))
            {
                chPrms->extraBlanking = 0x2u;
            }
            else if (((FVID2_CCSF_BITS12_UNPACKED16 == chPrms->ccsf) ||
                      (FVID2_CCSF_BITS12_PACKED == chPrms->ccsf)) &&
                (((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP0_LUMA12))
                  && (VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline))
                 || ((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP1_LUMA12))
                  && (VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline))))
            {
                chPrms->extraBlanking = 0x2u;
            }
        #endif /* #elif defined VHWA_VPAC_IP_REV_VPAC3 */
            else
            {
                chPrms->extraBlanking = 0x0u;
            }

            /* Increment the total Sl2 size used by this channel */
            hObj->totalSl2Size      = hObj->totalSl2Size +
                (chPrms->sl2Depth * chPrms->sl2Pitch);

            /* Pitch must be atleast same as widthInBytes */
            if (outFmt->pitch[0U] < chPrms->widthInBytes)
            {
                status = FVID2_EINVALID_PARAMS;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            status = FVID2_EINVALID_PARAMS;
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    else
    {
        status = FVID2_EINVALID_PARAMS;
    }
/* LDRA_JUSTIFY_END */

    /* Select the FCP muxes accordingly */
    if (FVID2_SOK == status)
    {
        /* FCP output mode for chroma is now 422 */
        fcpOutCfg->chromaMode = FCP_CHROMA_MODE_422;

        if(FVID2_CCSF_BITS8_PACKED == chPrms->ccsf)
        {
            fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_RGB2YUV_Y8;
            fcpOutCfg->chroma8OutSel = FCP_CHROMA8_OUT_CHROMA;

        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y8;
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV8;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP2_Y8;
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP2_UV8;
                }
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y8;
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV8;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP_Y8;
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP_UV8;
                }
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
        else
        {
            fcpOutCfg->luma12OutSel = FCP_LUMA12_OUT_RGB2YUV;
            fcpOutCfg->chroma12OutSel = FCP_CHROMA12_OUT_CHROMA;

        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y12;
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV12;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP2_Y12;
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP2_UV12;
                }
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y12;
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV12;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP_Y12;
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP_UV12;
                }
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
    }

    return (status);
}

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
static int32_t vhwaM2mVissInitOutChPrmsFor2PI(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    CslFcp_OutputSelect    *fcpOutCfg = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    chPrms  = &hObj->outChPrms[outId];
    sl2Prms = &instObj->sl2AllocPrms;

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

    fcpOutCfg = &hObj->fcpOutCfg;
    chPrms->maxVertBlanking = hObj->maxLseVertBlanking;
    chPrms->thr = hObj->thr;

#if defined VHWA_VPAC_IP_REV_VPAC3
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>  un-im
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
    <justification end> */
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        fcpOutCfg = &hObj->fcp2OutCfg;
        chPrms->maxVertBlanking = hObj->maxLseVertBlankingMV;
        chPrms->thr = hObj->thr;
    }
    /* LDRA_JUSTIFY_END */
#endif

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (VHWA_M2M_VISS_OUT_RGI_8B_IDX == outId)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
      if ((uint32_t)UFALSE == chPrms->isEnabled)
        /* LDRA_JUSTIFY_END */
        {
            chPrms->isEnabled       = (uint32_t)UTRUE;

            /* For RGI min bytes is width * 2*/
            chPrms->widthInBytes    = Vhwa_calcHorzSizeInBytes(
                (fmt->width * 2U), outFmt->ccsFormat);
            chPrms->width           = fmt->width;
            chPrms->height          = fmt->height;
            chPrms->ccsf            =
                (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
            chPrms->dataFmt         = (uint32_t)outFmt->dataFormat;
            /* For RGI, pitch is at 0th index */
            chPrms->imgPitch        = outFmt->pitch[0U];
            chPrms->sl2Pitch        =
                Vhwa_calcSl2Pitch(chPrms->widthInBytes);
            chPrms->sl2Depth        = sl2Prms->outDepth[outId];
            chPrms->extraBlanking   = 0x0u;
            chPrms->isLumaCh        = UFALSE;
            chPrms->frmIdx          = outId;
            chPrms->frmAddrIdx      = 0U;

            chPrms->sl2Addr         = sl2Addr;

            /* Increment the total Sl2 size used by this channel */
            hObj->totalSl2Size      = hObj->totalSl2Size +
                (chPrms->sl2Depth * chPrms->sl2Pitch);

            /* Pitch must be atleast same as widthInBytes */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (outFmt->pitch[0U] < chPrms->widthInBytes)
            /* LDRA_JUSTIFY_END */
            {
                status = FVID2_EINVALID_PARAMS;
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */
    /* Select the FCP muxes accordingly */
    /* Select the FCP muxes accordingly */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
<justification end> */
    if (FVID2_SOK == status)
    {
        /* FCP output mode is now RGI */
            fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_RED8;
            fcpOutCfg->chroma8OutSel = FCP_CHROMA8_OUT_GREEN;
#if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y8;
                hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV8;
            }
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y8;
                hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV8;
            }
#endif
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}
#endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */

static int32_t vhwaM2mVissInitOutChPrmsForRGB888(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId,
    uint32_t mode)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                cnt;
    uint32_t                pitch;
    uint64_t                sl2Addr;
    uint32_t                sl2Size;
    uint32_t                blanking, thr;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    CslFcp_OutputSelect    *fcpOutCfg = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;
    sl2Prms  = &instObj->sl2AllocPrms;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

    fcpOutCfg = &hObj->fcpOutCfg;
    blanking = hObj->maxLseVertBlanking;
    thr = hObj->thr;

#if defined VHWA_VPAC_IP_REV_VPAC3
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        fcpOutCfg = &hObj->fcp2OutCfg;
        blanking = hObj->maxLseVertBlankingMV;
        thr = hObj->thr;
    }
#endif

    /* Need to update three channels for RGB888 output,
     * all of them are almost same. */
    for (cnt = 0U; cnt < 3u; cnt ++)
    {
        if (FVID2_SOK != status)
        {
            break;
        }
        if (mode == (1U << cnt))
        {
            chPrms = &hObj->outChPrms[outId];
            pitch = outFmt->pitch[0u];
        }
        else
        {
            chPrms = &hObj->outChPrms[outId + cnt];
            pitch = outFmt->pitch[cnt];
        }

        if ((mode & ((uint32_t)1U << cnt)) == ((uint32_t)1U << cnt))
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
            if ((uint32_t)UFALSE == chPrms->isEnabled)
/* LDRA_JUSTIFY_END */
            {
                chPrms->isEnabled       = (uint32_t)UTRUE;

                chPrms->maxVertBlanking = blanking;
                chPrms->thr             = thr;

                chPrms->widthInBytes    = Vhwa_calcHorzSizeInBytes(fmt->width,
                    outFmt->ccsFormat);
                chPrms->width           = fmt->width;
                chPrms->height          = fmt->height;
                chPrms->ccsf            =
                    (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
                chPrms->dataFmt         = outFmt->dataFormat;
                chPrms->imgPitch        = outFmt->pitch[cnt];
                chPrms->sl2Pitch        =
                    Vhwa_calcSl2Pitch(chPrms->widthInBytes);
                chPrms->sl2Depth        = sl2Prms->outDepth[outId];
                chPrms->extraBlanking   = 0x0u;
                chPrms->isLumaCh        = UFALSE;
                chPrms->frmIdx          = outId;

                /* Only single FVID2 frame is used for RGB planar output,
                 * so frame address index is different */
                if (FVID2_DF_RGB24_888_PLANAR == outFmt->dataFormat)
                {
                    chPrms->frmAddrIdx      = cnt;
                    chPrms->imgPitch        = outFmt->pitch[cnt];
                }
                else /* One FVID2 Frame is used for each color component. */
                {
                    chPrms->frmAddrIdx      = 0U;
                    chPrms->imgPitch        = outFmt->pitch[0u];
                }

                chPrms->sl2Addr         = sl2Addr;

                sl2Size                 = (chPrms->sl2Depth *
                    chPrms->sl2Pitch);

                /* Increment the total Sl2 size used by this channel */
                hObj->totalSl2Size     += sl2Size;

                /* Move sl2 start address by required sl2 size */
                sl2Addr                 = sl2Addr + sl2Size;

                /* Pitch must be atleast same as widthInBytes */
                if (pitch < chPrms->widthInBytes)
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
            else
            {
                /* If channels are already enabled for some other output format,
                 * return error */
                status = FVID2_EINVALID_PARAMS;
            }
/* LDRA_JUSTIFY_END */
        }
    }

    /* Select the FCP muxes accordingly */
    if (FVID2_SOK == status)
    {
        /* FCP output mode is now RGB888 */
        if ((mode & VHWA_M2M_VISS_RED_COLOR_MODE) ==
                VHWA_M2M_VISS_RED_COLOR_MODE)
        {
            fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_RED8;
        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y8;
            }
		#endif
		#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        //REVIEW
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> branch <metric end>
        <justification start>
        Rationale:
        Effect on this unit:
        <justification end> */
        	if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
        /* LDRA_JUSTIFY_END_DISABLED */    
            {
                hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y8;
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
        if ((mode & VHWA_M2M_VISS_GREEN_COLOR_MODE) ==
                VHWA_M2M_VISS_GREEN_COLOR_MODE)
        {
            fcpOutCfg->chroma8OutSel = FCP_CHROMA8_OUT_GREEN;
        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV8;
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        //REVIEW
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> branch <metric end>
        <justification start>
        Rationale:
        Effect on this unit:
        <justification end> */
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV8;
            }
            /* LDRA_JUSTIFY_END_DISABLED */
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
        if ((mode & VHWA_M2M_VISS_BLUE_COLOR_MODE) ==
                VHWA_M2M_VISS_BLUE_COLOR_MODE)
        {
            fcpOutCfg->sat8OutSel = FCP_SAT8_OUT_BLUE;
        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.sat8OutSel = VISS_OUT_FCP2_S8;
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        //REVIEW
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> branch <metric end>
        <justification start>
        Rationale:
        Effect on this unit:
        <justification end> */
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                hObj->vissTopOutMux.sat8OutSel = VISS_OUT_FCP_S8;
            }
            /* LDRA_JUSTIFY_END_DISABLED */
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
    }

    return (status);
}

static int32_t vhwaM2mVissInitYuv420OutChPrms(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId,
    uint32_t mode)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    uint32_t                sl2Size;
    uint32_t                blanking, thr;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    Vhwa_M2mVissChParams   *chPrmsLm = NULL;
    Vhwa_M2mVissChParams   *chPrmsCh = NULL;
    CslFcp_OutputSelect    *fcpOutCfg = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    sl2Prms  = &instObj->sl2AllocPrms;

    if (mode == (VHWA_M2M_VISS_LUMA_MODE | VHWA_M2M_VISS_CHROMA_MODE))
    {
        chPrmsLm = &hObj->outChPrms[outId];
        chPrmsCh = &hObj->outChPrms[outId + 1u];
    }
    else if ((mode & VHWA_M2M_VISS_LUMA_MODE) == VHWA_M2M_VISS_LUMA_MODE)
    {
        chPrmsLm = &hObj->outChPrms[outId];
    }
    else
    {
        chPrmsCh = &hObj->outChPrms[outId];
    }

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

    fcpOutCfg = &hObj->fcpOutCfg;
    blanking = hObj->maxLseVertBlanking;
    thr = hObj->thr;

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
    if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
    {
        if(chPrmsLm != NULL)
        {
          chPrmsLm->vPipe = VHWA_VISS_PIPE_HV;
        }
        if(chPrmsCh != NULL)
        {
          chPrmsCh->vPipe = VHWA_VISS_PIPE_HV;
        }
    }
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        fcpOutCfg = &hObj->fcp2OutCfg;
        blanking = hObj->maxLseVertBlankingMV;
        thr = hObj->thr;
        if(chPrmsLm != NULL)
        {
          chPrmsLm->vPipe = VHWA_VISS_PIPE_MV;
        }
        if(chPrmsCh != NULL)
        {
          chPrmsCh->vPipe = VHWA_VISS_PIPE_MV;
        }
    }
#endif

    if ((mode & VHWA_M2M_VISS_LUMA_MODE) == VHWA_M2M_VISS_LUMA_MODE)
    {
     /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
      if(chPrmsLm != NULL)
    /* LDRA_JUSTIFY_END */
      {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        if ((uint32_t)UFALSE == chPrmsLm->isEnabled)
/* LDRA_JUSTIFY_END */
        {
            /* Initialize Channel Parameter for Luma output channel  */
            chPrmsLm->isEnabled    = (uint32_t)UTRUE;

            chPrmsLm->maxVertBlanking = blanking;
            chPrmsLm->thr             = thr;

            chPrmsLm->widthInBytes = Vhwa_calcHorzSizeInBytes(fmt->width,
                outFmt->ccsFormat);
            chPrmsLm->width        = fmt->width;
            chPrmsLm->height       = fmt->height;
            chPrmsLm->ccsf         =
                (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
            chPrmsLm->dataFmt      = (uint32_t)outFmt->dataFormat;
            chPrmsLm->imgPitch     = outFmt->pitch[0U];
            chPrmsLm->sl2Pitch     = Vhwa_calcSl2Pitch(chPrmsLm->widthInBytes);
            chPrmsLm->sl2Depth     = sl2Prms->outDepth[outId];
            chPrmsLm->isLumaCh     = (uint32_t)UTRUE;
            chPrmsLm->frmIdx       = outId;
            chPrmsLm->frmAddrIdx   = 0U;

            chPrmsLm->sl2Addr      = sl2Addr;

            /* For Luma channel, extra 2 lines of blanking required if
             * edge enhancer is enabled */
        #if defined VHWA_VPAC_IP_REV_VPAC
            if ((VHWA_M2M_VISS_OUT_YUV420_12B_IDX == outId) &&
                (VHWA_M2M_VISS_EE_ON_FCP0_LUMA12 == vsPrms->edgeEnhancerMode))
            {
                chPrmsLm->extraBlanking = 0x2u;
            }
            else if ((VHWA_M2M_VISS_OUT_YUV420_8B_IDX == outId) &&
                     (VHWA_M2M_VISS_EE_ON_FCP0_LUMA8 == vsPrms->edgeEnhancerMode))
            {
                chPrmsLm->extraBlanking = 0x2u;
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if ((FVID2_CCSF_BITS8_PACKED == chPrmsLm->ccsf) &&
                (((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP0_LUMA8))
                  && (VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline))
                 || ((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP1_LUMA8))
                  && (VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline))))
            {
                chPrmsLm->extraBlanking = 0x2u;
            }
            else if (((FVID2_CCSF_BITS12_UNPACKED16 == chPrmsLm->ccsf) ||
                      (FVID2_CCSF_BITS12_PACKED == chPrmsLm->ccsf)) &&
                (((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP0_LUMA12))
                  && (VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline))
                 || ((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP1_LUMA12))
                  && (VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline))))
            {
                chPrmsLm->extraBlanking = 0x2u;
            }
        #endif
            else
            {
                chPrmsLm->extraBlanking = 0x0u;
            }

            sl2Size             = (chPrmsLm->sl2Depth * chPrmsLm->sl2Pitch);
            /* Increment the total Sl2 size used by this channel */
            hObj->totalSl2Size  = hObj->totalSl2Size + sl2Size;
            /* Move sl2 start address by required sl2 size */
            sl2Addr             = sl2Addr + sl2Size;

            /* Pitch must be atleast same as widthInBytes */
            if (outFmt->pitch[0U] < chPrmsLm->widthInBytes)
            {
                status = FVID2_EINVALID_PARAMS;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            status = FVID2_EINVALID_PARAMS;
        }
/* LDRA_JUSTIFY_END */
        if(FVID2_CCSF_BITS8_PACKED == chPrmsLm->ccsf)
        {
            fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_RGB2YUV_Y8;

        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV420_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y8;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP2_Y8;
                }
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV420_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y8;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP_Y8;
                }
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
        else
        {
            /* FCP output mode is now YUV420 12bit output */
            fcpOutCfg->luma12OutSel = FCP_LUMA12_OUT_RGB2YUV;

        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV420_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y12;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP2_Y12;
                }
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                  /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion.
                Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
                A test case will be added to cover this gap in a future release.
                <justification end> */
                if (VHWA_M2M_VISS_OUT_YUV420_8B_IDX == outId)
                /* LDRA_JUSTIFY_END */
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y12;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion.
                Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
                A test case will be added to cover this gap in a future release.
                <justification end> */
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP_Y12;
                }
                /* LDRA_JUSTIFY_END */
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
      }
    }

    if ((mode & VHWA_M2M_VISS_CHROMA_MODE) == VHWA_M2M_VISS_CHROMA_MODE)
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
      if(chPrmsCh != NULL)
/* LDRA_JUSTIFY_END */
      {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        if ((uint32_t)UFALSE == chPrmsCh->isEnabled)
/* LDRA_JUSTIFY_END */
        {
            /* Initialiaze Channel Parameter for Chroma output channel  */
            chPrmsCh->isEnabled    = (uint32_t)UTRUE;

            chPrmsCh->maxVertBlanking = blanking;
            chPrmsCh->thr             = thr;

            if (NULL != chPrmsLm)
            {
                chPrmsCh->widthInBytes = chPrmsLm->widthInBytes;
            }
            else
            {
                chPrmsCh->widthInBytes = Vhwa_calcHorzSizeInBytes(fmt->width,
                                           outFmt->ccsFormat);
            }
            chPrmsCh->width        = fmt->width;
            chPrmsCh->height       = fmt->height / 2u;
            chPrmsCh->ccsf         =
                (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
            chPrmsCh->dataFmt      = outFmt->dataFormat;

            /* Pitch must be atleast same as widthInBytes */
            if (FVID2_DF_CHROMA_ONLY == outFmt->dataFormat)
            {
                chPrmsCh->imgPitch = outFmt->pitch[0U];
            }
            else
            {
                chPrmsCh->imgPitch = outFmt->pitch[1U];
            }

            if (NULL != chPrmsLm)
            {
                chPrmsCh->sl2Pitch     = chPrmsLm->sl2Pitch;
            }
            else
            {
                chPrmsCh->sl2Pitch     = Vhwa_calcSl2Pitch(chPrmsCh->widthInBytes);
            }
            /* Luma and chroma have same depth in SL2 */
            chPrmsCh->sl2Depth     = sl2Prms->outDepth[outId];
            /* Extra blanking required for the chroma channel */
            chPrmsCh->extraBlanking = 0x1u;
            chPrmsCh->isLumaCh     = UFALSE;
            chPrmsCh->dataFmt      = (uint32_t)outFmt->dataFormat;
            chPrmsCh->frmIdx       = outId;
            if (NULL != chPrmsLm)
            {
                chPrmsCh->frmAddrIdx   = 1U;
            }
            else
            {
                chPrmsCh->frmAddrIdx   = 0U;
            }

            chPrmsCh->sl2Addr      = sl2Addr;

            sl2Size                =
                (chPrmsCh->sl2Depth * chPrmsCh->sl2Pitch);

            /* Increment the total Sl2 size used by this channel */
            hObj->totalSl2Size = hObj->totalSl2Size + sl2Size;

            /* Pitch must be atleast same as widthInBytes */
            if (FVID2_DF_CHROMA_ONLY == outFmt->dataFormat)
            {
                if (outFmt->pitch[0U] < chPrmsCh->widthInBytes)
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
            else
            {
                if (outFmt->pitch[1U] < chPrmsCh->widthInBytes)
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            status = FVID2_EINVALID_PARAMS;
        }
/* LDRA_JUSTIFY_END */

        if(FVID2_CCSF_BITS8_PACKED == chPrmsCh->ccsf)
        {
            fcpOutCfg->chroma8OutSel = FCP_CHROMA8_OUT_CHROMA;

        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if ((outId == VHWA_M2M_VISS_OUT_YUV420_8B_IDX) ||
                    (outId == VHWA_M2M_VISS_OUT_CHROMA_8B_IDX))
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV8;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP2_UV8;
                }
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if ((outId == VHWA_M2M_VISS_OUT_YUV420_8B_IDX) ||
                    (outId == VHWA_M2M_VISS_OUT_CHROMA_8B_IDX))
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV8;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP_UV8;
                }
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
        else
        {
            fcpOutCfg->chroma12OutSel = FCP_CHROMA12_OUT_CHROMA;

        #if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if ((outId == VHWA_M2M_VISS_OUT_YUV420_8B_IDX) ||
                    (outId == VHWA_M2M_VISS_OUT_CHROMA_8B_IDX))
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV12;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP2_UV12;
                }
            }
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if ((outId == VHWA_M2M_VISS_OUT_YUV420_8B_IDX) ||
                    (outId == VHWA_M2M_VISS_OUT_CHROMA_8B_IDX))
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV12;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP_UV12;
                }
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
        }
      }
        if (FVID2_DF_CHROMA_ONLY == outFmt->dataFormat)
        {
            fcpOutCfg->chromaMode = vsPrms->fcp0ChromaMode;
            #if defined VHWA_VPAC_IP_REV_VPAC3
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The negative test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
                /* LDRA_JUSTIFY_END */
                {
                    fcpOutCfg->chromaMode = vsPrms->fcp1ChromaMode;
                }
            #endif
        }
        else
        {
            fcpOutCfg->chromaMode = FCP_CHROMA_MODE_420;
        }
    }

    return (status);
}

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
static int32_t vhwaM2mVissInitYuv422SpOutChPrms(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId,
    uint32_t mode)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    uint32_t                sl2Size;
    uint32_t                blanking, thr;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    Vhwa_M2mVissChParams   *chPrmsLm = NULL;
    Vhwa_M2mVissChParams   *chPrmsCh = NULL;
    CslFcp_OutputSelect    *fcpOutCfg = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    sl2Prms  = &instObj->sl2AllocPrms;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
    if (mode == (VHWA_M2M_VISS_LUMA_MODE | VHWA_M2M_VISS_CHROMA_MODE))
/* LDRA_JUSTIFY_END */
    {
        chPrmsLm = &hObj->outChPrms[outId];
        chPrmsCh = &hObj->outChPrms[outId + 1u];
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
    else if ((mode & VHWA_M2M_VISS_LUMA_MODE) == VHWA_M2M_VISS_LUMA_MODE)
    {
        chPrmsLm = &hObj->outChPrms[outId];
    }
    else
    {
        chPrmsCh = &hObj->outChPrms[outId];
    }
/* LDRA_JUSTIFY_END */

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    fcpOutCfg = &hObj->fcpOutCfg;
    blanking = hObj->maxLseVertBlanking;
    thr = hObj->thr;

#if defined VHWA_VPAC_IP_REV_VPAC3
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        fcpOutCfg = &hObj->fcp2OutCfg;
        blanking = hObj->maxLseVertBlankingMV;
        thr = hObj->thr;
    }
#endif

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
    if ((mode & VHWA_M2M_VISS_LUMA_MODE) == VHWA_M2M_VISS_LUMA_MODE)
/* LDRA_JUSTIFY_END */
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
      if(chPrmsLm != NULL)
/* LDRA_JUSTIFY_END */
      {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        if ((uint32_t)UFALSE == chPrmsLm->isEnabled)
/* LDRA_JUSTIFY_END */
        {
            /* Initialize Channel Parameter for Luma output channel  */
            chPrmsLm->isEnabled    = (uint32_t)UTRUE;
            chPrmsLm->maxVertBlanking = blanking;
            chPrmsLm->thr             = thr;

            chPrmsLm->widthInBytes = Vhwa_calcHorzSizeInBytes(fmt->width,
                outFmt->ccsFormat);
            chPrmsLm->width        = fmt->width;
            chPrmsLm->height       = fmt->height;
            chPrmsLm->ccsf         =
                (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
            chPrmsLm->dataFmt      = (uint32_t)outFmt->dataFormat;
            chPrmsLm->imgPitch     = outFmt->pitch[0U];
            chPrmsLm->sl2Pitch     = Vhwa_calcSl2Pitch(chPrmsLm->widthInBytes);
            chPrmsLm->sl2Depth     = sl2Prms->outDepth[outId];
            chPrmsLm->isLumaCh     = (uint32_t)UTRUE;
            chPrmsLm->frmIdx       = outId;
            chPrmsLm->frmAddrIdx   = 0U;

            chPrmsLm->sl2Addr      = sl2Addr;

             /* For Luma channel, extra 2 lines of blanking required if
             * edge enhancer is enabled */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion.
            Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
            A test case will be added to cover this gap in a future release.
            <justification end> */
            if ((FVID2_CCSF_BITS8_PACKED == chPrmsLm->ccsf) &&
                (((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP0_LUMA8))
                  && (VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline))
                 || ((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP1_LUMA8))
                  && (VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline))))
            {
                chPrmsLm->extraBlanking = 0x2u;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion.
            Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
            A test case will be added to cover this gap in a future release.
            <justification end> */
            else if (((FVID2_CCSF_BITS12_UNPACKED16 == chPrmsLm->ccsf) ||
                      (FVID2_CCSF_BITS12_PACKED == chPrmsLm->ccsf)) &&
                (((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP0_LUMA12))
                  && (VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline))
                 || ((0U != (vsPrms->edgeEnhancerMode & VHWA_M2M_VISS_EE_ON_FCP1_LUMA12))
                  && (VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline))))
            /* LDRA_JUSTIFY_END */
            {
                chPrmsLm->extraBlanking = 0x2u;
            }
            else
            {
                chPrmsLm->extraBlanking = 0x0u;
            }

            sl2Size             = (chPrmsLm->sl2Depth * chPrmsLm->sl2Pitch);
            /* Increment the total Sl2 size used by this channel */
            hObj->totalSl2Size  = hObj->totalSl2Size + sl2Size;
            /* Move sl2 start address by required sl2 size */
            sl2Addr             = sl2Addr + sl2Size;

            /* Pitch must be atleast same as widthInBytes */
            if (outFmt->pitch[0U] < chPrmsLm->widthInBytes)
            {
                status = FVID2_EINVALID_PARAMS;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            status = FVID2_EINVALID_PARAMS;
        }
/* LDRA_JUSTIFY_END */

        if(FVID2_CCSF_BITS8_PACKED == chPrmsLm->ccsf)
        {
            fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_RGB2YUV_Y8;
#if defined VHWA_VPAC_IP_REV_VPAC3
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y8;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP2_Y8;
                }
            }
            /* LDRA_JUSTIFY_END */
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            /* LDRA_JUSTIFY_END */
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y8;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP_Y8;
                }
            }
#endif
        }
        else
        {
            /* FCP output mode is now YUV420 12bit output */
            fcpOutCfg->luma12OutSel = FCP_LUMA12_OUT_RGB2YUV;
#if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP2_Y12;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP2_Y12;
                }
            }
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.luma8OutSel = VISS_OUT_FCP_Y12;
                }
                else
                {
                    hObj->vissTopOutMux.luma12OutSel = VISS_OUT_FCP_Y12;
                }
            }
#endif
        }
      }
    }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
    if ((mode & VHWA_M2M_VISS_CHROMA_MODE) == VHWA_M2M_VISS_CHROMA_MODE)
/* LDRA_JUSTIFY_END */
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The test framework and test apps cannot reach this portion. 
chPrmsCh is passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to chPrmsCh, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
      if(chPrmsCh != NULL)
/* LDRA_JUSTIFY_END */
      {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        if ((uint32_t)UFALSE == chPrmsCh->isEnabled)
/* LDRA_JUSTIFY_END */
        {
            /* Initialiaze Channel Parameter for Chroma output channel  */
            chPrmsCh->isEnabled    = (uint32_t)UTRUE;
            chPrmsCh->maxVertBlanking = blanking;
            chPrmsCh->thr             = thr;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            if (NULL != chPrmsLm)
/* LDRA_JUSTIFY_END */
            {
                chPrmsCh->widthInBytes = chPrmsLm->widthInBytes;
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            else
            {
                chPrmsCh->widthInBytes = Vhwa_calcHorzSizeInBytes(fmt->width,
                                           outFmt->ccsFormat);
            }
/* LDRA_JUSTIFY_END */
            chPrmsCh->width        = fmt->width;
            chPrmsCh->height       = fmt->height;
            chPrmsCh->ccsf         =
                (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
            chPrmsCh->dataFmt      = outFmt->dataFormat;

            /* Pitch must be atleast same as widthInBytes */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The negative test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            if (FVID2_DF_CHROMA_ONLY == outFmt->dataFormat)
            {
                chPrmsCh->imgPitch = outFmt->pitch[0U];
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                chPrmsCh->imgPitch = outFmt->pitch[1U];
            }
			
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            if (NULL != chPrmsLm)
/* LDRA_JUSTIFY_END */
            {
                chPrmsCh->sl2Pitch     = chPrmsLm->sl2Pitch;
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            else
            {
                chPrmsCh->sl2Pitch     = Vhwa_calcSl2Pitch(chPrmsCh->widthInBytes);
            }
/* LDRA_JUSTIFY_END */
            /* Luma and chroma have same depth in SL2 */
            chPrmsCh->sl2Depth     = sl2Prms->outDepth[outId];
            /* Extra blanking required for the chroma channel */
            chPrmsCh->extraBlanking = 0;
            chPrmsCh->isLumaCh     = UFALSE;
            chPrmsCh->dataFmt      = (uint32_t)outFmt->dataFormat;
            chPrmsCh->frmIdx       = outId;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            if (NULL != chPrmsLm)
/* LDRA_JUSTIFY_END */
            {
                chPrmsCh->frmAddrIdx   = 1U;
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            else
            {
                chPrmsCh->frmAddrIdx   = 0U;
            }
/* LDRA_JUSTIFY_END */
            chPrmsCh->sl2Addr      = sl2Addr;

            sl2Size                =
                (chPrmsCh->sl2Depth * chPrmsCh->sl2Pitch);

            /* Increment the total Sl2 size used by this channel */
            hObj->totalSl2Size = hObj->totalSl2Size + sl2Size;

            /* Pitch must be atleast same as widthInBytes */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            if (FVID2_DF_CHROMA_ONLY == outFmt->dataFormat)
            {
                if (outFmt->pitch[0U] < chPrmsCh->widthInBytes)
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                if (outFmt->pitch[1U] < chPrmsCh->widthInBytes)
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            status = FVID2_EINVALID_PARAMS;
        }
/* LDRA_JUSTIFY_END */

        if(FVID2_CCSF_BITS8_PACKED == chPrmsCh->ccsf)
        {
            fcpOutCfg->chroma8OutSel = FCP_CHROMA8_OUT_CHROMA;
#if defined VHWA_VPAC_IP_REV_VPAC3
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV8;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP2_UV8;
                }
            }
            /* LDRA_JUSTIFY_END */
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            /* LDRA_JUSTIFY_END */
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV8;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP_UV8;
                }
            }
#endif
        }
        else
        {
            fcpOutCfg->chroma12OutSel = FCP_CHROMA12_OUT_CHROMA;
#if defined VHWA_VPAC_IP_REV_VPAC3
            if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP2_UV12;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP2_UV12;
                }
            }
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
            {
                if (VHWA_M2M_VISS_OUT_YUV422_8B_IDX == outId)
                {
                    hObj->vissTopOutMux.chroma8OutSel = VISS_OUT_FCP_UV12;
                }
                else
                {
                    hObj->vissTopOutMux.chroma12OutSel = VISS_OUT_FCP_UV12;
                }
            }
#endif
        }
      }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: Unused feature, cannot be enabled, control cannot reach here.
        <justification end> */
        if (FVID2_DF_CHROMA_ONLY == outFmt->dataFormat)
        {
            fcpOutCfg->chromaMode = vsPrms->fcp0ChromaMode;
            #if defined VHWA_VPAC_IP_REV_VPAC3
                if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
                {
                    fcpOutCfg->chromaMode = vsPrms->fcp1ChromaMode;
                }
            #endif
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            fcpOutCfg->chromaMode = FCP_CHROMA_MODE_422;
        }
    }

    return (status);
}
#endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */

static int32_t vhwaM2mVissInitOutChPrmsForSinglePlane(
    Vhwa_M2mVissInstObj *instObj, Vhwa_M2mVissHandleObj *hObj,
    Vhwa_M2mVissParams *vsPrms, uint32_t outId)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    CslFcp_OutputSelect    *fcpOutCfg = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    chPrms  = &hObj->outChPrms[outId];
    sl2Prms = &instObj->sl2AllocPrms;

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

    fcpOutCfg = &hObj->fcpOutCfg;
    chPrms->maxVertBlanking = hObj->maxLseVertBlanking;
    chPrms->thr = hObj->thr;

#if defined VHWA_VPAC_IP_REV_VPAC3
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        fcpOutCfg = &hObj->fcp2OutCfg;
        chPrms->maxVertBlanking = hObj->maxLseVertBlankingMV;
        chPrms->thr = hObj->thr;
    }
#endif

    /* Set params only if output index is correct and channel is
     * not already enabled */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    if ((uint32_t)UFALSE == chPrms->isEnabled)
/* LDRA_JUSTIFY_END */
    {
        chPrms->isEnabled      = (uint32_t)UTRUE;
        chPrms->widthInBytes    = Vhwa_calcHorzSizeInBytes(fmt->width,
            outFmt->ccsFormat);
        chPrms->width           = fmt->width;
        chPrms->height          = fmt->height;
        chPrms->ccsf            = (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
        chPrms->dataFmt         = outFmt->dataFormat;
        chPrms->imgPitch        = outFmt->pitch[0U];
        chPrms->sl2Pitch        = Vhwa_calcSl2Pitch(chPrms->widthInBytes);
        chPrms->sl2Depth        = sl2Prms->outDepth[outId];
        chPrms->extraBlanking   = 0x0u;
        chPrms->isLumaCh        = (uint32_t)UFALSE;
        chPrms->frmIdx          = outId;
        chPrms->frmAddrIdx      = 0U;

        chPrms->sl2Addr         = sl2Addr;

        /* Increment the total Sl2 size used by this channel */
        hObj->totalSl2Size      = hObj->totalSl2Size +
            (chPrms->sl2Depth * chPrms->sl2Pitch);

        /* Pitch must be atleast same as widthInBytes */
        if (outFmt->pitch[0U] < chPrms->widthInBytes)
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    else
    {
        status = FVID2_EINVALID_PARAMS;
    }
/* LDRA_JUSTIFY_END */
    /* Select the FCP muxes accordingly */
    if (FVID2_SOK == status)
    {
        if (VHWA_M2M_VISS_OUT_GREY_12B_IDX == outId)
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
            if (FVID2_DF_GREY == outFmt->dataFormat)
/* LDRA_JUSTIFY_END */
            {
                /* Grey Output on Luma12 */
                fcpOutCfg->luma12OutSel = FCP_LUMA12_OUT_RGB2HSV;
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */

            else if (FVID2_DF_RAW == outFmt->dataFormat)
            {
                /* RAW Output on Luma12,
                 * TODO: add other options */
                fcpOutCfg->luma12OutSel = FCP_LUMA12_OUT_CFA_C1;
            }
            else
            {
                /* Nothing to do here */
            }
/* LDRA_JUSTIFY_END */
        }
        else if (VHWA_M2M_VISS_OUT_GREY_8B_IDX == outId)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if (FVID2_DF_GREY == outFmt->dataFormat)
            {
                /* Grey Output on Luma8 */
                fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_RGB2HSV_Y8;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if (FVID2_DF_RAW == outFmt->dataFormat)
            {
                /* RAW Output on Luma12,
                 * TODO: add other options */
                fcpOutCfg->luma8OutSel = FCP_LUMA8_OUT_CFA_C2;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else
            {
                /* Nothing to do here */
            }
            /* LDRA_JUSTIFY_END */
        }
        else if (VHWA_M2M_VISS_OUT_SATURA_8B_IDX == outId)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if (FVID2_DF_SATURATION == outFmt->dataFormat)
            /* LDRA_JUSTIFY_END */
            {
                /* Grey Output on Saturation */
                fcpOutCfg->sat8OutSel = FCP_SAT8_OUT_SATURATION;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if (FVID2_DF_RAW == outFmt->dataFormat)
            {
                /* RAW Output on Luma12,
                 * TODO: add other options */
                fcpOutCfg->sat8OutSel = FCP_SAT8_OUT_CFA_C4;
            }
            else
            {
                /* Nothing to do here */
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */
        else if (VHWA_M2M_VISS_OUT_CFA_C1_12B_IDX == outId)
        /* LDRA_JUSTIFY_END */
        {
            fcpOutCfg->chroma12OutSel = FCP_CHROMA12_OUT_C1;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach here.
        <justification end> */
        else if (VHWA_M2M_VISS_OUT_CFA_C3_12B_IDX == outId)
        {
            fcpOutCfg->chroma8OutSel = FCP_CHROMA8_OUT_CFA_C3;
        }
        else
        {
            /* Nothing to do here */
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}

static int32_t vhwaM2mVissInitOutChPrmsForH3a(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms, uint32_t outId)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    uint32_t                width;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    chPrms  = &hObj->outChPrms[outId];
    sl2Prms = &instObj->sl2AllocPrms;

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;

    /* Set params only if output index is correct and channel is
     * not already enabled,
     * Setting some default parameters here, it will be overwritten,
     * later on when actual h3a config is set and output size is known */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    if ((VHWA_M2M_VISS_OUT_H3A_IDX == outId) &&
        ((uint32_t)UFALSE == chPrms->isEnabled))
    {
/* LDRA_JUSTIFY_END */
        chPrms->isEnabled     = (uint32_t)UTRUE;
        width                 = Vhwa_calcSl2Pitch(sl2Prms->maxOutWidth[outId]);

        chPrms->widthInBytes  = width;
        chPrms->width         = chPrms->widthInBytes;
        chPrms->height        = 1;
        chPrms->ccsf          = FVID2_CCSF_BITS8_PACKED;
        chPrms->dataFmt       = (uint32_t)outFmt->dataFormat;
        chPrms->imgPitch      = chPrms->widthInBytes;
        chPrms->sl2Pitch      = chPrms->widthInBytes;
        chPrms->sl2Depth      = sl2Prms->outDepth[outId];
        chPrms->extraBlanking = 0x0u;
        chPrms->isLumaCh      = UFALSE;
        chPrms->frmIdx        = VHWA_M2M_VISS_OUT_H3A_IDX;
        chPrms->frmAddrIdx    = 0U;

        chPrms->sl2Addr       = sl2Addr;

        /* Increment the total Sl2 size used by this channel */
        hObj->totalSl2Size    = hObj->totalSl2Size +
            (chPrms->sl2Depth * chPrms->sl2Pitch);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    else
    {
        status = FVID2_EINVALID_PARAMS;
    }
/* LDRA_JUSTIFY_END */

    return (status);
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void Vhwa_m2mVissInitSl2Params.* <function end>
<justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    The control cannot reach here without allocating the sl2 in the current test apps and kernel, therefore it is not possible to cover. Cannot be removed since this is a fail safe mechanism.
    Effect on this unit: If the control reaches here, it will resolve the sl2 allocation issue and keep the functionality integrated.
<justification end> */
void Vhwa_m2mVissInitSl2Params(Vhwa_M2mVissSl2Params *sl2Prms)
{
    uint32_t cnt;

    if (NULL != sl2Prms)
    {
        sl2Prms->inCcsf = FVID2_CCSF_BITS12_UNPACKED16;
        sl2Prms->inDepth = VHWA_M2M_VISS_MIN_INPUT_DEPTH;
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt ++)
        {
            sl2Prms->maxInWidth[cnt] = 1920U;
        }

        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
        {
            if (VHWA_M2M_VISS_OUT_H3A_IDX == cnt)
            {
                sl2Prms->maxOutWidth[cnt] = 1024U;
            }
            else
            {
                sl2Prms->maxOutWidth[cnt] = 1920U;
            }

            if (cnt < VHWA_M2M_VISS_OUT_YUV420_8B_IDX)
            {
                sl2Prms->outCcsf[cnt] = FVID2_CCSF_BITS12_UNPACKED16;
            }
            else
            {
                sl2Prms->outCcsf[cnt] = FVID2_CCSF_BITS8_PACKED;
            }
            sl2Prms->outDepth[cnt] = VHWA_M2M_VISS_MIN_OUTPUT_DEPTH;
        }
    }
}

#if defined (VHWA_VPAC_IP_REV_VPAC3L)
static int32_t vhwaM2mVissInitOutChPrmsForIrOutput(
    Vhwa_M2mVissInstObj *instObj, Vhwa_M2mVissHandleObj *hObj,
    Vhwa_M2mVissParams *vsPrms, uint32_t outId)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    uint32_t               *opSelect = NULL;
    uint32_t               *opSelect2 = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    chPrms  = &hObj->outChPrms[outId];
    sl2Prms = &instObj->sl2AllocPrms;

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
    if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
    {
        chPrms->maxVertBlankingIR = hObj->maxLseVertBlankingIR;
        chPrms->thr = hObj->thr;
    }
#endif

    /* Set params only if output index is correct and channel is
     * not already enabled */
    if ((uint32_t)FALSE == chPrms->isEnabled)
    {
        chPrms->isEnabled      = (uint32_t)TRUE;
        chPrms->widthInBytes    = Vhwa_calcHorzSizeInBytes(fmt->width,
            outFmt->ccsFormat);
        chPrms->width           = fmt->width;
        chPrms->height          = fmt->height;
        chPrms->ccsf            = (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
        chPrms->dataFmt         = outFmt->dataFormat;
        chPrms->imgPitch        = outFmt->pitch[0U];
        chPrms->sl2Pitch        = Vhwa_calcSl2Pitch(chPrms->widthInBytes);
        chPrms->sl2Depth        = sl2Prms->outDepth[outId];
        chPrms->extraBlanking   = 0x00u;
        chPrms->isLumaCh        = (uint32_t)FALSE;
        chPrms->frmIdx          = outId;
        chPrms->frmAddrIdx      = 0U;

        chPrms->sl2Addr         = sl2Addr;

        /* Increment the total Sl2 size used by this channel */
        hObj->totalSl2Size      = hObj->totalSl2Size +
            (chPrms->sl2Depth * chPrms->sl2Pitch);

        /* Pitch must be atleast same as widthInBytes */
        if (outFmt->pitch[0U] < chPrms->widthInBytes)
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    else
    {
        status = FVID2_EINVALID_PARAMS;
    }
/* LDRA_JUSTIFY_END */

    /* Select the FCP muxes accordingly */
    if (FVID2_SOK == status)
    {
        switch (outId)
        {
            case 0:
                opSelect = &hObj->vissTopOutMux.luma12OutSel;
                opSelect2 = &hObj->vissTopOutMux.chroma12OutSel;
            break;
            case 1:
                opSelect = &hObj->vissTopOutMux.chroma12OutSel;
            break;
            case 2:
                opSelect = &hObj->vissTopOutMux.luma8OutSel;
                opSelect2 = &hObj->vissTopOutMux.chroma8OutSel;
            break;
            case 3:
                opSelect = &hObj->vissTopOutMux.chroma8OutSel;
            break;
            case 4:
                opSelect = &hObj->vissTopOutMux.sat8OutSel;
            break;
            default :
                status = FVID2_EINVALID_PARAMS;
                GT_1trace(VhwaVissTrace, GT_ERR,
                                "Illegal output channel no %d !!\n", outId);
            break;
        }
        if (FVID2_SOK == status)
        {
            //if (VISS_OUT_INVALID == *opSelect)
            {
                if((FVID2_DF_RAW08 == outFmt->dataFormat))
                {
                    *opSelect = VISS_OUT_IR_8;
                }
                else if((FVID2_DF_RAW12 == outFmt->dataFormat))
                {
                    *opSelect = VISS_OUT_IR_12;
                }
                else if(FVID2_DF_RAW16 == outFmt->dataFormat)
                {
                    *opSelect = VISS_OUT_IR_16;
                    *opSelect2 = VISS_OUT_IR_16;
                }
                else
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
            #if 0
            else
            {
                status = FVID2_EINVALID_PARAMS;
                GT_1trace(VhwaVissTrace, GT_ERR,
                                "IR Channel is being used %d !!\n", outId);
            }
            #endif
        }
    }

    return (status);
}

/*TODO: Remove this hack - Configures IR downstream only on Y12 channel*/
static int32_t vhwaM2mVissInitOutChPrmsForIrOutputOnBayer(
    Vhwa_M2mVissInstObj *instObj, Vhwa_M2mVissHandleObj *hObj,
    Vhwa_M2mVissParams *vsPrms, uint32_t outId)
{
    int32_t                 status = FVID2_SOK;
    uint64_t                sl2Addr;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Fvid2_Format           *fmt = NULL;
    Fvid2_Format           *outFmt = NULL;
    Vhwa_M2mVissSl2Params  *sl2Prms = NULL;
    uint32_t               *opSelect = NULL;

    /* Check for Null pointer,
     * Internal function so checking with assert */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));
    GT_assert(VhwaVissTrace, (outId < VHWA_M2M_VISS_MAX_OUTPUTS));

    chPrms  = &hObj->outChPrms[outId];
    sl2Prms = &instObj->sl2AllocPrms;

    /* Since VISS does not change size of the frame, frame
     * size is taken from input frame format, where as storage
     * format and pitch is taken from the output format */
    fmt    = &vsPrms->inFmt;
    outFmt = &vsPrms->outPrms[outId].fmt;

    /* Calculate the Sl2 Start address for this channel,
     * based on the total size used till now by this handle */
    sl2Addr = instObj->sl2Addr;
    sl2Addr = sl2Addr + hObj->totalSl2Size;
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
    if(VHWA_VISS_PIPE_HV == vsPrms->outPrms[outId].vPipeline)
    {
        chPrms->maxVertBlanking = hObj->maxLseVertBlanking;
        chPrms->thr = hObj->thr;
    }
#endif
#if defined VHWA_VPAC_IP_REV_VPAC3
    if(VHWA_VISS_PIPE_MV == vsPrms->outPrms[outId].vPipeline)
    {
        chPrms->maxVertBlanking = hObj->maxLseVertBlankingMV;
        chPrms->thr = hObj->thr;
    }
#endif

    /* Set params only if output index is correct and channel is
     * not already enabled */
    if ((uint32_t)FALSE == chPrms->isEnabled)
    {
        chPrms->isEnabled      = (uint32_t)TRUE;
        chPrms->widthInBytes    = Vhwa_calcHorzSizeInBytes(fmt->width,
            outFmt->ccsFormat);
        chPrms->width           = fmt->width;
        chPrms->height          = fmt->height;
        chPrms->ccsf            = (Fvid2_ColorCompStorageFmt)outFmt->ccsFormat;
        chPrms->dataFmt         = outFmt->dataFormat;
        chPrms->imgPitch        = outFmt->pitch[0U];
        chPrms->sl2Pitch        = Vhwa_calcSl2Pitch(chPrms->widthInBytes);
        chPrms->sl2Depth        = sl2Prms->outDepth[outId];
        chPrms->extraBlanking   = 0x00u;
        chPrms->isLumaCh        = (uint32_t)FALSE;
        chPrms->frmIdx          = outId;
        chPrms->frmAddrIdx      = 0U;

        chPrms->sl2Addr         = sl2Addr;

        /* Increment the total Sl2 size used by this channel */
        hObj->totalSl2Size      = hObj->totalSl2Size +
            (chPrms->sl2Depth * chPrms->sl2Pitch);

        /* Pitch must be atleast same as widthInBytes */
        if (outFmt->pitch[0U] < chPrms->widthInBytes)
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }
    //REVIEW
    /* LDRA_JUSTIFY_START_DISABLED
    <metric start> statement branch <metric end>
    <justification start>
    Rationale:
    Effect on this unit:
    <justification end> */
    else
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END_DISABLED */

    /* Select the FCP muxes accordingly */
    if (FVID2_SOK == status)
    {
        switch (outId)
        {
            case 0:
                opSelect = &hObj->vissTopOutMux.luma12OutSel;
            break;
            case 1:
                opSelect = &hObj->vissTopOutMux.chroma12OutSel;
            break;
            case 2:
                opSelect = &hObj->vissTopOutMux.luma8OutSel;
            break;
            case 3:
                opSelect = &hObj->vissTopOutMux.chroma8OutSel;
            break;
            case 4:
                opSelect = &hObj->vissTopOutMux.sat8OutSel;
            break;
            default :
                status = FVID2_EINVALID_PARAMS;
                GT_1trace(VhwaVissTrace, GT_ERR,
                                "Illegal output channel no %d !!\n", outId);
            break;
        }
        if (FVID2_SOK == status)
        {
            if (VISS_OUT_INVALID == *opSelect)
            {
                *opSelect = 0;
            }
            else
            {
                status = FVID2_EINVALID_PARAMS;
                GT_1trace(VhwaVissTrace, GT_ERR,
                                "IR Channel is being used %d !!\n", outId);
            }
        }
    }

    return (status);
}
#endif
