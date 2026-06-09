/*
 * System Firmware Trace layer
 *
 * Debug Trace layer APIs
 *
 * Copyright (C) 2018-2025, Texas Instruments Incorporated
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

#ifndef TRACE_H
#define TRACE_H

#include <config.h>
#include <lib/bitops.h>
#include <types/short_types.h>

#include <lib/trace_protocol.h>

#define TRACE_PRINT_MAX_LENGTH                           255U
#define TRACE_NUM_BUFFER_CONVERSION_SIZE                 ((sizeof(u32) * 8U) + 1U)

#define TRACE_LEVEL_ERR                                  0U
#define TRACE_LEVEL_WARN                                 1U
#define TRACE_LEVEL_INFO                                 2U
#define TRACE_LEVEL_DEBUG                                3U

#ifdef CONFIG_TRACE
/**
 * \brief Set the level of messages to be output.
 *
 * \param trace_level Level of messages to allow to be printed globally.
 */
void trace_set_print_level(u8 trace_level);

/**
 * \brief Prints a plain string to configured trace destinations.
 *
 * \param trace_level The trace level of this message.
 * \param fmt String to output.
 */
void trace_print(u8 trace_level, const u8 *fmt);

/**
 * \brief Prints a plain string with a u32 in hex format to configured
 *        trace destinations.
 *
 * \param trace_level The trace level of this message.
 * \param fmt String to output.
 * \param arg Value to print in hex format after the string.
 */
void trace_print_arg(u8 trace_level, const u8 *fmt, u32 arg);

/**
 * \brief Prints a single character to configured trace destinations.
 *
 * \param ch Character to output.
 */
s32 trace_print_output_char(u8 ch);

/**
 * \brief Prints a character to trace destination, use in secure world ONLY.
 *
 * \param ch Character to output.
 */
s32 trace_print_output_char_core(u8 ch);
/**
 * \brief Provide boardcfg values for trace src_enables and dst_enables
 *
 * \param src_enables Value from boardcfg_dbg_cfg data structure.
 * \param dst_enables Value from boardcfg_dbg_cfg data structure.
 */
void trace_reconfigure(u16 src_enables, u16 dst_enables);

/**
 * \brief Outputs a u32 trace debug value to configured trace destinations.
 *
 * \param channel Output channel to use.
 * \param val Trace debug value to output.
 */
void trace_debug(u8 channel, u32 val);
#else
static inline void trace_print(u8 trace_level __attribute__((unused)), const u8 *fmt __attribute__((unused)))
{
}

static inline s32 trace_print_output_char(u8 ch __attribute__((unused)))
{
	return 0;
}

static inline s32 trace_print_output_char_core(u8 ch __attribute__((unused)))
{
	return 0;
}

static inline void trace_reconfigure(u16 src_enables __attribute__((unused)), u16 dst_enables __attribute__((unused)))
{
}

static inline void trace_debug(u8 channel __attribute__((unused)), u32 val __attribute__((unused)))
{
}
#endif

/**
 * \brief Outputs a u32 trace debug value to configured trace destinations.
 *
 * \param channel Output channel to use.
 * \param val Trace debug value to output.
 *
 * This function should be invoked from privileged code.
 */
void trace_debug_core(u8 channel, u32 val);

/**
 * \brief Return the internal trace subsystem error code.
 *
 * If something inside the trace framework fails during a call to a public
 * API, such as a hardware character output, the error is cached inside the
 * trace framework. This call retrieves the internal error code.
 *
 * \return Trace subsystem error code or SUCCESS if no error has occurred.
 */
s32 trace_get_error(void);

/**
 * \brief Initialize the trace subsystem.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 trace_init(void);

/**
 *   \def TRACE_err(fmt)
 *	  Output a string at trace level TRACE_LEVEL_ERR.
 *   \def TRACE_warn(fmt)
 *	  Output a string at trace level TRACE_LEVEL_WARN.
 *   \def TRACE_info(fmt)
 *	  Output a string at trace level TRACE_LEVEL_INFO.
 *   \def TRACE_debug(fmt)
 *	  Output a string at trace level TRACE_LEVEL_DEBUG.
 *   \def TRACE_err_arg(fmt, arg)
 *	  Output a string at trace level TRACE_LEVEL_ERR with a u32 appended.
 *   \def TRACE_warn_arg(fmt, arg)
 *	  Output a string at trace level TRACE_LEVEL_WARN with a u32 appended.
 *   \def TRACE_info_arg(fmt, arg)
 *	  Output a string at trace level TRACE_LEVEL_INFO with a u32 appended.
 *   \def TRACE_debug_arg(fmt, arg)
 *	  Output a string at trace level TRACE_LEVEL_DEBUG with a u32 appended.
 *   \def bp_trace(action, val)
 *        Output a Baseport Trace Debug Value.
 *   \def sec_trace(action, val)
 *        Output a Security Trace Debug Value.
 *   \def rm_trace(action, val)
 *        Output a Resource Management Trace Debug Value.
 *   \def pm_trace(action, val)
 *        Output a Power Management Trace Debug Value.
 */
#ifdef CONFIG_TRACE
#define TRACE_err(fmt) trace_print(TRACE_LEVEL_ERR, fmt)
#define TRACE_warn(fmt) trace_print(TRACE_LEVEL_WARN, fmt)
#define TRACE_info(fmt) trace_print(TRACE_LEVEL_INFO, fmt)
#define TRACE_debug(fmt) trace_print(TRACE_LEVEL_DEBUG, fmt)
#define TRACE_err_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_ERR, fmt, arg)
#define TRACE_warn_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_WARN, fmt, arg)
#define TRACE_info_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_INFO, fmt, arg)
#define TRACE_debug_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_DEBUG, fmt, arg)

#define bp_trace(action, val) trace_debug(TRACE_DEBUG_CHANNEL_BP, \
					  ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT)	\
					   | (val)))
#define bp_trace_core(action, val) trace_debug_core(TRACE_DEBUG_CHANNEL_BP, \
						    ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT) \
						     | (val)))
#define sec_trace(action, val) trace_debug_core(TRACE_DEBUG_CHANNEL_SEC, \
						((((u32) action) << TRACE_DEBUG_ACTION_SHIFT) \
						 | (val)))
#define rm_trace(action, val) trace_debug(TRACE_DEBUG_CHANNEL_RM, \
					  ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT)	\
					   | (val)))
#define pm_trace(action, val) trace_debug(TRACE_DEBUG_CHANNEL_PM, \
					  ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT)	\
					   | (val)))
#define lpm_trace(action, val) trace_debug(TRACE_DEBUG_CHANNEL_LPM, \
					   ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT) \
					    | (val)))

#define bp_trace_sub(action, sub_action, val) trace_debug(TRACE_DEBUG_CHANNEL_BP, \
							  ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT)	\
							   | (((u32) sub_action) << TRACE_DEBUG_SUB_ACTION_SHIFT)	\
							   | (val)))
#define bp_trace_core_sub(action, sub_action, val) trace_debug_core(TRACE_DEBUG_CHANNEL_BP, \
								    ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT) \
								     | (((u32) sub_action) << TRACE_DEBUG_SUB_ACTION_SHIFT)	\
								     | (val)))
#define sec_trace_sub(action, sub_action, val) trace_debug(TRACE_DEBUG_CHANNEL_SEC, \
							   ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT) \
							    | (((u32) sub_action) << TRACE_DEBUG_SUB_ACTION_SHIFT)	\
							    | (val)))
#define rm_trace_sub(action, sub_action, val) trace_debug(TRACE_DEBUG_CHANNEL_RM, \
							  ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT)	\
							   | (((u32) sub_action) << TRACE_DEBUG_SUB_ACTION_SHIFT)	\
							   | (val)))
#define rm_trace_core_sub(action, sub_action, val) trace_debug_core(TRACE_DEBUG_CHANNEL_RM, \
								    ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT) \
								     | (((u32) sub_action) << TRACE_DEBUG_SUB_ACTION_SHIFT)	  \
								     | (val)))
#define pm_trace_sub(action, sub_action, val) trace_debug(TRACE_DEBUG_CHANNEL_PM, \
							  ((((u32) action) << TRACE_DEBUG_ACTION_SHIFT)	\
							   | (((u32) sub_action) << TRACE_DEBUG_SUB_ACTION_SHIFT)	\
							   | (val)))
#else
#define TRACE_err(fmt) trace_print(TRACE_LEVEL_ERR, fmt)
#define TRACE_warn(fmt) trace_print(TRACE_LEVEL_WARN, fmt)
#define TRACE_info(fmt) trace_print(TRACE_LEVEL_INFO, fmt)
#define TRACE_debug(fmt) trace_print(TRACE_LEVEL_DEBUG, fmt)
#define TRACE_err_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_ERR, fmt, arg)
#define TRACE_warn_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_WARN, fmt, arg)
#define TRACE_info_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_INFO, fmt, arg)
#define TRACE_debug_arg(fmt, arg) trace_print_arg(TRACE_LEVEL_DEBUG, fmt, arg)

#define bp_trace(action, val) trace_debug(action, val)
#define bp_trace_core(action, val) trace_debug(action, val)
#define sec_trace(action, val) trace_debug(action, val)
#define rm_trace(action, val) trace_debug(action, val)
#define pm_trace(action, val) trace_debug(action, val)
#define lpm_trace(action, val) trace_debug(action, val)

#define bp_trace_sub(action, sub_action, val) trace_debug(action + sub_action, val)
#define bp_trace_core_sub(action, sub_action, val) trace_debug(action + sub_action, val)
#define sec_trace_sub(action, sub_action, val) trace_debug(action + sub_action, val)
#define rm_trace_sub(action, sub_action, val) trace_debug(action + sub_action, val)
#define rm_trace_core_sub(action, sub_action, val) trace_debug(action + sub_action, val)
#define pm_trace_sub(action, sub_action, val) trace_debug(action + sub_action, val)
#endif

#endif
