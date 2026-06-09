/**
 *   Copyright (c) Texas Instruments Incorporated 2021
 *   All rights reserved.
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
 *  \file vhwa_m2mNfApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mFcDrvPriv.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
typedef struct
{
    uint32_t node;

    uint32_t nodeStatus;

} Vhwa_ResourceInfo;

typedef struct
{
    uint32_t schID;

    uint32_t schType;

    uint32_t inUse;
} Vhwa_SpareSchInfo;

Vhwa_ResourceInfo gResourceList[VHWA_FC_MAX_NODES] =
{
    {VHWA_FC_NODE_NONE, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_VISS, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_MSC0, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_MSC1, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_LDC, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_NF, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_L3, VHWA_FC_NODE_STATUS_FREE},
    {VHWA_FC_NODE_DDR, VHWA_FC_NODE_STATUS_FREE}
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Defination                             */
/* ========================================================================== */

int32_t Vhwa_m2mFcInitAndAllocRes(const Vhwa_M2mFcGraphObj *graphObj)
{
    uint32_t cnt, nodeId;
    int32_t status = FVID2_SOK;

    for(cnt = 0; cnt < VHWA_FC_MAX_NODES; cnt++)
    {
        nodeId = graphObj->graphNodeObj[cnt].nodeId;
        if((nodeId > VHWA_FC_NODE_NONE) && (nodeId < VHWA_FC_MAX_NODES))
        {
            gResourceList[nodeId].nodeStatus = VHWA_FC_NODE_STATUS_M2M;
        }
    }
    return status;
}

