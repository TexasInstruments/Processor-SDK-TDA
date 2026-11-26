/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_bev_cfg_params.cpp
 *
 * \brief BEV-demo-specific config — map-based parse implementation.
 */

#include "app_bev_cfg_params.h"
#include "app_modules_types.h"   /* APP_DUMP_TOTAL_FRAMES */
#include <cstdio>    /* sscanf */
#include <cstring>   /* memset */

AppBevCfgParams::AppBevCfgParams()
    : tidl_config()
    , tidl_network()
    , tidl_xycordinate()
    , input_file_path()
    , output_file_path("./output")
    , in_width(1920)
    , in_height(698)
    , dl_width(704)
    , dl_height(256)
    , start_frame(0)
    , num_frames(1)
    , en_out_img_write(1)
    , dump_output(0)
    , display_option(0)
    , delay_in_msecs(66)
    , num_iterations(1)
    , is_interactive(0)
    , test_mode(0)
    , enable_gui(1)
    , enable_pipeline_flow(0)
    , batch_enable(5)          // 6-batch processing (0-5 = 6 batches)
    , lidar_points(510000)
    , lidar2cam_points(16)
    , xycord_size(160000)
    , viz_th(0.35f)
    , ip_rgb_or_yuv(1)
    , op_rgb_or_yuv(1)
{
    // Initialize arrays
    memset(num_classes, 0, sizeof(num_classes));
    num_classes[0] = 5;
    num_classes[1] = 2;
    num_classes[2] = 255;

    memset(valid_region, -1, sizeof(valid_region));
}

void AppBevCfgParams::parse(vx_int32 argc, vx_char *argv[])
{
    vx_int32 dump_flag = 0;
    CfgMap m = app_cfg_parse(argc, argv, nullptr, &dump_flag);

    /* ---- String fields ---- */
    if (m.count("tidl_bev_config"))        tidl_config        = m["tidl_bev_config"];
    if (m.count("tidl_bev_network"))       tidl_network       = m["tidl_bev_network"];
    if (m.count("tidl_BEV_XYCordinate"))   tidl_xycordinate   = m["tidl_BEV_XYCordinate"];
    if (m.count("input_file_path"))        input_file_path    = m["input_file_path"];
    if (m.count("output_file_path"))       output_file_path   = m["output_file_path"];

    /* ---- Size pairs ---- */
    if (m.count("in_size"))
        sscanf(m["in_size"].c_str(),  "%d %d", &in_width,  &in_height);
    if (m.count("dl_size"))
        sscanf(m["dl_size"].c_str(),  "%d %d", &dl_width,  &dl_height);

    /* ---- Scalar fields ---- */
    if (m.count("start_frame"))            start_frame            = std::stoi(m["start_frame"]);
    if (m.count("num_frames"))             num_frames             = std::stoi(m["num_frames"]);
    if (m.count("en_out_img_write"))       en_out_img_write       = std::stoi(m["en_out_img_write"]);
    if (m.count("display_option"))         display_option         = std::stoi(m["display_option"]);
    if (m.count("delay_in_msecs"))         delay_in_msecs         = (vx_uint32)std::stoul(m["delay_in_msecs"]);
    if (m.count("num_iterations"))         num_iterations         = (vx_uint32)std::stoul(m["num_iterations"]);
    if (m.count("is_interactive"))         is_interactive         = (vx_uint32)std::stoul(m["is_interactive"]);
    if (m.count("test_mode"))              test_mode              = (vx_uint32)std::stoul(m["test_mode"]);
    if (m.count("enable_gui"))             enable_gui             = (vx_uint32)std::stoul(m["enable_gui"]);
    if (m.count("enable_pipeline_flow"))   enable_pipeline_flow   = std::stoi(m["enable_pipeline_flow"]);

    /* ---- BEV-specific ---- */
    if (m.count("batch_enable"))           batch_enable           = (vx_uint32)std::stoul(m["batch_enable"]);
    if (m.count("lidar_points"))           lidar_points           = std::stoi(m["lidar_points"]);
    if (m.count("lidar2cam_points"))       lidar2cam_points       = std::stoi(m["lidar2cam_points"]);
    if (m.count("xycord_size"))            xycord_size            = std::stoi(m["xycord_size"]);
    if (m.count("viz_th"))                 viz_th                 = std::stof(m["viz_th"]);
    if (m.count("ip_rgb_or_yuv"))          ip_rgb_or_yuv          = std::stoi(m["ip_rgb_or_yuv"]);
    if (m.count("op_rgb_or_yuv"))          op_rgb_or_yuv          = std::stoi(m["op_rgb_or_yuv"]);

    /* ---- num_classes (3 values) ---- */
    if (m.count("num_classes")) {
        int val1 = 5, val2 = 2, val3 = 255;
        int n = sscanf(m["num_classes"].c_str(), "%d %d %d", &val1, &val2, &val3);
        num_classes[0] = val1;
        if (n >= 2) num_classes[1] = val2;
        if (n >= 3) num_classes[2] = val3;
    }

    /* ---- valid_region (12 values: 3 classes × 4 coords) ---- */
    if (m.count("valid_region")) {
        // Parse up to 12 integers for 3 classes with 4 coordinates each
        // Format: x1 y1 w1 h1 x2 y2 w2 h2 x3 y3 w3 h3
        int vals[12];
        int n = sscanf(m["valid_region"].c_str(), "%d %d %d %d %d %d %d %d %d %d %d %d",
                       &vals[0], &vals[1], &vals[2], &vals[3],
                       &vals[4], &vals[5], &vals[6], &vals[7],
                       &vals[8], &vals[9], &vals[10], &vals[11]);
        for (int i = 0; i < n && i < 12; i++) {
            valid_region[i/4][i%4] = vals[i];
        }
    }

    /* ---- --dump CLI flag: force a fixed, non-interactive, single-iteration run ---- */
    dump_output = dump_flag;
    if (dump_output)
    {
        is_interactive = 0;
        num_iterations = 1;
        num_frames     = APP_DUMP_TOTAL_FRAMES + 1;
    }

    #ifdef x86_64
    display_option = 0;
    is_interactive = 0;
    #endif
}
