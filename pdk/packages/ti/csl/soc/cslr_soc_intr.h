/*
 *  Copyright (C) 2019 Texas Instruments Incorporated
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

#ifndef CSLR_SOC_INTR_H
#define CSLR_SOC_INTR_H

#if defined (SOC_AM65XX)
#include <ti/csl/soc/am65xx/src/cslr_soc_intr.h>
#endif

#if defined (SOC_J721E)
#include <ti/csl/soc/j721e/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM64X)
#include <ti/csl/soc/am64x/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM62X)
#include <ti/csl/soc/am62x/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM62A)
#include <ti/csl/soc/am62a/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM62PX)
#include <ti/csl/soc/am62px/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM62LX)
#include <ti/csl/soc/am62lx/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM275X)
#include <ti/csl/soc/am275x/src/cslr_soc_intr.h>
#endif

#if defined (SOC_AM283X)
#include <ti/csl/soc/am283x/src/cslr_soc_intr.h>
#endif

#endif /* CSLR_SOC_INTR_H */
