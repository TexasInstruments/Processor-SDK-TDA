/*
 *  Copyright (C) 2025 Texas Instruments Incorporated
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

/*
 *------------------------------------------------------------------------------
 * vIrqHandler responds to an interrupt by calling an external interrupt handler
 * and then yielding if required.
 *------------------------------------------------------------------------------
 */
    .global vApplicationFiqHandlerHook
    .arm
    .type vApplicationFiqHandlerHook, %function

    .global vApplicationFiqHandlerHook_c

vApplicationFiqHandlerHook:
    PUSH {LR}
    /* Call the interrupt handler. */
    BL      vApplicationFiqHandlerHook_c    /* This handler is provided in the .c file */
    POP {LR}


    .global vApplicationDataAbortHook
    .arm
    .type vApplicationDataAbortHook, %function

    .global vApplicationDataAbortHook_c

vApplicationDataAbortHook:
    PUSH {LR}
    /* Call the interrupt handler. */
    BL      vApplicationDataAbortHook_c     /* This handler is provided in the .c file */
    POP {LR}


    .global vApplicationPrefetchAbortHook
    .arm
    .type vApplicationPrefetchAbortHook, %function

    .global vApplicationPrefetchAbortHook_c

vApplicationPrefetchAbortHook:
    PUSH {LR}
    /* Call the interrupt handler. */
    BL      vApplicationPrefetchAbortHook_c  /* This handler is provided in the .c file */
    POP {LR}

    .global vApplicationAbort
    .arm
    .type vApplicationAbort, %function

    .global vApplicationAbort_c

vApplicationAbort:
    PUSH {LR}
    /* Call the interrupt handler. */
    BL      vApplicationAbort_c             /* This handler is provided in the .c file */
    POP {LR}


    .global vApplicationUndefAbortHook
    .arm
    .type vApplicationUndefAbortHook, %function

    .global vApplicationUndefAbortHook_c

vApplicationUndefAbortHook:
    PUSH {LR}
    /* Call the interrupt handler. */
    BL      vApplicationUndefAbortHook_c             /* This handler is provided in the .c file */
    POP {LR}