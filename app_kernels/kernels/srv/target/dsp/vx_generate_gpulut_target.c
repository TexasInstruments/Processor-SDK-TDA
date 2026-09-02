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

#include "TI/tivx.h"
#include "TI/tivx_srv.h"
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_generate_gpulut.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "core_generate_gpulut.h"
#ifdef HOST_EMULATION
#include "C6xSimulator.h"
#endif

static tivx_target_kernel vx_generate_gpulut_target_kernel = NULL;

static vx_status VX_CALLBACK tivxGenerateGpulutProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGenerateGpulutCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGenerateGpulutDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxGenerateGpulutProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxGenerateGpulutParams *prms = NULL;
    tivx_obj_desc_user_data_object_t *in_configuration_desc;
    tivx_obj_desc_user_data_object_t *in_ldclut_desc;
    tivx_obj_desc_user_data_object_t *in_calmat_scaled_desc;
    tivx_obj_desc_array_t *in_lut3dxyz_desc;
    tivx_obj_desc_array_t *out_gpulut3d_desc;

    svGpuLutGen_t                    *in_params;
    svLdcLut_t                       *in_ldclut; 
    svACCalmatStruct_t               *in_calmat_scaled;
    vx_float32                       *in_lut3dxyz;
    vx_uint16                        *out_gpulut;

    if ( (num_params != TIVX_KERNEL_GENERATE_GPULUT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        uint32_t size;
        in_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX];
        in_ldclut_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX];
        in_calmat_scaled_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX];
        in_lut3dxyz_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX];
        out_gpulut3d_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxGenerateGpulutParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {

        void *in_configuration_target_ptr;
        void *in_ldclut_target_ptr;
        void *in_calmat_scaled_target_ptr;
        void *in_lut3dxyz_target_ptr;
        void *out_gpulut3d_target_ptr;

        in_configuration_target_ptr = tivxMemShared2TargetPtr(&in_configuration_desc->mem_ptr);
        tivxMemBufferMap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_ldclut_target_ptr = tivxMemShared2TargetPtr(&in_ldclut_desc->mem_ptr);
        tivxMemBufferMap(in_ldclut_target_ptr,
           in_ldclut_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_calmat_scaled_target_ptr = tivxMemShared2TargetPtr(&in_calmat_scaled_desc->mem_ptr);
        tivxMemBufferMap(in_calmat_scaled_target_ptr,
           in_calmat_scaled_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        in_lut3dxyz_target_ptr = tivxMemShared2TargetPtr(&in_lut3dxyz_desc->mem_ptr);
        tivxMemBufferMap(in_lut3dxyz_target_ptr,
           in_lut3dxyz_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        out_gpulut3d_target_ptr = tivxMemShared2TargetPtr(&out_gpulut3d_desc->mem_ptr);
        tivxMemBufferMap(out_gpulut3d_target_ptr,
           out_gpulut3d_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY);



        {

            /* call kernel processing function */
            int32_t count;

            in_params             = (svGpuLutGen_t *)in_configuration_target_ptr;
            in_ldclut             = (svLdcLut_t      *)in_ldclut_target_ptr;
            in_calmat_scaled      = (svACCalmatStruct_t*) in_calmat_scaled_target_ptr; 
            in_lut3dxyz           = (vx_float32 *) in_lut3dxyz_target_ptr; 

            out_gpulut            = (vx_uint16 *) out_gpulut3d_target_ptr; 

            svGenerate_3D_GPULUT(in_params, in_ldclut, prms, in_calmat_scaled, in_lut3dxyz, out_gpulut, &count);

            out_gpulut3d_desc->num_items = count;
        }
        tivxMemBufferUnmap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_ldclut_target_ptr,
           in_ldclut_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_calmat_scaled_target_ptr,
           in_calmat_scaled_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(in_lut3dxyz_target_ptr,
           in_lut3dxyz_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(out_gpulut3d_target_ptr,
           out_gpulut3d_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);



    }

    return status;
}

static vx_status VX_CALLBACK tivxGenerateGpulutCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxGenerateGpulutParams *prms = NULL;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */
    if ( (num_params != TIVX_KERNEL_GENERATE_GPULUT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {

        tivx_obj_desc_user_data_object_t *in_configuration_desc;
        svGpuLutGen_t                    *in_params;
        void *in_configuration_target_ptr;

        in_configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX];
        in_configuration_target_ptr = tivxMemShared2TargetPtr(&in_configuration_desc->mem_ptr);
        tivxMemBufferMap(in_configuration_target_ptr,
           in_configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);
       in_params             = (svGpuLutGen_t *)in_configuration_target_ptr;

        prms = tivxMemAlloc(sizeof(tivxGenerateGpulutParams), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            /* < DEVELOPER_TODO: Verify correct amount of memory is allocated > */
            prms->buf_GLUT3d_undist_size = (1 + (in_params->SVOutDisplayWidth/(2 * in_params->subsampleratio))) *4 *2*sizeof(int16_t);
            prms->buf_GLUT3d_undist_ptr = tivxMemAlloc(prms->buf_GLUT3d_undist_size, TIVX_MEM_EXTERNAL);

            if (NULL == prms->buf_GLUT3d_undist_ptr)
            {
                status = VX_ERROR_NO_MEMORY;
                VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
            }
            else
            {
                /* < DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory > */
                memset(prms->buf_GLUT3d_undist_ptr, 0, prms->buf_GLUT3d_undist_size);
            }


        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxGenerateGpulutParams));
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }

        tivxMemBufferUnmap(in_configuration_target_ptr, in_configuration_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    }

    return status;
}

static vx_status VX_CALLBACK tivxGenerateGpulutDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxGenerateGpulutParams *prms = NULL;
    uint32_t size;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */
    if ( (num_params != TIVX_KERNEL_GENERATE_GPULUT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((NULL != prms) &&
            (sizeof(tivxGenerateGpulutParams) == size))
        {
            if (NULL != prms->buf_GLUT3d_undist_ptr)
            {
                tivxMemFree(prms->buf_GLUT3d_undist_ptr, prms->buf_GLUT3d_undist_size, TIVX_MEM_EXTERNAL);
            }

            tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

void tivxAddTargetKernelGenerateGpulut(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_generate_gpulut_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_GENERATE_GPULUT_NAME,
                            target_name,
                            tivxGenerateGpulutProcess,
                            tivxGenerateGpulutCreate,
                            tivxGenerateGpulutDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelGenerateGpulut(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_generate_gpulut_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_generate_gpulut_target_kernel = NULL;
    }
}




