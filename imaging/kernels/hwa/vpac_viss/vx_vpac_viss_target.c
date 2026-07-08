/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
#include "tivx_hwa_vpac_viss_priv.h"
#include <utils/ipc/include/app_ipc.h>
#if defined(LDRA_COVERAGE)
#include <kernels/coverage_files/include/ldra_remote_core_coverage_main.h>
#endif
#include <vhwa/include/vhwa_m2mViss.h>
#include <stddef.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* #undef below to see performance without VISS GLBCE context save/restore */
#define VHWA_VISS_GLBCE_CTX_SAVE_RESTORE_ENABLE

/* #undef below to see performance using CPU for VISS GLBCE context save/restore */
#define VHWA_VISS_GLBCE_CTX_SAVE_RESTORE_USE_DMA


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */ 

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxVpacVissObj *tivxVpacVissAllocObject(tivxVpacVissInstObj *instObj);
static void tivxVpacVissFreeObject(tivxVpacVissInstObj *instObj,
    tivxVpacVissObj *vissObj);
static void tivxVpacVissSetIsInvalidFlag(tivx_obj_desc_t *obj_desc[]);
static vx_status tivxVpacVissGetErrStatusCmd(const tivxVpacVissObj *vissObj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacVissGetPsaStatusCmd(const tivxVpacVissObj *vissObj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacVissEnableErrorEventsCmd(tivxVpacVissObj *vissObj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacVissMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxVpacVissMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

int32_t tivxVpacVissFrameComplCb(Fvid2_Handle handle, void *appData);
static void tivxVpacVissErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxVpacVissWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);

#if !defined(VPAC3L)
static vx_status tivxEnableVpacVissSafetyMechanisms(
    tivxVpacVissObj *vissObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);

static vx_status tivxVpacVissAllocReadbackBuffers(tivxVpacVissObj *vissObj, uint32_t config_reg_readback_addr);

static void tivxVpacVissFreeReadbackBuffers(tivxVpacVissObj *vissObj);
#endif
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_viss_target_kernel = NULL;
tivxVpacVissInstObj gTivxVpacVissInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacViss(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }

    #if (VPAC_COUNT > 1)
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */     
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #endif
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
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_FAILURE;
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
    if (status == (vx_status)VX_SUCCESS)
    {
    /* LDRA_JUSTIFY_END */
        memset(&gTivxVpacVissInstObj, 0x0, sizeof(tivxVpacVissInstObj));

        status = tivxMutexCreate(&gTivxVpacVissInstObj.lock);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Allocate lock \n");
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        vx_vpac_viss_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_VISS_NAME,
                            target_name,
                            tivxVpacVissProcess,
                            tivxVpacVissCreate,
                            tivxVpacVissDelete,
                            tivxVpacVissControl,
                            NULL);
    }
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */ 
void tivxRemoveTargetKernelVpacViss(void)
{

    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != gTivxVpacVissInstObj.lock)
    {
        (void)tivxMutexDelete(&gTivxVpacVissInstObj.lock);
    }

    status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Viss TargetKernel\n");
    }
}

/* LDRA_JUSTIFY_END */
/* Setting the output descriptor flags to invalid if received raw_image is invalid */
static void tivxVpacVissSetIsInvalidFlag(tivx_obj_desc_t *obj_desc[])
{
    uint32_t                   cnt;
    uint32_t                   out_start;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> oos
    Rationale: Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */    
    if ((uint32_t)tivxFlagIsBitSet(obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX]->flags, TIVX_REF_FLAG_IS_INVALID) == 1U)
    {
        out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if (NULL != obj_desc[out_start])
            {
                tivxFlagBitSet(&obj_desc[out_start]->flags, TIVX_REF_FLAG_IS_INVALID);
            }
            out_start ++;
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX])
        {
            tivxFlagBitSet(&obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX])
        {
            tivxFlagBitSet(&obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if (NULL != obj_desc[out_start])
            {
                tivxFlagBitClear(&obj_desc[out_start]->flags, TIVX_REF_FLAG_IS_INVALID);
            }
            out_start ++;
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX])
        {
            tivxFlagBitClear(&obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX])
        {
            tivxFlagBitClear(&obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    fvid2_status = FVID2_SOK;
    uint32_t                   cnt;
    uint32_t                   out_start;
    tivxVpacVissObj           *vissObj = NULL;
    Vhwa_M2mVissParams        *vissDrvPrms = NULL;
    tivx_vpac_viss_params_t   *vissPrms;
    tivx_obj_desc_raw_image_t *raw_img_desc;
    tivx_obj_desc_image_t     *img_desc[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    tivx_ae_awb_params_t      *ae_awb_result = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t *h3a_out_desc = NULL;
    tivx_obj_desc_user_data_object_t *dcc_buf_desc = NULL;
    tivx_obj_desc_distribution_t *raw_histogram_desc = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    else
    {
        vissObj = tivxVpacVissAllocObject(&gTivxVpacVissInstObj);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameter is expected to be pre-validated from a software layer above imaging.
Therefore, this failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */  
        if (NULL != vissObj)
/* LDRA_JUSTIFY_END */
        {
            /* Assign object descriptors */
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
            dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_DCC_BUF_IDX];
            raw_img_desc = (tivx_obj_desc_raw_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
            h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
            raw_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_RAW_HISTOGRAM_IDX];

            /* Get All output image object descriptors */
            out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameter is expected to be pre-validated from a software layer above imaging.
Therefore, this failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */  
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Alloc Failed for Viss Object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
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
        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the host kernel before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */  
            if(config_desc->mem_size != sizeof(tivx_vpac_viss_params_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_vpac_viss_params_t, host size (%d) != target size (%d)\n",
                    config_desc->mem_size, sizeof(tivx_vpac_viss_params_t));
            }
/* LDRA_JUSTIFY_END */            
        }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the host kernel before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */  
        if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */  
            if(aewb_res_desc->mem_size != sizeof(tivx_ae_awb_params_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_ae_awb_params_t, host size (%d) != target size (%d)\n",
                    aewb_res_desc->mem_size, sizeof(tivx_ae_awb_params_t));
            }
/* LDRA_JUSTIFY_END */
        }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */  
        if (((vx_status)VX_SUCCESS == status) && (NULL != raw_histogram_desc))
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel in before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */  
            if(raw_histogram_desc->mem_size != (128U*sizeof(uint32_t)))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "raw_histogram_desc, host size (%d) != target size (%d)\n",
                    raw_histogram_desc->mem_size, (128U*sizeof(uint32_t)));
            }
/* LDRA_JUSTIFY_END */
        }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */ 
        if (((vx_status)VX_SUCCESS == status) && (NULL != h3a_out_desc))
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */  
            if(h3a_out_desc->mem_size != sizeof(tivx_h3a_data_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_h3a_data_t, host size (%d) != target size (%d)\n",
                    h3a_out_desc->mem_size, sizeof(tivx_h3a_data_t));
            }
/* LDRA_JUSTIFY_END */
            if (NULL != dcc_buf_desc)
            {
                vissObj->h3a_out_enabled = (vx_bool)vx_true_e;
            }
            else
            {
                VX_PRINT(VX_ZONE_WARNING,
                    "VISS H3A output is not generated due to DCC not being enabled\n");
            }

            /* This is taking the app ipc cpu number, not the tivx mapped number, so it can
             * be used by AWB node for direct usage without translation */
            vissObj->cpu_id = appIpcGetSelfCpuId();

        }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */ 
        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            status = tivxMutexCreate(&vissObj->config_lock);
        }

        /* Now Map config Desc and get VISS Parameters */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */ 
        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            status = tivxVpacVissMapUserDesc(&vissObj->viss_prms_target_ptr,
                config_desc, sizeof(tivx_vpac_viss_params_t));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */ 
            if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
            {
                vissPrms = (tivx_vpac_viss_params_t *)
                    vissObj->viss_prms_target_ptr;
            }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */   
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Map VISS Parameters Descriptor\n");
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
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate mutex\n");
        }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */ 

        /* Extract AEWB Result parameters, it might be needed in
         * setting some of the VISS configuration */
        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            if(NULL != aewb_res_desc)
            {
                status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                    aewb_res_desc, sizeof(tivx_ae_awb_params_t));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */ 
                if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                {
                    ae_awb_result = (tivx_ae_awb_params_t *)
                        vissObj->aewb_res_target_ptr;
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */                  
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map AEWB Result Descriptor\n");
                }
/* LDRA_JUSTIFY_END */
            }
            else
            {
                uint32_t i, loop_break = 0U;

                /* AEWB Result sent by the graph is NULL */
                /* VISS needs to use the results sent by AEWB node through VISS_CMD_SET_2A_PARAMS command */
                /* RemoteService command is supported only on target*/
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                <justification end> */                
                for(i=0U;i<VHWA_M2M_VISS_MAX_HANDLES;i++)
                /* LDRA_JUSTIFY_END */
                {
                    status = tivxMutexLock(viss_aewb_lock[i]);
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
                        if(0U == viss_aewb_channel[i])
                        {
                            viss_aewb_channel[i] = 1u;
                            vissObj->channel_id = i;
                            memset(&viss_aewb_results[vissObj->channel_id], 0x0, sizeof(tivx_ae_awb_params_t));
                            loop_break = 1U;
                        }
                        status = tivxMutexUnlock(viss_aewb_lock[i]);
                    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
                    if(((vx_status)VX_SUCCESS != status) || (loop_break == 1U))
/* LDRA_JUSTIFY_END */
                    {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "viss_aewb_lock[%d] failed\n", i);
                        }
/* LDRA_JUSTIFY_END */
                        break;
                    }
                }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
                if(((vx_status)VX_SUCCESS == status) && (loop_break == 0U))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Number of instances has exceeded VHWA_M2M_VISS_MAX_HANDLES\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }

/* LDRA_JUSTIFY_END */
            }
        }

    }

    /* Now allocate the require resources and open the FVID2 driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_m2mVissCreateArgsInit(&vissObj->createArgs);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.

        <justification end> */
        if (NULL != vissObj)
        /* LDRA_JUSTIFY_END */
        {
            vissObj->createArgs.enablePsa = vissPrms->enable_psa;
        }

        status = tivxEventCreate(&vissObj->waitForProcessCmpl);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */  
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
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
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            vissObj->cbPrms.cbFxn   = tivxVpacVissFrameComplCb;
            vissObj->cbPrms.appData = vissObj;

            vissObj->handle = Fvid2_create(FVID2_VHWA_M2M_VISS_DRV_ID,
                vissObj->viss_drv_inst_id, &vissObj->createArgs,
                NULL, &vissObj->cbPrms);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (NULL == vissObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Open Driver\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* Register Error Callback */
    if ((vx_status)VX_SUCCESS == status)
    {
        vissObj->errEvtPrms.errEvents = VHWA_VISS_ALL_ERR_EVENTS;
        vissObj->errEvtPrms.cbFxn     = tivxVpacVissErrorCb;
        vissObj->errEvtPrms.appData   = vissObj;

        fvid2_status = Fvid2_control(vissObj->handle,
            IOCTL_VHWA_M2M_VISS_REGISTER_ERR_CB, &vissObj->errEvtPrms, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Register Error Callback\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */ 
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set default 64K cycle timeout during registration */
        vissObj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        /* Disable Watchdog Timer Error Events by default */
        vissObj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
        vissObj->wdTimerErrEvtPrms.cbFxn     = tivxVpacVissWdTimerErrorCb;
        vissObj->wdTimerErrEvtPrms.appData   = vissObj;
        fvid2_status = Fvid2_control(vissObj->handle,
            IOCTL_VHWA_M2M_VISS_REGISTER_WDTIMER_ERR_CB, &vissObj->wdTimerErrEvtPrms, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Register Watchdog timer Error Callback\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */ 
    }

    /* Allocate memory for the GLBCE Statistics */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vhwaVissAllocMemForCtx(vissObj, vissPrms);
    }

    /* Extract the format information from the config descriptor
     * and output images and set the format in Driver using
     * SET_PARAMS ioctl  */
    if ((vx_status)VX_SUCCESS == status)
    {
        vissDrvPrms = &vissObj->vissPrms;

        status = tivxVpacVissSetParams(vissDrvPrms, vissPrms);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set the input image format and number of inputs from
         * raw image descriptor */
        tivxVpacVissSetInputParams(vissObj, raw_img_desc);
        
        /* Set the output image format from the output images
            * this function also maps the vx_image format to
            * Fvid2_format and Fvid2_storage format and sets it in
            * viss output format */
        status = tivxVpacVissSetOutputParams(
            vissObj, vissPrms, img_desc);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Output Params\n");
        }
    }

    /* Set default values for ALL viss configuration parameters */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetDefaultParams(vissObj, vissPrms, NULL);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set H3A Source parameters, this mainly sets up the
         * input source to the h3a. */
        tivxVpacVissSetH3aSrcParams(vissObj, vissPrms);
    }

    /* Extract individual module specific parameters from the DCC data
     * base and set it in the driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != dcc_buf_desc)
        {
            vissObj->use_dcc = 1u;

            status = tivxVpacVissInitDcc(vissObj, vissPrms);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */ 
            {
                /* Parse DCC Database and store the output in local variables */
                status = tivxVpacVissSetParamsFromDcc(
                    vissObj, dcc_buf_desc, h3a_out_desc, ae_awb_result);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set DCC Params\n");
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set DCC Params\n");
            }
            /* LDRA_JUSTIFY_END */ 

            /* Enable DPC based in driver */
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion. The code block is unused because of no known use-case or configuration mode that is currently available for this feature.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */ 
            if (((uint32_t)UTRUE == vissObj->vissCfg.dpcLutCfg.enable) ||
                ((uint32_t)UTRUE == vissObj->vissCfg.dpcOtfCfg.enable))
            /* LDRA_JUSTIFY_END */
            {
                vissDrvPrms->enableDpc = (uint32_t)UTRUE;
            }
            else
            {
                vissDrvPrms->enableDpc = (uint32_t)UFALSE;
            }

            #ifdef VPAC3L
            /* Set the DPC OTF CFA mode */
            vissDrvPrms->cfa_mode = vissObj->vissCfg.dpcOtfCfg.cfa_mode;
            #endif

        }
    }

    /* before writing configuration set application buffer */
    if ((vx_status) VX_SUCCESS == status)
    {
        status = tivxVpacVissSetConfigBuffer(vissObj);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set ConfigBuf in driver\n");
        }
        /* LDRA_JUSTIFY_END */ 
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* All Formats, frame size, module enables are set in
         * viss parameters, call this ioctl to set validate and set
         * them in the driver */
        fvid2_status = Fvid2_control(vissObj->handle,
            IOCTL_VHWA_M2M_VISS_SET_PARAMS, (void *)vissDrvPrms, NULL);

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
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Params in driver\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */ 
    }

    /* Now Set the parsed parameters in the VISS Driver,
     * This is required even for the non-DCC parameters, like
     * H3A Input source */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetConfigInDrv(vissObj);

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
            /* Reset this flag, as the config is already applied to driver */
            vissObj->isConfigUpdated = 0U;
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set non-DCC Params\n");
        }
        /* LDRA_JUSTIFY_END */ 
    }

    /* Unmap descriptor memories */
    if ((vx_status)VX_SUCCESS == status)
    {
        /* If the target pointer is non null, descriptor is also non null,
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if ((NULL != aewb_res_desc) && (NULL != vissObj->aewb_res_target_ptr))
        {
            status = tivxVpacVissUnmapUserDesc(&vissObj->aewb_res_target_ptr,
                aewb_res_desc);
        }

        /* If the target pointer is non null, descriptor is also non null
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.

<justification end> */
        if (NULL != vissObj->viss_prms_target_ptr)
/* LDRA_JUSTIFY_END */
        {
            status = tivxVpacVissUnmapUserDesc(&vissObj->viss_prms_target_ptr,
                config_desc);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxSetTargetKernelInstanceContext(
            kernel, vissObj, sizeof(tivxVpacVissObj));

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
            /* Set up the input frame list */
            for (cnt = 0U; cnt < vissObj->num_in_buf; cnt ++)
            {
                vissObj->inFrmList.frames[cnt] = &vissObj->inFrm[cnt];
            }
            vissObj->inFrmList.numFrames = vissObj->num_in_buf;

            /* Set up the output frame list */
            for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
            {
                vissObj->outFrmList.frames[cnt] = &vissObj->outFrm[cnt];
            }
            vissObj->outFrmList.numFrames = VHWA_M2M_VISS_MAX_OUTPUTS;
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        if (NULL != vissObj)
        {
            tivxVpacVissDeInitDcc(vissObj);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.

<justification end> */ 
            if (NULL != vissObj->handle)
/* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(vissObj->handle, NULL);
                vissObj->handle = NULL;
            }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.

<justification end> */ 
            if (NULL != vissObj->waitForProcessCmpl)
/* LDRA_JUSTIFY_END */ 
            {
                (void)tivxEventDelete(&vissObj->waitForProcessCmpl);
            }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.

<justification end> */ 
            if (NULL != vissObj->config_lock)
/* LDRA_JUSTIFY_END */ 
            {
                (void)tivxMutexDelete(&vissObj->config_lock);
            }

            tivxVpacVissFreeObject(&gTivxVpacVissInstObj, vissObj);

            vhwaVissFreeCtxMem(vissObj);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxVpacVissObj         *vissObj = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */   
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
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
            vissObj = temp_kernel_context;
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        /* Check the validity of context object */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect kernel instance context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */ 
        {
            tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;

            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];

            if(NULL == aewb_res_desc)
            {
                status = tivxMutexLock(viss_aewb_lock[vissObj->channel_id]);
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
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. An error print statement can be added in a future release if required.
                    <justification end> */ 
                    if(0U != viss_aewb_channel[vissObj->channel_id])
                    /* LDRA_JUSTIFY_END */
                    {
                        viss_aewb_channel[vissObj->channel_id] = 0u;
                    }
                    status = tivxMutexUnlock(viss_aewb_lock[vissObj->channel_id]);
                }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */ 
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "viss_aewb_lock[%d] failed\n", vissObj->channel_id);
                }
/* LDRA_JUSTIFY_END */ 
            }

            tivxVpacVissDeInitDcc(vissObj);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, our code base is expected to perform error handling or configuration.
            However, due to the stated rationale, this is not tested.If the control reaches here, our code base is expected to perform error handling or configuration.
            <justification end> */            
            if (true == vissObj->configurationBuffer.configThroughUdmaFlag)
            /* LDRA_JUSTIFY_END */
            {
                tivxVpacVissDeleteConfigBuffer(vissObj);
            }
            (void)Fvid2_delete(vissObj->handle, NULL);
            vissObj->handle = NULL;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.

<justification end> */ 
            if (NULL != vissObj->waitForProcessCmpl)
/* LDRA_JUSTIFY_END */ 
            {
                (void)tivxEventDelete(&vissObj->waitForProcessCmpl);
            }

            /* Free config register readback buffers if allocated */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
#if !defined(VPAC3L)
            if ((vissObj->readback_mem_ptr_phys != 0u) || (vissObj->golden_reg_mem_ptr_phys != 0u))
            /* LDRA_JUSTIFY_END */
            {
                tivxVpacVissFreeReadbackBuffers(vissObj);
            }
#endif
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.

<justification end> */ 
            if (NULL != vissObj->config_lock)
/* LDRA_JUSTIFY_END */ 
            {
                (void)tivxMutexDelete(&vissObj->config_lock);
            }

            tivxVpacVissFreeObject(&gTivxVpacVissInstObj, vissObj);

            vhwaVissFreeCtxMem(vissObj);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    fvid2_status = FVID2_SOK;
    uint32_t                   cnt;
    uint32_t                   out_start;
    uint32_t                   buf_cnt;
    uint32_t                   size;
    tivxVpacVissObj           *vissObj = NULL;
    tivx_vpac_viss_params_t   *vissPrms = NULL;
    tivx_obj_desc_raw_image_t *raw_img_desc;
    tivx_obj_desc_image_t     *img_desc[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    tivx_ae_awb_params_t      *ae_awb_result = NULL;
    tivx_h3a_data_t           *h3a_out = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t *h3a_out_desc = NULL;
    tivx_obj_desc_distribution_t *raw_histogram_desc = NULL;
    tivx_obj_desc_t         *out_base_desc = NULL;
    #if !defined(VPAC3L)
    vx_status                   validate_reg_status = (vx_status)VX_SUCCESS;
    #endif

    uint64_t start_time, cur_time;
    tivx_ae_awb_params_t   aewb_params;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
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
            vissObj = temp_kernel_context;
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        /* Check the validity of context object */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect kernel instance context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */ 
        {
            raw_img_desc = (tivx_obj_desc_raw_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];

            /* Convert object descriptor to image descriptor */
            out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }

            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
            config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
            h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
#if defined(VPAC3) || defined(VPAC3L)
            raw_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_RAW_HISTOGRAM_IDX];
#endif
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
    if ( (vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */ 
    {
        tivxVpacVissSetIsInvalidFlag(obj_desc);
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
        /* Config Desc Cannot be null */
        status = tivxVpacVissMapUserDesc(&vissObj->viss_prms_target_ptr,
            config_desc, sizeof(tivx_vpac_viss_params_t));

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */  
        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            vissPrms = (tivx_vpac_viss_params_t *)vissObj->viss_prms_target_ptr;
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Map VISS Parameters Descriptor\n");
        }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */  
        /* AEWB Result is optional parameter */
        if((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            if(NULL != aewb_res_desc)
            {
                status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                    aewb_res_desc, sizeof(tivx_ae_awb_params_t));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */  
                if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                {
                    ae_awb_result = (tivx_ae_awb_params_t *)vissObj->aewb_res_target_ptr;
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map AEWB Result Descriptor\n");
                }
/* LDRA_JUSTIFY_END */
            }
            else
            {
                /* AEWB Result sent by the graph is NULL */
                /* VISS needs to use the results sent by AEWB node through VISS_CMD_SET_2A_PARAMS command */
                /* RemoteService command is supported only on target*/
                uint32_t chId = vissObj->channel_id;
                status = tivxMutexLock(viss_aewb_lock[chId]);

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
                    ae_awb_result = &aewb_params;
                    memcpy(ae_awb_result, &viss_aewb_results[chId], sizeof(tivx_ae_awb_params_t));
                    status = tivxMutexUnlock(viss_aewb_lock[chId]);
                }
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
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            if (NULL != h3a_out_desc)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameter is expected to be pre-validated from a software layer above imaging.
                Therefore, this failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                <justification end> */ 
                if (h3a_out_desc->mem_size == sizeof(tivx_h3a_data_t))
                /* LDRA_JUSTIFY_END */
                {
                    vissObj->h3a_out_target_ptr = tivxMemShared2TargetPtr(&h3a_out_desc->mem_ptr);

                    h3a_out = (tivx_h3a_data_t *)vissObj->h3a_out_target_ptr;

                    /* H3A output is special case, only need to map the header since rest is written by HW */
                    tivxCheckStatus(&status, tivxMemBufferMap(vissObj->h3a_out_target_ptr, offsetof(tivx_h3a_data_t, resv),
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameter is expected to be pre-validated from a software layer above imaging.
                Therefore, this failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                <justification end> */  
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map H3A Result Descriptor\n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
            }
        }

    #if defined(VPAC3) || defined (VPAC3L)
        /* Map Buffer memory for the Raw Histogram Statistics */
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
            if (NULL != raw_histogram_desc)
            {
                vissObj->raw_hist_out_target_ptr = tivxMemShared2TargetPtr(&raw_histogram_desc->mem_ptr);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != vissObj->raw_hist_out_target_ptr)
                /* LDRA_JUSTIFY_END */
                {
                    tivxCheckStatus(&status, tivxMemBufferMap(vissObj->raw_hist_out_target_ptr, raw_histogram_desc->mem_size,
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map buffer for Raw Histogram\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
                /* LDRA_JUSTIFY_END */
                vissObj->isConfigUpdated = 1U;
            }
        }
    #endif
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
        /* Update Configuration in Driver */
        status = tivxMutexLock(vissObj->config_lock);
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
            /* Check if there is any change in H3A Input Source */
            tivxVpacVissSetH3aSrcParams(vissObj, vissPrms);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging.
            Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != ae_awb_result)
            /* LDRA_JUSTIFY_END */
            {
                status = tivxVpacVissApplyAEWBParams(vissObj, ae_awb_result);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameter is expected to be pre-validated from a software layer above imaging.
                Therefore, this failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */  
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to apply AEWB Result\n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (((vx_status)VX_SUCCESS == status) && (1u == vissObj->isConfigUpdated))
            /* LDRA_JUSTIFY_END */
            {
                status = tivxVpacVissSetConfigInDrv(vissObj);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */  
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Set Config in Driver\n");
                }
                /* LDRA_JUSTIFY_END */
            }
            (void)tivxMutexUnlock(vissObj->config_lock);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
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
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set the buffer address in the input buffer */
        for (cnt = 0u; cnt < vissObj->num_in_buf; cnt ++)
        {
            vissObj->inFrm[cnt].addr[0u] = tivxMemShared2PhysPtr(
                raw_img_desc->img_ptr[cnt].shared_ptr,
                (int32_t)raw_img_desc->img_ptr[cnt].mem_heap_region);
        }

        out_base_desc = (tivx_obj_desc_t *)raw_img_desc;
        vissObj->timestamp = out_base_desc->timestamp;

        /* Set the buffer address in the output buffer */
        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            for (buf_cnt = 0U; buf_cnt < vissObj->num_out_buf_addr[cnt];
                    buf_cnt ++)
            {
                vissObj->outFrm[cnt].addr[buf_cnt] = tivxMemShared2PhysPtr(
                    img_desc[cnt]->mem_ptr[buf_cnt].shared_ptr,
                    (int32_t)img_desc[cnt]->mem_ptr[buf_cnt].mem_heap_region);
            }
        }

        if (NULL != h3a_out)
        {
            h3a_out->aew_af_mode = vissPrms->h3a_aewb_af_mode;
            h3a_out->h3a_source_data = vissPrms->h3a_in;
            h3a_out->cpu_id = vissObj->cpu_id;
            h3a_out->channel_id = vissObj->channel_id;
            h3a_out->size = vissObj->h3a_output_size;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> np
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            if(0U == vissPrms->h3a_aewb_af_mode)
            /* LDRA_JUSTIFY_END */
            {
                /* TI 2A Node may not need the aew config since it gets it from DCC, but this is copied
                 * in case third party 2A nodes which don't use DCC can easily see this information */
                memcpy(&h3a_out->aew_config, &vissObj->aew_config, sizeof(tivx_h3a_aew_config));
            }

            vissObj->outFrm[VHWA_M2M_VISS_OUT_H3A_IDX].addr[0u] = tivxMemShared2PhysPtr(
                (uint64_t)h3a_out->data,
                (vx_enum)TIVX_MEM_EXTERNAL);

            h3a_out_desc->valid_mem_size = vissObj->h3a_output_size + TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN;

            /* Unmap even before processing since the ARM is done, rest of buffer is HW */
            tivxCheckStatus(&status, tivxMemBufferUnmap(vissObj->h3a_out_target_ptr, offsetof(tivx_h3a_data_t, resv), (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            vissObj->h3a_out_target_ptr = NULL;
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
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        vissObj->wdTimerErrStatus = 0u;
        vissObj->err_stat = 0u;

        start_time = tivxPlatformGetTimeInUsecs();

        tivxLogRtTraceKernelInstanceExeStartTimestamp(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA, start_time);

        /* Submit the request to the driver */
        fvid2_status = Fvid2_processRequest(vissObj->handle, &vissObj->inFrmList,
            &vissObj->outFrmList, FVID2_TIMEOUT_FOREVER);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* Wait for Frame Completion */
            (void)tivxEventWait(vissObj->waitForProcessCmpl,
                VX_TIMEOUT_WAIT_FOREVER);

            cur_time = tivxPlatformGetTimeInUsecs();

            tivxLogRtTraceKernelInstanceExeEndTimestamp(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA, cur_time);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((0u == vissObj->wdTimerErrStatus) && (0u == (vissObj->err_stat & VHWA_VISS_STALLING_ERR_MASK)))
            /* LDRA_JUSTIFY_END */
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(0u != (vissObj->err_stat & ~VHWA_VISS_STALLING_ERR_MASK))
                {
                    VX_PRINT(VX_ZONE_INFO, "Error interrupt: VISS non-stalling error interrupt triggered \n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */

                fvid2_status = Fvid2_getProcessedRequest(vissObj->handle,
                    &vissObj->inFrmList, &vissObj->outFrmList, 0);
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                if(0u != vissObj->wdTimerErrStatus)
                {
                    VX_PRINT(VX_ZONE_ERROR, "IP stall: Watchdog timer error interrupt triggered \n");
                    status = (vx_status)VX_ERROR_TIMEOUT;
                }
                if(0u != (vissObj->err_stat & VHWA_VISS_STALLING_ERR_MASK))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Error interrupt: VISS stalling error interrupt triggered \n");
                    status = (vx_status)VX_ERROR_TIMEOUT;
                }
            }
            /* LDRA_JUSTIFY_END */
#if !defined(VPAC3L)
            /* Call the control command for statusReg/configReg validate */
            {
                fvid2_status = Fvid2_control(vissObj->handle, VHWA_M2M_IOCTL_VISS_VALIDATE_REG, NULL, NULL);
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Register validation failed (Fvid2_control returned %d)\n", fvid2_status);
                    validate_reg_status = (vx_status)VX_FAILURE;
                }
            }
#endif
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
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        appPerfStatsHwaUpdateLoad(vissObj->hwa_perf_id,
            (uint32_t)(cur_time-start_time),
            raw_img_desc->params.width*raw_img_desc->params.height /* pixels processed */
            );
    }

    #if defined(VPAC3) || defined (VPAC3L)
    /* Unmap buffer memory for the Raw Histogram Statistics */
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
        if (NULL != raw_histogram_desc)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.

            <justification end> */
            if (NULL != vissObj->raw_hist_out_target_ptr)
            /* LDRA_JUSTIFY_END */
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(vissObj->raw_hist_out_target_ptr, raw_histogram_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                
                vissObj->raw_hist_out_target_ptr = NULL;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to unmap buffer memory for Raw Histogram\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }
    }
    #endif

    /* If the target pointer is non null, descriptor is also non null,
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
    /* LDRA_JUSTIFY_END */
    {
        status = tivxVpacVissUnmapUserDesc(&vissObj->aewb_res_target_ptr, aewb_res_desc);
    }

    /* If the target pointer is non null, descriptor is also non null
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if (((vx_status)VX_SUCCESS == status) && (NULL != config_desc))
    /* LDRA_JUSTIFY_END */
    {
        status = tivxVpacVissUnmapUserDesc(&vissObj->viss_prms_target_ptr, config_desc);
    }

    #if !defined(VPAC3L)
    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }
    #endif

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacVissObj                  *vissObj = NULL;

    void *temp_kernel_context = NULL;


    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
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
        vissObj = temp_kernel_context;
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */ 
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == vissObj) ||
        (sizeof(tivxVpacVissObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
        status = (vx_status)VX_FAILURE;
    }
    else
/* LDRA_JUSTIFY_END */
    {
        /* do nothing */
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
        switch (node_cmd_id)
        {
            case TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS:
            {
                /* Update Configuration in Driver */
                status = tivxMutexLock(vissObj->config_lock);
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
                    status = tivxVpacVissSetParamsFromDcc(vissObj,
                        (tivx_obj_desc_user_data_object_t *)obj_desc[0U], NULL, NULL);
                    (void)tivxMutexUnlock(vissObj->config_lock);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
                }
                break;
            }
            case TIVX_VPAC_VISS_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacVissGetErrStatusCmd(vissObj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_VISS_CMD_GET_PSA_STATUS:
            {
                status = tivxVpacVissGetPsaStatusCmd(vissObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_VISS_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxVpacVissEnableErrorEventsCmd(vissObj,
                  (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
#if defined(LDRA_COVERAGE)
            case TIVX_VPAC_COVERAGE_END:
            {
                #if defined (C7X_FAMILY) || defined(R5F)
                appLogPrintf("################### DYNAMIC CODE COVERAGE TERMINATION STARTED ######################\n");
                ldra_terminate();
                appLogPrintf("################### DYNAMIC CODE COVERAGE TERMINATION FINISHED ######################\n");
                #endif
                break;
            }
#endif
#if !defined(VPAC3L)
            case TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM:
            {
                status = tivxEnableVpacVissSafetyMechanisms(vissObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to enable VPAC VISS safety mechanisms\n");
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#endif
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Node Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }
    return (status);
}


/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacVissObj *tivxVpacVissAllocObject(tivxVpacVissInstObj *instObj)
{
    uint32_t         cnt;
    tivxVpacVissObj *vissObj = NULL;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 8 handles.
        Effect on this unit: It will utilise all the 8 handles which is validated in the VISS driver.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (0U == instObj->vissObj[cnt].isAlloc)
            {
                vissObj = &instObj->vissObj[cnt];
                memset(vissObj, 0x0, sizeof(tivxVpacVissObj));
                instObj->vissObj[cnt].isAlloc = 1U;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */   
                if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
                /* LDRA_JUSTIFY_END */
                {
                    instObj->vissObj[cnt].viss_drv_inst_id = VHWA_M2M_VISS_DRV_INST0;
                    instObj->vissObj[cnt].hwa_perf_id      = APP_PERF_HWA_VPAC1_VISS;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */   
                #if (VPAC_COUNT > 1)
                else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
                {
                    instObj->vissObj[cnt].viss_drv_inst_id = VHWA_M2M_VPAC_1_VISS_DRV_INST_ID_0;
                    instObj->vissObj[cnt].hwa_perf_id      = APP_PERF_HWA_VPAC2_VISS;
                }
                #endif
                else
                {
                    /* Unsupported CPU ID */
                    VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                    vissObj = NULL;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */ 
        if (NULL != vissObj)
        /* LDRA_JUSTIFY_END */
        {
            /* Initialize few members to values, other than 0 */
            vissObj->lastH3aInSrc = RFE_H3A_IN_SEL_MAX;
        }

        /* Release instance mutex */
        (void)tivxMutexUnlock(instObj->lock);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
    return (vissObj);
}

static void tivxVpacVissFreeObject(tivxVpacVissInstObj *instObj,
    tivxVpacVissObj *vissObj)
{
    uint32_t cnt;
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 8 handles.
        Effect on this unit: It will utilise all the 8 handles which is validated in the VISS driver.
        <justification end> */
            for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
            {
                if (vissObj == &instObj->vissObj[cnt])
                {
                    vissObj->isAlloc = 0U;
                    break;
                }
            }

        /* Release instance mutex */
        (void)tivxMutexUnlock(instObj->lock);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
}

vx_status tivxVpacVissSetOutputParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms,
    tivx_obj_desc_image_t *obj_desc[])
{
    vx_status                 status = (vx_status)VX_SUCCESS;
    uint32_t                  cnt;
    uint32_t                  out_cnt;
    uint32_t                  out_start;
    uint32_t                  mux_val[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    Vhwa_M2mVissOutputParams *outPrms = NULL;
    Vhwa_M2mVissParams       *vissDrvPrms;
    tivx_obj_desc_image_t    *im_desc;

    status = tivxVpacVissMapMuxVal(mux_val, vissPrms);

    vissDrvPrms = &vissObj->vissPrms;

    /* Disable all Outputs first */
    for (out_cnt = 0u; out_cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; out_cnt ++)
    {
        vissDrvPrms->outPrms[out_cnt].enable = UFALSE;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
        for (out_cnt = 0u; out_cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; out_cnt ++)
        {
            vissObj->num_out_buf_addr[out_cnt] = 0U;
            im_desc = obj_desc[out_cnt];

            if (NULL != obj_desc[out_cnt])
            {
                outPrms = &vissDrvPrms->outPrms[out_cnt];

                            
                #ifdef VPAC3
                    if ((0u == vissPrms->output_fcp_mapping[out_cnt]) ||
                        (2u == vissPrms->output_fcp_mapping[out_cnt]))
                    {
                        outPrms->vPipeline = VHWA_VISS_PIPE_HV;
                    }
                    else
                    {
                        outPrms->vPipeline = VHWA_VISS_PIPE_MV;
                    }
                #endif

                status = tivxVpacVissMapFormat(
                    &outPrms->fmt.dataFormat, &outPrms->fmt.ccsFormat, out_start,
                    im_desc->format, mux_val[out_cnt]);

                if ((vx_status)VX_SUCCESS == status)
                {
                    #ifdef VPAC3L
                    if(TIVX_VPAC_VISS_IR_ENABLE == vissPrms->enable_ir_op)
                    {
                        outPrms->isIrOut = VHWA_VISS_IROUT_ENABLED;
                    }
                    else
                    {
                        outPrms->isIrOut = VHWA_VISS_IROUT_DISABLED;
                    }
                    #endif

                    outPrms->enable = UTRUE;
                    outPrms->fmt.width = im_desc->width;
                    outPrms->fmt.height = im_desc->height;

                    for (cnt = 0u; cnt < TIVX_IMAGE_MAX_PLANES; cnt ++)
                    {
                        outPrms->fmt.pitch[cnt] =
                            (uint32_t)im_desc->imagepatch_addr[cnt].stride_y;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to map format for output%d\n", out_cnt);
                }

                /* TODO: See if there are any others here */
                if ((FVID2_DF_YUV420SP_UV == outPrms->fmt.dataFormat) ||
                    (FVID2_DF_YUV422SP_UV == outPrms->fmt.dataFormat))
                {
                    vissObj->num_out_buf_addr[out_cnt] = 2U;
                }
                else
                {
                    vissObj->num_out_buf_addr[out_cnt] = 1U;
                }
            }

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }

            out_start ++;
        }
    }

    /* H3A is one of the output for the Driver,
     * Enable it if required */
    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_bool)vx_true_e == vissObj->h3a_out_enabled)
        {
            outPrms = &vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_H3A_IDX];
            outPrms->enable = (uint32_t)UTRUE;
            outPrms->fmt.dataFormat = FVID2_DF_RAW;
        }
    }

    return (status);
}

void tivxVpacVissSetInputParams(tivxVpacVissObj *vissObj,
    const tivx_obj_desc_raw_image_t *raw_img_desc)
{
    Fvid2_Format        *fmt;
    Vhwa_M2mVissParams  *vissDrvPrms;

    vissDrvPrms = &vissObj->vissPrms;
    fmt = &vissDrvPrms->inFmt;

    /* Set number of inputs */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
    <justification end> */
    if (1U == raw_img_desc->params.num_exposures)
    /* LDRA_JUSTIFY_END */
    {
        vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT;
    }
    else if (2U == raw_img_desc->params.num_exposures)
    {
        vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE;
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
    else if (3U == raw_img_desc->params.num_exposures)
    /* LDRA_JUSTIFY_END */
    {
        vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */    
    else
    {
        /* do nothing */
    }

    /* LDRA_JUSTIFY_END */
    /* Set the Input Format */
    fmt->width = raw_img_desc->params.width;
    fmt->height = raw_img_desc->params.height;
    fmt->pitch[0] = (uint32_t)raw_img_desc->imagepatch_addr[0U].stride_y;
    fmt->dataFormat = FVID2_DF_RAW;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    {
        vx_uint32 msb;
        switch (raw_img_desc->params.format[0U].pixel_container)
        {
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion.
        Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
        A test case will be added to cover this gap in a future release.
        <justification end> */
        case (vx_enum)TIVX_RAW_IMAGE_8_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        /* LDRA_JUSTIFY_END */
        case (vx_enum)TIVX_RAW_IMAGE_16_BIT:
            msb = raw_img_desc->params.format[0U].msb;
            if(msb < 7U)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Format \n");
                /* do nothing */
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if ((msb <= 11U) && (msb >= 8U))
            /* LDRA_JUSTIFY_END */
            {
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if ((msb == 12U) || (msb == 13U))
            /* LDRA_JUSTIFY_END */
            {
                fmt->ccsFormat = FVID2_CCSF_BITS14_UNPACKED16;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if (msb == 15U)
            /* LDRA_JUSTIFY_END */
            {
                fmt->ccsFormat = FVID2_CCSF_BITS16_PACKED;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The below condition is redundant and will be covered in one of the above statements.
            Effect on unit: None, redundant code.
            <justification end> */
            else if (msb == 9U)
            {
                fmt->ccsFormat = FVID2_CCSF_BITS16_PACKED;
            }
            else
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            {
                /*MSB = 7 translates to FVID2_CCSF_BITS8_UNPACKED16*/
                /*MSB = 11 translates to FVID2_CCSF_BITS12_UNPACKED16*/
                /*MSB = 14 translates to FVID2_CCSF_BITS15_UNPACKED16*/
                fmt->ccsFormat = FVID2_CCSF_BITS8_UNPACKED16 + msb - 7U;
            }
            /* LDRA_JUSTIFY_END */
            break;
        case (vx_enum)TIVX_RAW_IMAGE_P12_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        default:
            /* do nothing */
            break;
        /* LDRA_JUSTIFY_END */
        }
    }

    vissObj->num_in_buf = 1u;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    switch (vissDrvPrms->inputMode)
    {
    /* LDRA_JUSTIFY_END */
        case VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT:
            vissObj->num_in_buf = 1u;
            break;
        case VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE:
            vissObj->num_in_buf = 2u;
            break;
        case VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE:
            vissObj->num_in_buf = 3u;
            break;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */ 
        default:
            vissObj->num_in_buf = 1u;
            break;
        /* LDRA_JUSTIFY_END */
    }

    return;
}

vx_status tivxVpacVissMapStorageFormat(uint32_t *ccsFmt, uint32_t vxFmt)
{
    vx_status status = (vx_status)VX_SUCCESS;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>  Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
    if ((vx_df_image)VX_DF_IMAGE_U16 == vxFmt)
/* LDRA_JUSTIFY_END */
    {
        *ccsFmt = FVID2_CCSF_BITS12_UNPACKED16;
    }
    else if ((vx_df_image)TIVX_DF_IMAGE_P12 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS12_PACKED;
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>  Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    else if ((vx_df_image)VX_DF_IMAGE_U8 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS8_PACKED;
    }
    /* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>  Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Storage Format \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

/* LDRA_JUSTIFY_END */
    return (status);
}

vx_status tivxVpacVissMapFormat(uint32_t *fmt, uint32_t *ccsFmt,
    uint32_t out_id, uint32_t vxFmt, uint32_t mux_val)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to perform mux-specific configuration mapping.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    switch (mux_val)
    /* LDRA_JUSTIFY_END */
    {
    /* LDRA_JUSTIFY_END */
        case 0U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>  Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
/* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            /* Map data format on mux val0 */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_LUMA_ONLY;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT1_IDX == out_id) ||
                     (TIVX_KERNEL_VPAC_VISS_OUT3_IDX == out_id))
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_CHROMA_ONLY;
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
                VX_PRINT(VX_ZONE_ERROR, "mux0 not supported on output4\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
            /* LDRA_JUSTIFY_END */
        }
        case 1U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>  Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
/* LDRA_JUSTIFY_END */
            /* Map data format on mux val1 */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id)
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_RED;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            else if (TIVX_KERNEL_VPAC_VISS_OUT3_IDX == out_id)
            {
                *fmt = FVID2_DF_GREEN;
            }
            else if (TIVX_KERNEL_VPAC_VISS_OUT4_IDX == out_id)
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_BLUE;
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
                VX_PRINT(VX_ZONE_ERROR, "mux1 not supported on output0/1\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            /* LDRA_JUSTIFY_END */
            break;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */
        case 2U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);
        /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>  Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            /* Map data format on mux val2 */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT1_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT3_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT4_IDX == out_id))
            {
                *fmt = FVID2_DF_RAW;
            }
            /* LDRA_JUSTIFY_END */
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
                VX_PRINT(VX_ZONE_ERROR, "mux2 not supported on output0\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
            /* LDRA_JUSTIFY_END */
        }
        case 3U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>  Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
            /* LDRA_JUSTIFY_END */
            /* Map data format on mux val2 */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to set format configuration for specific output ports.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
            /* LDRA_JUSTIFY_END */    
            {
                *fmt = FVID2_DF_GREY;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if (TIVX_KERNEL_VPAC_VISS_OUT4_IDX == out_id)
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_SATURATION;
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
                VX_PRINT(VX_ZONE_ERROR, "mux3 not supported on output1/3\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
            /* LDRA_JUSTIFY_END */
        }
        case 4U:
        {
            if ((vx_df_image)VX_DF_IMAGE_NV12 == vxFmt)
            {
                *ccsFmt = FVID2_CCSF_BITS8_PACKED;
            }
            else if ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == vxFmt)
            {
                *ccsFmt = FVID2_CCSF_BITS12_PACKED;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "only NV12 supported on mux4\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to configure output ports for NV12 format.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                    (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
                /* LDRA_JUSTIFY_END */
                {
                    *fmt = FVID2_DF_YUV420SP_UV;
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
                    VX_PRINT(VX_ZONE_ERROR, "only output0/2 supports on mux4\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                /* LDRA_JUSTIFY_END */
            }
            break;
        }
        case 5U:
        {
            *ccsFmt = FVID2_CCSF_BITS8_PACKED;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
            /* LDRA_JUSTIFY_END */
            {
                if ((vx_df_image)VX_DF_IMAGE_UYVY == vxFmt)
                {
                    *fmt = FVID2_DF_YUV422I_UYVY;
                }
                else if ((vx_df_image)VX_DF_IMAGE_YUYV == vxFmt)
                {
                    *fmt = FVID2_DF_YUV422I_YUYV;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "only UYVY/YUYV formats supported \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux5 is supported only on output2 \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
            /* LDRA_JUSTIFY_END */
        }
        #ifdef VPAC3L
        case 6U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            /* Only packed 8 and unpacked 16 IR supported in this case*/
            if (TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id)
            {
                *fmt = FVID2_DF_RAW08;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to set RAW16 format for OUT2 port.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id)
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_RAW16;
                *ccsFmt = FVID2_CCSF_BITS16_PACKED;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to print the error and set error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux6 is supported only on output0/2 \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            /* LDRA_JUSTIFY_END */
            break;
        }
        case 7U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            /* Only packed 12 IR supported in this case */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to set RAW12 format for OUT0 port.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id)
            /* LDRA_JUSTIFY_END */
            {
                *fmt = FVID2_DF_RAW12;
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
                VX_PRINT(VX_ZONE_ERROR, "mux7 is supported only on output0 \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            /* LDRA_JUSTIFY_END */
            break;
        }
        #endif
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid value of mux \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}

vx_status tivxVpacVissMapUserDesc(void **target_ptr,
    const tivx_obj_desc_user_data_object_t *desc, uint32_t size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */   
    if (desc->mem_size == size)
    {
        *target_ptr = tivxMemShared2TargetPtr(&desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(*target_ptr, desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */    
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Incorrect descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    /* LDRA_JUSTIFY_END */
    return (status);
}

vx_status tivxVpacVissUnmapUserDesc(void **target_ptr,
    const tivx_obj_desc_user_data_object_t *desc)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, tivxMemBufferUnmap(*target_ptr, desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
        (vx_enum)VX_READ_ONLY));

    *target_ptr = NULL;

    return status;
}

vx_status tivxVpacVissCheckInputDesc(uint16_t num_params,
    tivx_obj_desc_t *obj_desc[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    uint32_t out_start;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale:The current negative test framework cannot cover this since num_params is always set to TIVX_KERNEL_VPAC_VISS_MAX_PARAMS, pre-validated from the host file tivxAddKernelVpacVissValidate()
    Effect:None, control will not reach here.
    <justification end> */    
    if (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Num params incorrect, = %d\n", num_params);
    }

    if ((NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;

        if (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR, "Configuration is NULL\n");
        }
        if (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR, "Raw input is NULL\n");
        }
    }
    else /* At least one output must be enabled */
    /* LDRA_JUSTIFY_END */
    {
        out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if (NULL != obj_desc[out_start])
            {
                break;
            }
            out_start ++;
        }

        if (cnt >= TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT)
        {
            VX_PRINT(VX_ZONE_ERROR, "Atleast one output must be enabled\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return (status);
}

vx_status vhwaVissAllocMemForCtx(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Glbce_Control       glbceCtrl;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */  
    if ((NULL != vissObj) && (NULL != vissPrms))
    /* LDRA_JUSTIFY_END */ 
    {
        if ((0U == vissPrms->bypass_glbce) && (1u == vissPrms->enable_ctx))
        {
            glbceCtrl.module = GLBCE_MODULE_GET_STATS_INFO;
            glbceCtrl.statsInfo = &vissObj->glbceStatInfo;
            fvid2_status = Fvid2_control(vissObj->handle,
                IOCTL_GLBCE_GET_CONFIG, (void *)&glbceCtrl, NULL);
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
                vissObj->ctx_mem_phys_ptr = 0u;
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "Failed to get GLBCE Stats Info!!!\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                #if defined(SOC_AM62A) || defined(SOC_J722S)
                /* ADASVISION-5065: Currently using TIVX_MEM_EXTERNAL while OCMC RAM is not enabled */
                tivxCheckStatus(&status, tivxMemBufferAlloc(&vissObj->ctx_mem_ptr,
                    vissObj->glbceStatInfo.size, (vx_enum)TIVX_MEM_EXTERNAL));
                #else
                tivxCheckStatus(&status, tivxMemBufferAlloc(&vissObj->ctx_mem_ptr,
                    vissObj->glbceStatInfo.size, (vx_enum)TIVX_MEM_INTERNAL_L3));
                #endif

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */ 
                if ((int32_t)NULL == (int32_t)vissObj->ctx_mem_ptr.host_ptr)
                {
                    vissObj->ctx_mem_phys_ptr = 0u;
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to allocate memory!!!\n");
                }
                else
                /* LDRA_JUSTIFY_END */ 
                {
                    vissObj->ctx_mem_phys_ptr = tivxMemShared2PhysPtr(
                        vissObj->ctx_mem_ptr.shared_ptr,
                        (int32_t)vissObj->ctx_mem_ptr.mem_heap_region);

                    VX_PRINT(VX_ZONE_INFO, "TIOVX: VISS: GLBCE ctx mem @ 0x%08x or size %d B\n", (uint32_t)vissObj->ctx_mem_phys_ptr, vissObj->glbceStatInfo.size);

                    /* Set Buffer for GLBCE ctx */
                    /* MISRA-compliant cast to remove volatile qualifier for API compatibility */
                    void *shared_ptr_addr = (void *)((uintptr_t)&vissObj->ctx_mem_ptr.shared_ptr);
                    fvid2_status = Fvid2_control(vissObj->handle,
                        IOCTL_GLBCE_SET_EXT_CNTXT_PTR, shared_ptr_addr, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set Buffer for GLBCE ctx\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }
        else
        {
            vissObj->ctx_mem_phys_ptr = 0u;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */ 
    else
    {
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */ 

    return (status);
}

void vhwaVissFreeCtxMem(tivxVpacVissObj *vissObj)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != vissObj)
    /* LDRA_JUSTIFY_END */
    {
        if (0u != vissObj->ctx_mem_phys_ptr)
        {
            vx_status status = tivxMemBufferFree(&vissObj->ctx_mem_ptr,
                vissObj->glbceStatInfo.size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free context buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            vissObj->ctx_mem_phys_ptr = 0u;
        }
    }
}


vx_status tivxVpacVissSetParams(Vhwa_M2mVissParams *vissDrvPrms, tivx_vpac_viss_params_t *vissPrms)
{
        vx_status status = (vx_status)VX_SUCCESS;

        /* Fill up the VISS Parameters and Set it in the driver */
        Vhwa_m2mVissParamsInit(vissDrvPrms);

        if (0U == vissPrms->bypass_glbce)
        {
            vissDrvPrms->enableGlbce = (uint32_t)UTRUE;
            if (1U == vissPrms->enable_ctx)
            {
                vissDrvPrms->enableGlbceSaveRestoreCtx = (uint32_t)UTRUE;
            }
            else
            {
                vissDrvPrms->enableGlbceSaveRestoreCtx = (uint32_t)UFALSE;
            }
        }
        else
        {
            vissDrvPrms->enableGlbce = (uint32_t)UFALSE;
        }

        if (0U == vissPrms->bypass_nsf4)
        {
            vissDrvPrms->enableNsf4 = (uint32_t)UTRUE;
        }
        else
        {
            vissDrvPrms->enableNsf4 = (uint32_t)UFALSE;
        }

        #ifdef VPAC3L
        if (0U == vissPrms->bypass_pcid)
        {
            vissDrvPrms->enablePcid = (uint32_t)UTRUE;
        }
        else
        {
            vissDrvPrms->enablePcid = (uint32_t)UFALSE;
        }
        #endif
        
        vissDrvPrms->edgeEnhancerMode = vissPrms->fcp[0].ee_mode;
        #ifdef VPAC3
        vissDrvPrms->edgeEnhancerMode |= vissPrms->fcp[1].ee_mode;
        #endif

#if defined(VPAC3) || defined(VPAC3L)
        if (0U == vissPrms->bypass_cac)
        {
            vissDrvPrms->enableCac = (uint32_t)UTRUE;
        }
        else
        {
            vissDrvPrms->enableCac = (uint32_t)UFALSE;
        }
#endif

#ifdef VPAC3
        /* Check if FCP1 is enabled */
        if(vissPrms->fcp1_config != TIVX_VPAC_VISS_FCP1_DISABLED)
        {
            vissDrvPrms->enableMVPipe = UTRUE;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            /* Check for MV input */
            switch(vissPrms->fcp1_config)
            {
            /* LDRA_JUSTIFY_END */
                /* Check for MV input from RawFE */
                case TIVX_VPAC_VISS_FCP1_INPUT_RFE:
                {
                    vissDrvPrms->mvPipeInSel = VHWA_VISS_MV_PIPE_INPUT_RFE;
                    break;
                }
                /* Check for MV input from NSF4 */
                case TIVX_VPAC_VISS_FCP1_INPUT_NSF4:
                {
                    vissDrvPrms->mvPipeInSel = VHWA_VISS_MV_PIPE_INPUT_NSF4;
                    break;
                }
                /* Check for MV input from GLBCE */
                case TIVX_VPAC_VISS_FCP1_INPUT_GLBCE:
                {
                    vissDrvPrms->mvPipeInSel = VHWA_VISS_MV_PIPE_INPUT_GLBC;
                    break;
                }
		/* Check for MV input from CAC */
                case TIVX_VPAC_VISS_FCP1_INPUT_CAC:
                {
                    vissDrvPrms->mvPipeInSel = VHWA_VISS_MV_PIPE_INPUT_CAC;
                    break;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. An error print statement can be added in a future release if required.
                <justification end> */
                default:
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid value of FCP1 MV input \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
#endif
    return (vx_status)status;
}

vx_status tivxVpacVissMapMuxVal(uint32_t *mux_val, const tivx_vpac_viss_params_t *vissPrms)
{
    vx_status                 status = (vx_status)VX_SUCCESS;

    #ifdef VPAC3
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        /*Mux_output0 mmapping*/
        switch (vissPrms->output_fcp_mapping[0u])
        {
        /* LDRA_JUSTIFY_END */
        case 0u:
            mux_val[0U] = vissPrms->fcp[0].mux_output0; 
            break;
        case 1u:
            mux_val[0U] = vissPrms->fcp[1].mux_output0;  
            break;
        case 2u:
            mux_val[0U] = vissPrms->fcp[0].mux_output2; 
            break;
        case 3u:
            mux_val[0U] = vissPrms->fcp[1].mux_output2; 
            break;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid FCP Out Mapping for Mux 0 \n");
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        /*Mux_output1 mmapping*/
        switch (vissPrms->output_fcp_mapping[1u])
        {
        /* LDRA_JUSTIFY_END */
        case 0u:
            mux_val[1U] = vissPrms->fcp[0].mux_output1; 
            break;
        case 1u:
            mux_val[1U] = vissPrms->fcp[1].mux_output1;  
            break;
        case 2u:
            mux_val[1U] = vissPrms->fcp[0].mux_output3; 
            break;
        case 3u:
            mux_val[1U] = vissPrms->fcp[1].mux_output3; 
            break;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid FCP Out Mapping for Mux 1 \n");
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        /*Mux_output2 mmapping*/
        switch (vissPrms->output_fcp_mapping[2u])
        {
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */ 
        case 0u:
            mux_val[2U] = vissPrms->fcp[0].mux_output0; 
            break;
        /* LDRA_JUSTIFY_END */
        case 1u:
            mux_val[2U] = vissPrms->fcp[1].mux_output0;  
            break;
        case 2u:
            mux_val[2U] = vissPrms->fcp[0].mux_output2; 
            break;
        case 3u:
            mux_val[2U] = vissPrms->fcp[1].mux_output2; 
            break;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid FCP Out Mapping for Mux 2 \n");
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        /*Mux_output3 mmapping*/
        switch (vissPrms->output_fcp_mapping[3u])
        {
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */ 
        case 0u:
            mux_val[3U] = vissPrms->fcp[0].mux_output1; 
            break;
        /* LDRA_JUSTIFY_END */

        case 1u:
            mux_val[3U] = vissPrms->fcp[1].mux_output1;  
            break;
        case 2u:
            mux_val[3U] = vissPrms->fcp[0].mux_output3; 
            break;
        case 3u:
            mux_val[3U] = vissPrms->fcp[1].mux_output3; 
            break;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid FCP Out Mapping for Mux 3 \n");
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        /*Mux_output4 mmapping*/
        switch (vissPrms->output_fcp_mapping[4u])
        {
        /* LDRA_JUSTIFY_END */
        case 0u:
            mux_val[4U] = vissPrms->fcp[0].mux_output4; 
            break;

        case 1u:
            mux_val[4U] = vissPrms->fcp[1].mux_output4; 
            break;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid FCP Out Mapping for Mux 4 \n");
            status = (vx_status)VX_FAILURE;
            break;
        /* LDRA_JUSTIFY_END */
        }
    #else
        mux_val[0U] = vissPrms->fcp[0].mux_output0;
        mux_val[1U] = vissPrms->fcp[0].mux_output1;
        mux_val[2U] = vissPrms->fcp[0].mux_output2;
        mux_val[3U] = vissPrms->fcp[0].mux_output3;
        mux_val[4U] = vissPrms->fcp[0].mux_output4;
    #endif

    return status;
}

/**
 * \brief Map VHWA error interrupt flags to TIVX error interrupt flags
 *
 * \param vhwa_err_events   VHWA error event flags (input)
 * \param wd_timer_err      VHWA watchdog timer error flags (input)
 * \param tivx_err_events   Pointer to TIVX error event flags (output)
 *
 * \return VX_SUCCESS on success, VX_FAILURE if null pointer,
 *         VX_ERROR_INVALID_PARAMETERS if invalid error event mask
 */
static vx_status tivxVpacVissMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t mapped_tivx_err_events = 0u;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == tivx_err_events)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((0u != (vhwa_err_events & ~VHWA_VISS_ALL_ERR_AND_INFO_EVENTS)) ||
             (0u != (wd_timer_err & ~VHWA_VISS_WDTIMER_ERR)))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map VHWA flags to TIVX flags for error and information interrupts */
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_CFG_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_CFG_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_AEW_PULSE_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_AEW_PULSE_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_AF_PULSE_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_AF_PULSE_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_H3A_PULSE_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_H3A_PULSE_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_NSF4V_LINEMEM_CFG_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_NSF4V_LINEMEM_CFG_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_NSF4V_HBLANK_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_NSF4V_HBLANK_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_NSF4V_VBLANK_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_NSF4V_VBLANK_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_GLBCE_CFG_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_GLBCE_CFG_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_GLBCE_FILT_START_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_GLBCE_FILT_START_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_GLBCE_FILT_DONE_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_GLBCE_FILT_DONE_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_GLBCE_HSYNC_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_GLBCE_HSYNC_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_GLBCE_VSYNC_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_GLBCE_VSYNC_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_GLBCE_VP_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_GLBCE_VP_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_FCFA_CFG_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_FCFA_CFG_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_FCC_CFG_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_FCC_CFG_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_FCC_OUTIF_OVF_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_FCC_OUTIF_OVF_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_FCC_HIST_READ_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_FCC_HIST_READ_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_LSE_SL2_RD_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_LSE_SL2_RD_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_LSE_SL2_WR_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_LSE_SL2_WR_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_LSE_CAL_VP_ERR_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_LSE_CAL_VP_ERR_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_EE_CFG_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_EE_CFG_ERR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_EE_SYNCOVF_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_EE_SYNCOVF_ERR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_LSE_FR_DONE_EVT_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_LSE_FR_DONE_EVT_INTR;
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
        if (0u != (vhwa_err_events & VHWA_VISS_LSE_OUT_FR_START_EVT_INTR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_LSE_OUT_FR_START_EVT_INTR;
        }
        /* LDRA_JUSTIFY_END */
#if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_X_Y_POINTER))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_X_Y_POINTER;
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
        if (0u != (vhwa_err_events & VHWA_VISS_CR_CFG_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_CR_CFG_ERR;
        }
        /* LDRA_JUSTIFY_END */
#endif
#if defined(VPAC3L)
        if (0u != (vhwa_err_events & VHWA_VISS_RAWFE_DPC_STATS_READ_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_RAWFE_DPC_STATS_READ_ERR;
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
        /* Map VHWA flags to TIVX flags for watchdog timer errors */
        if (0u != (wd_timer_err & VHWA_VISS_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_VISS_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_END */
        *tivx_err_events = mapped_tivx_err_events;
    }

    return status;
}

/**
 * \brief Map TIVX error interrupt flags to VHWA error interrupt flags
 *
 * \param tivx_err_events   TIVX error event flags (input)
 * \param vhwa_err_events   Pointer to VHWA error event flags (output)
 * \param wd_timer_err      Pointer to VHWA watchdog timer error flags (output)
 *
 * \return VX_SUCCESS on success, VX_FAILURE if null pointers,
 *         VX_ERROR_INVALID_PARAMETERS if invalid error event mask
 */
static vx_status tivxVpacVissMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                                uint32_t *vhwa_err_events,
                                                uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == vhwa_err_events) || (NULL == wd_timer_err))
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (0u != (tivx_err_events & (~TIVX_VPAC_VISS_ALL_ERR_AND_INFO_EVENTS)))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        *vhwa_err_events = 0u;
        *wd_timer_err = 0u;

        /* Map TIVX flags to VHWA flags for error and information interrupts */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_CFG_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_CFG_ERR_INTR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_AEW_PULSE_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_AEW_PULSE_INTR;
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
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_AF_PULSE_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_AF_PULSE_INTR;
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
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_H3A_PULSE_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_H3A_PULSE_INTR;
        }
        /* LDRA_JUSTIFY_END */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_NSF4V_LINEMEM_CFG_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_NSF4V_LINEMEM_CFG_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_NSF4V_HBLANK_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_NSF4V_HBLANK_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_NSF4V_VBLANK_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_NSF4V_VBLANK_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_GLBCE_CFG_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_GLBCE_CFG_ERR_INTR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_GLBCE_FILT_START_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_GLBCE_FILT_START_INTR;
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
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_GLBCE_FILT_DONE_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_GLBCE_FILT_DONE_INTR;
        }
        /* LDRA_JUSTIFY_END */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_GLBCE_HSYNC_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_GLBCE_HSYNC_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_GLBCE_VSYNC_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_GLBCE_VSYNC_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_GLBCE_VP_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_GLBCE_VP_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_FCFA_CFG_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_FCFA_CFG_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_FCC_CFG_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_FCC_CFG_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_FCC_OUTIF_OVF_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_FCC_OUTIF_OVF_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_FCC_HIST_READ_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_FCC_HIST_READ_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_LSE_SL2_RD_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_LSE_SL2_RD_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_LSE_SL2_WR_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_LSE_SL2_WR_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_LSE_CAL_VP_ERR_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_LSE_CAL_VP_ERR_INTR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_EE_CFG_ERR))
        {
            *vhwa_err_events |= VHWA_VISS_EE_CFG_ERR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_EE_SYNCOVF_ERR))
        {
            *vhwa_err_events |= VHWA_VISS_EE_SYNCOVF_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_LSE_FR_DONE_EVT_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_LSE_FR_DONE_EVT_INTR;
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
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_LSE_OUT_FR_START_EVT_INTR))
        {
            *vhwa_err_events |= VHWA_VISS_LSE_OUT_FR_START_EVT_INTR;
        }
        /* LDRA_JUSTIFY_END */
    #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_X_Y_POINTER))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_X_Y_POINTER;
        }
        /* LDRA_JUSTIFY_END */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_CR_CFG_ERR))
        {
            *vhwa_err_events |= VHWA_VISS_CR_CFG_ERR;
        }
    #endif
    #if defined(VPAC3L)
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_RAWFE_DPC_STATS_READ_ERR))
        {
            *vhwa_err_events |= VHWA_VISS_RAWFE_DPC_STATS_READ_ERR;
        }
    #endif

        /* Map TIVX flags to VHWA flags for watchdog timer errors */
        if (0u != (tivx_err_events & TIVX_VPAC_VISS_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_VISS_WDTIMER_ERR;
        }
    }

    return status;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxVpacVissGetErrStatusCmd(const tivxVpacVissObj *vissObj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != scalar_obj_desc)
    /* LDRA_JUSTIFY_END */
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxVpacVissMapVhwaToTivxErrEvents(vissObj->err_stat, vissObj->wdTimerErrStatus, &combined_err_stat);
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
            scalar_obj_desc->data.u32 = combined_err_stat;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask from hardware\n");
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch statement <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument\n");
        status = (vx_status)VX_FAILURE;
    }

    /* LDRA_JUSTIFY_END */
    return (status);
}

static vx_status tivxVpacVissGetPsaStatusCmd(const tivxVpacVissObj *vissObj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    Vhwa_M2mVissPsaSign                 psa;
    int32_t                             fvid2_status = FVID2_SOK;
    void                                *target_ptr;
    tivx_vpac_viss_psa_timestamp_data_t *psa_status_ptr;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != usr_data_obj)
    /* LDRA_JUSTIFY_END */ 
    {
        if (sizeof(tivx_vpac_viss_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = Fvid2_control(vissObj->handle,
                VHWA_M2M_IOCTL_VISS_GET_PSA_SIGN, &psa, NULL);

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
                    "tivxVpacVissGetPsa: Fvid2_control Failed\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */   
            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_vpac_viss_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, psa.psaSign, sizeof(psa.psaSign));

            /* Add timestamp to the structure */
            psa_status_ptr->timestamp = vissObj->timestamp;

            /* Unmap the memory after copying */
            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        /* LDRA_JUSTIFY_END */

    }
/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument. Expected pointer to tivx_obj_desc_user_data_object_t\n");
        status = (vx_status)VX_FAILURE;
    }
/* LDRA_JUSTIFY_END */
    return (status);
}

static vx_status tivxVpacVissEnableErrorEventsCmd(tivxVpacVissObj *vissObj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t  fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_vpac_viss_params_t *params_ptr = NULL;
    uint32_t enableErrorEventsMask = 0u;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != usr_data_obj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(sizeof(tivx_vpac_viss_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacVissEnableErrorEventsCmd: Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_vpac_viss_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxVpacVissMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    vissObj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(vissObj->handle,
                        IOCTL_VHWA_M2M_VISS_REGISTER_ERR_CB, &vissObj->errEvtPrms, NULL);

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK == fvid2_status)
                    /* LDRA_JUSTIFY_END */
                    {
                        /* Update WDTimer configuration from application */
                        vissObj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                        vissObj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(vissObj->handle,
                            IOCTL_VHWA_M2M_VISS_REGISTER_WDTIMER_ERR_CB, &vissObj->wdTimerErrEvtPrms, NULL);
                    }
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
                            "tivxVpacVissEnableErrorEventsCmd: Fvid2_control Failed \n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}
#if !defined(VPAC3L)
static vx_status tivxVpacVissAllocReadbackBuffers(tivxVpacVissObj *vissObj, uint32_t config_reg_readback_addr)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t   fvid2_status = FVID2_SOK;

    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == vissObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxVpacVissAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (NULL == vissObj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxVpacVissAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Valid vissObj and handle */
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
        if(NULL != vissObj->configurationBuffer.bufferPtr)
        {
            vissObj->golden_reg_mem_ptr_virt.shared_ptr = (uint32_t)(uintptr_t)vissObj->configurationBuffer.bufferPtr;
            vissObj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(vissObj->golden_reg_mem_ptr_virt.shared_ptr, (int32_t)vissObj->golden_reg_mem_ptr_virt.mem_heap_region);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U == vissObj->golden_reg_mem_ptr_phys)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to get physical address of golden configuration buffer\n");
                status = (vx_status)VX_FAILURE;
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
        else
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Configuration buffer pointer is NULL\n");
        }
        /* LDRA_JUSTIFY_END */

        /* Make FVID2 control call to update the register readback structure */
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
            Vhwa_M2mVissConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ptr   = (void *)vissObj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)config_reg_readback_addr;

            /* Register the callback for config register memory comparison */
            reg_ptrs.cbFxn = tivxVpacVissConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)vissObj;

            fvid2_status = Fvid2_control(
                vissObj->handle,
                VHWA_M2M_IOCTL_VISS_SET_CONFIG_REG_PTRS,
                (void *)&reg_ptrs,
                NULL
            );

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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set VISS register readback/golden pointers\n");
                status = (vx_status)VX_FAILURE;
            }

            if ((vx_status)VX_SUCCESS != status)
            {
                tivxVpacVissFreeReadbackBuffers(vissObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            tivxVpacVissFreeReadbackBuffers(vissObj);
            status = (vx_status)VX_FAILURE;
        }
    }

    return status;
}

static void tivxVpacVissFreeReadbackBuffers(tivxVpacVissObj *vissObj)
{
    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != vissObj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = vissObj->config_reg_mem_size;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Free readback buffer if allocated */
        if (vissObj->readback_mem_ptr_phys != 0u)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxMemBufferFree(&vissObj->readback_mem_ptr_virt, readback_size);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free readback buffer\n");
            }
            vissObj->readback_mem_ptr_phys = 0u;
        }

        vissObj->config_reg_mem_size = 0u;
    }
}

static vx_status tivxEnableVpacVissSafetyMechanisms(
    tivxVpacVissObj *vissObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *params_ptr = NULL;
    int32_t fvid2_status = FVID2_SOK;
    int32_t map_status = (vx_status)VX_SUCCESS;
    uint32_t enable;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == vissObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "vissObj is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else if (NULL != usr_data_obj)
    {
        params_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == params_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get shared memory pointer\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            tivxCheckStatus(&status, tivxMemBufferMap(params_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

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
                if (sizeof(tivx_vpac_viss_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_vpac_viss_safety_mechanism_params_t *safety_params =
                        (tivx_vpac_viss_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigMMR based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_reconfig_and_reinit_Reg;
                    fvid2_status = Fvid2_control(vissObj->handle, VHWA_M2M_IOCTL_VISS_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
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
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set reconfigMMR\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */

                    /* Enable or disable status register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_status_reg_validation;
                    fvid2_status = Fvid2_control(vissObj->handle, VHWA_M2M_IOCTL_VISS_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
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
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set status register validation\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */

                    /* Enable or disable config register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    if (0U != safety_params->enable_readback_config_registers)
                    {
                        uint32_t readback_size = 0U;

                        /* Fetch the TIVX_VPAC_VISS_READBACK_SIZE from the low level driver */
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(((vx_status)VX_SUCCESS == status))
                        /* LDRA_JUSTIFY_END */
                        {
                            fvid2_status = Fvid2_control(
                                vissObj->handle,
                                VHWA_M2M_IOCTL_VISS_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            vissObj->config_reg_mem_size = readback_size;

                            if ((FVID2_SOK == fvid2_status) && (readback_size > 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "VISS readback size allocation successful: %u bytes\n", readback_size);
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
                                VX_PRINT(VX_ZONE_ERROR, "Failed to fetch VISS readback size from driver\n");
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                        }

                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(((vx_status)VX_SUCCESS == status) && (readback_size > 0U))
                        /* LDRA_JUSTIFY_END */
                        {
                            /* Before allocation, check if both of the buffers are already allocated */
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((vissObj->readback_mem_ptr_phys != 0U) && (vissObj->golden_reg_mem_ptr_phys != 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback and golden register buffer already allocated, skipping allocation\n");
                            }
                            /* LDRA_JUSTIFY_END */
                            else
                            {
                                status = tivxVpacVissAllocReadbackBuffers(vissObj, safety_params->config_reg_readback_addr);
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
                        if ((vx_status)VX_SUCCESS == status)
                        /* LDRA_JUSTIFY_END */
                        {
                            enable = (uint32_t)safety_params->enable_readback_config_registers;
                            fvid2_status = Fvid2_control(vissObj->handle, VHWA_M2M_IOCTL_VISS_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                                tivxVpacVissFreeReadbackBuffers(vissObj);
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "VISS readback buffer allocation failed, skipping config register validation enable\n");
                        }
                    }
                    else
                    {
                        enable = 0U;
                        fvid2_status = Fvid2_control(vissObj->handle, VHWA_M2M_IOCTL_VISS_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                            VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");

                            tivxVpacVissFreeReadbackBuffers(vissObj);
                            status = (vx_status)VX_FAILURE;
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
                    status = (vx_status)VX_FAILURE;
                }
                tivxCheckStatus(&map_status, tivxMemBufferUnmap(params_ptr, usr_data_obj->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if((vx_status)VX_SUCCESS != map_status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxEnableVpacVissSafetyMechanisms: tivxMemBufferUnmap Failed \n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}
#endif
/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacVissFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacVissObj *vissObj = (tivxVpacVissObj *)appData;

/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
The parameters are pre-validated by the caller before the control reaches here.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. An error print statement can be added in a future release if required.
<justification end> */
    if (NULL != vissObj)
/* LDRA_JUSTIFY_END */ 
    {
        (void)tivxEventPost(vissObj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacVissErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacVissObj *vissObj = (tivxVpacVissObj *)appData;

    if (NULL != vissObj)
    {
        vissObj->err_stat = errEvents;

        if(0U != (vissObj->err_stat & VHWA_VISS_STALLING_ERR_MASK))
        {
            (void)tivxEventPost(vissObj->waitForProcessCmpl);
        }

    }
}
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacVissWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxVpacVissObj *vissObj = (tivxVpacVissObj *)appData;
    if (NULL != vissObj)
    {
        vissObj->wdTimerErrStatus = vissObj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents;

        if(0u != vissObj->wdTimerErrStatus)
        {
            (void)tivxEventPost(vissObj->waitForProcessCmpl);
        }
    }
}
#if !defined(VPAC3L)
/* Callback for config register and golden register memory comparison for VISS */
int32_t tivxVpacVissConfigRegMemCompareCb(const Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mVissConfigRegMemParams *viss_config_reg_prms = (Vhwa_M2mVissConfigRegMemParams *)configRegPrms;
    tivxVpacVissObj *viss_obj = NULL;

    int32_t status = FVID2_SOK;

    if ((NULL != viss_config_reg_prms) && (NULL != handle))
    {
        viss_obj = (tivxVpacVissObj *)viss_config_reg_prms->appData;
    }

    if ((NULL != viss_config_reg_prms) && (NULL != handle) && (NULL != viss_obj))
    {
        /* Compare the golden register memory and readback register memory using viss_config_reg_prms */
        if ((NULL != viss_config_reg_prms->golden_ptr) &&
            (NULL != viss_config_reg_prms->readback_ptr) &&
            (viss_config_reg_prms->readback_mem_size > 0U))
        {
            int32_t cmp_result = memcmp(
                viss_config_reg_prms->golden_ptr,
                viss_config_reg_prms->readback_ptr,
                viss_config_reg_prms->readback_mem_size);

            if (0 != cmp_result)
            {
                VX_PRINT(VX_ZONE_ERROR, "VISS config register mismatch detected!\n");
                status = FVID2_EFAIL;
            }
            else
            {
                VX_PRINT(VX_ZONE_INFO, "VISS config register match successful.\n");
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid parameters for VISS config register validation\n");
            status = FVID2_EBADARGS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer in VISS config register validation callback\n");
        status = FVID2_EBADARGS;
    }

    return status;
}
/* LDRA_JUSTIFY_END */
#endif
BUILD_ASSERT((sizeof(Vhwa_M2mVissPsaSign) == sizeof(((tivx_vpac_viss_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);
