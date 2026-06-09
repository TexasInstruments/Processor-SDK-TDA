/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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

/*!
 * \file ethfw_test_id.h
 *
 * \brief This file contains the jira id details for test cases
 *        required for Ethernet Firmware test automation.
 */

#ifndef ETHFW_TEST_ID_H_
#define ETHFW_TEST_ID_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */
#if defined(SAFERTOS)
/* Virtual Switch port Jira test case id */
#define ETHFW_UT_SWITCH_ATTACH_TEST1_ID                          (2645U)
#define ETHFW_UT_SWITCH_ATTACH_TEST2_ID                          (2666U)
#define ETHFW_UT_SWITCH_DETACH_TEST1_ID                          (2665U)
#define ETHFW_UT_SWITCH_DETACH_TEST2_ID                          (2664U)
#define ETHFW_UT_SWITCH_ALLOC_TX_TEST1_ID                        (2662U)
#define ETHFW_UT_SWITCH_ALLOC_TX_NEGTEST_ID                      (2661U)
#define ETHFW_UT_SWITCH_FREE_TX_TEST1_ID                         (2660U)
#define ETHFW_UT_SWITCH_FREE_TX_NEGTEST_ID                       (2659U)
#define ETHFW_UT_SWITCH_ALLOC_RX_TEST1_ID                        (2658U)
#define ETHFW_UT_SWITCH_FREE_RX_TEST1_ID                         (2657U)
#define ETHFW_UT_SWITCH_ALLOC_RX_NEGTEST_ID                      (2656U)
#define ETHFW_UT_SWITCH_FREE_RX_NEGTEST_ID                       (2655U)
#define ETHFW_UT_SWITCH_ALLOC_MAC_TEST1_ID                       (2654U)
#define ETHFW_UT_SWITCH_FREE_MAC_TEST1_ID                        (2653U)
#define ETHFW_UT_SWITCH_REGISTER_MAC_TEST_ID                     (2652U)
#define ETHFW_UT_SWITCH_REGISTER_MAC_NEGTEST_ID                  (2651U)
#define ETHFW_UT_SWITCH_UNREGISTER_MAC_TEST_ID                   (2650U)
#define ETHFW_UT_SWITCH_UNREGISTER_MAC_NEGTEST_ID                (2649U)
#define ETHFW_UT_SWITCH_REGISTER_IPV4_TEST_ID                    (2648U)
#define ETHFW_UT_SWITCH_DETACH_ONLY_TEST_ID                      (2881U)
#define ETHFW_UT_HW_PUSH_TEST_ID                                 (3037U)

/* Virtual MAC port Jira test case id */
#define ETHFW_UT_MAC_ATTACH_TEST1_ID                             (2690U)
#define ETHFW_UT_MAC_ATTACH_TEST2_ID                             (2691U)
#define ETHFW_UT_MAC_DETACH_TEST1_ID                             (2692U)
#define ETHFW_UT_MAC_DETACH_TEST2_ID                             (2693U)
#define ETHFW_UT_MAC_ALLOC_TX_TEST1_ID                           (2695U)
#define ETHFW_UT_MAC_ALLOC_TX_NEGTEST_ID                         (2696U)
#define ETHFW_UT_MAC_FREE_TX_TEST1_ID                            (2697U)
#define ETHFW_UT_MAC_FREE_TX_NEGTEST_ID                          (2698U)
#define ETHFW_UT_MAC_ALLOC_RX_TEST1_ID                           (2699U)
#define ETHFW_UT_MAC_FREE_RX_TEST1_ID                            (2700U)
#define ETHFW_UT_MAC_ALLOC_RX_NEGTEST_ID                         (2701U)
#define ETHFW_UT_MAC_FREE_RX_NEGTEST_ID                          (2702U)
#define ETHFW_UT_MAC_ALLOC_MAC_TEST1_ID                          (2703U)
#define ETHFW_UT_MAC_FREE_MAC_TEST1_ID                           (2704U)
#define ETHFW_UT_MAC_REGISTER_MAC_TEST_ID                        (2705U)
#define ETHFW_UT_MAC_REGISTER_MAC_NEGTEST_ID                     (2706U)
#define ETHFW_UT_MAC_UNREGISTER_MAC_TEST_ID                      (2707U)
#define ETHFW_UT_MAC_UNREGISTER_MAC_NEGTEST_ID                   (2708U)
#define ETHFW_UT_MAC_DETACH_ONLY_TEST_ID                         (2883U)

/* Server test case id*/
#define ETHFW_UT_SERVER_INIT_TEST_ID                             (2737U)
#define ETHFW_UT_SERVER_INIT2_TEST_ID                            (2738U)
#define ETHFW_UT_SERVER_MONITOR_TEST_ID                          (2739U)
#define ETHFW_UT_SERVER_GPTP_TEST_ID                             (2740U)
#define ETHFW_UT_SERVER_VERSION_TEST_ID                          (2741U)
#define ETHFW_UT_SERVER_SRC_PORT_MIRROR_TEST_ID                  (2922U)
#define ETHFW_UT_SERVER_DST_PORT_MIRROR_TEST_ID                  (2921U)
#define ETHFW_UT_SERVER_TBL_ENTRY_PORT_MIRROR_TEST_ID            (2920U)
#define ETHFW_UT_SERVER_DISABLE_PORT_MIRROR_TEST_ID              (2919U)
#define ETHFW_UT_SERVER_INVALID_PARAM_PORT_MIRROR_TEST_ID        (2918U)
#else
/* Virtual Switch port Jira test case id */
#define ETHFW_UT_SWITCH_ATTACH_TEST1_ID                          (2625U)
#define ETHFW_UT_SWITCH_ATTACH_TEST2_ID                          (2626U)
#define ETHFW_UT_SWITCH_DETACH_TEST1_ID                          (2627U)
#define ETHFW_UT_SWITCH_DETACH_TEST2_ID                          (2628U)
#define ETHFW_UT_SWITCH_ALLOC_TX_TEST1_ID                        (2630U)
#define ETHFW_UT_SWITCH_ALLOC_TX_NEGTEST_ID                      (2631U)
#define ETHFW_UT_SWITCH_FREE_TX_TEST1_ID                         (2632U)
#define ETHFW_UT_SWITCH_FREE_TX_NEGTEST_ID                       (2633U)
#define ETHFW_UT_SWITCH_ALLOC_RX_TEST1_ID                        (2634U)
#define ETHFW_UT_SWITCH_FREE_RX_TEST1_ID                         (2635U)
#define ETHFW_UT_SWITCH_ALLOC_RX_NEGTEST_ID                      (2636U)
#define ETHFW_UT_SWITCH_FREE_RX_NEGTEST_ID                       (2637U)
#define ETHFW_UT_SWITCH_ALLOC_MAC_TEST1_ID                       (2638U)
#define ETHFW_UT_SWITCH_FREE_MAC_TEST1_ID                        (2639U)
#define ETHFW_UT_SWITCH_REGISTER_MAC_TEST_ID                     (2640U)
#define ETHFW_UT_SWITCH_REGISTER_MAC_NEGTEST_ID                  (2641U)
#define ETHFW_UT_SWITCH_UNREGISTER_MAC_TEST_ID                   (2642U)
#define ETHFW_UT_SWITCH_UNREGISTER_MAC_NEGTEST_ID                (2643U)
#define ETHFW_UT_SWITCH_REGISTER_IPV4_TEST_ID                    (2644U)
#define ETHFW_UT_SWITCH_DETACH_ONLY_TEST_ID                      (2882U)
#define ETHFW_UT_HW_PUSH_TEST_ID                                 (3037U)

/* Virtual MAC port Jira test case id */
#define ETHFW_UT_MAC_ATTACH_TEST1_ID                             (2671U)
#define ETHFW_UT_MAC_ATTACH_TEST2_ID                             (2672U)
#define ETHFW_UT_MAC_DETACH_TEST1_ID                             (2673U)
#define ETHFW_UT_MAC_DETACH_TEST2_ID                             (2674U)
#define ETHFW_UT_MAC_ALLOC_TX_TEST1_ID                           (2676U)
#define ETHFW_UT_MAC_ALLOC_TX_NEGTEST_ID                         (2677U)
#define ETHFW_UT_MAC_FREE_TX_TEST1_ID                            (2678U)
#define ETHFW_UT_MAC_FREE_TX_NEGTEST_ID                          (2679U)
#define ETHFW_UT_MAC_ALLOC_RX_TEST1_ID                           (2680U)
#define ETHFW_UT_MAC_FREE_RX_TEST1_ID                            (2681U)
#define ETHFW_UT_MAC_ALLOC_RX_NEGTEST_ID                         (2682U)
#define ETHFW_UT_MAC_FREE_RX_NEGTEST_ID                          (2683U)
#define ETHFW_UT_MAC_ALLOC_MAC_TEST1_ID                          (2684U)
#define ETHFW_UT_MAC_FREE_MAC_TEST1_ID                           (2685U)
#define ETHFW_UT_MAC_REGISTER_MAC_TEST_ID                        (2686U)
#define ETHFW_UT_MAC_REGISTER_MAC_NEGTEST_ID                     (2687U)
#define ETHFW_UT_MAC_UNREGISTER_MAC_TEST_ID                      (2688U)
#define ETHFW_UT_MAC_UNREGISTER_MAC_NEGTEST_ID                   (2689U)
#define ETHFW_UT_MAC_DETACH_ONLY_TEST_ID                         (2884U)

/* Server test case id*/
#define ETHFW_UT_SERVER_INIT_TEST_ID                             (2725U)
#define ETHFW_UT_SERVER_INIT2_TEST_ID                            (2726U)
#define ETHFW_UT_SERVER_MONITOR_TEST_ID                          (2727U)
#define ETHFW_UT_SERVER_GPTP_TEST_ID                             (2729U)
#define ETHFW_UT_SERVER_VERSION_TEST_ID                          (2730U)
#define ETHFW_UT_SERVER_SRC_PORT_MIRROR_TEST_ID                  (2913U)
#define ETHFW_UT_SERVER_DST_PORT_MIRROR_TEST_ID                  (2914U)
#define ETHFW_UT_SERVER_TBL_ENTRY_PORT_MIRROR_TEST_ID            (2915U)
#define ETHFW_UT_SERVER_DISABLE_PORT_MIRROR_TEST_ID              (2916U)
#define ETHFW_UT_SERVER_INVALID_PARAM_PORT_MIRROR_TEST_ID        (2917U)
#endif

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_TEST_ID_H_ */
