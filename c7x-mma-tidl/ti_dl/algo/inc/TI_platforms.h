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
#ifndef TI_PLATFORMS_H__
#define TI_PLATFORMS_H__

#ifdef _HOST_BUILD

    #if defined (__C7100__) || defined (__C7120__) || defined (__C7504__) || defined (__C7524__) || defined (__C7604__)
        #include "c7x.h"
    #else
        #include "c6xsim/C6xSimulator.h"
    #endif
    #include <assert.h>
    #define restrict
    #define _nassert assert
    #define __int40_t              long long
    #ifdef _MSC_VER /* If using MS Visual Studio Compiler */
        #define inline            __inline
        #define memalign(x, y)      _aligned_malloc(y, x)
        #define align_free         _aligned_free
        #define SET_ALIGN(x, y)     __declspec(align(y))
        #if !defined(__cplusplus)
            #if (_MSC_VER < 1800) /* Before MS Visual Studio 2013 (v12), C99 was not supported */
                typedef unsigned int _Bool;
                #define bool _Bool
                #define true 1
                #define false 0
                #define round(x)           ((x) + 0.5f)
            #else
                #include <stdbool.h>
            #endif
        #endif
    #else
        #define align_free (free)
        #define SET_ALIGN(x, y)
        #include <stdbool.h>
    #endif
    #define CODE_SECTION(a, b)

#else  /* _HOST_BUILD */

    #if defined (__C7100__) || defined (__C7120__) || defined (__C7504__) || defined (__C7524__) || defined (__C7604__)
        #include <c7x.h>
        #ifndef CORE_DSP /* defined when BAM_PLUGIN=yes */
        #include <vcop/vcop.h>
        #endif
        #if defined (C6X_MIGRATION)
            #include <c6x_migration.h>
        #endif
    #else
        #include <c6x.h>
    #endif
    #include <stdbool.h>
    #define align_free (free)
    #define SET_ALIGN(x, y)

    /* ----------------------------------------------------------------
     *  MISRAC Rules 19.4, 19.7, 19.10, 19.13 Deviation: These rules
     *  are concerned with the different interpretations of macros
     *  across compilers, however, the follow 2 lines are a construction
     *  taken directly from the DSP compiler documentation regarding
     *  the use of pragma macro redirection.  Since this section is only
     *  applicable to the DSP compiler (explicitly using the else condition
     *  of the _HOST_BUILD macro, these instances are acceptable.
     * ---------------------------------------------------------------- */
    #define EMIT_PRAGMA(x) _Pragma(#x)
    #define CODE_SECTION(fn, section) EMIT_PRAGMA(CODE_SECTION(fn, section))
#if defined (__C7100__) || defined (__C7120__) || defined (__C7504__) || defined (__C7524__) || defined (__C7604__)
    #define _setFADCR(v) (__FPCR = v)
    #define _getFADCR()  (__FPCR)
#else
    #define _setFADCR(v) (FADCR = v)
    #define _getFADCR()  FADCR
#endif

#endif /* _HOST_BUILD */

#endif /* ! TI_PLATFORMS_H__ */

