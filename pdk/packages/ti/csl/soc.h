/**
 *   @file  csl/soc.h
 *
 *   @brief
 *      This file contains SOC specific defintions.
 *
 *  ============================================================================
 *  @n   (C) Copyright 2009-2019, Texas Instruments, Inc.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef CSL_SOC_H
#define CSL_SOC_H

#include <ti/csl/cslr.h>
#include <ti/csl/tistdtypes.h>

/*****************************************************************************\
* Static inline definition
\*****************************************************************************/
#ifndef CSL_IDEF_INLINE
#define CSL_IDEF_INLINE static inline
#endif

#if defined(SOC_K2K)

#include <ti/csl/soc/k2k/src/cslr_device.h>

#elif defined(SOC_K2H)

#include <ti/csl/soc/k2h/src/cslr_device.h>

#elif defined(SOC_K2E)

#include <ti/csl/soc/k2e/src/cslr_device.h>

#elif defined(SOC_K2L)

#include <ti/csl/soc/k2l/src/cslr_device.h>

#elif defined(SOC_AM574x)

#include <ti/csl/soc/am574x/src/cslr_soc.h>

#elif defined(SOC_AM572x)

#include <ti/csl/soc/am572x/src/cslr_soc.h>

#elif defined(SOC_AM571x)

#include <ti/csl/soc/am571x/src/cslr_soc.h>

#elif defined(SOC_K2G)

#include <ti/csl/soc/k2g/src/cslr_soc.h>

#elif defined (SOC_TDA2XX)

#include <ti/csl/soc/tda2xx/cslr_soc.h>

#elif defined(SOC_TDA2PX)

#include <ti/csl/soc/tda2px/cslr_soc.h>

#elif defined(SOC_TDA2EX)

#include <ti/csl/soc/tda2ex/cslr_soc.h>

#elif defined(SOC_TDA3XX)

#include <ti/csl/soc/tda3xx/cslr_soc.h>

#elif defined(SOC_DRA72x)

#include <ti/csl/soc/dra72x/cslr_soc.h>

#elif defined(SOC_DRA75x)

#include <ti/csl/soc/dra75x/cslr_soc.h>

#elif defined(SOC_DRA78x)

#include <ti/csl/soc/dra78x/cslr_soc.h>

#elif defined(SOC_C6678)

#include <ti/csl/soc/c6678/src/cslr_device.h>

#elif defined(SOC_C6657)

#include <ti/csl/soc/c6657/src/cslr_device.h>

#elif defined(SOC_OMAPL137)

#include <ti/csl/soc/omapl137/src/cslr_soc.h>

#elif defined(SOC_OMAPL138)

#include <ti/csl/soc/omapl138/src/cslr_soc.h>

#elif defined(SOC_AM65XX)

#include <ti/csl/soc/am65xx/src/cslr_soc.h>

#elif defined (SOC_J721E)

#include <ti/csl/soc/j721e/src/cslr_soc.h>

#elif defined (SOC_J7200)

#include <ti/csl/soc/j7200/src/cslr_soc.h>

#elif defined (SOC_AM77X)

#include <ti/csl/soc/am77x/src/cslr_soc.h>

#elif defined (SOC_AM64X)

#include <ti/csl/soc/am64x/src/cslr_soc.h>

#elif defined (SOC_TPR12)

#include <ti/csl/soc/tpr12/src/cslr_soc.h>
#include <ti/csl/soc/tpr12/src/csl_soc_util.h>

#elif defined (SOC_AWR294X)

#include <ti/csl/soc/awr294x/src/cslr_soc.h>
#include <ti/csl/soc/awr294x/src/csl_soc_util.h>

#elif defined (SOC_J721S2)

#include <ti/csl/soc/j721s2/src/cslr_soc.h>

#elif defined (SOC_AM263X)

#include <ti/csl/soc/am263x/src/cslr_soc.h>

#elif defined (SOC_AM62X)

#include <ti/csl/soc/am62x/src/cslr_soc.h>

#elif defined (SOC_J784S4) || defined (SOC_J742S2)

#include <ti/csl/soc/j784s4/src/cslr_soc.h>

#elif defined (SOC_AM62A)

#include <ti/csl/soc/am62a/src/cslr_soc.h>

#elif defined (SOC_AM62PX)

#include <ti/csl/soc/am62px/src/cslr_soc.h>

#elif defined (SOC_AM62LX)

#include <ti/csl/soc/am62lx/src/cslr_soc.h>

#elif defined (SOC_AM275X)

#include <ti/csl/soc/am275x/src/cslr_soc.h>

#elif defined (SOC_AM283X)

#include <ti/csl/soc/am283x/src/cslr_soc.h>

#endif /* SOC_XXXXX */

#endif  /* CSL_SOC_H*/

