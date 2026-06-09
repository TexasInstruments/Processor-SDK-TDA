/*
 *
 * Copyright (c) 2019-2021 Texas Instruments Incorporated
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

#include <vx_vpac_viss_target_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status tivxVpacVissSetRfeConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacVissSetGlbceConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacVissSetNsf4Config(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacVissSetFcpConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef, uint32_t fcpInstance);
#if defined(VPAC3L)
static vx_status tivxVpacVissSetPcidConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
#endif

#if defined(VPAC3) || defined(VPAC3L)
static vx_status tivxVpacVissSetCacConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
#endif
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissSetConfigInDrv(tivxVpacVissObj *vissObj)
{
    vx_status status;
#if defined(VPAC3) || defined(VPAC3L)
    tivx_vpac_viss_params_t   *vissPrms = NULL;
#endif

    status = tivxVpacVissSetRfeConfig(vissObj, &vissObj->vissCfgRef);

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
        status = tivxVpacVissSetGlbceConfig(vissObj, &vissObj->vissCfgRef);
    }

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
        status = tivxVpacVissSetNsf4Config(vissObj, &vissObj->vissCfgRef);
    }

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
        status = tivxVpacVissSetFcpConfig(vissObj, &vissObj->vissCfgRef, 0);
#if defined(VPAC3)
        vissPrms = (tivx_vpac_viss_params_t *) vissObj->viss_prms_target_ptr;
        if(TIVX_VPAC_VISS_FCP1_DISABLED != vissPrms->fcp1_config)
        {
            status = tivxVpacVissSetFcpConfig(vissObj, &vissObj->vissCfgRef, 1);
        }
#endif
    }
#if defined(VPAC3) || defined(VPAC3L)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    {
    /* LDRA_JUSTIFY_END */
        vissPrms = (tivx_vpac_viss_params_t *) vissObj->viss_prms_target_ptr;
        if (0U == vissPrms->bypass_cac)
        {
            status = tivxVpacVissSetCacConfig(vissObj, &vissObj->vissCfgRef);
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
        status = tivxVpacVissSetPcidConfig(vissObj, &vissObj->vissCfgRef);
    }
#endif

    return (status);
}

vx_status tivxVpacVissSetConfigBuffer(tivxVpacVissObj *vissObj)
{
    vx_status status =      (vx_status) VX_SUCCESS;

    int32_t fvid2_status =  FVID2_SOK;

    /* Initialize buffer pointer to NULL */
    vissObj->configurationBuffer.bufferPtr = NULL;
    /* Initialize length to zero */
    vissObj->configurationBuffer.length = 0U;
    /* Initialize configThroughUDMA to false */
    vissObj->configurationBuffer.configThroughUdmaFlag = false;

    /* Get configuration buffer information */
    fvid2_status = Fvid2_control(vissObj->handle,
    IOCTL_VHWA_M2M_VISS_GET_BUFF_INFO, (void*) &vissObj->configurationBuffer,
            NULL);

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
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get config buff info from driver\n");
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
    if (FVID2_SOK == fvid2_status)
    /* LDRA_JUSTIFY_END */
    {
        /* configThroughUdmaFlag is true, allocate config buffer */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption.
        This is fail safe mechanism and cannot be removed.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if (true == vissObj->configurationBuffer.configThroughUdmaFlag)
        /* LDRA_JUSTIFY_END */
        {
            /* Indicate that we are allocating out of memory that is cache write through policy */
            vissObj->configurationBuffer.isCacheWriteBackPolicy = false;

            /* allocate buffer for the length requested */
            vissObj->configurationBuffer.bufferPtr = (uint32_t*) tivxMemAlloc(
                    vissObj->configurationBuffer.length,
                    (vx_enum) TIVX_MEM_EXTERNAL_CACHEABLE_WT);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == vissObj->configurationBuffer.bufferPtr)
            {
                VX_PRINT(VX_ZONE_ERROR, "failed to allocate %d bytes !!!\n",
                        vissObj->configurationBuffer.length);
                fvid2_status = FVID2_EALLOC;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                /* Set configuration buffer information */
                fvid2_status = Fvid2_control(vissObj->handle,
                IOCTL_VHWA_M2M_VISS_SET_BUFF_INFO,
                        (void*) &vissObj->configurationBuffer, NULL);
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
                    VX_PRINT(VX_ZONE_ERROR,
                            "Failed to set config buff info into driver\n");
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }
    /* Convert FVID2 status to OpenVX Status */
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
        status = (vx_status) VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = (vx_status) VX_SUCCESS;
    }


    return (status);
}

void tivxVpacVissDeleteConfigBuffer(tivxVpacVissObj *vissObj)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
However, due to the stated rationale, this is not tested.
<justification end> */
    if (NULL != vissObj->configurationBuffer.bufferPtr)
/* LDRA_JUSTIFY_END */
    {
        vx_status free_status = tivxMemFree((void*) vissObj->configurationBuffer.bufferPtr,
                vissObj->configurationBuffer.length,
                (vx_enum) TIVX_MEM_EXTERNAL_CACHEABLE_WT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != free_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to free memory\n");
        }
        /* LDRA_JUSTIFY_END */
    }
}

static vx_status tivxVpacVissSetRfeConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Rfe_Control         rfeCtrl;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameter is expected to be pre-validated from the caller.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
    if (NULL != vissCfgRef->lPwlCfg)
/* LDRA_JUSTIFY_END */
    {
        /* PWL for Long Input */
        rfeCtrl.module  = RFE_MODULE_PWL1;
        rfeCtrl.pwl1Cfg = vissCfgRef->lPwlCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
<metric start>  branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */ 
    if ((NULL != vissCfgRef->sPwlCfg) && (FVID2_SOK == fvid2_status))
/* LDRA_JUSTIFY_END */
    {
        /* PWL for Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL2;
        rfeCtrl.pwl2Cfg = vissCfgRef->sPwlCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */ 
    if ((NULL != vissCfgRef->vsPwlCfg) && (FVID2_SOK == fvid2_status))
/* LDRA_JUSTIFY_END */
    {
        /* PWL for Very Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL3;
        rfeCtrl.pwl3Cfg = vissCfgRef->vsPwlCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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

    if ((NULL != vissCfgRef->dpcOtf) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_DPC_OTF;
        rfeCtrl.dpcOtfCfg    = vissCfgRef->dpcOtf;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
<metric start>  branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */ 
    if ((NULL != vissCfgRef->wbCfg) && (FVID2_SOK == fvid2_status))
/* LDRA_JUSTIFY_END */
    {
        rfeCtrl.module       = RFE_MODULE_GAIN_OFST;
        rfeCtrl.wbConfig     = vissCfgRef->wbCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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

        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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

        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
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
        vissCfgRef->h3aLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->h3aCfg) && (FVID2_SOK == fvid2_status))
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_H3A_SET_CONFIG,
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
            vissObj->h3a_output_size = vissCfgRef->h3aCfg->outputSize;
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
    else
/* LDRA_JUSTIFY_END */
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacVissSetGlbceConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;
    Glbce_Control       glbceCtrl;

    vissDrvPrms = &vissObj->vissPrms;

    /* GLBCE Parameters can be set only if it is enabled at SET_PARAMS time */
    if ((uint32_t)UTRUE == vissDrvPrms->enableGlbce)
    {
        if (NULL != vissCfgRef->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.glbceCfg = vissCfgRef->glbceCfg;
            fvid2_status = Fvid2_control(vissObj->handle, IOCTL_GLBCE_SET_CONFIG,
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
            fvid2_status = Fvid2_control(vissObj->handle, IOCTL_GLBCE_SET_CONFIG,
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
            fvid2_status = Fvid2_control(vissObj->handle, IOCTL_GLBCE_SET_CONFIG,
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

static vx_status tivxVpacVissSetNsf4Config(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &vissObj->vissPrms;

    if (((uint32_t)UTRUE == vissDrvPrms->enableNsf4) &&
        (NULL != vissCfgRef->nsf4Cfg))
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_NSF4_SET_CONFIG,
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
    if (NULL != vissObj->raw_hist_out_target_ptr)
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_SET_RAW_HIST_APP_PTR,
            (void *)vissObj->raw_hist_out_target_ptr, NULL);
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
    else
/* LDRA_JUSTIFY_END */
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacVissSetFcpConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef,
    uint32_t fcpInstance)
{
    int32_t             status = FVID2_SOK;
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
    vissDrvPrms = &vissObj->vissPrms;

    if(FVID2_SOK == status)
    {
        if (NULL != vissCfgRef->fcpCfg[fcpInstance].cfaLut16to12Cfg)
        {
            fcpCtrl.module = FCP_MODULE_COMPANDING;
            fcpCtrl.inComp = vissCfgRef->fcpCfg[fcpInstance].cfaLut16to12Cfg;
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vissCfgRef->fcpCfg[fcpInstance].ccm)
        /* LDRA_JUSTIFY_END */
        {

            fcpCtrl.module      = FCP_MODULE_CCM;
            fcpCtrl.ccm         = vissCfgRef->fcpCfg[fcpInstance].ccm;
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
        #if defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to perform platform-specific instance configuration.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        #endif
        if(fcpInstance == 0U)
        #if defined(VPAC3L)
        /* LDRA_JUSTIFY_END */
        #endif
        {
            EE_on_Y12 = VHWA_M2M_VISS_EE_ON_FCP0_LUMA12;
            EE_on_Y8 = VHWA_M2M_VISS_EE_ON_FCP0_LUMA8;
        }
        #ifdef VPAC3
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach the error portion.
        fcpInstance is pre-validated before calling tivxVpacVissSetFcpConfig() and can only have a value of 0 or 1. This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if(fcpInstance == 1U)
        /* LDRA_JUSTIFY_END */
        {
            EE_on_Y12 = VHWA_M2M_VISS_EE_ON_FCP1_LUMA12;
            EE_on_Y8 = VHWA_M2M_VISS_EE_ON_FCP1_LUMA8;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach the error portion.
        fcpInstance is pre-validated before calling tivxVpacVissSetFcpConfig() and can only have a value of 0 or 1. This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid FCP Instance !!!\n");
            status = (int32_t)VX_ERROR_INVALID_PARAMETERS;
        }
        /* LDRA_JUSTIFY_END */
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
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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

        #if defined(VPAC3) || defined(VPAC3L)

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].comLutCfg)
        {
            fcpCtrl.module          = FCP_MODULE_CFA_COMPANDING;
            fcpCtrl.cLutComp             = vissCfgRef->fcpCfg[fcpInstance].comLutCfg;
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set FCP_MODULE_CFA_COMPANDING Config !!!\n");
            }

    /* LDRA_JUSTIFY_END */
            vissCfgRef->fcpCfg[fcpInstance].comLutCfg = NULL;
        }

        if (NULL != vissCfgRef->fcpCfg[fcpInstance].decomLutCfg)
        {
            fcpCtrl.module          = FCP_MODULE_DECOMPANDING;
            fcpCtrl.dLutComp             = vissCfgRef->fcpCfg[fcpInstance].decomLutCfg;
            status = Fvid2_control(vissObj->handle, ioctl_set_config,
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set FCP_MODULE_DECOMPANDING Config !!!\n");
            }

    /* LDRA_JUSTIFY_END */
            vissCfgRef->fcpCfg[fcpInstance].decomLutCfg = NULL;

        }
        #endif
    }

    return (status);
}

#if defined(VPAC3) || defined(VPAC3L)
static vx_status tivxVpacVissSetCacConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &vissObj->vissPrms;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameter is expected to be pre-validated from the caller.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
    if (((uint32_t)UTRUE == vissDrvPrms->enableCac) &&
        (NULL != vissCfgRef->cacCfg))
/* LDRA_JUSTIFY_END */
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_CAC_SET_CONFIG,
            (void *)vissCfgRef->cacCfg, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameter is expected to be pre-validated from the caller.
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
    else
/* LDRA_JUSTIFY_END */
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}
#endif

#if defined(VPAC3L)
static vx_status tivxVpacVissSetPcidConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &vissObj->vissPrms;

    if (((uint32_t)UTRUE == (uint32_t)vissDrvPrms->enablePcid) &&
        (NULL != vissCfgRef->pcidCfg))
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_PCID_SET_CONFIG,
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

    /* Convert FVID2 status to OpenVX Status */
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
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}
#endif


