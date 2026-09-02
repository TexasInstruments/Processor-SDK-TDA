/*
 *  Copyright (c) Texas Instruments Incorporated 2019
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(LINUX) || defined(QNX)
#include <sys/stat.h>
#endif
#include <assert.h>

#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include "math.h"
#include "tivx_utils_file_rd_wr.h"
#include <limits.h>
#include <TI/tivx_srv.h>
#include "srv_bowl_lut_gen_applib/srv_bowl_lut_gen_applib.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"
#include <TI/tivx_task.h>
#include <render.h>
#include <tivx_utils_graph_perf.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#ifndef PC
#include <app_init.h>
#endif
#include "app_common.h"

#define MAX_ABS_FILENAME          (1024u)
#define MAXPATHLENGTH             (512u)

#define SV_OUT_DISPLAY_HEIGHT   (1080)
#define SV_OUT_DISPLAY_WIDTH    (1080)
#define SV_SUBSAMPLE            (4)
#define SV_XYZLUT3D_SIZE        (SV_OUT_DISPLAY_HEIGHT/SV_SUBSAMPLE) * (SV_OUT_DISPLAY_WIDTH/SV_SUBSAMPLE) *3
#define SV_GPULUT_SIZE          ((2 + SV_OUT_DISPLAY_WIDTH/SV_SUBSAMPLE) * (2 + SV_OUT_DISPLAY_HEIGHT/SV_SUBSAMPLE)) *7


#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

typedef struct {

    /* config options */
    uint32_t is2MP;
    uint32_t num_views;

    int32_t offsetXleft;
    int32_t offsetXright;
    int32_t offsetYfront;
    int32_t offsetYback;

    float camx[MAX_SRV_VIEWS];
    float camy[MAX_SRV_VIEWS];
    float camz[MAX_SRV_VIEWS];
    float targetx[MAX_SRV_VIEWS];
    float targety[MAX_SRV_VIEWS];
    float targetz[MAX_SRV_VIEWS];
    float anglex[MAX_SRV_VIEWS];
    float angley[MAX_SRV_VIEWS];
    float anglez[MAX_SRV_VIEWS];

    /* OpenVX Types */
    vx_context context;
    vx_graph graph1;
    vx_graph graph2;

    /* GPU LUT Applib API objects */
    vx_user_data_object         in_config;
    vx_user_data_object         in_calmat_object;
    vx_user_data_object         in_offset_object;
    vx_user_data_object         in_lens_param_object;
    vx_array                    out_gpulut_array;

    /* GPU LUT Applib params */
    srv_bowl_lut_gen_handle srv_handle;
    srv_bowl_lut_gen_createParams create_params;

    /* OpenGL SRV node objects */
    vx_node srv_node;
    vx_object_array input_array;
    vx_object_array srv_views_array;
    vx_image output_image;
    vx_user_data_object param_obj;
    vx_user_data_object srv_views;

    uint32_t is_interactive;
    uint32_t test_mode;
    tivx_task task;
    uint32_t stop_task;
    uint32_t stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

} AppObj;

AppObj gAppObj;

static void app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[]);
static void app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_run_graph1(AppObj *obj);
static vx_status app_run_graph2(AppObj *obj);
static void app_delete_graph(AppObj *obj);

static char *get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static void read_calmat_file( svCalmat_t *calmat, const char*fileName)
{
    char file[MAXPATHLENGTH];
    uint32_t cnt;
    FILE* f = 0;
    size_t sz;
    uint32_t  read_size;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    printf ("Reading calmat file \n");

    if (!fileName)
    {
        printf("calmat file name not specified\n");
        return;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", test_data_path, fileName);
    if (sz > MAXPATHLENGTH)
    {
        return;
    }

    f = fopen(file, "rb");
    if (!f)
    {
        printf("Cant open calmat file: %s\n", fileName);
        return;
    }

    read_size = fread((int8_t *)&calmat->numCameras,sizeof(uint8_t),4,f);
    if (read_size != 4)
    {
        printf("Incorrect Bytes read from calmat file: %s\n", fileName);
        fclose(f);
        return;
    }

    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        read_size = fread((int8_t *)&calmat->calMatSize[cnt],sizeof(uint8_t),4,f);
        if (read_size != 4)
        {
            printf("Incorrect Bytes read from calmat file: %s\n", fileName);
            fclose(f);
            return;
        }
    }

#if 0
    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        printf("Calmat size for cnt %d = %d \n",cnt,calmat->calMatSize[cnt]);
    }
#endif

    /* Set Pointer ahead by 128 bytes to skip over metadata */
    fseek(f,128,SEEK_SET);

    /* Read calmat per camera */
    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        read_size = fread((int8_t *)&calmat->calMatBuf + 48*cnt,sizeof(uint8_t),calmat->calMatSize[cnt],f);
        if (read_size != calmat->calMatSize[cnt])
        {
            printf("Incorrect Bytes read from calmat file: %s\n", fileName);
            fclose(f);
            return;
        }
    }

#if 0
    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        printf ("For Camera = %d Ref calmat[0] = %d Ref Calmat[11] = %d \n",cnt,calmat->calMatBuf[12*cnt+0],calmat->calMatBuf[12*cnt +11]);
    }
#endif
    printf ("file read completed \n");
    fclose(f);

}

static vx_status app_save_vximage_rgbx_to_bin_file(char *filename, vx_image image)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr1;
    vx_rectangle_t rect;
    vx_map_id map_id1;
    vx_df_image df;
    void *data_ptr1;
    vx_status status;

    status = vxGetStatus((vx_reference)image);
    if(status==VX_SUCCESS)
    {
        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        status = vxMapImagePatch(image,
            &rect,
            0,
            &map_id1,
            &image_addr1,
            &data_ptr1,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(status==VX_SUCCESS)
        {
            FILE *fp = fopen(filename,"wb");

            if(fp!=NULL)
            {
                uint32_t x, y;
                uint8_t *cur_ptr;
                size_t ret = 0;

                for(y=0; y<height; y++)
                {
                    cur_ptr = (uint8_t *)data_ptr1 + image_addr1.stride_y*y;

                    for(x=0; x<width; x++)
                    {
                        /* only write RGB, dont write the A value */
                        ret = fwrite(cur_ptr, 3, 1, fp);
                        if(ret!=1)
                        {
                            printf("# ERROR: Unable to write data to file [%s]\n", filename);
                            break;
                        }
                        cur_ptr += 4; /* skip over a RGBA pixel */
                    }
                    if(ret!=1)
                    {
                        break;
                    }
                }
                fclose(fp);
            }
            else
            {
                printf("# ERROR: Unable to open file for writing [%s]\n", filename);
                status = VX_FAILURE;
            }
            vxUnmapImagePatch(image, map_id1);
        }
    }
    else
    {
        printf("# ERROR: Invalid image specified for writing\n");
    }

    return status;
}

static int load_from_raw_file(vx_image copy_image, int width, int height, const char* filename, int offset)
{
    void* data;
    FILE* fp;
    int dataread;
    int numbytes = 3 * width * height;
    char file[MAXPATHLENGTH];
    vx_rectangle_t rect             = { 0, 0, width, height };
    vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;
    size_t sz;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    if (!filename)
    {
        printf("Raw file name not specified\n");
        return -1;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", test_data_path, filename);
    if (sz > MAXPATHLENGTH)
    {
        return -1;
    }

    fp = fopen(file, "rb");

    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    data = malloc(numbytes);
    if (NULL == data)
    {
        printf("Memory allocation failed!\n");
        fclose(fp);
        return -1;
    }
    memset(data, 0, numbytes);
    fseek(fp, offset, SEEK_CUR);
    dataread = fread(data, 1, numbytes, fp);
    fclose(fp);
    if(dataread != numbytes) {
        printf("Error in file size != width*height\n");
        free(data);
        return -1;
    }

    addr.dim_x = width;
    addr.dim_y = height;
    addr.stride_x = 3;
    addr.stride_y = width*3;

    vxCopyImagePatch(copy_image, &rect, 0, &addr, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    free(data);

    return 0;
}

static int get_offset(const char *filename)
{
    FILE *fp;
    uint8_t header[54], inter;
    uint32_t offset;
    char file[MAX_ABS_FILENAME];
    uint32_t  read_size;
    size_t sz;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    if (!filename)
    {
        printf("Image file name not specified\n");
        return -1;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, MAX_ABS_FILENAME, "%s/%s", test_data_path, filename);
    if (sz > MAX_ABS_FILENAME)
    {
        return -1;
    }

    fp = fopen(file, "rb");

    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    read_size = fread(header, sizeof(uint8_t), 54, fp);
    if (read_size != 54)
    {
        printf("Incorrect Bytes read from file: %s\n", filename);
        fclose(fp);
        return -1;
    }

    inter = *(uint8_t *)(&header[10]);

    offset = (uint32_t)inter;

    fclose(fp);

    return offset;
}

static vx_status load_input_images_2mp(vx_context context, vx_object_array input_array, int in_width, int in_height, int num_cameras)
{
    vx_status status = VX_SUCCESS;
    vx_image copy_image, input_image;
    int i, offset;
    char filename[MAXPATHLENGTH];

    for (i = 0; i < num_cameras; i++)
    {
        copy_image = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_RGB);

        input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)input_array, i);

        if (i == 0)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_2mpix/FRONT_0.bmp");
        }
        else if (i == 1)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_2mpix/RIGHT_0.bmp");
        }
        else if (i == 2)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_2mpix/BACK_0.bmp");
        }
        else if (i == 3)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_2mpix/LEFT_0.bmp");
        }

        offset = get_offset(filename);
        status = load_from_raw_file(copy_image, in_width, in_height, filename, offset);
        if (VX_SUCCESS != status)
        {
            return status;
        }

        vxuColorConvert(context, copy_image, input_image);

        vxReleaseImage(&input_image);
        vxReleaseImage(&copy_image);
    }
    return status;
}

static vx_status load_input_images(vx_context context, vx_object_array input_array, int in_width, int in_height, int num_cameras)
{
    vx_status status = VX_SUCCESS;
    vx_image copy_image, input_image;
    int i, offset;
    char filename[MAXPATHLENGTH];

    for (i = 0; i < num_cameras; i++)
    {
        copy_image = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_RGB);

        input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)input_array, i);

        if (i == 0)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_1mpix/FRONT_0.bmp");
        }
        else if (i == 1)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_1mpix/RIGHT_0.bmp");
        }
        else if (i == 2)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_1mpix/BACK_0.bmp");
        }
        else if (i == 3)
        {
            snprintf(filename, MAXPATHLENGTH, "%s", "psdkra/srv/applibTC_1mpix/LEFT_0.bmp");
        }

        offset = get_offset(filename);
        if (offset < 0)
        {
            status = offset;
            return status;
        }

        status = load_from_raw_file(copy_image, in_width, in_height, filename, offset);
        if (VX_SUCCESS != status)
        {
            return status;
        }

        vxuColorConvert(context, copy_image, input_image);

        vxReleaseImage(&input_image);
        vxReleaseImage(&copy_image);
    }
    return status;
}

static void read_lut_file(ldc_lensParameters *ldcParams, const char*fileName)
{
    char file[MAXPATHLENGTH];
    uint32_t  read_size;
    FILE* f = 0;
    size_t sz;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    if (!fileName)
    {
        printf("Image file name not specified\n");
        return;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", test_data_path, fileName);
    if (sz > MAXPATHLENGTH)
    {
        return;
    }

    f = fopen(file, "rb");
    if (!f)
    {
        printf("Can't open LUT file: %s\n", fileName);
        return;
    }

    read_size = fread((uint8_t *)ldcParams,sizeof(uint8_t),sizeof(ldc_lensParameters),f);
    if (read_size != sizeof(ldc_lensParameters))
    {
        printf("Incorrect Bytes read from  LUT file: %s\n", fileName);
        fclose(f);
        return;
    }

    fclose(f);

}

static LDC_status LDC_Init(LensDistortionCorrection* ldc,
                        dtype distCenterX, dtype distCenterY, dtype distFocalLength,
                        dtype *lut_d2u, vx_int32 lut_d2u_length, dtype lut_d2u_step,
                        dtype *lut_u2d, vx_int32 lut_u2d_length, dtype lut_u2d_step)
{
        /*FLOATING TYPE*/
        /*distortion center*/
        ldc->distCenterX = distCenterX;
        ldc->distCenterY = distCenterY;
        ldc->distFocalLength = distFocalLength;
        ldc->distFocalLengthInv = 1/ldc->distFocalLength;
        /*ldc look-up table parameters*/
        ldc->lut_d2u_indMax = lut_d2u_length-1;
        ldc->lut_d2u_step = lut_d2u_step;
        ldc->lut_u2d_indMax = lut_u2d_length - 1;
        ldc->lut_u2d_step = lut_u2d_step;

        ldc->lut_d2u_stepInv = 1/ldc->lut_d2u_step;
        ldc->lut_u2d_stepInv = 1/ldc->lut_u2d_step;


        /*ldc look-up table pointers*/
        memcpy (ldc->lut_d2u, (uint8_t *)lut_d2u,(sizeof(dtype)*LDC_D2U_TABLE_MAX_LENGTH) );
        memcpy (ldc->lut_u2d, (uint8_t *)lut_u2d,(sizeof(dtype)*LDC_U2D_TABLE_MAX_LENGTH) );

        return LDC_STATUS_OK;
}


static void app_show_usage(int argc, char* argv[])
{
    printf("\n");
    printf(" SRV Demo - (c) Texas Instruments 2019\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static void app_set_cfg_default(AppObj *obj)
{
    obj->is2MP = 0;
    obj->is_interactive = 0;
    obj->test_mode = 0;
}

static void app_parse_cfg_file(AppObj *obj, char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    char line_str[1024];
    char *token;
    uint32_t camx_idx = 0, camy_idx = 0, camz_idx = 0;
    uint32_t targetx_idx = 0, targety_idx = 0, targetz_idx = 0;
    uint32_t anglex_idx = 0, angley_idx = 0, anglez_idx = 0;

    if(fp==NULL)
    {
        printf("# ERROR: Unable to open config file [%s]\n", cfg_file_name);
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

        if (NULL != token)
        {
            if(strcmp(token, "is2MP")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->is2MP = atoi(token);
                }
            }
            else if(strcmp(token, "offsetXleft")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetXleft = atoi(token);
                }
            }
            else if(strcmp(token, "offsetXright")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetXright = atoi(token);
                }
            }
            else if(strcmp(token, "offsetYfront")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetYfront = atoi(token);
                }
            }
            else if(strcmp(token, "offsetYback")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetYback = atoi(token);
                }
            }
            else if(strcmp(token, "num_views")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->num_views = atoi(token);
                    if (obj->num_views > MAX_SRV_VIEWS)
                    {
                        printf("Config file num_views argument exceeds MAX_SRV_VIEWS = %d\n", MAX_SRV_VIEWS);
                        exit(0);
                    }
                }
            }
            else if(strcmp(token, "camx")==0)
            {
                if (obj->num_views < (camx_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->camx[camx_idx] = atof(token);
                    camx_idx++;
                }
            }
            else if(strcmp(token, "camy")==0)
            {
                if (obj->num_views < (camy_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->camy[camy_idx] = atof(token);
                    camy_idx++;
                }
            }
            else if(strcmp(token, "camz")==0)
            {
                if (obj->num_views < (camz_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->camz[camz_idx] = atof(token);
                    camz_idx++;
                }
            }
            else if(strcmp(token, "targetx")==0)
            {
                if (obj->num_views < (targetx_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->targetx[targetx_idx] = atof(token);
                    targetx_idx++;
                }
            }
            else if(strcmp(token, "targety")==0)
            {
                if (obj->num_views < (targety_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->targety[targety_idx] = atof(token);
                    targety_idx++;
                }
            }
            else if(strcmp(token, "targetz")==0)
            {
                if (obj->num_views < (targetz_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->targetz[targetz_idx] = atof(token);
                    targetz_idx++;
                }
            }
            else if(strcmp(token, "anglex")==0)
            {
                if (obj->num_views < (anglex_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->anglex[anglex_idx] = atof(token);
                    anglex_idx++;
                }
            }
            else if(strcmp(token, "angley")==0)
            {
                if (obj->num_views < (angley_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->angley[angley_idx] = atof(token);
                    angley_idx++;
                }
            }
            else if(strcmp(token, "anglez")==0)
            {
                if (obj->num_views < (anglez_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->anglez[anglez_idx] = atof(token);
                    anglez_idx++;
                }
            }
            else
            if(strcmp(token, "is_interactive")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    obj->is_interactive = atoi(token);
                    if(obj->is_interactive > 1)
                        obj->is_interactive = 1;
                }
            }
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[])
{
    int i;
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
            app_parse_cfg_file(obj, argv[i]);
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
    }
    if(set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->is_interactive = 0;
    }
}

static void app_init(AppObj *obj)
{
    appInit();

    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    tivxSrvLoadKernels(obj->context);
}

static int app_create_graph(AppObj *obj)
{
    vx_uint32 i;
    vx_uint32 is2MP = 0;
    vx_uint32 in_width, in_height, out_width, out_height, num_cameras;
    srv_coords_t local_srv_coords;
    tivx_srv_params_t params;
    vx_image img_exemplar;
    vx_status status;

    /* Applib data */
    svGpuLutGen_t               in_params;
    svACCalmatStruct_t          in_calmat;
    svGeometric_t               in_offset;
    ldc_lensParameters          lens_params;
    svLdcLut_t                  in_lens_params;
    svCalmat_t                  calmat_file;

    is2MP = obj->is2MP;
    out_width = 1920;
    out_height = 1080;
    num_cameras = 4;

    if (1 == is2MP)
    {
        read_calmat_file(&calmat_file, "psdkra/srv/applibTC_2mpix/CALMAT.BIN");
        in_width = 1920;
        in_height = 1080;
    }
    else
    {
        read_calmat_file(&calmat_file, "psdkra/srv/applibTC_1mpix/CALMAT.BIN");
        in_width = 1280;
        in_height = 720;
    }

    obj->graph1 = vxCreateGraph(obj->context);

    obj->graph2 = vxCreateGraph(obj->context);

    memset(&params, 0, sizeof(tivx_srv_params_t));
    params.cam_bpp = 24;

    memset(&params, 0, sizeof(tivx_srv_params_t));
    params.cam_bpp = 24;

    /* Populating data objects */
    memset(&in_params, 0, sizeof(svGpuLutGen_t));
    obj->in_config = vxCreateUserDataObject(obj->context, "svGpuLutGen_t",
                                       sizeof(svGpuLutGen_t), NULL);

    status = vxGetStatus((vx_reference) obj->in_config);

    memset(&in_calmat, 0, sizeof(svACCalmatStruct_t));
    obj->in_calmat_object = vxCreateUserDataObject(obj->context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) obj->in_calmat_object);
    }

    memset(&in_offset, 0, sizeof(svGeometric_t));
    obj->in_offset_object = vxCreateUserDataObject(obj->context, "svGeometric_t",
                                       sizeof(svGeometric_t), NULL);

    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) obj->in_offset_object);
    }
    memset(&in_lens_params, 0, sizeof(svLdcLut_t));
    obj->in_lens_param_object = vxCreateUserDataObject(obj->context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (obj->in_lens_param_object));
    }
    obj->out_gpulut_array = vxCreateArray(obj->context, VX_TYPE_UINT16, SV_GPULUT_SIZE);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (obj->out_gpulut_array));
    }

    /* Creating objects for OpenGL node */
    obj->output_image = vxCreateImage(obj->context, out_width, out_height, VX_DF_IMAGE_RGBX);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (obj->output_image));
    }

    obj->param_obj = vxCreateUserDataObject(obj->context, "tivx_srv_params_t", sizeof(tivx_srv_params_t), &params);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (obj->param_obj));
    }

    img_exemplar = vxCreateImage(obj->context, in_width, in_height, VX_DF_IMAGE_NV12);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (img_exemplar));
    }

    obj->srv_views = vxCreateUserDataObject(obj->context, "srv_coords_t", sizeof(srv_coords_t), &local_srv_coords);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (obj->srv_views));
    }

    printf("obj->num_views = %d\n", obj->num_views);
    obj->srv_views_array = vxCreateObjectArray(obj->context, (vx_reference)obj->srv_views, obj->num_views);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) (obj->srv_views_array));
    }

    if(status == VX_SUCCESS)
    {
        status = vxReleaseUserDataObject(&obj->srv_views);
    }
    for (i = 0; i < obj->num_views; i++)
    {
        local_srv_coords.camx =     obj->camx[i];
        local_srv_coords.camy =     obj->camy[i];
        local_srv_coords.camz =     obj->camz[i];
        local_srv_coords.targetx =  obj->targetx[i];
        local_srv_coords.targety =  obj->targety[i];
        local_srv_coords.targetz =  obj->targetz[i];
        local_srv_coords.anglex =   obj->anglex[i];
        local_srv_coords.angley =   obj->angley[i];
        local_srv_coords.anglez =   obj->anglez[i];

        obj->srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)obj->srv_views_array, i);

        if(status == VX_SUCCESS)
        {
            status = vxCopyUserDataObject(obj->srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
        vxReleaseUserDataObject(&obj->srv_views);
    }

    if(status == VX_SUCCESS)
    {
        obj->input_array = vxCreateObjectArray(obj->context, (vx_reference)img_exemplar, num_cameras);
        status = vxGetStatus((vx_reference) obj->input_array);
    }

    vxReleaseImage(&img_exemplar);

    /* Populate input data */
    in_params.SVInCamFrmHeight   = in_height;
    in_params.SVInCamFrmWidth    = in_width;
    in_params.SVOutDisplayHeight = SV_OUT_DISPLAY_HEIGHT;
    in_params.SVOutDisplayWidth  = SV_OUT_DISPLAY_WIDTH;
    in_params.numCameras         = num_cameras;
    in_params.subsampleratio     = SV_SUBSAMPLE;
    in_params.useWideBowl        = 1;

    in_offset.offsetXleft     = obj->offsetXleft;
    in_offset.offsetXright    = obj->offsetXright;
    in_offset.offsetYfront    = obj->offsetYfront;
    in_offset.offsetYback     = obj->offsetYback;

    if((1 == is2MP) && (status == VX_SUCCESS))
    {
        status = load_input_images_2mp(obj->context, obj->input_array, in_width, in_height, num_cameras);
    }
    else
    if(status == VX_SUCCESS)
    {
        status = load_input_images(obj->context, obj->input_array, in_width, in_height, num_cameras);
    }

    for (int idx=0;idx<48;idx++)
    {
        in_calmat.outcalmat[idx] = calmat_file.calMatBuf[idx];
    }

    /* Read Lens file */
    if (1 == is2MP)
    {
        read_lut_file(&lens_params,"psdkra/srv/applibTC_2mpix/LENS.BIN" );
    }
    else
    {
        read_lut_file(&lens_params,"psdkra/srv/applibTC_1mpix/LENS.BIN" );
    }

    /* Initialize all the 4 channel ldc luts within in_params    */

    for (int i=0;i<4; i++) {
        LDC_Init(&in_lens_params.ldc[i],
             lens_params.ldcLUT_distortionCenters[i*2],
             lens_params.ldcLUT_distortionCenters[i*2+1],
             lens_params.ldcLUT_focalLength,
             lens_params.ldcLUT_D2U_table,
             lens_params.ldcLUT_D2U_length,
             lens_params.ldcLUT_D2U_step,
             lens_params.ldcLUT_U2D_table,
             lens_params.ldcLUT_U2D_length,
             lens_params.ldcLUT_U2D_step);
    }

    /* Copying to user data objects */
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_config, 0, sizeof(svGpuLutGen_t), &in_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_calmat_object, 0, sizeof(svACCalmatStruct_t), &in_calmat, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_offset_object, 0, sizeof(svGeometric_t), &in_offset, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_lens_param_object, 0, sizeof(svLdcLut_t), &in_lens_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    /* Creating applib */
    obj->create_params.vxContext = obj->context;
    obj->create_params.vxGraph   = obj->graph1;

    /* Data object */
    obj->create_params.in_config    = obj->in_config;
    obj->create_params.in_calmat    = obj->in_calmat_object;
    obj->create_params.in_offset    = obj->in_offset_object;
    obj->create_params.in_ldclut    = obj->in_lens_param_object;
    obj->create_params.out_gpulut3d = obj->out_gpulut_array;

    if(status == VX_SUCCESS)
    {
        obj->srv_handle = srv_bowl_lut_gen_create(&obj->create_params);

        obj->srv_node = tivxGlSrvNode(obj->graph2, obj->param_obj, obj->input_array, obj->srv_views_array, obj->out_gpulut_array, obj->output_image);
        status = vxSetNodeTarget(obj->srv_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->srv_node, "OpenGL_SRV_Node");
    }
    APP_PRINTF("app_srv_fileio: Verifying graph 1  ... .\n");

    if(status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graph1);
        APP_PRINTF("app_srv_fileio: Verifying graph 1... Done\n");
    }

    if(status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graph2);
        APP_PRINTF("app_srv_fileio: Verifying graph 2  ... .\n");
    }

    APP_PRINTF("app_srv_fileio: Verifying graph 2... Done\n");
    return status;
}

static vx_status app_run_graph1(AppObj *obj)
{
    APP_PRINTF("app_srv_fileio: Running graph 1 ...\n");
    vx_status status = vxProcessGraph(obj->graph1);
    APP_PRINTF("app_srv_fileio: Running graph 1 ... Done\n");
    return status;
}

static vx_status app_run_graph2(AppObj *obj)
{
    APP_PRINTF("app_srv_fileio: Running graph 2 ...\n");
    vx_status status = vxProcessGraph(obj->graph2);
    APP_PRINTF("app_srv_fileio: Running graph 2 ... Done\n");
    return status;
}

static void app_delete_graph(AppObj *obj)
{
    /* Deleting applib */
    srv_bowl_lut_gen_delete(obj->srv_handle);

    vxReleaseUserDataObject(&obj->param_obj);
    vxReleaseObjectArray(&obj->input_array);
    vxReleaseObjectArray(&obj->srv_views_array);
    vxReleaseImage(&obj->output_image);

    vxReleaseUserDataObject(&obj->in_config);
    vxReleaseUserDataObject(&obj->in_calmat_object);
    vxReleaseUserDataObject(&obj->in_offset_object);
    vxReleaseUserDataObject(&obj->in_lens_param_object);
    vxReleaseArray(&obj->out_gpulut_array);

    vxReleaseNode(&obj->srv_node);
    vxReleaseGraph(&obj->graph1);
    vxReleaseGraph(&obj->graph2);
    return;
}

static void app_deinit(AppObj *obj)
{
    tivxSrvUnLoadKernels(obj->context);
    vxReleaseContext(&obj->context);
    appDeInit();
}

int main(int argc, char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;
    char output_file_path[MAXPATHLENGTH];
    size_t sz;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(output_file_path, MAXPATHLENGTH, "%s/output/srv_output_file.bin", test_data_path);
    if (sz > MAXPATHLENGTH)
    {
        return -1;
    }

    app_parse_cmd_line_args(obj, argc, argv);
    app_init(obj);
    status = app_create_graph(obj);
    if(VX_SUCCESS == status)
    {
        if(status == VX_SUCCESS)
        {
            status = app_run_graph1(obj);
        }
        if(status == VX_SUCCESS)
        {
            status = app_run_graph2(obj);
        }
        if(status == VX_SUCCESS)
        {
            status = app_save_vximage_rgbx_to_bin_file(output_file_path, (vx_image)obj->output_image);
        }
    }
    app_delete_graph(obj);
    app_deinit(obj);

    return status;
}
