/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_seg_cfg_params.cpp
 *
 * \brief Segmentation-demo-specific config — map-based parse implementation.
 *
 *  Replaces the old descriptor-table + offsetof() approach with explicit
 *  assignments from a std::map<string,string>.  This allows path fields
 *  to be std::string instead of fixed-size char arrays.
 */

#include "app_seg_cfg_params.h"
#include "app_modules_types.h"   /* APP_DUMP_TOTAL_FRAMES */
#include <cstdio>    /* sscanf */

/* ========================================================================== */
/*                          Constructor (defaults)                            */
/* ========================================================================== */

AppSegCfgParams::AppSegCfgParams()
    : tidl_config()
    , tidl_network()
    , input_file_path()
    , output_file_path("./output")
    , in_width(1024)          /* Default YUV input size (from scaler) */
    , in_height(512)
    , dl_width(512)           /* Default segmentation input size */
    , dl_height(512)
    , out_width(1024)         /* Default output/display size */
    , out_height(512)
    , viz_th(0.6f)            /* Default visualization threshold */
    , num_classes(21)         /* Default PASCAL VOC classes */
    , start_frame(0)
    , num_frames(1)
    , en_out_img_write(0)
    , dump_output(0)
    , display_option(0)
    , delay_in_msecs(0)
    , num_iterations(1)
    , is_interactive(0)
    , enable_pipeline_flow(0)
{
}

/* ========================================================================== */
/*                          parse() method                                    */
/* ========================================================================== */

void AppSegCfgParams::parse(vx_int32 argc, vx_char *argv[])
{
    vx_int32 dump_flag = 0;
    CfgMap m = app_cfg_parse(argc, argv, nullptr, &dump_flag);

    /* ---- String fields (support both old and new key names) ---- */
    if (m.count("tidl_config"))
        tidl_config = m["tidl_config"];
    else if (m.count("tidl_config_file_path"))
        tidl_config = m["tidl_config_file_path"];

    if (m.count("tidl_network"))
        tidl_network = m["tidl_network"];
    else if (m.count("tidl_network_file_path"))
        tidl_network = m["tidl_network_file_path"];

    if (m.count("input_file_path"))  input_file_path  = m["input_file_path"];
    if (m.count("output_file_path")) output_file_path = m["output_file_path"];

    /* ---- Size pairs (e.g. "in_size  480 480") ---- */
    if (m.count("in_size"))
        sscanf(m["in_size"].c_str(),  "%d %d", &in_width,  &in_height);
    if (m.count("dl_size"))
        sscanf(m["dl_size"].c_str(),  "%d %d", &dl_width,  &dl_height);
    if (m.count("out_size"))
        sscanf(m["out_size"].c_str(), "%d %d", &out_width, &out_height);

    /* ---- Segmentation-specific ---- */
    if (m.count("viz_th"))
        viz_th = std::stof(m["viz_th"]);
    if (m.count("num_classes"))
        num_classes = std::stoi(m["num_classes"]);

    /* ---- Scalar fields ---- */
    if (m.count("start_frame"))      start_frame      = std::stoi(m["start_frame"]);
    if (m.count("num_frames"))       num_frames       = std::stoi(m["num_frames"]);
    if (m.count("en_out_img_write")) en_out_img_write = std::stoi(m["en_out_img_write"]);
    if (m.count("display_option"))   display_option   = std::stoi(m["display_option"]);
    if (m.count("delay_in_msecs"))   delay_in_msecs   = (vx_uint32)std::stoul(m["delay_in_msecs"]);

    /* Support both "delay_in_msecs" and "delay" for backward compatibility */
    if (m.count("delay"))
        delay_in_msecs = (vx_uint32)std::stoul(m["delay"]);

    if (m.count("num_iterations"))   num_iterations   = (vx_uint32)std::stoul(m["num_iterations"]);
    if (m.count("is_interactive"))   is_interactive   = (vx_uint32)std::stoul(m["is_interactive"]);
    if (m.count("enable_pipeline_flow"))
        enable_pipeline_flow = std::stoi(m["enable_pipeline_flow"]);

    /* ---- --dump CLI flag: force a fixed, non-interactive, single-iteration run ---- */
    dump_output = dump_flag;
    if (dump_output)
    {
        is_interactive = 0;
        num_iterations = 1;
        num_frames     = APP_DUMP_TOTAL_FRAMES + 1;
    }
}
