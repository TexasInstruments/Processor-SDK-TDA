/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#include "app_grpx_priv.h"

int32_t appGrpxDrawCpuLoad(app_grpx_obj_t *obj,
                    uint32_t x, uint32_t y,
                    uint32_t barHeight
                    )
{
    Draw2D_FontPrm fontPrm;
    Draw2D_FontPrm fontPrm2;
    uint32_t procId;
    char string1[APP_GRPX_STRING_MAX];
    char string2[APP_GRPX_STRING_MAX];
    Draw2D_FontProperty fontProp;
    Draw2D_FontProperty fontProp2;
    uint32_t startX, startY1, startY2;
    uint32_t barWidth, charWidth, padX, padY;
    uint32_t cpu_load;

    fontPrm.fontIdx = 2;
    fontPrm2.fontIdx = 3;

    Draw2D_getFontProperty(&fontPrm, &fontProp);
    Draw2D_getFontProperty(&fontPrm2, &fontProp2);

    barWidth  = fontProp.width*3;
    charWidth = fontProp.width*APP_GRPX_LOAD_BAR_NUM_CHAR;
    padX = fontProp.width/2;
    padY = APP_GRPX_LOAD_PAD_Y;

    /* CPU name start X, Y */
    startY1 =  y + fontProp.height + padY + barHeight + padY;
    startY2 =  startY1 + fontProp.height + padY/2;
    startX =  x;

    for (procId = 0; procId < APP_IPC_CPU_MAX; procId++)
    {
        switch(procId)
        {
            case APP_IPC_CPU_MPU1_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "MPU"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "   "
                          );
                break;
            case APP_IPC_CPU_C7x_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x1"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "+MMA"
                          );
                break;
            #if defined(SOC_J721S2)
            case APP_IPC_CPU_C7x_2:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x2"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "   "
                          );
                break;
            #endif
            #if defined(SOC_J784S4) || defined(SOC_TDA54)
            case APP_IPC_CPU_C7x_2:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x2"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "+MMA"
                          );
                break;
            case APP_IPC_CPU_C7x_3:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x3"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "+MMA"
                          );
                break;
            case APP_IPC_CPU_C7x_4:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x4"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "+MMA"
                          );
                break;
            #endif
            #if defined(SOC_J742S2)
            case APP_IPC_CPU_C7x_2:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x2"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "+MMA"
                          );
                break;
            case APP_IPC_CPU_C7x_3:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C7x3"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "    "
                          );
                break;
            #endif
            #if defined(SOC_J721E)
            case APP_IPC_CPU_C6x_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C6x"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          " 1 "
                          );
                break;
            case APP_IPC_CPU_C6x_2:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "C6x"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          " 2 "
                          );
                break;
            #endif
            #if defined(SOC_TDA54)
            case APP_IPC_CPU_MCU0_M55:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "M55"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%1s",
                          "0"
                          );
                break;
            case APP_IPC_CPU_MCU1_M55:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "M55"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%1s",
                          "1"
                          );
                break;
            case APP_IPC_CPU_MCU2_M55:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "M55"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%1s",
                          "2"
                          );
                break;
            case APP_IPC_CPU_MCU3_M55:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "M55"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%1s",
                          "3"
                          );
                break;
            case APP_IPC_CPU_MCU4_M55:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "M55"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%1s",
                          "4"
                          );
                break;
            case APP_IPC_CPU_RMCU0_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R52"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "0-0"
                          );
                break;
            case APP_IPC_CPU_RMCU0_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R52"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "0-1"
                          );
                break;
            case APP_IPC_CPU_RMCU1_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R52"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "1-0"
                          );
                break;
            case APP_IPC_CPU_RMCU1_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R52"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "1-1"
                          );
                break;
            case APP_IPC_CPU_RMCU2_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R52"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "2-0"
                          );
                break;
            case APP_IPC_CPU_RMCU2_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R52"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "2-1"
                          );
                break;
            #else
            #if !defined(SOC_AM62A)
            case APP_IPC_CPU_MCU2_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "2-0"
                          );
                break;
            #if !defined(SOC_J722S)
            case APP_IPC_CPU_MCU2_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "2-1"
                          );
                break;
            case APP_IPC_CPU_MCU3_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s\n",
                          "3-0"
                          );
                break;
            case APP_IPC_CPU_MCU3_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "3-1"
                          );
                break;
            #endif
            #endif
            #if defined(SOC_J784S4) || defined(SOC_J742S2)
            case APP_IPC_CPU_MCU4_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s\n",
                          "4-0"
                          );
                break;
            case APP_IPC_CPU_MCU4_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "4-1"
                          );
                break;
            #endif
            case APP_IPC_CPU_MCU1_0:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "1-0"
                          );
                break;
            #if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
            case APP_IPC_CPU_MCU1_1:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "R5F"
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s",
                          "1-1"
                          );
                break;
            #endif
            #endif
            default:
                snprintf(string1, APP_GRPX_STRING_MAX,
                          "%3s",
                          "   "
                          );
                snprintf(string2, APP_GRPX_STRING_MAX,
                          "%3s\n",
                          "   "
                          );
                break;
        }

        /* draw CPU name - line 1 */
        Draw2D_drawString(obj->draw2d_obj,
                  startX,
                  startY1,
                  string1,
                  &fontPrm
                  );

        /* draw CPU name - line 2 */
        Draw2D_drawString(obj->draw2d_obj,
                  startX,
                  startY2,
                  string2,
                  &fontPrm2
                  );

        cpu_load = obj->cpu_load[procId];

        appGrpxDrawLoadBar
            (
                obj,
                cpu_load,
                startX,
                y + fontProp.height + padY,
                barWidth,
                barHeight
            );

        /* limit to 99 to stay within 3 char's for CPU load i.e nn% */
        if(cpu_load>99)
            cpu_load=99;

        /* draw CPU load as text */
        snprintf(string1, APP_GRPX_STRING_MAX,
                      "%2d%%\n",
                      cpu_load
                      );
        Draw2D_drawString(obj->draw2d_obj,
                  startX,
                  y,
                  string1,
                  &fontPrm
                  );

       startX += charWidth + padX;
    }

    return 0;
}

void appGrpxGetCpuLoad(app_grpx_obj_t *obj)
{
    uint32_t cpu_id;
    int32_t status=0;
    app_perf_stats_cpu_load_t cpu_load;

    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        obj->cpu_load[cpu_id] = 0;

        if(appIpcIsCpuEnabled(cpu_id))
        {
            status = appPerfStatsCpuLoadGet(cpu_id, &cpu_load);
            if(status==0)
            {
                /* round off and convert to integer */
                obj->cpu_load[cpu_id] = (cpu_load.cpu_load+50)/100;
                if(obj->cpu_load[cpu_id] > 100)
                    obj->cpu_load[cpu_id] = 100;
            }
        }
    }
}

int32_t appGrpxGetDimCpuLoad(uint16_t *width, uint16_t *height)
{
    int32_t status = 0;

    Draw2D_FontPrm font;
    Draw2D_FontProperty prop;

    Draw2D_FontPrm font2;
    Draw2D_FontProperty prop2;

    font.fontIdx = 2;
    font2.fontIdx = 3;

    *width = 0;
    *height = 0;

    status  = Draw2D_getFontProperty(&font, &prop);
    status |= Draw2D_getFontProperty(&font2, &prop2);
    if(status==0)
    {
        *height = APP_GRPX_LOAD_BAR_HEIGHT + prop.height*2 + prop2.height + APP_GRPX_LOAD_PAD_Y*2 + APP_GRPX_LOAD_PAD_Y/2;
        *width  = (prop.width*APP_GRPX_LOAD_BAR_NUM_CHAR + prop.width/2)*APP_IPC_CPU_MAX;
    }
    return status;
}


int32_t appGrpxShowCpuLoad(uint16_t startx, uint16_t starty)
{
    int32_t status = 0;
    app_grpx_obj_t *obj = (app_grpx_obj_t*)&g_app_grpx_obj;

    appGrpxGetCpuLoad(obj);
    appGrpxDrawCpuLoad(obj, startx, starty,
        APP_GRPX_LOAD_BAR_HEIGHT /* bar height */
        );
    return status;
}
