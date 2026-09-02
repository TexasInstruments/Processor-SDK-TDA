#include <vx_overlay_perf_stats_utils.h>

int32_t overlay_graph(Overlay *overlay, Stats *stats);
int32_t overlay_text(Overlay *overlay, Stats *stats);
void read_stats(Stats *stats);
void trim_string(char *s);

#define MCU_THRESHOLD (1000u) //10%

void initialize_app_perf_stats(AppPerfStats *perf_stats)
{
    perfStatsResetCpuLoadCalc(&perf_stats->stats.cpu_load);
    perfStatsSocTempInit (&perf_stats->stats.soc_temp);

    perf_stats->updateTimeuS = 1000000; // 1 Sec
    gettimeofday(&perf_stats->prevTime, NULL);
    perf_stats->frameCount = 0;
    perf_stats->firstCall = true;
    perf_stats->overlayType = OVERLAY_TYPE_NONE;
    perf_stats->numInstances = 1;

    perf_stats->overlay.imgYPtr = NULL;
    perf_stats->overlay.imgUVPtr = NULL;
    perf_stats->overlay.imgWidth = 0;
    perf_stats->overlay.imgHeight = 0;
    perf_stats->overlay.xPos = 0;
    perf_stats->overlay.yPos = 0;
    perf_stats->overlay.width = DEFAULT_OVERLAY_WIDTH;
    perf_stats->overlay.height = DEFAULT_OVERLAY_HEIGHT;
    getColor(&perf_stats->overlay.backgroundColor, 0, 0, 0);
    getColor(&perf_stats->overlay.colorWhite, 255, 255, 255);
    getColor(&perf_stats->overlay.colorBlack, 0, 0, 0);
    getColor(&perf_stats->overlay.colorGrey, 45, 45, 45);
    getColor(&perf_stats->overlay.colorRed, 255, 43, 43);
    getColor(&perf_stats->overlay.colorYellow, 245, 227, 66);
    getColor(&perf_stats->overlay.colorGreen, 43, 255, 43);
    getColor(&perf_stats->overlay.colorPurple, 113, 0, 199);
    getColor(&perf_stats->overlay.colorOrange, 255, 150, 46);
    getColor(&perf_stats->overlay.colorPink, 204, 73, 131);
    perf_stats->overlay.show_fps = true;
    perf_stats->overlay.show_temp = true;
}

int32_t update_app_perf_stats(AppPerfStats *perf_stats)
{
    struct timeval curr_time;
    long unsigned int time_diff;

    perf_stats->frameCount++;

    gettimeofday(&curr_time, NULL);
    time_diff = (curr_time.tv_sec - perf_stats->prevTime.tv_sec) * 1000000 +
                curr_time.tv_usec - perf_stats->prevTime.tv_usec;

    if(time_diff > perf_stats->updateTimeuS)
    {
        double factor = (double)perf_stats->frameCount/(double)time_diff;
        perf_stats->stats.fps = round(1000000 * factor)/perf_stats->numInstances;
        perf_stats->frameCount = 0;
        read_stats(&perf_stats->stats);
        perf_stats->prevTime = curr_time;
    }
    else if(perf_stats->firstCall)
    {
        perf_stats->stats.fps = 0;
        read_stats(&perf_stats->stats);
        perf_stats->firstCall = false;
    }

    if (OVERLAY_TYPE_GRAPH == perf_stats->overlayType)
    {
        return  overlay_graph(&perf_stats->overlay, &perf_stats->stats);
    }
    else if (OVERLAY_TYPE_TEXT == perf_stats->overlayType)
    {
        return  overlay_text(&perf_stats->overlay, &perf_stats->stats);
    }

    return 0;
}

int32_t overlay_graph(Overlay *overlay, Stats *stats)
{
    uint32_t i, value;
    char buffer[24];
    uint32_t graph_width, graph_height;
    uint32_t graph_pos_x, graph_pos_y;
    uint32_t graph_offset_x;
    uint32_t cpu_temp_idx = 0;

    Image imageHandler;

    FontProperty value_font;
    FontProperty title_font;
    FontProperty fps_font;

    if (NULL == overlay->imgYPtr || NULL == overlay->imgUVPtr)
    {
        return -1;
    }
    if (0 == overlay->imgWidth || 0 == overlay->imgHeight)
    {
        return -1;
    }

    imageHandler.width = overlay->imgWidth;
    imageHandler.height = overlay->imgHeight;
    imageHandler.yRowAddr = overlay->imgYPtr;
    imageHandler.uvRowAddr = overlay->imgUVPtr;
    
    fillRegion(&imageHandler,
               overlay->xPos,
               overlay->yPos,
               overlay->width,
               overlay->height,
               &overlay->backgroundColor);

    graph_width = 0.03 * overlay->width;
    graph_height = 0.5 * overlay->height;
    graph_offset_x = (overlay->width - (graph_width * stats->stats_count)) / stats->stats_count;
    graph_pos_x = graph_offset_x / 2;

    getFont(&fps_font, (int)(0.008 * overlay->width));

    if(stats->stats_count <= 12)
    {
        getFont(&value_font, (int)(0.01 * overlay->width));
        getFont(&title_font, (int)(0.01 * overlay->width));
    }
    else if (stats->stats_count <= 18)
    {
        getFont(&value_font, (int)(0.008 * overlay->width));
        getFont(&title_font, (int)(0.008 * overlay->width));
    }
    else
    {
        getFont(&value_font, (int)(0.006 * overlay->width));
        getFont(&title_font, (int)(0.006 * overlay->width));
    }

    graph_pos_y = overlay->yPos + (overlay->height - (value_font.height + graph_height + title_font.height))/2;

    if(overlay->show_fps)
    {
        sprintf(buffer, "FPS:%d", stats->fps);
        trim_string(buffer);
        drawText(&imageHandler,
                buffer,
                0,
                overlay->height - fps_font.height,
                &fps_font,
                &overlay->colorWhite);
    }

    if(overlay->show_temp && NUM_THERMAL_ZONE > 0)
    {
        for (i = 0; i < NUM_THERMAL_ZONE; i++)
        {
            if(NULL != strstr (stats->soc_temp.thermal_zone_name[i], "CPU"))
            {
                cpu_temp_idx = i;
                break;
            }
        }

        sprintf(buffer, "TEMP:%d", (int) stats->soc_temp.thermal_zone_temp[cpu_temp_idx]);
        trim_string(buffer);
        drawText(&imageHandler,
                 buffer,
                 overlay->width - (8*fps_font.width),
                 overlay->height - fps_font.height,
                 &fps_font,
                 &overlay->colorWhite);
    }

    i = 0;
    initGraph(&overlay->graphs[i],
              &imageHandler,
              graph_pos_x,
              graph_pos_y,
              graph_width,
              graph_height,
              100,
              "mpu",
              "%",
              &value_font,
              &title_font,
              &overlay->colorWhite,
              &overlay->colorGreen,
              &overlay->colorGrey);
    value = stats->cpu_load.cpu_load / 100u;
    if (value > 50 && value <= 80)
    {
        overlay->graphs[i].m_fillColor = &overlay->colorYellow;
    }
    else if (value > 80)
    {
        overlay->graphs[i].m_fillColor = &overlay->colorRed;
    }
    updateGraph (&overlay->graphs[i],value);
    graph_pos_x += graph_offset_x + graph_width;
    i++;

#if defined(A72) || defined(A53) || defined(A720)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
    app_perf_stats_cpu_load_t cpu_load;
    for (uint32_t j = 0; j < APP_IPC_CPU_MAX; j++)
    {
        const char *cpuName = appIpcGetCpuName(j);
        if (appIpcIsCpuEnabled(j))
        {
            if((NULL != strstr(cpuName, "c7x")) ||
               (NULL != strstr(cpuName, "mcu") &&
               (stats->cpu_loads[j].cpu_load >= MCU_THRESHOLD)))
            {
                cpu_load = stats->cpu_loads[j];
                initGraph(&overlay->graphs[i],
                        &imageHandler,
                        graph_pos_x,
                        graph_pos_y,
                        graph_width,
                        graph_height,
                        100,
                        cpuName,
                        "%",
                        &value_font,
                        &title_font,
                        &overlay->colorWhite,
                        &overlay->colorGreen,
                        &overlay->colorGrey);
                value = cpu_load.cpu_load / 100u;
                if (value > 50 && value <= 80)
                {
                    overlay->graphs[i].m_fillColor = &overlay->colorYellow;
                }
                else if (value > 80)
                {
                    overlay->graphs[i].m_fillColor = &overlay->colorRed;
                }
                updateGraph (&overlay->graphs[i],value);
                graph_pos_x += graph_offset_x + graph_width;
                i++;
            }
        }
    }

    for (uint32_t j = 0; j < stats->hwa_count; j++)
    {
        app_perf_stats_hwa_load_t *hwaLoad;
        for (uint32_t k = 0; k < APP_PERF_HWA_MAX; k++)
        {
            app_perf_hwa_id_t id = (app_perf_hwa_id_t) k;
            hwaLoad = &stats->hwa_loads[j].hwa_stats[id];
            if (hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                initGraph(&overlay->graphs[i],
                          &imageHandler,
                          graph_pos_x,
                          graph_pos_y,
                          graph_width,
                          graph_height,
                          100,
                          appPerfStatsGetHwaName(id),
                          "%",
                          &value_font,
                          &title_font,
                          &overlay->colorWhite,
                          &overlay->colorGreen,
                          &overlay->colorGrey);
                uint64_t load = (hwaLoad->active_time * 10000) / hwaLoad->total_time;
                value = load / 100;
                if (value > 50 && value <= 80)
                {
                    overlay->graphs[i].m_fillColor = &overlay->colorYellow;
                }
                else if (value > 80)
                {
                    overlay->graphs[i].m_fillColor = &overlay->colorRed;
                }
                updateGraph (&overlay->graphs[i],value);
                graph_pos_x += graph_offset_x + graph_width;
                i++;
            }
        }
    }
#endif
#endif

    initGraph(&overlay->graphs[i],
              &imageHandler,
              graph_pos_x,
              graph_pos_y,
              graph_width,
              graph_height,
              stats->ddr_load.total_available_bw,
              "DDR R",
              "MB/s",
              &value_font,
              &title_font,
              &overlay->colorWhite,
              &overlay->colorPurple,
              &overlay->colorGrey);
    updateGraph (&overlay->graphs[i], stats->ddr_load.read_bw_avg);
    graph_pos_x += graph_offset_x + graph_width;
    i++;

    initGraph(&overlay->graphs[i],
              &imageHandler,
              graph_pos_x,
              graph_pos_y,
              graph_width,
              graph_height,
              stats->ddr_load.total_available_bw,
              "DDR W",
              "MB/s",
              &value_font,
              &title_font,
              &overlay->colorWhite,
              &overlay->colorOrange,
              &overlay->colorGrey);
    updateGraph (&overlay->graphs[i], stats->ddr_load.write_bw_avg);
    graph_pos_x += graph_offset_x + graph_width;
    i++;

    initGraph(&overlay->graphs[i],
              &imageHandler,
              graph_pos_x,
              graph_pos_y,
              graph_width,
              graph_height,
              stats->ddr_load.total_available_bw,
              "DDR T",
              "MB/s",
              &value_font,
              &title_font,
              &overlay->colorWhite,
              &overlay->colorPink,
              &overlay->colorGrey);
    value = stats->ddr_load.write_bw_avg + stats->ddr_load.read_bw_avg;
    updateGraph (&overlay->graphs[i], value);

    return 0;
}

int32_t overlay_text(Overlay *overlay, Stats *stats)
{
    uint32_t overlay_height;
    uint32_t text_y_pos;
    uint32_t value;

    char buffer[50];

    Image imageHandler;
    
    FontProperty text_font;
    YUVColor *text_color;

    if (NULL == overlay->imgYPtr || NULL == overlay->imgUVPtr)
    {
        return -1;
    }
    if (0 == overlay->imgWidth || 0 == overlay->imgHeight)
    {
        return -1;
    }

    imageHandler.width = overlay->imgWidth;
    imageHandler.height = overlay->imgHeight;
    imageHandler.yRowAddr = overlay->imgYPtr;
    imageHandler.uvRowAddr = overlay->imgUVPtr;

    getFont(&text_font, (int)(0.005 * overlay->width));

    overlay_height = stats->stats_count * text_font.height + 10;
    if(overlay->show_fps)
    {
        overlay_height += text_font.height;
    }
    if(overlay->show_temp)
    {
        overlay_height += text_font.height;
    }

    text_y_pos = overlay->imgHeight - overlay_height + 5;

    fillRegion(&imageHandler,
               0,
               overlay->imgHeight - overlay_height,
               text_font.width * 20,
               overlay_height,
               &overlay->backgroundColor);

    value = stats->cpu_load.cpu_load / 100u;
    if (value <= 50)
    {
        text_color = &overlay->colorGreen;
    }
    else if (value > 50 && value <= 80)
    {
        text_color = &overlay->colorYellow;
    }
    else
    {
        text_color = &overlay->colorRed;
    }
    sprintf(buffer, "MPU: %d%%", value);
    trim_string(buffer);
    drawText(&imageHandler,
             buffer,
             5,
             text_y_pos,
             &text_font,
             text_color);
    text_y_pos += text_font.height;

#if defined(A72) || defined(A53) || defined(A720)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
    app_perf_stats_cpu_load_t cpu_load;
    for (uint32_t i = 0; i < APP_IPC_CPU_MAX; i++)
    {
        const char *cpuName = appIpcGetCpuName(i);
        if (appIpcIsCpuEnabled(i))
        {
            if((NULL != strstr(cpuName, "c7x")) ||
               (NULL != strstr(cpuName, "mcu") &&
               (stats->cpu_loads[i].cpu_load >= MCU_THRESHOLD)))
            {
                cpu_load = stats->cpu_loads[i];
                value = cpu_load.cpu_load / 100u;
                if (value <= 50)
                {
                    text_color = &overlay->colorGreen;
                }
                else if (value > 50 && value <= 80)
                {
                    text_color = &overlay->colorYellow;
                }
                else
                {
                    text_color = &overlay->colorRed;
                }
                sprintf(buffer, "%s: %d%%", cpuName, value);
                trim_string(buffer);
                drawText(&imageHandler,
                        buffer,
                        5,
                        text_y_pos,
                        &text_font,
                        text_color);
                text_y_pos += text_font.height;
            }
        }
    }

    for (uint32_t i = 0; i < stats->hwa_count; i++)
    {
        app_perf_stats_hwa_load_t *hwaLoad;
        for (uint32_t j = 0; j < APP_PERF_HWA_MAX; j++)
        {
            app_perf_hwa_id_t id = (app_perf_hwa_id_t) j;
            hwaLoad = &stats->hwa_loads[i].hwa_stats[id];
            if (hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                uint64_t load = (hwaLoad->active_time * 10000) / hwaLoad->total_time;
                value = load / 100;
                if (value <= 50)
                {
                    text_color = &overlay->colorGreen;
                }
                else if (value > 50 && value <= 80)
                {
                    text_color = &overlay->colorYellow;
                }
                else
                {
                    text_color = &overlay->colorRed;
                }
                sprintf(buffer, "%s: %d%%", appPerfStatsGetHwaName (id), value);
                trim_string(buffer);
                drawText(&imageHandler,
                         buffer,
                         5,
                         text_y_pos,
                         &text_font,
                         text_color);
                text_y_pos += text_font.height;
            }
        }
    }
#endif
#endif

    sprintf(buffer, "DDR RD: %d MB/s", stats->ddr_load.read_bw_avg);
    trim_string(buffer);
    drawText(&imageHandler,
             buffer,
             5,
             text_y_pos,
             &text_font,
             &overlay->colorPurple);
    text_y_pos += text_font.height;

    sprintf(buffer, "DDR WR: %d MB/s", stats->ddr_load.write_bw_avg);
    trim_string(buffer);
    drawText(&imageHandler,
             buffer,
             5,
             text_y_pos,
             &text_font,
             &overlay->colorOrange);
    text_y_pos += text_font.height;

    sprintf(buffer, "DDR Tot: %d MB/s", stats->ddr_load.write_bw_avg + stats->ddr_load.read_bw_avg);
    trim_string(buffer);
    drawText(&imageHandler,
             buffer,
             5,
             text_y_pos,
             &text_font,
             &overlay->colorPink);
    text_y_pos += text_font.height;

    if(overlay->show_fps)
    {
        sprintf(buffer, "FPS:%d", stats->fps);
        trim_string(buffer);
        drawText(&imageHandler,
                buffer,
                5,
                text_y_pos,
                &text_font,
                &overlay->colorWhite);
        text_y_pos += text_font.height;
    }

    if(overlay->show_temp && NUM_THERMAL_ZONE > 0)
    {
        uint32_t cpu_temp_idx = 0;
        for (uint32_t i = 0; i < NUM_THERMAL_ZONE; i++)
        {
            if(NULL != strstr (stats->soc_temp.thermal_zone_name[i], "CPU"))
            {
                cpu_temp_idx = i;
                break;
            }
        }

        sprintf(buffer, "TEMP:%d", (int) stats->soc_temp.thermal_zone_temp[cpu_temp_idx]);
        trim_string(buffer);
        drawText(&imageHandler,
                 buffer,
                 5,
                 text_y_pos,
                 &text_font,
                 &overlay->colorWhite);
    }

    return 0;
}

void read_stats(Stats *stats)
{
    // CPU Load
    perfStatsCpuLoadCalc(&stats->cpu_load);
    perfStatsResetCpuLoadCalc(&stats->cpu_load);

    // DDR Load
    stats->ddr_load = *(perfStatsDdrStatsGet());
    perfStatsResetDdrLoadCalcAll();

    // Calc temp
    perfStatsSocTempGet(&stats->soc_temp);

#if defined(A72) || defined(A53) || defined(A720)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
    // C7x and MCU Load
    for (uint32_t i = 0; i < APP_IPC_CPU_MAX; i++)
    {
        const char *cpuName = appIpcGetCpuName(i);
        if (appIpcIsCpuEnabled(i) &&
            (NULL != strstr(cpuName, "c7x") ||
            NULL != strstr(cpuName, "mcu")))
        {
            appPerfStatsCpuLoadGet(i, &stats->cpu_loads[i]);
        }
    }

    // HWA
    stats->hwa_count = 0;
#if defined(SOC_AM62A)
    appPerfStatsHwaStatsGet (APP_IPC_CPU_MCU1_0, &stats->hwa_loads[stats->hwa_count++]);
#else
    appPerfStatsHwaStatsGet (APP_IPC_CPU_MCU2_0, &stats->hwa_loads[stats->hwa_count++]);
#if !defined(SOC_J722S)
    appPerfStatsHwaStatsGet (APP_IPC_CPU_MCU2_1, &stats->hwa_loads[stats->hwa_count++]);
#endif
#if defined(SOC_J784S4) || defined(SOC_J742S2)
    appPerfStatsHwaStatsGet (APP_IPC_CPU_MCU4_0, &stats->hwa_loads[stats->hwa_count++]);
#endif
#endif
    appPerfStatsHwaStatsGet (APP_IPC_CPU_MPU1_0, &stats->hwa_loads[stats->hwa_count++]);

    // Reset C7X and MCU load
    for (uint32_t i = 0; i < APP_IPC_CPU_MAX; i++)
    {
        const char *cpuName = appIpcGetCpuName(i);
        if (appIpcIsCpuEnabled(i) &&
            (NULL != strstr(cpuName, "c7x") ||
            NULL != strstr(cpuName, "mcu")))
        {
            appPerfStatsCpuLoadReset(i);
        }
    }
#endif
#endif
    // Reset HWA
#if defined(A72) || defined(A53) || defined(A720)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
#if defined(SOC_AM62A)
    appRemoteServiceRun(APP_IPC_CPU_MCU1_0, APP_PERF_STATS_SERVICE_NAME,
                        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC, NULL, 0, 0);
#else
    appRemoteServiceRun(APP_IPC_CPU_MCU2_0, APP_PERF_STATS_SERVICE_NAME,
                        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC, NULL, 0, 0);
#if !defined(SOC_J722S)
    appRemoteServiceRun(APP_IPC_CPU_MCU2_1, APP_PERF_STATS_SERVICE_NAME,
                        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC, NULL, 0, 0);
#endif
#if defined(SOC_J784S4) || defined(SOC_J742S2)
    appRemoteServiceRun(APP_IPC_CPU_MCU4_0, APP_PERF_STATS_SERVICE_NAME,
                        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC, NULL, 0, 0);
#endif
#endif
    appRemoteServiceRun(APP_IPC_CPU_MPU1_0, APP_PERF_STATS_SERVICE_NAME,
                        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC, NULL, 0, 0);
#endif
#endif
    stats->stats_count = 0;
#if defined(A72) || defined(A53) || defined(A720)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)

    // C7x and MCU Load
    for (uint32_t i = 0; i < APP_IPC_CPU_MAX; i++)
    {
        const char *cpuName = appIpcGetCpuName(i);
        if (appIpcIsCpuEnabled(i))
        {
            if((NULL != strstr(cpuName, "c7x")) ||
               (NULL != strstr(cpuName, "mcu") &&
               (stats->cpu_loads[i].cpu_load >= MCU_THRESHOLD)))
            {
                stats->stats_count++;
            }
        }
    }

    //HWA
    for (uint32_t i = 0; i < stats->hwa_count; i++)
    {
        app_perf_stats_hwa_load_t *hwaLoad;
        for (uint32_t j = 0; j < APP_PERF_HWA_MAX; j++)
        {
            app_perf_hwa_id_t id = (app_perf_hwa_id_t) j;
            hwaLoad = &stats->hwa_loads[i].hwa_stats[id];
            if (hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                stats->stats_count++;
            }
        }
    }
#endif
#endif
    stats->stats_count += 4;
}

void trim_string(char *s)
{
    uint32_t i,j;
	for(i=0;s[i]==' '||s[i]=='\t';i++);

	for(j=0;s[i];i++)
	{
		s[j++]=s[i];
	}
	s[j]='\0';
	for(i=0;s[i]!='\0';i++)
	{
		if(s[i]!=' '&& s[i]!='\t')
				j=i;
	}
	s[j+1]='\0';
}
