/*
 * Copyright (c) 2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \file soc_devgrps.h
 *
 *  \brief Software defined Device groups (devgrps) for J722S device.
 *
 */
#ifndef SOC_J722S_DEVGRPS_H
#define SOC_J722S_DEVGRPS_H

#include <types/devgrps.h>

/**
 * Set of MCU domain peripherals to initialize at boot and use during runtime.
 */
#define SOC_DEVGRP_J722S_MCU_WAKEUP DEVGRP_01

/**
 * Set of Main domain peripherals to initialize at boot and use during
 * runtime.
 */
#define SOC_DEVGRP_J722S_MAIN DEVGRP_00

/**
 * Set of peripherals to initialize for TIFS internal usage
 */
#define SOC_DEVGRP_J722S_TIFS_INTERNAL DEVGRP_DMSC

/**
 * Set of peripherals to initialize for HSM internal usage
 */
#define SOC_DEVGRP_J722S_HSM_INTERNAL DEVGRP_HSM


#endif /* SOC_J722S_DEVGRPS_H */
