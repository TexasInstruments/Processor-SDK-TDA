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
#ifndef _APP_PRE_PROC_MODULE
#define _APP_PRE_PROC_MODULE

#define APP_PRE_PROC_MAX_TENSORS (8)

#include "app_modules.h"
#include "app_modules_types.h"
#include "tiadalg_interface.h"
#include "itidl_ti.h"

/* Macros that may not be provided by app_modules.h but are needed by
 * this module.  Guarded so that if the including app already defines
 * them (e.g. via app_common.h / app_modules_types.h), there is no
 * conflict. */

#ifndef APP_ASSERT
#define APP_ASSERT(x)               assert((x))
#endif

#ifndef APP_ASSERT_VALID_REF
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));
#endif

#ifndef APP_MAX_FILE_PATH
#define APP_MAX_FILE_PATH           (256u)
#endif

#ifndef APP_MAX_TENSOR_DIMS
#define APP_MAX_TENSOR_DIMS         (4u)
#endif

#ifndef APP_MAX_TENSORS
#define APP_MAX_TENSORS             (4u)
#endif

#ifndef NUM_CH
#define NUM_CH                      (1)
#endif

class PreProcObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::PreProc; }
    /*! Input object arrays - OD uses [0] only, BEV uses [0-5] */
    vx_object_array input_arr[6];

    vx_node  node;

    vx_user_data_object config;
    tivxDLPreProcArmv8Params params;

    vx_uint32 num_input_tensors;
    vx_uint32 num_output_tensors;

    vx_object_array  output_tensor_arr[APP_PRE_PROC_MAX_TENSORS];
    vx_int32 graph_parameter_index;

    /*! Number of channels (cameras). Defaults to NUM_CH; set before initModule(). */
    vx_int32 num_ch;

    /*! Number of input images for pre-processing (1 for OD, 6 for BEV). Defaults to 1. */
    vx_int32 num_images;

    sTIDL_IOBufDesc_t tidl_io_buf_desc;

    /*! \brief Constructor — sets safe defaults. */
    PreProcObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    PreProcObj(const PreProcObj&)            = delete;
    PreProcObj& operator=(const PreProcObj&) = delete;
    PreProcObj(PreProcObj&&)                 = delete;
    PreProcObj& operator=(PreProcObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the pre-proc node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the pre-proc node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Write pre-proc output to file */
    vx_status writeOutput(char* file_name);

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Update params and create tensors from TIDL config. */
    vx_status updateFromTIDLConfig(vx_context context, sTIDL_IOBufDesc_t *ioBufDesc);

    /*! \brief Create output tensor objects based on TIDL I/O descriptor. */
    void createOutputTensors(vx_context context, sTIDL_IOBufDesc_t *ioBufDesc, vx_tensor output_tensors[]);

    /*! \brief Map TIDL data type to OpenVX tensor data type. */
    vx_size getTensorDataType(vx_int32 tidl_type);
};

#endif
