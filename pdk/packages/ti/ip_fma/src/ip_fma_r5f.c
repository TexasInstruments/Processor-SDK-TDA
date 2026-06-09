/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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
 */

/**
 *  \ingroup  IP_FMA_R5F
 *  \defgroup IP_FMA_R5F_IMPLEMENTATION R5F Register Check Implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f.c
 *
 *  \brief    R5F static registers specific wrappers for periodic readback implementation.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ip_fma_r5f.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>

/* ========================================================================== */
/*                           File Scope Variables                             */
/* ========================================================================== */

/*	==========================================================================	*/
/*	                          Macros & Typedefs                               	*/
/*	==========================================================================	*/

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

/* WRITING */
/*	MPU memory region programming registers	*/
_DEFINE_COPROCR_WRITE_FUNC(rgnr, p15, 0, c6, c2, 0)	    /*	Region Number Register	*/

/* READING */
/*  Control registers	*/
_DEFINE_COPROCR_READ_FUNC(sctlr, p15, 0, c1, c0, 0)	    /*  System control register	 */
_DEFINE_COPROCR_READ_FUNC(actlr, p15, 0, c1, c0, 1)	    /*	Auxiliary control register	*/
_DEFINE_COPROCR_READ_FUNC(sactlr, p15, 0, c15, c0, 0)	/*	Second auxiliary control register  */

/*	MPU memory region programming registers	*/
_DEFINE_COPROCR_READ_FUNC(rbar, p15, 0, c6, c1, 0)	    /*	Region Base Address Registers  */
_DEFINE_COPROCR_READ_FUNC(rser, p15, 0, c6, c1, 2)	    /*	Region Size and Enable Registers  */
_DEFINE_COPROCR_READ_FUNC(racr, p15, 0, c6, c1, 4)	    /*	Region Access Control Registers	 */
_DEFINE_COPROCR_READ_FUNC(rgnr, p15, 0, c6, c2, 0)	    /*	Region Number Register	*/

_DEFINE_COPROCR_READ_FUNC(btcmrr, p15, 0, c9, c1, 0)	/*	BTCM region register  */
_DEFINE_COPROCR_READ_FUNC(atcmrr, p15, 0, c9, c1, 1)	/*	ATCM region register  */

_DEFINE_COPROCR_READ_FUNC(csselr, p15, 2, c0, c0, 0)	/*	Cache Size Selection Register  */

/*	Abbreviation names of the registers below are not set in TRM so full names are used	 */
_DEFINE_COPROCR_READ_FUNC(slavePortControl, p15, 0, c11, c0, 0)	    /*	Slave Port Control Register	 */

/*	Peripheral interface region registers  */
_DEFINE_COPROCR_READ_FUNC(llppNormalAxiRegion , p15, 0, c15, c0, 1)	    /*	LLPP Normal AXI region register  */
_DEFINE_COPROCR_READ_FUNC(llppVirtualAxiRegion , p15, 0, c15, c0, 2)	/*	LLPP Virtual AXI region register  */
_DEFINE_COPROCR_READ_FUNC(ahbPeriphIfRegion , p15, 0, c15, c0, 3)	    /*	AHB peripheral interface region register  */

/*	Thread and process ID registers	*/
_DEFINE_COPROCR_READ_FUNC(userReadWriteTpIds, p15, 0, c13, c0, 2)	    /*	User read/write Thread and Proc. ID Register  */
_DEFINE_COPROCR_READ_FUNC(userReadTpIds, p15, 0, c13, c0, 3)	        /*	User Read Only Thread and Proc. ID Register	  */
_DEFINE_COPROCR_READ_FUNC(privilegedTpIds, p15, 0, c13, c0, 4)	        /*	Privileged Only Thread and Proc. ID Register  */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_R5f_GetCoreCtrlRegs(IpFma_R5fCoreCtrlRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->sctlr = IpFma_R5f_read_sctlr();
        out->actlr = IpFma_R5f_read_actlr();
        out->sactlr = IpFma_R5f_read_sactlr();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetMpuRegionRegs(IpFma_R5fMpuRegionRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->rbar = IpFma_R5f_read_rbar();
        out->rser = IpFma_R5f_read_rser();
        out->racr = IpFma_R5f_read_racr();
        out->rgnr = IpFma_R5f_read_rgnr();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetMpuRegs(IpFma_R5fMpuRegionRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        uint32_t mpuRegionsCount = CSL_armR5MpuGetNumRegions();
        IpFma_R5fMpuRegionRegs mpuRegs;
        for (uint32_t region = 0U; region < mpuRegionsCount; region++)
        {
            IpFma_R5f_write_rgnr(region);

            status = IpFma_R5f_GetMpuRegionRegs(&mpuRegs);
            if (IPFMA_OK != status)
            {
                break;
            }

            out[region].rbar = mpuRegs.rbar;
            out[region].rser = mpuRegs.rser;
            out[region].racr = mpuRegs.racr;
            out[region].rgnr = mpuRegs.rgnr;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_GetAtcmRegionReg(IpFma_R5fAtcmRegionReg* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->atcmRegionReg = IpFma_R5f_read_atcmrr();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetBtcmRegionReg(IpFma_R5fBtcmRegionReg* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->btcmRegionReg = IpFma_R5f_read_btcmrr();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetSlavePortCtrlReg(IpFma_R5fSlavePortCtrlReg* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->slavePortCtrlReg = IpFma_R5f_read_slavePortControl();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetCacheSizeSelReg(IpFma_R5fCacheSizeSelReg* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->cacheSizeSelReg = IpFma_R5f_read_csselr();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetPeriphIfRegionRegs(IpFma_R5fPeriphIfRegionRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->llppNormalAxiRegion = IpFma_R5f_read_llppNormalAxiRegion();
        out->llppVirtualAxiRegion = IpFma_R5f_read_llppVirtualAxiRegion();
        out->ahbPeriphIfRegion = IpFma_R5f_read_ahbPeriphIfRegion();
    }

    return status;
}

IpFma_Status IpFma_R5f_GetThreadProcessIdsRegs(IpFma_R5fThreadProcessIdsRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        out->userReadWriteTpIds = IpFma_R5f_read_userReadWriteTpIds();
        out->userReadTpIds = IpFma_R5f_read_userReadTpIds();
        out->privilegedTpIds = IpFma_R5f_read_privilegedTpIds();
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareCoreCtrlRegs(IpFma_R5fCoreCtrlRegs* expected, IpFma_R5fCoreCtrlRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->sctlr != actual->sctlr)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->actlr != actual->actlr))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->sactlr != actual->sactlr))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareMpuRegionRegs(IpFma_R5fMpuRegionRegs* expected, IpFma_R5fMpuRegionRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->rbar != actual->rbar)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->rser != actual->rser))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->racr != actual->racr))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->rgnr != actual->rgnr))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareMpuRegs(IpFma_R5fMpuRegionRegs* expected, IpFma_R5fMpuRegionRegs* actual)
{
    IpFma_Status status = IPFMA_OK;
    
    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        uint32_t mpuRegionsCount = CSL_armR5MpuGetNumRegions();
        for (uint32_t region = 0U; region < mpuRegionsCount; region++)
        {
            if (IPFMA_OK == status)
            {
                status = IpFma_R5f_CompareMpuRegionRegs(&expected[region], &actual[region]);
            }
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareAtcmRegionReg(IpFma_R5fAtcmRegionReg* expected, IpFma_R5fAtcmRegionReg* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->atcmRegionReg != actual->atcmRegionReg)
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareBtcmRegionReg(IpFma_R5fBtcmRegionReg* expected, IpFma_R5fBtcmRegionReg* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->btcmRegionReg != actual->btcmRegionReg)
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareSlavePortCtrlReg(IpFma_R5fSlavePortCtrlReg* expected, IpFma_R5fSlavePortCtrlReg* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->slavePortCtrlReg != actual->slavePortCtrlReg)
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareCacheSizeSelReg(IpFma_R5fCacheSizeSelReg* expected, IpFma_R5fCacheSizeSelReg* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->cacheSizeSelReg != actual->cacheSizeSelReg)
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_ComparePeriphIfRegionRegs(IpFma_R5fPeriphIfRegionRegs* expected, IpFma_R5fPeriphIfRegionRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->llppNormalAxiRegion != actual->llppNormalAxiRegion)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->llppVirtualAxiRegion != actual->llppVirtualAxiRegion))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->ahbPeriphIfRegion != actual->ahbPeriphIfRegion))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_R5f_CompareThreadProcessIdsRegs(IpFma_R5fThreadProcessIdsRegs* expected, IpFma_R5fThreadProcessIdsRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == expected)
    {
        status = IPFMA_E_PARAM;
    }
    if ((IPFMA_OK == status) && (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    
    if (IPFMA_OK == status)
    {
        if (expected->userReadWriteTpIds != actual->userReadWriteTpIds)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->userReadTpIds != actual->userReadTpIds))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->privilegedTpIds != actual->privilegedTpIds))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/** @} */