/*
 *  Copyright (c) Texas Instruments Incorporated 2023
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
 * \file  ethfw_utils.h
 *
 * \brief This file contains Ethernet Firmware common helper macros.
 */

#ifndef ETHFW_UTILS_H_
#define ETHFW_UTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#if defined(__KLOCWORK__) || defined(__cplusplus)
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_UTILS
 * \defgroup ETHFW_UTILS_MISC Miscellaneous Utilities
 *
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief Whether assertions are enabled or not */
#define ETHFW_CFG_ASSERT_ENABLE                (1)

/*! \brief MAC address length */
#define ETHFW_MAC_ADDR_LEN                     (6U)

/*! \brief Unused variable */
#define ETHFW_UNUSED(x)                        (x = x)

/*! \brief Align a value by performing a round-up operation */
#define ETHFW_UTILS_ALIGN(x, y)                ((((x) + ((y) - 1)) / (y)) * (y))

/*! \brief Macro to determine if an address is aligned to a given size */
#define ETHFW_UTILS_IS_ALIGNED(addr, alignSz)  (((uintptr_t)addr & ((alignSz) - 1U)) == 0U)

/*! \brief Macro to copy arrays. They must be of the same size */
#define ETHFW_UTILS_ARRAY_COPY(dst, src)                                 \
    do                                                                   \
    {                                                                    \
        /* dst argument of macro should be array and not pointer.*/      \
        ETHFW_UTILS_COMPILETIME_ASSERT(sizeof(dst) != sizeof(uintptr_t)); \
        memcpy(dst, src, (ETHFW_ARRAYSIZE(dst) * sizeof(dst[0])));  \
    } while (0)

/*! \brief Assertion */
#if ETHFW_CFG_ASSERT_ENABLE
#if defined(__KLOCWORK__) || defined(__cplusplus)
#define EthFw_assert(cond, ...)            do { if (!(cond)) abort(); } while (0)
#else /* !defined(__KLOCWORK__) && !defined(__cplusplus) */
#define EthFw_assert(cond, ...)                              \
    do {                                                     \
        ETHFWTRACE_ERR_IF(!(bool)(cond), ETHFW_EFAIL, __VA_ARGS__); \
        EthFwUtils_assertLocal((bool)(cond),                 \
                              (const char *)# cond,          \
                              (const char *)__FILE__,        \
                              (int32_t)__LINE__);            \
    } while (0)
#endif /* defined(__KLOCWORK__) || defined(__cplusplus) */
#else /* !ETHFW_CFG_ASSERT_ENABLE */
#define EthFw_assert(cond, ...)           (void)(cond)
#endif /* ETHFW_CFG_ASSERT_ENABLE */

/*! \brief Development-time assertion. */
#if ETHFW_CFG_ASSERT_ENABLE
#if defined(__KLOCWORK__) || defined(__cplusplus)
#define EthFw_devAssert(cond, ...)         do { if (!(cond)) abort(); } while (0)
#else /* !defined(__KLOCWORK__) && !defined(__cplusplus) */
#define EthFw_devAssert(cond, ...)                            \
    do {                                                     \
        ETHFWTRACE_ERR_IF(!(bool)(cond), ETHFW_EFAIL, __VA_ARGS__); \
        EthFwUtils_assertLocal((bool)(cond),                 \
                              (const char *)# cond,          \
                              (const char *)__FILE__,        \
                              (int32_t)__LINE__);            \
    } while (0)
#endif /* defined(__KLOCWORK__) || defined(__cplusplus) */
#else /* !ETHFW_CFG_ASSERT_ENABLE || !ETHFW_CFG_IS_ON(DEV_ERROR) */
#define EthFw_devAssert(cond, fmt, ...)        (void)(cond)
#endif /* ETHFW_CFG_ASSERT_ENABLE && ETHFW_CFG_IS_ON(DEV_ERROR) */

/*!
 * \brief Compile-time assertion.
 */
#define ETHFW_UTILS_COMPILETIME_ASSERT(cond)                \
    do {                                                    \
        typedef char ErrorCheck[((cond) == true) ? 1 : -1]; \
        ErrorCheck a = {0};                                 \
        a[0U] = a[0U];                                      \
    } while (0)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void appLogPrintf(const char *format, ...);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

#if ETHFW_CFG_ASSERT_ENABLE && !defined(__KLOCWORK__)
static inline void EthFwUtils_assertLocal(bool condition,
                                          const char *str,
                                          const char *fileName,
                                          int32_t lineNum)
{
    volatile static bool gEthFwAssertWaitInLoop = true;

    if (!condition)
    {
        appLogPrintf("Assertion @ Line: %d in %s: %s\n", lineNum, fileName, str);
#ifdef QNX_OS
        gEthFwAssertWaitInLoop = false;
#endif
        while (gEthFwAssertWaitInLoop)
        {
            /* Do nothing */
        }
    }
}
#endif

static inline void EthFwUtils_copyMacAddr(uint8_t *dst,
                                          const uint8_t *src)
{
    memcpy(dst, src, ETHFW_MAC_ADDR_LEN);
}

static inline bool EthFwUtils_cmpMacAddr(const uint8_t *addr1,
                                         const uint8_t *addr2)
{
    return (memcmp(addr1, addr2, ETHFW_MAC_ADDR_LEN) == 0U);
}

static inline bool EthFwUtils_isMcastAddr(const uint8_t *addr)
{
    return ((addr[0U] & 1U) == 1U);
}

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_UTILS_H_ */
