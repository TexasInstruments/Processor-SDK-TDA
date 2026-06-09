/**
 *   Copyright (c) Texas Instruments Incorporated 2019
 *   All rights reserved.
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

/**
 *  \file csl_nf.h
 *
 *  \brief  Defines the structures / control operations that could be used to
 *              configure / control NF in VPAC
 */


#ifndef CSL_NF_H_
#define CSL_NF_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <soc/vhwa_soc.h>
#include "include/nf_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** None */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                 Internal Function Definitions                              */
/* ========================================================================== */

/**
 *  \brief Sets the entire NF configuration to the NR core registers.
 *         Except the filter tables
 *
 *  \param nfRegs     Pointer to structure containing DOF Base Address
 *  \param config     Pointer to #Nf_Config structure containing the register
 *                    configurations.
 *  \return           None
 */
void CSL_nfSetConfig(CSL_vpac_nfRegs *nfRegs, const Nf_Config *cfg);

/**
 *  \brief Sets the tables for NF (Both Bilateral and generic filter)
 *
 *  \param nfRegs     Pointer to structure containing DOF Base Address
 *  \param config     Pointer to #Nf_Config structure containing the filter
 *                    configuration.
 *  \return           Returns 0 on success else returns error value
 */
int32_t CSL_nfSetWgtTableConfig(CSL_vpac_nfRegs *nfRegs,
                                            const Nf_WgtTableConfig *cfg);

/**
 *  \brief Update the NF configuration for Luma and Chroma component. This
 *         function only update the parameter changes between Luma and Chroma.
 *         This shouldn't be called before CSL_nfSetConfig
 *
 *  \param nfRegs     Pointer to structure containing DOF Base Address
 *  \param config     Pointer to #Nf_Config structure containing the register
 *                    configurations.
 *  \return           None
 */
void CSL_nfUpdateConfig(CSL_vpac_nfRegs *nfRegs, const Nf_Config *cfg);


#ifdef __cplusplus
}
#endif

#endif  /* CSL_NF_H_ */
