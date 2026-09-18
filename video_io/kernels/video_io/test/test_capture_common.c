/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/video_io_kernels.h>
#include <TI/video_io_capture.h>
#include <TI/video_io_display.h>
#include "test_engine/test.h"
#include <TI/tivx_config.h>
#include <string.h>
#include "tivx_utils_file_rd_wr.h"
#include <TI/tivx_task.h>
#include "math.h"
#include <limits.h>
#include "test_tiovx/test_tiovx.h"
#include "tivx_utils_file_rd_wr.h"

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include "test_video_io_common.h"
#include <utils/iss/include/app_iss.h>


#if defined(LDRA_COVERAGE_ENABLED_VIDEO_IO)

#include "ldra_remote_core_coverage_main.h"

/** 
* \brief Function to start or stop coverage capture for a video capture node
*
*        This function creates a minimal OpenVX graph with a capture node
*        and configures it using dummy RAW image parameters. It is intended
*        for triggering coverage instrumentation on both target and host.
*
*        When start_stop is set to 1, coverage collection is started by
*        sending a command to the capture node and initializing the host
*        coverage tool. Otherwise, coverage collection is stopped by sending
*        the corresponding command and terminating the host coverage tool.
*
*        The function internally handles creation, verification, and cleanup
*        of all OpenVX resources used for this purpose.
*
* \param start_stop [in] Flag to control coverage capture
*                       - 1U : Start coverage capture
*                       - otherwise : Stop coverage capture
*
*/
void video_io_capture_coverage(uint32_t start_stop)
{
    /* Graph objects */
    vx_context local_context;
    vx_graph graph;
    vx_user_data_object config;
    tivx_raw_image raw_image = 0;
    tivx_raw_image black_raw_image = 0;
    tivx_capture_params_t params;
    tivx_raw_image_create_params_t dummy_raw_params;
    vx_object_array arr;
    vx_node node;

    local_context = vxCreateContext();

    tivxVideoIOLoadKernels(local_context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(local_context), VX_TYPE_GRAPH);


    tivx_capture_params_init(&params);

    params.numInst = 1;
    params.numCh = 1;

    params.timeoutInitial = 1;
    params.timeout = 1;

    ASSERT_VX_OBJECT(config = vxCreateUserDataObject(local_context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&dummy_raw_params, 0, sizeof(dummy_raw_params));

    dummy_raw_params.width  = 1920;
    dummy_raw_params.height = 1080;

    dummy_raw_params.num_exposures = 1;

    /* 12-bit RAW typical */
    dummy_raw_params.line_interleaved = vx_false_e;

    /* exposure 0 format */
    dummy_raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    dummy_raw_params.format[0].msb = 11;  /* 12-bit stored in 16-bit */

    /* Bayer pattern */
    dummy_raw_params.meta_height_before = 0;
    dummy_raw_params.meta_height_after  = 0;
    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(local_context, &dummy_raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    ASSERT_VX_OBJECT(arr = vxCreateObjectArray(local_context, (vx_reference)raw_image, 1), VX_TYPE_OBJECT_ARRAY);

    ASSERT_VX_OBJECT(node = tivxCaptureNode(graph, config, arr), VX_TYPE_NODE);

    vx_status status = vxVerifyGraph(graph);
    printf("Verify status = %d\n", status);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, status);

    ASSERT_VX_OBJECT(black_raw_image = tivxCreateRawImage(local_context, &dummy_raw_params), TIVX_TYPE_RAW_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(node, (vx_reference)black_raw_image));

    
    if (1U == start_stop)
    {
        /* Start coverage capture at TARGET */
        status = tivxNodeSendCommand(node, 0, TIVX_CAPTURE_COVERAGE_START, NULL, 0);

        tivxTaskWaitMsecs(2000U);

        /* Start coverage capture at HOST */
        printf("################## LDRA INIT START  ##################\n");
        ldra_initialize();
        printf("################## LDRA INIT FINISH ##################\n");
    }
    else
    {
        /* Stop coverage capture at TARGET */
        status = tivxNodeSendCommand(node, 0, TIVX_CAPTURE_COVERAGE_END, NULL, 0);
        
        tivxTaskWaitMsecs(2000U);
        
        /* Stop coverage capture at HOST */
        printf("################## LDRA TERMINATION  START ##################\n");
        ldra_terminate();
        printf("################## LDRA TERMINATION FINISH ##################\n");
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
    vxReleaseObjectArray(&arr);
    tivxReleaseRawImage(&raw_image);
    tivxReleaseRawImage(&black_raw_image);
    vxReleaseUserDataObject(&config);

    tivxVideoIOUnLoadKernels(local_context);
    vxReleaseContext(&local_context);
}
#endif /* LDRA_COVERAGE_ENABLED_VIDEO_IO */

#endif /* BUILD_CAPTURE */
