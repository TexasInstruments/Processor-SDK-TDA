/*
 *
 * Copyright (c) 2017-2024 Texas Instruments Incorporated
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
#include "app_display_module.h"

vx_status app_init_display(vx_context context, DisplayObj *displayObj, char *objName)
{
    vx_status status = VX_SUCCESS;

    if (displayObj->display_option == 1)
    {
#if !(defined(SOC_AM62A) && defined(QNX))
        if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1))
        {
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }
#endif

        if(VX_SUCCESS == status)
        {
            memset(&displayObj->disp_params, 0, sizeof(tivx_display_params_t));

            displayObj->disp_params.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;//TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE;
            displayObj->disp_params.pipeId = 0; /* pipe ID = 2 */
            displayObj->disp_params.outWidth = DISPLAY_WIDTH;
            displayObj->disp_params.outHeight = DISPLAY_HEIGHT;
            displayObj->disp_params.posX = (1920-DISPLAY_WIDTH)/2;
            displayObj->disp_params.posY = (1080-DISPLAY_HEIGHT)/2;

            displayObj->disp_params_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &displayObj->disp_params);
            status = vxGetStatus((vx_reference)displayObj->disp_params_obj);

            if(VX_SUCCESS == status)
            {
                vxSetReferenceName((vx_reference)displayObj->disp_params_obj, "display_node_disp_params_obj");
            }
        }
    }

    return status;
}

void app_deinit_display(DisplayObj *displayObj)
{
#if !(defined(SOC_AM62A) && defined(QNX))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (displayObj->display_option == 1))
#else
    if (displayObj->display_option == 1)
#endif
    {
        vxReleaseUserDataObject(&displayObj->disp_params_obj);
    }
}

void app_delete_display(DisplayObj *displayObj)
{
#if !(defined(SOC_AM62A) && defined(QNX))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (displayObj->display_option == 1))
#else
    if (displayObj->display_option == 1)
#endif
    {
        vxReleaseNode(&displayObj->disp_node);
    }
}

vx_status app_create_graph_display(vx_graph graph, DisplayObj *displayObj, vx_image disp_image)
{
    vx_status status = VX_SUCCESS;
#if !(defined(SOC_AM62A) && defined(QNX))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (displayObj->display_option == 1))
#else
    if (displayObj->display_option == 1)
#endif
    {
#if !(defined(SOC_AM62A) && defined(QNX))
        displayObj->disp_node = tivxDisplayNode(graph, displayObj->disp_params_obj, disp_image);
        status = vxGetStatus((vx_reference)displayObj->disp_node);
#endif
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)displayObj->disp_node, "DisplayNode");
#if !(defined(SOC_AM62A) && defined(QNX))
            vxSetNodeTarget(displayObj->disp_node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1);
#endif
        }
        else
        {
            printf("[DISPLAY-MODULE] Unable to create display node!\n");
        }
    }
    return status;
}
