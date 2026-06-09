/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \defgroup IP_FMA_SMS
 */

/**
 *  \ingroup  IP_FMA_SMS
 *  \section  IP_FMA_SMS Overview
 *            SMS (SMS) provides APIs to read
 *            the  configuration  registers  that sets firewall
 *            configuration and security control configurations
 *            and validate it against the Golden Reference.
 *
 *  @{
 */

/**
 *  \file     ip_fma_sms.h
 *
 *  \brief    This file contains SMS library interfaces and related data structures.
 *
 */

#ifndef IP_FMA_SMS_H_
#define IP_FMA_SMS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * TIFS DMTIMER registers offsets
 */
#define DMTIMER_TIDR           0x00
#define DMTIMER_TIOCP_CFG      0x10
#define DMTIMER_IRQ_EOI        0x20
#define DMTIMER_IRQSTATUS_RAW  0x24
#define DMTIMER_IRQSTATUS      0x28
#define DMTIMER_IRQSTATUS_SET  0x2C
#define DMTIMER_IRQSTATUS_CLR  0x30
#define DMTIMER_IRQWAKEEN      0x34
#define DMTIMER_TCLR           0x38
#define DMTIMER_TCRR           0x3C
#define DMTIMER_TLDR           0x40
#define DMTIMER_TTGR           0x44
#define DMTIMER_TWPS           0x48
#define DMTIMER_TMAR           0x4C
#define DMTIMER_TCAR1          0x50
#define DMTIMER_TSICR          0x54
#define DMTIMER_TCAR2          0x58
#define DMTIMER_TPIR           0x5C
#define DMTIMER_TNIR           0x60
#define DMTIMER_TCVR           0x64
#define DMTIMER_TOCR           0x68
#define DMTIMER_TOWR           0x6C

/**
 * TIFS WDT RTI registers offsets
 */
#define RTI_GCTRL           0x00
#define RTI_TBCTRL          0x04
#define RTI_CAPCTRL         0x08
#define RTI_COMPCTRL        0x0C
#define RTI_FRC0            0x10
#define RTI_UC0             0x14
#define RTI_CPUC0           0x18
#define RTI_CAFRC0          0x20
#define RTI_CAUC0           0x24
#define RTI_FRC1            0x30
#define RTI_UC1             0x34
#define RTI_CPUC1           0x38
#define RTI_CAFRC1          0x40
#define RTI_CAUC1           0x44
#define RTI_COMP0           0x50
#define RTI_UDCP0           0x54
#define RTI_COMP1           0x58
#define RTI_UDCP1           0x5C
#define RTI_COMP2           0x60
#define RTI_UDCP2           0x64
#define RTI_COMP3           0x68
#define RTI_UDCP3           0x6C
#define RTI_TBLCOMP         0x70
#define RTI_TBHCOMP         0x74
#define RTI_SETINT          0x80
#define RTI_CLEARINT        0x84
#define RTI_INTFLAG         0x88
#define RTI_DWDCTRL         0x90
#define RTI_DWDPRLD         0x94
#define RTI_WDSTATUS        0x98
#define RTI_WDKEY           0x9C
#define RTI_DWWDRXNCTRL     0xA4
#define RTI_DWWDSIZECTRL    0xA8
#define RTI_INTCLRENABLE    0xAC
#define RTI_COMP0CLR        0xB0
#define RTI_COMP1CLR        0xB4
#define RTI_COMP2CLR        0xB8
#define RTI_COMP3CLR        0xBC

/**
 * TIFS RTI MMR
 */
#define RTI_MMR_LOCK0_KICK0         0x020
#define RTI_MMR_LOCK0_KICK1         0x024
#define RTI_MMR_WWRTI_CTRL          0x040
#define RTI_MMR_WWRTI_CLOCK_CTRL    0x050

/**
 * TIFS PWR MMR registers offsets
 */
#define PWR_MMR_LOCK0_KICK0     0x020
#define PWR_MMR_LOCK0_KICK1     0x024
#define PWR_MMR_PMCTRL_SYS      0x080
#define PWR_MMR_PMCTRL_MOSC     0x090
#define PWR_MMR_PMCTRL_DMSC     0x094
#define PWR_MMR_CPU_STCALIB     0x140
#define PWR_MMR_PM_DMTIMER_CTRL 0x200
#define PWR_MMR_PM_PERMISSION   0x300

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * Structure containing values for TIFS DMTIMER registers.
 */
typedef struct
{
    uint32_t tidr;
    uint32_t tiocp_cfg;
    uint32_t irq_eoi;
    uint32_t irqstatus_raw;
    uint32_t irqstatus;
    uint32_t irqstatus_set;
    uint32_t irqstatus_clr;
    uint32_t irqwakeen;
    uint32_t tclr;
    uint32_t tcrr;
    uint32_t tldr;
    uint32_t ttgr;
    uint32_t twps;
    uint32_t tmar;
    uint32_t tcar1;
    uint32_t tsicr;
    uint32_t tcar2;
    uint32_t tpir;
    uint32_t tnir;
    uint32_t tcvr;
    uint32_t tocr;
    uint32_t towr;
} IpFma_SmsDmtimerRegs;

/**
 * Structure containing values for TIFS WDT RTI registers.
 */
typedef struct
{
    uint32_t gctrl;
    uint32_t tbctrl;
    uint32_t capctrl;
    uint32_t compctrl;
    uint32_t frc0;
    uint32_t uc0;
    uint32_t cpuc0;
    uint32_t cafrc0;
    uint32_t cauc0;
    uint32_t frc1;
    uint32_t uc1;
    uint32_t cpuc1;
    uint32_t cafrc1;
    uint32_t cauc1;
    uint32_t comp0;
    uint32_t udcp0;
    uint32_t comp1;
    uint32_t udcp1;
    uint32_t comp2;
    uint32_t udcp2;
    uint32_t comp3;
    uint32_t udcp3;
    uint32_t tblcomp;
    uint32_t tbhcomp;
    uint32_t setint;
    uint32_t clearint;
    uint32_t intflag;
    uint32_t dwdctrl;
    uint32_t dwdprld;
    uint32_t wdstatus;
    uint32_t wdkey;
    uint32_t dwwdrxnctrl;
    uint32_t dwwdsizectrl;
    uint32_t intclrenable;
    uint32_t comp0clr;
    uint32_t comp1clr;
    uint32_t comp2clr;
    uint32_t comp3clr;
} IpFma_SmsRtiRegs;

/**
 * Structure containing values for TIFS RTI MMR registers.
 */
typedef struct
{
    uint32_t lock0_kick0;
    uint32_t lock0_kick1;
    uint32_t wwrti_ctrl;
    uint32_t wwrti_clock_ctrl;
} IpFma_SmsRtiMmrRegs;

/**
 * Structure containing values for TIFS PWR MMR registers.
 */
typedef struct
{
    uint32_t lock0_kick0;
    uint32_t lock0_kick1;
    uint32_t pmctrl_sys;
    uint32_t pmctrl_mosc;
    uint32_t pmctrl_dmsc;
    uint32_t cpu_stcalib;
    uint32_t pm_dmtimer_ctrl;
    uint32_t pm_permission;
} IpFma_SmsPwrMmrRegs;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   API to read SMS register list (TIFS)
 *
 * \return  status   IPFMA_OK:      Registers read succesfully.
 *                   IPFMA_E_PARAM: Registers not read succesfully - invalid parameters passed.
 */
IpFma_Status IpFma_Sms_GetDmtimerRegs(IpFma_SmsDmtimerRegs* reg, uint8_t id);
IpFma_Status IpFma_Sms_GetRtiRegs(IpFma_SmsRtiRegs* reg);
IpFma_Status IpFma_Sms_GetRtiMmrRegs(IpFma_SmsRtiMmrRegs* reg);
IpFma_Status IpFma_Sms_GetPwrMmrRegs(IpFma_SmsPwrMmrRegs* reg);

/**
 * \brief   API to verify SMS register list (TIFS)
 *
 * \return  status   IPFMA_OK:          Registers values have not been modified.
 *                   IPFMA_E_PARAM:     Registers not read succesfully - invalid parameters passed.
 *                   IPFMA_E_MISMATCH:  Registers values have been modified.
 */
IpFma_Status IpFma_Sms_VerifyDmtimerRegs(const IpFma_SmsDmtimerRegs* reg, uint8_t id);
IpFma_Status IpFma_Sms_VerifyRtiRegs(const IpFma_SmsRtiRegs* reg);
IpFma_Status IpFma_Sms_VerifyRtiMmrRegs(const IpFma_SmsRtiMmrRegs* reg);
IpFma_Status IpFma_Sms_VerifyPwrMmrRegs(const IpFma_SmsPwrMmrRegs* reg);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_SMS_H_ */
/** @} */
