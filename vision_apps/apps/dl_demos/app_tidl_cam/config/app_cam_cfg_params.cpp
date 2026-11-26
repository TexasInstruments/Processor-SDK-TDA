/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_cam_cfg_params.cpp
 *
 * \brief Camera classification demo config — map-based parse implementation.
 *
 *  Replaces the old descriptor-table + offsetof() approach with explicit
 *  assignments from a std::map<string,string>.  This allows path fields
 *  to be std::string instead of fixed-size char arrays.
 */

#include "app_cam_cfg_params.h"
#include <cstdio>    /* sscanf */

/* ========================================================================== */
/*                          Constructor (defaults)                            */
/* ========================================================================== */

AppCamCfgParams::AppCamCfgParams()
    : tidl_config()
    , tidl_network()
    , output_file_path("/opt/vision_apps/app_cam_out")
    , sensor_name("SENSOR_SONY_IMX390_UB953_D3")
    , sensor_index(0)
    , usecase_option(0)
    , num_cameras_enabled(1)
    , ch_mask(0x1)
    , enable_ldc(1)
    , enable_error_detection(0)
    , dl_width(224)           /* Default ImageNet input size */
    , dl_height(224)
    , out_width(1280)         /* Default output/display size */
    , out_height(720)
    , num_top_results(5)      /* Top-5 classification */
    , en_out_capture_write(0)
    , en_out_viss_write(0)
    , en_out_ldc_write(0)
    , en_out_scaler_write(0)
    , en_out_pre_proc_write(0)
    , num_frames_to_write(1)
    , num_frames_to_skip(0)
    , is_interactive(1)
    , test_mode(0)
    , num_frames_to_run(1000000000)
    , delay_in_msecs(0)
    , num_iterations(1)
    , enable_pipeline_flow(1)
    , display_option(0)
{
}

/* ========================================================================== */
/*                          parse() method                                    */
/* ========================================================================== */

void AppCamCfgParams::parse(vx_int32 argc, vx_char *argv[])
{
    CfgMap m = app_cfg_parse(argc, argv, nullptr);

    /* ---- String fields ---- */
    if (m.count("tidl_config"))
        tidl_config = m["tidl_config"];
    else if (m.count("tidl_config_file_path"))
        tidl_config = m["tidl_config_file_path"];

    if (m.count("tidl_network"))
        tidl_network = m["tidl_network"];
    else if (m.count("tidl_network_file_path"))
        tidl_network = m["tidl_network_file_path"];

    if (m.count("output_file_path")) output_file_path = m["output_file_path"];
    if (m.count("sensor_name"))      sensor_name      = m["sensor_name"];

    /* ---- Camera/Sensor Parameters ---- */
    if (m.count("sensor_index"))        sensor_index        = std::stoi(m["sensor_index"]);
    if (m.count("usecase_option"))      usecase_option      = std::stoi(m["usecase_option"]);
    if (m.count("num_cameras_enabled")) num_cameras_enabled = (vx_uint32)std::stoul(m["num_cameras_enabled"]);
    if (m.count("ch_mask"))             ch_mask             = (vx_uint32)std::stoul(m["ch_mask"]);
    if (m.count("enable_ldc"))          enable_ldc          = std::stoi(m["enable_ldc"]);
    if (m.count("enable_error_detection")) enable_error_detection = std::stoi(m["enable_error_detection"]);

    /* ---- Size pairs (e.g. "dl_size  224 224") ---- */
    if (m.count("dl_size"))
        sscanf(m["dl_size"].c_str(),  "%d %d", &dl_width,  &dl_height);
    if (m.count("out_size"))
        sscanf(m["out_size"].c_str(), "%d %d", &out_width, &out_height);

    /* ---- Classification-specific ---- */
    if (m.count("num_top_results"))
        num_top_results = std::stoi(m["num_top_results"]);

    /* ---- Output Control ---- */
    if (m.count("en_out_capture_write"))   en_out_capture_write   = std::stoi(m["en_out_capture_write"]);
    if (m.count("en_out_viss_write"))      en_out_viss_write      = std::stoi(m["en_out_viss_write"]);
    if (m.count("en_out_ldc_write"))       en_out_ldc_write       = std::stoi(m["en_out_ldc_write"]);
    if (m.count("en_out_scaler_write"))    en_out_scaler_write    = std::stoi(m["en_out_scaler_write"]);
    if (m.count("en_out_pre_proc_write"))  en_out_pre_proc_write  = std::stoi(m["en_out_pre_proc_write"]);
    if (m.count("num_frames_to_write"))    num_frames_to_write    = std::stoi(m["num_frames_to_write"]);
    if (m.count("num_frames_to_skip"))     num_frames_to_skip     = std::stoi(m["num_frames_to_skip"]);

    /* ---- Execution Control ---- */
    if (m.count("is_interactive"))   is_interactive   = (vx_uint32)std::stoul(m["is_interactive"]);
    if (m.count("test_mode"))        test_mode        = (vx_uint32)std::stoul(m["test_mode"]);
    if (m.count("num_frames_to_run")) num_frames_to_run = (vx_uint32)std::stoul(m["num_frames_to_run"]);
    if (m.count("delay_in_msecs"))   delay_in_msecs   = (vx_uint32)std::stoul(m["delay_in_msecs"]);
    if (m.count("delay"))            delay_in_msecs   = (vx_uint32)std::stoul(m["delay"]);
    if (m.count("num_iterations"))   num_iterations   = (vx_uint32)std::stoul(m["num_iterations"]);
    if (m.count("enable_pipeline_flow")) enable_pipeline_flow = std::stoi(m["enable_pipeline_flow"]);
    if (m.count("display_option"))   display_option   = std::stoi(m["display_option"]);
}
