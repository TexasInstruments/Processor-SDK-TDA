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
#ifndef _APP_PIPELINE_CONFIG_H
#define _APP_PIPELINE_CONFIG_H

#include "app_modules_types.h"
#include <vector>

/* ========================================================================== */
/*                   Low-Level Pipeline Configuration (C API)                 */
/* ========================================================================== */

/*! \brief Per-node buffer configuration entry for pipeline setup.
 *
 *  Used with app_set_node_param_bufs() to configure multiple
 *  node parameter buffer depths in a single table-driven call.
 *
 * \ingroup app_modules
 */
typedef struct {
    vx_node   node;         /*!< Node to configure */
    vx_uint32 param_index;  /*!< Parameter index on the node */
    vx_uint32 num_bufs;     /*!< Number of buffers to allocate */
    vx_bool   enabled;      /*!< vx_true_e to apply, vx_false_e to skip */
} NodeBufConfig;

/*! \brief Apply a table of per-node buffer configurations.
 *
 *  Iterates over the config array and calls tivxSetNodeParameterNumBufByIndex
 *  for each enabled entry.  Stops on the first failure.
 *
 * \param [in] configs  Array of NodeBufConfig entries.
 * \param [in] count    Number of entries in the array.
 * \return VX_SUCCESS on success, or the first error status encountered.
 *
 * \ingroup app_modules
 */
vx_status app_set_node_param_bufs(const NodeBufConfig configs[], vx_int32 count);

/*! \brief Pipeline parameter configuration entry.
 *
 *  Used with app_setup_pipeline_params() to register graph
 *  parameters, configure the schedule, and set pipeline depth — all from
 *  a single table.
 *
 * \ingroup app_modules
 */
typedef struct {
    vx_node       node;              /*!< Node whose parameter becomes a graph parameter */
    vx_uint32     node_param_index;  /*!< Parameter index on the node */
    vx_reference *refs_list;         /*!< Array of refs to enqueue */
    vx_uint32     refs_list_size;    /*!< Number of refs (typically APP_BUFFER_Q_DEPTH) */
    vx_int32     *graph_param_store; /*!< Pointer to store assigned graph_parameter_index
                                          (e.g. &scalerObj->graph_parameter_index) */
    vx_bool       enabled;           /*!< vx_true_e to include, vx_false_e to skip */
} PipelineParamConfig;

/*! \brief Set up pipeline graph parameters, schedule config, and pipeline depth.
 *
 *  For each enabled entry: adds the node parameter as a graph parameter,
 *  stores the assigned index via graph_param_store, and populates the
 *  queue params list.  Then calls vxSetGraphScheduleConfig and
 *  tivxSetGraphPipelineDepth.
 *
 * \param [in]  graph           OpenVX graph.
 * \param [in]  configs         Array of PipelineParamConfig entries.
 * \param [in]  count           Number of entries in the array.
 * \param [in]  pipeline_depth  Pipeline depth (APP_PIPELINE_DEPTH).
 * \return VX_SUCCESS on success, or the first error status encountered.
 *
 * \ingroup app_modules
 */
vx_status app_setup_pipeline_params(vx_graph graph,
                                    const PipelineParamConfig configs[],
                                    vx_int32 count,
                                    vx_uint32 pipeline_depth);

/* ========================================================================== */
/*                  High-Level Pipeline Configuration (C++ API)               */
/* ========================================================================== */

/*!
 * \brief Builder-pattern class for configuring OpenVX graph pipeline params.
 *
 * Wraps the existing PipelineParamConfig / NodeBufConfig structs and the
 * app_setup_pipeline_params() / app_set_node_param_bufs() free functions
 * into a single object with a clean API.
 *
 * Usage:
 * \code
 *   PipelineConfig cfg(APP_PIPELINE_DEPTH);
 *
 *   cfg.addGraphParam(scalerObj.node, 0,
 *                     (vx_reference*)&graphObj.input_images[0],
 *                     APP_BUFFER_Q_DEPTH,
 *                     &scalerObj.graph_parameter_index);
 *
 *   cfg.addNodeBuf(scalerObj.node, 1, 2);
 *
 *   status = cfg.setup(graphObj.graph);
 * \endcode
 *
 * \ingroup app_modules
 */
class PipelineConfig {
public:
    /*! \brief Constructor.
     *  \param [in] pipeline_depth  Pipeline depth (e.g. APP_PIPELINE_DEPTH).
     */
    PipelineConfig(vx_uint32 pipeline_depth);

    /*! \brief Add a graph parameter entry.
     *
     * \param [in]  node              Node whose parameter becomes a graph parameter.
     * \param [in]  node_param_index  Parameter index on the node.
     * \param [in]  refs_list         Array of vx_reference to enqueue.
     * \param [in]  refs_list_size    Number of refs (typically APP_BUFFER_Q_DEPTH).
     * \param [out] graph_param_store Pointer to store the assigned graph_parameter_index.
     * \param [in]  enabled           vx_true_e to include, vx_false_e to skip (default: vx_true_e).
     */
    void addGraphParam(vx_node       node,
                       vx_uint32     node_param_index,
                       vx_reference *refs_list,
                       vx_uint32     refs_list_size,
                       vx_int32     *graph_param_store,
                       vx_bool       enabled = (vx_bool)vx_true_e);

    /*! \brief Add a per-node buffer configuration entry.
     *
     * \param [in] node        Node to configure.
     * \param [in] param_index Parameter index on the node.
     * \param [in] num_bufs    Number of buffers to allocate.
     * \param [in] enabled     vx_true_e to apply, vx_false_e to skip (default: vx_true_e).
     */
    void addNodeBuf(vx_node   node,
                    vx_uint32 param_index,
                    vx_uint32 num_bufs,
                    vx_bool   enabled = (vx_bool)vx_true_e);

    /*! \brief Execute the pipeline setup.
     *
     * Calls app_setup_pipeline_params() for graph parameters, then
     * app_set_node_param_bufs() for node buffer configs.
     *
     * \param [in] graph  The OpenVX graph to configure.
     * \return VX_SUCCESS on success, or the first error encountered.
     */
    vx_status setup(vx_graph graph);

private:
    vx_uint32                        pipeline_depth_;
    std::vector<PipelineParamConfig> graph_params_;
    std::vector<NodeBufConfig>       node_bufs_;
};

#endif /* _APP_PIPELINE_CONFIG_H */