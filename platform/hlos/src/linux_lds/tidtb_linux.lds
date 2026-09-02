/*
 * tidtb_linux.lds - simple linker file for stand-alone DTB loading
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */
OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)
INPUT(k3-j721e-common-proc-board.dtb)
SECTIONS
{
 . = 0x0000000082000000;
 dtb = .;
 .dtb : { k3-j721e-common-proc-board.dtb }
}
