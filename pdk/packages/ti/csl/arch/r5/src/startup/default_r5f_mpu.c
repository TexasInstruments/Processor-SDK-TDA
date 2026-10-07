/**
 *  \file   default_r5f_mpu.c
 *
 *  \brief  This file contains the default MPU region configurations.
 *
 */

/*
 * Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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
 *
 */

#include <startup.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                      Static Function Declarations                          */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


const CSL_ArmR5MpuRegionCfg __attribute__((section(".startupData"))) gCslR5MpuCfg[CSL_ARM_R5F_MPU_REGIONS_MAX] =
{

    {
        /* Region 0 configuration: complete 32 bit address space = 4Gbits */
        .regionId         = 0U,
        .enable           = 1U,
        .baseAddr         = 0x0U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_4GB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)FALSE,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Region 1 configuration: 128 bytes memory for exception vector execution */
        .regionId         = 1U,
        .enable           = 1U,
        .baseAddr         = 0x0U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_128B,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_WB_WA,
        .memAttr          = 0U,
    },
    {
        /* Region 2 configuration: 1MB KB MCU MSRAM */
        .regionId         = 2U,
        .enable           = 1U,
        .baseAddr         = 0x41C00000U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_1MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_WB_WA,
        .memAttr          = 0U,
    },
    {
        /* Region 3 configuration: MCMS3 RAM */
        .regionId         = 3U,
        .enable           = 1U,
        .baseAddr         = 0x70000000U,
#if defined (SOC_J721E) || defined (SOC_J784S4)
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_8MB,
#endif
#if defined (SOC_J7200)
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_1MB,
#endif
#if defined (SOC_J721S2) || defined (SOC_J742S2)
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_4MB,
#endif
#if defined (SOC_AM62X) || defined (SOC_AM62A) || defined (SOC_AM275X) || defined (SOC_AM283X)
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_64KB,
#endif
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_WB_WA,
        .memAttr          = 0U,
    },
    {
        /* Region 4 configuration: 2 GB DDR RAM */
        .regionId         = 4U,
        .enable           = 1U,
        .baseAddr         = 0x80000000U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_2GB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_WB_WA,
        .memAttr          = 0U,
    },
    {
        /* Region 5 configuration: 32 KB BTCM */
        /* Address of ATCM/BTCM are configured via MCU_SEC_MMR registers
           It can either be '0x0' or '0x41010000'. Application/Boot-loader shall
           take care this configurations and linker command file shall be
           in sync with this. For either of the above configurations,
           MPU configurations will not changes as both regions will have same
           set of permissions in almost all scenarios.
           Application can chose to overwrite this MPU configuration if needed.
           The same is true for the region corresponding to ATCM. */
        .regionId         = 5U,
        .enable           = 1U,
        .baseAddr         = 0x41010000U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_32KB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 0U,
    },
    {
        /* Region 6 configuration: 32 KB ATCM */
        .regionId         = 6U,
        .enable           = 1U,
        .baseAddr         = 0x0U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_32KB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 0U,
    },
#if defined (SOC_AM62X) || defined (SOC_AM62A) || defined (SOC_AM275X) || defined (SOC_AM283X)
     {
        /* Region 7 configuration: 256KB of HSM RAM, Program is currently loaded here*/
        .regionId         = 7U,
        .enable           = 1U,
        .baseAddr         = 0x43C00000U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_256KB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = (uint32_t)TRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_WB_WA,
        .memAttr          = 0U,
    },
#else
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Placeholder config. All zero entries */
        .regionId         = 0U,
        .enable           = 0U,
        .baseAddr         = 0x0U,
        .size             = 0U,
        .subRegionEnable  = 0U,
        .exeNeverControl  = 0U,
        .accessPermission = 0U,
        .shareable        = 0U,
        .cacheable        = 0U,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
#endif
};
