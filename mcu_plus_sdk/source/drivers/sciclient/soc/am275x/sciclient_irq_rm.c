/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 *  \file am275x/sciclient_irq_rm.c
 *
 *  \brief irq_tree for AM275X
 *
 */
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/sciclient/sciclient_rm_priv.h>
#include <drivers/sciclient/soc/am275x/sciclient_irq_rm.h>

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static struct Sciclient_rmIaUsedMapping rom_usage_DMASS0_INTAGGR_0[1U] = {
    {
        .event = 30U,
        .cleared = false,
    },
};
uint8_t vint_usage_count_DMASS0_INTAGGR_0[184U]= {0};

struct Sciclient_rmIaInst gRmIaInstances[SCICLIENT_RM_IA_NUM_INST] =
{
    {
        .dev_id             = TISCI_DEV_DMASS0_INTAGGR_0,
        .imap               = 0x48100000,
        .sevt_offset        = 0u,
        .n_sevt             = 1536u,
        .n_vint             = 184,
        .vint_usage_count   = &vint_usage_count_DMASS0_INTAGGR_0[0],
        .v0_b0_evt          = 0,
        .rom_usage = &rom_usage_DMASS0_INTAGGR_0[0U],
		.n_rom_usage = 1,
    }
};

struct Sciclient_rmIrInst gRmIrInstances[SCICLIENT_RM_IR_NUM_INST] =
{
    {
        .dev_id         = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
        .cfg            = 0xa00000,
        .n_inp          = 232u,
        .n_outp         = 80u,
        .inp0_mapping   = 0,
        .rom_usage      = NULL,
        .n_rom_usage    = 0U,
    },
    {
        .dev_id         = TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
        .cfg            = 0x4210000,
        .n_inp          = 32u,
        .n_outp         = 32u,
        .inp0_mapping   = 0,
        .rom_usage      = NULL,
        .n_rom_usage    = 0U,
    },
    {
        .dev_id         = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
        .cfg            = 0xa40000,
        .n_inp          = 24u,
        .n_outp         = 32u,
        .inp0_mapping   = 0,
        .rom_usage      = NULL,
        .n_rom_usage    = 0U,
    },
};

/* IRQ Tree definition */

/* Start of MAIN_GPIO_INTROUTER0 interface definition */
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_0_15_to_C7X256V0_CLEC_gic_spi_32_47 = {
	.lbase = 0,
	.len = 16,
	.rid = TISCI_DEV_C7X256V0_CLEC,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_0_15_to_C7X256V1_CLEC_gic_spi_32_47 = {
	.lbase = 0,
	.len = 16,
	.rid = TISCI_DEV_C7X256V1_CLEC,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_0_15_to_WKUP_R5FSS0_CORE0_intr_32_47 = {
	.lbase = 0,
	.len = 16,
	.rid = TISCI_DEV_WKUP_R5FSS0_CORE0,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_16_31_to_R5FSS0_CORE0_intr_32_47 = {
	.lbase = 16,
	.len = 16,
	.rid = TISCI_DEV_R5FSS0_CORE0,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_16_31_to_R5FSS0_CORE1_intr_32_47 = {
	.lbase = 16,
	.len = 16,
	.rid = TISCI_DEV_R5FSS0_CORE1,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_32_47_to_R5FSS1_CORE0_intr_32_47 = {
	.lbase = 32,
	.len = 16,
	.rid = TISCI_DEV_R5FSS1_CORE0,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_32_47_to_R5FSS1_CORE1_intr_32_47 = {
	.lbase = 32,
	.len = 16,
	.rid = TISCI_DEV_R5FSS1_CORE1,
	.rbase = 32,
};
const struct Sciclient_rmIrqIf MAIN_GPIO_INTROUTER0_outp_48_57_to_DMASS0_INTAGGR_0_intaggr_levi_pend_16_25 = {
	.lbase = 48,
	.len = 10,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 16,
};
const struct Sciclient_rmIrqIf * const tisci_if_MAIN_GPIO_INTROUTER0[] = {
	&MAIN_GPIO_INTROUTER0_outp_0_15_to_C7X256V0_CLEC_gic_spi_32_47,
	&MAIN_GPIO_INTROUTER0_outp_0_15_to_C7X256V1_CLEC_gic_spi_32_47,
	&MAIN_GPIO_INTROUTER0_outp_0_15_to_WKUP_R5FSS0_CORE0_intr_32_47,
	&MAIN_GPIO_INTROUTER0_outp_16_31_to_R5FSS0_CORE0_intr_32_47,
	&MAIN_GPIO_INTROUTER0_outp_16_31_to_R5FSS0_CORE1_intr_32_47,
	&MAIN_GPIO_INTROUTER0_outp_32_47_to_R5FSS1_CORE0_intr_32_47,
	&MAIN_GPIO_INTROUTER0_outp_32_47_to_R5FSS1_CORE1_intr_32_47,
	&MAIN_GPIO_INTROUTER0_outp_48_57_to_DMASS0_INTAGGR_0_intaggr_levi_pend_16_25,
};
static const struct Sciclient_rmIrqNode tisci_irq_MAIN_GPIO_INTROUTER0 = {
	.id = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.n_if = 8,
	.p_if = &tisci_if_MAIN_GPIO_INTROUTER0[0],
};

/* Start of MCU_MCU_GPIO_INTROUTER0 interface definition */
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_0_3_to_C7X256V0_CLEC_gic_spi_104_107 = {
	.lbase = 0,
	.len = 4,
	.rid = TISCI_DEV_C7X256V0_CLEC,
	.rbase = 104,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_0_3_to_C7X256V1_CLEC_gic_spi_104_107 = {
	.lbase = 0,
	.len = 4,
	.rid = TISCI_DEV_C7X256V1_CLEC,
	.rbase = 104,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_12_15_to_WKUP_R5FSS0_CORE0_intr_50_53 = {
	.lbase = 12,
	.len = 4,
	.rid = TISCI_DEV_WKUP_R5FSS0_CORE0,
	.rbase = 50,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_0_3_to_WKUP_R5FSS0_CORE0_intr_104_107 = {
	.lbase = 0,
	.len = 4,
	.rid = TISCI_DEV_WKUP_R5FSS0_CORE0,
	.rbase = 104,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_8_11_to_WKUP_ESM0_esm_pls_event0_88_91 = {
	.lbase = 8,
	.len = 4,
	.rid = TISCI_DEV_WKUP_ESM0,
	.rbase = 88,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_8_11_to_WKUP_ESM0_esm_pls_event1_92_95 = {
	.lbase = 8,
	.len = 4,
	.rid = TISCI_DEV_WKUP_ESM0,
	.rbase = 92,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_8_11_to_WKUP_ESM0_esm_pls_event2_96_99 = {
	.lbase = 8,
	.len = 4,
	.rid = TISCI_DEV_WKUP_ESM0,
	.rbase = 96,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_16_23_to_R5FSS0_CORE0_intr_100_107 = {
	.lbase = 16,
	.len = 8,
	.rid = TISCI_DEV_R5FSS0_CORE0,
	.rbase = 100,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_16_23_to_R5FSS0_CORE1_intr_100_107 = {
	.lbase = 16,
	.len = 8,
	.rid = TISCI_DEV_R5FSS0_CORE1,
	.rbase = 100,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_24_31_to_R5FSS1_CORE0_intr_100_107 = {
	.lbase = 24,
	.len = 8,
	.rid = TISCI_DEV_R5FSS1_CORE0,
	.rbase = 100,
};
const struct Sciclient_rmIrqIf MCU_MCU_GPIO_INTROUTER0_outp_24_31_to_R5FSS1_CORE1_intr_100_107 = {
	.lbase = 24,
	.len = 8,
	.rid = TISCI_DEV_R5FSS1_CORE1,
	.rbase = 100,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCU_MCU_GPIO_INTROUTER0[] = {
	&MCU_MCU_GPIO_INTROUTER0_outp_0_3_to_C7X256V0_CLEC_gic_spi_104_107,
	&MCU_MCU_GPIO_INTROUTER0_outp_0_3_to_C7X256V1_CLEC_gic_spi_104_107,
	&MCU_MCU_GPIO_INTROUTER0_outp_12_15_to_WKUP_R5FSS0_CORE0_intr_50_53,
	&MCU_MCU_GPIO_INTROUTER0_outp_0_3_to_WKUP_R5FSS0_CORE0_intr_104_107,
	&MCU_MCU_GPIO_INTROUTER0_outp_8_11_to_WKUP_ESM0_esm_pls_event0_88_91,
	&MCU_MCU_GPIO_INTROUTER0_outp_8_11_to_WKUP_ESM0_esm_pls_event1_92_95,
	&MCU_MCU_GPIO_INTROUTER0_outp_8_11_to_WKUP_ESM0_esm_pls_event2_96_99,
	&MCU_MCU_GPIO_INTROUTER0_outp_16_23_to_R5FSS0_CORE0_intr_100_107,
	&MCU_MCU_GPIO_INTROUTER0_outp_16_23_to_R5FSS0_CORE1_intr_100_107,
	&MCU_MCU_GPIO_INTROUTER0_outp_24_31_to_R5FSS1_CORE0_intr_100_107,
	&MCU_MCU_GPIO_INTROUTER0_outp_24_31_to_R5FSS1_CORE1_intr_100_107,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCU_MCU_GPIO_INTROUTER0 = {
	.id = TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
	.n_if = 11,
	.p_if = &tisci_if_MCU_MCU_GPIO_INTROUTER0[0],
};

/* Start of TIMESYNC_INTROUTER0 interface definition */
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_0_7_to_DMASS0_INTAGGR_0_intaggr_levi_pend_8_15 = {
	.lbase = 0,
	.len = 8,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_8_8_to_CPSW0_cpts_hw1_push_0_0 = {
	.lbase = 8,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 0,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_9_9_to_CPSW0_cpts_hw2_push_1_1 = {
	.lbase = 9,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 1,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_10_10_to_CPSW0_cpts_hw3_push_2_2 = {
	.lbase = 10,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 2,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_11_11_to_CPSW0_cpts_hw4_push_3_3 = {
	.lbase = 11,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 3,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_12_12_to_CPSW0_cpts_hw5_push_4_4 = {
	.lbase = 12,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 4,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_13_13_to_CPSW0_cpts_hw6_push_5_5 = {
	.lbase = 13,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 5,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_14_14_to_CPSW0_cpts_hw7_push_6_6 = {
	.lbase = 14,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 6,
};
const struct Sciclient_rmIrqIf TIMESYNC_INTROUTER0_outl_15_15_to_CPSW0_cpts_hw8_push_7_7 = {
	.lbase = 15,
	.len = 1,
	.rid = TISCI_DEV_CPSW0,
	.rbase = 7,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMESYNC_INTROUTER0[] = {
	&TIMESYNC_INTROUTER0_outl_0_7_to_DMASS0_INTAGGR_0_intaggr_levi_pend_8_15,
	&TIMESYNC_INTROUTER0_outl_8_8_to_CPSW0_cpts_hw1_push_0_0,
	&TIMESYNC_INTROUTER0_outl_9_9_to_CPSW0_cpts_hw2_push_1_1,
	&TIMESYNC_INTROUTER0_outl_10_10_to_CPSW0_cpts_hw3_push_2_2,
	&TIMESYNC_INTROUTER0_outl_11_11_to_CPSW0_cpts_hw4_push_3_3,
	&TIMESYNC_INTROUTER0_outl_12_12_to_CPSW0_cpts_hw5_push_4_4,
	&TIMESYNC_INTROUTER0_outl_13_13_to_CPSW0_cpts_hw6_push_5_5,
	&TIMESYNC_INTROUTER0_outl_14_14_to_CPSW0_cpts_hw7_push_6_6,
	&TIMESYNC_INTROUTER0_outl_15_15_to_CPSW0_cpts_hw8_push_7_7,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMESYNC_INTROUTER0 = {
	.id = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.n_if = 9,
	.p_if = &tisci_if_TIMESYNC_INTROUTER0[0],
};

/* Start of CPSW0 interface definition */
const struct Sciclient_rmIrqIf CPSW0_cpts_comp_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_0_0 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 0,
};
const struct Sciclient_rmIrqIf CPSW0_cpts_genf0_1_1_to_TIMESYNC_INTROUTER0_in_16_16 = {
	.lbase = 1,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 16,
};
const struct Sciclient_rmIrqIf CPSW0_cpts_genf1_2_2_to_TIMESYNC_INTROUTER0_in_17_17 = {
	.lbase = 2,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 17,
};
const struct Sciclient_rmIrqIf CPSW0_cpts_sync_3_3_to_TIMESYNC_INTROUTER0_in_18_18 = {
	.lbase = 3,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 18,
};
const struct Sciclient_rmIrqIf * const tisci_if_CPSW0[] = {
	&CPSW0_cpts_comp_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_0_0,
	&CPSW0_cpts_genf0_1_1_to_TIMESYNC_INTROUTER0_in_16_16,
	&CPSW0_cpts_genf1_2_2_to_TIMESYNC_INTROUTER0_in_17_17,
	&CPSW0_cpts_sync_3_3_to_TIMESYNC_INTROUTER0_in_18_18,
};
static const struct Sciclient_rmIrqNode tisci_irq_CPSW0 = {
	.id = TISCI_DEV_CPSW0,
	.n_if = 4,
	.p_if = &tisci_if_CPSW0[0],
};

/* Start of DMASS0_INTAGGR_0 interface definition */
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_16_23_to_R5FSS1_CORE0_intr_8_15 = {
	.lbase = 16,
	.len = 8,
	.rid = TISCI_DEV_R5FSS1_CORE0,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_0_15_to_R5FSS1_CORE0_intr_64_79 = {
	.lbase = 0,
	.len = 16,
	.rid = TISCI_DEV_R5FSS1_CORE0,
	.rbase = 64,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_24_31_to_R5FSS1_CORE0_intr_153_160 = {
	.lbase = 24,
	.len = 8,
	.rid = TISCI_DEV_R5FSS1_CORE0,
	.rbase = 153,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_16_23_to_R5FSS1_CORE1_intr_8_15 = {
	.lbase = 16,
	.len = 8,
	.rid = TISCI_DEV_R5FSS1_CORE1,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_0_15_to_R5FSS1_CORE1_intr_64_79 = {
	.lbase = 0,
	.len = 16,
	.rid = TISCI_DEV_R5FSS1_CORE1,
	.rbase = 64,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_24_31_to_R5FSS1_CORE1_intr_153_160 = {
	.lbase = 24,
	.len = 8,
	.rid = TISCI_DEV_R5FSS1_CORE1,
	.rbase = 153,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_32_39_to_C7X256V0_CLEC_gic_spi_96_103 = {
	.lbase = 32,
	.len = 8,
	.rid = TISCI_DEV_C7X256V0_CLEC,
	.rbase = 96,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_84_99_to_C7X256V0_CLEC_soc_events_in_16_31 = {
	.lbase = 84,
	.len = 16,
	.rid = TISCI_DEV_C7X256V0_CLEC,
	.rbase = 16,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_32_39_to_C7X256V1_CLEC_gic_spi_96_103 = {
	.lbase = 32,
	.len = 8,
	.rid = TISCI_DEV_C7X256V1_CLEC,
	.rbase = 96,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_100_115_to_C7X256V1_CLEC_soc_events_in_16_31 = {
	.lbase = 100,
	.len = 16,
	.rid = TISCI_DEV_C7X256V1_CLEC,
	.rbase = 16,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_72_79_to_WKUP_R5FSS0_CORE0_intr_8_15 = {
	.lbase = 72,
	.len = 8,
	.rid = TISCI_DEV_WKUP_R5FSS0_CORE0,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_40_71_to_WKUP_R5FSS0_CORE0_intr_64_95 = {
	.lbase = 40,
	.len = 32,
	.rid = TISCI_DEV_WKUP_R5FSS0_CORE0,
	.rbase = 64,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_168_175_to_R5FSS0_CORE0_intr_8_15 = {
	.lbase = 168,
	.len = 8,
	.rid = TISCI_DEV_R5FSS0_CORE0,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_152_167_to_R5FSS0_CORE0_intr_64_79 = {
	.lbase = 152,
	.len = 16,
	.rid = TISCI_DEV_R5FSS0_CORE0,
	.rbase = 64,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_176_183_to_R5FSS0_CORE0_intr_153_160 = {
	.lbase = 176,
	.len = 8,
	.rid = TISCI_DEV_R5FSS0_CORE0,
	.rbase = 153,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_168_175_to_R5FSS0_CORE1_intr_8_15 = {
	.lbase = 168,
	.len = 8,
	.rid = TISCI_DEV_R5FSS0_CORE1,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_152_167_to_R5FSS0_CORE1_intr_64_79 = {
	.lbase = 152,
	.len = 16,
	.rid = TISCI_DEV_R5FSS0_CORE1,
	.rbase = 64,
};
const struct Sciclient_rmIrqIf DMASS0_INTAGGR_0_intaggr_vintr_pend_176_183_to_R5FSS0_CORE1_intr_153_160 = {
	.lbase = 176,
	.len = 8,
	.rid = TISCI_DEV_R5FSS0_CORE1,
	.rbase = 153,
};
const struct Sciclient_rmIrqIf * const tisci_if_DMASS0_INTAGGR_0[] = {
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_16_23_to_R5FSS1_CORE0_intr_8_15,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_0_15_to_R5FSS1_CORE0_intr_64_79,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_24_31_to_R5FSS1_CORE0_intr_153_160,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_16_23_to_R5FSS1_CORE1_intr_8_15,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_0_15_to_R5FSS1_CORE1_intr_64_79,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_24_31_to_R5FSS1_CORE1_intr_153_160,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_32_39_to_C7X256V0_CLEC_gic_spi_96_103,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_84_99_to_C7X256V0_CLEC_soc_events_in_16_31,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_32_39_to_C7X256V1_CLEC_gic_spi_96_103,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_100_115_to_C7X256V1_CLEC_soc_events_in_16_31,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_72_79_to_WKUP_R5FSS0_CORE0_intr_8_15,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_40_71_to_WKUP_R5FSS0_CORE0_intr_64_95,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_168_175_to_R5FSS0_CORE0_intr_8_15,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_152_167_to_R5FSS0_CORE0_intr_64_79,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_176_183_to_R5FSS0_CORE0_intr_153_160,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_168_175_to_R5FSS0_CORE1_intr_8_15,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_152_167_to_R5FSS0_CORE1_intr_64_79,
	&DMASS0_INTAGGR_0_intaggr_vintr_pend_176_183_to_R5FSS0_CORE1_intr_153_160,
};
static const struct Sciclient_rmIrqNode tisci_irq_DMASS0_INTAGGR_0 = {
	.id = TISCI_DEV_DMASS0_INTAGGR_0,
	.n_if = 18,
	.p_if = &tisci_if_DMASS0_INTAGGR_0[0],
};

/* Start of TIMER0 interface definition */
const struct Sciclient_rmIrqIf TIMER0_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_216_216 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 216,
};
const struct Sciclient_rmIrqIf TIMER0_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_0_0 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 0,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER0[] = {
	&TIMER0_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_216_216,
	&TIMER0_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_0_0,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER0 = {
	.id = TISCI_DEV_TIMER0,
	.n_if = 2,
	.p_if = &tisci_if_TIMER0[0],
};

/* Start of TIMER1 interface definition */
const struct Sciclient_rmIrqIf TIMER1_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_217_217 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 217,
};
const struct Sciclient_rmIrqIf TIMER1_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_1_1 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 1,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER1[] = {
	&TIMER1_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_217_217,
	&TIMER1_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_1_1,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER1 = {
	.id = TISCI_DEV_TIMER1,
	.n_if = 2,
	.p_if = &tisci_if_TIMER1[0],
};

/* Start of TIMER2 interface definition */
const struct Sciclient_rmIrqIf TIMER2_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_218_218 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 218,
};
const struct Sciclient_rmIrqIf TIMER2_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_2_2 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 2,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER2[] = {
	&TIMER2_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_218_218,
	&TIMER2_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_2_2,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER2 = {
	.id = TISCI_DEV_TIMER2,
	.n_if = 2,
	.p_if = &tisci_if_TIMER2[0],
};

/* Start of TIMER3 interface definition */
const struct Sciclient_rmIrqIf TIMER3_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_219_219 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 219,
};
const struct Sciclient_rmIrqIf TIMER3_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_3_3 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 3,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER3[] = {
	&TIMER3_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_219_219,
	&TIMER3_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_3_3,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER3 = {
	.id = TISCI_DEV_TIMER3,
	.n_if = 2,
	.p_if = &tisci_if_TIMER3[0],
};

/* Start of TIMER4 interface definition */
const struct Sciclient_rmIrqIf TIMER4_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_220_220 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 220,
};
const struct Sciclient_rmIrqIf TIMER4_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_4_4 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 4,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER4[] = {
	&TIMER4_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_220_220,
	&TIMER4_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_4_4,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER4 = {
	.id = TISCI_DEV_TIMER4,
	.n_if = 2,
	.p_if = &tisci_if_TIMER4[0],
};

/* Start of TIMER5 interface definition */
const struct Sciclient_rmIrqIf TIMER5_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_221_221 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 221,
};
const struct Sciclient_rmIrqIf TIMER5_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_5_5 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 5,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER5[] = {
	&TIMER5_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_221_221,
	&TIMER5_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_5_5,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER5 = {
	.id = TISCI_DEV_TIMER5,
	.n_if = 2,
	.p_if = &tisci_if_TIMER5[0],
};

/* Start of TIMER6 interface definition */
const struct Sciclient_rmIrqIf TIMER6_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_222_222 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 222,
};
const struct Sciclient_rmIrqIf TIMER6_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_6_6 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 6,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER6[] = {
	&TIMER6_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_222_222,
	&TIMER6_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_6_6,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER6 = {
	.id = TISCI_DEV_TIMER6,
	.n_if = 2,
	.p_if = &tisci_if_TIMER6[0],
};

/* Start of TIMER7 interface definition */
const struct Sciclient_rmIrqIf TIMER7_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_223_223 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 223,
};
const struct Sciclient_rmIrqIf TIMER7_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_7_7 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 7,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER7[] = {
	&TIMER7_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_223_223,
	&TIMER7_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_7_7,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER7 = {
	.id = TISCI_DEV_TIMER7,
	.n_if = 2,
	.p_if = &tisci_if_TIMER7[0],
};

/* Start of TIMER8 interface definition */
const struct Sciclient_rmIrqIf TIMER8_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_224_224 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 224,
};
const struct Sciclient_rmIrqIf TIMER8_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_8_8 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 8,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER8[] = {
	&TIMER8_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_224_224,
	&TIMER8_timer_pwm_0_0_to_TIMESYNC_INTROUTER0_in_8_8,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER8 = {
	.id = TISCI_DEV_TIMER8,
	.n_if = 2,
	.p_if = &tisci_if_TIMER8[0],
};

/* Start of TIMER9 interface definition */
const struct Sciclient_rmIrqIf TIMER9_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_225_225 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 225,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER9[] = {
	&TIMER9_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_225_225,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER9 = {
	.id = TISCI_DEV_TIMER9,
	.n_if = 1,
	.p_if = &tisci_if_TIMER9[0],
};

/* Start of TIMER10 interface definition */
const struct Sciclient_rmIrqIf TIMER10_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_226_226 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 226,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER10[] = {
	&TIMER10_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_226_226,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER10 = {
	.id = TISCI_DEV_TIMER10,
	.n_if = 1,
	.p_if = &tisci_if_TIMER10[0],
};

/* Start of TIMER11 interface definition */
const struct Sciclient_rmIrqIf TIMER11_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_227_227 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 227,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER11[] = {
	&TIMER11_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_227_227,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER11 = {
	.id = TISCI_DEV_TIMER11,
	.n_if = 1,
	.p_if = &tisci_if_TIMER11[0],
};

/* Start of WKUP_GTC0 interface definition */
const struct Sciclient_rmIrqIf WKUP_GTC0_gtc_push_event_0_0_to_TIMESYNC_INTROUTER0_in_11_11 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 11,
};
const struct Sciclient_rmIrqIf * const tisci_if_WKUP_GTC0[] = {
	&WKUP_GTC0_gtc_push_event_0_0_to_TIMESYNC_INTROUTER0_in_11_11,
};
static const struct Sciclient_rmIrqNode tisci_irq_WKUP_GTC0 = {
	.id = TISCI_DEV_WKUP_GTC0,
	.n_if = 1,
	.p_if = &tisci_if_WKUP_GTC0[0],
};

/* Start of GPIO0 interface definition */
const struct Sciclient_rmIrqIf GPIO0_gpio_0_52_to_MAIN_GPIO_INTROUTER0_in_0_52 = {
	.lbase = 0,
	.len = 53,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 0,
};
const struct Sciclient_rmIrqIf GPIO0_gpio_55_59_to_MAIN_GPIO_INTROUTER0_in_55_59 = {
	.lbase = 55,
	.len = 5,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 55,
};
const struct Sciclient_rmIrqIf GPIO0_gpio_62_92_to_MAIN_GPIO_INTROUTER0_in_62_92 = {
	.lbase = 62,
	.len = 31,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 62,
};
const struct Sciclient_rmIrqIf GPIO0_gpio_bank_93_98_to_MAIN_GPIO_INTROUTER0_in_192_197 = {
	.lbase = 93,
	.len = 6,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 192,
};
const struct Sciclient_rmIrqIf * const tisci_if_GPIO0[] = {
	&GPIO0_gpio_0_52_to_MAIN_GPIO_INTROUTER0_in_0_52,
	&GPIO0_gpio_55_59_to_MAIN_GPIO_INTROUTER0_in_55_59,
	&GPIO0_gpio_62_92_to_MAIN_GPIO_INTROUTER0_in_62_92,
	&GPIO0_gpio_bank_93_98_to_MAIN_GPIO_INTROUTER0_in_192_197,
};
static const struct Sciclient_rmIrqNode tisci_irq_GPIO0 = {
	.id = TISCI_DEV_GPIO0,
	.n_if = 4,
	.p_if = &tisci_if_GPIO0[0],
};

/* Start of GPIO1 interface definition */
const struct Sciclient_rmIrqIf GPIO1_gpio_0_50_to_MAIN_GPIO_INTROUTER0_in_96_146 = {
	.lbase = 0,
	.len = 51,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 96,
};
const struct Sciclient_rmIrqIf GPIO1_gpio_72_72_to_MAIN_GPIO_INTROUTER0_in_168_168 = {
	.lbase = 72,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 168,
};
const struct Sciclient_rmIrqIf GPIO1_gpio_74_85_to_MAIN_GPIO_INTROUTER0_in_170_181 = {
	.lbase = 74,
	.len = 12,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 170,
};
const struct Sciclient_rmIrqIf GPIO1_gpio_bank_86_91_to_MAIN_GPIO_INTROUTER0_in_200_205 = {
	.lbase = 86,
	.len = 6,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 200,
};
const struct Sciclient_rmIrqIf * const tisci_if_GPIO1[] = {
	&GPIO1_gpio_0_50_to_MAIN_GPIO_INTROUTER0_in_96_146,
	&GPIO1_gpio_72_72_to_MAIN_GPIO_INTROUTER0_in_168_168,
	&GPIO1_gpio_74_85_to_MAIN_GPIO_INTROUTER0_in_170_181,
	&GPIO1_gpio_bank_86_91_to_MAIN_GPIO_INTROUTER0_in_200_205,
};
static const struct Sciclient_rmIrqNode tisci_irq_GPIO1 = {
	.id = TISCI_DEV_GPIO1,
	.n_if = 4,
	.p_if = &tisci_if_GPIO1[0],
};

/* Start of MCU_GPIO0 interface definition */
const struct Sciclient_rmIrqIf MCU_GPIO0_gpio_0_16_to_MCU_MCU_GPIO_INTROUTER0_in_0_16 = {
	.lbase = 0,
	.len = 17,
	.rid = TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
	.rbase = 0,
};
const struct Sciclient_rmIrqIf MCU_GPIO0_gpio_19_25_to_MCU_MCU_GPIO_INTROUTER0_in_19_25 = {
	.lbase = 19,
	.len = 7,
	.rid = TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
	.rbase = 19,
};
const struct Sciclient_rmIrqIf MCU_GPIO0_gpio_bank_26_27_to_MCU_MCU_GPIO_INTROUTER0_in_30_31 = {
	.lbase = 26,
	.len = 2,
	.rid = TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
	.rbase = 30,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCU_GPIO0[] = {
	&MCU_GPIO0_gpio_0_16_to_MCU_MCU_GPIO_INTROUTER0_in_0_16,
	&MCU_GPIO0_gpio_19_25_to_MCU_MCU_GPIO_INTROUTER0_in_19_25,
	&MCU_GPIO0_gpio_bank_26_27_to_MCU_MCU_GPIO_INTROUTER0_in_30_31,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCU_GPIO0 = {
	.id = TISCI_DEV_MCU_GPIO0,
	.n_if = 3,
	.p_if = &tisci_if_MCU_GPIO0[0],
};

/* Start of EPWM0 interface definition */
const struct Sciclient_rmIrqIf EPWM0_epwm_synco_o_0_0_to_TIMESYNC_INTROUTER0_in_9_9 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
	.rbase = 9,
};
const struct Sciclient_rmIrqIf * const tisci_if_EPWM0[] = {
	&EPWM0_epwm_synco_o_0_0_to_TIMESYNC_INTROUTER0_in_9_9,
};
static const struct Sciclient_rmIrqNode tisci_irq_EPWM0 = {
	.id = TISCI_DEV_EPWM0,
	.n_if = 1,
	.p_if = &tisci_if_EPWM0[0],
};

/* Start of WKUP_TIMER0 interface definition */
const struct Sciclient_rmIrqIf WKUP_TIMER0_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_214_214 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 214,
};
const struct Sciclient_rmIrqIf * const tisci_if_WKUP_TIMER0[] = {
	&WKUP_TIMER0_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_214_214,
};
static const struct Sciclient_rmIrqNode tisci_irq_WKUP_TIMER0 = {
	.id = TISCI_DEV_WKUP_TIMER0,
	.n_if = 1,
	.p_if = &tisci_if_WKUP_TIMER0[0],
};

/* Start of WKUP_TIMER1 interface definition */
const struct Sciclient_rmIrqIf WKUP_TIMER1_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_215_215 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 215,
};
const struct Sciclient_rmIrqIf * const tisci_if_WKUP_TIMER1[] = {
	&WKUP_TIMER1_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_215_215,
};
static const struct Sciclient_rmIrqNode tisci_irq_WKUP_TIMER1 = {
	.id = TISCI_DEV_WKUP_TIMER1,
	.n_if = 1,
	.p_if = &tisci_if_WKUP_TIMER1[0],
};

/* Start of MCRC64_0 interface definition */
const struct Sciclient_rmIrqIf MCRC64_0_int_mcrc_4_4_to_DMASS0_INTAGGR_0_intaggr_levi_pend_7_7 = {
	.lbase = 4,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 7,
};
const struct Sciclient_rmIrqIf MCRC64_0_dma_event_0_3_to_DMASS0_INTAGGR_0_intaggr_levi_pend_28_31 = {
	.lbase = 0,
	.len = 4,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 28,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCRC64_0[] = {
	&MCRC64_0_int_mcrc_4_4_to_DMASS0_INTAGGR_0_intaggr_levi_pend_7_7,
	&MCRC64_0_dma_event_0_3_to_DMASS0_INTAGGR_0_intaggr_levi_pend_28_31,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCRC64_0 = {
	.id = TISCI_DEV_MCRC64_0,
	.n_if = 2,
	.p_if = &tisci_if_MCRC64_0[0],
};

/* Start of DEBUGSS0 interface definition */
const struct Sciclient_rmIrqIf DEBUGSS0_davdma_level_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_27_27 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 27,
};
const struct Sciclient_rmIrqIf * const tisci_if_DEBUGSS0[] = {
	&DEBUGSS0_davdma_level_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_27_27,
};
static const struct Sciclient_rmIrqNode tisci_irq_DEBUGSS0 = {
	.id = TISCI_DEV_DEBUGSS0,
	.n_if = 1,
	.p_if = &tisci_if_DEBUGSS0[0],
};

/* Start of MCASP0 interface definition */
const struct Sciclient_rmIrqIf MCASP0_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_1_1 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 1,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCASP0[] = {
	&MCASP0_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_1_1,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCASP0 = {
	.id = TISCI_DEV_MCASP0,
	.n_if = 1,
	.p_if = &tisci_if_MCASP0[0],
};

/* Start of MCASP1 interface definition */
const struct Sciclient_rmIrqIf MCASP1_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_2_2 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 2,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCASP1[] = {
	&MCASP1_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_2_2,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCASP1 = {
	.id = TISCI_DEV_MCASP1,
	.n_if = 1,
	.p_if = &tisci_if_MCASP1[0],
};

/* Start of MCASP2 interface definition */
const struct Sciclient_rmIrqIf MCASP2_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_3_3 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 3,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCASP2[] = {
	&MCASP2_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_3_3,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCASP2 = {
	.id = TISCI_DEV_MCASP2,
	.n_if = 1,
	.p_if = &tisci_if_MCASP2[0],
};

/* Start of MCASP3 interface definition */
const struct Sciclient_rmIrqIf MCASP3_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_4_4 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 4,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCASP3[] = {
	&MCASP3_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_4_4,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCASP3 = {
	.id = TISCI_DEV_MCASP3,
	.n_if = 1,
	.p_if = &tisci_if_MCASP3[0],
};

/* Start of MCASP4 interface definition */
const struct Sciclient_rmIrqIf MCASP4_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_5_5 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_DMASS0_INTAGGR_0,
	.rbase = 5,
};
const struct Sciclient_rmIrqIf * const tisci_if_MCASP4[] = {
	&MCASP4_xmit_dma_event_req_0_0_to_DMASS0_INTAGGR_0_intaggr_levi_pend_5_5,
};
static const struct Sciclient_rmIrqNode tisci_irq_MCASP4 = {
	.id = TISCI_DEV_MCASP4,
	.n_if = 1,
	.p_if = &tisci_if_MCASP4[0],
};

/* Start of TIMER12 interface definition */
const struct Sciclient_rmIrqIf TIMER12_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_228_228 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 228,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER12[] = {
	&TIMER12_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_228_228,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER12 = {
	.id = TISCI_DEV_TIMER12,
	.n_if = 1,
	.p_if = &tisci_if_TIMER12[0],
};

/* Start of TIMER13 interface definition */
const struct Sciclient_rmIrqIf TIMER13_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_229_229 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 229,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER13[] = {
	&TIMER13_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_229_229,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER13 = {
	.id = TISCI_DEV_TIMER13,
	.n_if = 1,
	.p_if = &tisci_if_TIMER13[0],
};

/* Start of TIMER14 interface definition */
const struct Sciclient_rmIrqIf TIMER14_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_230_230 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 230,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER14[] = {
	&TIMER14_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_230_230,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER14 = {
	.id = TISCI_DEV_TIMER14,
	.n_if = 1,
	.p_if = &tisci_if_TIMER14[0],
};

/* Start of TIMER15 interface definition */
const struct Sciclient_rmIrqIf TIMER15_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_231_231 = {
	.lbase = 0,
	.len = 1,
	.rid = TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
	.rbase = 231,
};
const struct Sciclient_rmIrqIf * const tisci_if_TIMER15[] = {
	&TIMER15_timer_pwm_0_0_to_MAIN_GPIO_INTROUTER0_in_231_231,
};
static const struct Sciclient_rmIrqNode tisci_irq_TIMER15 = {
	.id = TISCI_DEV_TIMER15,
	.n_if = 1,
	.p_if = &tisci_if_TIMER15[0],
};


const struct Sciclient_rmIrqNode * const gRmIrqTree[RM_IRQ_TREE_MAX] = {
	&tisci_irq_MAIN_GPIO_INTROUTER0,
	&tisci_irq_MCU_MCU_GPIO_INTROUTER0,
	&tisci_irq_TIMESYNC_INTROUTER0,
	&tisci_irq_CPSW0,
	&tisci_irq_DMASS0_INTAGGR_0,
	&tisci_irq_TIMER0,
	&tisci_irq_TIMER1,
	&tisci_irq_TIMER2,
	&tisci_irq_TIMER3,
	&tisci_irq_TIMER4,
	&tisci_irq_TIMER5,
	&tisci_irq_TIMER6,
	&tisci_irq_TIMER7,
	&tisci_irq_TIMER8,
	&tisci_irq_TIMER9,
	&tisci_irq_TIMER10,
	&tisci_irq_TIMER11,
	&tisci_irq_WKUP_GTC0,
	&tisci_irq_GPIO0,
	&tisci_irq_GPIO1,
	&tisci_irq_MCU_GPIO0,
	&tisci_irq_EPWM0,
	&tisci_irq_WKUP_TIMER0,
	&tisci_irq_WKUP_TIMER1,
	&tisci_irq_MCRC64_0,
	&tisci_irq_DEBUGSS0,
	&tisci_irq_MCASP0,
	&tisci_irq_MCASP1,
	&tisci_irq_MCASP2,
	&tisci_irq_MCASP3,
	&tisci_irq_MCASP4,
	&tisci_irq_TIMER12,
	&tisci_irq_TIMER13,
	&tisci_irq_TIMER14,
	&tisci_irq_TIMER15,
};

const uint32_t gRmIrqTreeCount = sizeof(gRmIrqTree)/sizeof(gRmIrqTree[0]);
