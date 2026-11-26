/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

/**
 * \file  app_seg_cfg_params.h
 *
 * \brief Semantic segmentation demo-specific config parameter class and parse method.
 */
#ifndef _APP_SEG_CFG_PARAMS_H
#define _APP_SEG_CFG_PARAMS_H

#include <TI/tivx.h>
#include "app_cfg_parser.h"   /* APP_MAX_FILE_PATH, CfgMap, app_cfg_parse */

/**
 * \brief All config-file and CLI parameters for the semantic segmentation demo.
 *
 *  Constructor sets safe defaults.  Call parse() to read CLI + .cfg file;
 *  keys not present in the .cfg file are left at their default values.
 */
class AppSegCfgParams {
public:
    /** \brief Constructor — sets safe defaults. */
    AppSegCfgParams();

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
    std::string  input_file_path;
    std::string  output_file_path;

    /* ---- Size pairs ---- */
    vx_int32  in_width;
    vx_int32  in_height;
    vx_int32  dl_width;
    vx_int32  dl_height;
    vx_int32  out_width;
    vx_int32  out_height;

    /* ---- Segmentation-specific ---- */
    vx_float32 viz_th;            /**< Visualization threshold for filtering low-confidence pixels */
    vx_int32   num_classes;       /**< Number of semantic classes (e.g., 21 for PASCAL VOC) */

    /* ---- Scalars ---- */
    vx_int32   start_frame;
    vx_int32   num_frames;
    vx_int32   en_out_img_write;
    vx_int32   dump_output;
    vx_int32   display_option;
    vx_uint32  delay_in_msecs;
    vx_uint32  num_iterations;
    vx_uint32  is_interactive;
    vx_int32   enable_pipeline_flow;
};

#endif /* _APP_SEG_CFG_PARAMS_H */
