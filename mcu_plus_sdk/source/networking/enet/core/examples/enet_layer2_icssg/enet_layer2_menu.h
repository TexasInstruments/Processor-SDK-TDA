/*
 * Copyright (C) 2024-2026 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ENET_LAYER2_MENU_H_
#define ENET_LAYER2_MENU_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "enet_layer2_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/*!
 * \brief Return true if any peripheral context is a switch build.
 *
 * \param perCtxt     Array of peripheral contexts
 * \param numPerCtxts Number of valid entries in perCtxt
 */
bool EnetLayer2_isSwitchBuild(const EnetMp_PerCtxt *perCtxt,
                              uint32_t numPerCtxts);

/*!
 * \brief Display the switch configuration menu and dispatch to the selected
 *        sub-option (link speed, forwarding mode, or per-queue forwarding mode).
 *
 * \param perCtxt     Array of peripheral contexts to configure
 * \param numPerCtxts Number of valid entries in perCtxt
 * \param coreId      Core id to use for IOCTL calls
 */
void EnetLayer2_showSwitchConfigMenu(EnetMp_PerCtxt *perCtxt,
                                      uint32_t numPerCtxts,
                                      uint32_t coreId);

/*!
 * \brief Display link speed configuration menu and apply the selected speed.
 *
 * Prompts for PHY port and link speed (1G/100M/10M), and configures the PHY
 * via MDIO to negotiate the selected speed on the chosen context.
 *
 * \param perCtxt     Array of peripheral contexts to configure
 * \param numPerCtxts Number of valid entries in perCtxt
 */
void EnetLayer2_showLinkSpeedMenu(EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts);

/*!
 * \brief Display forwarding mode configuration menu and apply the selected
 *        mode to all priority queues of every MAC port on the chosen context.
 *
 * \param perCtxt     Array of peripheral contexts to configure
 * \param numPerCtxts Number of valid entries in perCtxt
 * \param coreId      Core id to use for IOCTL calls
 */
void EnetLayer2_showForwardModeMenu(EnetMp_PerCtxt *perCtxt,
                                     uint32_t numPerCtxts,
                                     uint32_t coreId);

/*!
 * \brief Display forwarding mode configuration menu for a single priority
 *        queue, leaving the other queues on the port unchanged.
 *
 * \param perCtxt     Array of peripheral contexts to configure
 * \param numPerCtxts Number of valid entries in perCtxt
 * \param coreId      Core id to use for IOCTL calls
 */
void EnetLayer2_showQueueForwardModeMenu(EnetMp_PerCtxt *perCtxt,
                                          uint32_t numPerCtxts,
                                          uint32_t coreId);

#ifdef __cplusplus
}
#endif

#endif /* ENET_LAYER2_MENU_H_ */
