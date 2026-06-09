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
 *  \ingroup  IP_FMA_R5F_CPU3
 *  \defgroup IP_FMA_R5F_CPU3_IMPLEMENTATION R5F MPU fault handler API implementation.
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f_cpu3.c
 *
 *  \brief    R5F MPU specific wrappers for fault handler.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ip_fma_r5f.h>
#include <ip_fma_r5f_cpu3.h>

#include <ti/csl/csl_types.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>

/* ========================================================================== */
/*                           File Scope Variables                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static uint32_t gExceptionInvokedCounter;

/*	==========================================================================	*/
/*	                          Macros & Typedefs                               	*/
/*	==========================================================================	*/

/* Fault status types */
#define FAULT_STATUS_TYPE_S_BIT                         (0x1U << 10U)
#define FAULT_STATUS_TYPE_S_BIT_SHIFT                   (6U)
#define FAULT_STATUS_TYPE_MASK                          (15U)

/* Fault status cause mask */
#define FAULT_STATUS_DFSR_READ_NOT_WRITE_BIT_MASK       (0x1U << 11U)

/* External abort cause mask */
#define FAULT_STATUS_EXTERNAL_ABORT_BIT_MASK            (0x1U << 12U)

/* SPSR register */
#define SPSR_THUMB_MODE_BIT_MASK                        (0x1U << 5U)

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

_DEFINE_COPROCR_READ_FUNC(dfsr, p15, 0, c5, c0, 0)	    /*  Data Fault Status Register   */
_DEFINE_COPROCR_READ_FUNC(dfar, p15, 0, c6, c0, 0)	    /*  Data Fault Address Register  */

_DEFINE_COPROCR_READ_FUNC(ifsr, p15, 0, c5, c0, 1)	    /*  Instruction Fault Status Register   */
_DEFINE_COPROCR_READ_FUNC(ifar, p15, 0, c6, c0, 2)	    /*  Instruction Fault Address Register  */

_DEFINE_COPROCR_WRITE_FUNC(dfsr, p15, 0, c5, c0, 0)	    /*  Data Fault Status Register   */
_DEFINE_COPROCR_WRITE_FUNC(dfar, p15, 0, c6, c0, 0)	    /*  Data Fault Address Register  */

_DEFINE_COPROCR_WRITE_FUNC(ifsr, p15, 0, c5, c0, 1)	    /*  Instruction Fault Status Register   */
_DEFINE_COPROCR_WRITE_FUNC(ifar, p15, 0, c6, c0, 2)	    /*  Instruction Fault Address Register  */

_DEFINE_COPROCR_READ_FUNC(adfsr, p15, 0, c5, c1, 0)	    /*  Auxiliary Data Fault Status Register   */
_DEFINE_COPROCR_READ_FUNC(aifsr, p15, 0, c5, c1, 1)	    /*  Auxiliary Instruction Fault Status Register   */

_DEFINE_COPROCR_WRITE_FUNC(adfsr, p15, 0, c5, c1, 0)	/*  Auxiliary Data Fault Status Register   */
_DEFINE_COPROCR_WRITE_FUNC(aifsr, p15, 0, c5, c1, 1)	/*  Auxiliary Instruction Fault Status Register   */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void IpFma_R5f_UpdateDataAbortExptnHandler(CSL_R5ExptnHandlers* handlers,
                                           void* dataAbortExptnHandler,
                                           IpFma_R5fDataAbortExptnHandlerArgs* dataAbortExptnHandlerArgs)
{
    handlers->dabtExptnHandler = dataAbortExptnHandler;
    handlers->dabtExptnHandlerArgs = dataAbortExptnHandlerArgs;
}

void IpFma_R5f_UpdatePrefetchAbortExptnHandler(CSL_R5ExptnHandlers* handlers,
                                               void* prefetchAbortExptnHandler,
                                               IpFma_R5fPrefetchAbortExptnHandlerArgs* prefetchAbortExptnHandlerArgs)
{
    handlers->pabtExptnHandler = prefetchAbortExptnHandler;
    handlers->pabtExptnHandlerArgs = prefetchAbortExptnHandlerArgs;
}

uint32_t IpFma_R5f_GetExceptionInvocationCount()
{
    return gExceptionInvokedCounter;
}

__attribute__((naked))
void IpFma_R5f_DataAbortExptnHandler(IpFma_R5fDataAbortExptnHandlerArgs* args)
{
    /* When an exception is invoked, assembly exception handler wrapper is ran first, it saves SPSR and LR to stack. */
    /* Register r0 contains args parameter, push r1, r2 on stack. Then place SPSR stack ptr into r1 and LR stack ptr into r2. */
    __asm__ volatile(
        "push {r1, r2, lr}\n"
        "add r1, sp, #12\n"
        "add r2, sp, #16\n"
        "blx  IpFma_R5f_DataAbortExptnHandlerCallback\n"
        "pop  {r1, r2, pc}\n"
    );
}

void IpFma_R5f_DataAbortExptnHandlerCallback(IpFma_R5fDataAbortExptnHandlerArgs* args,
                                             uint32_t* spsrReg,
                                             uint32_t* linkRegisterStackPtr)
{
    /**
     * Currently the assembly exception wrapper supports only THUMB instruction set but doesn't support THUMB-2.
     * The following logic checks if the failed instruction is 4 bytes in size(Thumb-2 instruction),
     * if so increment the LR by additional 2 bytes.
     */
    IpFma_R5f_InstructionSet instructionSet = (*spsrReg & SPSR_THUMB_MODE_BIT_MASK) >> 5U;
    /* Check if the faulting instruction belongs to THUMB instruction set (T bit is set == Thumb mode). */
    if (IPFMA_R5F_INSTRSET_THUMB_MODE == instructionSet)
    {
        /* The processor executes in Thumb state and increments the LR by 4 bytes and decrements 2 bytes in the assembly wrapper. */
        /* So we decrement additional 2 bytes to read the fault instruction. */
        *linkRegisterStackPtr = *linkRegisterStackPtr - THUMB_INSTR_SIZE;
        uint16_t thumbInstruction = *((uint16_t*) (*linkRegisterStackPtr));

        if ((thumbInstruction >> 11) > 28U)
        {
            /* Failed thumb instruction is 4 bytes in size. This means it is Thumb-2 instruction. */
            /* Increment the address by 4 bytes so the LR points to the next instruction. */
            *linkRegisterStackPtr = *linkRegisterStackPtr + THUMB_INSTR_SIZE_32;
        }
        else
        {
            /* Failed thumb instruction is 4 bytes in size. This means it is Thumb or Thumb-2 instruction. */
            /* Increment the address by 2 bytes so the LR points to the next instruction. */
            *linkRegisterStackPtr = *linkRegisterStackPtr + THUMB_INSTR_SIZE;
        }
    }

    uint32_t dfsrReg = IpFma_R5f_read_dfsr();
    uint32_t dfarReg = (uint32_t) IpFma_R5f_read_dfar();

    /* Type of fault field is split across two bit ranges: bits [0–3] and bit 10. */
    uint32_t typeOfExptn = (FAULT_STATUS_TYPE_S_BIT & dfsrReg) >> FAULT_STATUS_TYPE_S_BIT_SHIFT | \
                           (FAULT_STATUS_TYPE_MASK  & dfsrReg);

    uint32_t causeOfExptn = (FAULT_STATUS_DFSR_READ_NOT_WRITE_BIT_MASK & dfsrReg);
    uint32_t causeOfExtExptn = (FAULT_STATUS_EXTERNAL_ABORT_BIT_MASK & dfsrReg);

    args->type = typeOfExptn;
    args->address = dfarReg;
    args->cause = causeOfExptn;
    args->externalAbortCause = causeOfExtExptn;
    args->auxiliaryFaultStatusReg = IpFma_R5f_read_adfsr();

    /* Reset fault status and address registers. */
    IpFma_R5f_write_dfsr(0);
    IpFma_R5f_write_dfar(0);
    IpFma_R5f_write_adfsr(0);

    /* Increment the exception counter. */
    gExceptionInvokedCounter++;

    __asm__ volatile ("DSB");  // Data Synchronization Barrier
    __asm__ volatile ("ISB");  // Instruction Synchronization Barrier
}

void IpFma_R5f_PrefetchAbortExptnHandler(IpFma_R5fPrefetchAbortExptnHandlerArgs* args)
{
    uint32_t ifsrReg = IpFma_R5f_read_ifsr();
    uint32_t ifarReg = (uint32_t) IpFma_R5f_read_ifar();

    /* Type of fault field is split across two bit ranges: bits [0–3] and bit 10. */
    uint32_t typeOfExptn = (FAULT_STATUS_TYPE_S_BIT & ifsrReg) >> FAULT_STATUS_TYPE_S_BIT_SHIFT | \
                           (FAULT_STATUS_TYPE_MASK  & ifsrReg);

    uint32_t causeOfExtExptn = (FAULT_STATUS_EXTERNAL_ABORT_BIT_MASK & ifsrReg);

    args->type = typeOfExptn;
    args->address = ifarReg;
    args->externalAbortCause = causeOfExtExptn;
    args->auxiliaryFaultStatusReg = IpFma_R5f_read_aifsr();

    /* Reset fault status and address registers */
    IpFma_R5f_write_ifsr(0);
    IpFma_R5f_write_ifar(0);
    IpFma_R5f_write_aifsr(0);

    /**
     * When prefetch abort handler finishes executing, code flow continues at the same instruction
     * that caused the exception, this invokes the exception handler again into an infinite loop.
     * In the code below we reverse the cause of the forcibly caused exception.
     */
    if (FAULT_STATUS_TYPE_PERMISSION == typeOfExptn)
    {
        /**
         * This exception type in this example will be caused due to the
         * "execute never bit" of the region access control register of MPU test region 8.
         */
        uint32_t regionNum = 8U;
        IpFma_R5f_write_rgnr(regionNum);

        IpFma_R5fMpuRegionRegs mpuRegionRegs;
        IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);

        uint32_t regionAccessControlRegister = mpuRegionRegs.racr;
        regionAccessControlRegister &= ~(CSL_ARM_R5_MPU_REGION_AC_XN_MASK);

        IpFma_R5f_write_racr(regionAccessControlRegister);
    }
    else if (FAULT_STATUS_TYPE_BACKGROUND == typeOfExptn)
    {
        /**
         * Background exception type in this example will be caused due to disabling
         * three overlapping subregions. This leaves the function address outside any MPU region,
         * which causes a prefetch abort exception when the function is executed.
         */
        uint32_t regionNum = 0U;
        uint32_t regionSizeRegister = 0U;
        IpFma_R5fMpuRegionRegs mpuRegionRegs;

        /* Enable subregion 6 of region 0. */
        regionNum = 0U;
        IpFma_R5f_write_rgnr(regionNum);
        IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);

        regionSizeRegister = mpuRegionRegs.rser;
        regionSizeRegister &= ~(CSL_ARM_R5_MPU_SUB_REGION_6_DISABLE << CSL_ARM_R5_MPU_REGION_SZEN_SRD_SHIFT);
        IpFma_R5f_write_rser(regionSizeRegister);


        /* Enable subregion 4 of region 4. */
        regionNum = 4U;
        IpFma_R5f_write_rgnr(regionNum);
        IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);

        regionSizeRegister = mpuRegionRegs.rser;
        regionSizeRegister &= ~(CSL_ARM_R5_MPU_SUB_REGION_4_DISABLE << CSL_ARM_R5_MPU_REGION_SZEN_SRD_SHIFT);
        IpFma_R5f_write_rser(regionSizeRegister);


        /* Enable subregion  of test region 8. */
        regionNum = 8U;
        IpFma_R5f_write_rgnr(regionNum);
        IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);

        regionSizeRegister = mpuRegionRegs.rser;
        regionSizeRegister &= ~(CSL_ARM_R5_MPU_SUB_REGION_0_DISABLE << CSL_ARM_R5_MPU_REGION_SZEN_SRD_SHIFT);
        IpFma_R5f_write_rser(regionSizeRegister);
    }

    /* Increment the exception counter. */
    gExceptionInvokedCounter++;

    __asm__ volatile ("DSB");  // Data Synchronization Barrier
    __asm__ volatile ("ISB");  // Instruction Synchronization Barrier
}

/** @} */