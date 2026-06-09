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
 *  \defgroup IP_FMA_R5F R5F static Register Check Wrappers
 */

/**
 *  \ingroup  IP_FMA_R5F
 *  \defgroup IP_FMA_R5F_INTERFACE Register Check Interface.
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f.h
 *
 *  \brief    Register readback interface for r5f static registers.
 *
 */

#ifndef IP_FMA_R5F_
#define IP_FMA_R5F_

/*	==========================================================================	*/
/*	                            Include Files                                 	*/
/*	==========================================================================	*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>

#include <ip_fma_common.h>

/*	==========================================================================	*/
/*	                        Macros & Typedefs                            	*/
/*	==========================================================================	*/

/* Instruction sizes. */
#define THUMB_INSTR_SIZE                        (0x2U)  /* This instruction belongs to THUMB/THUMB-2 set. */
#define THUMB_INSTR_SIZE_32                     (0x4U)  /* This instruction belongs to THUMB-2 set.       */
#define ARM_INSTR_SIZE                          (0x4U)  /* This instruction belongs to ARM set.           */

/**
 * \brief Define an inline read accessor for an ARM coprocessor register.
 *
 * This macro generates a static inline function named
 * IpFma_R5f_read_<_name>() that reads the value from the coprocessor
 * register specified by the coproc/opc1/CRn/CRm/opc2 fields using the
 * ARM MRC instruction.
 *
 * \param _name  Suffix used in the generated function name.
 * \param coproc Coprocessor identifier passed to the MRC instruction.
 * \param opc1   Primary opcode field selecting the register group.
 * \param CRn    Coprocessor register number (Rn) to be written.
 * \param CRm    Coprocessor register number (Rm) to be written.
 * \param opc2   Secondary opcode field further qualifying the register.
 */
#define _DEFINE_COPROCR_READ_FUNC(_name, coproc, opc1, CRn, CRm, opc2)	\
static inline uint32_t IpFma_R5f_read_ ## _name(void)				\
{									\
	uint32_t v;							\
	__asm__ volatile ("mrc "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : "=r" (v));\
	return v;							\
}

/**
 * \brief Define an inline write accessor for an ARM coprocessor register.
 *
 * This macro generates a static inline function named
 * IpFma_R5f_write_<_name>() that writes the provided value to the coprocessor
 * register specified by the coproc/opc1/CRn/CRm/opc2 fields using the
 * ARM MRC instruction.
 *
 * \param _name  Suffix used in the generated function name.
 * \param coproc Coprocessor identifier passed to the MRC instruction.
 * \param opc1   Primary opcode field selecting the register group.
 * \param CRn    Coprocessor register number (Rn) to be written.
 * \param CRm    Coprocessor register number (Rm) to be written.
 * \param opc2   Secondary opcode field further qualifying the register.
 */
#define _DEFINE_COPROCR_WRITE_FUNC(_name, coproc, opc1, CRn, CRm, opc2)	\
static inline void IpFma_R5f_write_ ## _name(uint32_t v)			\
{									\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
}

/**
 *  \brief R5F core control registers.
 */
typedef struct
{
    uint32_t    sctlr;      			/**< System Control Register.          */
    uint32_t    actlr;          		/**< Auxiliary Control Register.       */
    uint32_t    sactlr;        		    /**< Secure Auxiliary Control Register */
} IpFma_R5fCoreCtrlRegs;

/**
 *  \brief R5F MPU region configuration registers.
 */
typedef struct
{
    uint32_t    rbar;            		/**< Region Base Address Register.      */
    uint32_t    rser;            		/**< Region Size and Enable Register.   */
    uint32_t    racr;            		/**< Region Access Control Registers.   */
    uint32_t    rgnr;            		/**< Region Number Register.			*/
} IpFma_R5fMpuRegionRegs;

/**
 *  \brief R5F ATCM configuration register.
 */
typedef struct
{
    uint32_t    atcmRegionReg;     		/**< ATCM region register.	*/
} IpFma_R5fAtcmRegionReg;

/**
 *  \brief R5F BTCM configuration register.
 */
typedef struct
{
	uint32_t	btcmRegionReg;   		/**< BTCM region register.	*/
} IpFma_R5fBtcmRegionReg;

/**
 *  \brief R5F slave port control configuration register.
 */
typedef struct
{
    uint32_t    slavePortCtrlReg;   	/**< Slave port control register.	*/
} IpFma_R5fSlavePortCtrlReg;

/**
 *  \brief R5F cache size selection control register.
 */
typedef struct
{
    uint32_t    cacheSizeSelReg;        /**< Cache Size Selection Register.	*/
} IpFma_R5fCacheSizeSelReg;

/**
 *  \brief R5F peripheral interface region configuration registers.
 */
typedef struct
{
    uint32_t    llppNormalAxiRegion;    /**< LLPP Normal AXI region register				*/
    uint32_t    llppVirtualAxiRegion;   /**< LLPP Virtual AXI region register				*/
    uint32_t    ahbPeriphIfRegion;      /**< AHB peripheral interface region register		*/
} IpFma_R5fPeriphIfRegionRegs;

/**
 *  \brief R5F Thread and process ID registers.
 */
typedef struct
{
	uint32_t    userReadWriteTpIds;     /**< User read/write Thread and Proc. ID Register	*/
    uint32_t    userReadTpIds;    	    /**< User Read Only Thread and Proc. ID Register	*/
    uint32_t    privilegedTpIds;        /**< Privileged Only Thread and Proc. ID Register	*/
} IpFma_R5fThreadProcessIdsRegs;

/**
 *  \brief R5F instruction set mode.
 */
typedef enum
{
    IPFMA_R5F_INSTRSET_ARM_MODE = 0U,
    IPFMA_R5F_INSTRSET_THUMB_MODE = 1U,
} IpFma_R5f_InstructionSet;

/*	==========================================================================	*/
/*	                        Structure Declarations                            	*/
/*	==========================================================================	*/

/* None */

/*	==========================================================================	*/
/*	                         Function Declarations                            	*/
/*	==========================================================================	*/

/**
 * \brief   This function gets R5F core control registers.
 *
 * \param   out     Structure containing R5F core control configuration registers states
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 			\ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetCoreCtrlRegs(IpFma_R5fCoreCtrlRegs* out);

/**
 * \brief   This function gets R5F MPU region configuration registers.
 *
 * \param	out		Structure containing R5F MPU region configuration registers states
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetMpuRegionRegs(IpFma_R5fMpuRegionRegs* out);

/**
 * \brief   This function gets R5F MPU configuration registers.
 *
 * \param	out		An array holding the configuration register states for all R5F MPU regions.
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetMpuRegs(IpFma_R5fMpuRegionRegs* out);

/**
 * \brief   This function gets R5F ATCM region configuration register.
 *
 * \param	out		Structure containing R5F ATCM region configuration register state
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 			\ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetAtcmRegionReg(IpFma_R5fAtcmRegionReg* out);

/**
 * \brief   This function gets r5f BTCM region configuration register.
 *
 * \param	out		Structure containing R5F BTCM region configuration register state
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetBtcmRegionReg(IpFma_R5fBtcmRegionReg* out);

/**
 * \brief   This function gets R5F slave port control configuration register.
 *
 * \param	out		Structure containing R5F slave port control configuration register state
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 *          \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetSlavePortCtrlReg(IpFma_R5fSlavePortCtrlReg* out);

/**
 * \brief   This function gets R5F cache size selection configuration register.
 *
 * \param	out		Structure containing R5F cache size selection configuration register state
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetCacheSizeSelReg(IpFma_R5fCacheSizeSelReg* out);

/**
 * \brief   This function gets R5F peripheral interface region configuration registers.
 *
 * \param	out		Structure containing R5F peripheral interface region configuration registers states
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 *          \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetPeriphIfRegionRegs(IpFma_R5fPeriphIfRegionRegs* out);

/**
 * \brief   This function gets R5F thread and process ids registers.
 *
 * \param	out		Structure containing R5F thread and process ids registers states
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_R5f_GetThreadProcessIdsRegs(IpFma_R5fThreadProcessIdsRegs* out);

/**
 * \brief   This function is used to compare R5F core control registers
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareCoreCtrlRegs(IpFma_R5fCoreCtrlRegs* expected, IpFma_R5fCoreCtrlRegs* actual);

/**
 * \brief   This function is used to compare R5F MPU region registers
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareMpuRegionRegs(IpFma_R5fMpuRegionRegs* expected, IpFma_R5fMpuRegionRegs* actual);

/**
 * \brief   This function is used to compare all R5F MPU registers
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareMpuRegs(IpFma_R5fMpuRegionRegs* expected, IpFma_R5fMpuRegionRegs* actual);

/**
 * \brief   This function is used to compare R5F ATCM region register
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareAtcmRegionReg(IpFma_R5fAtcmRegionReg* expected, IpFma_R5fAtcmRegionReg* actual);

/**
 * \brief   This function is used to compare R5F BTCM region register
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareBtcmRegionReg(IpFma_R5fBtcmRegionReg* expected, IpFma_R5fBtcmRegionReg* actual);

/**
 * \brief   This function is used to compare R5F slave port control register
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareSlavePortCtrlReg(IpFma_R5fSlavePortCtrlReg* expected, IpFma_R5fSlavePortCtrlReg* actual);

/**
 * \brief   This function is used to compare R5F cache size selection register
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareCacheSizeSelReg(IpFma_R5fCacheSizeSelReg* expected, IpFma_R5fCacheSizeSelReg* actual);

/**
 * \brief   This function is used to compare R5F peripheral interface regions registers
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_ComparePeriphIfRegionRegs(IpFma_R5fPeriphIfRegionRegs* expected, IpFma_R5fPeriphIfRegionRegs* actual);

/**
 * \brief   This function is used to compare R5F thread and process ids registers
 *
 * \param	expected	Previous status of r5f registers
 * \param	actual      Current status of r5f registers
 *
 * \retval 	\ref        IPFMA_OK 			Registers value has not been modified
 * 			\ref	    IPFMA_E_MISMATCH	Registers value modified
 */
IpFma_Status IpFma_R5f_CompareThreadProcessIdsRegs(IpFma_R5fThreadProcessIdsRegs* expected, IpFma_R5fThreadProcessIdsRegs* actual);

/*	==========================================================================	*/
/*	                      Static Function Definitions                         	*/
/*	==========================================================================	*/

#ifdef __cplusplus
}
#endif

#endif /*	#ifndef IP_FMA_R5F_	*/

/** @} */
