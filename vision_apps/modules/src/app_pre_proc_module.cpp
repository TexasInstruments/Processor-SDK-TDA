/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include "app_pre_proc_module.hpp"
#include <string.h>   /* memset */

/* ========================================================================== */
/*                  PreProcObj class method implementations                   */
/* ========================================================================== */

PreProcObj::PreProcObj() { setDefaults(); }

void PreProcObj::setDefaults()
{
    for (int i = 0; i < 6; i++)
    {
        this->input_arr[i] = NULL;
    }
    this->node                = NULL;
    this->config              = NULL;
    memset(&this->tidl_io_buf_desc, 0, sizeof(this->tidl_io_buf_desc));
    memset(&this->params, 0, sizeof(this->params));
    this->num_input_tensors   = 0;
    this->num_output_tensors  = 0;
    for (int i = 0; i < APP_PRE_PROC_MAX_TENSORS; i++)
    {
        this->output_tensor_arr[i] = NULL;
    }
    this->graph_parameter_index = 0;
    this->num_ch = NUM_CH;
    this->num_images = 1;  /* Default to 1 image (OD mode) */
}

/* ========================================================================== */
/*                  Private helper methods                                    */
/* ========================================================================== */

vx_size PreProcObj::getTensorDataType(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if(tidl_type == TIDL_SignedChar)
    { 
        openvx_type = VX_TYPE_INT8;
    }
    else if(tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    } 
    else if(tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if(tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if(tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if(tidl_type == TIDL_SinglePrecFloat)
    {
        openvx_type = VX_TYPE_FLOAT32;
    }

    return openvx_type;
}

vx_status PreProcObj::updateFromTIDLConfig(vx_context context, sTIDL_IOBufDesc_t *ioBufDesc)
{
    vx_status status = VX_SUCCESS;
    vx_tensor output_tensors[APP_MAX_TENSORS];

    /* Store ioBufDesc information needed for later */
    this->num_input_tensors = ioBufDesc->numInputBuf;
    this->num_output_tensors = ioBufDesc->numOutputBuf;

    this->createOutputTensors(context, ioBufDesc, output_tensors);

    /* Pre-proc node only outputs ONE tensor (the first TIDL input - image batch).
     * Additional TIDL inputs (e.g., XYCoordinate for BEV) come from elsewhere. */
    this->output_tensor_arr[0] = vxCreateObjectArray(context, (vx_reference)output_tensors[0], this->num_ch);
    vxReleaseTensor(&output_tensors[0]);

    return status;
}

/* ================================================================ */
/*  Uniform-signature public API                                    */
/* ================================================================ */

/* ---- Virtual dispatch methods ---- */

vx_status PreProcObj::initModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    vx_context context = appGraph->context;

    sTIDL_IOBufDesc_t *ioBufDesc = &this->tidl_io_buf_desc;
    if (ioBufDesc->numInputBuf == 0)
    {
        printf("app_pre_proc: ERROR: tidl_io_buf_desc not set\n");
        return VX_FAILURE;
    }

    /* Update params and create tensors from TIDL config */
    status = this->updateFromTIDLConfig(context, ioBufDesc);
    if (status != VX_SUCCESS)
    {
        printf("app_pre_proc: ERROR: updateFromTIDLConfig failed\n");
        return status;
    }

    /* Initialize preproc configuration */
    memset(&this->params, 0, sizeof(tivxDLPreProcArmv8Params));
    uint32_t i = 0;

    this->params.skip_flag = 0;
    for (i = 0; i < 3; i++)
    {
        this->params.scale[i] = 1;
        this->params.mean[i] = 0;
    }
    this->params.channel_order = 0; //0-NCHW

    /* Set number of images from module config (1 for OD, 6 for BEV) */
    this->params.num_images = this->num_images;

    /* Set channel pitch from TIDL IO buffer descriptor to match stride TIDL expects */
    this->params.channel_pitch = (vx_uint32)ioBufDesc->inChannelPitch[0];

    vx_size data_type = this->getTensorDataType(ioBufDesc->inElementType[0]);
    if ((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8))
    {
        if (ioBufDesc->inDataFormat[0] == 1)
            this->params.tensor_format = 0; //RGB
        else
            this->params.tensor_format = 1; //BGR
    }
    else if ((data_type == VX_TYPE_INT16) || (data_type == VX_TYPE_UINT16))
    {
        if (ioBufDesc->inDataFormat[0] == 1)
            this->params.tensor_format = 0; //RGB
        else
            this->params.tensor_format = 1; //BGR
    }

    for (i = 0; i < 4; i++)
    {
        this->params.crop[i] = 0;
    }

    this->config = vxCreateUserDataObject(context, "PreProcConfig", sizeof(tivxDLPreProcArmv8Params), &this->params);
    status = vxGetStatus((vx_reference)this->config);
    return status;
}

vx_status PreProcObj::deinitModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vxReleaseUserDataObject(&this->config);
    /* Pre-proc only creates output_tensor_arr[0] */
    vxReleaseObjectArray(&this->output_tensor_arr[0]);
    return VX_SUCCESS;
}

vx_status PreProcObj::deleteNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
    return VX_SUCCESS;
}

vx_status PreProcObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph = appGraph->graph;
    vx_status status = VX_SUCCESS;

    /* Get input images from each input array */
    vx_image input_images[6];
    for (int i = 0; i < this->num_images; i++)
    {
        input_images[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)this->input_arr[i], 0);
    }

    vx_tensor output = (vx_tensor)vxGetObjectArrayItem((vx_object_array)this->output_tensor_arr[0], 0);

    this->node = tivxDLPreProcArmv8Node(graph, this->config, input_images, this->num_images, output);

    APP_ASSERT_VALID_REF(this->node);
    status = vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

    vxSetReferenceName((vx_reference)this->node, "PreProcNode");

    if (this->num_ch > 1 || this->num_images == 1)
    {
        vx_uint32 actual_num_params = 0;
        vxQueryNode(this->node, VX_NODE_PARAMETERS, &actual_num_params, sizeof(actual_num_params));

        vx_bool replicate[8] = {vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e};
        if (actual_num_params <= 8) {
            replicate[actual_num_params - 1] = vx_true_e;  /* Ensure output is replicated */
            vxReplicateNode(graph, this->node, replicate, actual_num_params);
        }
    }

    /* Release image references */
    for (int i = 0; i < this->num_images; i++)
    {
        vxReleaseImage(&input_images[i]);
    }
    vxReleaseTensor(&output);

    return(status);
}

vx_status PreProcObj::writeOutput(char* file_name)
{
    vx_status status = VX_SUCCESS;

    vx_tensor output;
    vx_size numCh;
    vx_int32 ch;

    vxQueryObjectArray((vx_object_array)this->output_tensor_arr[0], VX_OBJECT_ARRAY_NUMITEMS, &numCh, sizeof(vx_size));

    for(ch = 0; ch < (vx_int32)numCh; ch++)
    {
        vx_size num_dims;
        void *data_ptr;
        vx_map_id map_id;

        vx_size    start[APP_MAX_TENSOR_DIMS];
        vx_size    tensor_strides[APP_MAX_TENSOR_DIMS];
        vx_size    tensor_sizes[APP_MAX_TENSOR_DIMS];
        vx_char new_name[APP_MAX_FILE_PATH];

        /* Write Y plane */
        output  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)this->output_tensor_arr[0], ch);

        vxQueryTensor(output, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));

        if(num_dims != 3)
        {
            printf("Number of dims are != 3! exiting.. \n");
            break;
        }

        vxQueryTensor(output, VX_TENSOR_DIMS, tensor_sizes, 3 * sizeof(vx_size));

        start[0] = start[1] = start[2] = 0;

        tensor_strides[0] = 1;
        tensor_strides[1] = tensor_strides[0];
        tensor_strides[2] = tensor_strides[1] * tensor_strides[1];

        status = tivxMapTensorPatch(output, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        snprintf(new_name, APP_MAX_FILE_PATH, "%s_%dx%d_ch%d.yuv", file_name, (uint32_t)tensor_sizes[0], (uint32_t)tensor_sizes[1], ch);

        FILE *fp = fopen(new_name, "wb");
        if(NULL == fp)
        {
            printf("Unable to open file %s \n", new_name);
            break;
        }

        if(VX_SUCCESS == status)
        {
            fwrite(data_ptr, 1, tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2], fp);

            tivxUnmapTensorPatch(output, map_id);
        }
        vxReleaseTensor(&output);

        fclose(fp);
    }

  return(status);
}

void PreProcObj::createOutputTensors(vx_context context, sTIDL_IOBufDesc_t *ioBufDesc, vx_tensor output_tensors[])
{
    vx_size input_sizes[APP_MAX_TENSOR_DIMS];
    vx_uint32 id;

    for (id = 0; id < (vx_uint32)ioBufDesc->numInputBuf; id++)
    {
        /* Standard 3D tensor dimensions: [W, H, C] */
        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = ioBufDesc->inNumChannels[id];

        vx_enum data_type = get_vx_tensor_datatype(ioBufDesc->inElementType[id]);

        /* Check if batch mode (BEV with multiple images) */
        if (ioBufDesc->inNumBatches[id] == 1)
        {
            /* OD mode: 3D tensor [W, H, C] */
            output_tensors[id] = vxCreateTensor(context, 3, input_sizes, data_type, 0);
        }
        else
        {
            /* BEV mode: 4D tensor [W, H, packed_C, N] */
            input_sizes[2] = (ioBufDesc->inNumChannels[id] + ioBufDesc->inPadCh[id] + 1) *
                             ioBufDesc->inDIM1[id] * ioBufDesc->inDIM2[id];
            input_sizes[3] = ioBufDesc->inNumBatches[id];
            output_tensors[id] = vxCreateTensor(context, 4, input_sizes, data_type, 0);
        }
    }
}
