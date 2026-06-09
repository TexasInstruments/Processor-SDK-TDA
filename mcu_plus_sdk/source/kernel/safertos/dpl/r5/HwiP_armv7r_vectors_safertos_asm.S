/*
 *  Copyright (C) 2018-2025 Texas Instruments Incorporated
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

    .text
    .arm
    .global     _vectors
    .align 2
    .type   _vectors,%function
    .section    .vectors, "ax", %progbits

/* Entry point needs to be nambed as _vectors. This is because _self_reset_start
 * Copies "_vectors" into 0x0000000 for WKUP R5 core.
 */
_vectors:
        LDR pc, safeRTOS_resetAddr      /* Reset */
        LDR pc, safeRTOS_undefInstAddr  /* Undefined Instruction */
        LDR pc, safeRTOS_svcAddr        /* Supervisor call */
        LDR pc, safeRTOS_preAbortAddr   /* Abort (prefetch) */
        LDR pc, safeRTOS_dataAbortAddr  /* Abort (data) */
        LDR pc, safeRTOS_rsvdAddr       /* rsvd */
        LDR pc, safeRTOS_irqAddr        /* IRQ */
        LDR pc, safeRTOS_fiqAddr        /* FIQ */

safeRTOS_resetAddr:     .long _c_int00
safeRTOS_undefInstAddr: .long vApplicationUndefAbortHook
safeRTOS_svcAddr:       .long vSafeRTOSSVCHandler
safeRTOS_preAbortAddr:  .long vApplicationPrefetchAbortHook
safeRTOS_dataAbortAddr: .long vApplicationDataAbortHook
safeRTOS_rsvdAddr:      .long vRsvdHandler
safeRTOS_irqAddr:       .long vIrqHandler
safeRTOS_fiqAddr:       .long vApplicationFiqHandlerHook
    .end

