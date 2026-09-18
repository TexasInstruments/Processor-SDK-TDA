/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#ifdef BUILD_CAPTURE

#include "TI/tivx.h"
#include "TI/video_io_capture.h"
#include "tivx_video_io_kernels.h"
#include "tivx_kernel_capture.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_video_io_host_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_ext_raw_image.h"
#include <TI/tivx_mutex.h>
#include <VX/vx_khr_safe_casts.h>

static vx_kernel vx_capture_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelCaptureValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelCaptureInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelCaptureValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    (void)node;
    (void)metas;
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object input = NULL;
    vx_object_array output = NULL;
    vx_char input_name[VX_MAX_REFERENCE_NAME];
    vx_size input_size, output_num_items;
    vx_reference obj_arr_element;
    vx_df_image img_fmt;
    vx_enum ref_type;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the
    error status and return the same to the application.
    However, due to the stated rationale, this is not tested. 
    <justification end> */
    if ( (num != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */

    if ((vx_status)VX_SUCCESS == status)
    {
        input  = vxCastRefAsUserDataObject(parameters[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX], NULL);
        output = vxCastRefAsObjectArray(parameters[TIVX_KERNEL_CAPTURE_OUTPUT_IDX], NULL);
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(input, (vx_enum)VX_USER_DATA_OBJECT_NAME, &input_name, sizeof(input_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(input, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &input_size, sizeof(input_size)));

        tivxCheckStatus(&status, vxQueryObjectArray(output, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &output_num_items, sizeof(output_num_items)));

    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((input_size != sizeof(tivx_capture_params_t)) ||
            (strncmp(input_name, "tivx_capture_params_t", sizeof(input_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be a user_data_object of type:\n tivx_capture_params_t \n");
        }

    }

    if ((vx_status)VX_SUCCESS == status)
    {
        obj_arr_element = vxGetObjectArrayItem(output, 0);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != obj_arr_element)
        /* LDRA_JUSTIFY_END */
        {
            tivxCheckStatus(&status, vxQueryReference(obj_arr_element, (vx_enum)VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type)));

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                if ( (TIVX_TYPE_RAW_IMAGE != ref_type) &&
                     ((vx_enum)VX_TYPE_IMAGE != ref_type) )
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "output object array must contain either TIVX_TYPE_RAW_IMAGE or VX_TYPE_IMAGE \n");
                }
                else if ((vx_enum)VX_TYPE_IMAGE == ref_type)
                {
                    tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(obj_arr_element, NULL), (vx_enum)VX_IMAGE_FORMAT, &img_fmt, sizeof(img_fmt)));
                    if (((vx_df_image)VX_DF_IMAGE_RGBX != img_fmt) &&
                        ((vx_df_image)TIVX_DF_IMAGE_BGRX != img_fmt) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 != img_fmt) &&
                        ((vx_df_image)VX_DF_IMAGE_UYVY != img_fmt) &&
                        ((vx_df_image)VX_DF_IMAGE_YUYV != img_fmt))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "image format is invalid \n");
                        VX_PRINT(VX_ZONE_ERROR, "image format should be an image of type:\n VX_DF_IMAGE_RGBX or VX_DF_IMAGE_RGBX or VX_DF_IMAGE_U16 or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV\n");
                    }
                }
                else
                {
                    /* do nothing */
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "query 'output' object array reference failed \n");
            }
            /* LDRA_JUSTIFY_END */
            vx_status release_status = vxReleaseReference(&obj_arr_element);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != release_status)
            {
                status = release_status;
                VX_PRINT(VX_ZONE_ERROR, "Cannot release reference obj_arr_element \n");
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' object array elements are NULL \n");
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelCaptureInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    (void)node;
    vx_status status = (vx_status)VX_SUCCESS;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to print the error status.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */
    return status;
}

vx_status tivxAddKernelCapture(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_CAPTURE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_CAPTURE_MAX_PARAMS,
                    tivxAddKernelCaptureValidate,
                    tivxAddKernelCaptureInitialize,
                    NULL);

        status = vxGetStatus(vxCastRefFromKernel(kernel));
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_uint32 num_bufs = TIVX_CAPTURE_MIN_PIPEUP_BUFS;

        status = vxSetKernelAttribute(kernel, (vx_enum)VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &num_bufs, sizeof(num_bufs));

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the video_io test framework.
        Effect on this unit: If the control reaches here, the code base is NOT expected to
        accumulate and return an error. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            status = vxGetStatus(vxCastRefFromKernel(kernel));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the video_io test framework.
        Effect on this unit: If the control reaches here, the code base is NOT expected to
        accumulate and return an error. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxSetKernelAttribute error! \n");
        }
        /* LDRA_JUSTIFY_END */
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        idx = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the
        error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        /* LDRA_JUSTIFY_END */
        {
            status = vxAddParameterToKernel(kernel,
                        idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the
        error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        /* LDRA_JUSTIFY_END */
        {
            /* add supported target's */
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE1);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE2);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE3);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE4);
            # if !defined(SOC_AM62A) && !defined(SOC_J722S)
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE5);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE6);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE7);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE8);
            #if defined(SOC_J784S4) || defined(SOC_J742S2)
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE9);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE10);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE11);
            (void)tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE12);
            #endif
            #endif
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the
        error status and return the same to the application. However, due to the stated rationale,
        this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        /* LDRA_JUSTIFY_END */
        {
            status = vxFinalizeKernel(kernel);
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            (void)vxReleaseKernel(&kernel);
            kernel = NULL;
        }
        /* LDRA_JUSTIFY_END */
    }
    else
    {
        kernel = NULL;
    }
    vx_capture_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelCapture(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_capture_kernel;

    status = vxRemoveKernel(kernel);
    vx_capture_kernel = NULL;

    return status;
}

void tivx_capture_params_init(tivx_capture_params_t *prms)
{
    uint32_t cnt, loopCnt;

    if (NULL != prms)
    {
        prms->numInst            = 1u;
        prms->numCh              = 0u;
        prms->timeout            = VX_TIMEOUT_WAIT_FOREVER;
        prms->timeoutInitial     = VX_TIMEOUT_WAIT_FOREVER;

        for (loopCnt = 0U ; loopCnt < TIVX_CAPTURE_MAX_INST ; loopCnt++)
        {
            prms->instId[loopCnt]                       = loopCnt;
            prms->instCfg[loopCnt].enableCsiv2p0Support = (uint32_t)vx_true_e;
            prms->instCfg[loopCnt].numDataLanes         = 4u;
            prms->instCfg[loopCnt].dataLanesMap[0u]     = 1u;
            prms->instCfg[loopCnt].dataLanesMap[1u]     = 2u;
            prms->instCfg[loopCnt].dataLanesMap[2u]     = 3u;
            prms->instCfg[loopCnt].dataLanesMap[3u]     = 4u;
            prms->instCfg[loopCnt].laneBandSpeed        = TIVX_CAPTURE_LANE_BAND_SPEED_1350_TO_1500_MBPS;
            prms->instCfg[loopCnt].numPixels            = 0U;
        }
        for (cnt = 0u; cnt < TIVX_CAPTURE_MAX_CH; cnt ++)
        {
            prms->chVcNum[cnt]   = cnt;
            prms->chInstMap[cnt] = 0U;
        }
    }
}

/* Compares provided ref to existing ref to validate that it contains the same properties */
static vx_status tivxCaptureValidateAllocFrame(vx_node node, vx_reference frame)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference node_ref;
    vx_parameter param = NULL;

    param = vxGetParameterByIndex(node, TIVX_KERNEL_CAPTURE_OUTPUT_IDX);

    if (NULL != param)
    {
        (void)vxQueryParameter(param, (vx_enum)VX_PARAMETER_REF, &node_ref, sizeof(node_ref));

        if (NULL != node_ref)
        {
            vx_object_array output = vxCastRefAsObjectArray(node_ref, NULL);
            vx_reference obj_arr_element;
            vx_enum node_ref_type, recv_ref_type;

            obj_arr_element = vxGetObjectArrayItem(output, 0);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != obj_arr_element)
            /* LDRA_JUSTIFY_END */
            {
                tivxCheckStatus(&status, vxQueryReference(obj_arr_element, (vx_enum)VX_REFERENCE_TYPE, &node_ref_type, sizeof(node_ref_type)));
                tivxCheckStatus(&status, vxQueryReference(frame, (vx_enum)VX_REFERENCE_TYPE, &recv_ref_type, sizeof(recv_ref_type)));

                if ((vx_status)VX_SUCCESS == status)
                {
                    if ( recv_ref_type != node_ref_type )
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "frame reference type is invalid!! \n");
                    }
                    else
                    {
                        if ((vx_enum)VX_TYPE_IMAGE == node_ref_type)
                        {
                            vx_df_image node_img_fmt,    recv_img_fmt;
                            vx_uint32   node_img_width,  recv_img_width;
                            vx_uint32   node_img_height, recv_img_height;

                            tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(obj_arr_element, NULL), (vx_enum)VX_IMAGE_FORMAT, &node_img_fmt, sizeof(node_img_fmt)));
                            tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(obj_arr_element, NULL), (vx_enum)VX_IMAGE_WIDTH, &node_img_width, sizeof(node_img_width)));
                            tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(obj_arr_element, NULL), (vx_enum)VX_IMAGE_HEIGHT, &node_img_height, sizeof(node_img_height)));

                            tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(frame, NULL), (vx_enum)VX_IMAGE_FORMAT, &recv_img_fmt, sizeof(recv_img_fmt)));
                            tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(frame, NULL), (vx_enum)VX_IMAGE_WIDTH, &recv_img_width, sizeof(recv_img_width)));
                            tivxCheckStatus(&status, vxQueryImage(vxCastRefAsImage(frame, NULL), (vx_enum)VX_IMAGE_HEIGHT, &recv_img_height, sizeof(recv_img_height)));

                            if (node_img_fmt != recv_img_fmt)
                            {
                                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                                VX_PRINT(VX_ZONE_ERROR, "frame reference format is invalid \n");
                            }

                            if (node_img_width != recv_img_width)
                            {
                                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                                VX_PRINT(VX_ZONE_ERROR, "frame reference width is invalid \n");
                            }

                            if (node_img_height != recv_img_height)
                            {
                                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                                VX_PRINT(VX_ZONE_ERROR, "frame reference height is invalid \n");
                            }
                        }
                        else if ((vx_enum)TIVX_TYPE_RAW_IMAGE == node_ref_type)
                        {
                            vx_uint32   node_img_width,  recv_img_width;
                            vx_uint32   node_img_height, recv_img_height;

                            tivxCheckStatus(&status, tivxQueryRawImage(vxCastRefAsRawImage(obj_arr_element, NULL), (vx_enum)TIVX_RAW_IMAGE_WIDTH, &node_img_width, sizeof(node_img_width)));
                            tivxCheckStatus(&status, tivxQueryRawImage(vxCastRefAsRawImage(obj_arr_element, NULL), (vx_enum)TIVX_RAW_IMAGE_HEIGHT, &node_img_height, sizeof(node_img_height)));

                            tivxCheckStatus(&status, tivxQueryRawImage(vxCastRefAsRawImage(frame, NULL), (vx_enum)TIVX_RAW_IMAGE_WIDTH, &recv_img_width, sizeof(recv_img_width)));
                            tivxCheckStatus(&status, tivxQueryRawImage(vxCastRefAsRawImage(frame, NULL), (vx_enum)TIVX_RAW_IMAGE_HEIGHT, &recv_img_height, sizeof(recv_img_height)));

                            if (node_img_width != recv_img_width)
                            {
                                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                                VX_PRINT(VX_ZONE_ERROR, "frame reference width is invalid \n");
                            }

                            if (node_img_height != recv_img_height)
                            {
                                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                                VX_PRINT(VX_ZONE_ERROR, "frame reference height is invalid \n");
                            }
                        }
                        else
                        {
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                            VX_PRINT(VX_ZONE_ERROR, "invalid reference object type \n");
                        }
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query object failed \n");
                }

                (void)vxReleaseObjectArray(&output);
                (void)vxReleaseReference(&obj_arr_element);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output' object array elements are NULL \n");
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Capture node index %d is NULL!!\n", TIVX_KERNEL_CAPTURE_OUTPUT_IDX);
        }

        (void)vxReleaseParameter(&param);
    }
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "Capture node index %d is NULL!!\n", TIVX_KERNEL_CAPTURE_OUTPUT_IDX);
    }

    return status;
}

static vx_status tivxCaptureAllocFrame(vx_reference frame)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum ref_type;

    tivxCheckStatus(&status, vxQueryReference(frame, (vx_enum)VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type)));
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status
    and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        if ((vx_enum)VX_TYPE_IMAGE == ref_type)
        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;

            (void)vxQueryImage(vxCastRefAsImage(frame, NULL), (vx_enum)VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            (void)vxQueryImage(vxCastRefAsImage(frame, NULL), (vx_enum)VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;

            status = vxMapImagePatch(vxCastRefAsImage(frame, NULL), &rect, 0, &map_id, &image_addr, &data_ptr, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

            if ((vx_status)VX_SUCCESS == status)
            {
                status = vxUnmapImagePatch(vxCastRefAsImage(frame, NULL), map_id);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate capture frame\n");
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if ((vx_enum)TIVX_TYPE_RAW_IMAGE == ref_type)
        /* LDRA_JUSTIFY_END */
        {
            vx_map_id map_id;
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;

            (void)tivxQueryRawImage(vxCastRefAsRawImage(frame, NULL), (vx_enum)TIVX_RAW_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            (void) tivxQueryRawImage(vxCastRefAsRawImage(frame, NULL), (vx_enum)TIVX_RAW_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;

            status = tivxMapRawImagePatch(vxCastRefAsRawImage(frame, NULL), &rect, 0, &map_id, &image_addr, &data_ptr, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)TIVX_RAW_IMAGE_ALLOC_BUFFER);

            if ((vx_status)VX_SUCCESS == status)
            {
                status = tivxUnmapRawImagePatch(vxCastRefAsRawImage(frame, NULL), map_id);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate capture frame\n");
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        tivxCaptureValidateAllocFrame() only allows frame's reference type to reach
        tivxCaptureAllocFrame() when it is VX_TYPE_IMAGE or TIVX_TYPE_RAW_IMAGE; any
        other type is rejected with VX_ERROR_INVALID_PARAMETERS before this function
        is ever invoked.
        Effect on this unit: status is set to VX_FAILURE and an error message is
        printed for an unsupported reference type.
        <justification end> */
        else
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Unsupported reference type\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

vx_status tivxCaptureRegisterErrorFrame(vx_node node, vx_reference frame)
{
    vx_status status;
    vx_reference ref[1];

    status = tivxCaptureValidateAllocFrame(node, frame);

    if ((vx_status)VX_SUCCESS == status)
    {
        ref[0] = frame;

        status = tivxCaptureAllocFrame(frame);

        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxNodeSendCommand(node, 0,
                TIVX_CAPTURE_REGISTER_ERROR_FRAME, ref, 1u);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Reference could not be allocated\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
    }

    return status;
}

#endif /* BUILD_CAPTURE */