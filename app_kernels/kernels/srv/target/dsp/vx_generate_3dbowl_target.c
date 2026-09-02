/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include <stdio.h>
#include <math.h>
#include "TI/tivx.h"
#include "TI/tivx_srv.h"
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_generate_3dbowl.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "srv_common.h"
#include "core_generate_3dbowl.h"

#ifdef HOST_EMULATION
#include "C6xSimulator.h"
#endif


static tivx_target_kernel vx_generate_3dbowl_target_kernel = NULL;

static vx_status VX_CALLBACK tivxGenerate3DbowlProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGenerate3DbowlCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGenerate3DbowlDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxGenerate3DbowlProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *in_configuration_desc;
    tivx_obj_desc_user_data_object_t *in_calmat_desc;
    tivx_obj_desc_user_data_object_t *in_offset_desc;
    tivx_obj_desc_array_t *out_lut3dxyz_desc;
    tivx_obj_desc_user_data_object_t *out_calmat_scaled_desc;
    svGpuLutGen_t       *in_params; 
    vx_float32               *out_lut3d;
    svACCalmatStruct_t               *in_calmat;
    svGeometric_t                    *in_offset_params;
    svACCalmatStruct_t               *out_calmat_scaled;

    if ( (num_params != TIVX_KERNEL_GENERATE_3DBOWL_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_IN_CALMAT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_IN_OFFSET_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_OUT_LUT3DXYZ_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_OUT_CALMAT_SCALED_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        in_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_IN_CONFIGURATION_IDX];
        in_calmat_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_IN_CALMAT_IDX];
        in_offset_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_IN_OFFSET_IDX];
        out_lut3dxyz_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_OUT_LUT3DXYZ_IDX];
        out_calmat_scaled_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_3DBOWL_OUT_CALMAT_SCALED_IDX];

    }

    if(VX_SUCCESS == status)
    {

        void *in_configuration_target_ptr;
        void *in_calmat_target_ptr;
        void *in_offset_target_ptr;
        void *out_lut3dxyz_target_ptr;
        void *out_calmat_scaled_target_ptr;

        in_configuration_target_ptr = tivxMemShared2TargetPtr(&in_configuration_desc->mem_ptr);
        tivxMemBufferMap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_calmat_target_ptr = tivxMemShared2TargetPtr(&in_calmat_desc->mem_ptr);
        tivxMemBufferMap(in_calmat_target_ptr,
           in_calmat_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_offset_target_ptr = tivxMemShared2TargetPtr(&in_offset_desc->mem_ptr);
        tivxMemBufferMap(in_offset_target_ptr,
           in_offset_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        out_lut3dxyz_target_ptr = tivxMemShared2TargetPtr(&out_lut3dxyz_desc->mem_ptr);
        tivxMemBufferMap(out_lut3dxyz_target_ptr,
           out_lut3dxyz_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY);

        out_calmat_scaled_target_ptr = tivxMemShared2TargetPtr(&out_calmat_scaled_desc->mem_ptr);
        tivxMemBufferMap(out_calmat_scaled_target_ptr,
           out_calmat_scaled_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY);


        {   /* Start core process block */
            vx_int16 i,j;
            /* 12 * number of cameras */
            vx_float32 calmat_scaled[12*4];




            in_params  = (svGpuLutGen_t *)in_configuration_target_ptr; 
            out_lut3d  = (vx_float32 *) out_lut3dxyz_target_ptr; 
            in_calmat         = (svACCalmatStruct_t *) in_calmat_target_ptr; 
            in_offset_params  = (svGeometric_t *) in_offset_target_ptr; 
            out_calmat_scaled = (svACCalmatStruct_t*) out_calmat_scaled_target_ptr; 

            //shift = 31U - _lmbd(1U, in_params->subsampleratio);

            /* For each camera */
            for(j=0; j<4; j++)
            {
                /* For each value : First 9 use R_SHIFT */
                for(i=0; i<9; i++)
                {
                    calmat_scaled[j*12+i] = (vx_float32) (in_calmat->outcalmat[j*12+i]) * POW_2_CALMAT_R_SHIFT_INV;
                    out_calmat_scaled->scaled_outcalmat[j*12+i]   = calmat_scaled[j*12+i];
                }
                /* For each value : Final 3 use T_SHIFT */
                for(; i<12; i++)
                {
                    calmat_scaled[j*12+i] = (vx_float32)(in_calmat->outcalmat[j*12+i]) * POW_2_CALMAT_T_SHIFT_INV;
                    out_calmat_scaled->scaled_outcalmat[j*12+i]   = calmat_scaled[j*12+i];
                }
            }
       
       
            /*
            view id 1 is Dominant Image
            view id 2 is Secondary Image
       
       
                                       view0
                               |-----------------------|
                               | Quadrant1 | Quadrant2 |
                       view3   |-----------------------| view1
                               | Quadrant4|  Quadrant3 |
                               |-----------------------|
                                       view2
       
       
            GALUT format:
       
            For each output grid vertex the following are stored in the array (u16 for each):
       
            0 - O/p X co-ordinate
            1 - O/p Y co-ordinate
            2 - O/p Z co-ordinate
            3 - Image 1 x co-ordinate (Dominant view)
            4 - Image 1 y co-ordinate (Dominant view)
            5 - Image 2 x co-ordinate
            6 - Image 2 y co-ordinate
       
            */
       
            /* Call function to populate the sv->GAlignLUT3D_XYZ array using
             * some algorithm (dependent on object distance potentially) */

            svGenerate_3D_Bowl(in_params, in_offset_params, calmat_scaled,out_lut3d);


            /* END Core Process */
        }
         
        tivxMemBufferUnmap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_calmat_target_ptr,
           in_calmat_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_offset_target_ptr,
           in_offset_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(out_lut3dxyz_target_ptr,
           out_lut3dxyz_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxMemBufferUnmap(out_calmat_scaled_target_ptr,
           out_calmat_scaled_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);



    }

    return status;
}

static vx_status VX_CALLBACK tivxGenerate3DbowlCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */

    return status;
}

static vx_status VX_CALLBACK tivxGenerate3DbowlDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */

    return status;
}

void tivxAddTargetKernelGenerate3Dbowl(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_generate_3dbowl_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_GENERATE_3DBOWL_NAME,
                            target_name,
                            tivxGenerate3DbowlProcess,
                            tivxGenerate3DbowlCreate,
                            tivxGenerate3DbowlDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelGenerate3Dbowl(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_generate_3dbowl_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_generate_3dbowl_target_kernel = NULL;
    }
}




