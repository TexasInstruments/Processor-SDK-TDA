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

class MemSection :

    def __init__(self, name, attributes, origin, length, comment="") :
        self.name = name;
        self.attributes = attributes;
        self.origin = origin;
        self.length = length;
        self.dtsNodeName = name
        self.dtsLabelName = name
        self.printCompatibility = True
        self.compatibility = "shared-dma-pool"
        self.no_map = True
        self.origin_tag = True
        self.split_origin = False;
        self.alignment = False;

        if comment == "" :
            self.comment = self.name;
        else:
            self.comment = comment;

    def setDtsName(self, labelName, nodeName) :
        self.dtsLabelName = labelName;
        self.dtsNodeName = nodeName;

    def setCompatibility(self, compatibility) :
        self.compatibility = compatibility;

    def setNoMap(self, value) :
        self.no_map = value;

    def setOriginTag(self, value) :
        self.origin_tag = value;

    def splitOrigin(self, value) :
        self.split_origin = value;

    def setAlignment(self, value) :
        self.alignment = value;

    def setPrintCompatibility(self, value) :
        self.printCompatibility = value;

    def concat(self, memSection) :
        # first section
        if self.origin == 0 and self.length == 0 :
            self.origin = memSection.origin
            self.length = memSection.length
        else:
            start_addr = self.origin;
            end_addr = self.origin + self.length;
            # concat section, length is calculated assuming sections are contigous
            if (memSection.origin < start_addr) :
                start_addr = memSection.origin;
            if (memSection.origin + memSection.length) > (end_addr) :
                end_addr = memSection.origin + memSection.length
            self.origin = start_addr;
            self.length = end_addr - start_addr;

    def __str__(self) :
        return self.name + " (" + self.attributes + ") "  \
               + " origin = " + hex(self.origin) + ", length = " + hex(self.length) \
               + " /* " + self.comment + " */";

class MemoryMap :

    def __init__(self, name="MyMemoryMap") :
        self.memoryMap = {};
        self.name = name;
        return;

    def addMemSection(self, memSection) :
        self.memoryMap[memSection.name] = memSection;

    def __str__(self) :
        self_str = self.name + " has " + str(len(self.memoryMap)) + " memory section(s) as listed below,\n";
        for key,value in self.memoryMap.items() :
            self_str += str(value) + '\n';
        return self_str;

    def checkOverlap(self) :
        is_overlap = False;
        for curKey,curMemSection in sorted(self.memoryMap.items()):
            cur_start_addr = curMemSection.origin;
            cur_end_addr = cur_start_addr + curMemSection.length;
            for key,memSection in sorted(self.memoryMap.items()):
                if( (memSection.name == curMemSection.name) or
                    (memSection.name == "L2RAM_C66x_1" and curMemSection.name == "L2RAM_C66x_2") or
                    (memSection.name == "L2RAM_C66x_2" and curMemSection.name == "L2RAM_C66x_1") ):
                    continue;
                if (memSection.origin >= 0x100000000):
                    continue;
                start_addr = memSection.origin;
                end_addr = start_addr + memSection.length;
                if(cur_start_addr >= start_addr and cur_start_addr < end_addr) :
                    is_overlap = True;
                    print( "WARNING: %s memory map has a overlap between %s and %s sections" %
                                (self.name, curMemSection.name, memSection.name)
                            );
                elif (cur_end_addr > start_addr and cur_end_addr <= end_addr) :
                    is_overlap = True;
                    print( "WARNING: %s memory map has a overlap between %s and %s sections" %
                                (self.name, curMemSection.name, memSection.name)
                            );
        return is_overlap;

