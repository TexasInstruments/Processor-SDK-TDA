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

#include "app_obj_arr_split_module.h"

static vx_status create_output_image(vx_context context, ObjArrSplitObj *objArrSplitObj)
{
    vx_status status = VX_SUCCESS;
    uint32_t total_output_elements;
    vx_size total_input_elements = 0;

    if ( (objArrSplitObj->num_outputs < 2) || (objArrSplitObj->num_outputs > 4))
    {
        printf("[OBJ-ARR-SPLIT-MODULE] objArrSplitObj->num_outputs value is invalid.  Must be between 2 and 4\n");
        status = VX_FAILURE;
    }

    if (NULL != objArrSplitObj->input_arr)
    {
        vxQueryObjectArray(objArrSplitObj->input_arr, VX_OBJECT_ARRAY_NUMITEMS, &total_input_elements, sizeof(total_input_elements));
    }

    total_output_elements = objArrSplitObj->output0_num_elements + objArrSplitObj->output1_num_elements + objArrSplitObj->output2_num_elements + objArrSplitObj->output3_num_elements;

    if (total_input_elements != total_output_elements)
    {
        printf("[OBJ-ARR-SPLIT-MODULE] Sum of all output num_elements (%d) does not equal the number of elements in objArrSplitObj->input_arr (%zd)\n", total_output_elements, total_input_elements);
        status = VX_FAILURE;
    }

    if(status == VX_SUCCESS)
    {
        if (NULL != objArrSplitObj->input_arr)
        {
            vx_reference exemplar;

            exemplar = vxGetObjectArrayItem((vx_object_array)objArrSplitObj->input_arr, 0);

            if (NULL != exemplar)
            {
                objArrSplitObj->output0_arr = vxCreateObjectArray(context, exemplar, objArrSplitObj->output0_num_elements);
                status = vxGetStatus((vx_reference)objArrSplitObj->output0_arr);
                if(status != VX_SUCCESS)
                {
                    printf("[OBJ-ARR-SPLIT-MODULE] Unable to create output0_arr\n");
                }
                else
                {
                    objArrSplitObj->output1_arr = vxCreateObjectArray(context, exemplar, objArrSplitObj->output1_num_elements);
                    status = vxGetStatus((vx_reference)objArrSplitObj->output1_arr);
                    if(status != VX_SUCCESS)
                    {
                        printf("[OBJ-ARR-SPLIT-MODULE] Unable to create output1_arr\n");
                    }
                    else
                    {
                        if (objArrSplitObj->num_outputs > 2)
                        {
                            objArrSplitObj->output2_arr = vxCreateObjectArray(context, exemplar, objArrSplitObj->output2_num_elements);
                            status = vxGetStatus((vx_reference)objArrSplitObj->output2_arr);
                            if(status != VX_SUCCESS)
                            {
                                printf("[OBJ-ARR-SPLIT-MODULE] Unable to create output2_arr\n");
                            }
                            else
                            {
                                if (objArrSplitObj->num_outputs > 3)
                                {
                                    objArrSplitObj->output3_arr = vxCreateObjectArray(context, exemplar, objArrSplitObj->output3_num_elements);
                                    status = vxGetStatus((vx_reference)objArrSplitObj->output3_arr);
                                    if(status != VX_SUCCESS)
                                    {
                                        printf("[OBJ-ARR-SPLIT-MODULE] Unable to create output3_arr\n");
                                    }
                                }
                                else
                                {
                                    objArrSplitObj->output3_arr = NULL;
                                }
                            }
                        }
                        else
                        {
                            objArrSplitObj->output2_arr = NULL;
                            objArrSplitObj->output3_arr = NULL;
                        }
                    }
                }

                if(status == VX_SUCCESS)
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    status = vxReleaseReference(&exemplar);

                    snprintf(name, VX_MAX_REFERENCE_NAME, "obj_arr_split_node_output0_arr");

                    vxSetReferenceName((vx_reference)objArrSplitObj->output0_arr, name);

                    snprintf(name, VX_MAX_REFERENCE_NAME, "obj_arr_split_node_output1_arr");

                    vxSetReferenceName((vx_reference)objArrSplitObj->output1_arr, name);

                    if (NULL != objArrSplitObj->output2_arr)
                    {
                        snprintf(name, VX_MAX_REFERENCE_NAME, "obj_arr_split_node_output2_arr");

                        vxSetReferenceName((vx_reference)objArrSplitObj->output2_arr, name);
                    }

                    if (NULL != objArrSplitObj->output3_arr)
                    {
                        snprintf(name, VX_MAX_REFERENCE_NAME, "obj_arr_split_node_output3_arr");

                        vxSetReferenceName((vx_reference)objArrSplitObj->output3_arr, name);
                    }
                }
            }
            else
            {
                printf("[OBJ-ARR-SPLIT-MODULE] objArrSplitObj->input_arr exemplar is NULL\n");
                status = VX_FAILURE;
            }
        }
        else
        {
            printf("[OBJ-ARR-SPLIT-MODULE] objArrSplitObj->input_arr is NULL\n");
            status = VX_FAILURE;
        }
    }

    return status;
}

vx_status app_init_obj_arr_split(vx_context context, ObjArrSplitObj *objArrSplitObj, char *objName)
{
    vx_status status = VX_SUCCESS;

    status = create_output_image(context, objArrSplitObj);

    return status;
}

void app_deinit_obj_arr_split(ObjArrSplitObj *objArrSplitObj)
{
    vxReleaseObjectArray(&objArrSplitObj->output0_arr);
    vxReleaseObjectArray(&objArrSplitObj->output1_arr);

    if (NULL != objArrSplitObj->output2_arr)
    {
        vxReleaseObjectArray(&objArrSplitObj->output2_arr);
    }

    if (NULL != objArrSplitObj->output3_arr)
    {
        vxReleaseObjectArray(&objArrSplitObj->output3_arr);
    }

    return;
}

void app_delete_obj_arr_split(ObjArrSplitObj *objArrSplitObj)
{
    if(objArrSplitObj->node != NULL)
    {
        vxReleaseNode(&objArrSplitObj->node);
    }

    return;
}

vx_status app_create_graph_obj_arr_split(vx_graph graph, ObjArrSplitObj *objArrSplitObj)
{

    vx_status status = VX_SUCCESS;

    objArrSplitObj->node = tivxObjArraySplitNode(graph,
                                           objArrSplitObj->input_arr,
                                           objArrSplitObj->output0_arr,
                                           objArrSplitObj->output1_arr,
                                           objArrSplitObj->output2_arr,
                                           objArrSplitObj->output3_arr);

    status = vxGetStatus((vx_reference)objArrSplitObj->node);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)objArrSplitObj->node, "obj_arr_split_node");
        vxSetNodeTarget(objArrSplitObj->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    }
    else
    {
        printf("[OBJ-ARR-MODULE] Unable to create obj array node! \n");
    }

    return (status);
}
