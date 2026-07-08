/*
 *   Copyright (c) Texas Instruments Incorporated 2018
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
 *  \ingroup  DRV_VHWA_MODULE
 *  \defgroup DRV_BIST_MODULE BIST Module
 *            BIST Module
 *
 */
/**
 *  \ingroup  DRV_BIST_MODULE
 *  \defgroup DRV_BIST_MODULE_INTERFACE BIST Interface
 *            Interface file for BIST M2M FVID2 driver
 *
 *  @{
 */
/**
 *  \file vhwa_m2mBist.h
 *
 *  \brief  Defines the structures / control operations that could be used to
 *          configure / control BIST M2M driver
 */

#ifndef VHWA_M2M_BIST_H_
#define VHWA_M2M_BIST_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <include/vhwa_common.h>
#include <soc/vhwa_soc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor Vhwa_Bist
 *  \name   VHWA BIST IOCTL macros
 *  \brief  Input/Output control MACRO's for BIST driver
 *
 *  @{
 */
#define VHWA_M2M_IOCTL_BIST_VPAC0_ACQUIRE_LOCK       (VHWA_IOCTL_COMMON_IOCTL_BASE + 1u)
#define VHWA_M2M_IOCTL_BIST_VPAC0_RELEASE_LOCK       (VHWA_M2M_IOCTL_BIST_VPAC0_ACQUIRE_LOCK + 1u)
#define VHWA_M2M_IOCTL_BIST_VPAC1_ACQUIRE_LOCK       (VHWA_M2M_IOCTL_BIST_VPAC0_RELEASE_LOCK + 1u)
#define VHWA_M2M_IOCTL_BIST_VPAC1_RELEASE_LOCK       (VHWA_M2M_IOCTL_BIST_VPAC1_ACQUIRE_LOCK + 1u)
#define VHWA_M2M_IOCTL_BIST_DMPAC_ACQUIRE_LOCK       (VHWA_M2M_IOCTL_BIST_VPAC1_RELEASE_LOCK + 1u)
#define VHWA_M2M_IOCTL_BIST_DMPAC_RELEASE_LOCK       (VHWA_M2M_IOCTL_BIST_DMPAC_ACQUIRE_LOCK + 1u)
 /** @} */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t Vhwa_m2mBistInit(void);

int32_t Vhwa_m2mBistDeInit(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */



#ifdef __cplusplus
}
#endif

#endif /* VHWA_M2M_BIST_H_ */

/** @} */

