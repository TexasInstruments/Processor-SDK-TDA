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
----------------------------------------------------------------------------
@file    tidl_mma_platform.h
@brief   This file defines various macros for various supported kernels by MMALIB
         based on different different devices
@version 0.1 (Apr 2022) : Initial version [Anshu]
----------------------------------------------------------------------------
*/
#ifndef TIDL_MMA_PLATFORM_H
#define TIDL_MMA_PLATFORM_H

/* ROW FLOW variants */
#if defined(__C7100__) || defined(__C7120__) 
    #define TIDL_MMA_ROW_FLOW_ES_STYLE
#else
    /* AM62A doesn't support ES style variants */
    #undef TIDL_MMA_ROW_FLOW_ES_STYLE
#endif

#if defined(__C7100__)
    /* OTF variants of row flow not supported in J7ES */
    #undef TIDL_MMA_ROW_FLOW_OTF
#else
    #define TIDL_MMA_ROW_FLOW_OTF
#endif


/* COL FLOW variants */
#if defined(__C7100__) || defined(__C7120__) 
    #define TIDL_MMA_COL_FLOW_ES_STYLE
#else
    /* AM62A doesn't support ES style variants */
    #undef TIDL_MMA_COL_FLOW_ES_STYLE
#endif

#if defined(__C7100__)
    /* PTF and High precision variants of col flow are not supported in J7ES */
    #undef TIDL_MMA_COL_FLOW_OTF_HIGH_THROUGHPUT
    #undef TIDL_MMA_COL_FLOW_OTF_HIGH_PRECISION
#else
    #define TIDL_MMA_COL_FLOW_OTF_HIGH_THROUGHPUT
    #define TIDL_MMA_COL_FLOW_OTF_HIGH_PRECISION
#endif

#if defined(__C7100__) || defined(__C7120__)
	#define C71XX
#else
	#undef C71XX
#endif

#if defined(__C7120)
	#define LEGACY
#else
	#undef LEGACY
#endif

#if defined(__C7120__) || defined(__C7504__) || defined (__C7524__) || defined (__C7604__)
	#define MMA_BIAS
#else
	#undef MMA_BIAS
#endif

#endif /* TIDL_MMA_PLATFORM_H */
