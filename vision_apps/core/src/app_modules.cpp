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
#include "app_scaler_module.hpp"

#include <VX/vx_khr_pipelining.h>
#include <TI/hwa_vpac_msc.h>
#include <TI/hwa_kernels.h>
#include <TI/video_io_kernels.h>
#include <TI/dl_kernels.h>
#include <TI/video_io_display.h>
#ifndef x86_64
#include <TI/j7_kernels_imaging_aewb.h>
#endif
#include <TI/tivx_fileio_kernels.h>

#ifdef APP_DEBUG
#define APP_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define APP_PRINTF(f_, ...)
#endif

#include <utils/perf_stats/include/app_perf_stats.h>

#ifndef x86_64
#include <utils/grpx/include/app_grpx.h>
#endif

/* ========================================================================== */
/*                  AppGraphObj class method implementations                  */
/* ========================================================================== */

AppGraphObj::AppGraphObj()
{
    /* Initialize all OpenVX handles to NULL for safety */
    context = NULL;
    graph = NULL;
    buffer_q_depth = 0;
    enable_pipeline_flow = 0;
    dump_output = 0;

    /* Fixed dump-target frame indices (matches upstream's --dump patch) */
    dump_frames_ = {
        { 0,  (vx_bool)vx_false_e },
        { 4,  (vx_bool)vx_false_e },
        { 9,  (vx_bool)vx_false_e },
        { 19, (vx_bool)vx_false_e }
    };

    APP_PRINTF("AppGraphObj constructor called\n");
}

AppGraphObj::~AppGraphObj()
{
    APP_PRINTF("AppGraphObj destructor called - automatic cleanup\n");

    /* Clean up in proper order: graph first, then resources */
    deleteGraph();
    deinit();

    APP_PRINTF("AppGraphObj destructor completed\n");
}

vx_status AppGraphObj::init()
{
    vx_status status = VX_SUCCESS;

    /* Zero out module list */
    module_list.clear();

    /* Create OpenVX Context */
    context = vxCreateContext();
    status = vxGetStatus((vx_reference)context);

    if (status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(context);
        tivxVideoIOLoadKernels(context);
#ifndef x86_64
        tivxImagingLoadKernels(context);
#endif
        tivxImgProcLoadKernels(context);
        tivxTIDLLoadKernels(context);
        tivxFileIOLoadKernels(context);
    }

    appPerfPointSetName(&total_perf, "TOTAL");
    appPerfPointSetName(&fileio_perf, "FILEIO");

    /* Pipeline defaults */
    pipelineParamsReset(APP_MODULES_BUFFER_Q_DEPTH);

    return status;
}

AppModuleObj* AppGraphObj::addNode(AppModuleBase *module_obj)
{
    AppModuleType module_type = module_obj->getType();

    if (module_type >= AppModuleType::Max)
    {
        printf("app_modules: ERROR: Invalid module type %d\n", (vx_int32)module_type);
        return NULL;
    }

    vx_int32 module_index = (vx_int32)module_list.size();

    /* Initialize the module immediately upon registration */
    vx_status status = module_obj->initModule(this, module_index);
    if (status != VX_SUCCESS)
    {
        printf("app_modules: ERROR: initModule failed for module type %d (index %d)\n",
               (vx_int32)module_type, module_index);
        return NULL;
    }

    module_list.push_back({module_type, module_index, module_obj});
    return &module_list.back();
}

vx_status AppGraphObj::createGraph()
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    /* Create the OpenVX graph */
    graph = vxCreateGraph(context);
    status = vxGetStatus((vx_reference)graph);

    if (status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)graph, "OpenVxGraph");
    }

    for (i = 0; i < (vx_int32)module_list.size() && status == VX_SUCCESS; i++)
    {
        AppModuleObj *mod = &module_list[i];
        if (mod->module_obj != NULL)
        {
            status = mod->module_obj->createNode(this, mod->module_index);
        }
    }

    return status;
}

vx_status AppGraphObj::verifyGraph()
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(graph, ".", dot_export_name.c_str());
    }

    /* Send MSC coefficients to all registered scaler modules */
    if (status == VX_SUCCESS)
    {
        vx_int32 i;
        for (i = 0; i < (vx_int32)module_list.size(); i++)
        {
            if (module_list[i].module_type == AppModuleType::Scaler)
            {
                ScalerObj *scalerObj = static_cast<ScalerObj *>(module_list[i].module_obj);
                vx_status s = scalerObj->sendMscCoeff(i);
                if (s != VX_SUCCESS)
                {
                    if (status == VX_SUCCESS) status = s;
                }
            }
        }
    }

    /* Wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    return status;
}

vx_status AppGraphObj::deleteGraph()
{
    if (context == NULL)
    {
        return VX_SUCCESS;
    }

    vx_status status = VX_SUCCESS;
    vx_int32 i;

    /* Call deleteNode for each registered module (virtual dispatch) */
    for (i = 0; i < (vx_int32)module_list.size(); i++)
    {
        AppModuleObj *mod = &module_list[i];
        if (mod->module_obj != NULL)
        {
            vx_status s = mod->module_obj->deleteNode(this, mod->module_index);
            if (s != VX_SUCCESS && status == VX_SUCCESS)
            {
                status = s;
            }
        }
    }

    if (graph != NULL)
    {
        vxReleaseGraph(&graph);
    }

    return status;
}

vx_status AppGraphObj::deinit()
{
    if (context == NULL)
    {
        return VX_SUCCESS;
    }

    vx_status status = VX_SUCCESS;
    vx_int32 i;

    for (i = 0; i < (vx_int32)module_list.size(); i++)
    {
        AppModuleObj *mod = &module_list[i];
        if (mod->module_obj != NULL)
        {
            vx_status s = mod->module_obj->deinitModule(this, mod->module_index);
            if (s != VX_SUCCESS && status == VX_SUCCESS)
            {
                status = s;
            }
        }
    }

    /* Unload kernels */
    tivxFileIOUnLoadKernels(context);
    tivxTIDLUnLoadKernels(context);
    tivxImgProcUnLoadKernels(context);
#ifndef x86_64
    tivxImagingUnLoadKernels(context);
#endif
    tivxVideoIOUnLoadKernels(context);
    tivxHwaUnLoadKernels(context);

    /* Release context */
    vxReleaseContext(&context);

    return status;
}

AppModuleObj* AppGraphObj::getModuleByType(AppModuleType module_type)
{
    vx_int32 i;

    for (i = 0; i < (vx_int32)module_list.size(); i++)
    {
        if (module_list[i].module_type == module_type)
        {
            return &module_list[i];
        }
    }

    return NULL;
}

void AppGraphObj::printPerformance()
{
    appPerfStatsPrintAll();
    tivx_utils_graph_perf_print(graph);
    appPerfPointPrint(&fileio_perf);
    appPerfPointPrint(&total_perf);
    printf("\n");
    appPerfPointPrintFPS(&total_perf);
    appPerfPointReset(&total_perf);
    printf("\n");
}

void AppGraphObj::pipelineParamsReset(vx_int32 buf_q_depth)
{
    pipeline   = -buf_q_depth;
    enqueueCnt = 0;
    dequeueCnt = 0;
}

/* ========================================================================== */
/*            Free utility functions (not class methods)                      */
/* ========================================================================== */

vx_status app_set_node_param_bufs(const NodeBufConfig configs[], vx_int32 count)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    for (i = 0; i < count && status == VX_SUCCESS; i++)
    {
        if (configs[i].enabled == (vx_bool)vx_true_e)
        {
            status = tivxSetNodeParameterNumBufByIndex(configs[i].node,
                                                        configs[i].param_index,
                                                        configs[i].num_bufs);
            if (status != VX_SUCCESS)
            {
                printf("[PIPELINE] FAILED setNodeBuf[%d]: node=%p param=%u bufs=%u\n",
                       i, (void*)configs[i].node, configs[i].param_index, configs[i].num_bufs);
                fflush(stdout);
            }
        }
    }

    return status;
}

/** Local helper — adds a node parameter as a graph parameter (free-function version). */
static vx_status add_graph_param_by_node_index(vx_graph graph,
                                               vx_node node,
                                               vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);
    vx_status status;

    status = vxAddParameterToGraph(graph, parameter);
    if (status == VX_SUCCESS)
    {
        status = vxReleaseParameter(&parameter);
    }

    return status;
}

vx_status app_setup_pipeline_params(vx_graph graph,
                                    const PipelineParamConfig configs[],
                                    vx_int32 count,
                                    vx_uint32 pipeline_depth)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    vx_int32 graph_parameter_index = 0;
    /* Zero-initialized by the vector constructor - no garbage values */
    std::vector<vx_graph_parameter_queue_params_t> queue_params_list(count);

    /* Register enabled entries as graph parameters */
    for (i = 0; i < count && status == VX_SUCCESS; i++)
    {
        if (configs[i].enabled != (vx_bool)vx_true_e)
            continue;

        /* SAFETY CHECK: Validate node before using it */
        if (configs[i].node == NULL)
        {
            printf("ERROR: app_setup_pipeline_params: node is NULL at config index %d (SKIPPING)\n", i);
            continue;
        }

        /* SAFETY CHECK: Validate refs_list */
        if (configs[i].refs_list == NULL)
        {
            printf("ERROR: app_setup_pipeline_params: refs_list is NULL at config index %d (SKIPPING)\n", i);
            continue;
        }

        status = add_graph_param_by_node_index(
                    graph, configs[i].node, configs[i].node_param_index);

        if (status == VX_SUCCESS)
        {
            /* Store assigned index back to the module struct */
            if (configs[i].graph_param_store != NULL)
            {
                *configs[i].graph_param_store = graph_parameter_index;
            }

            queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
            queue_params_list[graph_parameter_index].refs_list_size        = configs[i].refs_list_size;
            queue_params_list[graph_parameter_index].refs_list             = configs[i].refs_list;

            graph_parameter_index++;
        }
        else
        {
            printf("ERROR: app_setup_pipeline_params: add_graph_param_by_node_index failed with status=%d at config index %d\n", status, i);
        }
    }

    /* Configure the graph schedule */
    if (status == VX_SUCCESS)
    {
        /* SAFETY CHECK: Ensure we have at least one valid graph parameter */
        if (graph_parameter_index == 0)
        {
            printf("ERROR: app_setup_pipeline_params: No valid graph parameters configured! Pipeline mode requires at least one graph parameter.\n");
            status = VX_FAILURE;
        }
        else
        {
            /* SAFETY CHECK: Validate all refs_list entries before calling vxSetGraphScheduleConfig */
            vx_bool all_refs_valid = vx_true_e;
            for (i = 0; i < graph_parameter_index; i++)
            {
                if (queue_params_list[i].refs_list == NULL)
                {
                    printf("ERROR: app_setup_pipeline_params: refs_list is NULL at graph_parameter_index=%d\n", i);
                    all_refs_valid = vx_false_e;
                }
                else
                {
                    /* Check first reference in the list */
                    vx_reference ref = queue_params_list[i].refs_list[0];
                    if (ref == NULL)
                    {
                        printf("ERROR: app_setup_pipeline_params: refs_list[0] is NULL at graph_parameter_index=%d\n", i);
                        all_refs_valid = vx_false_e;
                    }
                    else
                    {
                        vx_status ref_status = vxGetStatus(ref);
                        if (ref_status != VX_SUCCESS)
                        {
                            printf("ERROR: app_setup_pipeline_params: refs_list[0] has invalid status=%d at graph_parameter_index=%d\n",
                                   ref_status, i);
                            all_refs_valid = vx_false_e;
                        }
                    }
                }
            }

            if (all_refs_valid == vx_false_e)
            {
                printf("ERROR: app_setup_pipeline_params: One or more refs_list entries are invalid! Cannot configure pipeline.\n");
                status = VX_FAILURE;
            }
            else
            {
                status = vxSetGraphScheduleConfig(graph,
                            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                            graph_parameter_index,
                            queue_params_list.data());

                if (status != VX_SUCCESS)
                {
                    printf("ERROR: vxSetGraphScheduleConfig failed with status=%d\n", status);
                }
            }
        }
    }

    /* Set pipeline depth */
    if (status == VX_SUCCESS)
    {
        status = tivxSetGraphPipelineDepth(graph, pipeline_depth);
    }

    return status;
}

vx_status AppGraphObj::addGraphParamByNodeIndex(vx_node node,
                                                vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);
    vx_status status;

    status = vxAddParameterToGraph(graph, parameter);
    if (status == VX_SUCCESS)
    {
        status = vxReleaseParameter(&parameter);
    }

    return status;
}

/* ========================================================================== */
/*            Generic run-graph / run-graph-interactive                       */
/* ========================================================================== */

static vx_status app_run_graph_loop(AppGraphObj *graphObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 frame_id;
    vx_int32 x;
    uint64_t cur_time;

    /* Reset pipeline counters for each run */
    graphObj->pipelineParamsReset(graphObj->buffer_q_depth);

    for (x = 0; x < (vx_int32)graphObj->num_iterations; x++)
    {
        for (frame_id = graphObj->start_frame;
             frame_id < (graphObj->start_frame + graphObj->num_frames);
             frame_id++)
        {
            APP_PRINTF("Running frame %d\n", frame_id);

            appPerfPointBegin(&graphObj->total_perf);

            cur_time = tivxPlatformGetTimeInUsecs();

            status = graphObj->run_one_frame(graphObj->run_one_frame_context, frame_id);

            cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
            /* convert to msecs */
            cur_time = cur_time / 1000;

            if (cur_time < graphObj->delay_in_msecs)
            {
                tivxTaskWaitMsecs(graphObj->delay_in_msecs - (uint32_t)cur_time);
            }

            appPerfPointEnd(&graphObj->total_perf);

            APP_PRINTF("app_run_graph: Frame ID %d of %d ... Done.\n",
                       frame_id, graphObj->start_frame + graphObj->num_frames);

            /* user asked to stop processing */
            if (graphObj->stop_task || status != VX_SUCCESS)
                break;
        }

        printf("app_run_graph: Iteration %d of %d ... Done.\n",
               x, graphObj->num_iterations);

        appPerfPointPrintFPS(&graphObj->total_perf);
        appPerfPointReset(&graphObj->total_perf);
        if (x == 0)
        {
            /* reset stats after first iteration so steady-state is captured */
            appPerfStatsResetAll();
        }

        if (graphObj->stop_task || status != VX_SUCCESS)
            break;
    }

    if (graphObj->enable_pipeline_flow)
    {
        vxWaitGraph(graphObj->graph);
    }


    graphObj->stop_task = 1;

    return status;
}

/* ---- Background task entry point for interactive mode ---- */
static void app_modules_run_task_fn(void *app_var)
{
    AppGraphObj *graphObj = (AppGraphObj *)app_var;
    vx_status status = VX_SUCCESS;

    while (!graphObj->stop_task && status == VX_SUCCESS)
    {
        status = app_run_graph_loop(graphObj);
    }
    graphObj->stop_task_done = 1;
}

static vx_status app_run_graph_interactive(AppGraphObj *graphObj)
{
    vx_status status = VX_SUCCESS;
    uint32_t done = 0;
    char ch;
    tivx_task_create_params_t params;
    FILE *fp;
    app_perf_point_t *perf_arr[1];

    /* Create background task */
    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_modules_run_task_fn;
    params.app_var   = graphObj;

    graphObj->stop_task      = 0;
    graphObj->stop_task_done = 0;

    status = tivxTaskCreate(&graphObj->task, &params);
    if (status != VX_SUCCESS)
    {
        printf("app_modules: ERROR: Unable to create task\n");
        return status;
    }

    appPerfStatsResetAll();

    /* Use the configurable demo name, or fall back to default */
    const char *banner = (!graphObj->demo_name.empty())
                             ? graphObj->demo_name.c_str()
                             : "Demo (generic interactive)";

    while (!done && status == VX_SUCCESS)
    {
        printf("\n"
               "\n ========================="
               "\n %s"
               "\n ========================="
               "\n"
               "\n p: Print performance statistics"
               "\n e: Export performance statistics"
               "\n"
               "\n x: Exit"
               "\n"
               "\n Enter Choice: ", banner);
        ch = getchar();
        printf("\n");

        switch (ch)
        {
            case 'p':
                graphObj->printPerformance();
                break;
            case 'e':
                perf_arr[0] = &graphObj->total_perf;
                fp = appPerfStatsExportOpenFile(".", graphObj->perf_export_name.c_str());
                if (fp != NULL)
                {
                    appPerfStatsExportAll(fp, perf_arr, 1);
                    tivx_utils_graph_perf_export(fp, graphObj->graph);
                    appPerfStatsExportCloseFile(fp);
                    appPerfStatsResetAll();
                }
                else
                {
                    printf("fp is null\n");
                }
                break;
            case 'x':
                graphObj->stop_task = 1;
                done = 1;
                break;
            default:
                break;
        }
    }

    /* Wait for task completion */
    while (graphObj->stop_task_done == 0)
    {
        tivxTaskWaitMsecs(100);
    }

    /* Delete task — preserve earlier status */
    if (status == VX_SUCCESS)
    {
        status = tivxTaskDelete(&graphObj->task);
    }
    else
    {
        tivxTaskDelete(&graphObj->task);
    }

    return status;
}

/* ========================================================================== */
/*            Public unified run method                                       */
/* ========================================================================== */

vx_status AppGraphObj::run(vx_bool is_interactive)
{
    if (is_interactive == (vx_bool)vx_true_e)
    {
        return app_run_graph_interactive(this);
    }
    else
    {
        return app_run_graph_loop(this);
    }
}

/* ========================================================================== */
/*            Pipeline utility: find object array index                       */
/* ========================================================================== */

void AppGraphObj::findObjectArrayIndex(vx_object_array object_array[],
                                       vx_reference ref,
                                       vx_int32 array_size,
                                       vx_int32 *array_idx)
{
    vx_int32 i;

    *array_idx = -1;
    for (i = 0; i < array_size; i++)
    {
        vx_image img_ref = (vx_image)vxGetObjectArrayItem((vx_object_array)object_array[i], 0);
        if (ref == (vx_reference)img_ref)
        {
            *array_idx = i;
            vxReleaseImage(&img_ref);
            break;
        }
        vxReleaseImage(&img_ref);
    }
}

/* ========================================================================== */
/*            Offline output dump: fixed-frame tracking (--dump)              */
/* ========================================================================== */

vx_bool AppGraphObj::shouldDumpFrame(vx_int32 frame_id)
{
    vx_int32 expected_idx = frame_id - this->start_frame - 1;

    for (vx_int32 i = 0; i < (vx_int32)dump_frames_.size(); i++)
    {
        if ((dump_frames_[i].frame_index == expected_idx) &&
            (dump_frames_[i].is_dumped == (vx_bool)vx_false_e))
        {
            dump_frames_[i].is_dumped = (vx_bool)vx_true_e;
            return (vx_bool)vx_true_e;
        }
    }

    return (vx_bool)vx_false_e;
}
