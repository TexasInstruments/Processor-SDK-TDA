/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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
 *  \file     boot_core_defs.h
 *
 *  \brief    Header file for slave boot core definitions
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/boot/sbl/src/mmcsd/sbl_mmcsd.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of boot stages for the main domain multi-stage boot sequence */
#define NUM_BOOT_STAGES     2

/* Maximum number of boot cores per boot stage */
#define MAX_CORES_PER_STAGE 4

#define MAX_APPIMAGE_NAME_LEN   (16)

/* Macros representing the offset where the App Image has to be written/Read from
   the OSPI Flash.
*/
#define OSPI_OFFSET_SI               (0x2E0000U)
#define OSPI_OFFSET_SYSFW            (0x80000U)

/* Location of ATF/OPTEE - used for both Linux and QNX */
#define OSPI_OFFSET_A72IMG1          (0x1C0000U)
/* Location of Kernel for Linux or IFS for QNX */
#define OSPI_OFFSET_A72IMG2          (0x7C0000U)
/* Location of DTB for Linux */
#define OSPI_OFFSET_A72IMG3          (0x1EC0000U)

#define MAIN_DOMAIN_APPS_FLASH_ADDR  (0x1FC0000U)
#define MAIN_DOMAIN_APPS_FLASH_ADDR2 (0x23C0000U)
#define MAIN_DOMAIN_APPS_FLASH_ADDR3 (0x37C0000U)

/* Location Address used as flag to indicate loading of
 * all HLOS appimages for OSPI */
#define MAIN_DOMAIN_HLOS             (0x1)
/* Name indicating that appimages for HLOS should be
 * loaded from MMCSD filesystem */
#define MAIN_DOMAIN_HLOS_NAME        "hlos"

/* Important RAM address macros */
#define ATF_START_RAM_ADDR          (0x70000000)

#ifdef __cplusplus
}
#endif
