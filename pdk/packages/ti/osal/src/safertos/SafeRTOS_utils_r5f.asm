;
;  Copyright (C) 2018-2021 Texas Instruments Incorporated
;
;  Redistribution and use in source and binary forms, with or without
;  modification, are permitted provided that the following conditions
;  are met:
;
;    Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
;
;    Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the
;    distribution.
;
;    Neither the name of Texas Instruments Incorporated nor the names of
;    its contributors may be used to endorse or promote products derived
;    from this software without specific prior written permission.
;
;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
;  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
;  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
;  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
;  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
;  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
;  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;

;
;  \file   SafeRTOS_Utilsasm.asm
;

        .text
        .arm
        .ref vDataAbort_c
        .ref vPrefetchAbort_c
        .ref vUndefAbort_c
        .ref FaultySP
        .ref FaultyLR
        .ref FaultyGPR
        .sect ".KERNEL_FUNCTION"

;------------------------------------------------------------------------------
; Constant Definitions
;------------------------------------------------------------------------------


ARM_MODE_MASK                           .equ ( 0x1F )
ISR_MODE                                   .equ ( 0x12 )

TRUE                                    .equ ( 1 )
FALSE                                   .equ ( 0 )

;------------------------------------------------------------------------------
    .global ulGetDataFaultStatusRegister
    .global ulGetDataFaultAddressRegister
    .global ulGetInstructionFaultStatusRegister
    .global ulGetInstructionFaultAddressRegister
    .global Osal_getSP
    .global ulGetCPSR

;-------------------------------------------------------------------------------
; portUInt32Type ulGetDataFaultStatusRegister( void )
;-------------------------------------------------------------------------------
ulGetDataFaultStatusRegister:
        MRC     p15, #0, r0, c5, c0, #0
        BX      LR

;-------------------------------------------------------------------------------
; portUInt32Type ulGetDataFaultAddressRegister( void )
;-------------------------------------------------------------------------------
ulGetDataFaultAddressRegister:
        MRC     p15, #0, r0, c6, c0, #0
        BX      LR

;-------------------------------------------------------------------------------
; portUInt32Type ulGetInstructionFaultStatusRegister( void )
;-------------------------------------------------------------------------------
ulGetInstructionFaultStatusRegister:
        MRC     p15, #0, r0, c5, c0, #1
        BX      LR

;-------------------------------------------------------------------------------
; portUInt32Type ulGetInstructionFaultAddressRegister( void )
;-------------------------------------------------------------------------------
ulGetInstructionFaultAddressRegister:
        MRC     p15, #0, r0, c6, c0, #2
        BX      LR

;-------------------------------------------------------------------------------
; portUInt32Type ulGetCPSR( void )
;-------------------------------------------------------------------------------
ulGetCPSR:
        MRS     R0, CPSR
        BX      LR

;-------------------------------------------------------------------------------
; void vApplicationDataAbortHook( void )
;-------------------------------------------------------------------------------
        .global vApplicationDataAbortHook
        .arm

vApplicationDataAbortHook:
    ;  Return to the instruction following the interrupted.
    SUB    lr, lr, #4

    ;  Push the return address and SPSR.
    PUSH    {lr}
    MRS    lr, SPSR
    PUSH    {lr}

    ;  Push used registers.
    PUSH    {r0-r4, r12}

    ; Push R12, as we will use this to index into FaultyGPR
    PUSH {r12}
    ; Push r0 to r11 to FaultyGPR
    LDR r12, faultyGPRConst
    STR r0, [r12], #4
    STR r1, [r12], #4
    STR r2, [r12], #4
    STR r3, [r12], #4
    STR r4, [r12], #4
    STR r5, [r12], #4
    STR r6, [r12], #4
    STR r7, [r12], #4
    STR r8, [r12], #4
    STR r9, [r12], #4
    STR r10, [r12], #4
    STR r11, [r12], #4
    MOV r0, r12
    ; Pop r12 to push it to the FaultyGPR
    POP {r12}
    STR r12, [r0]

    ; Capture the faulty SP
    MOVW r0, faultySPConst
    MOVT r0, faultySPConst
    LDR r0, [r0]
    STR SP, [r0]

    ; Capture the faulty LR
    MOVW r0, faultyLRConst
    MOVT r0, faultyLRConst
    LDR r0, [r0]
    STR LR, [r0]

    ;  Call the interrupt handler.
    LDR    r1, vApplicationDataAbortHandlerConst
    BLX    r1

    ;  Restore used registers, LR and SPSR before  returning.
    POP    {r0-r4, r12}
    POP    {LR}
    MSR    SPSR_cxsf, LR
    POP    {LR}
    MOVS    PC, LR


vApplicationDataAbortHandlerConst: .word vDataAbort_c
faultySPConst: .word FaultySP
faultyLRConst: .word FaultyLR
faultyGPRConst: .word FaultyGPR

;-------------------------------------------------------------------------------
; void vApplicationPrefetchAbortHook( void )
;-------------------------------------------------------------------------------
        .global vApplicationPrefetchAbortHook
        .arm

vApplicationPrefetchAbortHook:
    ;  Return to the instruction following the interrupted.
    SUB    lr, lr, #4

    ;  Push the return address and SPSR.
    PUSH    {lr}
    MRS    lr, SPSR
    PUSH    {lr}

    ;  Push used registers.
    PUSH    {r0-r4, r12}

    ;  Call the exception handler.
    LDR    r1, vApplicationPrefetchAbortHandlerConst
    BLX    r1

;  Restore used registers, LR and SPSR before  returning.
    POP    {r0-r4, r12}
    POP    {LR}
    MSR    SPSR_cxsf, LR
    POP    {LR}
    MOVS    PC, LR

vApplicationPrefetchAbortHandlerConst: .word vPrefetchAbort_c

;-------------------------------------------------------------------------------
; void vApplicationUndefAbortHook( void )
;-------------------------------------------------------------------------------
        .global vApplicationUndefAbortHook
        .arm

vApplicationUndefAbortHook:
    ;  Push the return address and SPSR.
    PUSH    {lr}
    MRS    lr, SPSR
    PUSH    {lr}

    ;  Push used registers.
    PUSH    {r0-r4, r12}

    ;  Call the exception handler.
    LDR    r1, vApplicationUndefAbortHandlerConst
    BLX    r1

;  Restore used registers, LR and SPSR before  returning.
    POP    {r0-r4, r12}
    POP    {LR}
    MSR    SPSR_cxsf, LR
    POP    {LR}
    MOVS    PC, LR

vApplicationUndefAbortHandlerConst: .word vUndefAbort_c

;-------------------------------------------------------------------------------
; uint32_t Osal_getSP( void )
;-------------------------------------------------------------------------------
Osal_getSP:
  MOV   r0, r13
  BX    LR

;-------------------------------------------------------------------------------
; void Osal_disableMPU( void )
;-------------------------------------------------------------------------------
        .sect .startupCode
        .global Osal_disableMPU
Osal_disableMPU:
    MRC     p15, #0, r1, c1, c0, #0
    BIC     r1, r1, #0x1
    DSB
    MCR     p15, #0, r1, c1, c0, #0
    ISB
    BX      lr

;-------------------------------------------------------------------------------
