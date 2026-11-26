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
#include "app_display_module.hpp"
#include <string.h>   /* memset */

/* ========================================================================== */
/*                  DisplayObj class method implementations                   */
/* ========================================================================== */

DisplayObj::DisplayObj() { setDefaults(); }

void DisplayObj::setDefaults()
{
    this->input_image           = NULL;
    this->disp_node             = NULL;
    this->disp_params_obj       = NULL;
    memset(&this->disp_params, 0, sizeof(this->disp_params));
    this->display_option        = 0;
    this->out_width              = DISPLAY_WIDTH;
    this->out_height             = DISPLAY_HEIGHT;
    this->graph_parameter_index = 0;
    memset(this->objName, 0, sizeof(this->objName));
}


bool DisplayObj::isDisplayEnabled() const
{
    return (this->display_option == 1) && (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1));
}

vx_status DisplayObj::initModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_context context = appGraph->context;
    vx_status status = VX_SUCCESS;

    if (this->display_option == 1)
    {
        /* Requested but not available on this target build: fail loudly
         * rather than silently skipping, since that's a config/build mismatch. */
        if (vx_true_e != tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1))
        {
            return VX_FAILURE;
        }

        memset(&this->disp_params, 0, sizeof(tivx_display_params_t));

        this->disp_params.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        this->disp_params.pipeId = 0;
        this->disp_params.outWidth = this->out_width;
        this->disp_params.outHeight = this->out_height;
        this->disp_params.posX = (DISPLAY_SCREEN_WIDTH - this->out_width)/2;
        this->disp_params.posY = (DISPLAY_SCREEN_HEIGHT - this->out_height)/2;

        this->disp_params_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &this->disp_params);
        status = vxGetStatus((vx_reference)this->disp_params_obj);

        if(VX_SUCCESS == status)
        {
            vxSetReferenceName((vx_reference)this->disp_params_obj, "display_node_disp_params_obj");
        }
    }

    return status;
}

vx_status DisplayObj::deinitModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    if (this->isDisplayEnabled())
    {
        vxReleaseUserDataObject(&this->disp_params_obj);
    }
    return VX_SUCCESS;
}

vx_status DisplayObj::deleteNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    if (this->isDisplayEnabled())
    {
        vxReleaseNode(&this->disp_node);
    }
    return VX_SUCCESS;
}

vx_status DisplayObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph_vx = appGraph->graph;
    vx_status status = VX_SUCCESS;
    if (this->isDisplayEnabled())
    {
        this->disp_node = tivxDisplayNode(graph_vx, this->disp_params_obj, this->input_image);
        status = vxGetStatus((vx_reference)this->disp_node);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->disp_node, "DisplayNode");
            vxSetNodeTarget(this->disp_node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1);
        }
        else
        {
            printf("[DISPLAY-MODULE] Unable to create display node!\n");
        }
    }
    return status;
}
