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
#ifndef _APP_MODULES_TYPES
#define _APP_MODULES_TYPES

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_host_utils.h"
#include <TI/tivx_img_proc.h>
#include <TI/j7_tidl.h>
#include <TI/tivx_fileio.h>
#include <tivx_utils_file_rd_wr.h>
#include <tivx_utils_graph_perf.h>
#include <utils/perf_stats/include/app_perf_stats.h>


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <string>
#include <vector>

/* ========================================================================== */
/*                              Debug Macros                                  */
/* ========================================================================== */

#ifndef APP_PRINTF
#define APP_PRINTF(f_, ...)
#endif

#define APP_ERROR(f_, ...) printf((f_), ##__VA_ARGS__)

/* ========================================================================== */
/*                              Type Definitions                              */
/* ========================================================================== */

/*! \brief Function pointer type for per-frame run callback.
 *
 *  The framework calls this on each frame during graph execution.
 *  The \a context parameter is an opaque pointer to the application
 *  object (e.g. AppObj*) that was stored in run_one_frame_context.
 *
 * \param [in] context   Opaque pointer to the application object.
 * \param [in] frame_id  Current frame number.
 * \return VX_SUCCESS on success.
 */
typedef vx_status (*RunOneFrameFn)(void *context, vx_int32 frame_id);


/* ========================================================================== */
/*                          Shared constants                                  */
/*                                                                            */
/* These use the same names as app_common.h so that either header (or both)   */
/* can be included without conflict.  Each is guarded with #ifndef so that    */
/* whichever header is included first wins; the values are identical.         */
/* ========================================================================== */

#ifndef APP_MAX_FILE_PATH
#define APP_MAX_FILE_PATH           (256u)
#endif

#ifndef APP_MAX_BUFQ_DEPTH
#define APP_MAX_BUFQ_DEPTH          (8u)
#endif

#ifndef APP_MAX_TENSOR_DIMS
#define APP_MAX_TENSOR_DIMS         (4u)
#endif

#ifndef APP_MAX_TENSORS
#define APP_MAX_TENSORS             (4u)
#endif

#ifndef APP_TIDL_MAX_PARAMS
#define APP_TIDL_MAX_PARAMS         (16u)
#endif

#ifndef MAX_IMG_WIDTH
#define MAX_IMG_WIDTH               (2048)
#endif

#ifndef MAX_IMG_HEIGHT
#define MAX_IMG_HEIGHT              (2048)
#endif

#ifndef NUM_CH
#define NUM_CH                      (1)
#endif

/* ========================================================================== */
/*                    Constants specific to modules framework               */
/* ========================================================================== */

#define APP_MODULES_MAX_OBJ_NAME       (512u)
#define APP_MODULES_MAX_OBJ_NAME_SIZE  (512u)  // Alias for compatibility

#define APP_MODULES_BUFFER_Q_DEPTH     (2)
#define APP_MODULES_MAX_BUFQ_DEPTH     (8u)    // Alias for compatibility
#define APP_MODULES_PIPELINE_DEPTH     (6)

/* ========================================================================== */
/*                  Module type enum                                          */
/* ========================================================================== */

/*!
 * \brief Enum for module types used in the app pipeline.
 */
enum class AppModuleType : vx_int32 {
    Scaler = 0,      /*!< Multi-scaler (MSC) module */
    PreProc,         /*!< DL pre-processing module */
    Tidl,            /*!< TIDL inference module */
    PostProc,        /*!< DL post-processing module */
    ImgMosaic,       /*!< Image mosaic module */
    Display,         /*!< Display module */
    BevPostProc,     /*!< BEV post-processing module */
    /* Camera pipeline modules */
    Sensor,          /*!< Image sensor module */
    Capture,         /*!< Raw frame capture module */
    Viss,            /*!< VISS (ISP) module */
    Ldc,             /*!< Lens distortion correction module */
    Aewb,            /*!< Auto exposure/white balance module */
    Max              /*!< Total number of module types */
};

/* ========================================================================== */
/*                       Forward declarations                                 */
/* ========================================================================== */

struct AppModuleObj;
class  AppGraphObj;

/* ========================================================================== */
/*                  Module base class (C++ virtual interface)                 */
/* ========================================================================== */

#ifdef __cplusplus
/*!
 * \brief Abstract base class for all pipeline modules.
 *
 * Each concrete module (ScalerObj, PreProcObj, TIDLObj, …) inherits from
 * this class and implements the four virtual methods.  The graph framework
 * dispatches through these virtuals — no function-pointer callback table
 * is needed.
 */
class AppModuleBase {
public:
    virtual ~AppModuleBase() = default;

    /*! \brief Return the module type enum for this module (no RTTI needed). */
    virtual AppModuleType getType() const = 0;

    /*! \brief Allocate data objects (equivalent to old app_init_xxx). */
    virtual vx_status initModule(AppGraphObj *graph, vx_int32 module_index)   = 0;

    /*! \brief Create the OpenVX node in the graph (equivalent to old app_create_graph_xxx). */
    virtual vx_status createNode(AppGraphObj *graph, vx_int32 module_index)   = 0;

    /*! \brief Release the OpenVX node (equivalent to old app_delete_xxx). */
    virtual vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index)   = 0;

    /*! \brief Release data objects (equivalent to old app_deinit_xxx). */
    virtual vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) = 0;
};

/* ========================================================================== */
/*                       Common Image Object                                  */
/* ========================================================================== */

/** \brief Image Data Structure
 *
 * Common image data structure used by sensor, scaler, and other modules.
 * Contains an object array and dimensions.
 */
class ImgObj {
public:
    /*! Object array for image */
    vx_object_array arr;

    /*! Image width */
    vx_int32 width;

    /*! Image height */
    vx_int32 height;

    /*! Constructor — zero-initialises all members. */
    ImgObj() : arr(nullptr), width(0), height(0) {}
};

#endif /* __cplusplus */

/* ========================================================================== */
/*                  Module object – one entry per module in the graph         */
/* ========================================================================== */

struct AppModuleObj {
    /*! Module type enum */
    AppModuleType        module_type;

    /*! Index of this module in the graph's module list */
    vx_int32            module_index;

    /*! Pointer to the concrete module object (virtual dispatch via AppModuleBase). */
    AppModuleBase       *module_obj;
};

/* ========================================================================== */
/*                  Offline output-dump constants (for --dump CLI option)     */
/* ========================================================================== */

/*! \brief Frame count a demo must run to reach the highest dump target index. */
#define APP_DUMP_TOTAL_FRAMES  (20)

/* ========================================================================== */
/*                  Graph object – wraps the entire application state         */
/* ========================================================================== */

class AppGraphObj {
public:
    /* ---- OpenVX references ---- */
    vx_context  context;
    vx_graph    graph;

    std::vector<AppModuleObj> module_list;

    /* ---- File paths ---- */
    std::string input_file_path;
    std::string output_file_path;

    /* ---- Graph export name (for .dot file) ---- */
    std::string dot_export_name;

    /* ---- Task control ---- */
    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;

    /* ---- Pipeline state ---- */
    int32_t pipeline;
    int32_t enqueueCnt;
    int32_t dequeueCnt;

    /* ---- Generic run-graph parameters ---- */

    /*! \brief Per-frame callback — set by the demo in apply_cfg().
     *
     *  Called by the generic run-graph loop in app_modules.cpp.
     *  The demo sets this to a static wrapper function that dispatches
     *  to either run_one_frame_sequential or run_one_frame_pipeline.
     *
     * \param [in] context   Opaque pointer passed as run_one_frame_context.
     * \param [in] frame_id  Current frame number.
     * \return VX_SUCCESS on success.
     */
    RunOneFrameFn  run_one_frame;

    /*! \brief Opaque pointer passed to run_one_frame (typically the AppObj*). */
    void          *run_one_frame_context;

    /*! Flag set by the demo: 0 = sequential, 1 = pipeline. */
    vx_int32 enable_pipeline_flow;

    /*! Runtime parameters — moved here so the generic run-graph works
     *  without knowing the app-specific AppObj layout. */
    vx_int32  start_frame;
    vx_int32  num_frames;
    vx_uint32 delay_in_msecs;
    vx_uint32 num_iterations;
    vx_uint32 is_interactive;
    vx_int32  buffer_q_depth;

    /*! Enable one-shot offline dump of mosaic output for a fixed set of
     *  frame indices (--dump CLI option). */
    vx_int32 dump_output;

    /*! Name used for perf-stats export file (the 'e' interactive command). */
    std::string perf_export_name;

    /* ---- Performance measurement points ---- */
    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;

    /*! Demo name shown in the interactive menu banner.
     *  Set by the application before calling run().
     *  If left empty, defaults to "Demo (generic interactive)". */
    std::string demo_name;

    /* ==== Class methods ==== */

    /*! \brief Constructor - initialize all pointers to NULL for safety. */
    AppGraphObj();

    /*! \brief Destructor - automatically cleanup resources (calls deleteGraph and deinit). */
    ~AppGraphObj();

    /*! Owns the vx_context/vx_graph and all registered modules' resources;
     *  copying would duplicate handles and cause double-release on destruction. */
    AppGraphObj(const AppGraphObj&)            = delete;
    AppGraphObj& operator=(const AppGraphObj&) = delete;
    AppGraphObj(AppGraphObj&&)                 = delete;
    AppGraphObj& operator=(AppGraphObj&&)      = delete;

    /*! \brief Initialize the graph object (create context, load kernels, create input). */
    vx_status init();

    /*! \brief Register a module and initialize it immediately (calls initModule). */
    AppModuleObj* addNode(AppModuleBase *module_obj);

    /*! \brief Create the OpenVX graph and all module nodes. */
    vx_status createGraph();

    /*! \brief Verify the graph (vxVerifyGraph + dot export + MSC coefficients). */
    vx_status verifyGraph();

    /*! \brief Delete all nodes and release the OpenVX graph. */
    vx_status deleteGraph();

    /*! \brief De-initialize all modules and release the OpenVX context. */
    vx_status deinit();

    /*! \brief Run the graph (interactive or non-interactive). */
    vx_status run(vx_bool is_interactive);

    /*! \brief Print performance statistics. */
    void printPerformance();

    /*! \brief Get a module object by its type. */
    AppModuleObj* getModuleByType(AppModuleType module_type);

    /*! \brief Reset the pipeline counters. */
    void pipelineParamsReset(vx_int32 buffer_q_depth);

    /*! \brief Add a graph parameter from a node parameter index. */
    vx_status addGraphParamByNodeIndex(vx_node node, vx_uint32 node_parameter_index);

    /*! \brief Find the index of an object array whose first element matches a reference. */
    static void findObjectArrayIndex(vx_object_array object_array[],
                                     vx_reference ref,
                                     vx_int32 array_size,
                                     vx_int32 *array_idx);

    /*! \brief Check whether frame_id is one of the fixed dump-target frames
     *         (relative to start_frame) and hasn't been dumped yet; marks it
     *         dumped if so. Used with dump_output — demos call this from
     *         run_one_frame_pipeline() around their own ImgMosaic
     *         dequeue/write/enqueue.
     *
     * \param [in] frame_id  Current frame number (as passed to run_one_frame).
     * \return vx_true_e if this frame should be dumped now, vx_false_e otherwise.
     */
    vx_bool shouldDumpFrame(vx_int32 frame_id);

private:
    /*! Fixed dump-target frame indices and their "already dumped" state. */
    struct DumpFrameInfo {
        vx_int32 frame_index;
        vx_bool  is_dumped;
    };
    std::vector<DumpFrameInfo> dump_frames_;
};

#endif /* _APP_MODULES_TYPES */
