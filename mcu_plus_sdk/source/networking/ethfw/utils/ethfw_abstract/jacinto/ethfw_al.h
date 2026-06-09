/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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
 * \file  ethfw_al.h
 *
 * \brief Ethernet Firmware abstraction layer for Jacinto
 */

#ifndef ETHFW_AL_H_
#define ETHFW_AL_H_
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/osal/osal.h>
#include <ti/drv/ipc/ipc.h>
#include <ti/drv/ipc/include/ipc_rsctypes.h>
#include <ti/drv/enet/include/core/enet_osal.h>

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

#define ETHFWOSAL_WAIT_FOREVER                  (~((uint32_t)0U))
#define ETHFWIPC_WAIT_FOREVER                   (~((uint32_t)0U))

#define ETHFWOSAL_SUCCESS                       ((int32_t )0)
#define ETHFWOSAL_FAILURE                       ((int32_t)-1)
#define ETHFWOSAL_TIMEOUT                       ((int32_t)-2)

/*!
 * \brief Helper macro used to first register IOCTL handler and then invoke the
 *        IOCTL
 *
 * This macro replaces the previous Enet_ioctl function which Issue an operation on the
 * Enet Peripheral.
 *
 * Issues a control operation on the Enet Peripheral.  The IOCTL parameters
 * should be built using the helper macros provided by the driver:
 * - #ENET_IOCTL_SET_NO_ARGS - If the IOCTL command doesn't take any arguments
 * - #ENET_IOCTL_SET_IN_ARGS - If the IOCTL command takes only input arguments
 * - #ENET_IOCTL_SET_OUT_ARGS - If the IOCTL command takes only output arguments
 * - #ENET_IOCTL_SET_INOUT_ARGS - If the IOCTL command takes input and output arguments
 *
 * \param hEnet        Enet driver handle
 * \param coreId       Caller's core id
 * \param ioctlCmd     IOCTL command Id
 * \param prms         IOCTL parameters
 * \param status       return status of the Enet_ioctl.
 *                     #ENET_SOK if operation is synchronous and it was successful.
 *                     #ENET_SINPROGRESS if operation is asynchronous and was initiated sucessfully.
 *                     \ref Enet_ErrorCodes in case of any failure.
 */
#define ENET_IOCTL(hEnet, coreId, ioctlCmd, prms, status)                         \
    do {                                                                          \
        status = Enet_ioctl(hEnet, coreId, ioctlCmd, prms);                       \
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

/* None */

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */


#ifdef __cplusplus
}
#endif

#endif /* ETHFW_AL_H_ */
