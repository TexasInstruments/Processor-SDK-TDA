/*******************************************************************************
**+--------------------------------------------------------------------------+**
**|                            ****                                          |**
**|                            ****                                          |**
**|                            ******o***                                    |**
**|                      ********_///_****                                   |**
**|                      ***** /_//_/ ****                                   |**
**|                       ** ** (__/ ****                                    |**
**|                           *********                                      |**
**|                            ****                                          |**
**|                            ***                                           |**
**|                                                                          |**
**|         Copyright (c) 2016 - 2026 Texas Instruments Incorporated                |**
**|                        ALL RIGHTS RESERVED                               |**
**|                                                                          |**
**| Permission to use, copy, modify, or distribute this software,            |**
**| whether in part or in whole, for any purpose is forbidden without        |**
**| a signed licensing agreement and NDA from Texas Instruments              |**
**| Incorporated (TI).                                                       |**
**|                                                                          |**
**| TI makes no representation or warranties with respect to the             |**
**| performance of this computer program, and specifically disclaims         |**
**| any responsibility for any damages, special or consequential,            |**
**| connected with the use of this program.                                  |**
**|                                                                          |**
**+--------------------------------------------------------------------------+**
*******************************************************************************/

#include "VXLIB_add_i8u_i8u_o8u_cn.h"

#define UINT8_MAX   0xff

VXLIB_STATUS C7xVXLIB_add_i8u_i8u_o8u_cn(uint8_t src0[],
                           VXLIB_bufParams2D_t *src0_addr,
                           uint8_t src1[],
                           VXLIB_bufParams2D_t *src1_addr,
                           uint8_t dst[],
                           VXLIB_bufParams2D_t *dst_addr,
                           uint16_t overflow_policy)
{
    uint32_t        x, y, src0Index, src1Index, dstIndex;
    int32_t         final_result_value, overflowing_result;
    VXLIB_STATUS    status = VXLIB_SUCCESS;

#ifdef VXLIB_CHECK_PARAMS
    status = VXLIB_add_i8u_i8u_o8u_checkParams_cn(src0, src0_addr, src1, src1_addr, dst, dst_addr);
    if( status == VXLIB_SUCCESS )
#endif
    {

        for( y=0; y < src0_addr->dim_y; y++ ) {

            for( x=0; x < src0_addr->dim_x; x++ ) {

                src0Index = y * src0_addr->stride_y + x;
                src1Index = y * src1_addr->stride_y + x;
                dstIndex = y * dst_addr->stride_y + x;
                overflowing_result = src0[src0Index] + src1[src1Index];

                /* Finally, overflow-check as per the target type and policy. */
                if( overflow_policy == VXLIB_CONVERT_POLICY_SATURATE ) {
                    if( overflowing_result > UINT8_MAX ) {
                        final_result_value = UINT8_MAX;
                    } else if( overflowing_result < 0 ) {
                        final_result_value = 0;
                    } else {
                        final_result_value = (uint8_t)overflowing_result;
                    }
                } else {
                    /* Just for show: the final assignment will wrap too. */
                    final_result_value = (uint8_t)overflowing_result;
                }

                dst[dstIndex] = final_result_value;

            }
        }
    }
    return (status);
}

VXLIB_STATUS C7xVXLIB_add_i8u_i8u_o8u_checkParams_cn(uint8_t src0[],
                                       VXLIB_bufParams2D_t *src0_addr,
                                       uint8_t src1[],
                                       VXLIB_bufParams2D_t *src1_addr,
                                       uint8_t dst[],
                                       VXLIB_bufParams2D_t *dst_addr)
{
    VXLIB_STATUS    status = VXLIB_SUCCESS;

    if((src0 == NULL) || (src1 == NULL) || (dst == NULL)) {
        status = VXLIB_ERR_NULL_POINTER;
    } else if( src0_addr->dim_x != src1_addr->dim_x ||
               src0_addr->dim_x != dst_addr->dim_x ||
               src0_addr->dim_y != src1_addr->dim_y ||
               src0_addr->dim_y != dst_addr->dim_y ||
               src0_addr->stride_y < src0_addr->dim_x ||
               src1_addr->stride_y < src0_addr->dim_x ||
               dst_addr->stride_y < dst_addr->dim_x ) {
        status = VXLIB_ERR_INVALID_DIMENSION;
    } else if((src0_addr->data_type != VXLIB_UINT8) ||
               (src1_addr->data_type != VXLIB_UINT8) ||
               (dst_addr->data_type != VXLIB_UINT8)) {
        status = VXLIB_ERR_INVALID_TYPE;
    }
    return (status);
}

