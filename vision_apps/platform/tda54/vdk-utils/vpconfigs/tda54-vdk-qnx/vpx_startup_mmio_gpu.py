#############################################################################
# Copyright 2026 Synopsys, Inc.                                             #
#                                                                           #
# Permission is hereby granted, free of charge, to any person obtaining a   #
# copy of this software and associated documentation files (the             #
# "Software"), to deal in the Software without restriction, including       #
# without limitation the rights to use, copy, modify, merge, publish,       #
# distribute, sublicense, and/or sell copies of the Software, and to        #
# permit persons to whom the Software is furnished to do so, subject to     #
# the following conditions:                                                 #
#                                                                           #
# The above copyright notice and this permission notice shall be included   #
# in all copies or substantial portions of the Software.                    #
#                                                                           #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS   #
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                #
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.    #
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY      #
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,      #
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE         #
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                    #
############################################################################# 

import os, inspect, sys
import subprocess
import time, platform

model_name = "virtio_gpu"

def get_sim_work_dir():
    return vpx.get_simulation_working_directory()

def get_path_to_extapps():
    ### Will try a few paths
    # 1 Packaged VDK from VDK Creator
    # 2 Extapps in SNPS_VP_HOME

    # Try path 1
    extAppsDir = os.path.abspath(os.path.join(get_sim_work_dir(), "../../extapps/"))

    if not os.path.isdir(extAppsDir):
        # Try path 2
        extAppsDir = os.path.abspath(os.path.join(os.environ['SNPS_VP_HOME'], "extapps/"))

        if not os.path.isdir(extAppsDir):
            print("Could not find the path to the extapps")
            return None

    # Returning path to extapps found
    return os.path.abspath(extAppsDir)
    
def init_display():
    global model_name
    dptx0_display = vpx.get_full_model_path(model_name)
    dptx0_display = dptx0_display.replace("/", ".")
    print("Initialise ATPS2LCD " + dptx0_display[1:])
    screen = "TDA5_System.TDA5_SoC.MCU_Domain.Designware_IP.VIRTIO_GPU.virtio_gpu.SCREEN0"
    
    if not screen in vpx.get_vpsession_clients():
        # do not open if already connected (e.g. for checkpointing)
        if vpx.get_os_name() == "Linux":
            launch_script = "launch.sh"
            atps2lcd =      "ATPS2LCD_NOKMI_SDL2"
        else:
            launch_script = "launch.bat"
            atps2lcd =      "ATPS2LCD_NOKMI_SDL2.exe"
        # Get path to extapps directory
        extAppsDir = get_path_to_extapps()
        print("#####LCD Panel Launch#####...")
        print(extAppsDir)
        launch = os.path.join(extAppsDir, launch_script)
        applet = os.path.join(extAppsDir, atps2lcd)
        cmd = [launch, applet,
               "--starter-key", vpx.get_starter_key(),
               "--clcdc", screen,
               "--title", screen,
               "--client-app-class", screen
               ]
        print(cmd)
        subprocess.Popen(cmd)
        time.sleep(1)
    



def disconnected_cb(sim):
    print("In the disconnected cb")
    #global proc_pid
    #print(f"Killing the Subprocess with PID: {proc_pid}")
    #os.kill(proc_pid, signal.SIGTERM)
    vpx.remove_callback('connected', 'init_display()')
    vpx.remove_callback('disconnected', 'disconnected_cb(__sim__)')

vpx.add_callback("connected", "init_display()")
vpx.add_callback('disconnected', 'disconnected_cb(__sim__)')
