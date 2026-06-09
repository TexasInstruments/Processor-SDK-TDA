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

class CHeaderFile :


    def __init__(self, memoryMap, phys_addr_base, virt_addr_base, name="app_mem_map.h"):
        self.memoryMap = memoryMap;
        self.name = name;
        self.indent = "    ";
        self.indent_level = 0;
        self.phys_addr_base = phys_addr_base;
        self.virt_addr_base = virt_addr_base;

    def open(self) :
        os.makedirs(os.path.dirname(self.name), exist_ok=True);
        self.c_header_file = open(self.name, "w");
        self.c_header_file.write ( Common.tiCopyrightHeader );

    def write_line(self, string) :
        for i in range(0, self.indent_level):
            self.c_header_file.write(self.indent);
        self.c_header_file.write(string + "\n");

    def write_comment(self,string) :
        self.write_line( "/* " + string + " */");

    def open_brace(self) :
        self.write_line("{");
        self.indent_level = self.indent_level+1;

    def close_brace(self) :
        self.indent_level = self.indent_level-1;
        self.write_line("}");

    def close(self) :
        self.c_header_file.close();

    @staticmethod
    def sortKey(mmapTuple) :
        return mmapTuple[1].origin;

    def export(self) :
        KB = 1024;
        MB = KB*KB;
        GB = KB*MB;

        self.open();
        self.write_line("#ifndef APP_MEM_MAP_H");
        self.write_line("#define APP_MEM_MAP_H");
        self.write_line( "" );
        self.write_line( "" );
        for key,memSection in sorted(self.memoryMap.memoryMap.items(), key=CHeaderFile.sortKey):
            sizeString = "";
            if (memSection.length > GB ) :
                sizeString = '%5.2f GB' % (memSection.length/GB)
            elif (memSection.length > MB ) :
                sizeString = '%5.2f MB' % (memSection.length/MB)
            elif (memSection.length > KB ) :
                sizeString = '%5.2f KB' % (memSection.length/KB)
            else :
                sizeString = '%d B' % (memSection.length)
            self.write_comment( memSection.comment + ' [ size %s ]' % (sizeString));
            attrString = "";
            if memSection.attributes != "" :
                attrString = '( %4s )' % (memSection.attributes);
            string = '#define %s_ADDR (0x%08Xu)' % (memSection.name, memSection.origin);
            self.write_line( string );
            string = '#define %s_SIZE (0x%08Xu)' % (memSection.name, memSection.length);
            self.write_line( string );
            self.write_line( "" );
        self.write_line( "#define DDR_64BIT_BASE_VADDR (0x%08Xu)"  % (self.virt_addr_base));
        self.write_line( "#define DDR_64BIT_BASE_PADDR (0x%08Xu)"  % (self.phys_addr_base));
        self.write_line( "" );
        self.write_line("#endif /* APP_MEM_MAP_H */");
        self.write_line( "" );
        self.close();
