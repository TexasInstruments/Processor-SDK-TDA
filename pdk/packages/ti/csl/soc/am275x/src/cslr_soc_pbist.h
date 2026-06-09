/********************************************************************
*
* SOC PBIST PROPERTIES. header file
*
* Copyright (C) 2015-2025 Texas Instruments Incorporated.
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
*
*/
#ifndef CSLR_SOC_PBIST_H_
#define CSLR_SOC_PBIST_H_

#include <ti/csl/cslr.h>
#include <ti/csl/tistdtypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
* Auto-generated CSL definitions for SoC PBIST Instances:
*/


/* Properties of PBIST instances in: C7X256V0 */
#define CSL_C7X256V0_NUM_TEST_VECTORS                                                              (2U)
#define CSL_C7X256V0_ALGO_BITMAP_0                                                                 (0x0000000000000005U)
#define CSL_C7X256V0_MEM_BITMAP_0                                                                  (0x00000000000E2366U)
#define CSL_C7X256V0_ALGO_BITMAP_1                                                                 (0x000000000000000AU)
#define CSL_C7X256V0_MEM_BITMAP_1                                                                  (0x0000000002C00000U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CA0                                                (0x00000000U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CA1                                                (0x0000001fU)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CA2                                                (0x0000001fU)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CA3                                                (0x00000000U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CL0                                                (0x00000007U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CL1                                                (0x00000003U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CL2                                                (0x00000004U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CL3                                                (0x0000001fU)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CMS                                                (0x00000000U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_CSR                                                (0x00000e00U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_I0                                                 (0x00000001U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_I1                                                 (0x00000004U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_I2                                                 (0x00000004U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_I3                                                 (0x00000000U)
#define CSL_C7X256V0_FAIL_INSERTION_TEST_VECTOR_RAMT                                               (0x4e0a2028U)

/* Properties of PBIST instances in: C7X256V1 */
#define CSL_C7X256V1_NUM_TEST_VECTORS                                                              (2U)
#define CSL_C7X256V1_ALGO_BITMAP_0                                                                 (0x0000000000000005U)
#define CSL_C7X256V1_MEM_BITMAP_0                                                                  (0x00000000000E2366U)
#define CSL_C7X256V1_ALGO_BITMAP_1                                                                 (0x000000000000000AU)
#define CSL_C7X256V1_MEM_BITMAP_1                                                                  (0x0000000002C00000U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CA0                                                (0x00000000U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CA1                                                (0x0000001fU)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CA2                                                (0x0000001fU)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CA3                                                (0x00000000U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CL0                                                (0x00000007U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CL1                                                (0x00000003U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CL2                                                (0x00000004U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CL3                                                (0x0000001fU)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CMS                                                (0x00000000U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_CSR                                                (0x00000e00U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_I0                                                 (0x00000001U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_I1                                                 (0x00000004U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_I2                                                 (0x00000004U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_I3                                                 (0x00000000U)
#define CSL_C7X256V1_FAIL_INSERTION_TEST_VECTOR_RAMT                                               (0x4e0a2028U)

/* Properties of PBIST instances in: PBIST0 */
#define CSL_PBIST0_NUM_TEST_VECTORS                                                                (2U)
#define CSL_PBIST0_ALGO_BITMAP_0                                                                   (0x0000000000000005U)
#define CSL_PBIST0_MEM_BITMAP_0                                                                    (0x000000009A38AA42U)
#define CSL_PBIST0_ALGO_BITMAP_1                                                                   (0x000000000000000AU)
#define CSL_PBIST0_MEM_BITMAP_1                                                                    (0x2545106000000000U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CA0                                                  (0x00000000U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CA1                                                  (0x00000027U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CA2                                                  (0x00000027U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CA3                                                  (0x00000007U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CL0                                                  (0x00000013U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CL1                                                  (0x00000001U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CL2                                                  (0x00000005U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CL3                                                  (0x0000001fU)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CMS                                                  (0x00000001U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_CSR                                                  (0x00380000U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_I0                                                   (0x00000001U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_I1                                                   (0x00000002U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_I2                                                   (0x00000004U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_I3                                                   (0x00000002U)
#define CSL_PBIST0_FAIL_INSERTION_TEST_VECTOR_RAMT                                                 (0x20954028U)

/* Properties of PBIST instances in: PBIST1 */
#define CSL_PBIST1_NUM_TEST_VECTORS                                                                (1U)
#define CSL_PBIST1_ALGO_BITMAP_0                                                                   (0x0000000000000003U)
#define CSL_PBIST1_MEM_BITMAP_0                                                                    (0x0000000000094824U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA0                                                  (0x00000000U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA1                                                  (0x000003ffU)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA2                                                  (0x000003ffU)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA3                                                  (0x00000000U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL0                                                  (0x000000ffU)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL1                                                  (0x00000003U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL2                                                  (0x00000009U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL3                                                  (0x000003ffU)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CMS                                                  (0x00000000U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_CSR                                                  (0x04000000U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_I0                                                   (0x00000001U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_I1                                                   (0x00000004U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_I2                                                   (0x00000009U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_I3                                                   (0x00000000U)
#define CSL_PBIST1_FAIL_INSERTION_TEST_VECTOR_RAMT                                                 (0x001a2728U)

/* Properties of PBIST instances in: PBIST2 */
#define CSL_PBIST2_NUM_TEST_VECTORS                                                                (1U)
#define CSL_PBIST2_ALGO_BITMAP_0                                                                   (0x0000000000000003U)
#define CSL_PBIST2_MEM_BITMAP_0                                                                    (0x0000000000094824U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CA0                                                  (0x00000000U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CA1                                                  (0x000003ffU)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CA2                                                  (0x000003ffU)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CA3                                                  (0x00000000U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CL0                                                  (0x000000ffU)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CL1                                                  (0x00000003U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CL2                                                  (0x00000009U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CL3                                                  (0x000003ffU)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CMS                                                  (0x00000000U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_CSR                                                  (0x04000000U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_I0                                                   (0x00000001U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_I1                                                   (0x00000004U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_I2                                                   (0x00000009U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_I3                                                   (0x00000000U)
#define CSL_PBIST2_FAIL_INSERTION_TEST_VECTOR_RAMT                                                 (0x001a2728U)

/* Properties of PBIST instances in: PBIST3 */
#define CSL_PBIST3_NUM_TEST_VECTORS                                                                (0U)

/* Properties of PBIST instances in: PBIST4 */
#define CSL_PBIST4_NUM_TEST_VECTORS                                                                (0U)

/* Properties of PBIST instances in: PBIST5 */
#define CSL_PBIST5_NUM_TEST_VECTORS                                                                (0U)

/* Properties of PBIST instances in: PBIST6 */
#define CSL_PBIST6_NUM_TEST_VECTORS                                                                (0U)

/* Properties of PBIST instances in: PBIST7 */
#define CSL_PBIST7_NUM_TEST_VECTORS                                                                (0U)

/* Properties of PBIST instances in: PBIST8 */
#define CSL_PBIST8_NUM_TEST_VECTORS                                                                (0U)

/* Properties of PBIST instances in: WKUP_PBIST1 */
#define CSL_WKUP_PBIST1_NUM_TEST_VECTORS                                                           (1U)
#define CSL_WKUP_PBIST1_ALGO_BITMAP_0                                                              (0x0000000000000003U)
#define CSL_WKUP_PBIST1_MEM_BITMAP_0                                                               (0x0000000000000002U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA0                                             (0x00000000U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA1                                             (0x000007ffU)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA2                                             (0x000007ffU)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CA3                                             (0x00000000U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL0                                             (0x000000ffU)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL1                                             (0x00000007U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL2                                             (0x0000000aU)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CL3                                             (0x000007ffU)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CMS                                             (0x00000000U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_CSR                                             (0x00000001U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_I0                                              (0x00000001U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_I1                                              (0x00000008U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_I2                                              (0x0000000aU)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_I3                                              (0x00000000U)
#define CSL_WKUP_PBIST1_FAIL_INSERTION_TEST_VECTOR_RAMT                                            (0x00002728U)

/* Properties of PBIST instances in: WKUP_PBIST0 */
#define CSL_WKUP_PBIST0_NUM_TEST_VECTORS                                                           (0U)

#ifdef __cplusplus
}
#endif
#endif /* CSLR_SOC_PBIST_H_ */

