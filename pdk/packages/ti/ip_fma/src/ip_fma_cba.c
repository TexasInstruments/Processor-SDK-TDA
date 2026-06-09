/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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

/**
 *  \ingroup  
 *  \defgroup 
 *
 *  @{
 */

/**
 *  \file     ip_fma_cba.c
 *
 *  \brief    
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "ip_fma_cba.h"
#include <ti/csl/src/ip/cbass/V0/cslr_cbass.h>
//
#include <ti/drv/uart/UART_stdio.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \brief Macro to calculate offset of the CBA ISC region_def_control register based on the ISC subgroup
 *        ordinal number (EP).
 */
#define CSL_CBASS_ISC_EP_REGION_DEF_CONTROL(EP)                             (0x00000080U+((EP)*0x400U))

/**
 * \brief Macro to calculate offset of the CBA Firewall Channel control register based on channel ordinal number.
 */
#define CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(CHANNEL)                     (0x00000000U+((CHANNEL)*0x820U))

/**
 * \brief Macro to calculate offset of the CBA Firewall Channel permission register based on the channel ordinal number and 
 *        permission ordinal number.
 */
#define CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(CHANNEL, PERMISSION)      (0x00000004U+((CHANNEL)*0x820U)+((PERMISSION)*0x4U))

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Cba_GetFwRegs(uintptr_t baseAddr, IpFma_CbaFwRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            // region_0
            { CSL_CBASS_FW_EP_REGION_CONTROL(0, 0),         0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 0, 0),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 0, 1),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 0, 2),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_L(0, 0),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_H(0, 0),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_L(0, 0),      0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_H(0, 0),      0u, IPFMA_WIDTH_32 },

            // region_1
            { CSL_CBASS_FW_EP_REGION_CONTROL(0, 1),         0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 1, 0),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 1, 1),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 1, 2),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_L(0, 1),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_H(0, 1),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_L(0, 1),      0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_H(0, 1),      0u, IPFMA_WIDTH_32 },

            // region_2
            { CSL_CBASS_FW_EP_REGION_CONTROL(0, 2),         0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 2, 0),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 2, 1),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 2, 2),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_L(0, 2),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_H(0, 2),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_L(0, 2),      0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_H(0, 2),      0u, IPFMA_WIDTH_32 },

            // region_3
            { CSL_CBASS_FW_EP_REGION_CONTROL(0, 3),         0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 3, 0),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 3, 1),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_PERMISSION(0, 3, 2),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_L(0, 3),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_START_ADDR_H(0, 3),    0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_L(0, 3),      0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FW_EP_REGION_END_ADDR_H(0, 3),      0u, IPFMA_WIDTH_32 }
        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, 
                                    (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                out->region[i].control           = registerDescriptorsArr[(i*8)+0].value;
                for (uint8_t j = 0; j < 3; j++)
                {
                    out->region[i].permission[j] = registerDescriptorsArr[(i*8)+(j+1)].value;
                }
                out->region[i].start_address_l    = registerDescriptorsArr[(i*8)+4].value;
                out->region[i].start_address_h    = registerDescriptorsArr[(i*8)+5].value;                
                out->region[i].end_address_l      = registerDescriptorsArr[(i*8)+6].value;
                out->region[i].end_address_h      = registerDescriptorsArr[(i*8)+7].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Cba_CompareFwRegs(const IpFma_CbaFwRegs* expected,
                                     const IpFma_CbaFwRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        for (uint8_t i = 0; i < 1; i++)
        {
            if ((IPFMA_OK == status) && (expected->region[i].control != actual->region[i].control))
            {
                status = IPFMA_E_MISMATCH;
            }
            for (uint8_t j = 0; j < 3; j++)
            {
                if((IPFMA_OK == status) && (expected->region[i].permission[j] != actual->region[i].permission[j]))
                {
                    status = IPFMA_E_MISMATCH;
                }
            }
            if ((IPFMA_OK == status) && (expected->region[i].start_address_l != actual->region[i].start_address_l))
            {
                status = IPFMA_E_MISMATCH;
            }
            if ((IPFMA_OK == status) && (expected->region[i].start_address_h != actual->region[i].start_address_h))
            {
                status = IPFMA_E_MISMATCH;
            }
            if ((IPFMA_OK == status) && (expected->region[i].end_address_l != actual->region[i].end_address_l))
            {
                status = IPFMA_E_MISMATCH;
            }
            if ((IPFMA_OK == status) && (expected->region[i].end_address_h != actual->region[i].end_address_h))
            {
                status = IPFMA_E_MISMATCH;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Cba_GetFwchRegs(uintptr_t baseAddr, IpFma_CbaFwchRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] = 
        {
            // region_0_ch_0
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(0),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(0, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(0, 1),  0u, IPFMA_WIDTH_32 },
            
            // region_0_ch_1
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(1),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(1, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(1, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_2
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(2),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(2, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(2, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_3
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(3),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(3, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(3, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_4
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(4),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(4, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(4, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_5
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(5),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(5, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(5, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_6
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(6),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(6, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(6, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_7
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(7),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(7, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(7, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_8
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(8),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(8, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(8, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_9
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(9),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(9, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(9, 1),  0u, IPFMA_WIDTH_32 },

            // region_0_ch_10
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(10),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(10, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(10, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_11
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(11),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(11, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(11, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_12
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(12),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(12, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(12, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_13
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(13),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(13, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(13, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_14
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(14),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(14, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(14, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_15
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(15),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(15, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(15, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_16
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(16),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(16, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(16, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_17
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(17),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(17, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(17, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_18
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(18),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(18, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(18, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_19
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(19),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(19, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(19, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_20
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(20),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(20, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(20, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_21
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(21),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(21, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(21, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_22
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(22),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(22, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(22, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_23
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(23),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(23, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(23, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_24
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(24),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(24, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(24, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_25
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(25),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(25, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(25, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_26
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(26),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(26, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(26, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_27
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(27),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(27, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(27, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_28
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(28),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(28, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(28, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_29
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(29),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(29, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(29, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_30
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(30),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(30, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(30, 1), 0u, IPFMA_WIDTH_32 },

            // region_0_ch_31
            { CSL_CBASS_FWCH_REGION0_CHANNEL_CONTROL(31),       0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(31, 0), 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_FWCH_REGION0_CHANNEL_PERMISSION(31, 0), 0u, IPFMA_WIDTH_32 }
        };

        if (IPFMA_OK == status)
        {
            for (uint8_t i = 0; i < 32; i++)
            {
                out->region_0_channel[i].control = registerDescriptorsArr[i*3].value;
                for (uint8_t j = 0; j < 2; j++)
                {
                    out->region_0_channel[i].permission[j] = registerDescriptorsArr[(i*3)+(j+1)].value;
                }
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Cba_CompareFwchRegs(const IpFma_CbaFwchRegs* expected,
                                       const IpFma_CbaFwchRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
         for (uint8_t i = 0; i < 32; i++)
        {
            if ((IPFMA_OK == status) && (expected->region_0_channel[i].control != actual->region_0_channel[i].control))
            {
                status = IPFMA_E_MISMATCH;
            }
            for (uint8_t j = 0; j < 2; j++)
            {
                if((IPFMA_OK == status) && (expected->region_0_channel[i].permission[j] != actual->region_0_channel[i].permission[j]))
                {
                    status = IPFMA_E_MISMATCH;
                }
            }
        }

    }

    return status;
}

IpFma_Status IpFma_Cba_GetIscRegs(uintptr_t baseAddr, IpFma_CbaIscRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            // region_0
            { CSL_CBASS_ISC_EP_REGION_CONTROL(0, 0),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_START_ADDR_L(0, 0),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_END_ADDR_H(0, 0),     0u, IPFMA_WIDTH_32 },

            // region_1
            { CSL_CBASS_ISC_EP_REGION_CONTROL(0, 1),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_START_ADDR_L(0, 1),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_END_ADDR_H(0, 1),     0u, IPFMA_WIDTH_32 },

            // region_2
            { CSL_CBASS_ISC_EP_REGION_CONTROL(0, 2),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_START_ADDR_L(0, 2),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_END_ADDR_H(0, 2),     0u, IPFMA_WIDTH_32 },

            // region_3
            { CSL_CBASS_ISC_EP_REGION_CONTROL(0, 3),        0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_START_ADDR_L(0, 3),   0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ISC_EP_REGION_END_ADDR_H(0, 3),     0u, IPFMA_WIDTH_32 },

            // def_control
            { CSL_CBASS_ISC_EP_REGION_DEF_CONTROL(0),       0u, IPFMA_WIDTH_32 }

        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, 
                                    (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                out->region[i].control          = registerDescriptorsArr[(i*3)+0].value;
                out->region[i].start_address_l  = registerDescriptorsArr[(i*3)+1].value;                
                out->region[i].end_address_h    = registerDescriptorsArr[(i*3)+2].value;
            }
            out->region_def_control         = registerDescriptorsArr[12].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Cba_CompareIscRegs(const IpFma_CbaIscRegs* expected,
                                      const IpFma_CbaIscRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            if ((IPFMA_OK == status) && (expected->region[i].control != actual->region[i].control))
            {
                status = IPFMA_E_MISMATCH;
            }
            if ((IPFMA_OK == status) && (expected->region[i].start_address_l != actual->region[i].start_address_l))
            {
                status = IPFMA_E_MISMATCH;
            }
            if ((IPFMA_OK == status) && (expected->region[i].end_address_h != actual->region[i].end_address_h))
            {
                status = IPFMA_E_MISMATCH;
            }
        }

        if ((IPFMA_OK == status) && (expected->region_def_control != actual->region_def_control))
        {
            status = IPFMA_E_MISMATCH;
        }
        
    }

    return status;
}

IpFma_Status IpFma_Cba_GetMapRegs(uintptr_t baseAddr, IpFma_CbaMapRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            { CSL_CBASS_QOS_EP_GRP_MAP1(0, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_QOS_EP_GRP_MAP2(0, 0),  0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_QOS_EP_MAP(0, 0),       0u, IPFMA_WIDTH_32 }
        };


        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, 
                                    (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            out->grp_map1   = registerDescriptorsArr[0].value;
            out->grp_map2   = registerDescriptorsArr[1].value;
            out->map0       = registerDescriptorsArr[2].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Cba_CompareMapRegs(const IpFma_CbaMapRegs* expected,
                                      const IpFma_CbaMapRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if ((IPFMA_OK == status) && (expected->grp_map1 != actual->grp_map2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->grp_map2 != actual->grp_map2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->map0 != actual->map0))
        {
            status = IPFMA_E_MISMATCH;
        }
    }
    return status;
}

IpFma_Status IpFma_Cba_GetErrRegs(uintptr_t baseAddr, IpFma_CbaErrRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            { CSL_CBASS_ERR_DESTINATION_ID, 0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ERR_EOI,            0u, IPFMA_WIDTH_32 },
            { CSL_CBASS_ERR_PID,            0u, IPFMA_WIDTH_32}

        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, 
                                    (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            out->destination_id = registerDescriptorsArr[0].value;
            out->eoi            = registerDescriptorsArr[1].value;
            out->pid            = registerDescriptorsArr[2].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Cba_CompareErrRegs(const IpFma_CbaErrRegs* expected,
                                      const IpFma_CbaErrRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if ((IPFMA_OK == status) && (expected->pid != actual->pid))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->destination_id != actual->destination_id))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->eoi != actual->eoi))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}
/** @} */