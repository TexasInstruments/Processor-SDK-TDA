/*
 *
 * Copyright (c) 2023-2024 Texas Instruments Incorporated
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

/*!
 * \file ethfw_trace.c
 *
 * \brief Ethernet Firmware trace functionality implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Helper macro to set the module id in the error code */
#define ETHFW_TRACE_ERRCODE_SET_MOD(x)        ((x) << ETHFW_TRACE_ERRCODE_MOD_OFFSET)

/* Helper macro to set the line number in the error code */
#define ETHFW_TRACE_ERRCODE_SET_LINE(x)       ((x) << ETHFW_TRACE_ERRCODE_LINE_OFFSET)

/* Helper macro to set the status value in the error code */
#define ETHFW_TRACE_ERRCODE_SET_STATUS(x)     ((x) << ETHFW_TRACE_ERRCODE_STATUS_OFFSET)

/* Helper macro to generate an error code from module id, line number
 * and status value */
#define ETHFW_TRACE_ERRCODE(mod, line, err)   (ETHFW_TRACE_ERRCODE_SET_MOD(mod) |   \
                                               ETHFW_TRACE_ERRCODE_SET_LINE(line) | \
                                               ETHFW_TRACE_ERRCODE_SET_STATUS(err))
/* Microseconds in a second */
#define ETHFW_TRACE_SEC_TO_USEC               (1000000ULL)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* EthFw Trace object */
typedef struct EthFwTrace_Obj_s
{
    /* Handle to mutex used to protect print buffer */
    EthFwOsal_MutexHandle hMutex;

    /* Print buffer */
    char printBuf[ETHFW_CFG_PRINT_BUF_LEN];

    /* Print function pointer */
    EthFwTrace_Print print;

    /* Trace timestamping function */
    EthFwTrace_TraceTsFunc tsFunc;

    /* Extended trace callback function */
    EthFwTrace_ExtTraceFunc extTraceFunc;
} EthFwTrace_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* EthFwTrace object */
static EthFwTrace_Obj gEthFwTraceObj;

/* Runtime log level */
EthFwTrace_TraceLevel gEthFwTrace_runtimeLevel;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwTrace_init(const EthFwTrace_Cfg *cfg)
{
    int32_t status = ETHFW_SOK;

    gEthFwTraceObj.hMutex = EthFwOsal_createMutex();
    if (gEthFwTraceObj.hMutex == NULL)
    {
        status = ETHFW_EFAIL;
        cfg->print("EthFwTrace_init() failed to create mutex\n");
    }

#if (ETHFW_CFG_TRACE_LEVEL > ETHFW_CFG_TRACE_LEVEL_NONE)
    gEthFwTrace_runtimeLevel = ETHFW_TRACE_INFO;
#else
    gEthFwTrace_runtimeLevel = ETHFW_TRACE_NONE;
#endif

    if (status == ETHFW_SOK)
    {
        gEthFwTraceObj.print = cfg->print;
        gEthFwTraceObj.tsFunc = cfg->traceTsFunc;
        gEthFwTraceObj.extTraceFunc = cfg->extTraceFunc;
    }

    return status;
}

void EthFwTrace_deinit(void)
{
    if (gEthFwTraceObj.hMutex != NULL)
    {
        EthFwOsal_deleteMutex(gEthFwTraceObj.hMutex);
        gEthFwTraceObj.hMutex = NULL;
    }
}


EthFwTrace_TraceLevel EthFwTrace_getLevel(void)
{
    return gEthFwTrace_runtimeLevel;
}

EthFwTrace_TraceLevel EthFwTrace_setLevel(EthFwTrace_TraceLevel level)
{
    EthFwTrace_TraceLevel prevLevel = gEthFwTrace_runtimeLevel;

    gEthFwTrace_runtimeLevel = level;

    return prevLevel;
}

void EthFwTrace_print(const char *fmt, ...)
{
    char *buf = &gEthFwTraceObj.printBuf[0U];
    va_list args;

    EthFwOsal_lockMutex(gEthFwTraceObj.hMutex);

    va_start(args, fmt);
    vsnprintf(buf, sizeof(gEthFwTraceObj.printBuf), fmt, args);
    gEthFwTraceObj.print(buf);
    va_end(args);

    EthFwOsal_unlockMutex(gEthFwTraceObj.hMutex);
}

#if (ETHFW_CFG_TRACE_LEVEL > ETHFW_CFG_TRACE_LEVEL_NONE)
void EthFwTrace_trace(EthFwTrace_TraceLevel globalLevel,
                      EthFwTrace_TraceLevel level,
                      uint32_t mod,
                      const char *file,
                      uint32_t line,
                      const char *func,
                      uint32_t status,
                      const char *fmt,
                      ...)
{
#if (ETHFW_CFG_TRACE_FORMAT >= ETHFW_CFG_TRACE_FORMAT_DFLT_TS)
    uint64_t tsUsecs = 0ULL;
#endif
    uint32_t errCode;
    int32_t p;
    int32_t n;
    int32_t w;
    va_list ap;
    char *buf = NULL;

    EthFwOsal_lockMutex(gEthFwTraceObj.hMutex);

    /* Check if specified level is enabled */
    if (globalLevel >= level)
    {
        /* Generate unique error code and call extended trace callback */
        if ((status != ETHFW_SOK) &&
            (gEthFwTraceObj.extTraceFunc != NULL))
        {
            errCode = ETHFW_TRACE_ERRCODE(mod, line, (uint8_t)(-1 * status));
            gEthFwTraceObj.extTraceFunc(errCode);
        }

        if (gEthFwTraceObj.print != NULL)
        {
            buf = &gEthFwTraceObj.printBuf[0U];
            p = 0;
            n = ETHFW_CFG_PRINT_BUF_LEN;

#if (ETHFW_CFG_TRACE_FORMAT >= ETHFW_CFG_TRACE_FORMAT_DFLT_TS)
            if (gEthFwTraceObj.tsFunc != NULL)
            {
                tsUsecs = gEthFwTraceObj.tsFunc();
            }
            w = snprintf(buf, n, "%6u.%06u: ",
                             (uint32_t)(tsUsecs / ETHFW_TRACE_SEC_TO_USEC),
                             (uint32_t)(tsUsecs % ETHFW_TRACE_SEC_TO_USEC));
            if ((w > 0) && (w < n))
            {
                p += w;
                n -= w;
            }
            else
            {
                n = 0;
            }
#endif

            va_start(ap, fmt);
            w = vsnprintf(buf + p, n, fmt, ap);
            va_end(ap);
            if ((w > 0) && (w < n))
            {
                p += w;
                n -= w;
            }
            else
            {
                n = 0;
            }

            if (status != ETHFW_SOK)
            {
                w = snprintf(buf + p, n, ": %d" ETHFW_CFG_TRACE_LINE_TERM, status);
            }
            else
            {
                w = snprintf(buf + p, n, ETHFW_CFG_TRACE_LINE_TERM);
            }

            gEthFwTraceObj.print(buf);
        }
    }

    EthFwOsal_unlockMutex(gEthFwTraceObj.hMutex);
}
#endif
