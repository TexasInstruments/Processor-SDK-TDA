/*
 *  Copyright (C) 2023-25 Texas Instruments Incorporated
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

/* FUNCTION DEF:   uint32_t UtilsP_ReadATCMRegionRegister(void) */
    .global UtilsP_ReadATCMRegionRegister
    .arm
UtilsP_ReadATCMRegionRegister:
    MRC     p15, #0, r0, c9, c1, #1        // Read ATCM Register Register
    BX      lr

/* FUNCTION DEF:   uint32_t UtilsP_ReadBTCMRegionRegister(void) */
    .global UtilsP_ReadBTCMRegionRegister
    .arm
UtilsP_ReadBTCMRegionRegister:
    MRC     p15, #0, r0, c9, c1, #0        // Read BTCM Register Register
    BX      lr

/* FUNCTION DEF:   void_t UtilsP_WriteATCMRegionRegister(uint32_t value) */
    .global UtilsP_WriteATCMRegionRegister
    .arm
UtilsP_WriteATCMRegionRegister:
    MCR     p15, #0, r0, c9, c1, #1        // Write ATCM Register Register
    BX      lr

/* FUNCTION DEF:   void  UtilsP_WriteBTCMRegionRegister(uint32_t value) */
    .global UtilsP_WriteBTCMRegionRegister
    .arm
UtilsP_WriteBTCMRegionRegister:
    MCR     p15, #0, r0, c9, c1, #0        // Write BTCM Register Register
    BX      lr

/* FUNCTION DEF:   void UtilsP_CacheEnableHardErrCache (uint32_t enable) */
    .global UtilsP_CacheEnableHardErrCache
    .arm
UtilsP_CacheEnableHardErrCache:
    MRC     p15, #0, r1, c15, c0, #0       // Read Secondary Auxiliary Control Register
    CMP     r0, #0
    BEQ     armR5HardErrCache_disable
    BIC     r1, r1, #(1<<22)               // Enabled. The cache logic recovers from some hard errors.
    B       armR5HardErrCache_set
armR5HardErrCache_disable:
    ORR     r1, r1, #(1<<22)               // Disabled. Most hard errors in the caches are fatal.
armR5HardErrCache_set:
    DSB
    MCR     p15, #0, r1, c15, c0, #0       // Write Secondary Auxiliary Control Register
    BX      lr

/* FUNCTION DEF: void UtilsP_Dsb( void ) */
    .global UtilsP_Dsb
    .arm
UtilsP_Dsb:
    DSB
    BX      lr

        .end
