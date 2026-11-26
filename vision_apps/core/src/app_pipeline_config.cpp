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

#include "app_pipeline_config.h"
#include <stdio.h>

/* ========================================================================== */
/*                  PipelineConfig class method implementations               */
/* ========================================================================== */

PipelineConfig::PipelineConfig(vx_uint32 pipeline_depth)
    : pipeline_depth_(pipeline_depth)
{
}

void PipelineConfig::addGraphParam(vx_node       node,
                                   vx_uint32     node_param_index,
                                   vx_reference *refs_list,
                                   vx_uint32     refs_list_size,
                                   vx_int32     *graph_param_store,
                                   vx_bool       enabled)
{
    PipelineParamConfig graph_param;
    graph_param.node              = node;
    graph_param.node_param_index  = node_param_index;
    graph_param.refs_list         = refs_list;
    graph_param.refs_list_size    = refs_list_size;
    graph_param.graph_param_store = graph_param_store;
    graph_param.enabled           = enabled;

    graph_params_.push_back(graph_param);
}

void PipelineConfig::addNodeBuf(vx_node   node,
                                vx_uint32 param_index,
                                vx_uint32 num_bufs,
                                vx_bool   enabled)
{
   /* SAFETY CHECK: Validate node before adding */
    if (node == NULL)
    {
        printf("PipelineConfig: ERROR: node is NULL for param_index=%d (SKIPPING)\n", param_index);
        return;
    }

    /* SAFETY CHECK: Verify parameter exists */
    vx_parameter param = vxGetParameterByIndex(node, param_index);
    if (param == NULL)
    {
        printf("PipelineConfig: ERROR: vxGetParameterByIndex returned NULL for param_index=%d (SKIPPING)\n", param_index);
        return;
    }

    vx_status param_status = vxGetStatus((vx_reference)param);
    if (param_status != VX_SUCCESS)
    {
        printf("PipelineConfig: ERROR: parameter at index=%d has invalid status=%d (SKIPPING)\n",
               param_index, param_status);
        vxReleaseParameter(&param);
        return;
    }
    vxReleaseParameter(&param);


    NodeBufConfig node_buf;
    node_buf.node        = node;
    node_buf.param_index = param_index;
    node_buf.num_bufs    = num_bufs;
    node_buf.enabled     = enabled;

    node_bufs_.push_back(node_buf);
}

vx_status PipelineConfig::setup(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    /* Set up graph parameters and pipeline schedule */
    if (!graph_params_.empty() && status == VX_SUCCESS)
    {
        status = app_setup_pipeline_params(graph,
                                           graph_params_.data(),
                                           (vx_int32)graph_params_.size(),
                                           pipeline_depth_);
    }

    /* Set up per-node buffer depths */
    if (!node_bufs_.empty() && status == VX_SUCCESS)
    {
        status = app_set_node_param_bufs(node_bufs_.data(), (vx_int32)node_bufs_.size());
    }

    return status;
}