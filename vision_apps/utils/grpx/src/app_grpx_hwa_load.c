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

uint32_t appGrpxGetNumHwa()
{
    return APP_PERF_HWA_MAX;
}

int32_t appGrpxDrawHwaLoad(app_grpx_obj_t *obj,
                    uint32_t x, uint32_t y,
                    uint32_t barHeight
                    )
{
    Draw2D_FontPrm fontPrm;
    Draw2D_FontPrm fontPrm2;
    app_perf_hwa_id_t hwa_id;
    char string1[APP_GRPX_STRING_MAX];
    char string2[APP_GRPX_STRING_MAX];
    Draw2D_FontProperty fontProp;
    Draw2D_FontProperty fontProp2;
    uint32_t startX, startY1, startY2;
    uint32_t barWidth, padX, padY;
    uint32_t hwa_load;

    fontPrm.fontIdx = 2;
    fontPrm2.fontIdx = 3;

    Draw2D_getFontProperty(&fontPrm, &fontProp);
    Draw2D_getFontProperty(&fontPrm2, &fontProp2);

    barWidth = fontProp.width*3;
    padX = fontProp.width/2;
    padY = APP_GRPX_LOAD_PAD_Y;

    /* CPU name start X, Y */
    startY1 =  y + fontProp.height + padY + barHeight + padY;
    startY2 =  startY1 + fontProp.height + padY/2;
    startX =  x;

    for (hwa_id = (app_perf_hwa_id_t)0; hwa_id < APP_PERF_HWA_MAX; hwa_id++)
    {
        if(obj->hwa_perf[hwa_id]>999)
            obj->hwa_perf[hwa_id]=999;

        snprintf(string1, APP_GRPX_STRING_MAX,
                  "%5s",
                  appPerfStatsGetHwaName(hwa_id)
                  );
        snprintf(string2, APP_GRPX_STRING_MAX,
                  "%3d MP/s",
                  obj->hwa_perf[hwa_id]
                  );

        /* draw HWA name - line 1 */
        Draw2D_drawString(obj->draw2d_obj,
                  startX,
                  startY1,
                  string1,
                  &fontPrm
                  );

        Draw2D_setFontColor(
            RGB888_TO_RGB565(0, 255, 255),
            RGB888_TO_RGB565(16, 16, 16),
            DRAW2D_TRANSPARENT_COLOR
            );

        /* draw HWA performance - line 2 */
        Draw2D_drawString(obj->draw2d_obj,
                  startX,
                  startY2,
                  string2,
                  &fontPrm2
                  );

        Draw2D_resetFontColor();

        hwa_load = obj->hwa_load[hwa_id];

        /* limit to 100 */
        if(hwa_load>100)
            hwa_load=100;

        appGrpxDrawLoadBar
            (
                obj,
                hwa_load,
                startX + fontProp.width,
                y + fontProp.height + padY,
                barWidth,
                barHeight
            );

        /* draw CPU load as text */
        snprintf(string1, APP_GRPX_STRING_MAX,
                      "%3d%%",
                      hwa_load
                      );
        Draw2D_drawString(obj->draw2d_obj,
                  startX,
                  y,
                  string1,
                  &fontPrm
                  );

       startX += fontProp2.width*APP_GRPX_LOAD_BAR_WIDTH_FACTOR + padX;
    }

    return 0;
}

void appGrpxGetHwaLoad(app_grpx_obj_t *obj)
{
    int32_t status=0;
    app_perf_stats_hwa_stats_t hwa_load;
    app_perf_hwa_id_t hwa_id;

    for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
    {
        obj->hwa_load[hwa_id] = 0;
        obj->hwa_perf[hwa_id] = 0;
    }

    #if defined(SOC_AM62A)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU1_0, &hwa_load);
    #elif defined(SOC_TDA54)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_M55, &hwa_load);
    #else
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_0, &hwa_load);
    #endif
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            if(hwa_id!=APP_PERF_HWA_GPU) /* This HWA is not controlled by mcu2-0 */
            {
                uint32_t load;
                app_perf_stats_hwa_load_t *hwaLoad;

                hwaLoad = &hwa_load.hwa_stats[hwa_id];

                if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
                {
                    load = (hwaLoad->active_time*10000)/hwaLoad->total_time;

                    obj->hwa_load[hwa_id] = (load+50)/100; /* round off and convert to 0..100 range */
                    obj->hwa_perf[hwa_id] = (uint16_t)(hwaLoad->pixels_processed/hwaLoad->total_time);
                }
            }
        }
    }
    #if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_1, &hwa_load);
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            if(hwa_id!=APP_PERF_HWA_GPU) /* This HWA is not controlled by mcu2-1 */
            {
                uint32_t load;
                app_perf_stats_hwa_load_t *hwaLoad;

                hwaLoad = &hwa_load.hwa_stats[hwa_id];

                if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
                {
                    load = (hwaLoad->active_time*10000)/hwaLoad->total_time;

                    obj->hwa_load[hwa_id] = (load+50)/100; /* round off and convert to 0..100 range */
                    obj->hwa_perf[hwa_id] = (uint16_t)(hwaLoad->pixels_processed/hwaLoad->total_time);
                }
            }
        }
    }
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU4_0, &hwa_load);
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            if(hwa_id!=APP_PERF_HWA_GPU) /* This HWA is not controlled by mcu4-0 */
            {
                uint32_t load;
                app_perf_stats_hwa_load_t *hwaLoad;

                hwaLoad = &hwa_load.hwa_stats[hwa_id];

                if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
                {
                    load = (hwaLoad->active_time*10000)/hwaLoad->total_time;

                    obj->hwa_load[hwa_id] = (load+50)/100; /* round off and convert to 0..100 range */
                    obj->hwa_perf[hwa_id] = (uint16_t)(hwaLoad->pixels_processed/hwaLoad->total_time);
                }
            }
        }
    }
    #endif
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MPU1_0, &hwa_load);
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            if(hwa_id==APP_PERF_HWA_GPU) /* This HWA is controlled by mpu1_0 */
            {
                uint32_t load = 0;
                app_perf_stats_hwa_load_t *hwaLoad;

                hwaLoad = &hwa_load.hwa_stats[hwa_id];

                if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
                {
                    FILE *fp;
                    vx_char line_str[1024];
                    vx_char *token;

                    fp = fopen("/sys/kernel/debug/pvr/gpu00/utilisation_stats","r");

                    if(fp != NULL)
                    {
                        while(fgets(line_str, sizeof(line_str), fp)!=NULL)
                        {
                            vx_char s[]=":";

                            token = strtok(line_str, s);
                            if (NULL != token)
                            {
                                if(strcmp(token, "GPU Utilisation")==0)
                                {
                                    token = strtok(NULL, s);
                                    if (NULL != token)
                                    {
                                        load = atoi(token);
                                    }
                                }
                            }
                        }
                        obj->hwa_load[hwa_id] = load;
                        fclose(fp);
                    }

                    obj->hwa_perf[hwa_id] = (uint16_t)(hwaLoad->pixels_processed/hwaLoad->total_time);
                }
            }
        }
    }
}

int32_t appGrpxGetDimHwaLoad(uint16_t *width, uint16_t *height)
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
        *width  = (prop2.width*APP_GRPX_LOAD_BAR_WIDTH_FACTOR + prop.width/2)*appGrpxGetNumHwa();
    }
    return status;
}

int32_t appGrpxShowHwaLoad(uint16_t startx, uint16_t starty)
{
    int32_t status = 0;
    app_grpx_obj_t *obj = (app_grpx_obj_t*)&g_app_grpx_obj;

    appGrpxGetHwaLoad(obj);
    appGrpxDrawHwaLoad(obj, startx, starty,
        APP_GRPX_LOAD_BAR_HEIGHT /* bar height */
        );
    return status;
}
