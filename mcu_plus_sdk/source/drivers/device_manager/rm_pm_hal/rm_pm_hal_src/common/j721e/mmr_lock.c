/*
 * DMSC firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 * Copyright (C) 2019-2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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

#include <types/array_size.h>
#include <lib/ioremap.h>
#include <lib/mmr_lock.h>
#include <lib/trace.h>
#include <osal_hwi.h>

#define MAIN_CTRL_BASE                                  (0x00100000U)
#define WKUP_CTRL_BASE                                  (0x43000000U)
#define MCU_CTRL_BASE                                   (0x40F00000U)

#define CTRL_MMR0_PARTITION_SIZE                        (0x4000U)

#define CTRLMMR_LOCK_KICK0                              (0x01008U)
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL                   (0x68ef3490U)
#define CTRLMMR_LOCK_KICK0_LOCK_VAL                     (0x0)
#define CTRLMMR_LOCK_KICK0_UNLOCKED_MASK                BIT(0)
#define CTRLMMR_LOCK_KICK0_UNLOCKED_SHIFT               (0U)
#define CTRLMMR_LOCK_KICK1                              (0x0100cU)
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL                   (0xd172bc5aU)
#define CTRLMMR_LOCK_KICK1_LOCK_VAL                     (0x0U)

#define MMR_WKUP_CTRL_BASE_PARTITION_2                  BIT(0)
#define MMR_MAIN_CTRL_BASE_PARTITION_2                  BIT(1)
#define MMR_MCU_CTRL_BASE_PARTITION_2                   BIT(2)
#define MMR_WKUP_CTRL_BASE_PARTITION_6                  BIT(3)

#define MMR_WKUPMCU2MAIN_LPSC_STATUS                    0x42000810U
#define MMR_LPSC_STATUS_ENABLE                          ((u32) 0x03U)

static volatile s32 mmr_lock_count = 0;
static volatile u32 mmr_read_value = 0U;
static volatile s32 main_mmr_lock_count = 0;
static volatile u32 main_mmr_read_value = 0U;

void mmr_read(u32 base, u32 partition, u32 value)
{
	/* Translate the base address */
	u32 part_base = base + (partition * CTRL_MMR0_PARTITION_SIZE);

	if ((readl(part_base + CTRLMMR_LOCK_KICK0) & CTRLMMR_LOCK_KICK0_UNLOCKED_MASK) != CTRLMMR_LOCK_KICK0_UNLOCKED_MASK) {
		mmr_read_value |= value;
	}
}

void mmr_unlock(u32 base, u32 partition)
{
	/* Translate the base address */
	u32 part_base = base + (partition * CTRL_MMR0_PARTITION_SIZE);

	/*
	 * Unlock the requested partition if locked using two-step sequence.
	 * This register does not read back the value written.
	 */
	writel(CTRLMMR_LOCK_KICK0_UNLOCK_VAL, part_base + CTRLMMR_LOCK_KICK0);
	writel(CTRLMMR_LOCK_KICK1_UNLOCK_VAL, part_base + CTRLMMR_LOCK_KICK1);
}

void mmr_lock(u32 base, u32 partition)
{
	/* Translate the base address */
	u32 part_base = base + (partition * CTRL_MMR0_PARTITION_SIZE);

	/*
	 * Unlock the requested partition if locked using two-step sequence.
	 * This register does not read back the value written.
	 */
	writel(CTRLMMR_LOCK_KICK0_LOCK_VAL, part_base + CTRLMMR_LOCK_KICK0);
	writel(CTRLMMR_LOCK_KICK1_LOCK_VAL, part_base + CTRLMMR_LOCK_KICK1);
}

void mmr_unlock_all(void)
{
	u32 key = osal_hwip_disable();
	u32 wkupmcu2main_lpsc_status = (u32) (readl(MMR_WKUPMCU2MAIN_LPSC_STATUS) & MMR_LPSC_STATUS_ENABLE);

	/* Initailize the value to zero */
	mmr_read_value = 0U;

	/* Check the previous unlock status of MMR partitions and store the status */
	mmr_read(WKUP_CTRL_BASE, 2, MMR_WKUP_CTRL_BASE_PARTITION_2);
	mmr_read(MCU_CTRL_BASE, 2, MMR_MCU_CTRL_BASE_PARTITION_2);
	mmr_read(WKUP_CTRL_BASE, 6, MMR_WKUP_CTRL_BASE_PARTITION_6);

	if (mmr_lock_count == 0) {
		/* Unlock CLKSEL MMR partitions */
		mmr_unlock(WKUP_CTRL_BASE, 2);
		mmr_unlock(MCU_CTRL_BASE, 2);
		/* Unlock WKUP_MCU_WARM_RST_CTRL MMR partitions*/
		mmr_unlock(WKUP_CTRL_BASE, 6);
	}

	/* Unlock Main MMR's only if Main domain is ON and accessible by DM. */
	if (wkupmcu2main_lpsc_status == MMR_LPSC_STATUS_ENABLE) {
		/*
		 * Update the main MMR read value to zero only if main domain is ON and accessible.
		 * This is to avoid the false update of main MMR read value if main domain is OFF.
		 */
		main_mmr_read_value = 0U;
		mmr_read(MAIN_CTRL_BASE, 2, MMR_MAIN_CTRL_BASE_PARTITION_2);
		main_mmr_read_value = ((u32) (mmr_read_value & MMR_MAIN_CTRL_BASE_PARTITION_2));
		/*
		 * Main MMR lock count is seperated to avoid false update of main MMR read value
		 * when main domain is OFF.
		 */
		if (main_mmr_lock_count == 0) {
			mmr_unlock(MAIN_CTRL_BASE, 2);
		}
		main_mmr_lock_count += 1;
	}

	mmr_lock_count += 1;
	osal_hwip_restore(key);
}

void mmr_lock_all(void)
{
	u32 key = osal_hwip_disable();
	u32 wkupmcu2main_lpsc_status = (u32) (readl(MMR_WKUPMCU2MAIN_LPSC_STATUS) & MMR_LPSC_STATUS_ENABLE);

	mmr_lock_count -= 1;
	if (mmr_lock_count == 0) {
		/* Lock MMR partitions only if it is locked before unlocking all MMRs in mmr_unlock_all */

		if ((mmr_read_value & MMR_WKUP_CTRL_BASE_PARTITION_2) == MMR_WKUP_CTRL_BASE_PARTITION_2) {
			/* Lock CLKSEL MMR partitions */
			mmr_lock(WKUP_CTRL_BASE, 2);
		}
		if ((mmr_read_value & MMR_MCU_CTRL_BASE_PARTITION_2) == MMR_MCU_CTRL_BASE_PARTITION_2) {
			/* Lock CLKSEL MMR partitions */
			mmr_lock(MCU_CTRL_BASE, 2);
		}
		if ((mmr_read_value & MMR_WKUP_CTRL_BASE_PARTITION_6) == MMR_WKUP_CTRL_BASE_PARTITION_6) {
			/* lock WKUP_MCU_WARM_RST_CTRL MMR partitions*/
			mmr_lock(WKUP_CTRL_BASE, 6);
		}
	}

	/* Lock Main MMR's only if Main domain is ON and accessible by DM. */
	if (wkupmcu2main_lpsc_status == MMR_LPSC_STATUS_ENABLE) {
		main_mmr_lock_count -= 1;
		if (main_mmr_lock_count == 0) {
			if ((main_mmr_read_value & MMR_MAIN_CTRL_BASE_PARTITION_2) == MMR_MAIN_CTRL_BASE_PARTITION_2) {
				/* Lock CLKSEL MMR partitions */
				mmr_lock(MAIN_CTRL_BASE, 2);
			}
		}
	}

	osal_hwip_restore(key);
}
