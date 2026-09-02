/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#include <tivx_utils_file_rd_wr.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include "test_engine/test.h"
#include <app_init.h>

#ifdef HAVE_VERSION_INC
#include "openvx_cts_version.inc"
#else
#define VERSION_STR "unknown"
#endif

#undef CT_TESTCASE
#define CT_TESTCASE(testcase) struct CT_TestCaseEntry* testcase##_register();
#include "kernels/hwa/test/test_main.h"

#undef CT_TESTCASE
#define CT_TESTCASE(testcase) testcase##_register,
CT_RegisterTestCaseFN g_testcase_hwa_register_fns[] = {
    #include "kernels/hwa/test/test_main.h"
    NULL
};

void TestModuleRegister();
void TestModuleUnRegister();
int vision_apps_test_main(int argc, char* argv[]);
int CT_main(int argc, char* argv[], const char* version_str, CT_RegisterTestCaseFN register_fns[]);;

#if defined (LDRA_COVERAGE)
int32_t imaging_vpac_coverage_start(void);
#if !defined(SOC_AM62A)
int32_t imaging_dmpac_coverage_start(void);
#endif
int32_t imaging_vpac_coverage_end(void);
#if !defined(SOC_AM62A)
int32_t imaging_dmpac_coverage_end(void);
#endif
#endif

int main(int argc, char* argv[])
{
    int status;
    appInit();
    TestModuleRegister();
#if defined (LDRA_COVERAGE)
    imaging_vpac_coverage_start();
    #if !defined(SOC_AM62A)
    imaging_dmpac_coverage_start();
    #endif
#endif
    status = vision_apps_test_main(argc, argv);
#if defined (LDRA_COVERAGE)
    imaging_vpac_coverage_end();
    #if !defined(SOC_AM62A)
    imaging_dmpac_coverage_end();
    #endif
#endif
    TestModuleUnRegister();
    appDeInit();
    return status;
}

int vision_apps_test_main(int argc, char* argv[])
{
    return CT_main(argc, argv, VERSION_STR, g_testcase_hwa_register_fns);
}

