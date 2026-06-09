/**
 *   Copyright (c) Texas Instruments Incorporated 2021-2022
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
 *  \file vhwa_dmpac_priv.h
 *
 *  \brief VHWA Driver J721E/J721S2/J784S4/J742S2 SOC file containing private APIs used by 
           VHWA DMPAC controller driver.
 */

#ifndef VHWA_DMPAC_PRIV_H_
#define VHWA_DMPAC_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#if defined (VHWA_PDK)
#include <ti/csl/cslr_dmpac.h>
#else
#include <drivers/hw_include/cslr_dmpac.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    CSL_dmpacRegs            *dmpacCntlRegs;
    /**< DMPAC Top Register Base address */
    CSL_dmpac_intd_cfgRegs   *dmpacIntdRegs;
    /**< DMPAC intd Register Base address */
    CSL_dmpac_foco_coreRegs  *dmpacFocoRegs;
    /**< DMPAC FOCO Register Base address */
    CSL_dmpac_dofRegs        *dofRegs;
    /**< DOF Register Base Address */
    CSL_lseRegs              *lseRegs;
    /**< LSE Register Base Address */
    CSL_htsRegs              *htsRegs;
    /**< UTC Base Address */
} Dof_SocInfo;

typedef struct
{
    CSL_dmpacRegs            *dmpacCntlRegs;
    /**< DMPAC Top Register Base address */
    CSL_dmpac_intd_cfgRegs   *dmpacIntdRegs;
    /**< DMPAC intd Register Base address */
    CSL_dmpac_foco_coreRegs  *dmpacFocoRegs;
    /**< DMPAC FOCO Register Base address */
    CSL_dmpac_sdeRegs        *sdeRegs;
    /**< SDE Register Base Address */
    CSL_lseRegs              *lseRegs;
    /**< LSE Register Base Address */
    CSL_htsRegs              *htsRegs;
    /**< UTC Base Address */
} Sde_SocInfo;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */
/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/**
 *  \brief API to get Soc Specific information, like base address,
 *         irq number ete., for DOF Driver.
 *
 *  \param  None
 *
 *  \return Pointer to Dof_SocInfo object
 */
void Dof_getSocInfo(Dof_SocInfo *socInfo);

/**
 *  \brief API to get Soc Specific information, like base address,
 *         irq number ete., for SDE Driver.
 *
 *  \param  None
 *
 *  \return Pointer to Sde_SocInfo object
 */
void Sde_getSocInfo(Sde_SocInfo *socInfo);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef VHWA_DMPAC_PRIV_H_ */
