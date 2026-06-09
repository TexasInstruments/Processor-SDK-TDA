/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <utils/ipc/include/app_ipc.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/iss/include/app_iss.h>
#include <utils/console_io/include/app_log.h>

/* DCC VISS INCLUDES */
#ifdef VPAC3L
#include <dcc_viss_ov2312.h>
#endif
#include <dcc_viss_imx390.h>
#include <dcc_viss_imx390_wdr.h>
#include <dcc_viss_imx390_discovery_mid.h>
#include <dcc_viss_imx390_discovery_mid_wdr.h>
#include <dcc_viss_imx390_discovery_narrow.h>
#include <dcc_viss_imx390_discovery_narrow_wdr.h>
#include <dcc_viss_imx390_discovery_wide.h>
#include <dcc_viss_imx390_discovery_wide_wdr.h>
#include <dcc_viss_imx728_wdr.h>
#include <dcc_viss_ar0233.h>
#include <dcc_viss_ar0233_wdr.h>
#include <dcc_viss_ar0820.h>
#include <dcc_viss_ar0820_wdr.h>
#include <dcc_viss_ox03f10_wdr.h>
#include <dcc_viss_ub9xx_raw_test_pattern.h>

/* DCC LDC INCLUDES */
#include <dcc_ldc_imx390.h>
#include <dcc_ldc_imx390_wdr.h>
#include <dcc_ldc_imx390_discovery_mid.h>
#include <dcc_ldc_imx390_discovery_mid_wdr.h>
#include <dcc_ldc_imx390_discovery_narrow.h>
#include <dcc_ldc_imx390_discovery_narrow_wdr.h>
#include <dcc_ldc_imx390_discovery_wide.h>
#include <dcc_ldc_imx390_discovery_wide_wdr.h>
#include <dcc_ldc_imx728_wdr.h>
#include <dcc_ldc_ar0233.h>
#include <dcc_ldc_ar0233_wdr.h>
#include <dcc_ldc_ar0820.h>
#include <dcc_ldc_ar0820_wdr.h>
#include <dcc_ldc_ox03f10_wdr.h>
#include <dcc_ldc_ub9xx_raw_test_pattern.h>
#include <dcc_ldc_ub96x_uyvytestpat.h>
#include <dcc_ldc_gw_ar0233.h>
#ifdef VPAC3L
#include <dcc_ldc_ov2312.h>
#endif

#define ENABLE_2A

/* DCC 2A INCLUDES */
#ifdef ENABLE_2A
#include <dcc_2a_imx390.h>
#include <dcc_2a_imx390_wdr.h>
#include <dcc_2a_imx390_discovery_mid.h>
#include <dcc_2a_imx390_discovery_mid_wdr.h>
#include <dcc_2a_imx390_discovery_narrow.h>
#include <dcc_2a_imx390_discovery_narrow_wdr.h>
#include <dcc_2a_imx390_discovery_wide.h>
#include <dcc_2a_imx390_discovery_wide_wdr.h>
#include <dcc_2a_imx728_wdr.h>
#include <dcc_2a_ar0233.h>
#include <dcc_2a_ar0233_wdr.h>
#include <dcc_2a_ar0820.h>
#include <dcc_2a_ar0820_wdr.h>
#include <dcc_2a_ox03f10_wdr.h>
#ifdef VPAC3L
#include <dcc_2a_ov2312.h>
#endif
#include <dcc_2a_ub9xx_raw_test_pattern.h>
#endif /* ENABLE_2A */


#ifdef VPAC3L
static uint8_t  dcc_viss_ov2312[DCC_VISS_OV2312_DCC_CFG_NUM_ELEM] = DCC_VISS_OV2312DCC_CFG;
#endif
static uint8_t  dcc_viss_imx390[DCC_VISS_IMX390_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390DCC_CFG;
static uint8_t  dcc_viss_imx390_wdr[DCC_VISS_IMX390_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDRDCC_CFG;
static uint8_t  dcc_viss_imx390_disco_mid[DCC_VISS_IMX390_DISCOVERY_MID_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_DISCOVERY_MIDDCC_CFG;
static uint8_t  dcc_viss_imx390_disco_mid_wdr[DCC_VISS_IMX390_DISCOVERY_MID_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_DISCOVERY_MID_WDRDCC_CFG;
static uint8_t  dcc_viss_imx390_disco_narrow[DCC_VISS_IMX390_DISCOVERY_NARROW_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_DISCOVERY_NARROWDCC_CFG;
static uint8_t  dcc_viss_imx390_disco_narrow_wdr[DCC_VISS_IMX390_DISCOVERY_NARROW_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_DISCOVERY_NARROW_WDRDCC_CFG;
static uint8_t  dcc_viss_imx390_disco_wide[DCC_VISS_IMX390_DISCOVERY_WIDE_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_DISCOVERY_WIDEDCC_CFG;
static uint8_t  dcc_viss_imx390_disco_wide_wdr[DCC_VISS_IMX390_DISCOVERY_WIDE_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_DISCOVERY_WIDE_WDRDCC_CFG;
static uint8_t  dcc_viss_imx728_wdr[DCC_VISS_IMX728_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX728_WDRDCC_CFG;
static uint8_t  dcc_viss_ar0233_linear[DCC_VISS_AR0233_DCC_CFG_NUM_ELEM] = DCC_VISS_AR0233DCC_CFG;
static uint8_t  dcc_viss_ar0233_wdr[DCC_VISS_AR0233_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_AR0233_WDRDCC_CFG;
static uint8_t  dcc_viss_ar0820_linear[DCC_VISS_AR0820_DCC_CFG_NUM_ELEM] = DCC_VISS_AR0820DCC_CFG;
static uint8_t  dcc_viss_ar0820_wdr[DCC_VISS_AR0820_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_AR0820_WDRDCC_CFG;
static uint8_t  dcc_viss_ox03f10_wdr[DCC_VISS_OX03F10_WDR_DCC_CFG_NUM_ELEM] = DCC_VISS_OX03F10_WDRDCC_CFG;
static uint8_t  dcc_viss_ub9xx_raw_test_pattern_linear[DCC_VISS_UB9XX_RAW_TEST_PATTERN_DCC_CFG_NUM_ELEM] = DCC_VISS_UB9XX_RAW_TEST_PATTERNDCC_CFG;


static uint8_t  dcc_ldc_imx390[DCC_LDC_IMX390_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390DCC_CFG;
static uint8_t  dcc_ldc_imx390_wdr[DCC_LDC_IMX390_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_WDRDCC_CFG;
static uint8_t  dcc_ldc_imx390_disco_mid[DCC_LDC_IMX390_DISCOVERY_MID_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_DISCOVERY_MIDDCC_CFG;
static uint8_t  dcc_ldc_imx390_disco_mid_wdr[DCC_LDC_IMX390_DISCOVERY_MID_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_DISCOVERY_MID_WDRDCC_CFG;
static uint8_t  dcc_ldc_imx390_disco_narrow[DCC_LDC_IMX390_DISCOVERY_NARROW_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_DISCOVERY_NARROWDCC_CFG;
static uint8_t  dcc_ldc_imx390_disco_narrow_wdr[DCC_LDC_IMX390_DISCOVERY_NARROW_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_DISCOVERY_NARROW_WDRDCC_CFG;
static uint8_t  dcc_ldc_imx390_disco_wide[DCC_LDC_IMX390_DISCOVERY_WIDE_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_DISCOVERY_WIDEDCC_CFG;
static uint8_t  dcc_ldc_imx390_disco_wide_wdr[DCC_LDC_IMX390_DISCOVERY_WIDE_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX390_DISCOVERY_WIDE_WDRDCC_CFG;
static uint8_t  dcc_ldc_imx728_wdr[DCC_LDC_IMX728_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_IMX728_WDRDCC_CFG;
static uint8_t  dcc_ldc_ar0233_linear[DCC_LDC_AR0233_DCC_CFG_NUM_ELEM] = DCC_LDC_AR0233DCC_CFG;
static uint8_t  dcc_ldc_ar0233_wdr[DCC_LDC_AR0233_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_AR0233_WDRDCC_CFG;
static uint8_t  dcc_ldc_ar0820_linear[DCC_LDC_AR0820_DCC_CFG_NUM_ELEM] = DCC_LDC_AR0820DCC_CFG;
static uint8_t  dcc_ldc_ar0820_wdr[DCC_LDC_AR0820_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_AR0820_WDRDCC_CFG;
static uint8_t  dcc_ldc_ox03f10_wdr[DCC_LDC_OX03F10_WDR_DCC_CFG_NUM_ELEM] = DCC_LDC_OX03F10_WDRDCC_CFG;
static uint8_t  dcc_ldc_ub9xx_raw_test_pattern_linear[DCC_LDC_UB9XX_RAW_TEST_PATTERN_DCC_CFG_NUM_ELEM] = DCC_LDC_UB9XX_RAW_TEST_PATTERNDCC_CFG;
static uint8_t  dcc_ldc_ub96x_uyvytestpat[DCC_LDC_UB96X_UYVYTESTPAT_DCC_CFG_NUM_ELEM] = DCC_LDC_UB96X_UYVYTESTPATDCC_CFG;
static uint8_t  dcc_ldc_gw_ar0233[DCC_LDC_GW_AR0233_DCC_CFG_NUM_ELEM] = DCC_LDC_GW_AR0233DCC_CFG;
#ifdef VPAC3L
static uint8_t  dcc_ldc_ov2312[DCC_LDC_OV2312_DCC_CFG_NUM_ELEM] = DCC_LDC_OV2312DCC_CFG;
#endif


#ifdef ENABLE_2A
static uint8_t  dcc_2a_imx390[DCC_2A_IMX390_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390DCC_CFG;
static uint8_t  dcc_2a_imx390_wdr[DCC_2A_IMX390_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_WDRDCC_CFG;
static uint8_t  dcc_2a_imx390_disco_mid[DCC_2A_IMX390_DISCOVERY_MID_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_DISCOVERY_MIDDCC_CFG;
static uint8_t  dcc_2a_imx390_disco_mid_wdr[DCC_2A_IMX390_DISCOVERY_MID_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_DISCOVERY_MID_WDRDCC_CFG;
static uint8_t  dcc_2a_imx390_disco_narrow[DCC_2A_IMX390_DISCOVERY_NARROW_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_DISCOVERY_NARROWDCC_CFG;
static uint8_t  dcc_2a_imx390_disco_narrow_wdr[DCC_2A_IMX390_DISCOVERY_NARROW_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_DISCOVERY_NARROW_WDRDCC_CFG;
static uint8_t  dcc_2a_imx390_disco_wide[DCC_2A_IMX390_DISCOVERY_WIDE_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_DISCOVERY_WIDEDCC_CFG;
static uint8_t  dcc_2a_imx390_disco_wide_wdr[DCC_2A_IMX390_DISCOVERY_WIDE_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_IMX390_DISCOVERY_WIDE_WDRDCC_CFG;
static uint8_t  dcc_2a_imx728_wdr[DCC_2A_IMX728_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_IMX728_WDRDCC_CFG;
static uint8_t  dcc_2a_ar0233_linear[DCC_2A_AR0233_DCC_CFG_NUM_ELEM] = DCC_2A_AR0233DCC_CFG;
static uint8_t  dcc_2a_ar0233_wdr[DCC_2A_AR0233_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_AR0233_WDRDCC_CFG;
static uint8_t  dcc_2a_ar0820_linear[DCC_2A_AR0820_DCC_CFG_NUM_ELEM] = DCC_2A_AR0820DCC_CFG;
static uint8_t  dcc_2a_ar0820_wdr[DCC_2A_AR0820_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_AR0820_WDRDCC_CFG;
static uint8_t  dcc_2a_ox03f10_wdr[DCC_2A_OX03F10_WDR_DCC_CFG_NUM_ELEM] = DCC_2A_OX03F10_WDRDCC_CFG;
#ifdef VPAC3L
static uint8_t  dcc_2a_ov2312[DCC_2A_OV2312_DCC_CFG_NUM_ELEM] = DCC_2A_OV2312DCC_CFG;
#endif
static uint8_t  dcc_2a_ub9xx_raw_test_pattern_linear[DCC_2A_UB9XX_RAW_TEST_PATTERN_DCC_CFG_NUM_ELEM] = DCC_2A_UB9XX_RAW_TEST_PATTERNDCC_CFG;
#endif /* ENABLE_2A */

/* Function prototypes */
static uint8_t is_viss_plugin(uint32_t plugin_id);
static uint8_t is_aewb_plugin(uint32_t plugin_id);
static uint8_t is_ldc_plugin(uint32_t plugin_id);
static int32_t appSplitVpacDcc(uint8_t *dcc_buf_in, uint32_t prmSize,
                        uint8_t **dcc_buf_viss, uint32_t *viss_bytes,
                        uint8_t **dcc_buf_aewb, uint32_t *aewb_bytes,
                        uint8_t **dcc_buf_ldc,  uint32_t *ldc_bytes);

int32_t appIssGetDCCSizeVISS(const char * sensor_name, uint32_t wdr_mode)
{
    int32_t size = -1;
    switch(wdr_mode)
    {
        case 0:
            /*Linear mode*/
            #ifdef VPAC3L
            if(0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                size = (int32_t)DCC_VISS_OV2312_DCC_CFG_NUM_ELEM;
            }
            else
            #endif
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                size = (int32_t)DCC_VISS_IMX390_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                size = (int32_t)DCC_VISS_IMX390_DISCOVERY_MID_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)) {
                    appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                    size = (int32_t)DCC_VISS_IMX390_DISCOVERY_NARROW_DCC_CFG_NUM_ELEM;
            } else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                size = (int32_t)DCC_VISS_IMX390_DISCOVERY_WIDE_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                size = (int32_t)DCC_VISS_AR0233_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                size = (int32_t)DCC_VISS_AR0820_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, UB9XX_RAW_TESTPAT))
            {
                size = (int32_t)DCC_VISS_UB9XX_RAW_TEST_PATTERN_DCC_CFG_NUM_ELEM;
            }
            else
            {
                size = -1;
            }
            break;
        case 1:
            /*WDR mode*/
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                size = (int32_t)DCC_VISS_IMX390_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                size = (int32_t)DCC_VISS_IMX390_DISCOVERY_MID_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                size = (int32_t)DCC_VISS_IMX390_DISCOVERY_NARROW_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                size = (int32_t)DCC_VISS_IMX390_DISCOVERY_WIDE_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX728_UB971_SONY))
            {
                size = (int32_t)DCC_VISS_IMX728_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                size = (int32_t)DCC_VISS_AR0233_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                size = (int32_t)DCC_VISS_AR0820_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_OX03F10_UB953_OV))
            {
                size = (int32_t)DCC_VISS_OX03F10_WDR_DCC_CFG_NUM_ELEM;
            }
#ifdef VPAC3L
            else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                size = (int32_t)DCC_VISS_OV2312_DCC_CFG_NUM_ELEM;
            }
#endif
            else
            {
                size = -1;
            }
            break;
        default:
            /*Unsupported mode*/
            size = -1;
            break;
    }
    return size;
}

int32_t appIssGetDCCBuffVISS(const char * sensor_name, uint32_t wdr_mode,  uint8_t * dcc_buf, int32_t num_bytes)
{
    int32_t status = 0;

    switch(wdr_mode)
    {
        case 0:
            /*Linear mode*/
            #ifdef VPAC3L
            if(0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_viss_ov2312, (size_t)num_bytes);
            }
            else
            #endif
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx390, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx390_disco_mid, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                (void)memcpy(dcc_buf, dcc_viss_imx390_disco_narrow, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx390_disco_wide, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                (void)memcpy(dcc_buf, dcc_viss_ar0233_linear, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_viss_ar0820_linear, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, UB9XX_RAW_TESTPAT))
            {
                (void)memcpy(dcc_buf, dcc_viss_ub9xx_raw_test_pattern_linear, (size_t)num_bytes);
            }
            else
            {
                status = -1;
            }
            break;
        case 1:
            /*WDR mode*/
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx390_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx390_disco_mid_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                (void)memcpy(dcc_buf, dcc_viss_imx390_disco_narrow_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx390_disco_wide_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX728_UB971_SONY))
            {
                (void)memcpy(dcc_buf, dcc_viss_imx728_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                (void)memcpy(dcc_buf, dcc_viss_ar0233_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_viss_ar0820_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_OX03F10_UB953_OV))
            {
                (void)memcpy(dcc_buf, dcc_viss_ox03f10_wdr, (size_t)num_bytes);
            }
#ifdef VPAC3L
            else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_viss_ov2312, (size_t)num_bytes);
            }
#endif
            else
            {
                status = -1;
            }
            break;
        default:
            /*Unsupported mode*/
            status = -1;
            break;
    }

    return status;
}

int32_t appIssGetDCCSize2A(const char * sensor_name, uint32_t wdr_mode)
{
    int32_t size = -1;
    switch(wdr_mode)
    {
        case 0:
            /*Linear mode*/
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                size = (int32_t)DCC_2A_IMX390_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                size = (int32_t)DCC_2A_IMX390_DISCOVERY_MID_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                size = (int32_t)DCC_2A_IMX390_DISCOVERY_NARROW_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                size = (int32_t)DCC_2A_IMX390_DISCOVERY_WIDE_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                size = (int32_t)DCC_2A_AR0233_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                size = (int32_t)DCC_2A_AR0820_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, UB9XX_RAW_TESTPAT))
            {
                size = (int32_t)DCC_2A_UB9XX_RAW_TEST_PATTERN_DCC_CFG_NUM_ELEM;
            }
#ifdef VPAC3L
            else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                size = (int32_t)DCC_2A_OV2312_DCC_CFG_NUM_ELEM;
            }
#endif
            else
            {
                size = -1;
            }
            break;
        case 1:
            /*WDR mode*/
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                size = (int32_t)DCC_2A_IMX390_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                size = (int32_t)DCC_2A_IMX390_DISCOVERY_MID_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)) {
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                size = (int32_t)DCC_2A_IMX390_DISCOVERY_NARROW_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                size = (int32_t)DCC_2A_IMX390_DISCOVERY_WIDE_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX728_UB971_SONY))
            {
                size = (int32_t)DCC_2A_IMX728_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                size = (int32_t)DCC_2A_AR0233_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                size = (int32_t)DCC_2A_AR0820_WDR_DCC_CFG_NUM_ELEM;
            }
            else if (0 == strcmp(sensor_name, SENSOR_OX03F10_UB953_OV))
            {
                size = (int32_t)DCC_2A_OX03F10_WDR_DCC_CFG_NUM_ELEM;
            }
#ifdef VPAC3L
            else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                size = (int32_t)DCC_2A_OV2312_DCC_CFG_NUM_ELEM;
            }
#endif
            else
            {
                size = -1;
            }
            break;
        default:
            /*Unsupported mode*/
            size = -1;
            break;
    }
    return size;
}

int32_t appIssGetDCCBuff2A(const char * sensor_name, uint32_t wdr_mode,  uint8_t * dcc_buf, int32_t num_bytes)
{
    int32_t status = 0;

    switch(wdr_mode)
    {
        case 0:
            /*Linear mode*/
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx390, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx390_disco_mid, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                (void)memcpy(dcc_buf, dcc_2a_imx390_disco_narrow, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx390_disco_wide, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                (void)memcpy(dcc_buf, dcc_2a_ar0233_linear, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_2a_ar0820_linear, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, UB9XX_RAW_TESTPAT))
            {
                (void)memcpy(dcc_buf, dcc_2a_ub9xx_raw_test_pattern_linear, (size_t)num_bytes);
            }
#ifdef VPAC3L
            else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_2a_ov2312, (size_t)num_bytes);
            }
#endif
            else
            {
                status = -1;
            }
            break;
        case 1:
            /*WDR mode*/
            if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx390_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx390_disco_mid_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)) {
                appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                (void)memcpy(dcc_buf, dcc_2a_imx390_disco_narrow_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx390_disco_wide_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX728_UB971_SONY))
            {
                (void)memcpy(dcc_buf, dcc_2a_imx728_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
            {
                (void)memcpy(dcc_buf, dcc_2a_ar0233_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_2a_ar0820_wdr, (size_t)num_bytes);
            }
            else if (0 == strcmp(sensor_name, SENSOR_OX03F10_UB953_OV))
            {
                (void)memcpy(dcc_buf, dcc_2a_ox03f10_wdr, (size_t)num_bytes);
            }
#ifdef VPAC3L
            else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
            {
                (void)memcpy(dcc_buf, dcc_2a_ov2312, (size_t)num_bytes);
            }
#endif
            else
            {
                status = -1;
            }
            break;
        default:
            /*Unsupported mode*/
            status = -1;
            break;
    }

    return status;
}

int32_t appIssGetDCCSizeLDC(const char * sensor_name, uint32_t wdr_mode)
{
    int32_t size = -1;

    if (0 == strcmp(sensor_name, UB96X_TESTPATTERN_UYVY))
    {
        size = (int32_t)DCC_LDC_UB96X_UYVYTESTPAT_DCC_CFG_NUM_ELEM;
    }
    else if (0 == strcmp(sensor_name, GW_AR0233_UYVY))
    {
        size = (int32_t)DCC_LDC_GW_AR0233_DCC_CFG_NUM_ELEM;
    }
    else
    {
        switch(wdr_mode)
        {
            case 0:
                /*Linear mode*/
                if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
                {
                    size = (int32_t)DCC_LDC_IMX390_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
                {
                    size = (int32_t)DCC_LDC_IMX390_DISCOVERY_MID_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                    appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                    size = (int32_t)DCC_LDC_IMX390_DISCOVERY_NARROW_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
                {
                    size = (int32_t)DCC_LDC_IMX390_DISCOVERY_WIDE_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
                {
                    size = (int32_t)DCC_LDC_AR0233_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
                {
                    size = (int32_t)DCC_LDC_AR0820_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, UB9XX_RAW_TESTPAT))
                {
                    size = (int32_t)DCC_LDC_UB9XX_RAW_TEST_PATTERN_DCC_CFG_NUM_ELEM;
                }
#ifdef VPAC3L
                else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
                {
                    size = (int32_t)DCC_LDC_OV2312_DCC_CFG_NUM_ELEM;
                }
#endif
                else
                {
                    size = -1;
                }
                break;
            case 1:
                /*WDR mode*/
                if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
                {
                    size = (int32_t)DCC_LDC_IMX390_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
                {
                    size = (int32_t)DCC_LDC_IMX390_DISCOVERY_MID_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                    appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                    size = (int32_t)DCC_LDC_IMX390_DISCOVERY_NARROW_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
                {
                    size = (int32_t)DCC_LDC_IMX390_DISCOVERY_WIDE_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX728_UB971_SONY))
                {
                    size = (int32_t)DCC_LDC_IMX728_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
                {
                    size = (int32_t)DCC_LDC_AR0233_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
                {
                    size = (int32_t)DCC_LDC_AR0820_WDR_DCC_CFG_NUM_ELEM;
                }
                else if (0 == strcmp(sensor_name, SENSOR_OX03F10_UB953_OV))
                {
                    size = (int32_t)DCC_LDC_OX03F10_WDR_DCC_CFG_NUM_ELEM;
                }
#ifdef VPAC3L
                else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
                {
                    size = (int32_t)DCC_LDC_OV2312_DCC_CFG_NUM_ELEM;
                }
#endif
                else
                {
                    size = -1;
                }
                break;
            default:
                /*Unsupported mode*/
                size = -1;
                break;
        }

    }

    return size;
}

int32_t appIssGetDCCBuffLDC(const char * sensor_name, uint32_t wdr_mode,  uint8_t * dcc_buf, int32_t num_bytes)
{
    int32_t status = 0;

    if (0 == strcmp(sensor_name, UB96X_TESTPATTERN_UYVY))
    {
        (void)memcpy(dcc_buf, dcc_ldc_ub96x_uyvytestpat, (size_t)num_bytes);
    }
    else if (0 == strcmp(sensor_name, GW_AR0233_UYVY))
    {
        (void)memcpy(dcc_buf, dcc_ldc_gw_ar0233, (size_t)num_bytes);
    }
    else
    {
        switch(wdr_mode)
        {
            case 0:
                /*Linear mode*/
                if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx390, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_disco_mid, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                    appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_disco_narrow, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_disco_wide, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ar0233_linear, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ar0820_linear, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, UB9XX_RAW_TESTPAT))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ub9xx_raw_test_pattern_linear, (size_t)num_bytes);
                }
#ifdef VPAC3L
                else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ov2312, (size_t)num_bytes);
                }
#endif
                else
                {
                    status = -1;
                }
                break;
            case 1:
                /*WDR mode*/
                if(0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_D3))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_MID))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_disco_mid_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_NARROW)){
                    appLogPrintf("********************imx390 Discovery %s [%d]********************\n", __func__, __LINE__);
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_disco_narrow_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX390_UB953_DISCOVERY_WIDE))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx390_disco_wide_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_SONY_IMX728_UB971_SONY))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_imx728_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0233_UB953_MARS))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ar0233_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_ONSEMI_AR0820_UB953_LI))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ar0820_wdr, (size_t)num_bytes);
                }
                else if (0 == strcmp(sensor_name, SENSOR_OX03F10_UB953_OV))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ox03f10_wdr, (size_t)num_bytes);
                }
#ifdef VPAC3L
                else if (0 == strcmp(sensor_name, SENSOR_OV2312_UB953_LI))
                {
                    (void)memcpy(dcc_buf, dcc_ldc_ov2312, (size_t)num_bytes);
                }
#endif
                else
                {
                    status = -1;
                }
                break;
            default:
                /*Unsupported mode*/
                status = -1;
                break;
        }
    }

    return status;
}

int32_t appIssGetResizeParams(uint16_t in_width, uint16_t in_height, uint16_t tgt_width, uint16_t tgt_height, uint16_t * out_width, uint16_t * out_height)
{
    uint32_t tmp = 0;
    uint32_t rsz_ratio = 1024U;
    uint32_t rsz_ratio_x = (((uint32_t)tgt_width * 1024U) / (uint32_t)in_width);
    uint32_t rsz_ratio_y = (((uint32_t)tgt_height * 1024U) / (uint32_t)in_height);

    /*Assuming downscale. Will pick the ratio which has a bigger downscale factor*/
    /*X and Y dimensions will be scaled by same factor to maintain aspect ratio*/
    rsz_ratio = (rsz_ratio_x < rsz_ratio_y) ? rsz_ratio_x : rsz_ratio_y;
    tmp = (in_width*rsz_ratio)/1024U;
    *out_width = (uint16_t)((tmp & 0xFFF8U) & 0xFFFFU); /*Align to 8x*/
    tmp = (in_height*rsz_ratio)/1024U;
    *out_height = (uint16_t)((tmp & 0xFFF8U) & 0xFFFFU); /*Align to 8x*/

    return 0;
}

/*
Checks if the plugin is relevant to VISS. Returns
    0, if False
    1, if True
*/
static uint8_t is_viss_plugin(uint32_t plugin_id)
{
    uint8_t result = 0U;

    switch (plugin_id)
    {
        case (uint32_t)DCC_ID_H3A_AEWB_CFG:
        case (uint32_t)DCC_ID_H3A_MUX_LUTS:
        case (uint32_t)DCC_ID_RFE_DECOMPAND:
        case (uint32_t)DCC_ID_IPIPE_RGB_RGB_1:
        case (uint32_t)DCC_ID_NSF4:
        case (uint32_t)DCC_ID_BLACK_CLAMP:
        case (uint32_t)DCC_ID_IPIPE_CFA:
        case (uint32_t)DCC_ID_VISS_GLBCE:
        case (uint32_t)DCC_ID_VISS_LSC:
        case (uint32_t)DCC_ID_VISS_DPC:
        case (uint32_t)DCC_ID_VISS_YEE:
        case (uint32_t)DCC_ID_RAWFE_WB1_VS:
        case (uint32_t)DCC_ID_VISS_GAMMA:
#ifdef VPAC3
        case (uint32_t)DCC_ID_VISS_CFAI3_A:
        case (uint32_t)DCC_ID_VISS_CFAI3_B:
        case (uint32_t)DCC_ID_VISS_CAC:
        case (uint32_t)DCC_ID_VISS_RAWHIST:
        case (uint32_t)DCC_ID_VISS_CCMV:
#endif
#ifdef VPAC3L
        case (uint32_t)DCC_ID_VISS_CFAI3_A:
        case (uint32_t)DCC_ID_VISS_CAC:
        case (uint32_t)DCC_ID_VISS_RAWHIST:
        case (uint32_t)DCC_ID_VISS_DPC_EXT:
        case (uint32_t)DCC_ID_VISS_LSC_EXT:
        case (uint32_t)DCC_ID_VISS_PCID:
#endif
            result = 1U;
            break;
        default:
            result = 0U;
            break;
    }
    return result;
}

/*
Checks if the plugin is relevant to AEWB. Returns
    0, if False
    1, if True
*/
static uint8_t is_aewb_plugin(uint32_t plugin_id)
{
    uint8_t result = 0U;

    switch (plugin_id)
    {
        case (uint32_t)DCC_ID_H3A_AEWB_CFG:
        case (uint32_t)DCC_ID_AAA_ALG_AWB_TI3:
            result = 1U;
            break;
        default:
            result = 0U;
            break;
    }
    return result;
}

/*
Checks if the plugin is relevant to LDC. Returns
    0, if False
    1, if True
*/
static uint8_t is_ldc_plugin(uint32_t plugin_id)
{
    uint8_t result = 0U;

    switch (plugin_id)
    {
        case (uint32_t)DCC_ID_MESH_LDC_J7:
            result = 1U;
            break;
        default:
            result = 0U;
            break;
    }
    return result;
}

#define MAX_VISS_COMPONENT_SIZE (1024U * 1024U)
#define MAX_AEWB_COMPONENT_SIZE (1024U * 1024U)
#define MAX_LDC_COMPONENT_SIZE (2048U * 1024U)
#define MAX_DCC_FULL_SIZE (4096U * 1024U)
static uint8_t g_dcc_viss_mem[MAX_VISS_COMPONENT_SIZE];
static uint8_t g_dcc_aewb_mem[MAX_AEWB_COMPONENT_SIZE];
static uint8_t g_dcc_ldc_mem[MAX_LDC_COMPONENT_SIZE];
static uint8_t g_dcc_full_buf[MAX_DCC_FULL_SIZE];
static int32_t appSplitVpacDcc(uint8_t *dcc_buf_in, uint32_t prmSize,
                              uint8_t **dcc_buf_viss, uint32_t *viss_bytes,
                              uint8_t **dcc_buf_aewb, uint32_t *aewb_bytes,
                              uint8_t **dcc_buf_ldc,  uint32_t *ldc_bytes)
{
    uint32_t offset = 0U;
    uint32_t off_viss = 0U;
    uint32_t off_aewb = 0U;
    uint32_t off_ldc = 0U;
    uint8_t *pluginBuf = dcc_buf_in;
    int32_t status = 0;

    /* Validate input pointers */
    if ((NULL == dcc_buf_in) || (NULL == dcc_buf_viss) || (NULL == viss_bytes) ||
        (NULL == dcc_buf_aewb) || (NULL == aewb_bytes) || (NULL == dcc_buf_ldc) ||
        (NULL == ldc_bytes))
    {
        status = -1;
    }
    else
    {
        /* Direct assignment to static memory */
        *dcc_buf_viss = g_dcc_viss_mem;
        *dcc_buf_aewb = g_dcc_aewb_mem;
        *dcc_buf_ldc  = g_dcc_ldc_mem;

        while ((offset < prmSize) && (0 == status))
        {
            dcc_component_header_type header;
            uint32_t sz;

            /* Validate minimum header size available */
            if ((offset + (uint32_t)sizeof(dcc_component_header_type)) > prmSize)
            {
                status = -1;
            }
            else
            {
                (void)memcpy((void *)&header, (const void *)pluginBuf, sizeof(dcc_component_header_type));
                sz = header.total_file_sz;

                /* Validate component size */
                if ((0U == sz) || ((offset + sz) > prmSize))
                {
                    status = -1;
                }
                else
                {
                    if (is_viss_plugin((uint32_t)header.dcc_descriptor_id) != 0U)
                    {
                        if ((off_viss + sz) <= MAX_VISS_COMPONENT_SIZE)
                        {
                            (void)memcpy(&(*dcc_buf_viss)[off_viss], pluginBuf, sz);
                            off_viss += sz;
                        }
                        else
                        {
                            /* Buffer overflow would occur */
                            status = -1;
                        }
                    }
                    else if (is_aewb_plugin((uint32_t)header.dcc_descriptor_id) != 0U)
                    {
                        if ((off_aewb + sz) <= MAX_AEWB_COMPONENT_SIZE)
                        {
                            (void)memcpy(&(*dcc_buf_aewb)[off_aewb], pluginBuf, sz);
                            off_aewb += sz;
                        }
                        else
                        {
                            /* Buffer overflow would occur */
                            status = -1;
                        }
                    }
                    else if (is_ldc_plugin((uint32_t)header.dcc_descriptor_id) != 0U)
                    {
                        if ((off_ldc + sz) <= MAX_LDC_COMPONENT_SIZE)
                        {
                            (void)memcpy(&(*dcc_buf_ldc)[off_ldc], pluginBuf, sz);
                            off_ldc += sz;
                        }
                        else
                        {
                            /* Buffer overflow would occur */
                            status = -1;
                        }
                    }
                    else
                    {
                        /* Unknown plugin type, skip */
                    }

                    if (0 == status)
                    {
                        uint64_t next_offset = (uint64_t)offset + (uint64_t)sz;

                        pluginBuf = &pluginBuf[sz];

                        if(next_offset > 0xFFFFFFFFU)
                        {
                            offset = 0xFFFFFFFFU;
                        }
                        else
                        {
                            offset = (uint32_t)next_offset;
                        }
                    }
                }
            }
        }

        if (0 == status)
        {
            *viss_bytes = off_viss;
            *aewb_bytes = off_aewb;
            *ldc_bytes  = off_ldc;
        }
    }

    return status;
}

