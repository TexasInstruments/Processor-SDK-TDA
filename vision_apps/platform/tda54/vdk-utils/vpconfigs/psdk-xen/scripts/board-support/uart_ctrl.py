# This distribution contains contributions or derivatives under copyright
# as follows:
#
# Copyright (c) 2026, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - Neither the name of Texas Instruments nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sim
import sim_utils
from sim_utils.sim_print_messages import *


class uart_ctrl(object):
    def __init__(self,UART_PHY_PATH):
        sim.wait_for((10,'ms'))
        self.UART_PHY_PROBE = sim_utils.UartPhyProbe(UART_PHY_PATH)
        self.uart_match_probe = sim.BoolProbe( UART_PHY_PATH + ".string_match_detected")
        self.string_match_detected=False

    def string_match_callback(self,probe):
        self.string_match_detected=True

    def expect(self,match_string, timeout_ms=10000, timestep_ms=100):
        self.string_match_detected=False
        self.UART_PHY_PROBE.execute_command('set_strcmp', [match_string])
        self.uart_match_probe.callback_on_any_value_change()
        self.uart_match_probe.set_callback(self.string_match_callback)    #it will be called only once
        time=0
        while not self.string_match_detected:
            sim.wait_for((timestep_ms,'ms'))
            time+=timestep_ms
            if time>timeout_ms:
                ERROR_MSG('Timeout waiting for string_match_detected for "' + match_string + '"')
                break

    def send_if_match(self,command_line):
        if self.string_match_detected:
            self.UART_PHY_PROBE.send_string(command_line)

    def send(self,command_line):
        self.UART_PHY_PROBE.send_string(command_line)



