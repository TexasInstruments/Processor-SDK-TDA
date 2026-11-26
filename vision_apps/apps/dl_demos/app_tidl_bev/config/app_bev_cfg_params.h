/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_bev_cfg_params.h
 *
 * \brief BEV-demo-specific config parameter class with 6-batch and LiDAR support
 */
#ifndef _APP_BEV_CFG_PARAMS_H
#define _APP_BEV_CFG_PARAMS_H

#include <TI/tivx.h>
#include "app_cfg_parser.h"

/**
 * \brief All config-file and CLI parameters for the BEV demo.
 *
 * Supports 6-batch processing and LiDAR integration
 */
class AppBevCfgParams {
public:
    /** \brief Constructor — sets safe defaults. */
    AppBevCfgParams();

    /**
     * \brief Parse CLI + .cfg file and populate this object.
     */
    void parse(vx_int32 argc, vx_char *argv[]);

    /* ---- Paths ---- */
    std::string  tidl_config;
    std::string  tidl_network;
    std::string  tidl_xycordinate;        // BEV XYCordinate file for LiDAR
    std::string  input_file_path;
    std::string  output_file_path;

    /* ---- Size pairs ---- */
    vx_int32  in_width;
    vx_int32  in_height;
    vx_int32  dl_width;
    vx_int32  dl_height;

    /* ---- Run parameters ---- */
    vx_int32   start_frame;
    vx_int32   num_frames;
    vx_int32   en_out_img_write;
    vx_int32   dump_output;
    vx_int32   display_option;
    vx_uint32  delay_in_msecs;
    vx_uint32  num_iterations;
    vx_uint32  is_interactive;
    vx_uint32  test_mode;
    vx_uint32  enable_gui;
    vx_int32   enable_pipeline_flow;

    /* ---- BEV-specific (6-batch processing) ---- */
    vx_uint32  batch_enable;         // Number of batches (0-5, where 5 means 6 batches)

    /* ---- LiDAR parameters ---- */
    vx_int32   lidar_points;
    vx_int32   lidar2cam_points;
    vx_int32   xycord_size;

    /* ---- Visualization ---- */
    vx_float32 viz_th;
    vx_int32   num_classes[3];       // For each output tensor
    vx_int32   valid_region[3][4];

    vx_int32   ip_rgb_or_yuv;
    vx_int32   op_rgb_or_yuv;
};

#endif /* _APP_BEV_CFG_PARAMS_H */
