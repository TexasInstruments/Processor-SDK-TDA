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

#include <TI/tivx.h>
#include <tivx_utils_file_rd_wr.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "app_c7x_kernel.h"
#include "app_test.h"

#define APP_MAX_FILE_PATH           (256u)
#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

typedef struct {

    /* config options */
    char input_file_1[APP_MAX_FILE_PATH];
    char input_file_2[APP_MAX_FILE_PATH];
    char output_file[APP_MAX_FILE_PATH];
    uint32_t width;
    uint32_t height;
    uint32_t test_mode;

    /* OpenVX references */
    vx_context context;
    vx_graph graph;
    vx_node  node;
    vx_image input_img1;
    vx_image input_img2;
    vx_image output_img;

} AppObj;

AppObj gAppObj;

static void app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[]);
static vx_status app_init(AppObj *obj);
static vx_status app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

int app_c7x_kernel_main(int argc, char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;

    app_parse_cmd_line_args(obj, argc, argv);
    app_init(obj);
    if(status == VX_SUCCESS)
    {
        status = app_create_graph(obj);
    }
    if(status == VX_SUCCESS)
    {
        status = app_run_graph(obj);
    }
    app_delete_graph(obj);
    if(status == VX_SUCCESS)
    {
        status = app_deinit(obj);
    }

    if(obj->test_mode == 1)
    {
        if((test_result == vx_true_e) && (status == VX_SUCCESS))
        {
            printf("\n\nTEST PASSED\n\n");
        }
        else
        {
            printf("\n\nTEST FAILED\n\n");
            status = (status == VX_SUCCESS) ? VX_FAILURE : status;
            print_new_checksum_structs();
        }
    }

    return status;
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference)obj->context);

    if(status == VX_SUCCESS)
    {
        tivxImgProcLoadKernels(obj->context);
    }

    return status;
}

static vx_status app_deinit(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    tivxImgProcUnLoadKernels(obj->context);

    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&obj->context);
    }
    return status;
}

/*
 * Graph,
 *
 * input_img_1 -> app_node_img_add -----> output_img
 *                  ^
 *                  |
 * input_img_2 -----+
 */
static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->graph = vxCreateGraph(obj->context);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference)obj->graph);
    }

    if(status == VX_SUCCESS)
    {
        obj->input_img1 = vxCreateImage(obj->context, obj->width, obj->height, VX_DF_IMAGE_U8);
        status = vxSetReferenceName((vx_reference)obj->input_img1, "Input1ImageU8");
    }
    if(status == VX_SUCCESS)
    {
        obj->input_img2 = vxCreateImage(obj->context, obj->width, obj->height, VX_DF_IMAGE_U8);
        status = vxGetStatus((vx_reference)obj->input_img2);
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->input_img2, "Input2ImageU8");
    }
    if(status == VX_SUCCESS)
    {
        obj->output_img = vxCreateImage(obj->context, obj->width, obj->height, VX_DF_IMAGE_U8);
        status = vxGetStatus((vx_reference)obj->output_img);
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->output_img, "OutputImageU8");
    }
    if(status == VX_SUCCESS)
    {
        obj->node = tivxAddImgNode(obj->graph,
                                   obj->input_img1,
                                   obj->input_img2,
                                   obj->output_img);
        status = vxGetStatus((vx_reference)obj->node);
    }

    if(status == VX_SUCCESS)
    {
        status = vxSetNodeTarget(obj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->node, "C7xImageAddition");
    }
    if(status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graph);
    }
    if(status == VX_SUCCESS)
    {
        /* Do not set status here because this step is not critical */
        tivxExportGraphToDot(obj->graph,".", "vx_app_c7x_kernel");
    }

    return status;
}

static void app_delete_graph(AppObj *obj)
{
    vxReleaseNode(&obj->node);
    vxReleaseGraph(&obj->graph);
    vxReleaseImage(&obj->input_img1);
    vxReleaseImage(&obj->input_img2);
    vxReleaseImage(&obj->output_img);
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 actual_checksum = 0;

    printf(" Loading [%s] ...\n", obj->input_file_1);
    status = tivx_utils_load_vximage_from_bmpfile(obj->input_img1, obj->input_file_1, vx_true_e);

    if(status == VX_SUCCESS)
    {
        printf(" Loading [%s] ...\n", obj->input_file_2);
        status = tivx_utils_load_vximage_from_bmpfile(obj->input_img2, obj->input_file_2, vx_true_e);
    }

    printf(" Running graph ...\n");
    #if 1
    if(status == VX_SUCCESS)
    {
        status = vxScheduleGraph(obj->graph);
    }
    if(status == VX_SUCCESS)
    {
        status = vxWaitGraph(obj->graph);
    }
    #endif

    if((obj->test_mode == 1) && (status == VX_SUCCESS))
    {
        if (app_test_check_image(obj->output_img, checksums_expected[0][0], &actual_checksum) == vx_false_e)
        {
            test_result = vx_false_e;
            status = VX_FAILURE;
        }
    }
    /* in case test fails and needs to change */
    populate_gatherer(0, 0, actual_checksum);

    if(status == VX_SUCCESS)
    {
        printf(" Saving [%s] ...\n", obj->output_file);
        status = tivx_utils_save_vximage_to_bmpfile(obj->output_file, obj->output_img);
        printf(" Done !!!\n");
    }

    return status;
}

static void app_show_usage(int argc, char* argv[])
{
    printf("\n");
    printf(" C7x Kernel Demo - (c) Texas Instruments 2018\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static void app_set_cfg_default(AppObj *obj)
{
    snprintf(obj->input_file_1,APP_MAX_FILE_PATH, "./img_1.bmp");
    snprintf(obj->input_file_2,APP_MAX_FILE_PATH, "./img_2.bmp");
    snprintf(obj->output_file,APP_MAX_FILE_PATH, "./out_img.bmp");
    obj->width = 640;
    obj->height = 480;
    obj->test_mode = 0;
}

static void app_parse_cfg_file(AppObj *obj, char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    char line_str[1024];
    char *token;

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
            if(strcmp(token, "input_file_1")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->input_file_1, token);
                }
            }
            else
            if(strcmp(token, "input_file_2")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->input_file_2, token);
                }
            }
            else
            if(strcmp(token, "output_file")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->output_file, token);
                }
            }
            else
            if(strcmp(token, "width")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->width = atoi(token);
                }
            }
            else
            if(strcmp(token, "height")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->height = atoi(token);
                }
            }
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[])
{
    int i;

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
            obj->test_mode = 1;
        }
    }
}
