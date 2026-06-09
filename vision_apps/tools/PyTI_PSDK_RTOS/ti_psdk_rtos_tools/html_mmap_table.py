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

class HtmlMmapTable :

    css = "" \
          "<style type=\"text/css\">\n" \
          ".tg  {border-collapse:collapse;border-spacing:0;border-color:#999;}\n" \
          ".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:#999;color:#444;background-color:#F7FDFA;}\n" \
          ".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:#999;color:#fff;background-color:#26ADE4;}\n" \
          ".tg .tg-kftd{background-color:#efefef;text-align:left;vertical-align:top}\n" \
          ".tg .tg-6sgx{background-color:#ffffff;text-align:left;vertical-align:top}\n" \
          ".tg .tg-fjir{background-color:#343434;color:#ffffff;text-align:left;vertical-align:top}\n" \
          "</style>\n"

    def __init__(self, memoryMap, name="./memory_map.html"):
        self.memoryMap = memoryMap;
        self.name = name;
        self.indent = "    ";
        self.indent_level = 0;

    def open(self) :
        os.makedirs(os.path.dirname(self.name), exist_ok=True);
        self.html_file = open(self.name, "w");
        self.write_line( "<!DOCTYPE html>" );
        self.write_line( "<html>" );
        self.write_line(self.css);
        self.inc_indent();
        self.write_line( "<head>" );
        self.inc_indent();
        self.write_line( "<title>" + self.memoryMap.name + "</title>");
        self.dec_indent();
        self.write_line( "</head>" );
        self.write_line( "<body>" );
        self.inc_indent();
        self.write_line( "<h1>" + self.memoryMap.name + "</h1>");
        self.write_line( "<p>" + "Note, this file is auto generated using PyTI_PSDK_RTOS tool" + "</p>");
        self.write_line( "<table class=\"tg\">" );
        self.inc_indent();
        self.write_line( "<tr>" );
        self.inc_indent();
        self.write_line( "<th class=\"tg-fjir\">" + "Name" + "</th>");
        self.write_line( "<th class=\"tg-fjir\">" + "Start Addr" + "</th>");
        self.write_line( "<th class=\"tg-fjir\">" + "End Addr" + "</th>");
        self.write_line( "<th class=\"tg-fjir\">" + "Size " + "</th>");
        self.write_line( "<th class=\"tg-fjir\">" + "Attributes" + "</th>");
        self.write_line( "<th class=\"tg-fjir\">" + "Description" + "</th>");
        self.dec_indent();
        self.write_line( "</tr>" );

    def write_line(self, string) :
        for i in range(0, self.indent_level):
            self.html_file.write(self.indent);
        self.html_file.write(string + "\n");

    def inc_indent(self) :
        self.indent_level = self.indent_level+1;

    def dec_indent(self) :
        self.indent_level = self.indent_level-1;

    def close(self) :
        self.dec_indent();
        self.write_line( "</table>" );
        self.dec_indent();
        self.write_line( "</body>" );
        self.dec_indent();
        self.write_line( "</html>" );
        self.html_file.close();

    @staticmethod
    def sortKey(mmapTuple) :
        return mmapTuple[1].origin;

    def export(self) :
        KB = 1024;
        MB = KB*KB;
        GB = KB*MB;
        odd = False;
        self.open();
        for key,memSection in sorted(self.memoryMap.memoryMap.items(), key=LinkerCmdFile.sortKey):
            sizeString = "";
            if (memSection.length > GB ) :
                sizeString = '%5.2f GB' % (memSection.length/GB)
            elif (memSection.length > MB ) :
                sizeString = '%5.2f MB' % (memSection.length/MB)
            elif (memSection.length > KB ) :
                sizeString = '%5.2f KB' % (memSection.length/KB)
            else :
                sizeString = '%d B' % (memSection.length)
            if ( odd ) :
                css_string = "\"tg-6sgx\"";
                odd = False;
            else :
                css_string = "\"tg-kftd\"";
                odd = True;
            self.write_line( "<tr>" );
            self.inc_indent();
            self.write_line( "<td class=" + css_string + ">" + memSection.name + "</td>");
            self.write_line( "<td class=" + css_string + ">" + "0x%08X" %  (memSection.origin) + "</td>");
            self.write_line( "<td class=" + css_string + ">" + "0x%08X" %  (memSection.origin + memSection.length -1 ) + "</td>");
            self.write_line( "<td class=" + css_string + ">" + sizeString + "</td>");
            self.write_line( "<td class=" + css_string + ">" + memSection.attributes + "</td>");
            self.write_line( "<td class=" + css_string + ">" + memSection.comment + "</td>");
            self.dec_indent();
            self.write_line( "</tr>" );
        self.close();
