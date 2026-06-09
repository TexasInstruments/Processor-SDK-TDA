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
 * \file  ethfw_types.h
 *
 * \brief This file contains the basic types using across the Ethernet Firmware,
 *        mostly on server side, but also shared with utils.
 */

#ifndef ETHFW_TYPES_H_
#define ETHFW_TYPES_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup ETHFW_UTILS Ethernet Firmware Common Utilities
 *
 * \brief This section contains APIs for common definitions, helper macros
 *        and other utilities used by ETHFW server and clients.
 *
 * @{
 */
/*! @} */

/*!
 * \ingroup  ETHFW_UTILS
 * \defgroup ETHFW_UTILS_TYPES Common Types
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*!
 * \anchor EthFw_ErrorCodes
 * \name Ethernet Firmware Error Codes
 *
 * Error codes returned by the EthFw driver APIs.
 *
 * @{
 */

/* EthFw driver error codes are same as CSL's and Enet LLD's in order to
 * to maintain consistency */

/*! \brief Success. */
#define ETHFW_SOK                              (0)

/*! \brief Operation in progress. */
#define ETHFW_SINPROGRESS                      (1)

/*! \brief Generic failure error condition (typically caused by hardware). */
#define ETHFW_EFAIL                            (-1)

/*! \brief Bad arguments (i.e. NULL pointer). */
#define ETHFW_EBADARGS                         (-2)

/*! \brief Invalid parameters (i.e. value out-of-range). */
#define ETHFW_EINVALIDPARAMS                   (-3)

/*! \brief Time out while waiting for a given condition to happen. */
#define ETHFW_ETIMEOUT                         (-4)

/*! \brief Allocation failure. */
#define ETHFW_EALLOC                           (-8)

/*! \brief Unexpected condition occurred (sometimes unrecoverable). */
#define ETHFW_EUNEXPECTED                      (-9)

/*! \brief The resource is currently busy performing an operation. */
#define ETHFW_EBUSY                            (-10)

/*! \brief Already open error. */
#define ETHFW_EALREADYOPEN                     (-11)

/*! \brief Operation not permitted. */
#define ETHFW_EPERM                            (-12)

/*! \brief Operation not supported. */
#define ETHFW_ENOTSUPPORTED                    (-13)

/*! \brief Resource not found. */
#define ETHFW_ENOTFOUND                        (-14)

/*! @} */

/*! \brief Macro to get the size of an array. */
#define ETHFW_ARRAYSIZE(x)                    (sizeof(x) / sizeof(x[0]))

/*! \brief Macro to set bit at given bit position. */
#define ETHFW_BIT(n)                           (1U << (n))

/*! \brief Macro to get bit at given bit position. */
#define ETHFW_GET_BIT(val, n)                  (((val) & ETHFW_BIT(n)) >> (n))

/*! \brief Macro to check if bit at given bit position is set. */
#define ETHFW_IS_BIT_SET(val, n)               (((val) & ETHFW_BIT(n)) != 0U)

/*! \brief Macro to check if value is not zero. */
#define ETHFW_NOT_ZERO(val)                    ((uint32_t)0U != (uint32_t)(val))

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

/* None */

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_TYPES_H_ */

