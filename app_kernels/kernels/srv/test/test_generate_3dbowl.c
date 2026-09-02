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

TESTCASE(tivxSrvGenerate3Dbowl, CT_VXContext, ct_setup_vx_context, 0)



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




static void bowl_generation_calmat_check(svACCalmatStruct_t *ref_calmat, svACCalmatStruct_t *out_calmat)
{
    int32_t   i,j;
    float  reference,out;

    for (i=0;i<4;i++) 
    {
        #ifdef DEBUG_TEST_2
        printf ("\nout_calmat for row  %d =",i); 
        #endif
        for (j=0;j<12;j++) {
            reference = ref_calmat->scaled_outcalmat[12*i+j]; 
            out       = out_calmat->scaled_outcalmat[12*i+j];
            #ifdef DEBUG_TEST_2
            printf("%f  ",out);
            #endif
            if (fabs(reference -out)>0.0001) {
                printf("Failure @ camera = %d and location = %d  reference = %f out = %f \n",i,j,reference,out);
            }
            ASSERT(fabs(reference - out) < 0.0001);
        } //j
    } //i

}


static void bowl_generation_3dlut_check(float *ref_lut, float *out_lut, int size)
{
    int32_t   i;
    float  reference,out;
    for (i=0;i<size;i++) 
    {
        reference = ref_lut[i]; 
        out       = out_lut[i];
        if (reference != out ) {
            printf ("Failure at location i = %d ref = %f out = %f \n",i,reference,out);
        }
        ASSERT(reference == out);
    } //i

}


typedef struct {
    const char* testName;
    int dummy_;
    int width, height;
} Arg;




TEST(tivxSrvGenerate3Dbowl, testBowl_2mpix)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0;
    vx_perf_t perf_node1,  perf_graph;


    svGpuLutGen_t               in_params;

    vx_user_data_object         in_param_object;


    svACCalmatStruct_t          in_calmat;
    vx_user_data_object         in_calmat_object;

    svGeometric_t               in_offset_params;
    vx_user_data_object         in_offset_params_object;

    vx_array                    out_lut3dxyz_array;
    float                       *out_lut3dxyz = (float*)ct_alloc_mem(sizeof(float)*SV_XYZLUT3D_SIZE);

    svACCalmatStruct_t          out_calmat;
    vx_user_data_object         out_calmat_object;

    svACCalmatStruct_t          ref_scaled_calmat; /* Ref for scaled calmat output for testing */
    vx_float32                  *ref_lut3dxyz = (vx_float32*)ct_alloc_mem(sizeof(vx_float32)*SV_XYZLUT3D_SIZE) ; // Read reference dumped from tda2p

    svCalmat_t                  calmat_file;

    vx_map_id  map_id_2;
    vx_size item_size_2;
    uint32_t *data_ptr_2;
    int idx;

    vx_float32         *returned_3dlutxyz;
    vx_float32         golden_scaled_calmat[48] = 
                      {  0.999841,   -0.017470,  0.010282,  -0.009986,  -0.557769,  0.829880,  -0.008763,   -0.829851,  -0.557855,  -380.659180,  607.506836,  -567.215820,
                         -0.025237,  -0.727346,  0.684830,  -0.995363,  -0.033693,  -0.097411,  0.093925,   -0.684113,  -0.723123,  569.925781,   535.681641,  -211.667969,
                         -0.989845,  0.097190,   0.103037,  -0.010456,   0.575207,  -0.818027,  -0.138771,  -0.810798,  -0.568350,  399.508789,   -44.314453,  310.340820  ,
                          0.069153,  0.474242,   -0.878358,  0.996557,  -0.026034,  0.070655,   0.010641,   -0.880220,  -0.474410,  -565.915039,  72.944336,   282.373047   
                      };

    #ifdef DEBUG_PRINT
    printf("Load Kernel                    \n");
    #endif
    tivxSrvLoadKernels(context); 


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    // Read reference data for 3d lut
    read_xyzlut3d_file(ref_lut3dxyz,"psdkra/srv/XYZ3DLUT.bin",SV_XYZLUT3D_SIZE);



    // Create objects
    memset(&in_params, 0, sizeof(svGpuLutGen_t));
    ASSERT_VX_OBJECT(in_param_object = vxCreateUserDataObject(context, "svGpuLutGen_t",
                                       sizeof(svGpuLutGen_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);



    memset(&in_calmat, 0, sizeof(svACCalmatStruct_t));
    ASSERT_VX_OBJECT(in_calmat_object = vxCreateUserDataObject(context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 

    memset(&in_offset_params, 0, sizeof(svGeometric_t));
    ASSERT_VX_OBJECT(in_offset_params_object = vxCreateUserDataObject(context, "svGeometric_t",
                                       sizeof(svGeometric_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);


    ASSERT_VX_OBJECT(out_lut3dxyz_array = vxCreateArray(context, VX_TYPE_FLOAT32, sizeof(float)*SV_XYZLUT3D_SIZE), VX_TYPE_ARRAY);

    memset(&out_calmat, 0, sizeof(svACCalmatStruct_t));
    ASSERT_VX_OBJECT(out_calmat_object = vxCreateUserDataObject(context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Populate reference data */
    for (idx =0;idx<48;idx++) {
        ref_scaled_calmat.scaled_outcalmat[idx] = golden_scaled_calmat[idx];
    }


    /* Populate input data */
    in_params.SVInCamFrmHeight = 1080;
    in_params.SVInCamFrmWidth  = 1920;
    in_params.SVOutDisplayHeight = SV_OUT_DISPLAY_HEIGHT;
    in_params.SVOutDisplayWidth =  SV_OUT_DISPLAY_WIDTH;
    in_params.numCameras        = 4;
    in_params.subsampleratio    = SV_SUBSAMPLE;
    in_params.useWideBowl       = 1;


    in_offset_params.offsetXleft     = -400;
    in_offset_params.offsetXright    = 400;
    in_offset_params.offsetYfront    = -450;
    in_offset_params.offsetYback     = 450;

    
    #ifdef DEBUG_TEST_1
    printf("Last entry of D2u table = %f , expected value is around 29.48 \n",in_params.ldc_ptr->lut_d2u[1023]);
    printf("Last entry of U2D table = %f , expected value is around 611.57 \n",in_params.ldc_ptr->lut_u2d[1023]);   
    #endif

    read_calmat_file (&calmat_file, "psdkra/srv/calmat_imx390_test_3dbowl.bin");
    for (idx=0;idx<48;idx++) {
        in_calmat.outcalmat[idx] = calmat_file.calMatBuf[idx];
        //printf("in_calmat for index %d = %d \n",idx,in_calmat.outcalmat[idx]); 
    }



    VX_CALL(vxCopyUserDataObject(in_param_object, 0, sizeof(svGpuLutGen_t), &in_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyUserDataObject(in_calmat_object, 0, sizeof(svACCalmatStruct_t), &in_calmat, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));   

    VX_CALL(vxCopyUserDataObject(in_offset_params_object, 0, sizeof(svGeometric_t), &in_offset_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));   

    VX_CALL(vxAddArrayItems(out_lut3dxyz_array, 1, out_lut3dxyz, sizeof(float)*SV_XYZLUT3D_SIZE));   


    // Create a graph object 
    #ifdef DEBUG_PRINT
    printf ("Create graph \n");
    #endif
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    //   Add nodes -> refer to tivx_srv_node_api.c file to see how it's called
    #ifdef DEBUG_PRINT
    printf ("Create node  \n");
    #endif
    ASSERT_VX_OBJECT(node1 = tivxGenerate3DbowlNode(graph, in_param_object, in_calmat_object,in_offset_params_object, 
                             out_lut3dxyz_array,out_calmat_object), VX_TYPE_NODE);


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


#ifdef CHECK_CALMAT 
    /* Map primary output-1 */
    VX_CALL(vxCopyUserDataObject(out_calmat_object, 0, sizeof(svACCalmatStruct_t), &out_calmat, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));   

    ASSERT_NO_FAILURE(bowl_generation_calmat_check(&ref_scaled_calmat,&out_calmat));
    #ifdef DEBUG_PRINT
    printf("Completed checking for calmat scaled \n");
    #endif
#endif

#ifdef CHECK_LUT
    /* Map primary output-2 */
    vxMapArrayRange(out_lut3dxyz_array, 0, 1, &map_id_2, &item_size_2,
                   (void *)&data_ptr_2,
                   VX_READ_ONLY,
                   VX_MEMORY_TYPE_HOST, 0);


    

    returned_3dlutxyz = (vx_float32 *)data_ptr_2;
    /* Two locations in this testcase are NaN */
    /* These are forced to zero               */
    if (NULL != ref_lut3dxyz)
    {
        ref_lut3dxyz[28457] = 0;
        ref_lut3dxyz[29057] = 0;
        //ref_lut3dxyz[0] = 0;  /* Negative test */
        ASSERT_NO_FAILURE(bowl_generation_3dlut_check (ref_lut3dxyz, returned_3dlutxyz,  SV_XYZLUT3D_SIZE ));
    }

    #ifdef DEBUG_PRINT
    printf("Completed checking for lut \n");
    #endif
#endif

#ifdef CHECK_LUT
    vxUnmapArrayRange(out_lut3dxyz_array, map_id_2);
#endif

    VX_CALL(vxReleaseUserDataObject(&in_param_object));
    VX_CALL(vxReleaseUserDataObject(&in_calmat_object));
    VX_CALL(vxReleaseUserDataObject(&in_offset_params_object));
    VX_CALL(vxReleaseUserDataObject(&out_calmat_object));
    VX_CALL(vxReleaseArray(&out_lut3dxyz_array));

    ct_free_mem(ref_lut3dxyz);
    ct_free_mem(out_lut3dxyz);
    ASSERT(in_param_object == 0);
    ASSERT(in_calmat_object == 0);
    ASSERT(in_offset_params_object == 0);
    ASSERT(out_calmat_object == 0);
    ASSERT(out_lut3dxyz_array == 0);

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


TESTCASE_TESTS(tivxSrvGenerate3Dbowl,
        testBowl_2mpix
)
