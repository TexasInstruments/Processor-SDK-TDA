/*
*
* Copyright (c) 2017 - 2026 Texas Instruments Incorporated
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

/**
 * \file app_c7x_kernel_img_add_kernel.c User Kernel implementation for C7x image add function
 *
 *  This file shows a sample implementation of a user kernel function.
 *
 *  To implement a user kernel the below top level interface functions are implemented
 *  - app_c7x_kernel_img_add_register() : Registers user kernel to OpenVX context
 *       - The implementation of this function has slight variation depending on the kernel implemented as
 *         as OpenVX user kernel or TIOVX target kernel
 *  - app_c7x_kernel_img_add_unregister() : Un-Registers user kernel from OpenVX context
 *       - The implementation of this function is same for both OpenVX user kernel and TIOVX target kernel
 *  - app_c7x_kernel_img_add_kernel_node(): Using the user kernel name, creates a OpenVX node within a graph
 *       - The implementation of this function is same for both OpenVX user kernel and TIOVX target kernel
 *
 *  When working with user kernel or target kernel,
 *  - app_c7x_kernel_img_add_register() MUST be called after vxCreateContext() and
 *     before app_c7x_kernel_img_add_kernel_node().
 *  - app_c7x_kernel_img_add_unregister() MUST be called before vxReleaseContext() and
 *  - app_c7x_kernel_img_add_kernel_node() is called to insert the user kernel node into a OpenVX graph
 *
 *  When working with target kernel, additional the target side implementation is done in file
 *  \ref app_c7x_target_kernel_img_add.c
 *
 *  Follow the comments for the different functions in the file to understand how a user/target kernel is implemented.
 */

#include <stdio.h>
#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_host_utils.h"
#include "tivx_img_add.h"

static vx_status VX_CALLBACK tivxAddKernelImgAddValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

/** \brief Handle to the registered user kernel [static global] */
static vx_kernel vx_img_add_kernel = NULL;

/** \brief Kernel ID of the registered user kernel. Used to create a node for the kernel function [static global] */
static vx_enum vx_img_add_kernel_id = 0;


/**
 * \brief Add user/target kernel to OpenVX context
 *
 * \param context [in] OpenVX context into with the user kernel is added
 */
vx_status tivxAddKernelImgAdd(vx_context context)
{
    vx_kernel kernel = NULL;
    vx_status status;
    uint32_t index;

    /**
     * - Dynamically allocate a kernel ID and store it in  the global 'vx_img_add_kernel_id'
     *
     * This is used later to create the node with this kernel function
     * \code
     */
    status = vxAllocateUserKernelId(context, &vx_img_add_kernel_id);
    /** \endcode */
    if(status!=VX_SUCCESS)
    {
        printf(" ERROR: vxAllocateUserKernelId failed (%d)!!!\n", status);
    }
    if(status==VX_SUCCESS)
    {
        /**
         * - Register kernel to OpenVX context
         *
         * A kernel can be identified with its kernel ID (allocated above). \n
         * A kernel can also be identified with its unique kernel name string.
         * APP_C7X_KERNEL_IMG_ADD_NAME defined in
         * file app_c7x_kernel.h in this case.
         *
         * When calling vxAddUserKernel(), additional callbacks are registered.
         * For target kernel, the 'run' callback is set to NULL.
         * Typically 'init' and 'deinit' callbacks are also set to NULL.
         * 'validate' callback is typically set
         *
         * \code
         */
        kernel = vxAddUserKernel(
                    context,
                    TIVX_C7X_KERNEL_IMG_ADD_NAME,
                    vx_img_add_kernel_id,
                    NULL,
                    3, /* number of parameters objects for this user function */
                    tivxAddKernelImgAddValidate,
                    NULL,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
        /** \endcode */

        if ( status == VX_SUCCESS)
        {
            /**
             * - Add supported target's on which this target kernel can be run
             *
             * \code
             */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1);
            /** \endcode */
        }
    }
    /**
     * - Checking is kernel was added successfully to OpenVX context
     * \code
     */
    status = vxGetStatus((vx_reference)kernel);
    /** \endcode */
    if ( status == VX_SUCCESS)
    {
        /**
         * - Now define parameters for the kernel
         *
         *   When specifying the parameters, the below attributes of each parameter are specified,
         *   - parameter index in the function parameter list
         *   - the parameter direction: VX_INPUT or VX_OUTPUT
         *   - parameter data object type
         *   - paramater state: VX_PARAMETER_STATE_REQUIRED or VX_PARAMETER_STATE_OPTIONAL
         * \code
         */
        index = 0;

        status = vxAddParameterToKernel(kernel,
            index,
            VX_INPUT,
            VX_TYPE_IMAGE,
            VX_PARAMETER_STATE_REQUIRED
            );
        index++;
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        /** \endcode */
        /**
         * - After all parameters are defined, now the kernel is finalized, i.e it is ready for use.
         * \code
         */
        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /** \endcode */
        if( status != VX_SUCCESS)
        {
            printf(" ERROR: vxAddParameterToKernel, vxFinalizeKernel failed (%d)!!!\n", status);
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
        printf(" ERROR: vxAddUserKernel failed (%d)!!!\n", status);
    }

    if(status==VX_SUCCESS)
    {
        /**
         * - Set kernel handle to the global user kernel handle
         *
         * This global handle is used later to release the kernel when done with it
         * \code
         */
        vx_img_add_kernel = kernel;
        /** \endcode */
    }

    return status;
}


/**
 * \brief Remove user/target kernel from context
 *
 * \param context [in] OpenVX context from which the kernel will be removed
 */
vx_status tivxRemoveKernelAddImg(vx_context context)
{
    vx_status status;

    /**
     * - Remove user kernel from context and set the global 'vx_img_add_kernel' to NULL
     *
     * \code
     */
    status = vxRemoveKernel(vx_img_add_kernel);
    vx_img_add_kernel = NULL;
    /** \endcode */

    if(status!=VX_SUCCESS)
    {
        printf(" Unable to remove kernel (%d)!!!\n", status);
    }

    return status;
}

/**
 *  \brief User/target kernel validate function
 *
 *  This function gets called during vxGraphVerify.
 *  The node which will runs the kernel, parameter references
 *  are passed as input to this function.
 *  This function checks the parameters to make sure all attributes of the parameter are as expected.
 *  ex, data format checks, image width, height relations between input and output.
 *
 *  \param node [in] OpenVX node which will execute the kernel
 *  \param parameters [in] Parameters references for this kernel function
 *  \param num [in] Number of parameter references
 *  \param metas [in/out] Meta references update with attribute values
 *
 *  \return VX_SUCCESS if validate is successful, else appropiate error code
 */
static vx_status VX_CALLBACK tivxAddKernelImgAddValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[TIVX_C7X_IMG_ADD_MAX_PARAMS];
    vx_uint32 w[TIVX_C7X_IMG_ADD_MAX_PARAMS];
    vx_uint32 h[TIVX_C7X_IMG_ADD_MAX_PARAMS];
    vx_uint32 i;
    vx_df_image fmt[TIVX_C7X_IMG_ADD_MAX_PARAMS];

    if (num != TIVX_C7X_IMG_ADD_MAX_PARAMS)
    {
        printf(" ERROR: Number of parameters dont match !!!\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; i < TIVX_C7X_IMG_ADD_MAX_PARAMS; i ++)
    {
        img[i] = (vx_image)parameters[i];

        /* Check for NULL */
        if (NULL == img[i])
        {
            printf(" ERROR: Parameter %d is NULL !!!\n", i);
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_C7X_IMG_ADD_IN0_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_C7X_IMG_ADD_IN0_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_C7X_IMG_ADD_IN0_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_C7X_IMG_ADD_IN0_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_C7X_IMG_ADD_IN0_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_C7X_IMG_ADD_IN0_IMG_IDX],
            sizeof(vx_uint32));

        if(status!=VX_SUCCESS)
        {
            printf(" ERROR: Unable to query input image !!!\n");
        }
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_C7X_IMG_ADD_IN1_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_C7X_IMG_ADD_IN1_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_C7X_IMG_ADD_IN1_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_C7X_IMG_ADD_IN1_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_C7X_IMG_ADD_IN1_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_C7X_IMG_ADD_IN1_IMG_IDX],
            sizeof(vx_uint32));

        if(status!=VX_SUCCESS)
        {
            printf(" ERROR: Unable to query input image !!!\n");
        }
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX],
            sizeof(vx_uint32));

        if(status!=VX_SUCCESS)
        {
            printf(" ERROR: Unable to query output image !!!\n");
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt[TIVX_C7X_IMG_ADD_IN0_IMG_IDX]
            ||
            VX_DF_IMAGE_U8 != fmt[TIVX_C7X_IMG_ADD_IN1_IMG_IDX]
            ||
            VX_DF_IMAGE_U8 != fmt[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX]
            )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            printf(" ERROR: Input/Output image format not correct !!!\n");
        }

        /* Check for frame sizes */
        if ((w[TIVX_C7X_IMG_ADD_IN0_IMG_IDX] !=
             w[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX]) ||
            (h[TIVX_C7X_IMG_ADD_IN0_IMG_IDX] !=
             h[TIVX_C7X_IMG_ADD_OUT0_IMG_IDX]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            printf(" ERROR: Input/Output image wxh do not match !!!\n");
        }
        if ((w[TIVX_C7X_IMG_ADD_IN0_IMG_IDX] !=
             w[TIVX_C7X_IMG_ADD_IN1_IMG_IDX]) ||
            (h[TIVX_C7X_IMG_ADD_IN0_IMG_IDX] !=
             h[TIVX_C7X_IMG_ADD_IN1_IMG_IDX]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            printf(" ERROR: Input/Output image wxh do not match !!!\n");
        }
    }

    return status;
}
