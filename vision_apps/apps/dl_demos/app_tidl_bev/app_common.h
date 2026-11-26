/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */
#ifndef _APP_COMMON_H
#define _APP_COMMON_H

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_host_utils.h"
#include <TI/tivx_img_proc.h>
#include <TI/tivx_img_proc_kernels.h>
#include <TI/tivx_img_proc_nodes.h>
#include <TI/j7_tidl.h>
#include <tivx_utils_file_rd_wr.h>
#include <tivx_utils_graph_perf.h>
#include <app_modules.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <math.h>

//#define APP_DEBUG
#define APP_USE_FILEIO
//#define APP_TIVX_LOG_RT_ENABLE

#define APP_MAX_FILE_PATH           (256u)
#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

#define ABS_FLT(a) ((a) > 0)?(a):(-(a))

#define MAX_IMG_WIDTH  (2048)
#define MAX_IMG_HEIGHT (2048)
#define NUM_CH    (1)
#define NUM_ALGOS (5)
#define CAM_2_IMG_MATRIX_SIZE (9)

/* BEV needs 6 tensors (vs 4 in generic modules) */
#undef APP_MAX_TENSORS
#define APP_MAX_TENSORS (6u)

/* BEV needs larger buffer depth (16 vs 8 in generic modules) */
#undef APP_MAX_BUFQ_DEPTH
#define APP_MAX_BUFQ_DEPTH (16)

/* BEV-specific constants */
#define APP_MAX_NUM_CLASSES (3u)

#ifdef APP_DEBUG
#define APP_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define APP_PRINTF(f_, ...)
#endif

#endif
