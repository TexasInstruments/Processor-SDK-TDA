/*
 * ifs_qnx.lds - simple linker file for stand-alone QNX IFS booting
 *
 * Copyright (C) 2026 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */
OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)
INPUT(qnx-ifs)
INPUT(dtb.bin)
SECTIONS
{
 . = 0x0000000080080000;
 qnx = .;
 .qnx : { qnx-ifs }
 . = 0x0000000088000000;
 dtb = .;
 .dtb : { dtb.bin }
}
