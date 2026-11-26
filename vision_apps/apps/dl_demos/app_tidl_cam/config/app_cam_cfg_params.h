/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_cam_cfg_params.h
 *
 * \brief Camera classification demo config parameter class and parse method.
 */
#ifndef _APP_CAM_CFG_PARAMS_H
#define _APP_CAM_CFG_PARAMS_H

#include <TI/tivx.h>
#include "app_cfg_parser.h"   /* APP_MAX_FILE_PATH, CfgMap, app_cfg_parse */

/**
 * \brief All config-file and CLI parameters for the camera classification demo.
 *
 *  Constructor sets safe defaults.  Call parse() to read CLI + .cfg file;
 *  keys not present in the .cfg file are left at their default values.
 */
class AppCamCfgParams {
public:
    /** \brief Constructor — sets safe defaults. */
    AppCamCfgParams();

    /**
     * \brief Parse CLI + .cfg file and populate this object.
     *
     *  Uses the map-based app_cfg_parse() API.  String fields are assigned
     *  directly from the map; scalar fields are converted with stoi/stoul/stof.
     *
     * \param [in] argc  CLI argument count.
     * \param [in] argv  CLI argument array.
     */
    void parse(vx_int32 argc, vx_char *argv[]);

    /* ---- Paths (now std::string) ---- */
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
    vx_int32  dl_width;
    vx_int32  dl_height;
    vx_int32  out_width;
    vx_int32  out_height;

    /* ---- Classification-specific ---- */
    vx_int32   num_top_results;   /**< Top-N classification (default 5) */

    /* ---- Output Control ---- */
    vx_int32   en_out_capture_write;
    vx_int32   en_out_viss_write;
    vx_int32   en_out_ldc_write;
    vx_int32   en_out_scaler_write;
    vx_int32   en_out_pre_proc_write;
    vx_int32   num_frames_to_write;
    vx_int32   num_frames_to_skip;

    /* ---- Execution Control ---- */
    vx_uint32  is_interactive;
    vx_uint32  test_mode;
    vx_uint32  num_frames_to_run;
    vx_uint32  delay_in_msecs;
    vx_uint32  num_iterations;
    vx_int32   enable_pipeline_flow;
    vx_int32   display_option;
};

#endif /* _APP_CAM_CFG_PARAMS_H */
