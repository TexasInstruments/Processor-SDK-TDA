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

#include "app_test.h"
#include <app_init.h>
#include <TI/video_io_kernels.h>

static AppObj g_AppObj;

extern IssAeDynamicParams g_ae_dynPrms;

static void x86_app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[]);
static void x86_app_init(AppObj *obj);
static void x86_app_deinit(AppObj *obj);
static vx_status x86_app_create_graph(AppObj *obj);
static vx_status x86_app_run_graph(AppObj *obj);
static void x86_app_delete_graph(AppObj *obj);

int main(int argc, char* argv[])
{
    AppObj *obj = &g_AppObj;
    vx_status status = VX_SUCCESS;
    if(status == VX_SUCCESS)
    {
        x86_app_parse_cmd_line_args(obj, argc, argv);
        x86_app_init(obj);
        APP_PRINTF("x86_app_init done\n");
    }
    if(status == VX_SUCCESS)
    {
        status = x86_app_create_graph(obj);
        APP_PRINTF("x86_app_create_graph done\n");
    }
    if(status == VX_SUCCESS)
    {
        status = x86_app_run_graph(obj);
        APP_PRINTF("x86_app_run_graph done\n");
    }
    x86_app_delete_graph(obj);
    APP_PRINTF("x86_app_delete_graph done\n");
    x86_app_deinit(obj);
    APP_PRINTF("x86_app_deinit done\n");
    printf("obj->test_mode = %d\n", obj->test_mode);
    printf("obj->sensor_sel = %d\n", obj->sensor_sel);
    if(obj->test_mode == 1)
    {
        if((test_result == vx_false_e) || (status == VX_FAILURE))
        {
            printf("\n\nTEST FAILED\n\n");
            print_new_checksum_structs();
            status = (status == VX_SUCCESS) ? VX_FAILURE : status;
        }
        else
        {
            printf("\n\nTEST PASSED\n\n");
        }
    }
    return 0;
}
static void x86_app_init(AppObj *obj)
{
    appInit();

    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    tivxHwaLoadKernels(obj->context);
    tivxVideoIOLoadKernels(obj->context);
    tivxImagingLoadKernels(obj->context);
    APP_PRINTF("tivxImagingLoadKernels done\n");
}
static void x86_app_deinit(AppObj *obj)
{
    tivxHwaUnLoadKernels(obj->context);
    APP_PRINTF("tivxHwaUnLoadKernels done\n");

    tivxVideoIOUnLoadKernels(obj->context);
    APP_PRINTF("tivxVideoIOUnLoadKernels done\n");

    tivxImagingUnLoadKernels(obj->context);
    APP_PRINTF("tivxImagingUnLoadKernels done\n");

    vxReleaseContext(&obj->context);
    APP_PRINTF("vxReleaseContext done\n");

    appDeInit();
    APP_PRINTF("tivxDeInit done\n");
}

static int32_t SetExpPrgFxn(IssAeDynamicParams *p_ae_dynPrms)
{
    uint8_t count = 0;
    p_ae_dynPrms->targetBrightnessRange.min = 30;
    p_ae_dynPrms->targetBrightnessRange.max = 45;
    p_ae_dynPrms->targetBrightness = 35;
    p_ae_dynPrms->threshold = 1;
    p_ae_dynPrms->exposureTimeStepSize = 1;
    p_ae_dynPrms->enableBlc = 0;

    p_ae_dynPrms->exposureTimeRange[count].min = 100;
    p_ae_dynPrms->exposureTimeRange[count].max = 40000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 1024;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->exposureTimeRange[count].min = 40000;
    p_ae_dynPrms->exposureTimeRange[count].max = 40000;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 15872;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (0);
}

static vx_status x86_app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    obj->configuration = NULL;
    obj->ae_awb_result = NULL;
    obj->raw = NULL;
    obj->y12 = NULL;
    obj->uv12_c1 = NULL;
    obj->y8_r8_c2 = NULL;
    obj->uv8_g8_c3 = NULL;
    obj->s8_b8_c4 = NULL;
    obj->histogram = NULL;
    obj->h3a_aew_af = NULL;

    tivx_raw_image_create_params_t raw_params;

    obj->graph = vxCreateGraph(obj->context);
    APP_ASSERT_VALID_REF(obj->graph);

    APP_ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1));

/*Hardcode for IMX390*/
    raw_params.width = obj->width_in;
    raw_params.height = obj->height_in;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = obj->sensor_raw_bpp - 1;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    obj->raw = tivxCreateRawImage(obj->context, &raw_params);


/*
    Read DCC from the PC folder "dcc_path"
*/
    if(obj->cam_dcc_id != -1)
    {
        int dcc_num_bytes;
        int dcc_bytes_read;
        uint8_t * dcc_buf;

        dcc_num_bytes = get_dcc_dir_size(obj->dcc_path);
        if(dcc_num_bytes > 0 )
        {
            dcc_buf = malloc(dcc_num_bytes);
            if(NULL != dcc_buf)
            {
                dcc_bytes_read = get_dcc_dir_data(obj->dcc_path, dcc_buf);
                printf("%d dcc_bytes Read from %s \n", dcc_bytes_read, obj->dcc_path);
                if(dcc_bytes_read > 0)
                {

                    status = appSplitVpacDcc(dcc_buf, dcc_bytes_read,
                                        &obj->fs_dcc_buf_viss, &obj->fs_dcc_numbytes_viss,
                                        &obj->fs_dcc_buf_2a, &obj->fs_dcc_numbytes_2a,
                                        &obj->fs_dcc_buf_ldc, &obj->fs_dcc_numbytes_ldc);
                }
                free(dcc_buf);
            }
            else
            {
                printf("Error allocating %d bytes\n", dcc_num_bytes);
            }
        }
    }

    status = app_create_viss(obj, obj->sensor_wdr_mode);
    if(VX_SUCCESS != status)
    {
        printf("app_create_viss failed \n");
        return -1;
    }

    /*This is needed only in x86 mode*/
    SetExpPrgFxn(&g_ae_dynPrms);

    status = app_create_aewb(obj, obj->sensor_wdr_mode);
    if(VX_SUCCESS != status)
    {
        printf("app_create_aewb failed \n");
        return -1;
    }

    if (obj->ldc_enable)
    {
        vx_image ldc_in_image = obj->y8_r8_c2;
        app_create_ldc(obj, ldc_in_image);
    }

    status = vxVerifyGraph(obj->graph);
    APP_ASSERT(status==VX_SUCCESS);

    status = tivxExportGraphToDot(obj->graph,".", "vx_x86_app_single_cam");
    APP_ASSERT(status==VX_SUCCESS);

    APP_PRINTF("x86_app_create_graph done\n");
    return status;
}

static void x86_app_delete_graph(AppObj *obj)
{
    APP_PRINTF("releasing raw\n");
    tivxReleaseRawImage(&obj->raw);

    APP_PRINTF("releasing VISS node and data objects\n");
    app_delete_viss(obj);

    APP_PRINTF("releasing AEWB node and data objects\n");
    app_delete_aewb(obj);

    APP_PRINTF("releasing LDC node and data objects\n");
    app_delete_ldc(obj);

    APP_PRINTF("releasing graph\n");
    vxReleaseGraph(&obj->graph);
    APP_PRINTF("releasing graph done\n");
}

extern tivx_ae_awb_params_t g_ae_awb_result;

static vx_status x86_app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    char input_file_name[APP_MAX_FILE_PATH];
    char output_file_name_img[APP_MAX_FILE_PATH];
    vx_int32 numBytesFileIO;
    vx_uint8 i;
    char dir_name[APP_MAX_FILE_PATH];
    FILE* fp_aewb_res = NULL;
    char aewb_csv_file_name[APP_MAX_FILE_PATH];
    FILE* fp_aewb_dbg = NULL;
    char aewb_dbg_csv_file_name[APP_MAX_FILE_PATH];

    /* create output directory if not already existing */
    snprintf(dir_name, APP_MAX_FILE_PATH, "%s/%s",
        obj->test_folder_root, "/output");
    mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);

    /* create output/viss directory if not already existing */
    snprintf(dir_name, APP_MAX_FILE_PATH, "%s/%s",
        obj->test_folder_root, "/output/viss");
    mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);


    /* create output/ldc directory if not already existing */
    snprintf(dir_name, APP_MAX_FILE_PATH, "%s/%s",
        obj->test_folder_root, "/output/ldc");
    mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);


    /* create output/h3a directory if not already existing */
    snprintf(dir_name, APP_MAX_FILE_PATH, "%s/%s",
        obj->test_folder_root, "/output/h3a");
    mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);


    /* create output/aewb directory if not already existing */
    snprintf(dir_name, APP_MAX_FILE_PATH, "%s/%s",
        obj->test_folder_root, "/output/aewb");
    mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);

    if(obj->ae_awb_result)
    {
        snprintf(aewb_csv_file_name, APP_MAX_FILE_PATH,
        "%s/output/aewb/aewb_res.csv", obj->test_folder_root);

        fp_aewb_res = fopen(aewb_csv_file_name, "w");
        fprintf(fp_aewb_res, "frame_num,expTime,analog_gain,colorTemp,wb_gain0,wb_gain1,wb_gain2,wb_gain3 \n");

        snprintf(aewb_dbg_csv_file_name, APP_MAX_FILE_PATH,
        "%s/output/aewb/aewb_dbg.csv", obj->test_folder_root);

        fp_aewb_dbg = fopen(aewb_dbg_csv_file_name, "w");
        fprintf(fp_aewb_dbg, "frame_num,ae_valid,ae_converged,awb_valid,awb_converged \n");
    }

    appUpdateVpacDcc(obj->fs_dcc_buf_viss, obj->fs_dcc_numbytes_viss, obj->context,
        obj->node_viss, 0,
        NULL, 0,
        NULL, 0
    );

    appUpdateVpacDcc(obj->fs_dcc_buf_2a, obj->fs_dcc_numbytes_2a, obj->context,
        NULL, 0,
        obj->node_aewb, 0,
        NULL, 0
    );

    appUpdateVpacDcc(obj->fs_dcc_buf_ldc, obj->fs_dcc_numbytes_ldc, obj->context,
        NULL, 0,
        NULL, 0,
        obj->node_ldc, 0
    );

/*The application reads and  processes NUM_FRAMES_TO_PROCESS images
AEWB result is available after 1 frame and is applied after 2 frames
Therefore, first 2 output images will have wrong colors
*/
    char output_file_path[3] = ".";

    for(i=obj->start_seq; i<obj->num_frames_to_process; i++)
    {
        if (obj->test_mode == 1)
        {
            snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/input2.raw", obj->test_folder_root);
        }
        else
        {
            snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/input/img_%04d.raw", obj->test_folder_root, i);
        }
        APP_PRINTF(" : Loading [%s] ...\n", input_file_name);
        if(-1 == read_test_image_raw(input_file_name, obj->raw, 2))
        {
            printf("Failed to read file %s\n", input_file_name);
            return -1;
        }

        APP_PRINTF(" Running graph ...\n");
        status = vxScheduleGraph(obj->graph);
        APP_ASSERT(status==VX_SUCCESS);

        APP_PRINTF(" Waiting for graph to finish...\n");
        status = vxWaitGraph(obj->graph);
        APP_ASSERT(status==VX_SUCCESS);
        APP_PRINTF(" Graph finished...\n");

        snprintf(output_file_name_img, APP_MAX_FILE_PATH, "%s/output/viss/img_viss_%04d.yuv", obj->test_folder_root, i);
        APP_PRINTF(" : Saving [%s] ...\n", output_file_name_img);
        numBytesFileIO = write_output_image_nv12_8bit(output_file_name_img, obj->y8_r8_c2);
        APP_ASSERT(numBytesFileIO >= 0);

#ifdef VPAC3
        if (obj->vpac3_dual_fcp_enable)
        {
            snprintf(output_file_name_img, APP_MAX_FILE_PATH, "%s/output/viss/img_viss_%04d.yuv12", obj->test_folder_root, i);
            APP_PRINTF(" : Saving [%s] ...\n", output_file_name_img);
            numBytesFileIO = write_output_image_nv12_8bit(output_file_name_img, obj->y12);
            APP_ASSERT(numBytesFileIO >= 0);
        }
#endif
        snprintf(output_file_name_img, APP_MAX_FILE_PATH, "%s/output/h3a/img_h3a_%04d.bin", obj->test_folder_root, i);
        APP_PRINTF(" : Saving [%s] ...\n", output_file_name_img);
        numBytesFileIO = write_h3a_image(output_file_name_img, obj->h3a_aew_af);
        APP_ASSERT(numBytesFileIO >= 0);

        if(fp_aewb_res)
        {
            int wb_index;
            fprintf(fp_aewb_res, "%d,",i);
            fprintf(fp_aewb_res, "%d,",g_ae_awb_result.exposure_time);
            fprintf(fp_aewb_res, "%d,",g_ae_awb_result.analog_gain);
            fprintf(fp_aewb_res, "%d,",g_ae_awb_result.color_temperature);
            for(wb_index=0;wb_index<4;wb_index++)
            {
                fprintf(fp_aewb_res, "%d,",g_ae_awb_result.wb_gains[wb_index]);
            }
            fprintf(fp_aewb_res, "\n");
            fflush(fp_aewb_res);
        }

        if(fp_aewb_dbg)
        {
            fprintf(fp_aewb_dbg, "%d,",i);
            fprintf(fp_aewb_dbg, "%d,",g_ae_awb_result.ae_valid);
            fprintf(fp_aewb_dbg, "%d,",g_ae_awb_result.ae_converged);
            fprintf(fp_aewb_dbg, "%d,",g_ae_awb_result.awb_valid);
            fprintf(fp_aewb_dbg, "%d,",g_ae_awb_result.awb_converged);
            fprintf(fp_aewb_dbg, "\n");
            fflush(fp_aewb_dbg);
        }

        if (obj->ldc_enable)
        {
            snprintf(output_file_name_img, APP_MAX_FILE_PATH, "%s/output/ldc/img_ldc_%04d.yuv", obj->test_folder_root, i);
            APP_PRINTF(" : Saving [%s] ...\n", output_file_name_img);
            numBytesFileIO = write_output_image_nv12_8bit(output_file_name_img, obj->ldc_out);
            APP_ASSERT(numBytesFileIO >= 0);
            if((obj->test_mode == 1) && (i > TEST_BUFFER) && (status == VX_SUCCESS))
            {
                vx_uint32 actual_checksum = 0;
                if(app_test_check_image(obj->ldc_out, checksums_expected[obj->sensor_sel][0], &actual_checksum) == vx_false_e)
                {
                    test_result = vx_false_e;
                }
                populate_gatherer(obj->sensor_sel, 0, actual_checksum);
            }
        }
        else
        {
            if((obj->test_mode == 1) && (i > TEST_BUFFER) && (status == VX_SUCCESS))
            {
                vx_uint32 actual_checksum = 0;
                if(app_test_check_image(obj->y8_r8_c2, checksums_expected[obj->sensor_sel][0], &actual_checksum) == vx_false_e)
                {
                    test_result = vx_false_e;
                }
                populate_gatherer(obj->sensor_sel, 0, actual_checksum);
            }
        }
    }

    if(fp_aewb_res)
    {
        printf("Closing fp_aewb_res \n");
        fclose(fp_aewb_res);
    }
    if(fp_aewb_dbg)
    {
        printf("Closing fp_aewb_dbg \n");
        fclose(fp_aewb_dbg);
    }

    return status;
}

static void app_show_usage(int argc, char* argv[])
{
    APP_PRINTF("\n");
    APP_PRINTF(" Single Cam PC Emulation - (c) Texas Instruments 2021\n");
    APP_PRINTF(" ========================================================\n");
    APP_PRINTF("\n");
    APP_PRINTF(" Usage,\n");
    APP_PRINTF("  %s --cfg <config file>\n", argv[0]);
    APP_PRINTF("\n");
}

static void x86_app_parse_cfg_file(AppObj *obj, char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    char line_str[1024];
    char *token;
    struct stat s;

    if(fp==NULL)
    {
        APP_PRINTF("# ERROR: Unable to open config file [%s]\n", cfg_file_name);
        exit(0);
    }

    while(fgets(line_str, sizeof(line_str), fp)!=NULL)
    {
        char s[]=" \t";

        if (strchr(line_str, '#'))
        {
            continue;
        }

        /* get the first token */
        token = strtok(line_str, s);
        if(strcmp(token, "test_folder_root")==0)
        {
            struct stat FileAttrib;
            token = strtok(NULL, s);
            token[strlen(token)-1]=0;
            strcpy(obj->test_folder_root, token);
            snprintf(obj->dcc_path, APP_MAX_FILE_PATH, "%s/%s", obj->test_folder_root, "dcc_bins");
            APP_PRINTF(" Test folder root = [%s]\n", obj->test_folder_root);
            if (stat((const char*)obj->test_folder_root, &FileAttrib))
            {
                printf("Test root path %s does not exist. \n Please update cfg file and try again. Exiting program \n", obj->test_folder_root);
                exit(-1);
            }

        }
        else if(strcmp(token, "raw_width")==0)
        {
            token = strtok(NULL, s);
            obj->width_in = atoi(token);
            APP_PRINTF(" width_in = [%d]\n", obj->width_in);
        }
        else if(strcmp(token, "raw_height")==0)
        {
            token = strtok(NULL, s);
            obj->height_in = atoi(token);
            APP_PRINTF(" height_in = [%d]\n", obj->height_in);
        }
        else if(strcmp(token, "viss_out_width")==0)
        {
            token = strtok(NULL, s);
            obj->width_out = atoi(token);
            APP_PRINTF(" width_out = [%d]\n", obj->width_out);
        }
        else if(strcmp(token, "viss_out_height")==0)
        {
            token = strtok(NULL, s);
            obj->height_out = atoi(token);
            APP_PRINTF(" height_out = [%d]\n", obj->height_out);
        }
        else if(strcmp(token, "sensor_dcc_id")==0)
        {
            token = strtok(NULL, s);
            obj->cam_dcc_id = atoi(token);
            APP_PRINTF(" sensor_dcc_id = [%d]\n", obj->cam_dcc_id);
        }
        else if(strcmp(token, "sensor_name")==0)
        {
            token = strtok(NULL, s);
            token[strlen(token)-1]=0;
            strcpy(obj->sensor_name, token);
            APP_PRINTF(" sensor_name = [%s]\n", obj->sensor_name);
        }
        else if(strcmp(token, "raw_bpp")==0)
        {
            token = strtok(NULL, s);
            obj->sensor_raw_bpp = atoi(token);
            APP_PRINTF(" sensor_raw_bpp = [%d]\n", obj->sensor_raw_bpp);
        }
        else if(strcmp(token, "wdr_mode")==0)
        {
            token = strtok(NULL, s);
            obj->sensor_wdr_mode = atoi(token);
            APP_PRINTF(" sensor_wdr_mode = [%d]\n", obj->sensor_wdr_mode);
        }
        else if(strcmp(token, "start_seq")==0)
        {
            token = strtok(NULL, s);
            obj->start_seq = atoi(token);
            APP_PRINTF(" start_seq = [%d]\n", obj->start_seq);
        }
        else if(strcmp(token, "num_frames_to_process")==0)
        {
            token = strtok(NULL, s);
            obj->num_frames_to_process = atoi(token);
            APP_PRINTF(" num_frames_to_process = [%d]\n", obj->num_frames_to_process);
        }
        else if(strcmp(token, "ldc_enable")==0)
        {
            token = strtok(NULL, s);
            obj->ldc_enable = atoi(token);
            APP_PRINTF(" ldc_enable = [%d]\n", obj->ldc_enable);
        }
#if defined(VPAC3) || defined(VPAC3L)
        else if(strcmp(token, "cac_enable")==0)
        {
            token = strtok(NULL, s);
            obj->cac_enable = atoi(token);
            APP_PRINTF(" cac_enable = [%d]\n", obj->cac_enable);
        }
#endif
#ifdef VPAC3
        else if(strcmp(token, "vpac3_dual_fcp_enable")==0)
        {
            token = strtok(NULL, s);
            obj->vpac3_dual_fcp_enable = atoi(token);
            APP_PRINTF(" vpac3_dual_fcp_enable = [%d]\n", obj->vpac3_dual_fcp_enable);
        }
#endif
    }

    fclose(fp);

    if(obj->width_in<128)
        obj->width_in = 128;
    if(obj->height_in<128)
        obj->height_in = 128;
    if(obj->width_out<128)
        obj->width_out = 128;
    if(obj->height_out<128)
        obj->height_out = 128;

}

static void x86_app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[])
{
    int i;
    vx_int8 sensor_override = 0xFF;

    vx_bool set_test_mode = vx_false_e;
    app_set_cfg_default(obj);

    if(argc==1)
    {
        app_show_usage(argc, argv);
        exit(0);
    }

    for(i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "--cfg")==0)
        {
            i++;
            if(i>=argc)
            {
                app_show_usage(argc, argv);
            }
            x86_app_parse_cfg_file(obj, argv[i]);
        }
        else
        if(strcmp(argv[i], "--help")==0)
        {
            app_show_usage(argc, argv);
            exit(0);
        }
        else
        if(strcmp(argv[i], "--test")==0)
        {
            set_test_mode = vx_true_e;
        }
        if(strcmp(argv[i], "--sensor")==0)
        {
            // check to see if there is another argument following --sensor
            if (argc > i+1)
            {
                sensor_override = atoi(argv[i+1]);
                // increment i again to avoid this arg
                i++;
            }
        }
    }
    if(set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->num_frames_to_process = 3;
        obj->start_seq = 0;
        if (sensor_override != 0xFF)
        {
            obj->sensor_sel = sensor_override;
        }
    }
}

