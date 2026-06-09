/*
 * Copyright (C) 2021-2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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

/*
 * \file osal_dm.h
 *
 * OSAL APIs for controlling DM
 *
 */
#ifndef OSAL_DM_H
#define OSAL_DM_H

#include <types/short_types.h>

/**
 * \brief Disable interrupts used by DM firmware
 *
 * \return None
 */
void osal_dm_disable_interrupt(void);


/**
 * \brief Enable interrupts used by DM firmware
 *
 * \return None
 */
void osal_dm_enable_interrupt(void);

/**
 * \brief Suspend DM firmware
 *
 * \return None
 */
void osal_suspend_dm(void);

/**
 * \brief Resume DM firmware
 *
 * \return CSL_PASS if it is a success, else error
 */
u32 osal_resume_dm(void);

/**
 * \brief Resume DM firmware
 *
 * \return SystemP_SUCCESS if success, else error
 */
s32 osal_dm_copy_fs_stub_from_ddr_to_local_mem(void);

/**
 * \brief Suspend the DM application
 *
 * \return SystemP_SUCCESS if success, else error
 */
s32 osal_suspend_dm_application(void);

/**
 * \brief Suspend the DM application
 *
 * \return SystemP_SUCCESS if success, else error
 */
s32 osal_resume_dm_application(void);

#endif /* OSAL_DM_H */
