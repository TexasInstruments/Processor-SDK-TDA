/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_od_cfg_params.cpp
 *
 * \brief OD-demo-specific config — map-based parse implementation.
 *
 *  Replaces the old descriptor-table + offsetof() approach with explicit
 *  assignments from a std::map<string,string>.  This allows path fields
 *  to be std::string instead of fixed-size char arrays.
 */

#include "app_od_cfg_params.h"
#include "app_modules_types.h"   /* APP_DUMP_TOTAL_FRAMES */
#include <cstdio>    /* sscanf */

/* ========================================================================== */
/*                          Constructor (defaults)                            */
/* ========================================================================== */

AppOdCfgParams::AppOdCfgParams()
    : tidl_config()
    , tidl_network()
    , input_file_path()
    , output_file_path("./output")
    , in_width(1280)
    , in_height(720)
    , dl_width(512)
    , dl_height(512)
    , out_width(1920)
    , out_height(1080)
    , start_frame(0)
    , num_frames(1)
    , en_out_img_write(0)
    , dump_output(0)
    , display_option(0)
    , num_classes(0)
    , delay_in_msecs(0)
    , num_iterations(1)
    , is_interactive(0)
    , cpu_core_id(0)
    , viz_th(0.6f)
    , enable_pipeline_flow(0)
{
}

/* ========================================================================== */
/*                          parse() method                                    */
/* ========================================================================== */

void AppOdCfgParams::parse(vx_int32 argc, vx_char *argv[])
{
    vx_int32 dump_flag = 0;
    CfgMap m = app_cfg_parse(argc, argv, nullptr, &dump_flag);

    /* ---- String fields ---- */
    if (m.count("tidl_config"))      tidl_config      = m["tidl_config"];
    if (m.count("tidl_network"))     tidl_network     = m["tidl_network"];
    if (m.count("input_file_path"))  input_file_path  = m["input_file_path"];
    if (m.count("output_file_path")) output_file_path = m["output_file_path"];

    /* ---- Size pairs (e.g. "in_size  1024 512") ---- */
    if (m.count("in_size"))
        sscanf(m["in_size"].c_str(),  "%d %d", &in_width,  &in_height);
    if (m.count("dl_size"))
        sscanf(m["dl_size"].c_str(),  "%d %d", &dl_width,  &dl_height);
    if (m.count("out_size"))
        sscanf(m["out_size"].c_str(), "%d %d", &out_width, &out_height);

    /* ---- Scalar fields ---- */
    if (m.count("start_frame"))      start_frame      = std::stoi(m["start_frame"]);
    if (m.count("num_frames"))       num_frames       = std::stoi(m["num_frames"]);
    if (m.count("en_out_img_write")) en_out_img_write = std::stoi(m["en_out_img_write"]);
    if (m.count("display_option"))   display_option   = std::stoi(m["display_option"]);
    if (m.count("num_classes"))      num_classes      = std::stoi(m["num_classes"]);
    if (m.count("delay_in_msecs"))   delay_in_msecs   = (vx_uint32)std::stoul(m["delay_in_msecs"]);
    if (m.count("num_iterations"))   num_iterations   = (vx_uint32)std::stoul(m["num_iterations"]);
    if (m.count("is_interactive"))   is_interactive   = (vx_uint32)std::stoul(m["is_interactive"]);
    if (m.count("cpu_core_id"))      cpu_core_id      = (vx_uint32)std::stoul(m["cpu_core_id"]);
    if (m.count("viz_th"))           viz_th           = std::stof(m["viz_th"]);
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