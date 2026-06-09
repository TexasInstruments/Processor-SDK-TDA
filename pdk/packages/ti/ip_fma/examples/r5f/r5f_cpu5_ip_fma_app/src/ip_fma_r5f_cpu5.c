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
#include <ip_fma_r5f_cpu5.h>

#include <ti/csl/csl_types.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>

/* ========================================================================== */
/*                           File Scope Variables                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static uint32_t gUndefExceptionInvokedCounter;

/*	==========================================================================	*/
/*	                          Macros & Typedefs                               	*/
/*	==========================================================================	*/

/* THUMB INSTRUCTION ENCODINGS */
#define THUMB_DIV_INSTR_MASK                    (0xfff0f0f0U)
#define THUMB_UDIV_INSTRUCTION                  (0xfbb0f0f0U)
#define THUMB_SDIV_INSTRUCTION                  (0xfb90f0f0U)

#define THUMB_DOUBLE_INSTR_MASK                 (0x0ff00f00U)
#define THUMB_VMOVE_DOUBLE_INSTRUCTION          (0x0eb00b00U)
#define THUMB_VLDR_DOUBLE_INSTRUCTION           (0x0d900b00U)
#define THUMB_DIVIDE_DOUBLE_INSTRUCTION         (0x0e800b00U)
#define THUMB_MULTIPLY_DOUBLE_INSTRUCTION       (0x0e200b00U)
#define THUMB_ADD_DOUBLE_INSTRUCTION            (0x0e300b00U)
#define THUMB_PUSH_DOUBLE_INSTRUCTION           (0x0d200b00U)

/* ARM INSTRUCTION ENCODINGS */
#define ARM_DIV_INSTR_MASK                      (0x0ff0f0f0U)
#define ARM_UDIV_INSTRUCTION                    (0x0730f010U)
#define ARM_SDIV_INSTRUCTION                    (0x0710f010U)

#define ARM_MOVE_DOUBLE_VALUE_INSTR_MASK        (0x0fff0f00U)
#define ARM_VMOVE_DOUBLE_CONST_INSTRUCTION      (0x0eb20b00U)
#define ARM_VMOVE_REG_DOUBLE_INSTRUCTION        (0x0eb00b00U)
#define ARM_VLDR_DOUBLE_INSTRUCTION             (0x0d9d0b00U)
#define ARM_VSTR_DOUBLE_INSTRUCTION             (0x0d8d0b00U)

#define ARM_DOUBLE_INSTR_MASK                   (0x0ff00f00U)
#define ARM_DIVIDE_DOUBLE_INSTRUCTION           (0x0e800b00U)
#define ARM_MULTIPLY_DOUBLE_INSTRUCTION         (0x0e200b00U)
#define ARM_ADD_DOUBLE_INSTRUCTION              (0x0e300b00U)

/* SPSR register */
#define SPSR_BIT_0_MASK                         ((uint32_t) 0x1U << 0U)
#define SPSR_BIT_1_MASK                         ((uint32_t) 0x1U << 1U)
#define SPSR_BIT_2_MASK                         ((uint32_t) 0x1U << 2U)
#define SPSR_BIT_3_MASK                         ((uint32_t) 0x1U << 3U)
#define SPSR_BIT_4_MASK                         ((uint32_t) 0x1U << 4U)
#define SPSR_BIT_10_MASK                        ((uint32_t) 0x1U << 10U)
#define SPSR_BIT_11_MASK                        ((uint32_t) 0x1U << 11U)
#define SPSR_BIT_12_MASK                        ((uint32_t) 0x1U << 12U)
#define SPSR_BIT_25_MASK                        ((uint32_t) 0x1U << 25U)
#define SPSR_BIT_26_MASK                        ((uint32_t) 0x1U << 26U)

#define SPSR_THUMB_MODE_BIT_MASK                (0x1U << 5U)

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

static bool IpFma_R5f_IsDivInstruction(uint32_t instruction)
{
    bool divInstr = false;
    /* Compare the undefined instruction with real instructions encodings. */
    /* Mask is used to remove register(destination and source registers) mentions from the instruction encoding. */
    if ((THUMB_UDIV_INSTRUCTION == (instruction & THUMB_DIV_INSTR_MASK)) || \
        (THUMB_SDIV_INSTRUCTION == (instruction & THUMB_DIV_INSTR_MASK)) || \
        (ARM_UDIV_INSTRUCTION   == (instruction & ARM_DIV_INSTR_MASK)) || \
        (ARM_SDIV_INSTRUCTION   == (instruction & ARM_DIV_INSTR_MASK)))
    {
        divInstr = true;
    }

    return divInstr;
}

static bool IpFma_R5f_IsDoubleOpInstruction(uint32_t instruction)
{
    bool doubleOperationInstr = false;
    /* Compare the undefined instruction with real instructions encodings. */
    /* Mask is used to remove register(destination and source registers) mentions from the instruction encoding. */
    if ((THUMB_VMOVE_DOUBLE_INSTRUCTION    == (instruction & THUMB_DOUBLE_INSTR_MASK)) || \
        (THUMB_DIVIDE_DOUBLE_INSTRUCTION   == (instruction & THUMB_DOUBLE_INSTR_MASK)) || \
        (THUMB_MULTIPLY_DOUBLE_INSTRUCTION == (instruction & THUMB_DOUBLE_INSTR_MASK)) || \
        (THUMB_ADD_DOUBLE_INSTRUCTION      == (instruction & THUMB_DOUBLE_INSTR_MASK)) || \
        (THUMB_PUSH_DOUBLE_INSTRUCTION     == (instruction & THUMB_DOUBLE_INSTR_MASK)) || \
        (THUMB_VLDR_DOUBLE_INSTRUCTION     == (instruction & THUMB_DOUBLE_INSTR_MASK)) || \
        (ARM_VMOVE_DOUBLE_CONST_INSTRUCTION == (instruction & ARM_MOVE_DOUBLE_VALUE_INSTR_MASK)) || \
        (ARM_VMOVE_REG_DOUBLE_INSTRUCTION   == (instruction & ARM_MOVE_DOUBLE_VALUE_INSTR_MASK)) || \
        (ARM_VLDR_DOUBLE_INSTRUCTION        == (instruction & ARM_MOVE_DOUBLE_VALUE_INSTR_MASK)) || \
        (ARM_VSTR_DOUBLE_INSTRUCTION        == (instruction & ARM_MOVE_DOUBLE_VALUE_INSTR_MASK)) || \
        (ARM_DIVIDE_DOUBLE_INSTRUCTION      == (instruction & ARM_DOUBLE_INSTR_MASK)) || \
        (ARM_MULTIPLY_DOUBLE_INSTRUCTION    == (instruction & ARM_DOUBLE_INSTR_MASK)) || \
        (ARM_ADD_DOUBLE_INSTRUCTION         == (instruction & ARM_DOUBLE_INSTR_MASK)))
    {
        doubleOperationInstr = true;
    }

    return doubleOperationInstr;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void IpFma_R5f_UpdateUndefInstrExptnHandler(CSL_R5ExptnHandlers* handlers,
                                                  void* undefInstrExptnHandler,
                                                  IpFma_R5f_UndefInstrExptnHandlerArgs* undefInstrExptnHandlerArgs)
{
    handlers->udefExptnHandler = undefInstrExptnHandler;
    handlers->udefExptnHandlerArgs = undefInstrExptnHandlerArgs;
}

uint32_t IpFma_R5f_GetExceptionInvocationCount()
{
    return gUndefExceptionInvokedCounter;
}

__attribute__((naked))
void IpFma_R5f_UndefInstrExptnHandler(IpFma_R5f_UndefInstrExptnHandlerArgs* args)
{
    /* When an exception is invoked, assembly exception handler wrapper is ran first, it saves SPSR and LR onto stack. */
    /* Register r0 contains args parameter, push r1, r2 on stack. Then place SPSR stack ptr into r1 and LR stack ptr into r2. */
    __asm__ volatile(
        "push {r1, r2, lr}\n"
        "add r1, sp, #12\n"
        "add r2, sp, #16\n"
        "blx  IpFma_R5f_UndefInstrExptnHandlerCallback\n"
        "pop  {r1, r2, pc}\n"
    );
}

void IpFma_R5f_UndefInstrExptnHandlerCallback(IpFma_R5f_UndefInstrExptnHandlerArgs* args,
                                                    uint32_t* spsrRegPtr,
                                                    uint32_t* linkRegisterPtr)
{
    uint32_t undefinedInstruction = 0U;
    uint32_t undefinedInstructionSizeBytes = 0U;

    IpFma_R5f_InstructionSet instructionSet = (*spsrRegPtr & SPSR_THUMB_MODE_BIT_MASK) >> 5U;
    /* Check if the faulting instruction belongs to ARM or THUMB instruction set. */
    if (IPFMA_R5F_INSTRSET_ARM_MODE == instructionSet)
    {
        /* When the T bit is clear, the processor executes in ARM state and increments the LR by 4 bytes. */
        *linkRegisterPtr = *linkRegisterPtr - ARM_INSTR_SIZE;
        undefinedInstruction = *((uint32_t*) *linkRegisterPtr);
        undefinedInstructionSizeBytes = ARM_INSTR_SIZE;
    }
    else
    {
        /* When the T bit is set, the processor executes in Thumb state and increments the LR by 2 bytes. */
        *linkRegisterPtr = *linkRegisterPtr - THUMB_INSTR_SIZE;
        uint16_t thumbInstruction = *((uint16_t*) (*linkRegisterPtr));

        if ((thumbInstruction >> 11) > 28U)
        {
            /* Undefined thumb instruction is 32 bits in size. This means it is Thumb-2 instruction. */
            /* Thumb-2 instructions are split into two halfwords: high part is at the lower address. */
            undefinedInstruction = (thumbInstruction << 16) | (*((uint16_t*) (*linkRegisterPtr + THUMB_INSTR_SIZE)));
            undefinedInstructionSizeBytes = THUMB_INSTR_SIZE_32;
        }
        else
        {
            /* Undefined instruction is 16 bits in size. */
            undefinedInstruction = thumbInstruction;
            undefinedInstructionSizeBytes = THUMB_INSTR_SIZE;
        }
    }

    /* Log undefined exception details. */
    args->address = *linkRegisterPtr;
    args->undefInstrEncoding = undefinedInstruction;
    args->instrSet = instructionSet;

    /* Fix the issue causing the undef exception or skip the instruction if it is not part of the covered instructions. */
    if (IpFma_R5f_IsDivInstruction(undefinedInstruction))
    {
        /* UDIV or SDIV fault */
        /* Enable division by zero without invoking undefined exception. */
        IpFma_R5fCoreCtrlRegs coreCtrlRegs;
        IpFma_R5f_GetCoreCtrlRegs(&coreCtrlRegs);
        uint32_t systemControlRegisterModified = (coreCtrlRegs.sctlr & ~((uint32_t) R5F_APP_SCTLR_REG_DZ_MASK));
        IpFma_R5f_write_sctlr(systemControlRegisterModified);
    }
    else if (IpFma_R5f_IsDoubleOpInstruction(undefinedInstruction))
    {
        /* Double precision operation fault. */
        uint32_t cpacrReg = IpFma_R5f_read_cpacr();
        cpacrReg |= R5F_APP_CPACR_FPU_ACCESS_PERMISSION_MASK;
        IpFma_R5f_write_cpacr(cpacrReg);
    }
    else
    {
        /* Skip the instruction. */
        if (IPFMA_R5F_INSTRSET_ARM_MODE == instructionSet)
        {
            *linkRegisterPtr = *linkRegisterPtr + ARM_INSTR_SIZE;
        }
        else if ((IPFMA_R5F_INSTRSET_THUMB_MODE == instructionSet) && (undefinedInstructionSizeBytes ==  THUMB_INSTR_SIZE_32))
        {
            *linkRegisterPtr = *linkRegisterPtr + THUMB_INSTR_SIZE_32;
        }
        else if ((IPFMA_R5F_INSTRSET_THUMB_MODE == instructionSet) && (undefinedInstructionSizeBytes ==  THUMB_INSTR_SIZE))
        {
            *linkRegisterPtr = *linkRegisterPtr + THUMB_INSTR_SIZE;
        }
    }

    /* Advance the IT execution state bits in the SPSR before restoring SPSR to CPSR. */
    uint32_t spsrMasked = 0U;
    spsrMasked |= ((*spsrRegPtr & SPSR_BIT_25_MASK) >> 25U);   // Shift bit 25 on bit 0 position
    spsrMasked |= ((*spsrRegPtr & SPSR_BIT_26_MASK) >> 25U);   // Shift bit 26 on bit 1 position
    spsrMasked |= ((*spsrRegPtr & SPSR_BIT_10_MASK) >> 8U);    // Shift bit 10 on bit 2 position
    spsrMasked |= ((*spsrRegPtr & SPSR_BIT_11_MASK) >> 8U);    // Shift bit 11 on bit 3 position

    if (0 != spsrMasked)
    {
        spsrMasked = spsrMasked << 1;

        /* Clear bits 10,11,12,25 and 26 in SPSR register */
        *spsrRegPtr &= ~(SPSR_BIT_10_MASK | SPSR_BIT_11_MASK | SPSR_BIT_12_MASK | SPSR_BIT_25_MASK | SPSR_BIT_26_MASK);

        *spsrRegPtr |= ((spsrMasked & SPSR_BIT_0_MASK) << 25U);  /* Move bit spsrMasked[0] to bit SPSR[25] */
        *spsrRegPtr |= ((spsrMasked & SPSR_BIT_1_MASK) << 25U);  /* Move bit spsrMasked[1] to bit SPSR[26] */
        *spsrRegPtr |= ((spsrMasked & SPSR_BIT_2_MASK) << 8U);   /* Move bit spsrMasked[2] to bit SPSR[10] */
        *spsrRegPtr |= ((spsrMasked & SPSR_BIT_3_MASK) << 8U);   /* Move bit spsrMasked[3] to bit SPSR[11] */
        *spsrRegPtr |= ((spsrMasked & SPSR_BIT_4_MASK) << 8U);   /* Move bit spsrMasked[4] to bit SPSR[12] */
    }
    else
    {
        /* SPSR[15:12] bits are set to zero. */
        *spsrRegPtr &= ~(0xf << 12);
    }

    /* Increment the undefined exception counter. */
    gUndefExceptionInvokedCounter++;
}

/** @} */