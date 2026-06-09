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
 *  \defgroup DDR_PARITY_PROTECTION DDR parity protection configuration API
 */

/**
 *  \ingroup  DDR_PARITY_PROTECTION
 *  \defgroup DDR_PARITY_PROTECTION_INTERFACE DDR parity protection configuration API interface.
 *
 *  @{
 */

/**
 *  \file     ddr_parity_protection.h
 *
 *  \brief    Interface for DDR parity checkers configuration.
 *
 */

#ifndef DDR_PARITY_PROTECTION_
#define DDR_PARITY_PROTECTION_

/*	==========================================================================	*/
/*	                            Include Files                                 	*/
/*	==========================================================================	*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ip_fma_common.h>

/*	==========================================================================	*/
/*	                        Macros & Typedefs                            	    */
/*	==========================================================================	*/

/**
 *  Base addresses of DDR modules.
 *  J784S4 contains 4 DDR modules, J721S2 contains 2 DDR modules.
 */
#if defined(SOC_J721S2)
#define DDR_MODULE_COUNT                (2U)
#define DDRSS0_BASE_ADDRESS             CSL_COMPUTE_CLUSTER0_CTL_CFG_BASE
#define DDRSS1_BASE_ADDRESS             CSL_COMPUTE_CLUSTER0_DDR1_1_CTL_CFG_BASE
#endif
#if defined(SOC_J784S4)
#define DDRSS0_BASE_ADDRESS             CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS0_CTLCFG_BASE
#define DDRSS1_BASE_ADDRESS             CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS1_CTLCFG_BASE
#define DDRSS2_BASE_ADDRESS             CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS2_CTLCFG_BASE
#define DDRSS3_BASE_ADDRESS             CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS3_CTLCFG_BASE
#define DDR_MODULE_COUNT                (4U)
#endif

/**
 *  CTL PARITY CONFIGURATION REGISTERS OFFSETS AND CORRESPONDING MASKS.
 */
#define DDRSS_CTL_PCFG1_REG_CTL_442_OFFSET                                  (0x6E8U)
#define DDRSS_CTL_PCFG1_MC_PARITY_ERROR_TYPE                                (0x00000001U)
#define DDRSS_CTL_PCFG1_REGPORT_ADDR_PARITY_PROTECTION_EN                   (0x00000100U)
#define DDRSS_CTL_PCFG1_REGPORT_WRITEMASK_PARITY_PROTECTION_EN              (0x00010000U)
#define DDRSS_CTL_PCFG1_REGPORT_WRITE_PARITY_PROTECTION_EN                  (0x01000000U)

#define DDRSS_CTL_PCFG1_REG_CTL_443_OFFSET                                  (0x6ECU)
#define DDRSS_CTL_PCFG2_REGPORT_READ_PARITY_PROTECTION_EN                   (0x00000001U)
#define DDRSS_CTL_PCFG2_PARAMREG_PARITY_PROTECTION_EN                       (0x00000100U)
#define DDRSS_CTL_PCFG2_REGPORT_ADDR_PARITY_PROTECTION_INJECTION_EN         (0x00010000U)
#define DDRSS_CTL_PCFG2_REGPORT_WRITEMASK_PARITY_PROTECTION_INJECTION_EN    (0x01000000U)

#define DDRSS_CTL_PCFG3_REG_CTL_444_OFFSET                                  (0x6F0U)
#define DDRSS_CTL_PCFG3_PARAMREG_PARITY_PROTECTION_INJECTION_EN             (0x00000001U)
#define DDRSS_CTL_PCFG3_REGPORT_READ_PARITY_PROTECTION_INJECTION_EN         (0x00000100U)
#define DDRSS_CTL_PCFG3_REGPORT_WRITE_PARITY_PROTECTION_INJECTION_EN        (0x00010000U)

/* Indicates the source of DDR CTL safety error interrupts in bits [24:31]. */
#define DDRSS_CTL_GLOBAL_ERROR_INFO_REG_OFFSET          (0x6D4U)
#define DDRSS_CTL_GLOBAL_ERROR_INFO_ERROR_MASK          (0xFF000000U)
#define DDRSS_CTL_GLOBAL_ERROR_INFO_ERROR_SHIFT         (24U)
#define DDRSS_CTL_GLOBAL_ERROR_INFO_PARITY_ERROR        (0x20U)
#define DDRSS_CTL_GLOBAL_ERROR_INFO_RESET               (0xFF000000U)

/**
 *  Specifies the source of the GLOBAL_ERROR_INFO Bit 5 error(parity error) in bits [16:20].
 *  Bit [0] correlates to an address parity error,
 *  bit [1] correlates to a write data mask parity error,
 *  bit [2] correlates to a param register parity error,
 *  bit [3] correlates to a read data parity error and
 *  bit [4] correlates to a write data parity error.
 *  This is represented in \ref IpFma_Ddr17CtlParityErrorSource.
 */
#define DDRSS_CTL_PARITY_ERROR_SOURCE_REG_OFFSET        (0x6DCU)
#define DDRSS_CTL_PARITY_ERROR_SOURCE_ERROR_MASK        (0x001F0000U)
#define DDRSS_CTL_PARITY_ERROR_SOURCE_ERROR_SHIFT       (16U)

/**
 *  \brief DDR CTL parity error source types.
 */
typedef enum{
    ADDRESS_PARITY_ERROR            = 0x1U,     /* Bit 0*/
    WRITE_DATA_MASK_PARITY_ERROR    = 0x2U,     /* Bit 1*/
    PARAM_REGISTER_PARITY_ERROR     = 0x4U,     /* Bit 2*/
    READ_DATA_PARITY_ERROR          = 0x8U,     /* Bit 3*/
    WRITE_DATA_PARITY_ERROR         = 0x10U     /* Bit 4*/
} IpFma_Ddr17CtlParityErrorSource;


/**
 *  PHY AND PI PARITY CONFIGURATION REGISTERS OFFSETS AND CORRESPONDING MASKS.
 */

/* These registers(10 bits) are used to inject specific parity errors. */
#define DDRSS_PHY_REG_81_OFFSET                             (0x4144U)
#define DDRSS_PHY_REG_81_PARITY_ERROR_INJECTION_OFFSET      (0U)

#define DDRSS_PHY_REG_337_OFFSET                            (0x4544U)
#define DDRSS_PHY_REG_337_PARITY_ERROR_INJECTION_OFFSET     (0U)

#define DDRSS_PHY_REG_593_OFFSET                            (0x4944U)
#define DDRSS_PHY_REG_593_PARITY_ERROR_INJECTION_OFFSET     (0U)

#define DDRSS_PHY_REG_849_OFFSET                            (0x4D44U)
#define DDRSS_PHY_REG_849_PARITY_ERROR_INJECTION_OFFSET     (0U)

#define DDRSS_PHY_REG_1060_OFFSET                           (0x5090U)
#define DDRSS_PHY_REG_1060_PARITY_ERROR_INJECTION_OFFSET    (16U)

#define DDRSS_PHY_REG_1349_OFFSET                           (0x5514U)
#define DDRSS_PHY_REG_1349_PARITY_ERROR_INJECTION_OFFSET    (16U)

/* Register phy 1370 also contains error injection enable bit. */
#define DDRSS_PHY_REG_1370_OFFSET                           (0x5568U)
#define DDRSS_PHY_REG_1370_PARITY_ERROR_INJECTION_EN        (0x00000001U)
#define DDRSS_PHY_REG_1370_PARITY_ERROR_INJECTION_OFFSET    (8U)

/* PI errors are represented through PHY error registers, so they are grouped together. */
#define DDRSS_PI_REG_299_OFFSET                             (0x5514U)
#define DDRSS_PI_REG_299_PARITY_ERROR_INJECTION_OFFSET      (16U)


/* Indicates the source of DDR PHY safety error interrupts in bits [24:31]. */
#define DDRSS_PHY_GLOBAL_ERROR_INFO_REG_OFFSET              (0x5594U)
#define DDRSS_PHY_GLOBAL_ERROR_INFO_MASK                    (0x0000003FU)
#define DDRSS_PHY_GLOBAL_ERROR_INFO_MASK_TIMEOUT_BIT        (0x00000100U)
#define DDRSS_PHY_GLOBAL_ERROR_INFO_PARITY_ERROR            (0x2U)

#define DDRSS_PHY_PARITY_ERROR_INFO_REG_OFFSET              (0x556CU)
#define DDRSS_PHY_PARITY_ERROR_INFO_SOURCE_MASK             (0x0000007FU)
#define DDRSS_PHY_PARITY_ERROR_INFO_RESET                   (0x007F0000U)
#define DDRSS_PHY_PARITY_ERROR_INFO_RESET_SHIFT             (16U)

/**
 *  \brief DDR PHY parity error source types.
 */
typedef enum{
    DDR_PHY_PARITY_ERR_BIT1 = 0x2U,      /* Bit 1 */
    DDR_PHY_PARITY_ERR_BIT2 = 0x4U,      /* Bit 2 */
    DDR_PHY_PARITY_ERR_BIT3 = 0x8U,      /* Bit 3 */
    DDR_PHY_PARITY_ERR_BIT4 = 0x10U,     /* Bit 4 */
    DDR_PHY_PARITY_ERR_BIT5 = 0x20U,     /* Bit 5 */
    DDR_PHY_PARITY_ERR_BIT6 = 0x40U,     /* Bit 6 */
} IpFma_Ddr17PhyParityErrorSource;

/*	==========================================================================	*/
/*	                        Structure Declarations                            	*/
/*	==========================================================================	*/

/*	==========================================================================	*/
/*	                         Function Declarations                            	*/
/*	==========================================================================	*/

/**
 * \brief   Disables CTL parity protection injection.
 *
 * \param   source                  Value representing which type of parity protection injection to disable.
 * \param   baseAddress             Base address of the DDR module CTL configuration MMR.
 *
 * \retval  \ref IPFMA_OK 		    Parity protection injection disabled
 * 			\ref IPFMA_E_PARAM      Parity protection injection not disabled - Invalid parameter(s)
 */
int32_t IpFma_Ddr17_DisableCtlParityProtectionInjection(uint32_t baseAddress, IpFma_Ddr17CtlParityErrorSource source);

/**
 * \brief   Enables CTL parity protection injection.
 *
 * \param   source                  Value representing which type of parity protection injection to enable.
 * \param   baseAddress             Base address of the DDR module CTL configuration MMR.
 *
 * \retval  \ref IPFMA_OK 		    Parity protection injection enabled
 * 			\ref IPFMA_E_PARAM      Parity protection injection not enabled - Invalid parameter(s)
 */
int32_t IpFma_Ddr17_EnableCtlParityProtectionInjection(uint32_t baseAddress, IpFma_Ddr17CtlParityErrorSource source);

/**
 * \brief   Enables CTL parity protection.
 *
 * \param   baseAddress  Base address of the DDR module CTL configuration MMR.
 *
 */
void IpFma_Ddr17_EnableCtlParityProtection(uint32_t baseAddress);


/*	==========================================================================	*/
/*	                      Static Function Definitions                         	*/
/*	==========================================================================	*/

/* None */

#ifdef __cplusplus
}
#endif

#endif /*	#ifndef DDR_PARITY_PROTECTION_	*/

/** @} */
