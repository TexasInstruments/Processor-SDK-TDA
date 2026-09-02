/*
*
* Copyright (c) 2020-2026 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <TI/tivx_fileio.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"

#include <tivx_fileio_write_tensor_host.h>

#define WRITE_FILE_OUTPUT

typedef struct {
    tivxFileIOWriteCmd cmd;
    vx_uint32 frame_counter;
    vx_uint32 skip_counter;
    vx_uint32 ch_num;

} tivxWriteTensorParams;

static tivx_target_kernel vx_write_tensor_target_kernel = NULL;

static vx_int32 inst_id = 0;

static vx_status tivxKernelWriteTensorCmd
(
    tivxWriteTensorParams *prms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != usr_data_obj)
    {
        void  *target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        if (sizeof(tivxFileIOWriteCmd) ==  usr_data_obj->mem_size)
        {
            memcpy(&prms->cmd, target_ptr, sizeof(tivxFileIOWriteCmd));
        }
        prms->skip_counter = 0;
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxKernelWriteTensorCmd: Data Object is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelWriteTensorControl
(
    tivx_target_kernel_instance kernel,
    uint32_t node_cmd_id,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxWriteTensorParams *prms = NULL;

    if (NULL == obj_desc[TIVX_KERNEL_WRITE_TENSOR_INPUT_IDX])
    {
        printf("Input Tensor handle is NULL!\n");
        status = VX_FAILURE;
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxWriteTensorParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(status==VX_SUCCESS)
    {
        switch (node_cmd_id)
        {
            case TIVX_FILEIO_CMD_SET_FILE_WRITE:
            {
                tivxKernelWriteTensorCmd(prms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxKernelWriteTensorControl: Invalid Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelWriteTensorCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  vx_uint16 num_params,
  void *priv_arg
)
{
  tivxWriteTensorParams * prms = NULL;

  prms = tivxMemAlloc(sizeof(tivxWriteTensorParams), TIVX_MEM_EXTERNAL);
  if(prms == NULL)
  {
      printf("Unable to allcate memory for tivxWriteTensorParams");
      return VX_FAILURE;
  }
  prms->ch_num = inst_id;
  prms->frame_counter   = 0;
  prms->skip_counter    = 0;
  prms->cmd.start_frame = -1;
  prms->cmd.num_frames  = 0;
  prms->cmd.num_skip    = 0;

  inst_id++;
  tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxWriteTensorParams));

  return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelWriteTensorDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;


    if (NULL == obj_desc[TIVX_KERNEL_WRITE_TENSOR_INPUT_IDX])
    {
        printf("Input Image handle is NULL!\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxWriteTensorParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        tivxMemFree(prms, sizeof(tivxWriteTensorParams), TIVX_MEM_EXTERNAL);

    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelWriteTensorProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxWriteTensorParams *prms = NULL;

    if (NULL == obj_desc[TIVX_KERNEL_WRITE_TENSOR_INPUT_IDX])
    {
        printf("Input Image handle is NULL!\n");
        status = VX_FAILURE;
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxWriteTensorParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_tensor_t *in_tensor_desc;
        void* in_tensor_target_ptr = NULL;

        tivx_obj_desc_array_t* file_path_desc;
        void * file_path_target_ptr = NULL;

        tivx_obj_desc_array_t* file_prefix_desc;
        void * file_prefix_target_ptr = NULL;

        in_tensor_desc  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_WRITE_TENSOR_INPUT_IDX];
        in_tensor_target_ptr  = tivxMemShared2TargetPtr(&in_tensor_desc->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        file_path_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_WRITE_TENSOR_FILE_PATH_IDX];
        if(file_path_desc != NULL)
        {
            file_path_target_ptr = tivxMemShared2TargetPtr(&file_path_desc->mem_ptr);
            tivxMemBufferMap(file_path_target_ptr, file_path_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        }

        file_prefix_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_WRITE_TENSOR_FILE_PREFIX_IDX];
        if(file_prefix_desc != NULL)
        {
            file_prefix_target_ptr = tivxMemShared2TargetPtr(&file_prefix_desc->mem_ptr);
            tivxMemBufferMap(file_prefix_target_ptr, file_prefix_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        }

        /* This helps in synchonizing frames across nodes */
        if(prms->frame_counter == prms->cmd.start_frame)
            prms->skip_counter = 0;

        if((prms->frame_counter >= prms->cmd.start_frame) &&
           (prms->frame_counter < (prms->cmd.start_frame + (prms->cmd.num_frames * (prms->cmd.num_skip + 1)))) &&
           (prms->skip_counter == 0))
        {
            char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
            char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];
            char file_name[TIVX_FILEIO_FILE_PATH_LENGTH * 2];

            if(file_path_desc != NULL)
            {
                strcpy(file_path, file_path_target_ptr);
            }
            else
            {
                strcpy(file_path, ".");
            }

            if(file_prefix_desc != NULL)
            {
                strcpy(file_prefix, file_prefix_target_ptr);
            }
            else
            {
                strcpy(file_prefix, "");
            }

            sprintf(file_name, "%s/%s_%dx%dx%dx%d_ch_%d_%08d.bin", file_path, file_prefix,
                              in_tensor_desc->dimensions[0], in_tensor_desc->dimensions[1],
                              in_tensor_desc->dimensions[2], in_tensor_desc->dimensions[3],
                              prms->ch_num, prms->frame_counter);

            printf("Writing %s ..\n", file_name);

#ifdef WRITE_FILE_OUTPUT
            FILE *fp = fopen(file_name, "wb");
            if(fp == NULL)
            {
                printf("Unable to write file %s\n", file_name);
            }
            else
            {
                uint8_t *pData  = in_tensor_target_ptr;

                fwrite(pData, 1, in_tensor_desc->mem_size, fp);

                fflush(fp);
                fclose(fp);
            }
#endif
            printf("Done!\n");

        }
        prms->skip_counter++;
        prms->skip_counter = prms->skip_counter % (prms->cmd.num_skip + 1);
        prms->frame_counter++;

        tivxMemBufferUnmap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if(file_path_target_ptr != NULL)
        {
            tivxMemBufferUnmap(file_path_target_ptr, file_path_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        if(file_prefix_target_ptr != NULL)
        {
            tivxMemBufferUnmap(file_prefix_target_ptr, file_prefix_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

    }

    return (status);
}
void tivxAddTargetKernelWriteTensor()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_MPU_0))
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);

        vx_write_tensor_target_kernel = tivxAddTargetKernelByName
                                        (
                                          TIVX_KERNEL_WRITE_TENSOR_NAME,
                                          target_name,
                                          tivxKernelWriteTensorProcess,
                                          tivxKernelWriteTensorCreate,
                                          tivxKernelWriteTensorDelete,
                                          tivxKernelWriteTensorControl,
                                          NULL
                                        );
    }
}

void tivxRemoveTargetKernelWriteTensor()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_write_tensor_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_write_tensor_target_kernel = NULL;
    }
}
