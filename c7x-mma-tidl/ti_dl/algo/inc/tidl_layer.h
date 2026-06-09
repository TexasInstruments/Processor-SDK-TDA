/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
 *  \file tidl_layer.h
 *
 *  \brief This file defines the process function prototype of batch norm layer
 */

#ifndef TIDL_LAYERS_H_
#define TIDL_LAYERS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_eltWise.h"
#include "tidl_shuffleChannel.h"
#include "tidl_resize.h"
#include "tidl_deconv2d.h"
#include "tidl_argop.h"
#include "tidl_softmax.h"
#include "tidl_flatten.h"
#include "tidl_batchNorm.h"
#include "tidl_crop.h"
#include "tidl_batchReshape.h"
#include "tidl_slice.h"
#include "tidl_concat.h"
#include "tidl_pad.h"
#include "tidl_odOutputReformat.h"
#include "tidl_pooling.h"
#include "tidl_innerProduct.h"
#include "tidl_commonUtils.h"
#include "tidl_detectionOutput.h"
#include "tidl_roiPooling.h"
#include "tidl_colorConversion.h"
#include "tidl_reduce.h"
#include "tidl_squeeze.h"
#include "tidl_scatterElements.h"
#include "tidl_gatherLayer.h"
#include "tidl_custom_int.h"
#include "tidl_dataConvert.h"
#include "tidl_conv2d.h"
#include "tidl_const.h"
#include "tidl_reshape.h"
#include "tidl_transpose.h"
#include "tidl_layerNorm.h"
#include "tidl_gridsample.h"
#include "tidl_deformConv.h"
#include "tidl_topK.h"
#include "tidl_tile.h"
#include "tidl_logicalOpLayer.h"
#include "tidl_rmsNorm.h"
#include "tidl_lstm.h"
#include "tidl_gru.h"
#include "tidl_rnn.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#endif /* TIDL_LAYERS_H_*/
