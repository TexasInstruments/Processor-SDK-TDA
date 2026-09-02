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



#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_target_utils.h>

#include <tivx_od_postprocessing_host.h>
#include "tiadalg_interface.h"
#include "itidl_ti.h"
#include <math.h>
//#define DEBUG_KP
static tivx_target_kernel vx_ODPostProc_kernel = NULL;

/*Function to interpolate the points between start and end point*/
static vx_int32 lineInterp(vx_int16 startPoint[2], vx_int16 endPoint[2], vx_int16 *outPoints, vx_int32 totalLineParts);

typedef struct{
    tivxODPostProcParams prms_host;
    vx_int16* fe_points_ptr;
    vx_int16* fec_points_ptr;
    vx_int16* fec_list_points_ptr;
    vx_uint16* scratch_fwd;
    vx_uint16* scratch_bwd;
    vx_uint16  is_scratch_fwd_filled;
    vx_uint16  is_scratch_bwd_filled;

}tivxODPostProcParamsTarget;

static vx_status VX_CALLBACK tivxKernelODPostProcCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    tivxODPostProcParamsTarget * prms_target = NULL;
    tivx_obj_desc_array_t *params_array = (tivx_obj_desc_array_t*)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_CONFIGURATION_IDX];


    tivxODPostProcParams * prms_host;
    void *params_array_target_ptr;
    vx_int32 points_per_line;
    vx_int32 num_keypoints;

    prms_target = tivxMemAlloc(sizeof(tivxODPostProcParamsTarget), TIVX_MEM_EXTERNAL);

    if (prms_target == NULL){
        return(VX_FAILURE);
    }

    params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

    tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    prms_host = (tivxODPostProcParams *)params_array_target_ptr;

    points_per_line = prms_host->points_per_line;
    num_keypoints = prms_host->num_keypoints;

    // Host prameter are copied as it is.
    memcpy(&prms_target->prms_host,prms_host, sizeof(tivxODPostProcParams));

    prms_target->fe_points_ptr =
        (vx_int16 *)tivxMemAlloc(prms_host->num_max_det*num_keypoints*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL);// 4 xy points per detection

    prms_target->fec_points_ptr =
        (vx_int16 *)tivxMemAlloc(prms_host->num_max_det*num_keypoints*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL);// 4 xy points per detection

    prms_target->fec_list_points_ptr =
        (vx_int16 *)tivxMemAlloc(prms_host->num_max_det*points_per_line*num_keypoints*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL); // 4 edges with 16 points of xy. 4*16*2 = 128

    prms_target->scratch_fwd =
        (vx_uint16 *)tivxMemAlloc((1024 + 1)*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL); // 1024*2*2 size of scratch needed for forward trnaformation

    prms_target->scratch_bwd =
        (vx_uint16 *)tivxMemAlloc((1024 + 1)*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL); // 1024*2*2 size of scratch needed for backward trnaformation

    prms_target->is_scratch_fwd_filled = 0;
    prms_target->is_scratch_bwd_filled = 0;

    if ((prms_target->fe_points_ptr == NULL) || (prms_target->fec_points_ptr==NULL) || (prms_target->fec_list_points_ptr==NULL)||
        (prms_target->scratch_fwd == NULL) || (prms_target->scratch_bwd ==NULL)
        ){
        return(VX_FAILURE);
    }

    tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    tivxSetTargetKernelInstanceContext(kernel, prms_target,  sizeof(tivxODPostProcParamsTarget));

    return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelODPostProcDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxODPostProcParamsTarget *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxODPostProcParamsTarget) == size))
        {
            vx_int32 points_per_line = prms->prms_host.points_per_line;
            vx_int32 num_keypoints = prms->prms_host.num_keypoints;

            tivxMemFree(prms->fe_points_ptr, prms->prms_host.num_max_det*num_keypoints*2*sizeof(vx_int16), TIVX_MEM_EXTERNAL);
            tivxMemFree(prms->fec_points_ptr, prms->prms_host.num_max_det*num_keypoints*2*sizeof(vx_int16), TIVX_MEM_EXTERNAL);
            tivxMemFree(prms->fec_list_points_ptr, prms->prms_host.num_max_det*points_per_line*num_keypoints*2*sizeof(vx_int16), TIVX_MEM_EXTERNAL);
            tivxMemFree(prms->scratch_fwd, (1024+1)*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL);
            tivxMemFree(prms->scratch_bwd, (1024+1)*2*sizeof(vx_uint16), TIVX_MEM_EXTERNAL);
        }
        tivxMemFree(prms, sizeof(tivxODPostProcParamsTarget), TIVX_MEM_EXTERNAL);

    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelODPostProcProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i,j;
    tivxODPostProcParamsTarget *prms = NULL;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxODPostProcParamsTarget) != size))
        {
            status = VX_FAILURE;
        }
    }
    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_array_t* configuration_desc;
        void * configuration_target_ptr;

        tivx_obj_desc_tensor_t *table_desc;
        void* table_target_ptr;

        tivx_obj_desc_tensor_t *rev_table_desc;
        void* rev_table_target_ptr;

        tivx_obj_desc_tensor_t *in_points_desc;
        void* in_tensor_target_ptr;

        tivx_obj_desc_tensor_t *out_points_desc;
        void *out_tensor_target_ptr;

        tivx_obj_desc_tensor_t *out_valid_desc;
        void *out_valid_tensor_target_ptr;

        vx_int16 startPoint[2];
        vx_int16 endPoint[2];
        int32_t numObjs;
        TIDL_ODLayerHeaderInfo* pHeader;
        TIDL_ODLayerObjInfo * pPSpots;
        TIDL_ODLayerObjInfo * pObjInfo;
        vx_int16 x,y;
        vx_int32 next_kp_idx;

        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_CONFIGURATION_IDX];
        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_points_desc  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_INPUT_POINTS_IDX];
        in_tensor_target_ptr  = tivxMemShared2TargetPtr(&in_points_desc->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, in_points_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        table_desc  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_INPUT_TABLE_IDX];
        table_target_ptr  = tivxMemShared2TargetPtr(&table_desc->mem_ptr);
        tivxMemBufferMap(table_target_ptr, table_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        rev_table_desc  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_INPUT_REV_TABLE_IDX];
        rev_table_target_ptr  = tivxMemShared2TargetPtr(&rev_table_desc->mem_ptr);
        tivxMemBufferMap(rev_table_target_ptr, rev_table_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        out_valid_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_POINTS_VALID_IDX];
        out_valid_tensor_target_ptr = tivxMemShared2TargetPtr(&out_valid_desc->mem_ptr);
        tivxMemBufferMap(out_valid_tensor_target_ptr, out_valid_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        pHeader = (TIDL_ODLayerHeaderInfo *)((vx_float32*)in_tensor_target_ptr + prms->prms_host.output_buffer_offset);
        pObjInfo = (TIDL_ODLayerObjInfo *)((vx_uint8 *)pHeader + (vx_uint32)pHeader->objInfoOffset);
        numObjs = (vx_uint32)pHeader->numDetObjects;

        for(i = 0; i < numObjs; i++)
        {
            pPSpots = (TIDL_ODLayerObjInfo *)((uint8_t *)pObjInfo + (i * ((vx_uint32)pHeader->objInfoSize)));

            /*Setting the default valid flag for each object*/
            ((vx_uint8*) out_valid_tensor_target_ptr)[i] = 1;

            /*Since camera parameters e.g focal length or angle tables are in base resolution of 1280x720,
                hence convert the output key points into base resolution of 1280x720, widthxheight.
                co-ordinate output from DL is normalized form [0.0 1.0].
            */
#ifdef DEBUG_KP
            if (pPSpots->label !=3)
                ((vx_uint8*) out_valid_tensor_target_ptr)[i] = 0;

            if (pPSpots->score <= 0.9999)
                ((vx_uint8*) out_valid_tensor_target_ptr)[i] = 0;
#endif
            for(j = 0; j < prms->prms_host.num_keypoints; j++)
            {
                x = (vx_int16)((pPSpots->keyPoints[j].x * prms->prms_host.width) + 0.5);
                y = (vx_int16)((pPSpots->keyPoints[j].y * prms->prms_host.height) + 0.5);

                if((x >= prms->prms_host.width) || (y >= prms->prms_host.height) ||
                   (x < 0) || (y < 0)
                  )
                {
                    ((vx_uint8*) out_valid_tensor_target_ptr)[i] = 0;
                }

                prms->fe_points_ptr[(i*prms->prms_host.num_keypoints * 2) + 2 * j] = x;
                prms->fe_points_ptr[(i*prms->prms_host.num_keypoints * 2) + 2 * j + 1] = y;
            }
        }

#ifdef DEBUG_KP
            for(i = 0; i < numObjs; i++)
            {
                if(((vx_uint8*) out_valid_tensor_target_ptr)[i] !=0)
                {
                    for(j = 0; j < prms->prms_host.num_keypoints; j++)
                    {
#ifndef PRINT_ALL_KP
                        if((prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*2) + 2*j]>=prms->prms_host.width) ||
                           (prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*2) + 2*j+1]>=prms->prms_host.height) ||
                           (prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*2) + 2*j]<=0) ||
                           (prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*2) + 2*j+1] <=0)
                            )
#endif
                        {
                            printf("fe detected kp--> \n");
                            printf("  Obj Id = %d ", i);
                            printf("(%d,%d,%d) ",j,prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*2) + 2*j],prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*2) + 2*j+1]);
                            printf("\n");
                        }
                    }
                }
            }
#endif

        /*Convert detected keypoint in fish eye corrected domain*/
        /*center_x and center_y is optical center of original image resolution widthxheight*/
        #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
        /* ADASVISION-5496: Using the natural C algorithm for tiadalg_fisheye_transformation
         *                  for J721S2/J784S4 as the recompiled version is not functional */
        status = tiadalg_fisheye_transformation_cn
                 (
                    prms->fe_points_ptr,
                    numObjs*prms->prms_host.num_keypoints,
                    prms->prms_host.center_x,
                    prms->prms_host.center_y,
                    prms->prms_host.focal_length,
                    prms->prms_host.center_x * prms->prms_host.inter_center_x_fact,
                    prms->prms_host.center_y * prms->prms_host.inter_center_y_fact,
                    rev_table_target_ptr,
                    prms->prms_host.num_table_rows,
                    prms->scratch_fwd,prms->is_scratch_fwd_filled,
                    prms->fec_points_ptr
                 );
        #else
        status = tiadalg_fisheye_transformation_c66
                 (
                    prms->fe_points_ptr,
                    numObjs*prms->prms_host.num_keypoints,
                    prms->prms_host.center_x,
                    prms->prms_host.center_y,
                    prms->prms_host.focal_length,
                    prms->prms_host.center_x * prms->prms_host.inter_center_x_fact,
                    prms->prms_host.center_y * prms->prms_host.inter_center_y_fact,
                    rev_table_target_ptr,
                    prms->prms_host.num_table_rows,
                    prms->scratch_fwd,prms->is_scratch_fwd_filled,
                    prms->fec_points_ptr
                 );
        #endif
        prms->is_scratch_fwd_filled = 0x1;
        if(status!=TIADALG_PROCESS_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tiadalg failed !!!\n");
        }

        for(i = 0; i < numObjs; i++)
        {
            for(j = 0; j < prms->prms_host.num_keypoints; j++)
            {
                startPoint[0] = prms->fec_points_ptr[i*prms->prms_host.num_keypoints*2 + j*2 + 0];
                startPoint[1] = prms->fec_points_ptr[i*prms->prms_host.num_keypoints*2 + j*2 + 1];
                /*Number of key points is assumed to be power of 2*/
                next_kp_idx = (j + 1) & (prms->prms_host.num_keypoints - 1);
                endPoint[0] = prms->fec_points_ptr[i*prms->prms_host.num_keypoints*2 + next_kp_idx*2 + 0];
                endPoint[1] = prms->fec_points_ptr[i*prms->prms_host.num_keypoints*2 + next_kp_idx*2 + 1];

                /*If any of the point of object is outside the valid region then make complete object as invalid*/

                if ((startPoint[0] >= (prms->prms_host.width*prms->prms_host.inter_center_x_fact)) ||
                    (startPoint[1] >= (prms->prms_host.height*prms->prms_host.inter_center_y_fact)) ||
                    (startPoint[0] < 0) ||
                    (startPoint[1] < 0))
                {
                    ((vx_uint8*) out_valid_tensor_target_ptr)[i] = 0;
                }

                /* If the number of line parts is N, then total number  of points on line will be one extra N + 1*/
                lineInterp(startPoint, endPoint,
                        (prms->fec_list_points_ptr +\
                        i*prms->prms_host.points_per_line*prms->prms_host.num_keypoints*2\
                        + j*prms->prms_host.points_per_line*2),
                        (prms->prms_host.points_per_line));
            }
        }
#ifdef DEBUG_KP
        for(i = 0; i < numObjs; i++)
        {
            if(((vx_uint8*) out_valid_tensor_target_ptr)[i] !=0)
            {
                for(j = 0; j < prms->prms_host.num_keypoints*prms->prms_host.points_per_line; j++)
                {
#ifndef PRINT_ALL_KP
                    if((prms->fec_list_points_ptr[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j] >= (prms->prms_host.width*prms->prms_host.inter_center_x_fact)) ||
                    (prms->fec_list_points_ptr[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j+1] >= (prms->prms_host.height*prms->prms_host.inter_center_y_fact)) ||
                    (prms->fec_list_points_ptr[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j] <= 0) ||
                    (prms->fec_list_points_ptr[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j + 1] <= 0)
                      )
#endif
                    {
                        printf("fe corrected kp--> \n");
                        printf("  Obj Id = %d ", i);
                        printf("(%d,%d,%d) ",j,prms->fec_list_points_ptr[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j],
                            prms->fe_points_ptr[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j+1]);
                        printf("\n");
                    }
                }
            }
        }
#endif
        out_points_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OD_POSTPROCESS_OUTPUT_POINTS_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_points_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_points_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        status = tiadalg_fisheye_transformation_cn
                (
                    prms->fec_list_points_ptr,
                    numObjs*prms->prms_host.points_per_line*prms->prms_host.num_keypoints,
                    prms->prms_host.center_x * prms->prms_host.inter_center_x_fact,
                    prms->prms_host.center_y * prms->prms_host.inter_center_y_fact,
                    prms->prms_host.focal_length,
                    prms->prms_host.center_x,
                    prms->prms_host.center_y,
                    table_target_ptr,
                    prms->prms_host.num_table_rows,
                    prms->scratch_bwd,prms->is_scratch_bwd_filled,
                    out_tensor_target_ptr
                );

          prms->is_scratch_bwd_filled = 0x1;

#ifdef DEBUG_KP
        for(i = 0; i < numObjs; i++)
        {
            pPSpots = (TIDL_ODLayerObjInfo *)((uint8_t *)pObjInfo + (i * ((vx_uint32)pHeader->objInfoSize)));
            if(((vx_uint8*) out_valid_tensor_target_ptr)[i] !=0)
            {
                printf(" Obj Id = %d ", i);
                printf(" label is %f ",pPSpots->label);
                printf(" score is %f ",pPSpots->score);
                printf(" %f %f %f %f \n",pPSpots->xmin,pPSpots->ymin,pPSpots->xmax,pPSpots->ymax);

                for(j = 0; j < prms->prms_host.num_keypoints*prms->prms_host.points_per_line; j++)
                {
#ifndef PRINT_ALL_KP
                    if((((vx_uint16*)out_tensor_target_ptr)[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j] >= prms->prms_host.width) ||
                       (((vx_uint16*)out_tensor_target_ptr)[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j + 1] >= prms->prms_host.height) ||
                       (((vx_uint16*)out_tensor_target_ptr)[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j] <= 0)||
                       (((vx_uint16*)out_tensor_target_ptr)[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j+1] <= 0)
                      )
#endif
                      {
                        printf("(%d,%d,%d) ",j,((vx_uint16*)out_tensor_target_ptr)[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j],
                            ((vx_uint16*)out_tensor_target_ptr)[(i*prms->prms_host.num_keypoints*prms->prms_host.points_per_line*2) + 2*j+1]);
                      }
                }
                printf("\n");
            }
        }
#endif

        tivxMemBufferUnmap(in_tensor_target_ptr, in_points_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(out_tensor_target_ptr, out_points_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(out_valid_tensor_target_ptr, out_valid_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(table_target_ptr, table_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(rev_table_target_ptr, rev_table_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

    }

    return (status);
}
void tivxAddTargetKernelODPostProc()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name);
    }

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_ODPostProc_kernel = tivxAddTargetKernelByName
                                (
                                  TIVX_KERNEL_OD_POSTPROCESS_NAME,
                                  target_name,
                                  tivxKernelODPostProcProcess,
                                  tivxKernelODPostProcCreate,
                                  tivxKernelODPostProcDelete,
                                  NULL,
                                  NULL
                                );
    }
}

void tivxRemoveTargetKernelODPostProc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_ODPostProc_kernel);
    if (status == VX_SUCCESS)
    {
        vx_ODPostProc_kernel = NULL;
    }
}


static vx_int32 lineInterp(vx_int16 startPoint[2], vx_int16 endPoint[2], vx_int16 *outPoints, vx_int32 total_points)
{

    int32_t x_0 = startPoint[0];
    int32_t y_0 = startPoint[1];

    int32_t x_1 = endPoint[0];
    int32_t y_1 = endPoint[1];
    int32_t i;
    float y_inc,x_inc,x,y;
    int32_t dx,dy;
    int32_t steps;

    dx = x_1 - x_0;
    dy = y_1 - y_0;

    if(abs(dx) > abs(dy))
    {
        /*increment in x direction and have all the x axis pixels filled up*/
        steps = abs(dx);
    }
    else
    {
        steps = abs(dy);
    }

    if(steps > total_points)
    {
        steps = total_points;
    }

    x_inc = ((float)dx)/(float)(steps);
    y_inc = ((float)dy)/(float)(steps);

    x = x_0;
    y = y_0;

    for(i = 0; i < steps; i++)
    {
        outPoints[2*i + 0] = (uint16_t)x;
        outPoints[2*i + 1] = (uint16_t)y;

        x = x + x_inc;
        y = y + y_inc;
    }

    for(; i < total_points; i++)
    {
        outPoints[2*i + 0] = outPoints[2*(i-1) + 0];
        outPoints[2*i + 1] = outPoints[2*(i-1) + 1];
    }

    return(steps);
}
