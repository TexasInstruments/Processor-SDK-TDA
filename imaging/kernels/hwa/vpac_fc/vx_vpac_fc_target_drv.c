/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <vx_vpac_fc_target_priv.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status tivxVpacFcVissSetRfeConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacFcVissSetGlbceConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacFcVissSetNsf4Config(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacFcVissSetFcpConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef, uint32_t fcpInstance);
#if defined(VPAC3L)
static vx_status tivxVpacFcVissSetPcidConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef);
#endif

#if defined(VPAC3) || defined(VPAC3L)
static vx_status tivxVpacFcVissSetCacConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef);
#endif
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacFcVissSetConfigInDrv(tivxVpacFcObj *fcObj)
{
    vx_status status;
#if defined(VPAC3) || defined(VPAC3L)
    tivx_vpac_viss_params_t   *tivxVissPrms = NULL;
    tivx_vpac_fc_viss_msc_params_t   *fcPrms = NULL;
#endif

    status = tivxVpacFcVissSetRfeConfig(fcObj, &fcObj->vissObj.vissCfgRef);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxVpacFcVissSetGlbceConfig(fcObj, &fcObj->vissObj.vissCfgRef);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxVpacFcVissSetNsf4Config(fcObj, &fcObj->vissObj.vissCfgRef);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxVpacFcVissSetFcpConfig(fcObj, &fcObj->vissObj.vissCfgRef, 0);
#if defined(VPAC3)
        fcPrms = (tivx_vpac_fc_viss_msc_params_t *) fcObj->fc_prms_target_ptr;
        tivxVissPrms = &fcPrms->tivxVissPrms;
        if(TIVX_VPAC_VISS_FCP1_DISABLED != tivxVissPrms->fcp1_config)
        {
            status = tivxVpacFcVissSetFcpConfig(fcObj, &fcObj->vissObj.vissCfgRef, 1);
        }
#endif
    }
#if defined(VPAC3) || defined(VPAC3L)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        fcPrms = (tivx_vpac_fc_viss_msc_params_t *) fcObj->fc_prms_target_ptr;
        tivxVissPrms = &fcPrms->tivxVissPrms;
        if (0U == tivxVissPrms->bypass_cac)
        {
            status = tivxVpacFcVissSetCacConfig(fcObj, &fcObj->vissObj.vissCfgRef);
        }
    }
#endif

#if defined(VPAC3L)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxVpacFcVissSetPcidConfig(fcObj, &fcObj->vissObj.vissCfgRef);
    }
#endif

    return (status);
}

vx_status tivxVpacFcVissSetConfigBuffer(tivxVpacFcObj *fcObj)
{
    vx_status status =      (vx_status) VX_SUCCESS;

    int32_t fvid2_status =  FVID2_SOK;

    /* Initialize buffer pointer to NULL */
    fcObj->vissObj.configurationBuffer.bufferPtr = NULL;
    /* Initialize length to zero */
    fcObj->vissObj.configurationBuffer.length = 0U;
    /* Initialize configThroughUDMA to false */
    fcObj->vissObj.configurationBuffer.configThroughUdmaFlag = false;

    /* Get configuration buffer information */
    fvid2_status = Fvid2_control(fcObj->handle,
    IOCTL_VHWA_M2M_VISS_GET_BUFF_INFO, (void*) &fcObj->vissObj.configurationBuffer,
            NULL);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to print the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK != fvid2_status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get config buff info from driver\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to proceed with successful configuration.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == fvid2_status)
    /* LDRA_JUSTIFY_END */
    {
        /* configThroughUdmaFlag is true, allocate config buffer */
        if (true == fcObj->vissObj.configurationBuffer.configThroughUdmaFlag)
        {
            /* Indicate that we are allocating out of memory that is cache write through policy */
            fcObj->vissObj.configurationBuffer.isCacheWriteBackPolicy = false;

            /* allocate buffer for the length requested */
            fcObj->vissObj.configurationBuffer.bufferPtr = (uint32_t*) tivxMemAlloc(
                    fcObj->vissObj.configurationBuffer.length,
                    (vx_enum) TIVX_MEM_EXTERNAL_CACHEABLE_WT);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == fcObj->vissObj.configurationBuffer.bufferPtr)
            {
                VX_PRINT(VX_ZONE_ERROR, "failed to allocate %d bytes !!!\n",
                        fcObj->vissObj.configurationBuffer.length);
                fvid2_status = FVID2_EALLOC;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* Set configuration buffer information */
                fvid2_status = Fvid2_control(fcObj->handle,
                IOCTL_VHWA_M2M_VISS_SET_BUFF_INFO,
                        (void*) &fcObj->vissObj.configurationBuffer, NULL);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to proceed with successful configuration.
                However, due to the stated rationale, this is not tested.
                <justification end> */                      
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                            "Failed to set config buff info into driver\n");
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to proceed with successful configuration.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status) VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = (vx_status) VX_SUCCESS;
    }

    return (status);
}

void tivxVpacFcVissDeleteConfigBuffer(tivxVpacFcObj *fcObj)
{
    if (NULL != fcObj->vissObj.configurationBuffer.bufferPtr)
    {
        vx_status free_status = tivxMemFree((void*) fcObj->vissObj.configurationBuffer.bufferPtr,
                fcObj->vissObj.configurationBuffer.length,
                (vx_enum) TIVX_MEM_EXTERNAL_CACHEABLE_WT);

        if ((vx_status)VX_SUCCESS != free_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to free memory\n");
        }
    }
}

static vx_status tivxVpacFcVissSetRfeConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Rfe_Control         rfeCtrl;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != vissCfgRef->lPwlCfg)
    /* LDRA_JUSTIFY_END */
    {
        /* PWL for Long Input */
        rfeCtrl.module  = RFE_MODULE_PWL1;
        rfeCtrl.pwl1Cfg = vissCfgRef->lPwlCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL1 Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        /* Reset Pointer */
        vissCfgRef->lPwlCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((NULL != vissCfgRef->sPwlCfg) && (FVID2_SOK == fvid2_status))
    /* LDRA_JUSTIFY_END */
    {
        /* PWL for Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL2;
        rfeCtrl.pwl2Cfg = vissCfgRef->sPwlCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL2 Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        /* Reset Pointer */
        vissCfgRef->sPwlCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((NULL != vissCfgRef->vsPwlCfg) && (FVID2_SOK == fvid2_status))
    /* LDRA_JUSTIFY_END */
    {
        /* PWL for Very Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL3;
        rfeCtrl.pwl3Cfg = vissCfgRef->vsPwlCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL3 Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->vsPwlCfg = NULL;
    }

    if ((NULL != vissCfgRef->lLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL Lut for Long Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT1;
        rfeCtrl.decomp1Cfg      = vissCfgRef->lLutCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL1_LUT Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->lLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->sLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL Lut for Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT2;
        rfeCtrl.decomp2Cfg      = vissCfgRef->sLutCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL2_LUT Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->sLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->vsLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL Lut for Very Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT3;
        rfeCtrl.decomp3Cfg      = vissCfgRef->vsLutCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL3_LUT Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->vsLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->wdr1Cfg) && (FVID2_SOK == fvid2_status))
    {
        /* WDR Merge 1 block configuration */
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA1;
        rfeCtrl.wdrMergeMa1  = vissCfgRef->wdr1Cfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set WDR_MERGE1 Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->wdr1Cfg = NULL;
    }

    if ((NULL != vissCfgRef->wdr2Cfg) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA2;
        rfeCtrl.wdrMergeMa2  = vissCfgRef->wdr2Cfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set WDR_MERGE2 Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->wdr2Cfg = NULL;
    }

    if ((NULL != vissCfgRef->comp20To16LutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* Set Companding Lut to convert from 20 to 16bits */
        rfeCtrl.module       = RFE_MODULE_COMP_LUT;
        rfeCtrl.compCfg      = vissCfgRef->comp20To16LutCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set COMP_LUT Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->comp20To16LutCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((NULL != vissCfgRef->dpcOtf) && (FVID2_SOK == fvid2_status))
    /* LDRA_JUSTIFY_END */
    {
        rfeCtrl.module       = RFE_MODULE_DPC_OTF;
        rfeCtrl.dpcOtfCfg    = vissCfgRef->dpcOtf;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set DPC_OTF Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->dpcOtf = NULL;
    }

    if ((NULL != vissCfgRef->dpcLut) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_DPC_LUT;
        rfeCtrl.dpcLutCfg    = vissCfgRef->dpcLut;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */         
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set DPC_LUT Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->dpcLut = NULL;
    }

    if ((NULL != vissCfgRef->lscCfg) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module      = RFE_MODULE_LSC;
        rfeCtrl.lscConfig   = vissCfgRef->lscCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set LSC Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->lscCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((NULL != vissCfgRef->wbCfg) &&(FVID2_SOK == fvid2_status))
    /* LDRA_JUSTIFY_END */
    {
        rfeCtrl.module       = RFE_MODULE_GAIN_OFST;
        rfeCtrl.wbConfig     = vissCfgRef->wbCfg;
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set WhiteBalance Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->wbCfg = NULL;
    }

    if ((NULL != vissCfgRef->rfeH3aInCfg) && (FVID2_SOK == fvid2_status))
    {
        /* H3A Input Selection and Lut configuration */
        rfeCtrl.module   = RFE_MODULE_H3A;
        rfeCtrl.h3aInCfg = vissCfgRef->rfeH3aInCfg;

        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set H3A Input Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->rfeH3aInCfg = NULL;
    }

    if ((NULL != vissCfgRef->h3aLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* H3A Input Selection and Lut configuration */
        rfeCtrl.module   = RFE_MODULE_H3A_LUT;
        rfeCtrl.h3aLutCfg = vissCfgRef->h3aLutCfg;

        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set H3A Lut Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->h3aLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->h3aCfg) && (FVID2_SOK == fvid2_status))
    {
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_H3A_SET_CONFIG,
            (void *)vissCfgRef->h3aCfg, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set H3A Config !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            fcObj->vissObj.h3a_output_size = vissCfgRef->h3aCfg->outputSize;
        }

        vissCfgRef->h3aCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacFcVissSetGlbceConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;
    Glbce_Control       glbceCtrl;

    vissDrvPrms = &fcObj->vissObj.vissPrms;

    /* GLBCE Parameters can be set only if it is enabled at SET_PARAMS time */
    if ((uint32_t)UTRUE == vissDrvPrms->enableGlbce)
    {
        if (NULL != vissCfgRef->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.glbceCfg = vissCfgRef->glbceCfg;
            fvid2_status = Fvid2_control(fcObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set GLBCE Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */
            vissCfgRef->glbceCfg = NULL;
        }

        if (NULL != vissCfgRef->fwdPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_FWD_PERCEPT;
            glbceCtrl.fwdPrcptCfg = vissCfgRef->fwdPrcpCfg;
            fvid2_status = Fvid2_control(fcObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set GLBCE_FWD_PERCEPT Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fwdPrcpCfg = NULL;
        }

        if (NULL != vissCfgRef->revPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_REV_PERCEPT;
            glbceCtrl.revPrcptCfg = vissCfgRef->revPrcpCfg;
            fvid2_status = Fvid2_control(fcObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set GLBCE_REV_PERCEPT Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->revPrcpCfg = NULL;
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
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacFcVissSetNsf4Config(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &fcObj->vissObj.vissPrms;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (((uint32_t)UTRUE == vissDrvPrms->enableNsf4) &&
        (NULL != vissCfgRef->nsf4Cfg))
    /* LDRA_JUSTIFY_END */
    {
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_NSF4_SET_CONFIG,
            (void *)vissCfgRef->nsf4Cfg, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set NFS4 Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->nsf4Cfg = NULL;
    }

    #if defined(VPAC3) || defined (VPAC3L)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */    
    if (NULL != fcObj->vissObj.raw_hist_out_target_ptr)
    /* LDRA_JUSTIFY_END */
    {
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_SET_RAW_HIST_APP_PTR,
            (void *)fcObj->vissObj.raw_hist_out_target_ptr, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */    
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Raw Histogram Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */
    }
    #endif

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacFcVissSetFcpConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef,
    uint32_t fcpInstance)
{
    int32_t             status = (vx_status)VX_SUCCESS;
    uint32_t            ioctl_set_config = IOCTL_FCP_SET_CONFIG;
    Fcp_Control         fcpCtrl;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;
    uint8_t EE_on_Y12 = 0U;
    uint8_t EE_on_Y8 = 0U;
    uint8_t outID = 0U;

    if(0U == fcpInstance)
    {
        ioctl_set_config = IOCTL_FCP_SET_CONFIG;
    }
    #if defined(VPAC3)
    else if(1U == fcpInstance)
    {
        ioctl_set_config = IOCTL_FCP2_SET_CONFIG;
    }
    #endif
    else
    {
        status = (int32_t)VX_ERROR_INVALID_PARAMETERS;
    }
    vissDrvPrms = &fcObj->vissObj.vissPrms;

    if(FVID2_SOK == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (NULL != vissCfgRef->fcpCfg[fcpInstance].cfaLut16to12Cfg)
        /* LDRA_JUSTIFY_END */
        {
            fcpCtrl.module = FCP_MODULE_COMPANDING;
            fcpCtrl.inComp = vissCfgRef->fcpCfg[fcpInstance].cfaLut16to12Cfg;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set COMP Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].cfaLut16to12Cfg = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].cfaCfg)
        {
            fcpCtrl.module          = FCP_MODULE_CFA;
            fcpCtrl.cfa             = vissCfgRef->fcpCfg[fcpInstance].cfaCfg;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set CFA Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].cfaCfg = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].ccm)
        {

            fcpCtrl.module      = FCP_MODULE_CCM;
            fcpCtrl.ccm         = vissCfgRef->fcpCfg[fcpInstance].ccm;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set CCM Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].ccm = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].gamma)
        {
            fcpCtrl.module = FCP_MODULE_GAMMA;
            fcpCtrl.gamma  = vissCfgRef->fcpCfg[fcpInstance].gamma;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set Gamma Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].gamma = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].rgb2Hsv)
        {
            fcpCtrl.module          = FCP_MODULE_RGB2HSV;
            fcpCtrl.rgb2Hsv         = vissCfgRef->fcpCfg[fcpInstance].rgb2Hsv;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set RGB2HSV Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].rgb2Hsv = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].rgb2yuv)
        {
            fcpCtrl.module          = FCP_MODULE_RGB2YUV;
            fcpCtrl.rgb2Yuv         = vissCfgRef->fcpCfg[fcpInstance].rgb2yuv;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set RGB2YUV Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].rgb2yuv = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].yuvSatLutCfg)
        {
            fcpCtrl.module                  = FCP_MODULE_YUV_SAT_LUT;
            fcpCtrl.yuvSatLut               = vissCfgRef->fcpCfg[fcpInstance].yuvSatLutCfg;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set YUV_LUT Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].yuvSatLutCfg = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].histCfg)
        {
            fcpCtrl.module                  = FCP_MODULE_HISTOGRAM;
            fcpCtrl.hist                    = vissCfgRef->fcpCfg[fcpInstance].histCfg;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set Histogram Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].histCfg = NULL;
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to perform error handling or configuration.
        However, due to the stated rationale, this is not tested. 
        <justification end> */
        if(fcpInstance == 0U)
        /* LDRA_JUSTIFY_END */
        {
            EE_on_Y12 = VHWA_M2M_VISS_EE_ON_FCP0_LUMA12;
            EE_on_Y8 = VHWA_M2M_VISS_EE_ON_FCP0_LUMA8;
        }
        #ifdef VPAC3
        else if(fcpInstance == 1U)
        {
            EE_on_Y12 = VHWA_M2M_VISS_EE_ON_FCP1_LUMA12;
            EE_on_Y8 = VHWA_M2M_VISS_EE_ON_FCP1_LUMA8;
        }
        else
        {
            /* Unsupported FCP instance */
        }
        if(vissDrvPrms->outPrms[2].vPipeline == fcpInstance)
        {
            outID = 2;
        }
        #else
        if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_12B_IDX].fmt.dataFormat) ||
                    (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_12B_IDX].fmt.dataFormat) )
        {
            outID = VHWA_M2M_VISS_OUT_YUV422_12B_IDX;
        }
        if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_8B_IDX].fmt.dataFormat) ||
                    (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_8B_IDX].fmt.dataFormat) )
        {
            outID = VHWA_M2M_VISS_OUT_YUV422_8B_IDX;
        }
        #endif

        if (((0U != (EE_on_Y12 & vissDrvPrms->edgeEnhancerMode)) ||
            (0U != (EE_on_Y8 & vissDrvPrms->edgeEnhancerMode))) &&
            (NULL != vissCfgRef->fcpCfg[fcpInstance].eeCfg))
        {
            if (0U != (EE_on_Y12 & vissDrvPrms->edgeEnhancerMode))
            {
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassY12 = UFALSE;
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassC12 = UTRUE;
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->eeForY12OrY8 = 0u;

                if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[outID].fmt.dataFormat) ||
                    (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[outID].fmt.dataFormat) )
                    {
                        vissCfgRef->fcpCfg[fcpInstance].eeCfg->alignY12withChroma = UTRUE;
                        vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassC12 = UFALSE;
                    }
            }
            else
            {
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassY12 = UTRUE;
            }

            if (0U != (EE_on_Y8 & vissDrvPrms->edgeEnhancerMode))
            {
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassY8 = UFALSE;
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassC8 = UTRUE;
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->eeForY12OrY8 = 1u;
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->leftShift = 2u;
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->rightShift = 2u;

                if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[outID].fmt.dataFormat) ||
                    (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[outID].fmt.dataFormat) )
                {
                    vissCfgRef->fcpCfg[fcpInstance].eeCfg->alignY8withChroma = UTRUE;
                    vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassC8 = UFALSE;
                }
            }
            else
            {
                vissCfgRef->fcpCfg[fcpInstance].eeCfg->bypassY8 = UTRUE;
            }

            fcpCtrl.module              = FCP_MODULE_EE;
            fcpCtrl.eeCfg               = vissCfgRef->fcpCfg[fcpInstance].eeCfg;
            status = Fvid2_control(fcObj->handle, ioctl_set_config,
                (void *)&fcpCtrl, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set YEE Config !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            vissCfgRef->fcpCfg[fcpInstance].eeCfg = NULL;
        }
    }
    return (status);
}

#if defined(VPAC3) || defined(VPAC3L)
static vx_status tivxVpacFcVissSetCacConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &fcObj->vissObj.vissPrms;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (((uint32_t)UTRUE == vissDrvPrms->enableCac) &&
    /* LDRA_JUSTIFY_END */
        (NULL != vissCfgRef->cacCfg))
    {
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_CAC_SET_CONFIG,
            (void *)vissCfgRef->cacCfg, NULL);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set CAC Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->cacCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}
#endif

#if defined(VPAC3L)
static vx_status tivxVpacFcVissSetPcidConfig(tivxVpacFcObj *fcObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &fcObj->vissObj.vissPrms;

    if (((uint32_t)UTRUE == (uint32_t)vissDrvPrms->enablePcid) &&
        (NULL != vissCfgRef->pcidCfg))
    {
        fvid2_status = Fvid2_control(fcObj->handle, IOCTL_PCID_SET_CONFIG,
            (void *)vissCfgRef->pcidCfg, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PCID Config !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        vissCfgRef->pcidCfg = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}
#endif


vx_status tivxVpacFcMscScaleSetCoeffsCmd(tivxVpacFcObj *fcObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj, const tivx_obj_desc_user_data_object_t *usr_data_obj2)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          cnt;
    tivx_vpac_msc_coefficients_t     *coeffs = NULL;
    uint32_t                         *msc_inst_id = NULL;
    void                             *target_ptr;
    Msc_Coeff                        *coeffCfg = NULL;
    tivxVpacMscScaleObj              *msc_obj = NULL;
    tivxVpacMscArgs                  msc_args;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != usr_data_obj)
    /* LDRA_JUSTIFY_END */
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_coefficients_t) ==
                usr_data_obj->mem_size)
        {
            msc_obj = &fcObj->msc_obj;            

            coeffs = (tivx_vpac_msc_coefficients_t *)target_ptr;
            coeffCfg = &msc_obj->coeffCfg;

            Msc_coeffInit(coeffCfg);

            for (cnt = 0u; cnt < MSC_MAX_SP_COEFF_SET; cnt ++)
            {
                coeffCfg->spCoeffSet[cnt] = &coeffs->single_phase[cnt][0u];
            }

            for (cnt = 0u; cnt < MSC_MAX_MP_COEFF_SET; cnt ++)
            {
                coeffCfg->mpCoeffSet[cnt] = &coeffs->multi_phase[cnt][0u];
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != usr_data_obj2)
    /* LDRA_JUSTIFY_END */
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj2->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj2->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(uint32_t) ==
                usr_data_obj2->mem_size)
        {
            msc_inst_id = (uint32_t *)target_ptr;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object_2 Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj2->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object_2 is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != msc_inst_id)
        {
            if (VPAC_MSC_INST_ID_0 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_0;
            }
            else if (VPAC_MSC_INST_ID_1 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_1;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid MSC Instance ID\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        msc_args.mscArgs   = (Ptr) coeffCfg;
        fvid2_status = Fvid2_control(fcObj->handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
            &msc_args, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create coefficients\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Update Flexconnect config for VISS and MSC */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }

    return (status);
}

vx_status tivxVpacFcMscScaleSetOutputParamsCmd(tivxVpacFcObj *fcObj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[], uint16_t num_params)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, idx;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    uint32_t                         *msc_inst_id = NULL;
    void                             *target_ptr;
    Msc_ScConfig                     *sc_cfg = NULL;
    tivxVpacMscScaleObj              *msc_obj = NULL;
    tivxVpacMscArgs                  msc_args;
    Vhwa_M2mMscParams                *msc_prms = NULL;

    uint32_t loop_params = (num_params < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT) ? \
                            num_params : TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT;
    for (cnt = 0u; cnt < loop_params; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);
            msc_obj = &fcObj->msc_obj;            
            msc_prms = &msc_obj->msc_prms;

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            if (sizeof(tivx_vpac_msc_output_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_output_params_t *)target_ptr;

                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                tivxVpacMscScaleCopyOutPrmsToScCfg(sc_cfg, out_prms);

                msc_obj->user_init_phase_x[cnt] = out_prms->multi_phase.init_phase_x;
                msc_obj->user_init_phase_y[cnt] = out_prms->multi_phase.init_phase_y;
                msc_obj->user_offset_x[cnt] =     out_prms->offset_x;
                msc_obj->user_offset_y[cnt] =     out_prms->offset_y;

                tivxVpacMscScaleUpdateStartPhases(msc_obj, sc_cfg, cnt);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Mem Size for Output Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }
    if (NULL != usr_data_obj[10])
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[10]->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[10]->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        if (sizeof(uint32_t) ==
                usr_data_obj[10]->mem_size)
        {
            msc_inst_id = (uint32_t *)target_ptr;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object_10 Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[10]->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object_10 is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (NULL != msc_inst_id)
        /* LDRA_JUSTIFY_END */
        {
            if (VPAC_MSC_INST_ID_0 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_0;
            }
            else if (VPAC_MSC_INST_ID_1 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_1;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid MSC Instance ID\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            msc_args.mscArgs   = (Ptr) msc_prms;
            status = Fvid2_control(fcObj->handle,
                VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_args, NULL);

            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Set Output Params\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    /* Update Flexconnect config for VISS and MSC */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }
    return (status);
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> vx_status tivxVpacFcMscScaleSetInputParamsCmd.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
vx_status tivxVpacFcMscScaleSetInputParamsCmd(tivxVpacFcObj *fcObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj, const tivx_obj_desc_user_data_object_t *usr_data_obj2)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_msc_input_params_t     *in_prms = NULL;
    uint32_t                         *msc_inst_id = NULL;
    void                             *target_ptr;
    Vhwa_M2mMscParams                *msc_prms = NULL;
    tivxVpacMscScaleObj              *msc_obj = NULL;
    tivxVpacMscArgs                  msc_args;
    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_input_params_t) ==
                usr_data_obj->mem_size)
        {
            in_prms = (tivx_vpac_msc_input_params_t *)target_ptr;

            msc_obj = &fcObj->msc_obj;
            msc_obj->msc_prms.enableLineSkip = in_prms->src_ln_inc_2;
            msc_prms = &msc_obj->msc_prms;
            msc_obj->yc_mode = in_prms->yc_mode;
            msc_obj->enable_error_events = in_prms->enable_error_events;
            #if defined(VPAC3) || defined(VPAC3L)
            msc_obj->msc_prms.isEnableSimulProcessing = in_prms->is_enable_simul_processing;
            msc_obj->in_img0_yc_mode = in_prms->in_img0_yc_mode;
            msc_obj->in_img1_yc_mode = in_prms->in_img1_yc_mode;
            #endif
            tivxVpacMscScaleSetScParamsForEachLevel(msc_obj,msc_prms);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if (NULL != usr_data_obj2)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj2->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj2->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(uint32_t) ==
                usr_data_obj2->mem_size)
        {
            msc_inst_id = (uint32_t *)target_ptr;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object_2 Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj2->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object_2 is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != msc_inst_id)
        {
            if (VPAC_MSC_INST_ID_0 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_0;
            }
            else if (VPAC_MSC_INST_ID_1 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_1;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid MSC Instance ID\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            msc_args.mscArgs   = (Ptr) msc_prms;
            status = Fvid2_control(fcObj->handle,
                VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_args, NULL);

            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Set Params\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    /* Update Flexconnect config for VISS and MSC */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }
    return (status);
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> vx_status tivxVpacFcMscScaleSetCropParamsCmd.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
vx_status tivxVpacFcMscScaleSetCropParamsCmd(tivxVpacFcObj *fcObj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[], uint16_t num_params)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, idx;
    tivx_vpac_msc_crop_params_t      *out_prms = NULL;
    uint32_t                         *msc_inst_id = NULL;
    void                             *target_ptr;
    Msc_ScConfig                     *sc_cfg = NULL;
    tivxVpacMscScaleObj              *msc_obj = NULL;
    tivxVpacMscArgs                  msc_args;
    uint32_t loop_params = (num_params < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT) ? \
                            num_params : TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT;

    for (cnt = 0u; cnt < loop_params; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);
            msc_obj = &fcObj->msc_obj;

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            if (sizeof(tivx_vpac_msc_crop_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_crop_params_t *)target_ptr;

                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                sc_cfg->inRoi.cropStartX = out_prms->crop_start_x;
                sc_cfg->inRoi.cropStartY = out_prms->crop_start_y;
                sc_cfg->inRoi.cropWidth = out_prms->crop_width;
                sc_cfg->inRoi.cropHeight = out_prms->crop_height;

                msc_obj->user_crop_start_x[cnt] = out_prms->crop_start_x;
                msc_obj->user_crop_start_y[cnt] = out_prms->crop_start_y;

                tivxVpacMscScaleUpdateStartPhases(msc_obj, sc_cfg, cnt);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Mem Size for Crop Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    if (NULL != usr_data_obj[10])
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[10]->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[10]->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(uint32_t) ==
                usr_data_obj[10]->mem_size)
        {
            msc_inst_id = (uint32_t *)target_ptr;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object_10 Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[10]->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object_10 is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != msc_inst_id)
        {
            if (VPAC_MSC_INST_ID_0 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_0;
            }
            else if (VPAC_MSC_INST_ID_1 == *msc_inst_id)
            {
                msc_args.mscInstId = VPAC_MSC_INST_ID_1;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid MSC Instance ID\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            msc_args.mscArgs   = (Ptr) &msc_obj->msc_prms;
            status = (vx_status)Fvid2_control(fcObj->handle,
                VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_args, NULL);

            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Set Crop Params\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    /* Update Flexconnect config for VISS and MSC */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }

    return (status);
}
