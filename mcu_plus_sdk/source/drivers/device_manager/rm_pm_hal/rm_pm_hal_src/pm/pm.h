/*
 * System Firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 * Copyright (C) 2018-2026, Texas Instruments Incorporated
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

#ifndef PM_H
#define PM_H

#include <types/short_types.h>

/**
 *  \brief  Set Clock Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 set_clock_handler(u32 *msg_recv);

/**
 *  \brief  Get Clock Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_clock_handler(u32 *msg_recv);

/**
 *  \brief  Set Clock Parent Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 set_clock_parent_handler(u32 *msg_recv);

/**
 *  \brief  Get Clock Parent Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_clock_parent_handler(u32 *msg_recv);

/**
 *  \brief  Get Number of Clock Parents Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_num_clock_parents_handler(u32 *msg_recv);

/**
 *  \brief  Set clock frequency Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 set_freq_handler(u32 *msg_recv);

/**
 *  \brief  Query clock frequency Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 query_freq_handler(u32 *msg_recv);

/**
 *  \brief  Get clock frequency Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_freq_handler(u32 *msg_recv);

/**
 *  \brief  Set clock SSC Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 set_ssc_handler(u32 *msg_recv);

/**
 *  \brief  Get clock SSC Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_ssc_handler(u32 *msg_recv);

/**
 *  \brief  Set Device State Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 set_device_handler(u32 *msg_recv);

/**
 *  \brief  Get Device State Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_device_handler(u32 *msg_recv);

/**
 *  \brief  Get Multiple Device States Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 get_device_multiple_handler(u32 *msg_recv);

/**
 *  \brief  Set Device Resets Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 set_device_resets_handler(u32 *msg_recv);

/**
 *  \brief  Drop Device Ref counts Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 device_drop_powerup_ref_handler(u32 *msg_recv);

/**
 *  \brief  Wake up reason Get Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 wake_reason_handler(u32 *msg_recv);

/**
 *  \brief  Wake reset Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 wake_reset_handler(u32 *msg_recv);

/**
 *  \brief  Enable WDT Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 enable_wdt_handler(u32 *msg_recv);

/**
 *  \brief  Device Goodbye Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 goodbye_handler(u32 *msg_recv);

/**
 *  \brief  System Reset Handler PM Function.
 *
 *  \param  msg_recv Pointer to the recieved and returned message. Same buffer
 *                   used.
 *  \return ret      SUCCESS if the API executed successfully.
 *                   EFAIL   if the API failed to execute.
 */
s32 sys_reset_handler(u32 *msg_recv);

#endif /* PM_H */
