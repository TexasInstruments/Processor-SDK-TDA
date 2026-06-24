/*
*
* Copyright (c) {2015 - 2024} Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/**
@file      itidl_rt_profile.h
@brief     This file defines an interface for profiling the TIDL Runtime API. \n
           This interface is targeted for profiling within the TIDL RT stack 

@version 0.1 April 2024 : Initial Code

*/
//#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "tidl_rt_profile.h"
#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   /* for nanosleep */
int nanosleep(const struct timespec *req,
              struct timespec *rem);
int clock_gettime (clockid_t __clock_id, struct timespec *__tp);
#endif

const sTIDLProfilePrintInfo_t gTimeProfilePrintInfo[TIDLRT_PROFILE_MAX_POINTS] =
{
  {"tidlrt_create"            ,  TIDLRT_PROFILE_TIDLRT_CREATE},
  {"tidl_rt_ovx_Init"         ,  TIDLRT_PROFILE_TIDL_RT_OVX_INIT},
  {"vxCreateContext"          ,  TIDLRT_PROFILE_VXCREATECONTEXT},
  {"init_tidl_tiovx"          ,  TIDLRT_PROFILE_INIT_TIDL_TIOVX},
  {"create_graph_tidl_tiovx"  ,  TIDLRT_PROFILE_CREATE_GRAPH_TIDL_TIOVX},
  {"verify_graph_tidl_tiovx"  ,  TIDLRT_PROFILE_VERIFY_GRAPH_TIDL_TIOVX},
  {"tivxTIDLLoadKernels"      ,  TIDLRT_PROFILE_TIVX_TIDL_LOAD_KERNEL},
  {"mapConfig"                ,  TIDLRT_PROFILE_MAPCONFIG},
  {"tivxAddKernelTIDL"        ,  TIDLRT_PROFILE_TIVXADDKERNELTIDL},
  {"mapNetwork"               ,  TIDLRT_PROFILE_MAPNETWORK},
  {"setCreateParams"          ,  TIDLRT_PROFILE_SETCREATEPARAMS},
  {"setArgs"                  ,  TIDLRT_PROFILE_SETARGS},
  {"vxCreateUserDataObject"   ,  TIDLRT_PROFILE_VX_CREATE_USER_DATA_OBJECT},
  {"vxMapUserDataObject"      ,  TIDLRT_PROFILE_VX_MAP_USER_DATA_OBJECT},
  {"memcopy_network_buffer"   ,  TIDLRT_PROFILE_MEMCOPY_NETWORK_BUFFER},
  {"vxUnmapUserDataObject"    ,  TIDLRT_PROFILE_VX_UNMAP_USER_DATA_OBJECT}
};

void init_rt_profile(sProfilePoints_t *profiler, int32_t traceLogLevel)
{
    (void)memset(profiler, 0, sizeof(sProfilePoints_t));
    profiler->enable = (traceLogLevel > 1) ? 1U : 0U;
    return;
}

static void get_time_u64(uint64_t *t)
{
    struct timespec ts;
    /*MISRA Rule 17.3: Reviewed clock_gettime is defined in time.h with _POSIX_C_SOURCE 200809L*/
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    *t = ((uint64_t)ts.tv_sec * (uint64_t)1000000000ULL) + (uint64_t)ts.tv_nsec;
}

void TIDLRT_profileStart(sProfilePoints_t*  profile_points, int32_t profileName)
{
    if (profile_points->enable != 0U) 
    {
        get_time_u64(&profile_points->points[profileName].startTime);
    }
}

void TIDLRT_profileEnd(sProfilePoints_t*  profile_points, int32_t profileName)
{
    if (profile_points->enable != 0U)
    {
        uint64_t endTime;
        get_time_u64(&endTime);
        profile_points->points[profileName].totalTime += endTime - profile_points->points[profileName].startTime;
    }
}

void printProfileInfo(sProfilePoints_t* profile_points)
{
    if (profile_points->enable != 0U) 
    {
        printf ("RT-Profile: TIDLRT_init_profiling \n");
        for (uint32_t i = 0; i < (uint32_t)TIDLRT_PROFILE_MAX_POINTS; i++)
        {
            printf("%-25s:", gTimeProfilePrintInfo[i].string);
            printf("%15" PRIu64 " ns,", profile_points->points[i].totalTime);
            printf("\n");
        }
    }
    return;
}

