/*
 * atf_optee.lds - simple linker file for stand-alone ATF/OPTEE booting
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */
OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)
INPUT(bl31.bin)
INPUT(bl32.bin)
/* Internal binary tee-pager.bin = bl32.bin in PSDK Linux */
/* INPUT(tee-pager.bin) */

SECTIONS
{
 . = 0x0000000070000000;
 atf = .;
 .atf : { bl31.bin }
 . = 0x000000009e800000;
 tee = .;
 .tee : { bl32.bin }
 /* Internal binary tee-pager.bin = bl32.bin in PSDK Linux */
 /* .tee : { tee-pager.bin } */
}
