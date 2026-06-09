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
import os,inspect, sys
import time, platform
import subprocess
import signal

#proc_pid = 0
#vpCfg = vpx.get_current_vpconfig()
#vpCfgPath = os.path.dirname(vpCfg.get_path())
def get_sim_work_dir():
    return vpx.get_current_vpconfig().get_working_dir()

def find_libstdcpp():
    lib_paths = [
        "/usr/lib/x86_64-linux-gnu",
        "/usr/lib/x86_64-linux-gnu/dri"
    ]
    
    return lib_paths

def get_path_to_extapps():
    ### Will try a few paths
    # 1 Packaged VDK from VDK Creator
    # 2 Packaged VDK with PCT script
    # 3 Extapps in SNPS_VP_HOME
    # Try path 1
    extAppsDir = os.path.abspath(os.path.join(get_sim_work_dir(), "../../extapps/"))
    if not os.path.isdir(extAppsDir):

        # Try path 2
        extAppsDir = os.path.abspath(os.path.join(get_sim_work_dir(), "../extapps/"))

        if not os.path.isdir(extAppsDir):

            # Try path 3
            extAppsDir = os.path.abspath(os.path.join(os.environ['SNPS_VP_HOME'], "extapps/"))

            if not os.path.isdir(extAppsDir):
                print("Could not find the path to the extapps")
                return None

    # Returning path to extapps found
    return os.path.abspath(extAppsDir)

#print("Within vpx_startup_launch_lcd_display.py")
def connected_cb():
    print("In the connected cb")
    #global proc_pid
    #lcd_app = "lcd_display"
    #lcd_dir_path = os.path.abspath(os.path.join(vpCfgPath + "/../shared/extapps"))
    #print('lcd_dir_path ', lcd_dir_path)
    
    devicename = "TDA5_System.TDA5_SoC.Main_Domain.DSS.DISPC_0_VP1"
    panel=devicename
    print("Open virtual LCD Panel " + panel)
    if not panel in vpx.get_vpsession_clients():
        if vpx.get_os_name() == "Linux":
            launch_script = "launch.sh"
            atps2lcd =      "ATPS2LCD_NOKMI_SDL2"
            
            os_info = platform.freedesktop_os_release()
            os_id = os_info["ID"]
            
            # check if we are on an Ubuntu - the ATPS2LCD panel application does not start unless we append the STD C++ library upfront
            #  TODO this is possibily a workaround in lieu of compiling the ATPS2LCD app with a newer compiler chain
            if os_id == "ubuntu":
                current_ld_lib_path = os.getenv('LD_LIBRARY_PATH')
                if current_ld_lib_path:
                    current_paths = current_ld_lib_path.split(':')
                else:
                    current_paths = []

                # Initialize found_lib
                found_lib = find_libstdcpp()
                for lib in found_lib:
                    if lib:
                        if lib not in current_paths:
                            current_ld_lib_path = f"{current_ld_lib_path}:{lib}"
                        else:
                            print("libstdcpp not found in the expected directories")
                os.environ['LD_LIBRARY_PATH'] = current_ld_lib_path                   
        else:
            launch_script = "launch.bat"
            atps2lcd =      "ATPS2LCD_NOKMI_SDL2.exe"
        #Get path to extapps directory
        extAppsDir = get_path_to_extapps()
        print("#####LCD Panel Launch#####...")
        print(extAppsDir)
        launch = os.path.join(extAppsDir, launch_script)
        applet = os.path.join(extAppsDir, atps2lcd)
    
        cmd = [launch, applet,
            "--starter-key", vpx.get_starter_key(),
            "--clcdc", panel,
            "--title", panel,
            "--client-app-class", "ATPS2LCD_" + panel,
            "--lcd-format-override", "BGRA32"
            ">", vpx.get_simulation_working_directory() + "/ATPS2LCD_" + panel + ".log",
            "2>&1"
            ]
        print(cmd)
        process=subprocess.Popen(cmd)
        time.sleep(1)

def disconnected_cb(sim):
    print("In the disconnected cb")
    vpx.remove_callback('connected', 'connected_cb()')
    vpx.remove_callback('disconnected', 'disconnected_cb(__sim__)')

vpx.add_callback('connected', 'connected_cb()')
vpx.add_callback('disconnected', 'disconnected_cb(__sim__)')
