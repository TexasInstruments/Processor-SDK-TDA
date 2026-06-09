#############################################################################
# Copyright 1996-2026 Synopsys, Inc.                                        #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
#############################################################################

import os, inspect, sys
import subprocess
import time, platform
from pathlib import Path

model_name = "virtio_gpu"

# def get_sim_work_dir():
#   return os.getcwd()


def get_sim_work_dir():
    return vpx.get_current_vpconfig().get_working_dir()


def find_libstdcpp():
    lib_paths = ["/usr/lib/x86_64-linux-gnu", "/usr/lib/x86_64-linux-gnu/dri"]

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
            extAppsDir = os.path.abspath(
                os.path.join(os.environ["SNPS_VP_HOME"], "extapps/")
            )

            if not os.path.isdir(extAppsDir):
                print("Could not find the path to the extapps")
                return None

    # Returning path to extapps found
    print("extAppsDir: " + extAppsDir)
    return os.path.abspath(extAppsDir)


def init_display():
    global model_name
    screen = (
        "TDA5_System.TDA5_SoC.MCU_Domain.Designware_IP.VIRTIO_GPU.virtio_gpu.SCREEN0"
    )

    if not screen in vpx.get_vpsession_clients():
        # do not open if already connected (e.g. for checkpointing)
        if vpx.get_os_name() == "Linux":
            launch_script = "launch.sh"
            atps2lcd = "ATPS2LCD_NOKMI_SDL2"
            os_info = platform.freedesktop_os_release()
            os_id = os_info["ID"]

            # check if we are on an Ubuntu - the ATPS2LCD panel application does not start unless we append the STD C++ library upfront
            #  TODO this is possibily a workaround in lieu of compiling the ATPS2LCD app with a newer compiler chain
            if os_id == "ubuntu":
                current_ld_lib_path = os.getenv("LD_LIBRARY_PATH")
                if current_ld_lib_path:
                    current_paths = current_ld_lib_path.split(":")
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
                os.environ["LD_LIBRARY_PATH"] = current_ld_lib_path
        else:
            launch_script = "launch.bat"
            atps2lcd = "ATPS2LCD_NOKMI_SDL2.exe"
        # Get path to extapps directory
        extAppsDir = get_path_to_extapps()
        print("#####LCD Panel Launch#####...")
        print(extAppsDir)
        launch = os.path.join(extAppsDir, launch_script)
        applet = os.path.join(extAppsDir, atps2lcd)
        cmd = [
            launch,
            applet,
            "--starter-key",
            vpx.get_starter_key(),
            "--clcdc",
            screen,
            "--title",
            screen,
            "--client-app-class",
            screen,
        ]
        print(cmd)
        subprocess.Popen(cmd)
        time.sleep(1)


def disconnected_cb(sim):
    print("In the disconnected cb")
    # global proc_pid
    # print(f"Killing the Subprocess with PID: {proc_pid}")
    # os.kill(proc_pid, signal.SIGTERM)
    vpx.remove_callback("connected", "init_display()")
    vpx.remove_callback("disconnected", "disconnected_cb(__sim__)")


vpx.add_callback("connected", "init_display()")
vpx.add_callback("disconnected", "disconnected_cb(__sim__)")
