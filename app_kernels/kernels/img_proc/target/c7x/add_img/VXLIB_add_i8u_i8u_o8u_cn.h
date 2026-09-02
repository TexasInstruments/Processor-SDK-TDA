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

#ifndef VXLIB_ADD_I8U_I8U_O8U_CN_H_
#define VXLIB_ADD_I8U_I8U_O8U_CN_H_ 1

#include "VXLIB_types.h"

VXLIB_STATUS    C7xVXLIB_add_i8u_i8u_o8u_cn(uint8_t src0[],
                              VXLIB_bufParams2D_t * src0_addr,
                              uint8_t src1[],
                              VXLIB_bufParams2D_t * src1_addr,
                              uint8_t dst[],
                              VXLIB_bufParams2D_t * dst_addr,
                              uint16_t overflow_policy);

VXLIB_STATUS    C7xVXLIB_add_i8u_i8u_o8u_checkParams_cn(uint8_t src0[],
                                          VXLIB_bufParams2D_t * src0_addr,
                                          uint8_t src1[],
                                          VXLIB_bufParams2D_t * src1_addr,
                                          uint8_t dst[],
                                          VXLIB_bufParams2D_t * dst_addr);

#endif

