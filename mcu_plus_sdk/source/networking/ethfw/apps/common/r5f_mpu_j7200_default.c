/*
 *  Copyright (c) Texas Instruments Incorporated 2021
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

#include <stdlib.h>
#include <stdint.h>
#include <ti/csl/arch/r5/csl_arm_r5.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>
#include <ti/csl/arch/r5/interrupt.h>
#include <ti/csl/arch/r5/csl_cache.h>

/*
 * This structure specifies the entries for mpu configuration to override the
 * default MPU configuration which is part of the CSL init.
 */
const CSL_ArmR5MpuRegionCfg  gCslR5MpuCfg[CSL_ARM_R5F_MPU_REGIONS_MAX] =
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
        .cacheable        = UFALSE,
        .cachePolicy      = 0U,
        .memAttr          = 0U,
    },
    {
        /* Region 1 configuration: ATCM memory */
        .regionId         = 1U,
        .enable           = 1U,
        .baseAddr         = 0x0U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_32KB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
    {
        /* Region 2 configuration: BTCM memory as cacheable */
        .regionId         = 2U,
        .enable           = 1U,
        .baseAddr         = 0x41010000U,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_32KB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UTRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 0U,
    },
    {
        /* Region 3 configuration: 2 GB DDR RAM */
        .regionId         = 3U,
        .enable           = 1U,
        .baseAddr         = 0x80000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_2GB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 0U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UTRUE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_WB_WA,
        .memAttr          = 0U,
    },
    {
        /* Region 4 configuration: DDR_SHARED_MEM as non-cache */
        .regionId         = 4U,
        .enable           = 1U,
        .baseAddr         = 0xB8000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_32MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
    {
        /* Region 5 configuration: IPC_VRING_MEM_ADDR as non-cache */
        .regionId         = 5U,
        .enable           = 1U,
        .baseAddr         = 0xA4000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_16MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
    {
        /* Region 6 configuration: DDR_MCU2_1_IPC_ADDR as non-cache */
        .regionId         = 6U,
        .enable           = 1U,
        .baseAddr         = 0xA3000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_1MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
    {
        /* Region 7 configuration: DDR_MCU2_0_IPC_ADDR as non-cache */
        .regionId         = 7U,
        .enable           = 1U,
        .baseAddr         = 0xA2000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_1MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
    {
        /* Region 8 configuration: INTERCORE_ETH_DESC_MEM as non-cache 2MB */
        .regionId         = 8U,
        .enable           = 1U,
        .baseAddr         = 0xA5000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_2MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 1U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
    {
        /* Region 9 configuration: DDR_MCU1_0_IPC_ADDR as non-cache */
        .regionId         = 9U,
        .enable           = 1U,
        .baseAddr         = 0xA0000000,
        .size             = CSL_ARM_R5_MPU_REGION_SIZE_1MB,
        .subRegionEnable  = CSL_ARM_R5_MPU_SUB_REGION_ENABLE_ALL,
        .exeNeverControl  = 1U,
        .accessPermission = CSL_ARM_R5_ACC_PERM_PRIV_USR_RD_WR,
        .shareable        = 0U,
        .cacheable        = UFALSE,
        .cachePolicy      = CSL_ARM_R5_CACHE_POLICY_NON_CACHEABLE,
        .memAttr          = 4U,
    },
};
