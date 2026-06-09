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
#include <TI/video_io_display.h>

#ifndef VDK
#define ENABLE_GRPX
#endif

app_grpx_obj_t g_app_grpx_obj;

void appGrpxDraw(app_grpx_obj_t *obj, uint16_t update_type)
{
    int32_t status = 0;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    void *data_ptr;
    uint8_t *buf_addr[3] = {NULL, NULL, NULL};

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = obj->prm.width;
    rect.end_y = obj->prm.height;

    status = vxMapImagePatch(obj->disp_image,
        &rect,
        0,
        &map_id,
        &image_addr,
        &data_ptr,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
    );

    buf_addr[0] = data_ptr;

    if(status==VX_SUCCESS)
    {
        Draw2D_updateBufAddr(obj->draw2d_obj, buf_addr);

        if(update_type==0)
        {
            /* for full update, clear buffer before calling callback */
            Draw2D_clearBuf(obj->draw2d_obj);
        }

        if(obj->prm.draw_callback)
        {
            obj->prm.draw_callback(obj->draw2d_obj, &obj->draw2d_buf_info, update_type);
        }

        vxUnmapImagePatch(obj->disp_image, map_id);
    }
    else
    {
        printf("GRPX: ERROR: Unable to map vx_image for graphics !!!\n");
    }
}

void appGrpxTaskMain(void *app_var)
{
    app_grpx_obj_t *obj = (app_grpx_obj_t*)app_var;

    while(!obj->task_stop)
    {
        uint64_t cur_time;
        uint32_t elasped_time;

        cur_time = tivxPlatformGetTimeInUsecs();

        /* only update, dont draw all graphics */
        appGrpxDraw(obj, 1);

        elasped_time = (uint32_t)(tivxPlatformGetTimeInUsecs() - cur_time)/1000; /* in msecs */

        if(elasped_time < obj->prm.update_interval)
        {
            tivxTaskWaitMsecs(obj->prm.update_interval - elasped_time);
        }
    }
    obj->task_stop_done = 1;
}

int32_t appGrpxInit(app_grpx_init_prms_t *prm)
{
    int32_t status = 0;
    #ifdef ENABLE_GRPX
    app_grpx_obj_t *obj = (app_grpx_obj_t*)&g_app_grpx_obj;

    memset(obj, 0, sizeof(app_grpx_obj_t));

    obj->logo_bmp_idx = DRAW2D_BMP_IDX_TI_LOGO_3;
    obj->prm = *prm;
    obj->disp_graph = vxCreateGraph(prm->openvx_context);
    status = vxGetStatus((vx_reference)obj->disp_graph);
    if(status!=VX_SUCCESS)
    {
        printf("GRPX: ERROR: Unable to create vx_graph for graphics !!!\n");
    }
    if(status==VX_SUCCESS)
    {
        tivx_display_params_t disp_config_prm;

        memset(&disp_config_prm, 0, sizeof(tivx_display_params_t));
        disp_config_prm.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        disp_config_prm.pipeId = prm->display_pipe;
        disp_config_prm.outWidth = prm->width;
        disp_config_prm.outHeight = prm->height;
        disp_config_prm.posX = prm->pos_x;
        disp_config_prm.posY = prm->pos_y;

        obj->disp_config = vxCreateUserDataObject(prm->openvx_context, "tivx_display_params_t", sizeof(tivx_display_params_t), &disp_config_prm);
        status = vxGetStatus((vx_reference)obj->disp_config);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to create vx_user_data_object for graphics !!!\n");
        }
    }
    if(status==VX_SUCCESS)
    {
        obj->disp_image = vxCreateImage(prm->openvx_context, prm->width, prm->height, TIVX_DF_IMAGE_RGB565);
        status = vxGetStatus((vx_reference)obj->disp_image);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to create vx_image for graphics !!!\n");
        }
    }
    if(status==VX_SUCCESS)
    {
#if !defined(SOC_AM62A)
        obj->disp_node = tivxDisplayNode(obj->disp_graph, obj->disp_config, obj->disp_image);
        status = vxGetStatus((vx_reference)obj->disp_node);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to create vx_node for graphics !!!\n");
        }
#endif
    }
    if(status==VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->disp_graph);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to verify graph for graphics !!!\n");
        }
    }
    if(status==VX_SUCCESS)
    {
        status = Draw2D_create(&obj->draw2d_obj);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to create draw2d handle for graphics !!!\n");
        }
    }
    if(status==VX_SUCCESS)
    {
        vx_imagepatch_addressing_t image_addr;
        vx_rectangle_t rect;
        vx_map_id map_id;
        void *data_ptr;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = prm->width;
        rect.end_y = prm->height;

        status = vxMapImagePatch(obj->disp_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
        );

        if(status==VX_SUCCESS)
        {
            memset(&obj->draw2d_buf_info, 0, sizeof(Draw2D_BufInfo));
            obj->draw2d_buf_info.bufAddr[0] = data_ptr;
            obj->draw2d_buf_info.bufWidth = prm->width;
            obj->draw2d_buf_info.bufHeight = prm->height;
            obj->draw2d_buf_info.bufPitch[0] = image_addr.stride_y;
            obj->draw2d_buf_info.dataFormat = DRAW2D_DF_BGR16_565;
            obj->draw2d_buf_info.transperentColor = DRAW2D_TRANSPARENT_COLOR;
            obj->draw2d_buf_info.transperentColorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;

            status = Draw2D_setBufInfo(obj->draw2d_obj, &obj->draw2d_buf_info);
            if(status!=VX_SUCCESS)
            {
                printf("GRPX: ERROR: Unable to set draw2d buf info for graphics !!!\n");
            }

            vxUnmapImagePatch(obj->disp_image, map_id);
        }
        else
        {
            printf("GRPX: ERROR: Unable to map vx_image for graphics !!!\n");
        }
    }
    if(status==VX_SUCCESS)
    {
        appGrpxDraw(obj, 0);
    }
    if(status==VX_SUCCESS)
    {
        status = vxProcessGraph(obj->disp_graph);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to process disp_graph for graphics !!!\n");
        }
    }
    if(status==VX_SUCCESS)
    {
        tivx_task_create_params_t task_prms;

        tivxTaskSetDefaultCreateParams(&task_prms);
        task_prms.task_main = appGrpxTaskMain;
        task_prms.app_var = obj;
        snprintf(task_prms.task_name, TIVX_MAX_TASK_NAME, "APP_GRPX");

        status = tivxTaskCreate(&obj->task, &task_prms);
        if(status!=VX_SUCCESS)
        {
            printf("GRPX: ERROR: Unable to create task for graphics !!!\n");
        }
        if(status==VX_SUCCESS)
        {
            obj->is_task_created = 1;
        }
    }
    if(status==VX_SUCCESS)
    {
        /* reset all perf stats counters */
        appPerfStatsResetAll();
    }
    #endif
    return status;
}

int32_t appGrpxDeInit()
{
    int32_t status = 0;
    #ifdef ENABLE_GRPX
    app_grpx_obj_t *obj = (app_grpx_obj_t*)&g_app_grpx_obj;

    if(obj->is_task_created)
    {
        obj->task_stop = 1;
        while(!obj->task_stop_done)
        {
            tivxTaskWaitMsecs(10);
        }
        tivxTaskDelete(&obj->task);
        obj->is_task_created = 0;
    }
    if(obj->draw2d_obj)
    {
        Draw2D_delete(obj->draw2d_obj);
        obj->draw2d_obj = NULL;
    }
    if(obj->disp_node)
    {
        vxReleaseNode(&obj->disp_node);
    }
    if(obj->disp_graph)
    {
        vxReleaseGraph(&obj->disp_graph);
    }
    if(obj->disp_image)
    {
        vxReleaseImage(&obj->disp_image);
    }
    if(obj->disp_config)
    {
        vxReleaseUserDataObject(&obj->disp_config);
    }
    #endif
    return status;
}

void appGrpxDrawDefault(Draw2D_Handle *draw2d_obj, Draw2D_BufInfo *draw2d_buf_info, uint32_t update_type)
{
    #ifdef ENABLE_GRPX
    uint16_t width, height, startx, starty;
    uint16_t borderx, bordery;
    uint16_t logoy;

    borderx = 20;
    bordery = 20;
    logoy = 0;

    if(update_type==0)
    {
        appGrpxShowLogo(borderx, logoy);
    }
    else
    {
        /* CPU load is at bottom left of screen */
        appGrpxGetDimCpuLoad(&width, &height);

        startx = draw2d_buf_info->bufWidth - width - borderx;
        starty = draw2d_buf_info->bufHeight - height - bordery;

        appGrpxShowCpuLoad(startx, starty);

        /* HWA load is at left of CPU load */
        appGrpxGetDimHwaLoad(&width, &height);

        startx = startx - width - borderx;

        appGrpxShowHwaLoad(startx, starty);

        /* DDR load is at left of HWA load */
        appGrpxGetDimDdrLoad(&width, &height);

        startx = startx - width - borderx;

        appGrpxShowDdrLoad(startx, starty);
    }
    #endif
}

void appGrpxInitParamsInit(app_grpx_init_prms_t *prm, vx_context context)
{
    #ifdef ENABLE_GRPX
    memset(prm, 0, sizeof(app_grpx_init_prms_t));
    prm->width = 1920;
    prm->height = 1080;
    prm->pos_x = 0;
    prm->pos_y = 0;
    prm->display_pipe = 1;
    prm->update_interval = 500;
    prm->draw_callback = appGrpxDrawDefault;
    prm->openvx_context = context;
    #endif
}


int32_t appGrpxGetDimLogo(uint16_t *width, uint16_t *height)
{
    int32_t status = 0;
    app_grpx_obj_t *obj = (app_grpx_obj_t*)&g_app_grpx_obj;
    Draw2D_BmpPrm bmp;
    Draw2D_BmpProperty prop;

    *width = *height = 0;

    bmp.bmpIdx = obj->logo_bmp_idx;

    status = Draw2D_getBmpProperty(&bmp, &prop);
    if(status==0)
    {
        *width = prop.width;
        *height = prop.height;
    }
    return status;
}

int32_t appGrpxShowLogo(uint16_t startx, uint16_t starty)
{
    int32_t status = 0;
    app_grpx_obj_t *obj = (app_grpx_obj_t*)&g_app_grpx_obj;
    Draw2D_BmpPrm bmp;

    bmp.bmpIdx = obj->logo_bmp_idx;
    status = Draw2D_drawBmp(obj->draw2d_obj, startx, starty, &bmp);

    return status;
}

void appGrpxDrawLoadBar(
                    app_grpx_obj_t *obj,
                    uint32_t cpuLoad, /* 0 .. 100 */
                    uint32_t startX,
                    uint32_t startY,
                    uint32_t width,
                    uint32_t height
                )
{
    Draw2D_RegionPrm region;
    uint32_t color[2];
    uint32_t barHeight[2];

    color[0] = RGB888_TO_RGB565(40, 40, 40);
    color[1] = RGB888_TO_RGB565(0, 160, 0);

    barHeight[0] = (height * (100 - cpuLoad))/100;

    if(barHeight[0] > height)
        barHeight[0] = height;

    barHeight[1] = height - barHeight[0];

    /* fill in active load color */
    region.color  = color[0];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX;
    region.startY = startY;
    region.height = barHeight[0];
    region.width  = width;

    Draw2D_fillRegion(obj->draw2d_obj, &region);

    /* fill active load color */
    region.color  = color[1];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX;
    region.startY = startY + barHeight[0];
    region.height = barHeight[1];
    region.width  = width;

    Draw2D_fillRegion(obj->draw2d_obj,&region);
}

void appGrpxDrawLoadBar2(
                    app_grpx_obj_t *obj,
                    uint32_t load1, /* 0 .. 100 */
                    uint32_t load2, /* 0 .. 100 */
                    uint32_t startX,
                    uint32_t startY,
                    uint32_t width,
                    uint32_t height,
                    char *label1,
                    char *label2
                )
{
    Draw2D_RegionPrm region;
    uint32_t color[3];
    uint32_t barHeight[3];
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;

    fontPrm.fontIdx = 3;
    Draw2D_getFontProperty(&fontPrm, &fontProp);

    /* clip load1 and load2 to 100 and load1+load2 to 100 */
    if(load1>100)
    {
        load1=100;
    }
    if(load2>100)
    {
        load2=100;
    }
    if((load1+load2)>100)
    {
        load2 = 100 - load1;
    }

    color[0] = RGB888_TO_RGB565(40, 40, 40);
    color[1] = RGB888_TO_RGB565(255, 128, 0);
    color[2] = RGB888_TO_RGB565(0, 160, 0);

    barHeight[0] = (height * (100 - (load1+load2)))/100;
    barHeight[1] = (height*load1)/100;
    if((barHeight[1]+barHeight[0]) > height)
    {
        barHeight[1] = height - barHeight[0];
    }
    barHeight[2] = height - barHeight[0] - barHeight[1];

    /* fill in inactive load color */
    region.color  = color[0];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX;
    region.startY = startY;
    region.height = barHeight[0];
    region.width  = width;

    Draw2D_fillRegion(obj->draw2d_obj, &region);

    /* fill active load 1 color */
    region.color  = color[1];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX;
    region.startY = startY + barHeight[0];
    region.height = barHeight[1];
    region.width  = width;

    Draw2D_fillRegion(obj->draw2d_obj,&region);

    /* fill active load 2 color */
    region.color  = color[2];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX;
    region.startY = startY + barHeight[0] + barHeight[1];
    region.height = barHeight[2];
    region.width  = width;

    Draw2D_fillRegion(obj->draw2d_obj,&region);

    /* draw legend box */

    /* for load1 */
    region.color  = color[2];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX + width + 10;
    region.startY = startY + height - fontProp.height + (fontProp.height-10)/2;
    region.height = 10;
    region.width  = 10;

    Draw2D_fillRegion(obj->draw2d_obj,&region);

    /* for load2 */
    region.color  = color[1];
    region.colorFormat = obj->draw2d_buf_info.dataFormat;
    region.startX = startX + width + 10;
    region.startY = startY + height - fontProp.height*2 + (fontProp.height-10)/2;
    region.height = 10;
    region.width  = 10;

    Draw2D_fillRegion(obj->draw2d_obj,&region);

    /* draw legend text */

    /* for load1 */
    Draw2D_drawString(obj->draw2d_obj,
              startX + width + 10*2 + 5,
              startY + height - fontProp.height,
              label2,
              &fontPrm
              );

    /* for load2 */
    Draw2D_drawString(obj->draw2d_obj,
              startX + width + 10*2 + 5,
              startY + height - fontProp.height*2,
              label1,
              &fontPrm
              );
}
