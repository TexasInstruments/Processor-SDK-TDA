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
//#define DEBUG_PRINT
#define CHECK_CALMAT
#define CHECK_LUT


#define SV_OUT_DISPLAY_HEIGHT   (1080)
#define SV_OUT_DISPLAY_WIDTH    (1080)
#define SV_SUBSAMPLE            (4)
#define SV_XYZLUT3D_SIZE        (SV_OUT_DISPLAY_HEIGHT/SV_SUBSAMPLE) * (SV_OUT_DISPLAY_WIDTH/SV_SUBSAMPLE) *3
#define SV_GPULUT_SIZE          ((2 + SV_OUT_DISPLAY_WIDTH/SV_SUBSAMPLE) * (2 + SV_OUT_DISPLAY_HEIGHT/SV_SUBSAMPLE)) *7

TESTCASE(tivxSrvGenerateGpulut, CT_VXContext, ct_setup_vx_context, 0)

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

/* lut_size is the number of entries in the LUT */
static void read_xyzlut3d_file(vx_float32 *xyzlut3d, const char*fileName, vx_int32 lut_size)
{ 
    char file[MAXPATHLENGTH];  
    FILE* f = 0;
    size_t sz;
    uint32_t  read_size;

    #ifdef DEBUG_PRINT
    printf ("Reading xyzlut3d file \n");
    #endif

    if (!fileName)
    {
        printf("xyzlut3d file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Cant open xyzlut3d file: %s\n", fileName);
        return;
    }

    read_size = fread((float *)xyzlut3d,sizeof(float),lut_size,f);
    if (read_size != lut_size)
    {
        CT_ADD_FAILURE("Incorrect Bytes read from xyzlut3d file: %s\n", fileName);
        fclose(f);
        return;
    }

    #ifdef DEBUG_PRINT
    printf ("file read completed \n");
    #endif
    fclose(f);

}


/* lut_size is the number of entries in the LUT */
static void read_gpulut_file(vx_uint16 *gpulut, const char*fileName, vx_int32 lut_size)
{ 
    char file[MAXPATHLENGTH];  
    FILE* f = 0;
    size_t sz;
    uint32_t  read_size;

    #ifdef DEBUG_PRINT
    printf ("Reading gpulut file \n");
    #endif

    if (!fileName)
    {
        printf("gpulut file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Cant open gpulut file: %s\n", fileName);
        return;
    }

    read_size = fread((int16_t *)gpulut,sizeof(int16_t),lut_size,f);
    if (read_size != lut_size)
    {
        CT_ADD_FAILURE("Incorrect Bytes read from gpulut file: %s\n", fileName);
        fclose(f);
        return;
    }

    #ifdef DEBUG_PRINT
    printf ("file read completed \n");
    #endif
    fclose(f);

}


static void gpulut_generation_lut_check(uint16_t *ref_lut, uint16_t *out_lut, int size)
{
    int32_t   i;
    int16_t  reference,out;
    for (i=0;i<size;i++) 
    {
        reference = ref_lut[i]; 
        out       = out_lut[i];
        if (abs(reference -out ) >1) {
            printf ("Failure at location i = %d ref = %d out = %d \n",i,reference,out);
        }
        ASSERT(abs(reference -out)<2);
    } //i

}


TEST(tivxSrvGenerateGpulut, testBowl_2mpix)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0;
    vx_perf_t perf_node1,  perf_graph;


    svGpuLutGen_t               in_params;
    vx_user_data_object         in_param_object;

    svLdcLut_t                  in_lens_params;
    vx_user_data_object         in_lens_param_object;

    svACCalmatStruct_t          in_calmat_scaled;
    vx_user_data_object         in_calmat_scaled_object;

    vx_array                    in_lut3dxyz_array;
    float                       *in_lut3dxyz = (float*)ct_alloc_mem(sizeof(float)*SV_XYZLUT3D_SIZE);

    vx_array                    out_gpulut_array;
    uint16_t                    *out_gpulut = (uint16_t*)ct_alloc_mem(sizeof(uint16_t)*SV_GPULUT_SIZE);
    int idx, i;

    vx_uint16                   *ref_gpulut = (vx_uint16*)ct_alloc_mem(sizeof(vx_uint16)*SV_GPULUT_SIZE) ; // Read reference dumped from tda2p
    ldc_lensParameters lens_params; // For file i/o


    vx_map_id map_id_2;
    vx_size item_size_2;
    uint32_t *data_ptr_2;

    vx_uint16                    *returned_gpulut;

    vx_float32         in_scaled_calmat[48] = 
                      {  0.999841,   -0.017470,  0.010282,  -0.009986,  -0.557769,  0.829880,  -0.008763,   -0.829851,  -0.557855,  -380.659180,  607.506836,  -567.215820,
                         -0.025237,  -0.727346,  0.684830,  -0.995363,  -0.033693,  -0.097411,  0.093925,   -0.684113,  -0.723123,  569.925781,   535.681641,  -211.667969,
                         -0.989845,  0.097190,   0.103037,  -0.010456,   0.575207,  -0.818027,  -0.138771,  -0.810798,  -0.568350,  399.508789,   -44.314453,  310.340820  ,
                          0.069153,  0.474242,   -0.878358,  0.996557,  -0.026034,  0.070655,   0.010641,   -0.880220,  -0.474410,  -565.915039,  72.944336,   282.373047   
                      };
   /* Output from generate_3dbowl node */

    #ifdef DEBUG_PRINT
    printf("Load Kernel                    \n");
    #endif
    tivxSrvLoadKernels(context); 


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    // Read reference data for 3d lut
    read_gpulut_file(ref_gpulut,"psdkra/srv/GAlignLUT3D.bin",SV_GPULUT_SIZE);



    // Create objects
    memset(&in_params, 0, sizeof(svGpuLutGen_t));
    ASSERT_VX_OBJECT(in_param_object = vxCreateUserDataObject(context, "svGpuLutGen_t",
                                       sizeof(svGpuLutGen_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&in_lens_params, 0, sizeof(svLdcLut_t));
    ASSERT_VX_OBJECT(in_lens_param_object = vxCreateUserDataObject(context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    memset(&in_calmat_scaled, 0, sizeof(svACCalmatStruct_t));
    ASSERT_VX_OBJECT(in_calmat_scaled_object = vxCreateUserDataObject(context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
    ASSERT_VX_OBJECT(in_lut3dxyz_array = vxCreateArray(context, VX_TYPE_FLOAT32, SV_XYZLUT3D_SIZE), VX_TYPE_ARRAY);


    ASSERT_VX_OBJECT(out_gpulut_array = vxCreateArray(context, VX_TYPE_UINT16, SV_GPULUT_SIZE), VX_TYPE_ARRAY);


    /* Populate input data */
    in_params.SVInCamFrmHeight = 1080;
    in_params.SVInCamFrmWidth  = 1920;
    in_params.SVOutDisplayHeight = SV_OUT_DISPLAY_HEIGHT;
    in_params.SVOutDisplayWidth =  SV_OUT_DISPLAY_WIDTH;
    in_params.numCameras        = 4;
    in_params.subsampleratio    = SV_SUBSAMPLE;
    in_params.useWideBowl       = 1;


    
    #ifdef DEBUG_TEST_1
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",in_params.ldc_ptr->lut_d2u[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",in_params.ldc_ptr->lut_u2d[1023]);   
    #endif

    for (idx=0;idx<48;idx++) {
        in_calmat_scaled.scaled_outcalmat[idx] = in_scaled_calmat[idx];
        //printf("in_calmat_scaled for index %d = %f \n",idx,in_calmat_scaled.scaled_outcalmat[idx]); 
    }

    // Read Lens file
    read_lut_file(&lens_params,"psdkra/srv/LENS_IMX390.bin" );
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
   
    read_xyzlut3d_file(in_lut3dxyz,"psdkra/srv/XYZ3DLUT.bin",SV_XYZLUT3D_SIZE); // Read bowl lut
    if (NULL != in_lut3dxyz)
    {
        in_lut3dxyz[28457]= 0;
        in_lut3dxyz[29057]= 0;   
    }

    VX_CALL(vxCopyUserDataObject(in_param_object, 0, sizeof(svGpuLutGen_t), &in_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyUserDataObject(in_lens_param_object, 0, sizeof(svLdcLut_t), &in_lens_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyUserDataObject(in_calmat_scaled_object, 0, sizeof(svACCalmatStruct_t), &in_calmat_scaled, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));   
    VX_CALL(vxAddArrayItems(in_lut3dxyz_array, SV_XYZLUT3D_SIZE, in_lut3dxyz, sizeof(float)));   
    VX_CALL(vxCopyArrayRange(in_lut3dxyz_array, 0,SV_XYZLUT3D_SIZE, sizeof(float), (void *)in_lut3dxyz, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));   


    // Create a graph object 
    #ifdef DEBUG_PRINT
    printf ("Create graph \n");
    #endif
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    //   Add nodes -> refer to tivx_srv_node_api.c file to see how it's called
    #ifdef DEBUG_PRINT
    printf ("Create node  \n");
    #endif
    ASSERT_VX_OBJECT(node1 = tivxGenerateGpulutNode(graph, in_param_object, in_lens_param_object, 
                                                    in_calmat_scaled_object,in_lut3dxyz_array, 
                                                    out_gpulut_array), VX_TYPE_NODE);


    #ifdef DEBUG_PRINT
    printf ("Node Created  \n");
    #endif
    VX_CALL(vxVerifyGraph(graph)); 
    #ifdef DEBUG_PRINT
    printf ("Verify completed  \n");
    #endif
    VX_CALL(vxProcessGraph(graph)); 
    #ifdef DEBUG_PRINT
    printf ("Process completed  \n");
    #endif

    // Performance data
    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));



#ifdef CHECK_LUT
    /* Map primary output-2 */
    vxMapArrayRange(out_gpulut_array, 0, 1, &map_id_2, &item_size_2,
                   (void *)&data_ptr_2,
                   VX_READ_ONLY,
                   VX_MEMORY_TYPE_HOST, 0);


    

    returned_gpulut = (vx_uint16 *)data_ptr_2;
    if (NULL != ref_gpulut)
    {
        ASSERT_NO_FAILURE(gpulut_generation_lut_check (ref_gpulut, returned_gpulut,  SV_GPULUT_SIZE ));
    }
    #ifdef DEBUG_PRINT
    printf("Completed checking for lut \n");
    #endif
#endif


#ifdef CHECK_LUT
    vxUnmapArrayRange(out_gpulut_array, map_id_2);
#endif

    VX_CALL(vxReleaseUserDataObject(&in_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_lens_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_calmat_scaled_object));
    VX_CALL(vxReleaseArray(&in_lut3dxyz_array));
    VX_CALL(vxReleaseArray(&out_gpulut_array));

    ct_free_mem(in_lut3dxyz);
    ct_free_mem(out_gpulut);
    ct_free_mem(ref_gpulut);
    ASSERT(in_param_object == 0);
    ASSERT(in_lens_param_object == 0);
    ASSERT(in_calmat_scaled_object == 0);
    ASSERT(in_lut3dxyz_array == 0);
    ASSERT(out_gpulut_array == 0);

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
    printf("node /graph released \n");
    #endif


    //printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    //printPerformance(perf_graph, arg_->width*arg_->height, "G1");
    
    #ifdef DEBUG_PRINT
    printf("Test completed, unloading kernels \n");
    #endif
    tivxSrvUnLoadKernels(context); 
    
}


TESTCASE_TESTS(tivxSrvGenerateGpulut,
        testBowl_2mpix
)
