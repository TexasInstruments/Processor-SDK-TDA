/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

#ifndef _APP_COMMON
#define _APP_COMMON

#include "app_modules.h"

// #define APP_DEBUG
#define APP_USE_FILEIO

#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

#define ABS_FLT(a) ((a) > 0)?(a):(-(a))

#undef APP_PRINTF
#ifdef APP_DEBUG
#define APP_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define APP_PRINTF(f_, ...)
#endif

#endif
