#
# Copyright (c) 2025 Texas Instruments Incorporated
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
from ti_psdk_rtos_tools import *

class QNX_BSP_UPDATE :
    def __init__(self, memoryMap):
        self.memoryMap = memoryMap;

    @staticmethod
    def sortKey(mmapTuple) :
        return mmapTuple[1].origin;

    def export(self) :
        KB = 1024;
        MB = KB*KB;
        GB = KB*MB;

        with open("../../../../psdkqa/qnx/bsp/images/am62a-evm-ti.build", "r") as file:
            data = file.read()
        
        carveout_text_1 = "[+keeplinked] startup-am62a-evm -u arg -v -r0x80000000,0x0007FFFF,1 -r"

        for key,memSection in sorted(self.memoryMap.memoryMap.items(), key=CHeaderFile.sortKey):
            if (memSection.name == "DDR_C7x_1_BOOT"):
                string1 = f'{carveout_text_1}0x{memSection.origin:08X},'
                start_addr = memSection.origin
            if (memSection.name == "DDR_C7X_1_SCRATCH"):
                bsp_size_bytes = memSection.origin + memSection.length - start_addr
                string2 = f'0x{bsp_size_bytes:08X},1'
        
        string = string1 + string2
        carveout_pattern = rf'{re.escape(carveout_text_1)}0x[0-9A-Fa-f]+,\s*0x[0-9A-Fa-f]+,1'
        data = re.sub(carveout_pattern, string, data)

        with open("../../../../psdkqa/qnx/bsp/images/am62a-evm-ti.build", "w") as file:
           data = file.write(data)
