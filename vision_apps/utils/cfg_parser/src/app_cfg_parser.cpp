/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 *
 * DISCLAIMER. THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS".
 */

/**
 * \file  app_cfg_parser.cpp
 *
 * \brief Generic config infrastructure — all key-value internals are hidden.
 *
 *  Public function:
 *    - app_cfg_parse()  — parse CLI and .cfg file, return std::map
 */

#include "app_cfg_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

/* ========================================================================== */
/*                       Internal helpers                                     */
/* ========================================================================== */

static std::string trim(const std::string &s)
{
    size_t begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
        return "";

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

/** Read a .cfg file directly into a map. */
static void cfg_read_file(CfgMap &map, const char *filename)
{
    std::ifstream file(filename);
    std::string   line;

    if (!file.is_open())
    {
        printf("app_cfg_parser: ERROR — cannot open config file '%s'\n",
               filename);
        return;
    }

    while (std::getline(file, line))
    {
        std::string trimmed = trim(line);

        if (trimmed.empty() || trimmed[0] == '#')
            continue;

        size_t key_end = trimmed.find_first_of(" \t");
        std::string key   = trimmed.substr(0, key_end);
        std::string value = (key_end == std::string::npos)
                             ? ""
                             : trim(trimmed.substr(key_end + 1));

        // Directly insert into map
        map[key] = value;
    }
}

/* ========================================================================== */
/*                          Internal helper - usage message                   */
/* ========================================================================== */

static void app_cfg_show_usage(vx_int32 argc, vx_char *argv[])
{
    (void)argc;
    printf("\n");
    printf(" Usage: %s --cfg <config.cfg> [--test] [--dump]\n", argv[0]);
    printf("\n");
    printf("   --cfg <file>   Path to a .cfg file with key-value pairs.\n");
    printf("   --test         Enable test mode (sets test_mode=1).\n");
    printf("   --dump         Enable one-shot offline output dumping (sets dump_output=1).\n");
    printf("   --help         Show this message.\n");
    printf("\n");
}

/* ========================================================================== */
/*                     Public API - map-based                                 */
/* ========================================================================== */

CfgMap app_cfg_parse(vx_int32 argc, vx_char *argv[], vx_int32 *test_mode_out,
                     vx_int32 *dump_output_out)
{
    CfgMap    result;
    vx_int32  i;
    vx_int32  test_mode = 0;
    vx_int32  dump_output = 0;
    char      cfg_file[APP_MAX_FILE_PATH] = "";

    /* ---- Parse CLI flags ---- */
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--cfg") == 0 && (i + 1) < argc)
        {
            snprintf(cfg_file, APP_MAX_FILE_PATH, "%s", argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--test") == 0)
        {
            test_mode = 1;
        }
        else if (strcmp(argv[i], "--dump") == 0)
        {
            dump_output = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            app_cfg_show_usage(argc, argv);
        }
    }

    /* ---- Read .cfg file directly into map ---- */
    if (cfg_file[0] != '\0')
    {
        cfg_read_file(result, cfg_file);
    }

    /* ---- Return test_mode / dump_output flags ---- */
    if (test_mode_out != nullptr)
    {
        *test_mode_out = test_mode;
    }
    if (dump_output_out != nullptr)
    {
        *dump_output_out = dump_output;
    }

    return result;
}
