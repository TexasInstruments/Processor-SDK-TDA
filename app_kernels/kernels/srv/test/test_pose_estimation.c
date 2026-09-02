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
#include "TI/tivx.h"
#include "TI/tivx_srv.h"
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_point_detect.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"

//#define DEBUG_TEST_1
//#define DEBUG_TEST_2

TESTCASE(tivxSrvPoseEstimation, CT_VXContext, ct_setup_vx_context, 0)

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

static void read_chartpos_file(vx_int8 *inChartPos, const char*fileName)
{ 
    char file[MAXPATHLENGTH];  
    FILE* f = 0;
    size_t sz;
    uint32_t  read_size;

    #ifdef DEBUG_PRINT
    printf ("Reading chartpos file \n");
    #endif

    if (!fileName)
    {
        printf("chartpos file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Cant open chartpos file: %s\n", fileName);
        return;
    }

    read_size = fread((int8_t *)inChartPos,sizeof(uint8_t),164,f);

    if (read_size != 164)
    {
        CT_ADD_FAILURE("Incorrect Bytes read from chartpos file: %s: %d should be 164\n", fileName, read_size);
        fclose(f);
        return;
    }

    #ifdef DEBUG_PRINT
    printf ("file read completed \n");
    #endif
    fclose(f);

}

static void read_calmat_file( svCalmat_t *calmat, const char*fileName)
{ 
    char file[MAXPATHLENGTH];  
    uint32_t cnt;
    FILE* f = 0;
    size_t sz;
    uint32_t  read_size;

    #ifdef DEBUG_PRINT
    printf ("Reading calmat file \n");
    #endif

    if (!fileName)
    {
        printf("calmat file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Cant open calmat file: %s\n", fileName);
        return;
    }

    read_size = fread((int8_t *)&calmat->numCameras,sizeof(uint8_t),4,f);
    if (read_size != 4)
    {
        CT_ADD_FAILURE("Incorrect Bytes read from calmat file: %s\n", fileName);
        fclose(f);
        return;
    }

    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        read_size = fread((int8_t *)&calmat->calMatSize[cnt],sizeof(uint8_t),4,f);
        if (read_size != 4)
        {
            CT_ADD_FAILURE("Incorrect Bytes read from calmat file: %s\n", fileName);
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
            CT_ADD_FAILURE("Incorrect Bytes read from calmat file: %s\n", fileName);
            fclose(f);
            return;
        }
    }

#if 0
    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        printf ("For Camera = %d Ref calmat[0] = %d Ref Calmat[11] = %d \n",cnt,calmat->calMatBuf[12*cnt+0],calmat->calMatBuf[12*cnt +11]);
    }
#endif
    #ifdef DEBUG_PRINT
    printf ("file read completed \n");
    #endif
    fclose(f);

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




static void pose_estimation_calmat_check(svCalmat_t ref_calmat, svCalmat_t out_calmat)
/* ref corners are downshifted by 4, out corners need to be shifted down */
{
    int32_t   i,j;
    int32_t  reference,out;

    for (i=0;i<4;i++) 
    {
        for (j=0;j<12;j++) {
            reference = ref_calmat.calMatBuf[12*i+j]; 
            out       = out_calmat.calMatBuf[12*i+j];
            ASSERT(reference == out);
        } //j
    } //i

}





// scenario-1
TEST(tivxSrvPoseEstimation, testSingleImage)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0;
    vx_perf_t perf_node1,  perf_graph;
    int iter, i, idx, jdx;
    vx_int32 corners[8][8] = {
    {5356, 5163, 8138, 4915, 6178, 8588, 3042, 7778                            },
    {12451, 4756, 15140, 4772, 17598, 7113, 14859, 8155                        },

    {3671, 4180, 5507, 3824, 3197, 7492, 1997, 6798                            },
    {15013, 3970, 16914, 4413, 18497, 7166, 17203, 7849                        },

    {5666, 5670, 8305, 5763, 5681, 9401, 2983, 7841                            },
    {12745, 5917, 15609, 5996, 18137, 8569, 15089, 10011                       },

    {3679, 4160, 5554, 3771, 3316, 7809, 2080, 6964                           },
    {15521, 3941, 17293, 4424, 18743, 7284, 17583, 8030                       }
    };

    svPoseEstimation_t    in_params;
    svLdcLut_t            in_lens_params;

    svCalmat_t            calmat_ref;
    svCalmat_t            calmat_out;
    vx_user_data_object   in_param_object;
    vx_user_data_object   in_lens_param_object;


    svACDetectStructFinalCorner_t in_corners;  /* Output from previous node */
    vx_user_data_object   in_corner_object; //acts as the exemplar (has data only for 1 cam now)
    vx_object_array       in_corner_object_array; //array of type above
    



    ldc_lensParameters lens_params; // For file i/o

    svACCalmatStruct_t out_params;
    vx_user_data_object   out_param_object;


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



    // Create Objects
    memset(&in_params, 0, sizeof(svPoseEstimation_t));
    ASSERT_VX_OBJECT(in_param_object = vxCreateUserDataObject(context, "svPoseEstimation_t",
                                       sizeof(svPoseEstimation_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&in_lens_params, 0, sizeof(svLdcLut_t));
    ASSERT_VX_OBJECT(in_lens_param_object = vxCreateUserDataObject(context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);


    memset(&in_corners, 0, sizeof(svACDetectStructFinalCorner_t));
    ASSERT_VX_OBJECT(in_corner_object = vxCreateUserDataObject(context, "svACDetectStructFinalCorner_t",
                             sizeof(svACDetectStructFinalCorner_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(in_corner_object_array = vxCreateObjectArray(context, (vx_reference)in_corner_object, 4), VX_TYPE_OBJECT_ARRAY);  
    VX_CALL(vxReleaseUserDataObject(&in_corner_object)); 

    memset(&out_params, 0, sizeof(svACCalmatStruct_t));
    ASSERT_VX_OBJECT(out_param_object = vxCreateUserDataObject(context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Set Inputs */
    for (iter =0;iter<4;iter++) {
       /* set in_corners/camera here */      
        in_corners.finalCorners[0][1] = corners[iter*2][0];
        in_corners.finalCorners[0][0] = corners[iter*2][1];
        in_corners.finalCorners[0][3] = corners[iter*2][2];
        in_corners.finalCorners[0][2] = corners[iter*2][3];
        in_corners.finalCorners[0][5] = corners[iter*2][4];
        in_corners.finalCorners[0][4] = corners[iter*2][5];
        in_corners.finalCorners[0][7] = corners[iter*2][6];
        in_corners.finalCorners[0][6] = corners[iter*2][7];

        in_corners.finalCorners[1][1] = corners[iter*2 +1][0];
        in_corners.finalCorners[1][0] = corners[iter*2 +1][1];
        in_corners.finalCorners[1][3] = corners[iter*2 +1][2];
        in_corners.finalCorners[1][2] = corners[iter*2 +1][3];
        in_corners.finalCorners[1][5] = corners[iter*2 +1][4];
        in_corners.finalCorners[1][4] = corners[iter*2 +1][5];
        in_corners.finalCorners[1][7] = corners[iter*2 +1][6];
        in_corners.finalCorners[1][6] = corners[iter*2 +1][7];
        in_corners.numFPDetected = 8;


        ASSERT_VX_OBJECT(in_corner_object=(vx_user_data_object)vxGetObjectArrayItem(in_corner_object_array,iter), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(in_corner_object, 0, sizeof(svACDetectStructFinalCorner_t),
                                   &in_corners, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));
         
        VX_CALL(vxReleaseUserDataObject(&in_corner_object)); 
        
    }

    in_params.Ransac = 0;
    in_params.SingleChartPose = 0;
    in_params.numCameras = 4;
    
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
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",in_params.ldc_ptr->lut_d2u[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",in_params.ldc_ptr->lut_u2d[1023]);   
    #endif

    /* Add routine to read chartPos */
    read_chartpos_file(in_params.inChartPos ,"psdkra/srv/CHARTPOS.bin");
    read_calmat_file (&calmat_ref, "psdkra/srv/calmat_ref.bin");
    #ifdef DEBUG_TEST_1
    printf("chartpos[0] = %d \n",in_params.inChartPos[0]);
    printf("chartpos[255] = %d \n",in_params.inChartPos[255]);
    #endif


    /* Copy data to user data objects */
    VX_CALL(vxCopyUserDataObject(in_param_object, 0, sizeof(svPoseEstimation_t),
                                 &in_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyUserDataObject(in_lens_param_object, 0, sizeof(svLdcLut_t),
                                 &in_lens_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));

    // Create a graph object 
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    #ifdef DEBUG_PRINT
    printf("Defining Nodes \n");
    #endif
    ASSERT_VX_OBJECT(node1 = tivxPoseEstimationNode(graph, in_param_object, in_lens_param_object, in_corner_object_array,  out_param_object), VX_TYPE_NODE);


    VX_CALL(vxVerifyGraph(graph)); 
    #ifdef DEBUG_PRINT
    printf("Call Graph \n");
    #endif
    VX_CALL(vxProcessGraph(graph)); 
    #ifdef DEBUG_PRINT
    printf("Graph Call completed \n");
    #endif

    // Performance data
    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxCopyUserDataObject(out_param_object, 0, sizeof(svACCalmatStruct_t), &out_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));   

    for (idx=0;idx<4;idx++) {
        for (jdx=0;jdx<12;jdx++ ) {
          calmat_out.calMatBuf[12*idx + jdx] = out_params.outcalmat[12*idx + jdx];
        }
    }


    #ifdef DEBUG_TEST_2
    for (idx=0;idx<4;idx++) {
      printf("Calmat for Camera number %d  =  %d %d %d %d %d %d %d %d %d %d %d %d\n",idx,
      out_params.outcalmat[12*idx +0], out_params.outcalmat[12*idx +1],
      out_params.outcalmat[12*idx +2], out_params.outcalmat[12*idx +3],
      out_params.outcalmat[12*idx +4], out_params.outcalmat[12*idx +5],
      out_params.outcalmat[12*idx +6], out_params.outcalmat[12*idx +7],
      out_params.outcalmat[12*idx +8], out_params.outcalmat[12*idx +9],
      out_params.outcalmat[12*idx +10], out_params.outcalmat[12*idx +11]);
    } 

    #endif

    /* Uncomment for negative testing */
    //calmat_out.calMatBuf[35] = 0;
    
    ASSERT_NO_FAILURE(pose_estimation_calmat_check ( calmat_ref , calmat_out));
    #ifdef DEBUG_PRINT
    printf("Reference calmat 1 mpix tested \n");
    #endif

    VX_CALL(vxReleaseUserDataObject(&in_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_lens_param_object));
    VX_CALL(vxReleaseObjectArray(&in_corner_object_array));
    VX_CALL(vxReleaseUserDataObject(&out_param_object));


    ASSERT(in_param_object == 0);
    ASSERT(in_lens_param_object == 0);
    ASSERT(in_corner_object == 0);
    ASSERT(in_corner_object_array == 0);
    ASSERT(out_param_object == 0);

    VX_CALL(vxReleaseNode(&node1));
    #ifdef DEBUG_PRINT
    printf("Node released \n");
    #endif
    VX_CALL(vxReleaseGraph(&graph));
    #ifdef DEBUG_PRINT
    printf("Graph released \n");
    #endif

    ASSERT(node1 == 0);
    ASSERT(graph == 0);
    #ifdef DEBUG_PRINT
    printf("node /graph released released \n");
    #endif


    //printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    //printPerformance(perf_graph, arg_->width*arg_->height, "G1");
    
    #ifdef DEBUG_PRINT
    printf("Test completed, unloading kernels \n");
    #endif
    tivxSrvUnLoadKernels(context); 
    
}


TEST(tivxSrvPoseEstimation, testSingleImage_2mpix)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0;
    int iter, i, idx, jdx;
    vx_perf_t perf_node1,  perf_graph;
    vx_int32 corners[8][8] = {
    {7722, 9790, 11860, 9754, 9141, 15532, 5029, 13267                            },
    {19721, 9593, 23684, 9450, 26404, 12827, 22633, 15208                         },

    {5328, 7668, 7752, 7633, 4873, 12860, 3303, 10994                             },
    {22951, 7885, 25333, 7996, 27141, 11448, 25488, 13279                         },

    {7651, 9874, 11466, 9319, 9142, 15355, 5412, 13769                            },
    {19457, 8632, 23687, 8539, 26577, 12246, 22591, 14547                         },

    {5731, 10400, 8125, 10466, 5417, 15240, 3734, 13429                           },
    {22686, 10007, 25213, 9781, 27453, 12675, 25863, 14608                        }
    };

    svPoseEstimation_t    in_params;
    svLdcLut_t            in_lens_params;

    svCalmat_t            calmat_ref;
    svCalmat_t            calmat_out;
    vx_user_data_object   in_param_object;
    vx_user_data_object   in_lens_param_object;


    svACDetectStructFinalCorner_t in_corners;  /* Output from previous node */
    vx_user_data_object   in_corner_object; //acts as the exemplar (has data only for 1 cam now)
    vx_object_array       in_corner_object_array; //array of type above
    



    ldc_lensParameters lens_params; // For file i/o

    svACCalmatStruct_t out_params;
    vx_user_data_object   out_param_object;


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




    // Create Objects
    memset(&in_params, 0, sizeof(svPoseEstimation_t));
    ASSERT_VX_OBJECT(in_param_object = vxCreateUserDataObject(context, "svPoseEstimation_t",
                                       sizeof(svPoseEstimation_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&in_lens_params, 0, sizeof(svLdcLut_t));
    ASSERT_VX_OBJECT(in_lens_param_object = vxCreateUserDataObject(context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);


    memset(&in_corners, 0, sizeof(svACDetectStructFinalCorner_t));
    ASSERT_VX_OBJECT(in_corner_object = vxCreateUserDataObject(context, "svACDetectStructFinalCorner_t",
                             sizeof(svACDetectStructFinalCorner_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(in_corner_object_array = vxCreateObjectArray(context, (vx_reference)in_corner_object, 4), VX_TYPE_OBJECT_ARRAY);  
    VX_CALL(vxReleaseUserDataObject(&in_corner_object)); 

    memset(&out_params, 0, sizeof(svACCalmatStruct_t));
    ASSERT_VX_OBJECT(out_param_object = vxCreateUserDataObject(context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);


    /* Set Inputs */
    for (iter =0;iter<4;iter++) {
       /* set in_corners/camera here */      
        in_corners.finalCorners[0][1] = corners[iter*2][0];
        in_corners.finalCorners[0][0] = corners[iter*2][1];
        in_corners.finalCorners[0][3] = corners[iter*2][2];
        in_corners.finalCorners[0][2] = corners[iter*2][3];
        in_corners.finalCorners[0][5] = corners[iter*2][4];
        in_corners.finalCorners[0][4] = corners[iter*2][5];
        in_corners.finalCorners[0][7] = corners[iter*2][6];
        in_corners.finalCorners[0][6] = corners[iter*2][7];

        in_corners.finalCorners[1][1] = corners[iter*2 +1][0];
        in_corners.finalCorners[1][0] = corners[iter*2 +1][1];
        in_corners.finalCorners[1][3] = corners[iter*2 +1][2];
        in_corners.finalCorners[1][2] = corners[iter*2 +1][3];
        in_corners.finalCorners[1][5] = corners[iter*2 +1][4];
        in_corners.finalCorners[1][4] = corners[iter*2 +1][5];
        in_corners.finalCorners[1][7] = corners[iter*2 +1][6];
        in_corners.finalCorners[1][6] = corners[iter*2 +1][7];
        in_corners.numFPDetected = 8;


        ASSERT_VX_OBJECT(in_corner_object=(vx_user_data_object)vxGetObjectArrayItem(in_corner_object_array,iter), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(in_corner_object, 0, sizeof(svACDetectStructFinalCorner_t),
                                   &in_corners, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));
         
        VX_CALL(vxReleaseUserDataObject(&in_corner_object)); 
        
    }

    in_params.Ransac = 0;
    in_params.SingleChartPose = 0;
    in_params.numCameras = 4;
    
    /* Initialize all the 4 channel ldc luts within in_params    */ 

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
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",in_params.ldc_ptr->lut_d2u[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",in_params.ldc_ptr->lut_u2d[1023]);   
    #endif

    /* Add routine to read chartPos */
    read_chartpos_file(in_params.inChartPos ,"psdkra/srv/CHARTPOS.bin");
    read_calmat_file (&calmat_ref, "psdkra/srv/calmat_imx390_ref.bin"); 
    #ifdef DEBUG_TEST_1
    printf("chartpos[0] = %d \n",in_params.inChartPos[0]);
    printf("chartpos[163] = %d \n",in_params.inChartPos[163]);
    #endif


    /* Copy data to user data objects */
    VX_CALL(vxCopyUserDataObject(in_param_object, 0, sizeof(svPoseEstimation_t),
                                 &in_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyUserDataObject(in_lens_param_object, 0, sizeof(svLdcLut_t),
                                 &in_lens_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST));

    // Create a graph object 
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    #ifdef DEBUG_PRINT
    printf("Defining Nodes \n");
    #endif
    ASSERT_VX_OBJECT(node1 = tivxPoseEstimationNode(graph,  in_param_object, in_lens_param_object,in_corner_object_array, out_param_object), VX_TYPE_NODE);


    VX_CALL(vxVerifyGraph(graph)); 
    #ifdef DEBUG_PRINT
    printf("Call Graph \n");
    #endif
    VX_CALL(vxProcessGraph(graph)); 
    #ifdef DEBUG_PRINT
    printf("Graph Call completed \n");
    #endif

    // Performance data
    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));


    VX_CALL(vxCopyUserDataObject(out_param_object, 0, sizeof(svACCalmatStruct_t), &out_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));   

    for (idx=0;idx<4;idx++) {
        for (jdx=0;jdx<12;jdx++ ) {
          calmat_out.calMatBuf[12*idx + jdx] = out_params.outcalmat[12*idx + jdx];
        }
    }


    #ifdef DEBUG_TEST_2
    for (idx=0;idx<4;idx++) {
      printf("Calmat for Camera number %d  =  %d %d %d %d %d %d %d %d %d %d %d %d\n",idx,
      out_params.outcalmat[12*idx +0], out_params.outcalmat[12*idx +1],
      out_params.outcalmat[12*idx +2], out_params.outcalmat[12*idx +3],
      out_params.outcalmat[12*idx +4], out_params.outcalmat[12*idx +5],
      out_params.outcalmat[12*idx +6], out_params.outcalmat[12*idx +7],
      out_params.outcalmat[12*idx +8], out_params.outcalmat[12*idx +9],
      out_params.outcalmat[12*idx +10], out_params.outcalmat[12*idx +11]);
    } 

    #endif

    /* Uncomment for negative testing */
    //calmat_out.calMatBuf[35] = 0;
    
    ASSERT_NO_FAILURE(pose_estimation_calmat_check ( calmat_ref , calmat_out));
    #ifdef DEBUG_PRINT
    printf("Reference calmat 2mpix tested \n");
    #endif

    VX_CALL(vxReleaseUserDataObject(&in_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_lens_param_object));
    VX_CALL(vxReleaseObjectArray(&in_corner_object_array));
    VX_CALL(vxReleaseUserDataObject(&out_param_object));


    ASSERT(in_param_object == 0);
    ASSERT(in_lens_param_object == 0);
    ASSERT(in_corner_object == 0);
    ASSERT(in_corner_object_array == 0);
    ASSERT(out_param_object == 0);

    VX_CALL(vxReleaseNode(&node1));
    #ifdef DEBUG_PRINT
    printf("Node released \n");
    #endif
    VX_CALL(vxReleaseGraph(&graph));
    #ifdef DEBUG_PRINT
    printf("Graph released \n");
    #endif

    ASSERT(node1 == 0);
    ASSERT(graph == 0);
    #ifdef DEBUG_PRINT
    printf("node /graph released released \n");
    #endif


    //printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    //printPerformance(perf_graph, arg_->width*arg_->height, "G1");
    
    #ifdef DEBUG_PRINT
    printf("Test completed, unloading kernels \n");
    #endif
    tivxSrvUnLoadKernels(context); 
    
}

TESTCASE_TESTS(tivxSrvPoseEstimation,
        testSingleImage,// test-scenario-1
        testSingleImage_2mpix
)
