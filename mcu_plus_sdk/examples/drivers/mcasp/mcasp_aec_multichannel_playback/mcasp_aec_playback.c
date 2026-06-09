/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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

#include <kernel/dpl/DebugP.h>
#include <drivers/i2c.h>
#include <drivers/mcasp.h>
#include <board/ioexp/ioexp_tca6424.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "audio_sample.h"

/* ========================================================================== */
/*                        Extern Function Declaration                         */
/* ========================================================================== */

extern int32_t Board_codecConfig(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

MCASP_Transaction txTxn_AEC1;
MCASP_Transaction txTxn_AEC2;

SemaphoreP_Object gSemAudioChime;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void mcasp_playback(void *args)
{
    int32_t status = SystemP_SUCCESS;
    MCASP_Handle mcaspHandle_AEC1;
    MCASP_Handle mcaspHandle_AEC2;
    char            valueChar;

    SemaphoreP_constructBinary(&gSemAudioChime, 0);

    /*Configure AEC1, AEC2*/
    status = Board_codecConfig();
    DebugP_assert(status == SystemP_SUCCESS);

    /*Initialize transaction details for AEC1*/
    txTxn_AEC1.buf = (void *)audio_sample;
    txTxn_AEC1.count = audio_sample_size;
    txTxn_AEC1.timeout = 0xFFFFFF;

    /*Initialize transaction details for AEC2*/
    txTxn_AEC2.buf = (void *)audio_sample;
    txTxn_AEC2.count = audio_sample_size;
    txTxn_AEC2.timeout = 0xFFFFFF;

    DebugP_log("AEC Audio playback example started. \r\n");

    /*Submit data for AEC1*/
    mcaspHandle_AEC1 = MCASP_getHandle(CONFIG_MCASP0);
    MCASP_submitTx(mcaspHandle_AEC1, &txTxn_AEC1);

    /*Submit data for AEC2*/
    mcaspHandle_AEC2 = MCASP_getHandle(CONFIG_MCASP1);
    MCASP_submitTx(mcaspHandle_AEC2, &txTxn_AEC2);

    /* Trigger McASP transmit operation for AEC1*/
    status = MCASP_startTransferTx(mcaspHandle_AEC1);
    DebugP_assert(status == SystemP_SUCCESS);

    /* Trigger McASP transmit operation for AEC2*/
    status = MCASP_startTransferTx(mcaspHandle_AEC2);
    DebugP_assert(status == SystemP_SUCCESS);

    DebugP_log("Enter your response on UART terminal");

    do
    {
        DebugP_log("\r\nStop the demo? (y/n) : ");
        status = DebugP_scanf("%c", &valueChar);
        DebugP_assert(status == SystemP_SUCCESS);
    } while (valueChar != 'y');

    MCASP_stopTransferTx(mcaspHandle_AEC1);
    MCASP_stopTransferTx(mcaspHandle_AEC2);
}

void mcasp_txcb(MCASP_Handle handle,
                          MCASP_Transaction *transaction)
{
    MCASP_submitTx(handle, transaction);
}
