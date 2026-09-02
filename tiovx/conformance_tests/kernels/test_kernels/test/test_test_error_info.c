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

#include "test_engine/test.h"
#include "test_tiovx.h"

#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>
#include "tivx_kernel_test_error_info.h"
#include <string.h>
#include <TI/tivx_config.h>

#define ERROR_INFO_TEST_APP_VALUE (7u)
#define ERROR_INFO_OFFSET (8u)
#define ERROR_INFO_TEST_SIZE (4u)

TESTCASE(tivxTestKernelErrorInfo, CT_VXContext, ct_setup_vx_context, 0)

/*
 * tivxTestErrorInfoNode's target-side process() callback calls
 * tivxSetTargetKernelInstanceErrorInfo() and always returns non success.
 * This allows the Node Parameters, a custom value and a size, to cross from the target kernel's process(),
 * through the object-descriptor/IPC path, back to the host.
 */
TEST(tivxTestKernelErrorInfo, testErrorInfo)
{
    vx_graph graph;
    vx_node node;
    vx_scalar error_info_value;
    vx_scalar error_info_size;
    vx_uint64 test_value = 0x000000DEADBEEF00ULL;
    vx_uint16 test_size = ERROR_INFO_TEST_SIZE;
    vx_uint32 i;
    vx_event_t event = {0};
    vx_context context = context_->vx_context_;

    tivxTestKernelsLoadKernels(context);

    test_value = test_value >> ERROR_INFO_OFFSET;
    ASSERT_VX_OBJECT(graph             = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(error_info_value  = vxCreateScalar(context, VX_TYPE_UINT64, &test_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(error_info_size   = vxCreateScalar(context, VX_TYPE_UINT16, &test_size), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(node              = tivxTestErrorInfoNode(graph, error_info_value, error_info_size), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    VX_CALL(vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, ERROR_INFO_TEST_APP_VALUE));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    /* vxProcessGraph() still returns VX_SUCCESS at the graph level even if target kernel fails.
     * The failure is only observable via the VX_EVENT_NODE_ERROR event below. */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitEvent(context, &event, vx_false_e));
    ASSERT_EQ_INT(event.type, VX_EVENT_NODE_ERROR);
    ASSERT_EQ_INT(event.app_value, ERROR_INFO_TEST_APP_VALUE);
    ASSERT_EQ_VX_STATUS(VX_FAILURE, event.event_info.node_error.status);
    ASSERT_EQ_INT(event.event_info.node_error.error_info_size, test_size);
    ASSERT(0 == memcmp(event.event_info.node_error.error_info, &test_value, test_size));

    /* Diagnostic prints
    printf("Event error_info_size: %u\nEvent error as 64bit int = 0x%016" PRIx64 "\nEvent error_info byte array:\n",
        (unsigned)event.event_info.node_error.error_info_size, (*(uint64_t *)event.event_info.node_error.error_info));
    for (i = event.event_info.node_error.error_info_size; i > 0; i--)
    {
        printf("  [%d]: 0x%02x\n", i - 1, event.event_info.node_error.error_info[i - 1]);
    }
    printf("\n");
    */

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseScalar(&error_info_value));
    VX_CALL(vxReleaseScalar(&error_info_size));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TEST(tivxTestKernelErrorInfo, testErrorInfoReplicate)
{
    vx_graph graph;
    vx_node node;
    vx_object_array error_info_val_array;
    vx_scalar temp_scalar_exemplar, value_placeholder_scalar;
    vx_scalar error_info_size;
    vx_bool replicate[TIVX_KERNEL_TEST_ERROR_INFO_MAX_PARAMS] = { vx_true_e, vx_false_e };
    vx_uint64 test_values[TIVX_NODE_MAX_REPLICATE];
    vx_uint64 exemplar_value = 0;
    vx_uint16 test_size = ERROR_INFO_TEST_SIZE;
    vx_bool replica_seen[TIVX_NODE_MAX_REPLICATE] = {0};
    vx_uint32 i;
    vx_event_t event = {0};
    vx_context context = context_->vx_context_;

    tivxTestKernelsLoadKernels(context);

    /* Making unique values for each replica based on their index */
    for (i = 0; i < TIVX_NODE_MAX_REPLICATE; i++)
    {
        test_values[i] = 0x00000000DEADBE00ULL | (vx_uint64)i;
    }

    /* Make object array of error_info values for replicate */
    ASSERT_VX_OBJECT(graph            = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(temp_scalar_exemplar   = vxCreateScalar(context, VX_TYPE_UINT64, &exemplar_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(error_info_val_array = vxCreateObjectArray(context,
        (vx_reference)temp_scalar_exemplar, TIVX_NODE_MAX_REPLICATE), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseScalar(&temp_scalar_exemplar));

    /* Set each error_info value in the obj_array to the unique ones made earlier */
    for (i = 0; i < TIVX_NODE_MAX_REPLICATE; i++)
    {
        ASSERT_VX_OBJECT(value_placeholder_scalar = (vx_scalar)vxGetObjectArrayItem(error_info_val_array, i), VX_TYPE_SCALAR);
        VX_CALL(vxCopyScalar(value_placeholder_scalar, &test_values[i], VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxReleaseScalar(&value_placeholder_scalar));
    }

    ASSERT_VX_OBJECT(error_info_size  = vxCreateScalar(context, VX_TYPE_UINT16, &test_size), VX_TYPE_SCALAR);

    /* Prep the first node to be replicated */
    ASSERT_VX_OBJECT(value_placeholder_scalar = (vx_scalar)vxGetObjectArrayItem(error_info_val_array, 0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(node                     = tivxTestErrorInfoNode(graph, value_placeholder_scalar, error_info_size), VX_TYPE_NODE);
    VX_CALL(vxReleaseScalar(&value_placeholder_scalar));

    /* Replicate */
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
    VX_CALL(vxReplicateNode(graph, node, replicate, TIVX_KERNEL_TEST_ERROR_INFO_MAX_PARAMS));

    VX_CALL(vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, ERROR_INFO_TEST_APP_VALUE));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    /* One event per failing replica -- collect all of them and check each
     * replica's own value showed up exactly once, without assuming a
     * specific delivery order across replicas. */
    for (i = 0; i < TIVX_NODE_MAX_REPLICATE; i++)
    {
        vx_uint16 idx;

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitEvent(context, &event, vx_false_e));
        ASSERT_EQ_INT(event.type, VX_EVENT_NODE_ERROR);
        ASSERT_EQ_VX_STATUS(VX_FAILURE, event.event_info.node_error.status);
        ASSERT_EQ_INT(event.event_info.node_error.error_info_size, test_size);

        idx = event.event_info.node_error.replicated_node_idx;
        ASSERT(idx < TIVX_NODE_MAX_REPLICATE);
        ASSERT(vx_false_e == replica_seen[idx]);
        replica_seen[idx] = vx_true_e;

        ASSERT(0 == memcmp(event.event_info.node_error.error_info, &test_values[idx], test_size));
    }

    for (i = 0; i < TIVX_NODE_MAX_REPLICATE; i++)
    {
        ASSERT(vx_true_e == replica_seen[i]);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseObjectArray(&error_info_val_array));
    VX_CALL(vxReleaseScalar(&error_info_size));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

/*
 * tivxSetTargetKernelInstanceErrorInfo() writes through
 * target_kernel_instance->node_obj_desc, which ownTargetNodeDescNodeExecuteTargetKernel
 * now re-points to the actually-executing obj_desc[pipeline_id] on every call, instead
 * of staying pinned to obj_desc[0] from node-create time.
 */

#define ERROR_INFO_PIPE_DEPTH (2u)
#define ERROR_INFO_PIPE_NUM_BUF (2u)
#define ERROR_INFO_PIPE_ITERATIONS (10u)

TEST(tivxTestKernelErrorInfo, testErrorInfoPipeline)
{
    vx_graph graph;
    vx_node node;
    vx_scalar error_info_test_scalars[ERROR_INFO_PIPE_NUM_BUF];
    vx_scalar error_info_size, done_scalar;
    vx_parameter value_param;
    vx_graph_parameter_queue_params_t queue_params_list[1];
    vx_uint64 error_info_test_int;
    vx_uint16 test_size = ERROR_INFO_TEST_SIZE;
    vx_uint32 i, buf_id, num_refs;
    vx_event_t event = {0};
    vx_context context = context_->vx_context_;
    vx_uint32 context_event_timeout_val = 1000;

    tivxTestKernelsLoadKernels(context);
    ASSERT_VX_OBJECT(graph           = vxCreateGraph(context), VX_TYPE_GRAPH);
    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_EVENT_TIMEOUT, &context_event_timeout_val, sizeof(context_event_timeout_val)));

    ASSERT_VX_OBJECT(error_info_size = vxCreateScalar(context, VX_TYPE_UINT16, &test_size), VX_TYPE_SCALAR);

    /* Making unique values for each pipelined node based on their obj_desc index. */
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        error_info_test_int = 0xDEADBEEF0ULL | (vx_uint64)buf_id;
        ASSERT_VX_OBJECT(error_info_test_scalars[buf_id] = vxCreateScalar(context, VX_TYPE_UINT64, &error_info_test_int), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(node = tivxTestErrorInfoNode(graph, error_info_test_scalars[0], error_info_size), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
    VX_CALL(vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, ERROR_INFO_TEST_APP_VALUE));

    /* Enable Pipelining */
    /* Expose the error_info input scalar param as graph param */
    ASSERT_VX_OBJECT(value_param = vxGetParameterByIndex(node, TIVX_KERNEL_TEST_ERROR_INFO_VALUE_IDX), VX_TYPE_PARAMETER);
    VX_CALL(vxAddParameterToGraph(graph, value_param));
    VX_CALL(vxReleaseParameter(&value_param));

    queue_params_list[0].graph_parameter_index = 0;
    queue_params_list[0].refs_list_size = ERROR_INFO_PIPE_NUM_BUF;
    queue_params_list[0].refs_list = (vx_reference*)&error_info_test_scalars[0];

    VX_CALL(vxSetGraphScheduleConfig(graph, VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO, 1, queue_params_list));
    VX_CALL(tivxSetGraphPipelineDepth(graph, ERROR_INFO_PIPE_DEPTH));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /* Triggers Pipeline execution via Enqueue (Auto mode) */
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&error_info_test_scalars[buf_id], 1));
    }

    /* Recycle for iterations */
    for (vx_uint32 iter = 0; iter < ERROR_INFO_PIPE_ITERATIONS; iter++)
    {
        error_info_test_int = 0xDEADBEEF0ULL | (vx_uint64)(iter % ERROR_INFO_PIPE_NUM_BUF);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitEvent(context, &event, vx_false_e));
        ASSERT_EQ_INT(event.type, VX_EVENT_NODE_ERROR);
        ASSERT_EQ_VX_STATUS(VX_FAILURE, event.event_info.node_error.status);
        ASSERT_EQ_INT(event.event_info.node_error.error_info_size, test_size);
        ASSERT(0 == memcmp(event.event_info.node_error.error_info, &error_info_test_int, test_size));

        /* done value gets overwritten with what was dequeued */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&done_scalar, 1, &num_refs));

        /* Only recycle the buffer if there are more than PIPE_NUM_BUF iterations left.
         * Last PIPE_NUM_BUF iterations don't need to recycle */
        if (iter < (ERROR_INFO_PIPE_ITERATIONS - ERROR_INFO_PIPE_NUM_BUF))
        {
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&done_scalar, 1));
        }
    }

    /* Clean up - every enqueued buffer was already dequeued above, nothing left in flight */
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&error_info_test_scalars[buf_id]));
    }
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseScalar(&error_info_size));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TEST(tivxTestKernelErrorInfo, testErrorInfoReplicateAndPipeline)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node;

    vx_scalar error_info_test_scalars[ERROR_INFO_PIPE_NUM_BUF];
    vx_object_array error_info_val_arrays[ERROR_INFO_PIPE_NUM_BUF];
    vx_scalar error_value_exemplar, value_placeholder_scalar;
    vx_scalar error_info_size, done_scalar;

    vx_parameter value_param;
    vx_graph_parameter_queue_params_t queue_params_list[1];
    vx_uint32 i, buf_id, num_refs;

    vx_event_t event = {0};
    vx_bool replicate[TIVX_KERNEL_TEST_ERROR_INFO_MAX_PARAMS] = { vx_true_e, vx_false_e };
    vx_uint32 context_event_timeout_val = 1000;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph           = vxCreateGraph(context), VX_TYPE_GRAPH);
    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_EVENT_TIMEOUT, &context_event_timeout_val, sizeof(context_event_timeout_val)));

    vx_uint16 test_size = ERROR_INFO_TEST_SIZE;
    ASSERT_VX_OBJECT(error_info_size = vxCreateScalar(context, VX_TYPE_UINT16, &test_size), VX_TYPE_SCALAR);

    /* Make object array of error_info values for replicate */
    vx_uint64 exemplar_value = 0;
    ASSERT_VX_OBJECT(error_value_exemplar = vxCreateScalar(context, VX_TYPE_UINT64, &exemplar_value), VX_TYPE_SCALAR);
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        ASSERT_VX_OBJECT(error_info_val_arrays[buf_id] = vxCreateObjectArray(context,
            (vx_reference)error_value_exemplar, TIVX_NODE_MAX_REPLICATE), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(error_info_test_scalars[buf_id] = (vx_scalar)vxGetObjectArrayItem(
            error_info_val_arrays[buf_id], 0), VX_TYPE_SCALAR);
    }
    VX_CALL(vxReleaseScalar(&error_value_exemplar));

    /* Set each error_info value in the obj_array to the unique ones made earlier */
    vx_uint64 error_info_test_int;
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        for (i = 0; i < TIVX_NODE_MAX_REPLICATE; i++)
        {
            error_info_test_int = TIVX_NODE_MAX_REPLICATE * buf_id + i;
            ASSERT_VX_OBJECT(value_placeholder_scalar = (vx_scalar)vxGetObjectArrayItem(error_info_val_arrays[buf_id], i), VX_TYPE_SCALAR);
            VX_CALL(vxCopyScalar(value_placeholder_scalar, &error_info_test_int, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxReleaseScalar(&value_placeholder_scalar));
        }
    }

    ASSERT_VX_OBJECT(node = tivxTestErrorInfoNode(graph, error_info_test_scalars[0], error_info_size), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
    VX_CALL(vxReplicateNode(graph, node, replicate, TIVX_KERNEL_TEST_ERROR_INFO_MAX_PARAMS));

    VX_CALL(vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, ERROR_INFO_TEST_APP_VALUE));

    /* Enable Pipelining */
    /* Expose the error_info input scalar param as graph param */
    ASSERT_VX_OBJECT(value_param = vxGetParameterByIndex(node, TIVX_KERNEL_TEST_ERROR_INFO_VALUE_IDX), VX_TYPE_PARAMETER);
    VX_CALL(vxAddParameterToGraph(graph, value_param));
    VX_CALL(vxReleaseParameter(&value_param));

    queue_params_list[0].graph_parameter_index = 0;
    queue_params_list[0].refs_list_size = ERROR_INFO_PIPE_NUM_BUF;
    queue_params_list[0].refs_list = (vx_reference*)&error_info_test_scalars[0];

    VX_CALL(vxSetGraphScheduleConfig(graph, VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO, 1, queue_params_list));
    VX_CALL(tivxSetGraphPipelineDepth(graph, ERROR_INFO_PIPE_DEPTH));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    /* Triggers Pipeline execution via Enqueue (Auto mode) */
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&error_info_test_scalars[buf_id], 1));
    }

    /* Recycle for iterations */
    for (vx_uint32 iter = 0; iter < ERROR_INFO_PIPE_ITERATIONS; iter++)
    {
        for (i = 0; i < TIVX_NODE_MAX_REPLICATE; i++)
        {
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitEvent(context, &event, vx_false_e));
            ASSERT_EQ_INT(event.type, VX_EVENT_NODE_ERROR);
            ASSERT_EQ_VX_STATUS(VX_FAILURE, event.event_info.node_error.status);
            ASSERT_EQ_INT(event.event_info.node_error.error_info_size, test_size);

            vx_uint16 idx = event.event_info.node_error.replicated_node_idx;
            ASSERT(idx < TIVX_NODE_MAX_REPLICATE);
            error_info_test_int = idx + TIVX_NODE_MAX_REPLICATE * (vx_uint64)((iter % ERROR_INFO_PIPE_NUM_BUF));
            ASSERT(0 == memcmp(event.event_info.node_error.error_info, &error_info_test_int, test_size));
        }
        
        /* done value gets overwritten with what was dequeued */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&done_scalar, 1, &num_refs));

        /* Only recycle the buffer if there are more than PIPE_NUM_BUF iterations left.
	     * Last PIPE_NUM_BUF iterations don't need to recycle */
        if (iter < (ERROR_INFO_PIPE_ITERATIONS - ERROR_INFO_PIPE_NUM_BUF))
        {
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&done_scalar, 1));
        }
    }

    /* Clean up - every enqueued buffer was already dequeued above, nothing left in flight */
    for (buf_id = 0; buf_id < ERROR_INFO_PIPE_NUM_BUF; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&error_info_test_scalars[buf_id]));
        VX_CALL(vxReleaseObjectArray(&error_info_val_arrays[buf_id]));
    }
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseScalar(&error_info_size));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxTestKernelErrorInfo, testErrorInfo, testErrorInfoReplicate, testErrorInfoPipeline, testErrorInfoReplicateAndPipeline)
