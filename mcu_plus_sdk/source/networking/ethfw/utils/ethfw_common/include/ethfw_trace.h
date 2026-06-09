/*
 *  Copyright (c) Texas Instruments Incorporated 2023-2024
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

/*!
 * \file  ethfw_trace.h
 *
 * \brief Ethernet Firmware trace funtionality interface.
 */

#ifndef ETHFW_TRACE_H_
#define ETHFW_TRACE_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


#include <stdint.h>
#include <utils/ethfw_common/include/ethfw_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_UTILS
 * \defgroup ETHFW_UTILS_TRACE Ethernet Firmware Trace API
 *
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief Line terminator to be used by all EthFwTrace macros */
#ifndef ETHFW_CFG_TRACE_LINE_TERM
#define ETHFW_CFG_TRACE_LINE_TERM             "\r\n"
#endif

/*! \brief Trace print buffer length. */
#define ETHFW_CFG_PRINT_BUF_LEN               (1024U)

/*!
 * \anchor EthFwTrace_TraceLevels
 * \name Build-Time Trace Levels
 *
 * Ethernet Firmware supports two types of trace levels: build-time and runtime.
 * Build time trace level is specified via \ref ETHFW_CFG_TRACE_LEVEL
 * and can be any of the levels in this group.  The runtime trace level is set
 * through \ref EthFwTrace_setLevel().
 *
 * The runtime trace level should be set equal to or less than the build-time
 * trace level.
 *
 * @{
 */
/*! \brief All traces disabled at build-time. */
#define ETHFW_CFG_TRACE_LEVEL_NONE            (0U)

/*! \brief Build-time error level. */
#define ETHFW_CFG_TRACE_LEVEL_ERROR           (1U)

/*! \brief Build-time warning level. */
#define ETHFW_CFG_TRACE_LEVEL_WARN            (2U)

/*! \brief Build-time information level. */
#define ETHFW_CFG_TRACE_LEVEL_INFO            (3U)

/*! \brief Build-time debug level. */
#define ETHFW_CFG_TRACE_LEVEL_DEBUG           (4U)

/*! \brief Build-time verbose level. */
#define ETHFW_CFG_TRACE_LEVEL_VERBOSE         (5U)
/*! @} */

/*!
 * \anchor EthFwTrace_Formats
 * \name Trace Formats
 *
 * The following trace formats are supported:
 * - Function prefix: `<func>: string`
 * - File and line number prefix: `<func>: <line>: string`
 * - File, line number and function prefix: `<file>: <line>: <func>: string`
 *
 * Any of these trace formats can also be prefixed with a timestamp.
 *
 * @{
 */
/*! \brief Trace prefix: "<mod>: string" */
#define ETHFW_CFG_TRACE_FORMAT_DFLT           (0U)

/*! \brief Trace prefix: "<func>: string" */
#define ETHFW_CFG_TRACE_FORMAT_FUNC           (1U)

/*! \brief Trace prefix: "<file>: <line>: string" */
#define ETHFW_CFG_TRACE_FORMAT_FILE           (2U)

/*! \brief Trace prefix: "<file>: <line>: <func>: string" */
#define ETHFW_CFG_TRACE_FORMAT_FULL           (3U)

/*! \brief Trace prefix: "<timestamp>: <mod>: string" */
#define ETHFW_CFG_TRACE_FORMAT_DFLT_TS        (4U)

/*! \brief Trace prefix: "<timestamp>: <func>: string" */
#define ETHFW_CFG_TRACE_FORMAT_FUNC_TS        (5U)

/*! \brief Trace prefix: "<timestamp>: <file>: <line>: string" */
#define ETHFW_CFG_TRACE_FORMAT_FILE_TS        (6U)

/*! \brief Trace prefix: "<timestamp>: <file>: <line>: <func>: string" */
#define ETHFW_CFG_TRACE_FORMAT_FULL_TS        (7U)
/*! @} */

/*!
 * \anchor EthFwTrace_Configs
 * \name EthFw Trace Configuration Parameters
 *
 * The default values of the EthFw Trace configuration parameters if none is
 * provided.
 *
 * @{
 */
/*! \brief Default trace level if none is set. */
#ifndef ETHFW_CFG_TRACE_LEVEL
#define ETHFW_CFG_TRACE_LEVEL                 (ETHFW_CFG_TRACE_LEVEL_INFO)
#endif

#if (ETHFW_CFG_TRACE_LEVEL > ETHFW_CFG_TRACE_LEVEL_VERBOSE)
#error "Invalid Ethernet Firmware trace level"
#endif

/*! \brief Default trace format if none is specified. */
#ifndef ETHFW_CFG_TRACE_FORMAT
#define ETHFW_CFG_TRACE_FORMAT                (ETHFW_CFG_TRACE_FORMAT_DFLT)
#endif
/*! @} */

/*!
 * \anchor EthFwTrace_ErrorCode
 * \name Ethernet Firmware Trace Error Code
 *
 * EthFw Trace is capable of generating unique error codes from module id
 * (CpswProxy, VEPA, VLAN, etc), line number (where error code was detected)
 * and status (error value reported by the module).
 *
 * The error code is a 32-bit value which is generated as follows:
 * - Bits 31:28 - Module's major number (see `ETHFW_TRACE_MOD_ID`)
 * - Bits 27:20 - Module's minor number (see `ETHFW_TRACE_MOD_ID`)
 * - Bits 19:8  - Line number where error was reported
 * - Bits 7:0   - Status value (positive value)
 *
 * Major Number |     Module         | File location
 * -------------|--------------------|------------------------
 *            1 | ETHFW Server       | ethremotecfg/server
 *            2 | ETHFW Client       | ethremotecfg/client
 *            3 | ETHFW Common Utils | utils/ethfw_common
 *            4 | Board Library      | utils/board
 *            5 | Other Utils        | utils/\*
 *            6 | Server App         | apps/app_remoteswitchcfg_server
 *            7 | Client App         | apps/app_remoteswitchcfg_client
 *
 * Each individual source file in the locations shown in previous table is
 * considered a _module_ and is tagged with a unique `ETHFW_TRACE_MOD_ID`,
 * denoting the module's major (4-bit) and minor (16-bit) numbers.
 *
 * @{
 */
/*! \brief Module id offset in the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_OFFSET        (20U)

/*! \brief Module id (major number) offset in the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MAJ_OFFSET    (28U)

/*! \brief Module id (minor number) offset in the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MIN_OFFSET    (20U)

/*! \brief Line number offset in the error code value */
#define ETHFW_TRACE_ERRCODE_LINE_OFFSET       (8U)

/*! \brief Status offset in the error code value */
#define ETHFW_TRACE_ERRCODE_STATUS_OFFSET     (0U)

/*! \brief Module id mask in the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MASK          (0xFFF00000U)

/*! \brief Module id (major number) mask in the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MAJ_MASK      (0xF0000000U)

/*! \brief Module id (minor number) mask in the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MIN_MASK      (0x0FF00000U)

/*! \brief Line number mask in the error code value */
#define ETHFW_TRACE_ERRCODE_LINE_MASK         (0x000FFF00U)

/*! \brief Status mask in the error code value */
#define ETHFW_TRACE_ERRCODE_STATUS_MASK       (0x000000FFU)

/*! \brief Helper macro to extract the module id from the error code value */
#define ETHFW_TRACE_ERRCODE_MOD(x)            (((x) & ETHFW_TRACE_ERRCODE_MOD_MASK) \
                                               >> ETHFW_TRACE_ERRCODE_MOD_OFFSET)

/*! \brief Helper macro to extract the module id (major number) from the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MAJ(x)        (((x) & ETHFW_TRACE_ERRCODE_MOD_MAJ_MASK) \
                                               >> ETHFW_TRACE_ERRCODE_MOD_MAJ_OFFSET)

/*! \brief Helper macro to extract the module id (minor number) from the error code value */
#define ETHFW_TRACE_ERRCODE_MOD_MIN(x)        (((x) & ETHFW_TRACE_ERRCODE_MOD_MIN_MASK) \
                                               >> ETHFW_TRACE_ERRCODE_MOD_MIN_OFFSET)

/*! \brief Helper macro to extract the line number from the error code value */
#define ETHFW_TRACE_ERRCODE_LINE(x)           (((x) & ETHFW_TRACE_ERRCODE_LINE_MASK) \
                                               >> ETHFW_TRACE_ERRCODE_LINE_OFFSET)

/*! \brief Helper macro to extract the status value from the error code value */
#define ETHFW_TRACE_ERRCODE_STATUS(x)         (((x) & ETHFW_TRACE_ERRCODE_STATUS_MASK) \
                                               >> ETHFW_TRACE_ERRCODE_STATUS_OFFSET)
/*! @} */

/*!
 * \anchor EthFwTrace_Macros
 * \name EthFw Trace Macros
 *
 * Macros used throughout Ethernet Firmware modules to log/trace errors, warnings,
 * info and debug messages.
 *
 * @{
 */
/*!
 *\brief Trace module name.
 *
 * Trace module name which can be set individually per source code file by
 * defining it before this header file is included. If not defined, it will
 * default to "ETHFW".
 */
#ifndef ETHFWTRACE_MOD_NAME
#define ETHFWTRACE_MOD_NAME                   "ETHFW"
#endif

/*! \brief Trace prefix type */
#if (ETHFW_CFG_TRACE_LEVEL > ETHFW_CFG_TRACE_LEVEL_NONE)
#if (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_DFLT) || \
    (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_DFLT_TS)
/* Trace prefix: "<mod>: fmt" */
#define ETHFWTRACE_trace(globalLevel, level, status, fmt, ...)  \
    EthFwTrace_trace((globalLevel), (level), ETHFWTRACE_MOD_ID, \
                     __FILE__, __LINE__, __func__, (status),    \
                     "%s: " fmt,                                \
                     ETHFWTRACE_MOD_NAME, ## __VA_ARGS__)
/* Error trace prefix: "<mod>: <func>: fmt" */
#define ETHFWTRACE_errTrace(globalLevel, level, status, fmt, ...) \
    EthFwTrace_trace((globalLevel), (level), ETHFWTRACE_MOD_ID,   \
                     __FILE__, __LINE__, __func__, (status),      \
                     "%s: %s: " fmt,                              \
                     ETHFWTRACE_MOD_NAME, __func__, ## __VA_ARGS__)
#elif (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_FUNC) || \
    (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_FUNC_TS)
/* Trace prefix: "<func>: fmt" */
#define ETHFWTRACE_trace(globalLevel, level, status, fmt, ...)  \
    EthFwTrace_trace((globalLevel), (level), ETHFWTRACE_MOD_ID, \
                     __FILE__, __LINE__, __func__, (status),    \
                     "%s: " fmt,                                \
                     __func__, ## __VA_ARGS__)
/* Error trace prefix */
#define ETHFWTRACE_errTrace                  ETHFWTRACE_trace
#elif (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_FILE) || \
      (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_FILE_TS)
/* Trace prefix: "<file>: <line>: fmt" */
#define ETHFWTRACE_trace(globalLevel, level, status, fmt, ...)  \
    EthFwTrace_trace((globalLevel), (level), ETHFWTRACE_MOD_ID, \
                     __FILE__, __LINE__, __func__, (status),    \
                     "%s: %d: " fmt,                            \
                     __FILE__, __LINE__, ## __VA_ARGS__)
/* Error trace prefix */
#define ETHFWTRACE_errTrace                  ETHFWTRACE_trace
#elif (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_FULL) || \
      (ETHFW_CFG_TRACE_FORMAT == ETHFW_CFG_TRACE_FORMAT_FULL_TS)
/* Trace prefix: "<file>: <line>: <func>: fmt" */
#define ETHFWTRACE_trace(globalLevel, level, status, fmt, ...)  \
    EthFwTrace_trace((globalLevel), (level), ETHFWTRACE_MOD_ID, \
                     __FILE__, __LINE__, __func__,              \
                     (status),                                  \
                     "%s: %d: %s: " fmt,                        \
                     __FILE__, __LINE__, __func__, ## __VA_ARGS__)
/* Error trace prefix */
#define ETHFWTRACE_errTrace                  ETHFWTRACE_trace
#else
#error "Invalid Ethernet Firmware trace format"
#endif
#else /* ETHFW_CFG_TRACE_LEVEL_NONE */
#define ETHFWTRACE_trace(globalLevel, level, status, fmt, ...)
#endif

/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_ERROR level.
 */
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_ERROR)
#define ETHFWTRACE_ERR(status, fmt, ...) ETHFWTRACE_errTrace(gEthFwTrace_runtimeLevel,\
                                                             ETHFW_TRACE_ERROR,       \
                                                             status,                  \
                                                             fmt,                     \
                                                             ## __VA_ARGS__)
/* TODO: Replace the below #ifdef with a method applicable for both C & C++ */
/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_ERROR level if
 *        a condition is met.
 */
#ifdef __cplusplus
#define ETHFWTRACE_ERR_IF(cond, ...) ((cond) ? ETHFWTRACE_ERR(__VA_ARGS__) : void())
#else
#define ETHFWTRACE_ERR_IF(cond, ...) ((cond) ? ETHFWTRACE_ERR(__VA_ARGS__) : 0U)
#endif
#else
#define ETHFWTRACE_ERR(fmt, ...)
#define ETHFWTRACE_ERR_IF(cond, ...)
#endif

/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_WARN level
 */
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_WARN)
#define ETHFWTRACE_WARN(fmt, ...) ETHFWTRACE_trace(gEthFwTrace_runtimeLevel,\
                                                   ETHFW_TRACE_WARN,        \
                                                   0U,                      \
                                                   fmt,                     \
                                                   ## __VA_ARGS__)
/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_WARN level if
 *        a condition is met.
 */
#ifdef __cplusplus
#define ETHFWTRACE_WARN_IF(cond, ...) ((cond) ? ETHFWTRACE_WARN(__VA_ARGS__) : void())
#else
#define ETHFWTRACE_WARN_IF(cond, ...) ((cond) ? ETHFWTRACE_WARN(__VA_ARGS__) : 0U)
#endif
#else
#define ETHFWTRACE_WARN(fmt, ...)
#define ETHFWTRACE_WARN_IF(cond, ...)
#endif

/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_INFO level
 *
 * Traces with this level should give only important informational messages
 * to the user, which typically they don't occur very often (i.e. "NIMU is
 * ready", "PHY n link is up").
 * This trace level may be enabled by default.
 */
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_INFO)
#define ETHFWTRACE_INFO(fmt, ...) ETHFWTRACE_trace(gEthFwTrace_runtimeLevel,\
                                                   ETHFW_TRACE_INFO,        \
                                                   0U,                      \
                                                   fmt,                     \
                                                   ## __VA_ARGS__)
/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_INFO level if
 *        a condition is met.
 */
#ifdef __cplusplus
#define ETHFWTRACE_INFO_IF(cond, ...) ((cond) ? ETHFWTRACE_INFO(__VA_ARGS__) : void())
#else
#define ETHFWTRACE_INFO_IF(cond, ...) ((cond) ? ETHFWTRACE_INFO(__VA_ARGS__) : 0U)
#endif
#else
#define ETHFWTRACE_INFO(fmt, ...)
#define ETHFWTRACE_INFO_IF(cond, ...)
#endif

/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_DEBUG level
 *
 * Traces with this level will provide the user further information about
 * operations taking place in Ethernet Firmware.
 */
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
#define ETHFWTRACE_DBG(fmt, ...) ETHFWTRACE_trace(gEthFwTrace_runtimeLevel,\
                                                  ETHFW_TRACE_DEBUG,       \
                                                  0U,                      \
                                                  fmt,                     \
                                                  ## __VA_ARGS__)
/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_DEBUG level if
 *        a condition is met.
 */
#ifdef __cplusplus
#define ETHFWTRACE_DBG_IF(cond, ...) ((cond) ? ETHFWTRACE_DBG(__VA_ARGS__) : void())
#else
#define ETHFWTRACE_DBG_IF(cond, ...) ((cond) ? ETHFWTRACE_DBG(__VA_ARGS__) : 0U)
#endif
#else
#define ETHFWTRACE_DBG(fmt, ...)
#define ETHFWTRACE_DBG_IF(cond, ...)
#endif

/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_VERBOSE level
 *
 * Traces with this level will provide even more information and much more
 * often than the DEBUG level.
 *
 * Enabling this trace level is likely going to flood with messages, so
 * developers must ensure that their debug messages that occur often enough
 * are set with VERBOSE level.
 */
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_VERBOSE)
#define ETHFWTRACE_VERBOSE(fmt, ...) ETHFWTRACE_trace(gEthFwTrace_runtimeLevel,\
                                                      ETHFW_TRACE_VERBOSE,     \
                                                      0U,                      \
                                                      fmt,                     \
                                                      ## __VA_ARGS__)
/*!
 * \brief Helper macro to add trace message with #ETHFW_TRACE_VERBOSE level if
 *        a condition is met.
 */
#ifdef __cplusplus
#define ETHFWTRACE_VERBOSE_IF(cond, ...) ((cond) ? ETHFWTRACE_VERBOSE(__VA_ARGS__) : void())
#else
#define ETHFWTRACE_VERBOSE_IF(cond, ...) ((cond) ? ETHFWTRACE_VERBOSE(__VA_ARGS__) : 0U)
#endif
#else
#define ETHFWTRACE_VERBOSE(fmt, ...)
#define ETHFWTRACE_VERBOSE_IF(cond, ...)
#endif

/*!
 * \brief Variable declaration helper macro to avoid unused variable error
 *        (-Werror=unused-variable) when variable is used in TRACE and when
 *        corresponding trace level is not enabled.
 */
#define ETHFWTRACE_VAR(var)                    ((var) = (var))
/*! @} */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Enumerates the types of trace level.
 */
typedef enum EthFwTrace_TraceLevel_e
{
    /*! All traces are disabled at runtime */
    ETHFW_TRACE_NONE    = 0U,

    /*! Error trace level */
    ETHFW_TRACE_ERROR   = 1U,

    /*! Warning trace level */
    ETHFW_TRACE_WARN    = 2U,

    /*! Info trace level: enables only important informational messages for the
     * user.
     *
     * The amount of info logs is not invasive in nature so this trace level
     * may be enabled at init time. */
    ETHFW_TRACE_INFO    = 3U,

    /*! Debug trace level: enables further information messages about operations
     * taking place in Ethernet Firmware.
     *
     * The debug level should be enabled by the user on a need basis (i.e.
     * for debugging or tracing execution flow, etc) as the number of messages
     * will increase considerably with respect to #ETHFW_TRACE_INFO level.
     *
     * This trace level should be enabled at runtime only in debug builds. */
    ETHFW_TRACE_DEBUG   = 4U,

    /*! Verbose trace level: enables even further messages about operations
     * taking place in Ethernet Firmware that are periodic in nature or simply
     * happen very often during normal execution.
     *
     * The amount of messages will increase drastically when the verbose level
     * is enabled, so it's recommended to set it only if really needed.
     *
     * This trace level should be enabled at runtime only in debug builds. */
    ETHFW_TRACE_VERBOSE = 5U,
} EthFwTrace_TraceLevel;

/*!
 * \brief Print function used for traces.
 *
 * Trace print callback function.  It must be provided by application at
 * init time.
 */
typedef void (*EthFwTrace_Print)(const char *format, ...);

/*!
 * \brief Callback function used to get trace timestamps.
 *
 * Callback function called when \ref ETHFW_CFG_TRACE_FORMAT is set to
 * \ref ETHFW_CFG_TRACE_FORMAT_DFLT_TS, \ref ETHFW_CFG_TRACE_FORMAT_FUNC_TS,
 * \ref ETHFW_CFG_TRACE_FORMAT_FILE_TS or \ref ETHFW_CFG_TRACE_FORMAT_FULL_TS.
 *
 * The timestamp value must be in microseconds.
 */
typedef uint64_t (*EthFwTrace_TraceTsFunc)(void);

/*!
 * \brief Callback function called to report errors.
 *
 * Callback function called by EthFwTrace when an error has been reported by
 * any  module in Ethernet Firmware.  A unique 32-bit error code is passed to
 * the callback, which encodes information about the module where error occurred,
 * line of code and the status value.
 *
 * Application can use the helper macros \ref ETHFW_TRACE_ERRCODE_MOD,
 * \ref ETHFW_TRACE_ERRCODE_MOD_MAJ, \ref ETHFW_TRACE_ERRCODE_MOD_MIN,
 * \ref ETHFW_TRACE_ERRCODE_LINE and \ref ETHFW_TRACE_ERRCODE_STATUS to decode
 * the error code.
 */
typedef void (*EthFwTrace_ExtTraceFunc)(uint32_t errCode);


/*!
 * \brief EthFw trace configuration parameters.
 *
 * Init time trace configuration parameters.
 */
typedef struct EthFwTrace_Cfg_s
{
    /*! Function used to print traces */
    EthFwTrace_Print print;

    /*! Trace timestamp provider */
    EthFwTrace_TraceTsFunc traceTsFunc;

    /*! Extended trace function to be called with a 32-bit unique error code
     *  only for error traces */
    EthFwTrace_ExtTraceFunc extTraceFunc;
} EthFwTrace_Cfg;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/*! \brief Runtime log level */
extern EthFwTrace_TraceLevel gEthFwTrace_runtimeLevel;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize ETHFW trace functionality.
 *
 * Initializes ETHFW trace functionality with the provided callback functions,
 * including the function used to print traces for logging purpose, timestamp
 * provider and an extended trace function which can be used to log errors
 * for tracing purposes (i.e. breadcrum of unique error codes).
 *
 * \param cfg                 Trace configuration parameters
 *
 * \returns ETHFW_SOK if trace functionality was correctly initialized, or a
 *          negative error in case of failure.
 */
int32_t EthFwTrace_init(const EthFwTrace_Cfg *cfg);

/*!
 * \brief Deinitialize ETHFW trace functionality.
 *
 * Deinitializes trace functionality.  No trace macros or functions should be
 * called after deinitialization is done.
 */
void EthFwTrace_deinit(void);

/*!
 * \brief Set runtime trace level.
 *
 * Set the Ethetnet Firmware's runtime trace level.
 *
 * \param level    Trace level to be enabled
 * \return Previous trace level
 */
EthFwTrace_TraceLevel EthFwTrace_setLevel(EthFwTrace_TraceLevel level);

/*!
 * \brief Get runtime trace level.
 *
 * Get the Ethernet Firmware's runtime trace level.
 *
 * \return Current trace level
 */
EthFwTrace_TraceLevel EthFwTrace_getLevel(void);

/*!
 * \brief Set trace timestamp function.
 *
 * Sets the function to be used to timestamp EthFw traces.  The timestamp
 * values returned by this function must be in microseconds.  If application
 * provides a NULL function pointer, all traces will have a timestamp value
 * of 0.
 *
 * The timestamping function will be called only when \ref ETHFW_CFG_TRACE_FORMAT
 * is set to \ref ETHFW_CFG_TRACE_FORMAT_DFLT_TS, \ref ETHFW_CFG_TRACE_FORMAT_FUNC_TS,
 * \ref ETHFW_CFG_TRACE_FORMAT_FILE_TS or ETHFW_CFG_TRACE_FORMAT_FULL_TS.
 *
 * \param func   Trace timestamping function
 */
void EthFwTrace_setTsFunc(EthFwTrace_TraceTsFunc func);

/*!
 * \brief Set extended trace funtion.
 *
 * Sets the extended trace function called when any module in Ethernet Firmware
 * has reported an error.  A 32-bit unique error code is passed to this callback
 * function.
 *
 * \param func   Extended trace callback function
 */
void EthFwTrace_setExtTraceFunc(EthFwTrace_ExtTraceFunc func);

/*!
 * \brief Print a message.
 *
 * Print messages using common print function provided at \ref EthFwTrace_init()
 * via \ref EthFwTrace_Cfg::print.
 *
 * This function can be provided as the _print_ function for drivers such as
 * Enet or UDMA.
 *
 * \param fmt            Print string
 */
void EthFwTrace_print(const char *fmt,
                      ...);

/*!
 * \brief Log a trace message if log level is enabled
 *
 * Log trace messages for log levels that are enabled at runtime.
 *
 * \param globalLevel    Trace module global level
 * \param level          Trace level intended to be logged
 * \param mod            Module id
 * \param file           File name
 * \param line           Line number
 * \param func           Function name
 * \param status         Status code (see \ref EthFwTrace_ErrorCode).
 * \param fmt            Print string
 */
void EthFwTrace_trace(EthFwTrace_TraceLevel globalLevel,
                      EthFwTrace_TraceLevel level,
                      uint32_t mod,
                      const char *file,
                      uint32_t line,
                      const char *func,
                      uint32_t status,
                      const char *fmt,
                      ...);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_TRACE_H_ */

