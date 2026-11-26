/*
 * Copyright (c) 2024-2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_od_cam_cfg_params.h
 *
 * \brief Camera-based object detection demo config parameter class.
 */
#ifndef _APP_OD_CAM_CFG_PARAMS_H
#define _APP_OD_CAM_CFG_PARAMS_H

#include <TI/tivx.h>
#include "app_cfg_parser.h"   /* APP_MAX_FILE_PATH, CfgMap, app_cfg_parse */

/**
 * \brief All config-file and CLI parameters for the camera object detection demo.
 *
 *  Constructor sets safe defaults.  Call parse() to read CLI + .cfg file;
 *  keys not present in the .cfg file are left at their default values.
 */
class AppOdCamCfgParams {
public:
    /** \brief Constructor — sets safe defaults. */
    AppOdCamCfgParams();

    /**
     * \brief Parse CLI + .cfg file and populate this object.
     *
     * \param [in] argc  CLI argument count.
     * \param [in] argv  CLI argument array.
     */
    void parse(vx_int32 argc, vx_char *argv[]);

    /* ---- Paths ---- */
    std::string  tidl_config;
    std::string  tidl_network;
    std::string  output_file_path;
    std::string  sensor_name;

    /* ---- Camera/Sensor Parameters ---- */
    vx_int32   sensor_index;
    vx_int32   usecase_option;
    vx_uint32  num_cameras_enabled;
    vx_uint32  ch_mask;
    vx_int32   enable_ldc;
    vx_int32   enable_error_detection;

    /* ---- Image Size Pairs ---- */
    vx_int32  dl_width;          /**< DL network input width */
    vx_int32  dl_height;         /**< DL network input height */
    vx_int32  out_width;         /**< Display/overlay output width */
    vx_int32  out_height;        /**< Display/overlay output height */

    /* ---- Object-detection-specific ---- */
    vx_float32 viz_th;           /**< Detection visualization threshold */
    vx_int32   num_classes;      /**< Kept for cfg-file compatibility with legacy demo; not
                                       consumed by the generic PostProc detection kernel. */
    vx_uint32  cpu_core_id;      /**< TIDL C7x core select; inert on j721e (single TIDL core) */

    /* ---- Output Control ---- */
    vx_int32   en_out_capture_write;
    vx_int32   en_out_viss_write;
    vx_int32   en_out_ldc_write;
    vx_int32   en_out_scaler_write;
    vx_int32   en_out_pre_proc_write;
    vx_int32   num_frames_to_write;
    vx_int32   num_frames_to_skip;
    vx_int32   en_out_img_write;   /**< Write final mosaic output frames */

    /* ---- Execution Control ---- */
    vx_uint32  is_interactive;
    vx_uint32  num_frames_to_run;
    vx_uint32  delay_in_msecs;
    vx_uint32  num_iterations;
    vx_int32   display_option;
};

#endif /* _APP_OD_CAM_CFG_PARAMS_H */
