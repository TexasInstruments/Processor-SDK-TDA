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

#include <drivers/bootloader.h>
#include <drivers/bootloader/bootloader_sd.h>
#include <kernel/dpl/CacheP.h>

/*
 * FreeRTOS+FAT
 */
#include <fs/freertos_fat/FreeRTOS-FAT/include/ff_headers.h>
#include <fs/freertos_fat/portable/ff_mmcsd.h>
#include <fs/freertos_fat/FreeRTOS-FAT/include/ff_stdio.h>

static int32_t SD_imgOpen(void *args, Bootloader_Params *params);
static void SD_imgClose(void *handle, void *args);
static int32_t SD_imgRead(void *dst, uint32_t len, void *args);
static void SD_imgSeek(uint32_t offset, void *args);

Bootloader_Fxns gBootloaderSdFxns = {
    .imgOpenFxn   = SD_imgOpen,
    .imgReadFxn   = SD_imgRead,
    .imgOffsetFxn = NULL,
    .imgSeekFxn   = SD_imgSeek,
    .imgCloseFxn  = SD_imgClose,
    .imgCustomFxn = NULL,
};

static int32_t SD_imgOpen(void *args, Bootloader_Params *params)
{
    int32_t status = SystemP_FAILURE;
    Bootloader_SdArgs *sdArgs=(Bootloader_SdArgs*)args;
    FF_FILE *appFp = ff_fopen(params->fileName, "rb");
    sdArgs->filePtr = (void*)appFp;

    if(appFp != NULL)
    {
        status = SystemP_SUCCESS;
    }

    return status;
}

static void SD_imgClose(void *handle, void *args)
{
    Bootloader_SdArgs *sdArgs=(Bootloader_SdArgs*)args;
    FF_FILE *appFp = sdArgs->filePtr;

    /* Close the file */
    if(appFp != NULL)
    {
        ff_fclose(appFp);
    }

    return;
}

static int32_t SD_imgRead(void *dst, uint32_t len, void *args)
{
    int32_t status = SystemP_FAILURE;
    Bootloader_SdArgs *sdArgs=(Bootloader_SdArgs*)args;
    FF_FILE *appFp=(FF_FILE *)(sdArgs->filePtr);

    if(appFp != NULL)
    {
        ff_fread(dst, len, 1, appFp);
        CacheP_wb(dst, len, CacheP_TYPE_ALL);
        status = SystemP_SUCCESS;
    }

    return status;
}

static void SD_imgSeek(uint32_t offset, void *args)
{
    Bootloader_SdArgs *sdArgs=(Bootloader_SdArgs*)args;
    FF_FILE *appFp=(FF_FILE *)(sdArgs->filePtr);

    if(appFp != NULL)
    {
        ff_fseek(appFp, offset, FF_SEEK_SET);
    }

    return;
}

