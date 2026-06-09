#
# Copyright (c) 2026 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

import os, sys, re
from . import *

class DtsFile :


    def __init__(self, memoryMap, name="app_mem_map.h"):
        self.memoryMap = memoryMap;
        self.name = name;
        self.indent = "\t";
        self.indent_level = 0;

    def open(self) :
        os.makedirs(os.path.dirname(self.name), exist_ok=True);
        self.dts_file = open(self.name, "w");

    def write_line(self, string) :
        for i in range(0, self.indent_level):
            self.dts_file.write(self.indent);
        self.dts_file.write(string + "\n");

    def write_comment(self,string) :
        self.write_line( "/* " + string + " */");

    def open_brace(self) :
        self.write_line("{");
        self.inc_indent();

    def inc_indent(self) :
        self.indent_level = self.indent_level+1;

    def dec_indent(self) :
        self.indent_level = self.indent_level-1;

    def close_brace(self) :
        self.dec_indent();
        self.write_line("};");

    def close(self) :
        self.dts_file.close();

    @staticmethod
    def sortKey(mmapTuple) :
        return mmapTuple[1].origin;

    def export(self) :
        KB = 1024;
        MB = KB*KB;
        GB = KB*MB;

        header = " \n" \
                 " /* \n" \
                 "  * IMPORTANT NOTE: Follow the below instructions to apply the updated memory map to the linux dtsi file,\n" \
                 "  * \n" \
                 "  * 1. Copy the contents of the generated k3-{SOC}-rtos-memory-map.dtsi file and replace the\n" \
                 "  *      reserved_memory section of the k3-{SOC}-rtos-memory-map.dtsi file that is located here:\n" \
                 "  *      PSDK_LINUX_PATH/board-support/linux*/arch/arm64/boot/dts/ti\n" \
                 "  * \n" \
                 "  * 2. Rebuild the dtb and dtbo from the PSDK_LINUX_PATH directory with the following:\n" \
                 "  *      make linux-dtbs\n" \
                 "  * \n" \
                 "  * 3. Install the dtb and dtbo to the rootfs/boot folder on the SD card from the same directory with the following:\n" \
                 "  *      sudo make linux-dtbs_install; sync\n" \
                 "  * \n " \
                 "  */\n"

        print(header);
        self.open();
        self.write_line( "" );
        self.write_line( header );
        self.write_line( "" );
        self.inc_indent();
        for key,memSection in sorted(self.memoryMap.memoryMap.items(), key=CHeaderFile.sortKey):
            if(memSection.origin_tag) :
                string = '%s: %s@%08x {' % (memSection.dtsLabelName, memSection.dtsNodeName, memSection.origin );
            else:
                string = '%s: %s {' % (memSection.dtsLabelName, memSection.dtsNodeName );
            self.write_line( string );
            self.inc_indent();
            if(memSection.printCompatibility) :
                string = 'compatible = "%s";' % (memSection.compatibility);
                self.write_line( string );
            if (memSection.split_origin) :
                val_lo = memSection.origin & 0xFFFFFFFF;
                val_hi = (memSection.origin & 0xFF00000000 ) >> 32;
                string = 'reg = <0x%02x 0x%08x 0x00 0x%08x>;' % (val_hi, val_lo, memSection.length);
            else:
                string = 'reg = <0x00 0x%08x 0x00 0x%08x>;' % (memSection.origin, memSection.length);
            self.write_line( string );
            if( memSection.alignment) :
                self.write_line( 'alignment = <0x1000>;' );
            if( memSection.no_map) :
                self.write_line( 'no-map;' );
            self.close_brace();
        self.dec_indent();
        self.dec_indent();
        self.write_line( "" );
        self.close();
