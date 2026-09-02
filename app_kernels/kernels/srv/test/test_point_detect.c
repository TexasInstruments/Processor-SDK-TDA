/*

 * Copyright (c) 2012-2026 The Khronos Group Inc.
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


#include "test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

#include <TI/tivx_srv.h>
#include "lens_distortion_correction.h"
#include "tivx_utils_file_rd_wr.h"
//#define DEBUG_TEST_1
//#define DEBUG_TEST_2
//#define DEBUG_PRINT

TESTCASE(tivxSrvPointDetect, CT_VXContext, ct_setup_vx_context, 0)

static LDC_status LDC_Init(LensDistortionCorrection* ldc,
                        dtype distCenterX, dtype distCenterY, dtype distFocalLength,
                        dtype *lut_d2u, vx_int32 lut_d2u_length, dtype lut_d2u_step,
                        dtype *lut_u2d, vx_int32 lut_u2d_length, dtype lut_u2d_step)
{
#if LDC_LIB_DATA_TYPE!=0 && LDC_LIB_DATA_TYPE!=1
        "LDC_LIB_DATA_TYPE must be 0 (float) or 1 (double) in lens_distortion_correction.h"
#endif
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

static void read_lut_file(ldc_lensParameters *ldcParams, const char*fileName)
{ 
    char file[MAXPATHLENGTH];  
    uint32_t  read_size;
    FILE* f = 0;
    size_t sz;

    #ifdef DEBUG_PRINT
    printf ("Reading LUT file \n");
    #endif

    if (!fileName)
    {
        printf("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        printf("Can't open LUT file: %s\n", fileName);
        return;
    }

    read_size = fread((uint8_t *)ldcParams,sizeof(uint8_t),sizeof(ldc_lensParameters),f);
    if (read_size != sizeof(ldc_lensParameters))
    {
        CT_ADD_FAILURE("Incorrect Bytes read from  LUT file: %s\n", fileName);
        fclose(f);
        return;
    }

    #ifdef DEBUG_PRINT
    printf ("file read completed \n");
    #endif
    fclose(f);

}

static void ct_read_image2(vx_image image, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if (NULL != buf)
        {
            if( fread(buf, 1, sz, f) == sz )
            {
                vx_uint32 width, height;
                vx_imagepatch_addressing_t image_addr;
                vx_rectangle_t rect;
                vx_map_id map_id;
                vx_df_image df;
                void *data_ptr;
                vx_uint32 num_bytes = 1;

                vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
                vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
                vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
                
                if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
                {
                    num_bytes = 2;
                }
                else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
                {
                    num_bytes = 4;
                }

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                vxMapImagePatch(image,
                    &rect,
                    0,
                    &map_id,
                    &image_addr,
                    &data_ptr,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    VX_NOGAP_X
                    );

                if(file_byte_pack == num_bytes)
                {
                    memcpy(data_ptr, buf, width*height*num_bytes);
                }
                else if((file_byte_pack == 2) && (num_bytes == 1))
                {
                    int i;
                    uint8_t *dst = data_ptr;
                    uint16_t *src = (uint16_t*)buf;
                    for(i = 0; i < width*height; i++)
                    {
                        dst[i] = src[i];
                    }
                }
                vxUnmapImagePatch(image, map_id);
            }
        }
        else
        {
            fclose(f);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}


static void ct_write_image2(vx_image image, const char* fileName)
{
    FILE* f = 0;
    size_t sz;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "wb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }
    else
    {
        vx_uint32 width, height;
        vx_imagepatch_addressing_t image_addr;
        vx_rectangle_t rect;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr;
        vx_uint32 num_bytes = 1;

        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
        
        if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
        {
            num_bytes = 2;
        }
        else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
        {
            num_bytes = 4;
        }

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        vxMapImagePatch(image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        fwrite(data_ptr, 1, width*height*num_bytes, f);

        vxUnmapImagePatch(image, map_id);
    }

    fclose(f);
}



static void point_detect_corners_check(svACDetectStructFinalCorner_t ref_corners, svACDetectStructFinalCorner_t out_corners)
/* ref corners are downshifted by 4, out corners need to be shifted down */
{
    int32_t   ref_corner;
    int32_t   out_corner;
    int32_t   i,j;

    for (i=0;i<2;i++) 
    {
        for (j=0;j<8;j++) {
            ref_corner = ref_corners.finalCorners[i][j];
            out_corner = out_corners.finalCorners[i][j]>>4;
            ASSERT(ref_corner == out_corner);
        }
    }
    ASSERT(ref_corners.numFPDetected == out_corners.numFPDetected);

}

static void point_detect_binarize_check(vx_image img_out, vx_image img_ref)
{
    CT_Image ct_output_bw = NULL, ct_ref_bw = NULL;

    ASSERT(img_out && img_ref);

    //Convert vx to CT
    ASSERT_NO_FAILURE(ct_output_bw = ct_image_from_vx_image(img_out));
    ASSERT_NO_FAILURE(ct_ref_bw = ct_image_from_vx_image(img_ref));
    
    EXPECT_EQ_CTIMAGE(ct_ref_bw, ct_output_bw); 
}



typedef struct {
    const char* testName;
    int dummy_;
    int width, height;
} Arg;



// scenario-1
TEST(tivxSrvPointDetect, testSingleImage)
{
    vx_context context = context_->vx_context_;
    vx_image vx_input_y = 0, vx_output_bw = 0, vx_ref_bw = 0;
    vx_graph graph = 0;
    vx_node node1 = 0;
    vx_perf_t perf_node1,  perf_graph;
    vx_uint32 width,height, i;

    svPointDetect_t in_params;
    svLdcLut_t      in_lens_params;
    ldc_lensParameters lens_params; /* This is only for reading from file */
    vx_user_data_object in_param_object;
    vx_user_data_object in_lens_param_object;

    svACDetectStructFinalCorner_t out_params;
    vx_user_data_object           out_param_object;

    svACDetectStructFinalCorner_t ref_params; /* Pre-calculated ref output values */

    ref_params.finalCorners[0][0] = 263;
    ref_params.finalCorners[0][1] = 219;
    ref_params.finalCorners[0][2] = 234;
    ref_params.finalCorners[0][3] = 330;
    ref_params.finalCorners[0][4] = 475;
    ref_params.finalCorners[0][5] = 195;
    ref_params.finalCorners[0][6] = 432;
    ref_params.finalCorners[0][7] = 125;

    ref_params.finalCorners[1][0] = 232;
    ref_params.finalCorners[1][1] = 944;
    ref_params.finalCorners[1][2] = 264;
    ref_params.finalCorners[1][3] = 1066;
    ref_params.finalCorners[1][4] = 446;
    ref_params.finalCorners[1][5] = 1163;
    ref_params.finalCorners[1][6] = 492;
    ref_params.finalCorners[1][7] = 1081;
    ref_params.numFPDetected = 8;



    #ifdef DEBUG_PRINT
    printf("Load Kernel                    \n");
    #endif
    tivxSrvLoadKernels(context); 

    // Read Lens file
    read_lut_file(&lens_params,"psdkra/srv/LENS.BIN" );  
    #ifdef DEBUG_TEST_1
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",lens_params.ldcLUT_D2U_table[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",lens_params.ldcLUT_U2D_table[1023]);
    #endif

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(vx_input_y = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(vxQueryImage(vx_input_y, VX_IMAGE_WIDTH, &width, sizeof(width)));
    VX_CALL(vxQueryImage(vx_input_y, VX_IMAGE_HEIGHT, &height, sizeof(height)));
    ct_read_image2(vx_input_y, "psdkra/srv/testLeft_1.yuv", 1);

    ASSERT_VX_OBJECT(vx_output_bw = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(vx_ref_bw = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    memset(&in_params, 0, sizeof(svPointDetect_t));
    ASSERT_VX_OBJECT(in_param_object = vxCreateUserDataObject(context, "svPointDetect_t",
                                       sizeof(svPointDetect_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&in_lens_params, 0, sizeof(svPointDetect_t));
    ASSERT_VX_OBJECT(in_lens_param_object = vxCreateUserDataObject(context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&out_params, 0, sizeof(svACDetectStructFinalCorner_t));
    ASSERT_VX_OBJECT(out_param_object = vxCreateUserDataObject(context, "svACDetectStructFinalCorner_t",
                                       sizeof(svACDetectStructFinalCorner_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);


    in_params.thresholdMode = 0;
    in_params.windowMode = 0;
    in_params.Ransac = 0;
    in_params.SVROIWidth = 1280;
    in_params.SVROIHeight = 720;
    in_params.binarizeOffset = 75;
    in_params.borderOffset = 50;
    in_params.smallestCenter = 2;
    in_params.largestCenter = 50;
    in_params.maxWinWidth = 180;
    in_params.maxWinHeight = 180;
    in_params.maxBandLen = 160;
    in_params.minBandLen = 2;
    in_params.minSampleInCluster = 4;
    in_params.firstROITop = 150;
    in_params.firstROIBottom = 670;
    in_params.firstROILeft = 100;
    in_params.firstROIRight = 600;
    in_params.secondROITop = 150;
    in_params.secondROIBottom = 670;
    in_params.secondROILeft = 700;
    in_params.secondROIRight = 1200;
    in_params.camera_id     = 3;

    
    /* Initialize all the 4 channel ldc luts within in_params    */ 
    /* Only one of these would be used, however the actual graph */
    /* will mimic this behavior                                  */

    for (i=0;i<4; i++) {
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

    #ifdef DEBUG_TEST_1
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",in_params.ldc[3].lut_d2u[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",in_params.ldc[3].lut_u2d[1023]);   
    #endif

    VX_CALL(vxCopyUserDataObject(in_param_object, 0, sizeof(svPointDetect_t),
                                 &in_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyUserDataObject(in_lens_param_object, 0, sizeof(svLdcLut_t),
                                 &in_lens_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));


    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    #ifdef DEBUG_PRINT
    printf("Defining Nodes \n");
    #endif
    ASSERT_VX_OBJECT(node1 = tivxPointDetectNode(graph,  in_param_object,in_lens_param_object,vx_input_y, out_param_object,vx_output_bw), VX_TYPE_NODE);


    VX_CALL(vxVerifyGraph(graph)); // Verify the sanity of the graph (Validate etc)
    #ifdef DEBUG_PRINT
    printf("Call Graph \n");
    #endif
    VX_CALL(vxProcessGraph(graph)); // Running the graph a single time
    #ifdef DEBUG_PRINT
    printf("Graph Call completed \n");
    #endif

    // Performance data
    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));



    ct_read_image2(vx_ref_bw, "psdkra/srv/testBWImg1_ref.yuv", 1);

    /* Copy output object using VX_READ_ONLY */
    VX_CALL(vxCopyUserDataObject(out_param_object, 0, sizeof(svACDetectStructFinalCorner_t), &out_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));   


    #ifdef DEBUG_TEST_1
    printf("Test bench Final Corners 1 = %d %d %d %d %d %d %d %d\n",out_params.finalCorners[0][0]>>4,
    out_params.finalCorners[0][1]>>4,out_params.finalCorners[0][2]>>4, out_params.finalCorners[0][3]>>4,
    out_params.finalCorners[0][4]>>4,out_params.finalCorners[0][5]>>4, out_params.finalCorners[0][6]>>4,
    out_params.finalCorners[0][7]>>4);

    printf("Test bench Final Corners 2 = %d %d %d %d %d %d %d %d\n",out_params.finalCorners[1][0]>>4,
    out_params.finalCorners[1][1]>>4,out_params.finalCorners[1][2]>>4, out_params.finalCorners[1][3]>>4,
    out_params.finalCorners[1][4]>>4,out_params.finalCorners[1][5]>>4, out_params.finalCorners[1][6]>>4,
    out_params.finalCorners[1][7]>>4);
    printf("numFPDetected = %d \n",out_params.numFPDetected);

    #endif


    ASSERT_NO_FAILURE(point_detect_binarize_check(vx_output_bw, vx_ref_bw)); 
    ASSERT_NO_FAILURE(point_detect_corners_check ( ref_params , out_params));


    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&vx_input_y));
    VX_CALL(vxReleaseImage(&vx_output_bw));
    VX_CALL(vxReleaseImage(&vx_ref_bw));
    VX_CALL(vxReleaseUserDataObject(&in_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_lens_param_object));
    VX_CALL(vxReleaseUserDataObject(&out_param_object));


    ASSERT(vx_input_y == 0);
    ASSERT(vx_output_bw == 0);
    ASSERT(vx_ref_bw == 0);
    ASSERT(in_param_object == 0);
    ASSERT(in_lens_param_object == 0);
    ASSERT(out_param_object == 0);

    //printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    //printPerformance(perf_graph, arg_->width*arg_->height, "G1");
    tivxSrvUnLoadKernels(context); 
    
}

TEST(tivxSrvPointDetect, testSingleImage_2mpix)
{
    vx_context context = context_->vx_context_;
    vx_image vx_input_y = 0, vx_output_bw = 0, vx_ref_bw = 0;
    vx_graph graph = 0;
    vx_node node1 = 0;
    vx_perf_t perf_node1,  perf_graph;
    vx_uint32 width,height, i;

    svPointDetect_t in_params;
    svLdcLut_t      in_lens_params;
    ldc_lensParameters lens_params; /* This is only for reading from file */
    vx_user_data_object in_param_object;
    vx_user_data_object in_lens_param_object;

    svACDetectStructFinalCorner_t out_params;
    vx_user_data_object           out_param_object;

    svACDetectStructFinalCorner_t ref_params; /* Pre-calculated ref output values */

    /* TODO: Update */
    ref_params.finalCorners[0][0] = 570;
    ref_params.finalCorners[0][1] = 436;
    ref_params.finalCorners[0][2] = 563;
    ref_params.finalCorners[0][3] = 690;
    ref_params.finalCorners[0][4] = 916;
    ref_params.finalCorners[0][5] = 531;
    ref_params.finalCorners[0][6] = 792;
    ref_params.finalCorners[0][7] = 273;

    ref_params.finalCorners[1][0] = 552;
    ref_params.finalCorners[1][1] = 1173;
    ref_params.finalCorners[1][2] = 547;
    ref_params.finalCorners[1][3] = 1420;
    ref_params.finalCorners[1][4] = 753;
    ref_params.finalCorners[1][5] = 1597;
    ref_params.finalCorners[1][6] = 887;
    ref_params.finalCorners[1][7] = 1357;
    ref_params.numFPDetected = 8;



    #ifdef DEBUG_PRINT
    printf("Load Kernel                    \n");
    #endif
    tivxSrvLoadKernels(context); 

    // Read Lens file
    read_lut_file(&lens_params,"psdkra/srv/LENS_IMX390.bin" );  
    #ifdef DEBUG_TEST_1
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",lens_params.ldcLUT_D2U_table[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",lens_params.ldcLUT_U2D_table[1023]);
    #endif


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(vx_input_y = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(vxQueryImage(vx_input_y, VX_IMAGE_WIDTH, &width, sizeof(width)));
    VX_CALL(vxQueryImage(vx_input_y, VX_IMAGE_HEIGHT, &height, sizeof(height)));
    ct_read_image2(vx_input_y, "psdkra/srv/testBack_2mpix.yuv", 1);

    ASSERT_VX_OBJECT(vx_output_bw = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(vx_ref_bw = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);


    memset(&in_params, 0, sizeof(svPointDetect_t));
    ASSERT_VX_OBJECT(in_param_object = vxCreateUserDataObject(context, "svPointDetect_t",
                                       sizeof(svPointDetect_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&in_lens_params, 0, sizeof(svPointDetect_t));
    ASSERT_VX_OBJECT(in_lens_param_object = vxCreateUserDataObject(context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&out_params, 0, sizeof(svACDetectStructFinalCorner_t));
    ASSERT_VX_OBJECT(out_param_object = vxCreateUserDataObject(context, "svACDetectStructFinalCorner_t",
                                       sizeof(svACDetectStructFinalCorner_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Set for 2 mpix */
    in_params.thresholdMode = 0;
    in_params.windowMode = 0;
    in_params.Ransac = 0;

    in_params.SVROIWidth = 1920;
    in_params.SVROIHeight = 1080;
    in_params.binarizeOffset = 80;
    in_params.borderOffset = 80;
    in_params.smallestCenter = 10;
    in_params.largestCenter = 50;
    in_params.maxWinWidth  = 400;
    in_params.maxWinHeight = 400;
    in_params.maxBandLen = 400;
    in_params.minBandLen = 4;
    in_params.minSampleInCluster = 16;
    in_params.firstROITop = 300;
    in_params.firstROIBottom = 1050;
    in_params.firstROILeft = 50;
    in_params.firstROIRight = 900;
    in_params.secondROITop = 300;
    in_params.secondROIBottom = 1050;
    in_params.secondROILeft = 950;
    in_params.secondROIRight = 1870;
    in_params.camera_id     = 3;
    
    /* Initialize all the 4 channel ldc luts within in_params    */ 
    /* Only one of these would be used, however the actual graph */
    /* will mimic this behavior                                  */

    for (i=0;i<4; i++) {
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


    #ifdef DEBUG_TEST_1
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",in_params.ldc[3].lut_d2u[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",in_params.ldc[3].lut_u2d[1023]);   
    #endif

    VX_CALL(vxCopyUserDataObject(in_param_object, 0, sizeof(svPointDetect_t),
                                 &in_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyUserDataObject(in_lens_param_object, 0, sizeof(svLdcLut_t),
                                 &in_lens_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));


    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    #ifdef DEBUG_PRINT
    printf("Defining Nodes \n");
    #endif
    ASSERT_VX_OBJECT(node1 = tivxPointDetectNode(graph, in_param_object,in_lens_param_object,vx_input_y,  out_param_object,vx_output_bw), VX_TYPE_NODE);


    VX_CALL(vxVerifyGraph(graph)); // Verify the sanity of the graph (Validate etc)
    #ifdef DEBUG_PRINT
    printf("Call Graph \n");
    #endif
    VX_CALL(vxProcessGraph(graph)); // Running the graph a single time
    #ifdef DEBUG_PRINT
    printf("Graph Call completed \n");
    #endif

    // Performance data
    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));



    ct_read_image2(vx_ref_bw, "psdkra/srv/testBWImg_2mpix_ref.yuv", 1);

    /* Copy output object using VX_READ_ONLY */
    VX_CALL(vxCopyUserDataObject(out_param_object, 0, sizeof(svACDetectStructFinalCorner_t), &out_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));   

    #ifdef DEBUG_TEST_1
    printf("Test bench Final Corners 1 = %d %d %d %d %d %d %d %d\n",out_params.finalCorners[0][0]>>4,
    out_params.finalCorners[0][1]>>4,out_params.finalCorners[0][2]>>4, out_params.finalCorners[0][3]>>4,
    out_params.finalCorners[0][4]>>4,out_params.finalCorners[0][5]>>4, out_params.finalCorners[0][6]>>4,
    out_params.finalCorners[0][7]>>4);

    printf("Test bench Final Corners 2 = %d %d %d %d %d %d %d %d\n",out_params.finalCorners[1][0]>>4,
    out_params.finalCorners[1][1]>>4,out_params.finalCorners[1][2]>>4, out_params.finalCorners[1][3]>>4,
    out_params.finalCorners[1][4]>>4,out_params.finalCorners[1][5]>>4, out_params.finalCorners[1][6]>>4,
    out_params.finalCorners[1][7]>>4);
    printf("numFPDetected = %d \n",out_params.numFPDetected);

    #endif


    ASSERT_NO_FAILURE(point_detect_binarize_check(vx_output_bw, vx_ref_bw)); 
    ASSERT_NO_FAILURE(point_detect_corners_check ( ref_params , out_params));


    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&vx_input_y));
    VX_CALL(vxReleaseImage(&vx_output_bw));
    VX_CALL(vxReleaseImage(&vx_ref_bw));
    VX_CALL(vxReleaseUserDataObject(&in_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_lens_param_object));
    VX_CALL(vxReleaseUserDataObject(&out_param_object));


    ASSERT(vx_input_y == 0);
    ASSERT(vx_output_bw == 0);
    ASSERT(vx_ref_bw == 0);
    ASSERT(in_param_object == 0);
    ASSERT(in_lens_param_object == 0);
    ASSERT(out_param_object == 0);

    //printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    //printPerformance(perf_graph, arg_->width*arg_->height, "G1");
    tivxSrvUnLoadKernels(context); 
    
}
TESTCASE_TESTS(tivxSrvPointDetect,
        testSingleImage, // test-scenario-1
        testSingleImage_2mpix
)
