/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#include <utils/console_io/include/app_log.h>
#include <file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define APP_LOG_CIO_BUF_MAX_SIZE    ((uint32_t)1024U)

static char g_app_log_cio_buf[APP_LOG_CIO_BUF_MAX_SIZE];
static uint32_t g_app_log_cio_buf_idx = 0;
static FILE *g_app_log_cio_fid = NULL;

static int32_t appLogCioOpen(const char *path,
                             uint32_t flags,
                             int32_t llv_fd);

static int32_t appLogCioClose(int32_t dev_fd);

static int32_t appLogCioRead(int32_t dev_fd,
                             char *buf,
                             uint32_t count);

static int32_t appLogCioWrite(int32_t dev_fd,
                              const char *buf,
                              uint32_t count);

static off_t   appLogCioLseek(int32_t dev_fd,
                              off_t offset,
                              int32_t origin);

static int32_t appLogCioUnlink(const char *path);

static int32_t appLogCioRename(const char *old_name,
                               const char *new_name);

static int32_t appLogCioOpen(const char *path,
                             uint32_t flags,
                             int32_t llv_fd)
{
    g_app_log_cio_buf_idx = 0;
    return 0;
}

static int32_t appLogCioClose(int32_t dev_fd)
{
    if (g_app_log_cio_buf_idx > 0)
    {
        g_app_log_cio_buf[g_app_log_cio_buf_idx] = (char)0;
        g_app_log_cio_buf_idx++;
        appLogPrintf(g_app_log_cio_buf);
        g_app_log_cio_buf_idx = 0;
    }

    return 0;
}

static int32_t appLogCioRead(int32_t dev_fd,
                             char *buf,
                             uint32_t count)
{
    return 0;
}

static int32_t appLogCioWrite(int32_t dev_fd,
                              const char *buf,
                              uint32_t count)
{
    uint32_t flush_buf = 0;
    uint32_t i;

    for (i = 0; i < count; i++)
    {
        /* MISRA.PTR.ARITH
         * MISRAC_2004_Rule_11.1
         * MISRAC_WAIVER:
         * Pointer is accessed as an array.
         * For loop makes sure that the array is never accessed out of bound
         */
        g_app_log_cio_buf[g_app_log_cio_buf_idx] = buf[i];
        g_app_log_cio_buf_idx++;

        /* MISRA.PTR.ARITH
         * MISRAC_2004_Rule_11.1
         * MISRAC_WAIVER:
         * Pointer is accessed as an array.
         * For loop makes sure that the array is never accessed out of bound
         */
        if (buf[i] == (char)'\n')
        {
            g_app_log_cio_buf[g_app_log_cio_buf_idx] = (char)0;
            g_app_log_cio_buf_idx++;
            flush_buf = 1;
        }

        /* MISRA.PTR.ARITH
         * MISRAC_2004_Rule_11.1
         * MISRAC_WAIVER:
         * Pointer is accessed as an array.
         * For loop makes sure that the array is never accessed out of bound
         */
        if (buf[i] == (char)0)
        {
            flush_buf = 1;
        }

        if (g_app_log_cio_buf_idx == (APP_LOG_CIO_BUF_MAX_SIZE - 1U))
        {
            g_app_log_cio_buf[g_app_log_cio_buf_idx] = (char)0;
            g_app_log_cio_buf_idx++;
            flush_buf = 1;
        }

        if (flush_buf)
        {
            appLogPrintf(g_app_log_cio_buf);
            g_app_log_cio_buf_idx = 0;
            flush_buf = 0;
        }
    }

    return (int32_t)count;
}

static off_t appLogCioLseek(int32_t dev_fd,
                            off_t offset,
                            int32_t origin)
{
    return -(int32_t)1;
}

static int32_t appLogCioUnlink(const char *path)
{
    return 0;
}

static int32_t appLogCioRename(const char *old_name,
                               const char *new_name)
{
    return 0;
}

int32_t appLogCioInit(void)
{
    int32_t status = 0;

    status = add_device("appLog",
                        _SSA,
                        appLogCioOpen,
                        appLogCioClose,
                        appLogCioRead,
                        appLogCioWrite,
                        appLogCioLseek,
                        appLogCioUnlink,
                        appLogCioRename);

    if (status >= 0)
    {
        /* MISRA.STDLIB.STDIO
         * MISRAC_2004 Rule_20.9
         * This function can use printf,fopen functions for debug purpose..
         */
        g_app_log_cio_fid = fopen("appLog", "w");

        if (g_app_log_cio_fid == NULL)
        {
            status = -1;
        }
        else
        {
            /* MISRA.EXPANSION.UNSAFE
             * MISRA C 20.1, 20.5, 20.6, 20.7
             * MISRAC_WAIVER:
             * This is a requirement of this API and cannot be avoided
             */

            /* MISRA.STDLIB.STDIO
             * MISRAC_2004 Rule_20.9
             * This function can use printf,fopen functions for debug purpose..
             */
            freopen("appLog:", "w", stdout); /* redirect stdout to VpsLog */

            /* MISRA.EXPANSION.UNSAFE
             * MISRA C 20.1, 20.5, 20.6, 20.7
             * MISRAC_WAIVER:
             * This is a requirement of this API and cannot be avoided
             */

            /* MISRA.STDLIB.STDIO
             * MISRAC_2004 Rule_20.9
             * This function can use printf,fopen functions for debug purpose..
             */
            setvbuf(stdout, NULL, _IONBF, 0); /* turn off buffering for stdout */

            /* MISRA.STDLIB.STDIO
             * MISRAC_2004 Rule_20.9
             * This function can use printf,fopen functions for debug purpose..
             */
            printf("CIO: Init ... Done !!!\r\n");

            status = 0;
        }
    }

    return status;
}

void appLogCioDeInit(void)
{
    if (g_app_log_cio_fid != NULL)
    {
        /* MISRA.STDLIB.STDIO
         * MISRAC_2004 Rule_20.9
         * This function can use printf,fopen functions for debug purpose..
         */
        fclose(g_app_log_cio_fid);
    }
}
